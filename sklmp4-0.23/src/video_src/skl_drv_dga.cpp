/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_drv_dga.cpp
 *
 * DGA video driver (sub of X11)
 ********************************************************/

#if !defined(SKL_NO_VIDEO)
#if defined(SKL_USE_X11) && defined(SKL_USE_DGA)

#include "skl_video.h"
#include "./skl_drv_x11.h"

#include <X11/extensions/xf86dga.h>
//#include <X11/extensions/xf86vmode.h>

//////////////////////////////////////////////////////////
// SKL_DGA_INFO
//////////////////////////////////////////////////////////

int SKL_DGA_INFO::Init(Display *Dsp, int screen)
{
  if ( geteuid() ) return 0;
  if (!XF86DGAQueryDirectVideo(Dsp, screen, &_Flags)) return 0;
  if (!(_Flags&XF86DGADirectPresent)) return 0;
  if (!XF86DGAQueryVersion(Dsp, &_Maj, &_Min)) return 0;
  if (!XF86DGAQueryExtension(Dsp, &_Completion_Type, &_Error_Base))
    return 0;
  if (!XF86DGAGetVideo(Dsp, screen,
         (char **)&_Base_Ptr, &_Line_Width,
         &_Bank_Size, &_Mem_Size))
    return 0;
  if ( _Bank_Size >= _Mem_Size ) {
    _Bank_Nb = 0; // <- means:  Linear buffer
    _Left_Over = 0;
  }
  else {
    _Bank_Nb = _Mem_Size/_Bank_Size;
    _Left_Over = _Mem_Size - _Bank_Size*_Bank_Nb;
  }
  Get_Viewport_Size(Dsp, screen);
  return 1;
}

void SKL_DGA_INFO::Get_Viewport_Size(Display *XDsp, int screen) {
  XF86DGAGetViewPortSize(XDsp, screen, &_View_W, &_View_H);
}

void SKL_DGA_INFO::Print_Infos() const {
  printf( "  Bank:%d View=%dx%d\n", _Bank_Size, _View_W, _View_H);
}

//////////////////////////////////////////////////////////
// SKL_X11_DGA_WIN
//////////////////////////////////////////////////////////

class SKL_X11_DGA_WIN : public SKL_X11_RAW_WIN
{
  protected:

    void Clean_DGA();
    virtual int Create_XWindow();

    const SKL_DGA_INFO *Get_DGA_Infos() const { return Drv()->Get_DGA_Infos(); }

  public:

    SKL_X11_DGA_WIN(SKL_X11_WIN *w, int W=0, int H=0);
    virtual ~SKL_X11_DGA_WIN();

    virtual void Store_CMap();
    virtual void Print_Infos() const;
};

//////////////////////////////////////////////////////////

#define MASKS_DGA KeyReleaseMask | KeyPressMask       \
/* | ButtonPressMask*/ | ButtonReleaseMask    \
   | ButtonMotionMask | PointerMotionMask

//////////////////////////////////////////////////////////

SKL_X11_DGA_WIN::SKL_X11_DGA_WIN(SKL_X11_WIN *w, int W, int H)
 : SKL_X11_RAW_WIN(w, W, H)
{}

SKL_X11_DGA_WIN::~SKL_X11_DGA_WIN() {
  Clean_DGA();
}

//////////////////////////////////////////////////////////

int SKL_X11_DGA_WIN::Create_XWindow()
{
  setuid(0);
  Display *Dsp = Get_XDisplay();

  _Win = DefaultRootWindow( Dsp );
  XSelectInput(Dsp, _Win, MASKS_DGA);
  XStoreName(Dsp, _Win, Get_Name());

  XGrabPointer(Dsp, _Win, True, PointerMotionMask,
               GrabModeAsync, GrabModeAsync,
               None, None, CurrentTime);
  XGrabKeyboard(Dsp, _Win, True, GrabModeAsync, GrabModeAsync, CurrentTime);

  setuid( getuid() );  // Give up root privileges
  return 1;
}

void SKL_X11_DGA_WIN::Clean_DGA()
{
  if (_Win==0) return;
  Display *Dsp = Get_XDisplay();
  int Screen = Get_Screen();

  XFlush( Dsp );
  XF86DGADirectVideo(Dsp, Screen, 0x0);
  XUngrabKeyboard(Dsp, CurrentTime);
  XUngrabPointer(Dsp, CurrentTime);
  _Win = 0;
}

void SKL_X11_DGA_WIN::Store_CMap()
{
  SKL_X11_WIN::Store_CMap();
  Display *Dsp = Get_XDisplay();
  int Screen = Get_Screen();
  XF86DGAInstallColormap( Dsp, Screen, _XCMap );
}

void SKL_X11_DGA_WIN::Print_Infos() const {
  SKL_X11_RAW_WIN::Print_Infos();
  Get_DGA_Infos()->Print_Infos();
}

//////////////////////////////////////////////////////////

#endif  /* SKL_USE_X11 && SKL_USE_DGA */
#endif  /* SKL_NO_VIDEO */
