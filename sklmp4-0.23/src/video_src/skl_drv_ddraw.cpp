/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_drv_ddraw.cpp
 *
 * DDraw video driver
 ********************************************************/

#include "skl_video.h"

#if !defined( SKL_NO_VIDEO )
#if defined(_WINDOWS)

#include "skl_syst/skl_dyn_load.h"
#include "skl_syst/skl_exception.h"
#include "skl_syst/skl_syswin.h"
#include "./skl_drv_ddraw.h"

//////////////////////////////////////////////////////////
// SKL_DDRAW_VIDEO_I
//////////////////////////////////////////////////////////

SKL_DDRAW_VIDEO_I::SKL_DDRAW_VIDEO_I(SKL_MEM_I *Mem, SKL_CST_STRING Name)
  : SKL_VIDEO_I(Mem, Name)
  , _Main_Display(this)
{
  _DD = 0;
  _DDSPrimary = 0;
  _DDSBack = 0;
  _DDPalette = 0;

  if (!Ping())
    Skl_Throw( SKL_MSG_EXCEPTION("DDraw: Ping failed") );
  _Caps = (CAPS)( HAS_BACKBUFFER );
// DEBUG
  _Caps = (CAPS)(_Caps|HAS_FULLSCREEN);
}

SKL_DDRAW_VIDEO_I::~SKL_DDRAW_VIDEO_I() {
  Cleanup();
}

//////////////////////////////////////////////////////////

static HRESULT CALLBACK ModeCount_Fnc( LPDDSURFACEDESC Desc,
                                       LPVOID lParam )
{
  SKL_DDRAW_VIDEO_I *Drv = (SKL_DDRAW_VIDEO_I*)lParam;
  SKL_DDRAW_WIN *Mode = new (Drv->Get_Mem()) SKL_DDRAW_WIN(Drv, Desc);
  Drv->Add_Mode( Mode );

  return DDENUMRET_OK;
}

int SKL_DDRAW_VIDEO_I::Scan_Modes()
{
  Clear_Modes();

  int Bpp = SKL_WIN_SYSTEM::Init_Main_Display(_Main_Display);
  _Main_Display.Set_Bpp( Bpp );

  _DD->EnumDisplayModes( 0, 0, (LPVOID)this, ModeCount_Fnc );

  return Get_Nb_Modes();
}


//////////////////////////////////////////////////////////

int SKL_DDRAW_VIDEO_I::Ping()
{
  if (Ok()) return 1;
 
  if (_DD==0)
    if ( DirectDrawCreate( 0, &_DD, 0 )!=DD_OK ) 
      return 0;

  Clear_Modes();
  if (!Scan_Modes()) Cleanup();
  else Set_Ok();
  return Ok();
}

void SKL_DDRAW_VIDEO_I::Cleanup()
{
  Clear_Modes();
  if ( _DD!=0 )
  {
    if ( _DDSPrimary!=0 ) _DDSPrimary->Release( );
    if ( _DDPalette!=0 ) _DDPalette->Release( );
    _DD->Release();

    _DD = 0;
    _DDSPrimary = 0;
    _DDPalette = 0;
    _DDSBack = 0;
  }
  Set_Not_Ok();
}

//////////////////////////////////////////////////////////

int SKL_DDRAW_VIDEO_I::Create_Palette()
{
  if ( _DDPalette==0 )
  {
    _DD->CreatePalette( DDPCAPS_8BIT | DDPCAPS_ALLOW256,
                        0, &_DDPalette, 0 );
    if ( _DDPalette==0 )
      return 0;
    _DDSPrimary->SetPalette( _DDPalette );
  }
  return 1;
}

void SKL_DDRAW_VIDEO_I::Make_Primary_Desc(DDSURFACEDESC *Desc, SKL_DDRAW_WIN *Win)
{
  SKL_BZERO( Desc, sizeof( DDSURFACEDESC ) );
  Desc->dwSize = sizeof( DDSURFACEDESC );
  Desc->dwFlags = DDSD_CAPS;
  Desc->ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
  if (Win!=0) {
    Desc->dwFlags |= DDSD_PIXELFORMAT;
    Desc->ddpfPixelFormat.dwSize = sizeof(_DDPIXELFORMAT);
    Desc->ddpfPixelFormat.dwFlags = DDPF_RGB;
    Desc->ddpfPixelFormat.dwRGBBitCount = Win->Get_Bpp();
  }
}

int SKL_DDRAW_VIDEO_I::Translate_Format(const DDPIXELFORMAT &pf,
                                        SKL_FORMAT &Fmt)
{
  int BitsPP = pf.dwRGBBitCount;
  int BytesPP;
  switch( BitsPP )
  {
    case 15: BytesPP = 2; break;
    case 16: BytesPP = 2; break;
    case 24: BytesPP = 3; break;
    case 32: BytesPP = 4; break;
    default: 
      BytesPP = 1;
      SKL_ASSERT( (pf.dwFlags & DDPF_PALETTEINDEXED8)!=0 );
    break;  // 8-bits
  }
  SKL_ARGB R_Mask = pf.dwRBitMask;
  SKL_ARGB G_Mask = pf.dwGBitMask;
  SKL_ARGB B_Mask = pf.dwBBitMask;

  Fmt = SKL_FORMAT(R_Mask, G_Mask, B_Mask, BytesPP);
  return BitsPP;
}

const DDSURFACEDESC *
SKL_DDRAW_VIDEO_I::Lock_Primary(SKL_DDRAW_WIN *Win)
{
  SKL_ASSERT( Ok() && _DDSPrimary!=0 );
  Make_Primary_Desc(&_Desc, Win);
  if (_DDSPrimary->Lock( 0, &_Desc,
                         DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY, 0 ) != DD_OK )
    return 0;
  return &_Desc;
}

void SKL_DDRAW_VIDEO_I::Unlock_Primary(SKL_DDRAW_WIN *Win)
{
  _DDSPrimary->Unlock( &_Desc );
}

int SKL_DDRAW_VIDEO_I::Create_Surface(SKL_DDRAW_WIN *Win)
{
  DDSURFACEDESC Desc;
  Make_Primary_Desc(&Desc, 0);
#if 0
  Desc.dwFlags |= DDSD_WIDTH | DDSD_HEIGHT;
  Desc.dwWidth = Win->Width();
  Desc.dwHeight = Win->Height();
#endif

/*
  if (With_Backbuffer) {
    Desc.dwFlags |= DDSD_BACKBUFFERCOUNT;
    Desc.ddsCaps.dwCaps |= | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
    Desc.dwBackBufferCount = 1;
  }                        
*/                        

  if ( _DD->CreateSurface(&Desc, &_DDSPrimary, 0 ) != DD_OK )
    return 0;

  //_DD->GetDisplayMode(&Desc);
  //Win->Init( &Desc );

  DDPIXELFORMAT pf;
  SKL_BZERO(&pf, sizeof(pf));
  pf.dwSize = sizeof(pf);
  if (_DDSPrimary->GetPixelFormat(&pf)!= DD_OK)
    return 0;

  if (Win->Format()==0x10000)
    if (!Create_Palette())
      return 0;

/*
  if (With_Backbuffer) {
    DDSCAPS ddscaps;
    ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
    if (_DDSPrimary->GetAttachedSurface(&ddscaps, &_DDSBack)!=DD_OK) {
      return 0;
    }
  }
*/
  return 1;
}

int SKL_DDRAW_VIDEO_I::Do_Set_Fullscreen(HWND Win, int Do_It)
{
  return (_DD->SetCooperativeLevel( Win, 
                 Do_It ? DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT
                       : DDSCL_NORMAL)
           == DD_OK);
}

int SKL_DDRAW_VIDEO_I::Set_Display(const SKL_DDRAW_WIN *Mode)
{
  HRESULT Ok = _DD->SetDisplayMode( Mode->Width(), Mode->Height(), 
                                    Mode->Get_Bpp() );
  return (Ok==DD_OK);
}

//////////////////////////////////////////////////////////
// Params
//////////////////////////////////////////////////////////

int SKL_DDRAW_VIDEO_I::Set_Param(PARAM Opt, 
                                 SKL_CST_STRING SValue,
                                 int IValue)
{
  switch(Opt) {
    case HIDE_FULLSCREEN: return 0;
    case SHOW_FULLSCREEN: return 0;
    default: return 0;
  }
  return 1;
}

int SKL_DDRAW_VIDEO_I::Get_Param(PARAM Opt, 
                                 SKL_CST_STRING *SValue,
                                 int *IValue) const
{
  switch(Opt) {
    case HIDE_FULLSCREEN: return 0;
    case SHOW_FULLSCREEN: return 0;
    default: break;
  }
  return 0;
}

//////////////////////////////////////////////////////////
// Fullscreen modes API
//////////////////////////////////////////////////////////

SKL_WINDOW *SKL_DDRAW_VIDEO_I::Set_Fullscreen_Mode(const SKL_BTM *FMode,
                                                   int Show)
{
  const SKL_DDRAW_WIN *Mode = (const SKL_DDRAW_WIN *)FMode;
  SKL_DDRAW_WIN *Win = new (Get_Mem()) SKL_DDRAW_WIN( this, 
                                                     *Mode,
                                                      Get_Name() );
  
  if (!Do_Set_Fullscreen( Win->Get_Win(), 1 )) goto Failed;
  if (!Set_Display( Mode )) goto Failed;
  if (!Create_Surface( Win )) goto Failed;

  Win->Adjust_Window( Mode->Width(), Mode->Height(), Get_Name() );
  if (Show) Win->Show();

  return Win;

Failed:
  delete Win;
  return 0;
}

void SKL_DDRAW_VIDEO_I::Shutdown_Fullscreen()
{
  // SKL_DDRAW_WIN *Mode = (SKL_DDRAW_WIN *)Get_Fullscreen_Mode();
  if (_DD)
    _DD->RestoreDisplayMode();
}

//////////////////////////////////////////////////////////
// Window API
//////////////////////////////////////////////////////////

SKL_WINDOW *SKL_DDRAW_VIDEO_I::Open_Window(int W, int H, SKL_FORMAT Fmt,
                                           int With_Backbuffer, 
                                           int Show)
{
  const SKL_DDRAW_WIN *Mode = (const SKL_DDRAW_WIN *)Get_Fullscreen_Mode();
  if (!Is_Fullscreen()) Mode = &_Main_Display;
 
  SKL_DDRAW_WIN *Win = new (Get_Mem()) SKL_DDRAW_WIN( this, 
                                                     *Mode, Get_Name(),
                                                      W, H );
  if (!Is_Fullscreen())
    if (!Do_Set_Fullscreen( Win->Get_Win(), 0 ))
      goto Failed;
  if (!Create_Surface( Win ))
    goto Failed;
  if (Show) Win->Show();
  return Win;

Failed:
  delete Win;
  return 0;
}

int SKL_DDRAW_VIDEO_I::Needs_Conversion(const SKL_WINDOW *Win,
                                        SKL_FORMAT Fmt) const
{
  SKL_ASSERT(Win!=0);
  // return !Win->Format().Is_Compatible_With(Fmt);
  return 1;
}

//////////////////////////////////////////////////////////
// -- external symbol exported
//////////////////////////////////////////////////////////

#include "skl_syst/skl_dyn_load.h"
SKL_DYN_FACTORY(SKL_DDRAW_VIDEO_I) {
  return (SKL_ANY)::new SKL_DDRAW_VIDEO_I(SKL_MEM, "Drv DDraw");
}

//////////////////////////////////////////////////////////
// SKL_DDRAW_WIN
//////////////////////////////////////////////////////////

void SKL_DDRAW_WIN::Init( LPDDSURFACEDESC Desc )
{
  SKL_FORMAT Fmt;
  int BitsPP = SKL_DDRAW_VIDEO_I::Translate_Format(Desc->ddpfPixelFormat,
                                                   Fmt);
  int Width  = Desc->dwWidth;
  int Height = Desc->dwHeight; 
  int BpS    = Desc->lPitch;

  Set_Virtual( Width, Height, Fmt, 0, BpS );
  Set_Bpp( BitsPP );
  _Locked = 0;
}

SKL_DDRAW_WIN::SKL_DDRAW_WIN(SKL_DDRAW_VIDEO_I *Drv, LPDDSURFACEDESC Desc)
  : SKL_WINDOW(Drv->Get_Mem())
  , _Use_Fullscreen(0)
  , _Drv(Drv)
  , _Win(0)
{
  if (Desc) Init(Desc);
}

SKL_DDRAW_WIN::SKL_DDRAW_WIN(SKL_DDRAW_VIDEO_I *Drv,
                             const SKL_DDRAW_WIN &Mode,
                             SKL_CST_STRING Name, int W, int H)
  : SKL_WINDOW(Drv->Get_Mem())
  , _Use_Fullscreen(0)
  , _Bpp(Mode._Bpp)
  , _Drv(Drv)
  , _Win(0)
{
  if (W==0 && H==0) // fullscreen Mode 
  {
    W = Mode.Width();
    H = Mode.Height();
    _Use_Fullscreen = 1;
  }
  Set_Virtual( W, H, Mode.Format(), 0, Mode.BpS() );
  _Locked = 0;
  Set_Name(Name);

  _Win = new (Get_Mem()) SKL_WIN_SYSTEM;
  _Win->Init_Window();
  _Win->Adjust_Window( W, H, Get_Name() );
}

SKL_DDRAW_WIN::~SKL_DDRAW_WIN()
{
  if (_Win==0) return;
  _Win->Destroy_Window();
  delete _Win;
  _Win = 0;
}

void SKL_DDRAW_WIN::Adjust_Window(int W, int H, 
                                  SKL_CST_STRING Name) const
{
  if (_Win) _Win->Adjust_Window(W, H, Name);
}

//////////////////////////////////////////////////////////

void SKL_DDRAW_WIN::Set_Position(int Xo, int Yo)
{
  if (_Win!=0) _Win->Set_Position(Xo, Yo);
  SKL_WINDOW::Set_Position(Xo, Yo);
}

void SKL_DDRAW_WIN::Set_Name(SKL_CST_STRING Name)
{
  if (_Win!=0) _Win->Set_Name(Name);
  SKL_WINDOW::Set_Name(Name);
}

void SKL_DDRAW_WIN::Hide() {
  if (_Win) _Win->Hide_Window();
  SKL_WINDOW::Hide();
}
void SKL_DDRAW_WIN::Show() {
  if (_Win) _Win->Show_Window();
  SKL_WINDOW::Show();
}

void SKL_DDRAW_WIN::Print_Infos() const {
  SKL_WINDOW::Print_Infos();
  printf( "Bpp=%d _Win=0x%x\n", _Bpp, _Win);
  if (_Win!=0) _Win->Print_Infos();
}

//////////////////////////////////////////////////////////
// system calls
//////////////////////////////////////////////////////////

SKL_BYTE *SKL_DDRAW_WIN::Lock() 
{
  if (_Drv==0 || _Drv->_DD==0) return 0;
  if (_Locked) return _Locked;  // ==return _Ptr

  const DDSURFACEDESC *Desc = _Drv->Lock_Primary(this);
  // printf("Desc=0x%x\n", Desc);
  if (Desc==0) return 0; // Lock() failed
 
  int BpS = Desc->lPitch;
  int Offset;
  if ( !_Use_Fullscreen )
  {
    POINT pt;
    pt.x = 0; pt.y = 0;
    ClientToScreen(_Win->Get_Win(), &pt);

    RECT Rect;
    GetClientRect(_Win->Get_Win(), &Rect);
    OffsetRect(&Rect, pt.x, pt.y);

#if 0
    int i = Rect.right - Rect.left;  
    Offset  = ( Rect.left + (i-Width())/2 )*Quantum();
    i = Rect.bottom - Rect.top;
    Offset += ( Rect.top + (i-Height())/2 )*BpS;
#else
    Offset = Rect.left*Quantum() + Rect.top*BpS;
#endif

    // Offset += GetSystemMetrics(SM_CYCAPTION)*BpS;
  }
  else {
    Offset = _Xo*Quantum() + BpS*_Yo;
  }
  _Locked = (BYTE*)Desc->lpSurface + Offset;
  Set_Virtual( Width(), Height(), Format(), _Locked, BpS, Get_CMap_Ptr() );
  // printf("Offset=%d, BpS=%d\n", Offset, BpS);

  return _Locked;  // ==return _Ptr
}

void SKL_DDRAW_WIN::Real_Unlock(int Xo, int Yo, int W, int H)
{
  if (_Locked) {
    _Drv->Unlock_Primary( this );
    _Locked = 0;
  }
}

//////////////////////////////////////////////////////////
/*
int Create_Backbuffer()
{
  DDSURFACEDESC ddsd;
  SKL_BZERO( &ddsd, 0, sizeof(ddsd) );
  ddsd.dwSize = sizeof( ddsd );
  ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
  ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
  ddsd.dwWidth = 640;
  ddsd.dwHeight = 480;

         // create the backbuffer separately
  if (lpDD->CreateSurface( &ddsd, &_DDSBack, NULL )!=DD_OK)
    return 0;
  return 1;
}

bool MyFlip()
{
  HRESULT ddrval;

    // if we're windowed do the blit, else just Flip
  if (IsWindowed)
  {
    // first we need to figure out where on the primary surface our window lives
    RECT Src, Dst;
    POINT p;
    p.x = 0; p.y = 0;
    ClientToScreen(ddWnd, &p);
    GetClientRect(ddWnd, &Dst);
    OffsetRect(&Dst, p.x, p.y);
    SetRect(&Src, 0, 0, 640, 480);
    ddrval = lpDDSPrimary->Blt( &Dst, _DDSBack, &Src, DDBLT_WAIT, NULL);
  } else {
    ddrval = lpDDSPrimary->Flip( NULL, DDFLIP_WAIT);
  }

  return (ddrval == DD_OK);
}
*/

//////////////////////////////////////////////////////////

void SKL_DDRAW_WIN::Store_CMap()
{
  if (!Has_CMap()) return;
  const SKL_COLOR *Cols = Get_CMap().Get_Colors();
  const int N = Get_CMap().Get_Nb_Colors();
  PALETTEENTRY Pal[256];
  for(int i=0; i<N; ++i)
  {
    Pal[i].peRed   = Cols[i].R();
    Pal[i].peGreen = Cols[i].G();
    Pal[i].peBlue  = Cols[i].B();
    Pal[i].peFlags = PC_RESERVED|PC_NOCOLLAPSE;  // ?!?
  }
  _Drv->_DDPalette->SetEntries( 0, 0, N, Pal );
}

//////////////////////////////////////////////////////////

void SKL_DDRAW_WIN::Get_Event(SKL_EVENT &New)
{
  if (_Win->Has_Key())
    New.Set_Key(_Win->Get_Key());
}

//////////////////////////////////////////////////////////

#endif  /* _WINDOWS */
#endif  /* SKL_NO_VIDEO */
