/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_drv_x11.h
 *
 * X11 video drivers
 ********************************************************/

#ifndef SKL_NO_VIDEO
#ifdef SKL_USE_X11

#ifndef _SKL_DRV_X11_H_
#define _SKL_DRV_X11_H_

//////////////////////////////////////////////////////////

#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#if defined(SKL_USE_SHM)

#include <X11/extensions/XShm.h>
#include "skl_syst/skl_destroy.h"

extern "C" {
int XShmQueryExtension(Display*);
int XShmGetEventBase(Display*);
}

#endif  /* SKL_USE_SHM */

class SKL_EVENT;
class SKL_X11_WIN;
class SKL_X11_VIDEO_I;

//////////////////////////////////////////////////////////
// SKL_X11_WIN (contains minimal infos)
//////////////////////////////////////////////////////////

class SKL_X11_WIN : public SKL_WINDOW
{
  protected:

    SKL_X11_VIDEO_I *_Drv;
    int              _Depth;
    Visual          *_Visual;

    virtual void Real_Unlock(int Xo, int Yo, int W, int H) {}

  public:

    SKL_X11_WIN(SKL_X11_VIDEO_I *Drv);
    virtual ~SKL_X11_WIN();
    void Init(XVisualInfo *Info);
    int Init(SKL_FORMAT Dsp_Format);

    int Get_Depth() const         { return _Depth; }
    Visual *Get_Visual() const    { return _Visual; }
    SKL_X11_VIDEO_I *Drv() const  { SKL_ASSERT(_Drv!=0); return _Drv; }
    inline int Get_Screen() const;
    inline Display *Get_XDisplay() const;

    virtual void Unlock() { Real_Unlock(0, 0, Width(), Height()); }
    virtual void Print_Infos() const;
};

//////////////////////////////////////////////////////////
// SKL_X11_RAW_WIN: visible base class for X11-mapped windows
//////////////////////////////////////////////////////////

class SKL_X11_RAW_WIN : public SKL_X11_WIN
{
  protected:

    GC              _GC;
    XImage         *_XImg;
    Window          _Win;
    unsigned long  *_Pixels;
    Colormap        _XCMap;
    Colormap        _Root_CMap;
    int             _Cells;

    void Cleanup();

    Cursor _Void_Cursor;
    int Hide_Cursor();

    virtual int Create_XImage();
    virtual int Create_XWindow();
    int Create_XCMap();
    void Set_Resize_Mode(int Resize);

    virtual void Hide();
    virtual void Show();

    virtual void Real_Unlock(int Xo, int Yo, int W, int H);
    void Translate_X11_Event(XEvent *Event, SKL_EVENT &New);

  public:

    SKL_X11_RAW_WIN(const SKL_X11_WIN *w, int W=0, int H=0);
    virtual ~SKL_X11_RAW_WIN();
    virtual void Store_CMap();
    virtual int Create_Window(int Show=1);

    virtual void Set_Position(int Xo, int Yo);

    virtual void Get_Event(SKL_EVENT &Event);
    virtual void Set_Name(SKL_CST_STRING Name);
};

//////////////////////////////////////////////////////////
// SKL_X11_SHM_WIN
//////////////////////////////////////////////////////////

#if defined(SKL_USE_SHM)

class SKL_X11_SHM_WIN : public SKL_X11_RAW_WIN
{
  private:

    XShmSegmentInfo _Shm_Info;
    int             _Completion_Type;

  protected:

    virtual int Create_XImage();
    void Clear_Shm();
    virtual void Real_Unlock(int Xo, int Yo, int W, int H);

  public:

    SKL_X11_SHM_WIN(const SKL_X11_WIN *w, int W=0, int H=0);
    virtual ~SKL_X11_SHM_WIN();
};

#endif  /* SKL_USE_SHM */


//////////////////////////////////////////////////////////
// SKL_DGA_INFO
//////////////////////////////////////////////////////////

#if defined(SKL_USE_DGA)

struct SKL_DGA_INFO
{
  int _Maj, _Min;
  int _Flags;
  int _First_Bank, _Bank_Nb, _Cur_Bank, _Line_Width;
  int _Left_Over;
  int _Bank_Size, _Mem_Size;
  int _Error_Base;
  int _Completion_Type;
  int _View_W, _View_H;
  SKL_ANY _Base_Ptr;

  int Init(Display *XDsp, int screen);
  void Get_Viewport_Size(Display *XDsp, int screen);
  void Print_Infos() const;
};

#endif  /* SKL_USE_DGA */

//////////////////////////////////////////////////////////
// SKL_X11_VIDEO_I
//////////////////////////////////////////////////////////

class SKL_X11_VIDEO_I : public SKL_VIDEO_I
{
  private:

    int             _Screen;
    Display        *_XDisplay;
    SKL_CST_STRING  _Display_Name;
    int             _Use_Shm, _Use_DGA;
    int             _Has_Shm, _Has_DGA;
    int             _VSync;

    int Init_Display(SKL_CST_STRING Display_Name);

#if defined(SKL_USE_DGA)
    SKL_DGA_INFO _DGA_Infos;
#endif
    void Cleanup();
    int Scan_Modes(SKL_FORMAT fmt=0x0);

  protected:

    virtual SKL_WINDOW *Set_Fullscreen_Mode(const SKL_BTM *FMode,
                                            int Show=1);
    virtual void Shutdown_Fullscreen();

    virtual SKL_WINDOW *Open_Window(int W, int H, SKL_FORMAT Fmt,
                                    int With_Backbuffer=0,
                                    int Show=1);

    virtual int Set_Param(PARAM opt, SKL_CST_STRING SValue, int IValue);
    virtual int Get_Param(PARAM opt, SKL_CST_STRING *, int *) const;
    virtual int Needs_Conversion(const SKL_WINDOW *Win,
                                 SKL_FORMAT Fmt) const;

  public:

    SKL_X11_VIDEO_I(SKL_MEM_I* Mem, SKL_CST_STRING Name=0, SKL_CST_STRING Dsp_Name=0);
    virtual ~SKL_X11_VIDEO_I();

    virtual int Ping();

    Display *Get_XDisplay() const { return _XDisplay; }
    int Get_Screen() const        { return _Screen; }
    SKL_FORMAT Get_Visual_Format(XVisualInfo *Info) const;
#if defined(SKL_USE_DGA)
    const SKL_DGA_INFO *Get_DGA_Infos() const { return &_DGA_Infos; }
#endif

    int VSync_Is_On() const  { return _VSync; }
};

//////////////////////////////////////////////////////////
// inlined methods

inline int SKL_X11_WIN::Get_Screen() const        { return Drv()->Get_Screen(); }
inline Display *SKL_X11_WIN::Get_XDisplay() const { return Drv()->Get_XDisplay(); }

//////////////////////////////////////////////////////////

#endif  /* _SKL_DRV_X11_H_ */
#endif  /* SKL_USE_X11 */
#endif  /* SKL_NO_VIDEO */
