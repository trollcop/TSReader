#include <windows.h>
#include <commctrl.h>
#include "TSReader.h"
#include "bcdmux.h"
#include "util.h"
#include "sources.h"

#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>

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

	buffer = LocalAlloc(LMEM_FIXED, PES_BUFSIZ);
	if (!buffer)
		return;

	const AVCodec *codec = avcodec_find_decoder(codec_id);
	AVCodecContext *ctx = avcodec_alloc_context3(codec);
	ret = avcodec_open2(ctx, codec, NULL);

	AVFrame *frame = av_frame_alloc();

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
			/* this will fail to decode but whatever */
			if (nReadLength > PES_BUFSIZ)
				nReadLength = PES_BUFSIZ;
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
			dbg_printf("%s: sent packet of %d bytes, avcodec_send_packet returned 0x%08x\n", __FUNCTION__, nPacketLength, ret);
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

static uint8_t *pThumbnail[REAL_MAX_ES_PARSERS] = { 0, };
static int16_t *pSamples[REAL_MAX_ES_PARSERS] = { 0, };
static int nSampleWriteIndex[REAL_MAX_ES_PARSERS] = { 0, };

static void AudioDecoderThread(PESPARSERINFO esparserinfo, enum AVCodecID codec_id, BOOL incl_size)
{
	uint8_t *buffer = NULL;
	BOOL fGeneratedThumbnail = FALSE;
	BOOL fDone = FALSE;
	int nDestWidth = 240;
	int nDestHeight = 52;
	int ret;

	dbg_printf("%s (%d)+ %d/%d\n", __FUNCTION__, codec_id, esparserinfo->nProgramNumber, esparserinfo->nES);

	EnterCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);
	v->fMPEG2DecoderThreadRunning[esparserinfo->nES] = TRUE;
	LeaveCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);

	switch (v->nThumbnailSize) {
		case 1:
			nDestWidth *= 2; nDestWidth /= 3;
			break;
		case 2:
			nDestWidth *= 2;
			break;
	}

	pThumbnail[esparserinfo->nES] = LocalAlloc(LPTR, nDestWidth * nDestHeight * 3);
	pSamples[esparserinfo->nES] = LocalAlloc(LMEM_FIXED, sizeof(int16_t) * SAMPLES_REQUIRED);
	nSampleWriteIndex[esparserinfo->nES] = 0;
	buffer = LocalAlloc(LMEM_FIXED, PES_BUFSIZ);
	if (!pThumbnail || !pSamples || !buffer)
		return;

	const AVCodec *codec = avcodec_find_decoder(codec_id);
	AVCodecContext *ctx = avcodec_alloc_context3(codec);
	ret = avcodec_open2(ctx, codec, NULL);

	AVFrame *frame = av_frame_alloc();

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
			/* this will fail to decode but whatever */
			if (nReadLength > PES_BUFSIZ)
				nReadLength = PES_BUFSIZ;
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
			dbg_printf("%s: sent packet of %d bytes, avcodec_send_packet returned 0x%08x\n", __FUNCTION__, nPacketLength, ret);
		av_packet_unref(pkt);
		av_packet_free(&pkt);

		while (avcodec_receive_frame(ctx, frame) == 0) {
			int j;
			AVChannelLayout stereo = AV_CHANNEL_LAYOUT_STEREO;

			/* setup resampling/downmixing */
			SwrContext *swr_ctx = swr_alloc();
			av_opt_set_chlayout(swr_ctx, "in_chlayout", &frame->ch_layout, 0);
			av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", frame->format, 0);
			av_opt_set_int(swr_ctx, "in_sample_rate", frame->sample_rate, 0);

			av_opt_set_chlayout(swr_ctx, "out_chlayout", &stereo, 0);
			av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
			av_opt_set_int(swr_ctx, "out_sample_rate", frame->sample_rate, 0);
			swr_init(swr_ctx);

			int out_samples = (int)av_rescale_rnd(swr_get_delay(swr_ctx, frame->sample_rate) + frame->nb_samples, frame->sample_rate, frame->sample_rate, AV_ROUND_UP);
			uint8_t *raw_buffer = NULL;
			av_samples_alloc(&raw_buffer, NULL, 2, out_samples, AV_SAMPLE_FMT_S16, 0);
			int converted_samples = swr_convert(swr_ctx, &raw_buffer, out_samples, (const uint8_t **)frame->data, frame->nb_samples);
			int16_t *audio_samples = (int16_t *)raw_buffer;

			for (j = 0; j < 256 * 2; j += 2) {
				pSamples[esparserinfo->nES][nSampleWriteIndex[esparserinfo->nES]++] = audio_samples[0];
				pSamples[esparserinfo->nES][nSampleWriteIndex[esparserinfo->nES]++] = audio_samples[1];

				audio_samples += 2;

				if (nSampleWriteIndex[esparserinfo->nES] == SAMPLES_REQUIRED)
					break;
			}

			av_frame_unref(frame);

			av_freep(&raw_buffer);
			swr_close(swr_ctx);
			swr_free(&swr_ctx);


			if (nSampleWriteIndex[esparserinfo->nES] == SAMPLES_REQUIRED) {

				GenerateAudioThumbnail(pSamples[esparserinfo->nES], 2, nDestWidth, nDestHeight, pThumbnail[esparserinfo->nES], v->nESParsePMTIndex[esparserinfo->nES], v->nESParseESIndex[esparserinfo->nES]);

				fGeneratedThumbnail = TRUE;
				fDone = TRUE;

				break;
			}
		}
	} while (!fDone);

	dbg_printf("TSReader: Completed Audio decoding for program %d ES thread %d\n", esparserinfo->nProgramNumber, esparserinfo->nES);

	/* close PES pipes */
	CloseHandle(v->hMPEGDecoderReadPipe[esparserinfo->nES]);
	CloseHandle(v->hMPEGDecoderWritePipe[esparserinfo->nES]);

	LocalFree(buffer);
	av_frame_free(&frame);
	avcodec_free_context(&ctx);

	if (!fGeneratedThumbnail)
		LocalFree(pThumbnail[esparserinfo->nES]);
	LocalFree(pSamples[esparserinfo->nES]);

	EnterCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);
	v->fESParseDecoderCompletedLibMPEG[esparserinfo->nES] = TRUE;
	v->fMPEG2DecoderThreadRunning[esparserinfo->nES] = FALSE;
	LeaveCriticalSection(&v->esparserinfo[esparserinfo->nES].csThreadSignal);

	dbg_printf("%s (%d)- %d/%d\n", __FUNCTION__, codec_id, esparserinfo->nProgramNumber, esparserinfo->nES);
}

DWORD WINAPI GenericDecoderThread(LPVOID lpv)
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

		case DEC_AAC:
			AudioDecoderThread(esparserinfo, AV_CODEC_ID_AAC, TRUE);
			break;

		default:
			dbg_printf("%s: Unhandled decoder %d\n", __FUNCTION__, esparserinfo->eDecoder);
			break;
	}

	return 0;
}
