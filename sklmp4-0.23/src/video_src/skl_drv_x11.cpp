/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_drv_x11.cpp
 *
 * X11 (/SHM) video main driver
 ********************************************************/

#include "skl_video.h"

#ifndef SKL_NO_VIDEO
#ifdef SKL_USE_X11

#include "skl_syst/skl_dyn_load.h"
#include "./skl_drv_x11.h"

#if defined(__IRIX__)
// #define COMPLETION_BUG
#endif

#if defined(__SUN__) || defined(__IRIX__)
#define DONT_DETACH_FIRST
#endif

//////////////////////////////////////////////////////////
// SKL_X11_VIDEO_I
//////////////////////////////////////////////////////////

SKL_X11_VIDEO_I::SKL_X11_VIDEO_I(SKL_MEM_I *Mem,
                                 SKL_CST_STRING Name,
                                 SKL_CST_STRING Dsp_Name)
  : SKL_VIDEO_I(Mem, Name)
  , _Screen(-1)
  , _XDisplay(0)
  , _Display_Name(Dsp_Name)
  , _Use_Shm(0), _Use_DGA(0)
  , _Has_Shm(0), _Has_DGA(0)
  , _VSync(1)
{ 
  _Caps = (CAPS)( HAS_BACKBUFFER );

#if defined(SKL_USE_SHM)
  Set_Param(USE_SHM, 0, 1);
#endif

#if defined(SKL_USE_DGA)
  Set_Param(USE_DGA, 0, 1);
  _Caps = (CAPS)(_Caps|HAS_FULLSCREEN);
#endif

  Ping();
}

SKL_X11_VIDEO_I::~SKL_X11_VIDEO_I()
{
  Cleanup();
}

void SKL_X11_VIDEO_I::Cleanup()
{
  Clear_Modes();
  if (_XDisplay!=0) XCloseDisplay(_XDisplay);  
  _Screen = -1;
  _XDisplay = 0;
  Set_Not_Ok();
}

//////////////////////////////////////////////////////////

SKL_FORMAT SKL_X11_VIDEO_I::Get_Visual_Format(XVisualInfo *Info) const
{
  int Dsp_Depth;
  SKL_FORMAT Fmt;
  Visual *v = Info->visual;

    // Ackward method to get the exact Pixel_Depth of display

  XImage *XImg = XCreateImage( _XDisplay, v, Info->depth, 
                               ZPixmap, 0, 0,
                               16, 16,
                               BitmapPad(_XDisplay), 0 );
  if (XImg!=0) {
    Dsp_Depth = XImg->bytes_per_line / 16;
    XDestroyImage( XImg );
  }
  else {
    Dsp_Depth = (Info->depth+7)/8; // geee!!! finger crossing
  }

  if (v->c_class==PseudoColor) 
    Fmt = 0x00000 | (Dsp_Depth<<16);  // cmapped format
  else
    Fmt = SKL_FORMAT(v->red_mask, v->green_mask, v->blue_mask, 0, Dsp_Depth);
  return Fmt;
}

int SKL_X11_VIDEO_I::Scan_Modes(SKL_FORMAT Fmt)
{
  int Nb_Modes;
  XVisualInfo Template;
  Template.screen = DefaultScreen(_XDisplay); // _Screen;
  Template.c_class = TrueColor;

  XVisualInfo *Info = XGetVisualInfo( _XDisplay,      
                                      VisualClassMask | VisualScreenMask,
                                      &Template, &Nb_Modes );
  if (Info==0) {
    fprintf( stderr, "Can't get X visual\n" );
    return 0;
  }

  int Depth = -1;
  if (Fmt.Raw_Depth()!=0) Depth = 8 * Fmt.Raw_Depth();
  int Nb=0;

  for( int i=0; i<Nb_Modes; i++)
  {
    // Get rid of Visual with depth<8 and skip bad visuals
    if (Depth!=-1 && Info[i].depth!=Depth) continue;
    else if (Info[i].depth<8) continue;
    else if ( Info[i].c_class==StaticColor ||
              Info[i].c_class==StaticGray ||
              Info[i].c_class==GrayScale )
      continue;  
    SKL_X11_WIN *Mode = new (Get_Mem()) SKL_X11_WIN(this);
    Mode->Init( &Info[i] );
    if (Fmt.Bits()==0x0 || Mode->Format().Bits()==Fmt.Bits()) {
      Add_Mode( Mode );
      Nb++;
    }
    else delete Mode;
  }
  XFree( Info );

  return Nb;
}

int SKL_X11_VIDEO_I::Ping()
{
  Cleanup();
  if (_Display_Name==0) _Display_Name = XDisplayName(0);
  _XDisplay = XOpenDisplay(_Display_Name);
  if (_XDisplay==0) return 0;
  _Screen = DefaultScreen(_XDisplay); // TODO: should it be an option?

  _Has_Shm = 0;
#ifdef SKL_USE_SHM
  if ( (XShmQueryExtension(_XDisplay)!=0) && (_Display_Name[0]!=':'))
    fprintf( stderr, "Can't use SHM when display is redirected.\n" );
  else
    _Has_Shm = 1;
#endif

  _Has_DGA = 0;
#ifdef SKL_USE_DGA
  _Has_DGA = _DGA_Infos.Init(_XDisplay, _Screen);
#endif

  Clear_Modes();
  Scan_Modes();

  Set_Ok();
  return 1;
}

//////////////////////////////////////////////////////////
// Params
//////////////////////////////////////////////////////////

int SKL_X11_VIDEO_I::Set_Param(PARAM Opt, 
                               SKL_CST_STRING SValue,
                               int IValue)
{
  switch(Opt) {
    case USE_DISPLAY: _Display_Name = SValue; return Ping();
    case USE_SHM: _Use_Shm = (IValue!=0); break;
    case USE_DGA: _Use_DGA = (IValue!=0); break;
    case VSYNC:   _VSync = (IValue!=0);    break;
    default: return 0;
  }
  return 1;
}

int SKL_X11_VIDEO_I::Get_Param(PARAM Opt, 
                               SKL_CST_STRING *SValue,
                               int *IValue) const
{
  switch(Opt) {
    case USE_DISPLAY: *SValue = _Display_Name; return 1;
    case USE_SHM: *IValue = ((_Has_Shm&_Use_Shm)!=0); return 1;
    case USE_DGA: *IValue = ((_Has_DGA&_Use_DGA)!=0); return 1;
    case VSYNC:   *IValue = _VSync; return 1;
    default: break;
  }
  return 0;
}

//////////////////////////////////////////////////////////
// Fullscreen API
//////////////////////////////////////////////////////////

SKL_WINDOW *SKL_X11_VIDEO_I::Set_Fullscreen_Mode(const SKL_BTM *FMode,
                                                 int Show)
{
  const SKL_X11_WIN *Mode = (const SKL_X11_WIN *)FMode;

  SKL_X11_RAW_WIN *Win;
#ifdef SKL_USE_SHM
  if (_Has_Shm & _Use_Shm) {
    Win = new (Get_Mem()) SKL_X11_SHM_WIN( Mode );
  }
  else 
#endif
    Win = new (Get_Mem()) SKL_X11_RAW_WIN( Mode );

  if (!Win->Create_Window(Show)) {
    delete Win; // Argh!
    return 0;
  } 
  return Win;
}

void SKL_X11_VIDEO_I::Shutdown_Fullscreen() {
}

//////////////////////////////////////////////////////////
// Window API
//////////////////////////////////////////////////////////

SKL_WINDOW *SKL_X11_VIDEO_I::Open_Window(int W, int H, SKL_FORMAT Fmt,
                                         int With_Backbuffer,
                                         int Show)
{
  const SKL_X11_WIN *Mode = (const SKL_X11_WIN *)Get_Fullscreen_Mode();
  if (!Is_Fullscreen())
    Mode = (const SKL_X11_WIN *)Get_Mode_Infos(Search_Best_Mode(W, H, Fmt));

  SKL_X11_RAW_WIN *Win;
#ifdef SKL_USE_SHM
  if (_Has_Shm&_Use_Shm) {
    Win = new (Get_Mem()) SKL_X11_SHM_WIN( Mode, W, H );
  }
  else 
#endif
    Win = new (Get_Mem()) SKL_X11_RAW_WIN( Mode, W, H );

  if (!Win->Create_Window(Show))
  {
    delete Win;  // Argh!
    return 0;
  }
  return Win;
}

int SKL_X11_VIDEO_I::Needs_Conversion(const SKL_WINDOW *Win,
                                      SKL_FORMAT Fmt) const
{
  SKL_ASSERT(Win!=0);
  return !Win->Format().Is_Compatible_With(Fmt);
}

//////////////////////////////////////////////////////////
// -- external symbol exported
//////////////////////////////////////////////////////////

#include "skl_syst/skl_dyn_load.h"
SKL_DYN_FACTORY(SKL_X11_VIDEO_I) {
  return (SKL_ANY)::new SKL_X11_VIDEO_I(SKL_MEM, "Drv X11");
}

//////////////////////////////////////////////////////////
// SKL_X11_WIN
//////////////////////////////////////////////////////////

SKL_X11_WIN::SKL_X11_WIN(SKL_X11_VIDEO_I *Drv)
  : SKL_WINDOW(Drv->Get_Mem())
  , _Drv(Drv)
  , _Depth(0)
  , _Visual(0)
{}

SKL_X11_WIN::~SKL_X11_WIN() {
  SKL_ASSERT(_Drv!=0);
}

void SKL_X11_WIN::Init(XVisualInfo *Info)
{
  Display *Dsp = Get_XDisplay();
  int Screen = Get_Screen();

  _Depth = Info->depth;
  _Visual = Info->visual;

  SKL_FORMAT Fmt = _Drv->Get_Visual_Format(Info);
  int W = DisplayWidth( Dsp, Screen );
  int H = DisplayHeight( Dsp, Screen );
  Set_Virtual( W, H, Fmt );
}

void SKL_X11_WIN::Print_Infos() const
{
  SKL_BTM::Print_Infos();
  printf( "  [Depth=%d Q=%d Visual=0x%8p]", _Depth, Quantum(), (SKL_BYTE*)_Visual );
  if (_Visual)
    printf( " (id=%d c=%d cmapsize=%d b/rgb=%d)",
       (int)_Visual->visualid, 
       (int)_Visual->c_class, 
       (int)_Visual->map_entries, 
       (int)_Visual->bits_per_rgb );
  printf( "\n");
}

//////////////////////////////////////////////////////////
// -- Raw X11 calls. It's time for the Real Stuff :) -- //
//////////////////////////////////////////////////////////

#define X11_MASKS   StructureNotifyMask| ExposureMask |    \
 KeyReleaseMask | KeyPressMask  |       \
 ButtonPressMask | ButtonReleaseMask |  \
 ButtonMotionMask | PointerMotionMask

//////////////////////////////////////////////////////////
// SKL_X11_RAW_WIN
//////////////////////////////////////////////////////////

SKL_X11_RAW_WIN::SKL_X11_RAW_WIN(const SKL_X11_WIN *w, 
                                 int W, int H)
  : SKL_X11_WIN(w->Drv())
  , _XImg(0)
  , _Win(0)
  , _Pixels(0)
  , _XCMap(None)
  , _Root_CMap(None)
  , _Cells(0)
{
  _Depth = w->Get_Depth();
  _Visual = w->Get_Visual();
  if (W==0) W = w->Width();
  if (H==0) H = w->Height();
  int BpS = ( W*w->Quantum() + 7) & ~7;  // pad to 8
  Set_Virtual( W, H, w->Format(), 0, BpS, 0);

//  Display *Dsp = Get_XDisplay();
//  int Screen = Get_Screen();
//  _XCMap = (Colormap)DefaultColormap( Dsp, Screen );

}

SKL_X11_RAW_WIN::~SKL_X11_RAW_WIN()
{
  Hide();
  Cleanup();
}

void SKL_X11_RAW_WIN::Cleanup()
{
  Display *Dsp = Get_XDisplay();
  if ( _XImg!=0 ) {
    XDestroyImage( _XImg );
    _XImg = 0;
  }
  if (_Root_CMap!=None) {
    XFreeColormap( Dsp, _Root_CMap );
    _Root_CMap = None;
  }
  if ( _Win!=(Window)0 )
  {
    if (_Cells) XFreeColors( Dsp, _XCMap, _Pixels, 256, 0 );
    if (_XCMap!=None) XFreeColormap( Dsp, _XCMap );
    if (_Pixels) Get_Mem()->Delete( _Pixels, 256*sizeof(*_Pixels) );
    XFreeGC( Dsp, _GC );
    XDestroyWindow( Dsp, _Win );
    _Win = (Window)0;
    _XCMap = None;
    _Cells = 0;
    _Pixels = 0;
  }
}

void SKL_X11_RAW_WIN::Set_Name(SKL_CST_STRING Name) {
  SKL_WINDOW::Set_Name( Name );
  if (_Win!=0 && _Drv!=0)
    XStoreName( _Drv->Get_XDisplay(), _Win, Name );
}

//////////////////////////////////////////////////////////

int SKL_X11_RAW_WIN::Create_XCMap()
{
  SKL_ASSERT( _XCMap == (Colormap)None && _Pixels == 0 );
  Display *Dsp = Get_XDisplay();
  if ( Format().Depth()==1 )
  {   
    _Pixels = (unsigned long*)Get_Mem()->New( 256*sizeof(unsigned long) );
    _XCMap = XCreateColormap( Dsp, _Win, _Visual, AllocNone );
    if (_Visual->c_class==PseudoColor) {
      _Cells = XAllocColorCells( Dsp, _XCMap, 0, 0, 0, _Pixels, 256 );
      if ( _Cells==0 ) return 0;
    }
    XSetWindowColormap( Dsp, _Win, _XCMap );
    XInstallColormap( Dsp, _XCMap );
  }   
  else { _XCMap = (Colormap)None; _Pixels=0; }
  return 1;
}

void SKL_X11_RAW_WIN::Store_CMap()
{
  if (!Has_CMap()) return;
  XColor XColors[ 256 ];
  const SKL_COLOR *Cols = Get_CMap().Get_Colors();
  int N = Get_CMap().Get_Nb_Colors();
  for( int i=0; i<N; ++i )
  {
    XColors[i].red   = Cols[i].R() << 8;
    XColors[i].green = Cols[i].G() << 8;
    XColors[i].blue  = Cols[i].B() << 8;
    XColors[i].pixel = _Pixels[ i ];
    XColors[i].flags = DoRed | DoGreen | DoBlue;
  }
  Display *Dsp = Get_XDisplay();
  XStoreColors( Dsp, _XCMap, XColors, N );
}

//////////////////////////////////////////////////////////

void SKL_X11_RAW_WIN::Set_Position(int Xo, int Yo)
{
  SKL_WINDOW::Set_Position(Xo,Yo);  // updates _Xo and _Yo

  Display *Dsp = Get_XDisplay();
  int Screen = Get_Screen();

  XMoveWindow( Dsp, _Win, Xo, Yo );
  XWarpPointer(
    Dsp, 
    RootWindow( Dsp, Screen ),
    RootWindow( Dsp, Screen ),
    0, 0, 
    DisplayWidth( Dsp, Screen ), DisplayHeight( Dsp, Screen ),
    Xo, Yo );
}

void SKL_X11_RAW_WIN::Set_Resize_Mode(int Resize)
{
  Display *Dsp = Get_XDisplay();
  int Screen = Get_Screen();

  XSizeHints Size_Hints; 
  Size_Hints.width  = Width();
  Size_Hints.height = Height();
  if (!Resize) {
    Size_Hints.min_width = Size_Hints.width;
    Size_Hints.max_width = Size_Hints.width;
    Size_Hints.min_height = Size_Hints.height;
    Size_Hints.max_height = Size_Hints.height;
  }
  else {
    Size_Hints.min_width = 16;
    Size_Hints.max_width = DisplayWidth( Dsp, Screen );
    Size_Hints.min_height = 16;
    Size_Hints.max_height = DisplayHeight( Dsp, Screen );
  }
  Size_Hints.flags = PSize | PMinSize | PMaxSize;
  XSetWMNormalHints( Dsp, _Win, &Size_Hints );
}

//////////////////////////////////////////////////////////

int SKL_X11_RAW_WIN::Hide_Cursor()
{
  XColor Dummy;
  Dummy.flags = 0; 
  Dummy.pixel = 0;
  Dummy.red = Dummy.green = Dummy.blue = 0;

  Display *Dsp = Get_XDisplay();

  Pixmap Mouse_Void_Pixmap = XCreatePixmap( Dsp, _Win, 1, 1, 1 );
  Pixmap Data = XCreateBitmapFromData( Dsp, Mouse_Void_Pixmap,
                                      (char *)"\1", 1, 1 );
  if ( Data == None )
    return 0;       // X server could not even allocate a 1x1 Pixmap <gee> !!

  _Void_Cursor = XCreatePixmapCursor(Dsp, Mouse_Void_Pixmap, Mouse_Void_Pixmap,
                                     &Dummy, &Dummy,
                                     0, 0 );
  XDefineCursor( Dsp, _Win, _Void_Cursor );
  XFreePixmap( Dsp, Mouse_Void_Pixmap );
  return 1;
}

void SKL_X11_RAW_WIN::Hide() {
  Display *Dsp = Get_XDisplay();
  if ( _Win!=0 ) XUnmapWindow(Dsp, _Win);
  SKL_WINDOW::Hide();
}

void SKL_X11_RAW_WIN::Show() {
  Display *Dsp = Get_XDisplay();
  if ( _Win!=0 ) XMapWindow(Dsp, _Win);
  SKL_WINDOW::Show();
}

//////////////////////////////////////////////////////////

int SKL_X11_RAW_WIN::Create_XWindow()
{
  Display *Dsp = Get_XDisplay();
  if (_Win!=0) {
    XResizeWindow( Dsp, _Win, Width(), Height() );
  }
  else {

      // Border_pixel and dflt Colormap are MANDATORY (on SGI+Sun) :(
    SKL_ASSERT(_Root_CMap==None);
    _Root_CMap = XCreateColormap(Dsp, RootWindow(Dsp, Get_Screen()), _Visual, AllocNone);

    XSetWindowAttributes W_Attribs;
    W_Attribs.backing_store = NotUseful; // Always;
    W_Attribs.event_mask = X11_MASKS;
    W_Attribs.border_pixel = 0;
    W_Attribs.colormap = _Root_CMap;

    _Win = XCreateWindow(Dsp, RootWindow( Dsp, Get_Screen() ),
                         0, 0, Width(), Height(), 0,
                         _Depth, InputOutput, _Visual,
                         CWBackingStore | CWEventMask | CWBorderPixel | CWColormap,
                         &W_Attribs );  
    if ( _Win==0 ) return 0;
    XGCValues gcVals;
    _GC = XCreateGC( Dsp, _Win, 0, &gcVals );
//    _GC = DefaultGC( Dsp, Screen );
    XStoreName(Dsp, _Win, Get_Name());
//    XSelectInput( Dsp, _Win, X11_MASKS );
  }
  return 1;
}

int SKL_X11_RAW_WIN::Create_XImage()
{
  Display *Dsp = Get_XDisplay();
  _XImg = XCreateImage( Dsp, _Visual,
                        _Depth, ZPixmap, 0, 0,
                        Width(), Height(), BitmapPad( Dsp ), 0 );
  if ( _XImg==0 ) return 0;

  int bps = _XImg->bytes_per_line;
  _XImg->data = (char*)Get_Mem()->New( _XImg->height*bps * sizeof(char) );

  Set_Virtual(Width(), Height(), Format(), (SKL_BYTE*)_XImg->data, bps);

  return 1;
}

int SKL_X11_RAW_WIN::Create_Window(int show)
{
  if (!Create_XWindow()) return 0;
  if (!Create_XImage()) return 0;
  if (!Create_XCMap()) return 0;
  if (!Hide_Cursor()) return 0;  
  Set_Resize_Mode(0);
  if (show) Show();
  return 1;
}

void SKL_X11_RAW_WIN::Real_Unlock(int Xo, int Yo, int W, int H)
{
  Display *Dsp = Get_XDisplay();
  SKL_ASSERT(Xo>=0 && Yo>=0 && (W-Xo)<=Width() && (H-Yo)<=Height());
  XPutImage( Dsp, _Win, _GC, _XImg,
             Xo, Yo, Xo, Yo, W, H);
  XFlush(Dsp);
}

//////////////////////////////////////////////////////////

void SKL_X11_RAW_WIN::Translate_X11_Event(XEvent *Event, SKL_EVENT &New)
{
  KeySym  Touche;
  char    Buffer;

  int Type = Event->type;
  if ( Type == KeyPress )
  {
    if ( XLookupString( (XKeyPressedEvent*)Event,
                        &Buffer, 1, &Touche, 0 ) != 1 )
//    Touche = XLookupKeysym( (XKeyEvent*)&Event->xkey, 0 );
//    if ( Touche!=NoSymbol )
    {
      if ( Touche == XK_Shift_L || Touche == XK_Shift_R )
        New.Add_Modifier(SKL_EVENT::SHIFT_MODIFIER);
      if ( Touche == XK_Control_L || Touche == XK_Control_R )
        New.Add_Modifier(SKL_EVENT::CTRL_MODIFIER);
      if ( Touche == XK_Alt_L || Touche == XK_Alt_R )
        New.Add_Modifier(SKL_EVENT::ALT_MODIFIER);

      if ( Touche == XK_Left ) New.Add_Modifier(SKL_EVENT::LEFT_MODIFIER);
      if ( Touche == XK_Right ) New.Add_Modifier(SKL_EVENT::RIGHT_MODIFIER);
      if ( Touche == XK_Up ) New.Add_Modifier(SKL_EVENT::UP_MODIFIER);
      if ( Touche == XK_Down ) New.Add_Modifier(SKL_EVENT::DOWN_MODIFIER);
    }
  }
  else if ( Type == KeyRelease )
  {
    SKL_UINT32 Mod = Event->xkey.state;
    if ( Mod & ShiftMask ) New.Add_Modifier(SKL_EVENT::SHIFT_MODIFIER); 
    if ( Mod & ControlMask ) New.Add_Modifier(SKL_EVENT::CTRL_MODIFIER);
    if ( Mod & Mod1Mask ) New.Add_Modifier(SKL_EVENT::ALT_MODIFIER);

//    if ( XLookupString( ( XKeyReleasedEvent *)( Event ),
//      &Buffer, 1, &Touche, NULL ) != 1 )
    Touche = XLookupKeysym( (XKeyEvent*)&Event->xkey, 0 );
    if ( Touche==NoSymbol )
      return;
    if ((Touche>>8)==0xff)   // Special Key
    {
      if ( (Touche&0xff) == 0x1b )
        New.Set_Key(SKL_EVENT::ESCAPE);
      else if ( (Touche&0xff) == 0xbe )
        New.Set_Key(SKL_EVENT::F1);
      else if ( (Touche&0xff) == 0xbf )
        New.Set_Key(SKL_EVENT::F2);
      else if ( (Touche&0xff) == 0xc0 )
        New.Set_Key(SKL_EVENT::F3);
      else if ( (Touche&0xff) == 0xc1 )
        New.Set_Key(SKL_EVENT::F4);
      else if ( (Touche&0xff) == 0xc2 )
        New.Set_Key(SKL_EVENT::F5);
      else if ( (Touche&0xff) == 0xc3 )
        New.Set_Key(SKL_EVENT::F6);
      else if ( (Touche&0xff) == 0xc4 )
        New.Set_Key(SKL_EVENT::F7);
      else if ( (Touche&0xff) == 0xc5 )
        New.Set_Key(SKL_EVENT::F8);
      else if ( (Touche&0xff) == 0xc6 )
        New.Set_Key(SKL_EVENT::F9);
      else if ( (Touche&0xff) == 0xc7 )
        New.Set_Key(SKL_EVENT::F10);
      else if ( (Touche&0xff) == 0xc8 )
        New.Set_Key(SKL_EVENT::F11);
      else if ( (Touche&0xff) == 0xc9 )
        New.Set_Key(SKL_EVENT::F12);
//      else fprintf( stderr, "KeySym=0x%x\n", Touche );
    }
    else New.Set_Key( (int)Touche );
    New.Add_Type(SKL_EVENT::KEY_PRESS);
  }
  else if ( Type==MotionNotify )
  {
    XMotionEvent *MEvent = ( XMotionEvent *)&Event->xmotion;
    SKL_UINT32 But = Event->xbutton.state;
    New.Set_Position( MEvent->x, MEvent->y );
    New.Add_Type(SKL_EVENT::MOVE);

    if ( But & Button1Mask ) New.Add_Type(SKL_EVENT::CLICK1);
    else New.Remove_Type(SKL_EVENT::CLICK1);
    if ( But & Button2Mask ) New.Add_Type(SKL_EVENT::CLICK2);
    else New.Remove_Type(SKL_EVENT::CLICK2);
    if ( But & Button3Mask ) New.Add_Type(SKL_EVENT::CLICK3);
    else New.Remove_Type(SKL_EVENT::CLICK3);
  }
  else if ( Type == ButtonRelease )
  {
    XButtonEvent *BEvent = (XButtonEvent *)&Event->xbutton;
    New.Set_Position( BEvent->x, BEvent->y );
    SKL_UINT32 But = BEvent->state;
    if ( But & Button1Mask ) New.Add_Type(SKL_EVENT::RELEASE1);
    if ( But & Button2Mask ) New.Add_Type(SKL_EVENT::RELEASE2);
    if ( But & Button3Mask ) New.Add_Type(SKL_EVENT::RELEASE3);
  }
  else if ( Type == ButtonPress )
  {
    XButtonEvent *BEvent = (XButtonEvent *)&Event->xbutton;
    New.Set_Position( BEvent->x, BEvent->y );
    SKL_UINT32 But = BEvent->button;
    if ( But & Button1 ) New.Add_Type(SKL_EVENT::CLICK1);
    if ( But & Button2 ) New.Add_Type(SKL_EVENT::CLICK2);
    if ( But & Button3 ) New.Add_Type(SKL_EVENT::CLICK3);
  }
}

void SKL_X11_RAW_WIN::Get_Event(SKL_EVENT &New)
{
  Display *Dsp = Get_XDisplay();
  XEvent Event;
  while( XCheckWindowEvent( Dsp, _Win, X11_MASKS, &Event ) )
    Translate_X11_Event( &Event, New );
}

//////////////////////////////////////////////////////////
// SKL_X11_SHM_WIN
//////////////////////////////////////////////////////////

#if defined(SKL_USE_SHM)

SKL_X11_SHM_WIN::SKL_X11_SHM_WIN(const SKL_X11_WIN *w, int W, int H)
  : SKL_X11_RAW_WIN(w, W, H)
{
  _Shm_Info.shmid = -1;
  _Shm_Info.shmaddr = 0;
}

SKL_X11_SHM_WIN::~SKL_X11_SHM_WIN()
{
  Clear_Shm();
}

//////////////////////////////////////////////////////////

int SKL_X11_SHM_WIN::Create_XImage()
{
  Display *Dsp = Get_XDisplay();
  int bps;

  _XImg = XShmCreateImage( Dsp, _Visual,
                           _Depth, ZPixmap, NULL /* no alloc */,
                           &_Shm_Info,
                           Width(), Height() );
  if ( _XImg==0 ) goto Failed;
  if ( _XImg->xoffset!=0 ) goto Failed;

  bps = _XImg->bytes_per_line;

  _Shm_Info.shmid = shmget( IPC_PRIVATE, 
                            _XImg->height*bps,
                            IPC_CREAT | 0777);
  if ( _Shm_Info.shmid<0 ) goto Failed;
  _Shm_Info.shmaddr = (char*)shmat( _Shm_Info.shmid, 0, 0 );
  if ( _Shm_Info.shmaddr==(char*)-1 ) goto Failed;
  XSync(Dsp, False);
  _Shm_Info.readOnly = False;

  if ( !XShmAttach( Dsp, &_Shm_Info ) ) goto Failed;

#ifndef COMPLETION_BUG
  _Completion_Type = XShmGetEventBase( Dsp ) + ShmCompletion;
#endif

#ifndef DONT_DETACH_FIRST
  shmctl( _Shm_Info.shmid, IPC_RMID, 0 );
#endif

  _XImg->data = (char*)_Shm_Info.shmaddr;

  Set_Virtual(Width(), Height(), Format(), (SKL_BYTE*)_XImg->data, bps);
  return 1;
 
Failed:
  Clear_Shm();
  Cleanup();
  return 0;
}

void SKL_X11_SHM_WIN::Clear_Shm()
{
  if (_Shm_Info.shmid>=0) {
    SKL_ASSERT(_Shm_Info.shmaddr!=0);
    shmdt( _Shm_Info.shmaddr );
    _Shm_Info.shmaddr = 0;
#ifdef DONT_DETACH_FIRST
    shmctl( _Shm_Info.shmid, IPC_RMID, 0 );
#endif
    XShmDetach( Get_XDisplay(), &_Shm_Info );
  }
  _Shm_Info.shmid = -1;
}

//////////////////////////////////////////////////////////

void SKL_X11_SHM_WIN::Real_Unlock(int Xo, int Yo, int W, int H)
{
  SKL_ASSERT(Xo>=0 && Yo>=0 && (W-Xo)<=Width() && (H-Yo)<=Height());

  Display *Dsp = Get_XDisplay();
  XShmPutImage( Dsp, _Win, _GC, _XImg,
                Xo, Yo, Xo, Yo, W, H, True );

  if (_Drv->VSync_Is_On()) {
#ifndef COMPLETION_BUG
    XEvent Event;
    while ( !XCheckTypedWindowEvent(Dsp, _Win, _Completion_Type, &Event) );
//    while( XNextEvent(Dsp, &Event) );
// SKL_ANY Addr = (SKL_ANY)( _Shm_Info..shmaddr + ((XShmCompletionEvent*)&Event)->offset );                            
        
#else
    XFlush(Dsp); // TODO: Gahhh!
#endif
  }
}

#endif  /* SKL_USE_SHM */

//////////////////////////////////////////////////////////

#endif  /* SKL_USE_X11 */
#endif  /* SKL_NO_VIDEO */
