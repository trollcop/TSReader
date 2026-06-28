#include <windows.h>
#include <commctrl.h>
#include "TSReader.h"
#include "bcdmux.h"
#include "util.h"

#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>

extern PVARIABLES v;

#define BUFFER_SIZE 20480

static uint8_t *pThumbnail[REAL_MAX_ES_PARSERS] = { 0, };
static int16_t *pSamples[REAL_MAX_ES_PARSERS] = { 0, };
static int nSampleWriteIndex[REAL_MAX_ES_PARSERS] = { 0, };

#define PES_BUFSIZ	(64 * 1024)

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
	pSamples[esparserinfo->nES] = LocalAlloc(LMEM_FIXED, sizeof(signed short) * SAMPLES_REQUIRED);
	nSampleWriteIndex[esparserinfo->nES] = 0;

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

DWORD WINAPI GenericAudioDecoderThread(LPVOID lpv)
{
	PESPARSERINFO esparserinfo = (PESPARSERINFO)lpv;

	switch (esparserinfo->eDecoder) {
		case DEC_AAC:
			AudioDecoderThread(esparserinfo, AV_CODEC_ID_AAC, TRUE);
			break;

		default:
		case DEC_MPEG2:
		case DEC_MPEG4:
		case DEC_H264:
		case DEC_H265:
		case DEC_VC1:
		case DEC_AV1:
			dbg_printf("GenericAudioDecoderThread: Unhandled decoder %d\n", esparserinfo->eDecoder);
			break;
	}

	return 0;
}
