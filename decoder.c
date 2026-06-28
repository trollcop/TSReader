#include <windows.h>
#include <commctrl.h>
#include "TSReader.h"
#include "bcdmux.h"
#include "util.h"

#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>

extern PVARIABLES v;

static float GetVideoFrameRate(AVCodecContext *ctx)
{
	AVRational fps_rational;

	if (ctx->framerate.num != 0 && ctx->framerate.den != 0) {
		/* codec context has the exact framerate */
		fps_rational = ctx->framerate;
	} else if (ctx->time_base.num != 0 && ctx->time_base.den != 0) {
		/* fallback - invert time_base to get frame rate */
		fps_rational.num = ctx->time_base.den;
		fps_rational.den = ctx->time_base.num;
	} else {
		/* nothing provided at all */
		fps_rational = (AVRational){ 0, 1 };
	}

	if (fps_rational.num > 0)
		return (float)av_q2d(fps_rational);

	return 0.0f;
}

static ChromaFormat GetChromaSubsampling(AVCodecContext *ctx)
{
	const char *pix_fmt_name = av_get_pix_fmt_name(ctx->pix_fmt);
	if (pix_fmt_name) {
		dbg_printf("%s pixel format: %s\n", __FUNCTION__, pix_fmt_name);

		if (strstr(pix_fmt_name, "420"))
			return CHROMA_420;
		else if (strstr(pix_fmt_name, "422"))
			return CHROMA_422;
		else if (strstr(pix_fmt_name, "444"))
			return CHROMA_444;
	}

	return CHROMA_RESERVED;
}

static void GetAspectRatio(const AVFrame *frame, int *dar_num, int *dar_den)
{
	AVRational sar = frame->sample_aspect_ratio;
	/* if it wasn't specified */
	if (sar.num == 0 || sar.den == 0) {
		sar.num = 1;
		sar.den = 1;
	}

	av_reduce(dar_num, dar_den,
		frame->width * sar.num,
		frame->height * sar.den,
		1024 * 1024);
}

static InterlacedType GetFrameInterlace(AVFrame *frame)
{
	if (frame->flags & AV_FRAME_FLAG_INTERLACED)
		if (frame->flags & AV_FRAME_FLAG_TOP_FIELD_FIRST)
			return INT_TFF;
		else
			return INT_BFF;
	
	return INT_PROGRESSIVE;
}

#define PES_BUFSIZ	(1024 * 1024)

static void VideoDecoderThread(PESPARSERINFO esparserinfo, enum AVCodecID codec_id, BOOL incl_size)
{
	uint8_t *buffer = NULL;
	int nMaximumPictures = v->nMaximumMPEGPictures;
	int nPictureCount = 0;
	BOOL fDone = FALSE;
	int ret;

	dbg_printf("%s (%d)+ %d/%d\n", __FUNCTION__, codec_id, esparserinfo->nProgramNumber, esparserinfo->nES);

	EnterCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);
	v->fMPEG2DecoderThreadRunning[esparserinfo->nES] = TRUE;
	LeaveCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);

	if (v->pat.pmt[v->nESParsePMTIndex[esparserinfo->nES]].es[v->nESParseESIndex[esparserinfo->nES]].nStreamType == 0x80)
		nMaximumPictures = v->nMaximumDCIIPictures;

	const AVCodec *codec = avcodec_find_decoder(codec_id);
	AVCodecContext *ctx = avcodec_alloc_context3(codec);
	ret = avcodec_open2(ctx, codec, NULL);

	AVFrame *frame = av_frame_alloc();
	buffer = LocalAlloc(LMEM_FIXED, PES_BUFSIZ);

	do {
		int nReadLength;
		int nReturned;
		int nPacketLength;

		if (v->fRunning == FALSE)
			break;

		if (incl_size) {
			/* read PES packet length from pipe */
			nReturned = ReadFromMPEG2ESPipe((BYTE *)&nReadLength, sizeof(nReadLength), esparserinfo->nES);
			if (nReadLength == 0 || nReturned == 0)
				break;
		} else {
			nReadLength = PES_BUFSIZ;
		}

		nPacketLength = ReadFromMPEG2ESPipe(buffer, nReadLength, esparserinfo->nES);
		if (nPacketLength == 0)
			break;

		if (v->fRunning == FALSE)
			break;

		AVPacket *pkt = av_packet_alloc();

		pkt->data = buffer;
		pkt->size = nPacketLength;

		ret = avcodec_send_packet(ctx, pkt);
		if (ret != 0)
			dbg_printf("sent packet of %d bytes, avcodec_send_packet returned 0x%08x\n", nPacketLength, ret);
		av_packet_unref(pkt);
		av_packet_free(&pkt);

		while (avcodec_receive_frame(ctx, frame) == 0) {
			int nDestHeight, nDestWidth;
			int nSourceHeight = frame->height;

			/* skip non-I frames for display, as those might have decoding garbage artifacts */
			if (frame->pict_type != AV_PICTURE_TYPE_I) {
				av_frame_unref(frame);
				break;
			}

			/* add parsed video header for further display */
			if (v->pat.pmt[v->nESParsePMTIndex[esparserinfo->nES]].es[v->nESParseESIndex[esparserinfo->nES]].pParsedData == NULL) {
				PPARSEDGENERICVIDEO pVideo = LocalAlloc(LPTR, sizeof(PARSEDGENERICVIDEO));
				if (pVideo) {
					pVideo->tag = PARSER_TAG;
					pVideo->width = frame->width;
					pVideo->height = frame->height;
					pVideo->interlaced = GetFrameInterlace(frame);
					pVideo->framerate = GetVideoFrameRate(ctx);
					pVideo->chroma = GetChromaSubsampling(ctx);
					GetAspectRatio(frame, &pVideo->dar_num, &pVideo->dar_den);
					v->pat.pmt[v->nESParsePMTIndex[esparserinfo->nES]].es[v->nESParseESIndex[esparserinfo->nES]].pParsedData = (BYTE *)pVideo;
				}
			}

			GetNewThumbnailSize(&nSourceHeight, &nDestHeight, &nDestWidth);
			struct SwsContext *sws_ctx = sws_getContext(ctx->width, ctx->height, ctx->pix_fmt, nDestWidth, nDestHeight, AV_PIX_FMT_BGR24, SWS_BILINEAR, NULL, NULL, NULL);

			/* thumbnail RGB frame */
			AVFrame *rgb_frame = av_frame_alloc();
			rgb_frame->format = AV_PIX_FMT_BGR24;
			rgb_frame->width = nDestWidth;
			rgb_frame->height = nDestHeight;
			av_frame_get_buffer(rgb_frame, 0);

			ret = sws_scale(sws_ctx, (const uint8_t *const *)frame->data, frame->linesize, 0, frame->height, rgb_frame->data, rgb_frame->linesize);

			int buffer_size = av_image_get_buffer_size(AV_PIX_FMT_BGR24, rgb_frame->width, rgb_frame->height, 1);
			uint8_t *rgb_buffer = (uint8_t *)av_malloc(buffer_size);

			av_image_copy_to_buffer(rgb_buffer, buffer_size, rgb_frame->data, rgb_frame->linesize, AV_PIX_FMT_BGR24, rgb_frame->width, rgb_frame->height, 1);

			GenerateThumbnail(rgb_buffer, rgb_frame->width, rgb_frame->height, v->nESParsePMTIndex[esparserinfo->nES], v->nESParseESIndex[esparserinfo->nES]);

			av_free(rgb_buffer);
			sws_freeContext(sws_ctx);
			av_frame_free(&rgb_frame);
			av_frame_unref(frame);

			nPictureCount++;
			if (nPictureCount > nMaximumPictures) {
				fDone = TRUE;
				break;
			}
		}
	} while (!fDone);

	dbg_printf("TSReader: Completed Video decoding for program %d ES thread %d\n", esparserinfo->nProgramNumber, esparserinfo->nES);

	/* close PES pipes */
	CloseHandle(v->hMPEGDecoderReadPipe[esparserinfo->nES]);
	CloseHandle(v->hMPEGDecoderWritePipe[esparserinfo->nES]);

	/* free up used memory/frames */
	LocalFree(buffer);
	av_frame_free(&frame);
	avcodec_free_context(&ctx);

#if 0
	/* TODO */
	LoadVideoDecoderCrashThumbnail(v->nESParsePMTIndex[esparserinfo->nES], v->nESParseESIndex[esparserinfo->nES]);
#endif

	EnterCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);
	v->fESParseDecoderCompletedLibMPEG[esparserinfo->nES] = TRUE;
	v->fMPEG2DecoderThreadRunning[esparserinfo->nES] = FALSE;
	LeaveCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);

	dbg_printf("%s (%d)- %d/%d\n", __FUNCTION__, codec_id, esparserinfo->nProgramNumber, esparserinfo->nES);
}

DWORD WINAPI GenericVideoDecoderThread(LPVOID lpv)
{
	PESPARSERINFO esparserinfo = (PESPARSERINFO)lpv;

	switch (esparserinfo->eDecoder) {
		case DEC_MPEG2:
			VideoDecoderThread(esparserinfo, AV_CODEC_ID_MPEG2VIDEO, TRUE);
			break;

		case DEC_H264:
			VideoDecoderThread(esparserinfo, AV_CODEC_ID_H264, TRUE);
			break;

		case DEC_H265:
			VideoDecoderThread(esparserinfo, AV_CODEC_ID_HEVC, TRUE);
			break;

		case DEC_MPEG4:
			VideoDecoderThread(esparserinfo, AV_CODEC_ID_MPEG4, TRUE);
			break;

		case DEC_VC1:
			VideoDecoderThread(esparserinfo, AV_CODEC_ID_VC1, TRUE);
			break;

		case DEC_AV1:
			VideoDecoderThread(esparserinfo, AV_CODEC_ID_AV1, TRUE);
			break;

		default:
		case DEC_AAC:
			dbg_printf("GenericVideoDecoderThread: Unhandled decoder %d\n", esparserinfo->eDecoder);
			break;
	}

	return 0;
}
