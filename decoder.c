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
	buffer = LocalAlloc(LMEM_FIXED, 1024 * 1024);

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
			nReadLength = 0x100000; // arbitrary
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
		dbg_printf("sent packet of %d bytes, avcodec_send_packet returned 0x%08x\n", nPacketLength, ret);
		av_packet_unref(pkt);
		av_packet_free(&pkt);

		while (avcodec_receive_frame(ctx, frame) == 0) {
			int nDestHeight, nDestWidth;
			int nSourceHeight = frame->height;

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

			nPictureCount++;
			if (nPictureCount > nMaximumPictures) {
				fDone = TRUE;
				break;
			}
		}
	} while (!fDone);

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

#ifdef USE_FFMPEG
DWORD WINAPI MPEG2DecoderThread(LPVOID lpv)
{
	PESPARSERINFO esparserinfo = (PESPARSERINFO)lpv;

	VideoDecoderThread(esparserinfo, AV_CODEC_ID_MPEG2VIDEO, TRUE);

	return 0;
}

DWORD WINAPI H264DecoderThread(LPVOID lpv)
{
	PESPARSERINFO esparserinfo = (PESPARSERINFO)lpv;

	VideoDecoderThread(esparserinfo, AV_CODEC_ID_H264, FALSE);

	return 0;
}

DWORD WINAPI MPEG4DecoderThread(LPVOID lpv)
{
	PESPARSERINFO esparserinfo = (PESPARSERINFO)lpv;

	VideoDecoderThread(esparserinfo, AV_CODEC_ID_MPEG4, FALSE);

	return 0;
}
#endif
