/* DirectShowTSTarget.h */
#ifndef DIRECTSHOWTSTARGET_H___
#define DIRECTSHOWTSTARGET_H___

#include "TSTarget.h"
#include "Interface.h"
#include "InterfaceSourceFilter.h"

#include <streams.h>
#include <Bdaiface.h>

class TSPidFilter;
/*********************************************************************************
 *                                                                               *
 * DirectShowTSTarget.h TSTarget Implementation using DirecShow                  *
 *                                                                               *
 * Copyright (C) 2003      Ware4z                                                *                                                                               *
 *                                                                               *
 * This program is free software; you can redistribute it and/or                 *
 * modify it under the terms of the GNU General Public License                   *
 * as published by the Free Software Foundation; either version 2                *
 * of the License, or (at your option) any later version.                        *
 *                                                                               *
 *                                                                               *          
 * This program is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of                *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *
 * GNU General Public License for more details.                                  *
 *                                                                               *
 *                                                                               *
 * You should have received a copy of the GNU General Public License             *
 * along with this program; if not, write to the Free Software                   *
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.    *
 * Or, point your browser to http://www.gnu.org/copyleft/gpl.html                *
 *                                                                               *
 *                                                                               *
 * The author can be reached at j_anderw@sbox.tugraz.at                          *
 *
 *     Version: 0.0.2
 *
 *********************************************************************************/ 




/********************************************************************
 * class DirectShowTSTarget
 *
 * This class implements the requested interface of TSTarget. It uses
 * MS DirectShow Playback to display video and play audio.
 *
 *
 *******************************************************************/



class DirectShowTSTarget : public TSTarget
{
protected:
	bool Mute_;
	bool FullScreen_;
	long Volume_;
	bool running_;
	IGraphBuilder *GraphBuilder_;
	IBasicAudio   * BasicAudio_;
	IVideoWindow  * VideoWindow_;
	IMediaControl * MediaControl_;
	//IBase Filters
	IBaseFilter * SourceFilter_;
	IBaseFilter * DVBDemux_;
	IBaseFilter * VideoDecoder_;
	IBaseFilter * AudioDecoder_;
	IBaseFilter * AudioRenderer_;
	IBaseFilter * VideoRenderer_;
	//Interfaces
	ISourceFilter *ISourceFilter_;
	IMpeg2Demultiplexer *IMPEG2Demux_;
	TSPidFilter *PATFilter_;
	TSPidFilter *PMTFilter_;
	TSPidFilter *AudioFilter_;
	TSPidFilter *VideoFilter_;
	TSPidFilter *PCRFilter_;
public:
	DirectShowTSTarget(const char *Name);

	virtual ~DirectShowTSTarget();

	//------------------------------------------------------------------
	//creates TSPidFilter
	//does not store it internaly, must be deleted by the caller
	HRESULT createTSTargetFilter(const unsigned short Pid, const unsigned int PidType, const char *PidName, TSPidFilter **Filter);
	//------------------------------------------------------------------
	HRESULT deleteTSTargetFilter( const unsigned int PidType);
	//------------------------------------------------------------------
	//starts / stops playback
	HRESULT togglePlayback();
	//------------------------------------------------------------------
	//sets to mute or volume on
	HRESULT toggleVolume();
	
	//------------------------------------------------------------------
	//sets either to full screen mode or to normal mode
	HRESULT toggleFullScreen(HWND hWnd);
	
	//------------------------------------------------------------------
	//returns the property page of this module when available
	HRESULT getProperties(HINSTANCE hInstOwner, HWND hWndOwner, HWND * Window, unsigned int * CommandId);
	//------------------------------------------------------------------
	//sets the video target window & position
	HRESULT setVideoProperties(HWND hWnd, RECT rect);
	
	//------------------------------------------------------------------
	//used for dshow implementation to load an external graph file
	HRESULT setTargetFileName(const char * Name);

protected:

	//Internal DShow Filter Functions
	//creates an IBaseFilter and adds it to the Graph
	HRESULT createIBaseFilterAndAddToGraph(wchar_t * filter_name, CLSID filter_id, IBaseFilter ** Filter);
	//queries an IBasefilter for a requested interface
	HRESULT queryInterface(IBaseFilter * Filter, REFIID ID, void ** Interface);
	//removes all Filter from the Graph
	HRESULT removeAllFilterFromGraph();
	//connects two IBaseFilter given on their pin names
	HRESULT connectIBaseFilters(IBaseFilter * source_filter, wchar_t * source_pin, IBaseFilter * target_filter, wchar_t * target_pin);
	//creates an IPin on the MS MPEG-2 Demultiplexer
	HRESULT createMPEG2DemuxPin(wchar_t * PinName, IPin ** Pin, unsigned char PinType);
	//maps an Pid of the input stream to an existing pin of the MS MPEG-2 Demultiplexer
	HRESULT mapMPEG2DemuxPin(IPin * pin, unsigned short Pid, MEDIA_SAMPLE_CONTENT content);
	//queries an IBaseFilter for a pin
	HRESULT getPin(IBaseFilter *filter, wchar_t * pin_name, IPin ** pin);
	


};

#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }

#define FN_AUDIO_DECODER	L"Audio Decoder"
#define PN_AUDIO_DECODER_IN			L"XForm In"
#define PN_AUDIO_DECODER_OUT		L"XFrom Out"
#define PN_MPEG2_DEMUX_IN	L"MPEG-2 Stream"
#define PN_AUDIO_RENDERER_IN	L"Audio Input pin (rendered)"
#ifdef USE_DX3R_ACCELERATION	//Sigma Designs Hardware MPEG-2 Decoder"
	#define FN_VIDEO_DECODER	L"Sigma Designs Hardware MPEG-2 Decoder"
	#define PN_VIDEO_DECODER_IN			L"Video In"
	#define PN_VIDEO_DECODER_OUT		L"Overlay Out"
static CLSID CLSID_VideoDecoder = //Sigma Design MPEG-2 hardware decoder rmmpeg2.ax
	{0x4E3ABD41,0x458E,0x11D1,{0x91,0x7E,0x00,0x00,0x1B,0x4F,0x00,0x6F}};
#else							//InterVideo Video Decoder
	#define FN_VIDEO_DECODER	L"InterVideo Video Decoder"
	#define PN_VIDEO_DECODER_IN			L"Video Input"
	#define PN_VIDEO_DECODER_OUT		L"Video Output"

static CLSID CLSID_VideoDecoder =  //InterVideo Video Decoder
	{0x0246ca20,0x776d,0x11d2,{0x80,0x10,0x00,0x10,0x4b,0x9b,0x85,0x92}};
#endif

// Default Video Renderer
#define FN_VIDEO_RENDERER	L"Video Renderer"
#define PN_VIDEO_RENDERER_IN			L"Input"





#endif /* DirectShowTSTarget.h */