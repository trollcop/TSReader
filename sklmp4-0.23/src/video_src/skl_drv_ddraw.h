/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_drv_ddraw.h
 *
 * Direct Draw video drivers
 ********************************************************/

#ifndef SKL_NO_VIDEO
#if defined(_WINDOWS)

#ifndef _SKL_DRV_DDRAW_H_
#define _SKL_DRV_DDRAW_H_

//////////////////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN
#include <ddraw.h>

class SKL_DDRAW_VIDEO_I;

//////////////////////////////////////////////////////////
// SKL_DDRAW_WIN
//////////////////////////////////////////////////////////

class SKL_DDRAW_WIN : public SKL_WINDOW
{
    SKL_DEL_OP(SKL_DDRAW_WIN);

  protected:

    SKL_BYTE *_Locked;
    int       _Use_Fullscreen;
    int       _Bpp;
    SKL_DDRAW_VIDEO_I *_Drv;
    SKL_WIN_SYSTEM    *_Win;

    virtual void Real_Unlock(int Xo, int Yo, int W, int H);

    void Init( LPDDSURFACEDESC Desc );

  public:

    SKL_DDRAW_WIN( SKL_DDRAW_VIDEO_I *Drv=0, LPDDSURFACEDESC Desc=0 );
    SKL_DDRAW_WIN(SKL_DDRAW_VIDEO_I *Drv,
                  const SKL_DDRAW_WIN &Mode,                  
                  SKL_CST_STRING Name=0, int W=0, int H=0);
    virtual ~SKL_DDRAW_WIN();

    virtual void Hide();
    virtual void Show();
    virtual SKL_BYTE *Lock();
    virtual void Unlock() { Real_Unlock(0,0, Width(),Height()); }
    virtual void Get_Event(class SKL_EVENT &Event);

    virtual void Store_CMap();

    virtual void Set_Position(int Xo, int Yo);
    virtual void Set_Name(SKL_CST_STRING Name);

    virtual void Print_Infos() const;

    HWND Get_Win() const { return _Win->Get_Win(); }
    void Adjust_Window(int W, int H, SKL_CST_STRING Name) const;

    int Get_Bpp() const   { return _Bpp; }
    void Set_Bpp(int Bpp) { _Bpp = Bpp; }
};

//////////////////////////////////////////////////////////
// SKL_DDRAW_VIDEO_I
//////////////////////////////////////////////////////////

class SKL_DDRAW_VIDEO_I : public SKL_VIDEO_I
{
    SKL_DEL_OP(SKL_DDRAW_VIDEO_I)

  private:

    friend SKL_DDRAW_WIN;  // beuarkk

    IDirectDraw        *_DD;
    IDirectDrawSurface *_DDSPrimary;
    IDirectDrawSurface *_DDSBack;
    IDirectDrawPalette *_DDPalette;
    DDSURFACEDESC       _Desc;
    SKL_DDRAW_WIN       _Main_Display;
    unsigned            _RedMask, _GreenMask, _BlueMask, _AlphaMask;

    void Cleanup();
    int Scan_Modes();

      // it's time someone teach MS OO-programming :(
    friend static HRESULT CALLBACK ModeCount_Fnc(LPDDSURFACEDESC Desc, LPVOID lParam);

    static int Translate_Format(const DDPIXELFORMAT &pf, SKL_FORMAT &Fmt);

    int Create_Surface(SKL_DDRAW_WIN *Win);
    int Create_Palette();
    int Do_Set_Fullscreen(HWND Win, int Do_It=1);
    int Set_Display(const SKL_DDRAW_WIN *Mode);
    void Make_Primary_Desc(DDSURFACEDESC *Desc, SKL_DDRAW_WIN *Win);
    const DDSURFACEDESC *Lock_Primary(SKL_DDRAW_WIN *Win);
    void Unlock_Primary(SKL_DDRAW_WIN *Win);

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

    SKL_DDRAW_VIDEO_I(SKL_MEM_I *Mem, SKL_CST_STRING Name);
    virtual ~SKL_DDRAW_VIDEO_I();

    virtual int Ping();
};

//////////////////////////////////////////////////////////

#endif  /* _SKL_DRV_DDRAW_H_ */
#endif  /* SKL_USE_X11 */
#endif  /* SKL_NO_VIDEO */
