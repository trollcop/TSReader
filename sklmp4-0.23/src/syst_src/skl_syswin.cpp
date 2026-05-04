/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_syswin.cpp
 *
 *  Windows fluff
 ********************************************************/

#ifdef _WINDOWS

#include "skl.h"
#include "skl_syst/skl_exception.h"
#include "skl_syst/skl_syswin.h"

#include <direct.h>  // for getcwd()...

#define SKL_CLASS_NAME   "SKL"

//////////////////////////////////////////////////////////
/////// MainWndproc. Callback for Win messages ///////////
//////////////////////////////////////////////////////////

long FAR PASCAL MainWndproc( HWND hWnd, UINT message,
                             WPARAM wParam, LPARAM lParam )
{
  switch( message )
  {
    case WM_SETCURSOR:
      SetCursor(NULL);
      return 1;
    break;

    case WM_CREATE:
    break;

    case WM_PAINT:
    {
      PAINTSTRUCT ps;
      HDC         hdc;
      hdc = BeginPaint( hWnd, &ps );
      EndPaint( hWnd, &ps );
      return 1;
    }
    break;

    case WM_CLOSE:
      PostQuitMessage( 0 );
    break;

    case WM_DESTROY:
      PostQuitMessage( 0 );
    break;

    case WM_MOVE:
      return 0;
    break;

    case WM_DISPLAYCHANGE:
    case WM_SIZE:
    {
//      float x, y, Width, Height;
//      SKL_WIN_SYSTEM::Get_Win_Infos( x, y, Width, Height );
    }
    break;

    case WM_KEYDOWN:
      if ( wParam==VK_ESCAPE ) PostMessage(hWnd, WM_CLOSE, 0, 0);
    break;

    case WM_CHAR:
      SKL_WIN_SYSTEM::Search(hWnd)->Enqueue(tolower(wParam));
      return 0;
    break;

    case WM_ACTIVATEAPP:
      switch(wParam) {
        case 0:
        break;
      }
    break;
    
    default:
    break;
  }
  return DefWindowProc(hWnd, message, wParam, lParam);
}

//////////////////////////////////////////////////////////
// SKL_WIN_SYSTEM
//////////////////////////////////////////////////////////

SKL_WIN_SYSTEM *SKL_WIN_SYSTEM::_List = 0;
SKL_WIN_SYSTEM *SKL_WIN_SYSTEM::_Last = 0;
HINSTANCE SKL_WIN_SYSTEM::_hInst = 0;
SKL_BTM SKL_WIN_SYSTEM::_Display(SKL_MEM);
HWND SKL_WIN_SYSTEM::_App_Win = 0;

//////////////////////////////////////////////////////////

SKL_WIN_SYSTEM::SKL_WIN_SYSTEM()
{
  if (_App_Win==0)
    SKL_WIN_SYSTEM::Init(0);    // HACK!? (for console app...)
  _Head = _Tail = 0;
  _Win_Shown = 0;
  _Win = 0;
  _Fullscreen = 0;
  _W = _H = 0;
  _Next = _List;
  _List = this;
  _Last = this;
  _Is_Active = 0;
}

SKL_WIN_SYSTEM::~SKL_WIN_SYSTEM()
{
  for( SKL_WIN_SYSTEM **WS = &_List; (*WS)!=0; WS=&(*WS)->_Next) {
    if ((*WS) == this) {
      (*WS) = (*WS)->_Next;
      break;
    }
  }
  if (_Last==this) _Last = _List;
  Kill();
}

void SKL_WIN_SYSTEM::Kill() {
  Destroy_Window();
}

//////////////////////////////////////////////////////////

int SKL_WIN_SYSTEM::Flush_Messages(int Stop_On_Event)
{
  MSG msg;
  while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    if (Stop_On_Event)
      if ( !Queue_Empty() ) return 1;
  }
  return 0;
}

//////////////////////////////////////////////////////////
// keys handling

void SKL_WIN_SYSTEM::Enqueue(char c) { 
  _Queue[_Head++] = c;
  if (_Head>=QUEUE_SIZE) _Head = 0;
}

char SKL_WIN_SYSTEM::Dequeue() {
  int c = _Queue[_Tail++];
  if (_Tail>=QUEUE_SIZE) _Tail = 0;
  return c;
}

int SKL_WIN_SYSTEM::Has_Key() const
{
  ((SKL_WIN_SYSTEM*)this)->Flush_Messages(1);
  if ( !Queue_Empty() ) return 1;
  return 0;
}

int SKL_WIN_SYSTEM::Get_Key()
{
  if ( !Queue_Empty() ) return Dequeue();
  return 0;
}

//////////////////////////////////////////////////////////
// Windows managment

void SKL_WIN_SYSTEM::Set_Position(int x, int y)
{
  if (_Win==0) return;

  int W = _W;
  int H = _H;

/*
  RECT Last;
  Last.left = Last.top = 0;
  Last.right = W; Last.bottom = H;
  AdjustWindowRect( &Last, GetWindowLong(_Win, GWL_STYLE), FALSE );
  W = Last.right - Last.left;
  H = Last.bottom - Last.top;
*/

  W += 2*GetSystemMetrics(SM_CXSIZEFRAME);  // SM_CXBORDER?
  H += 2*GetSystemMetrics(SM_CYSIZEFRAME)   // SM_CYBORDER?
        + GetSystemMetrics(SM_CYCAPTION);

  SetWindowPos( _Win, NULL, 
                x, y, W, H,
                (SWP_NOCOPYBITS | SWP_NOZORDER) );
}

void SKL_WIN_SYSTEM::Set_Name(SKL_CST_STRING Name) {
  if (_Win!=0 && Name!=0)
    SetWindowText( _Win, Name );
}

//////////////////////////////////////////////////////////

void SKL_WIN_SYSTEM::Adjust_Window( int W, int H,
                                    SKL_CST_STRING Name,
                                    int Show_Me )
{
  if ( _Win==0 ) return;

  _W = W;
  _H = H;
  Set_Position( (_Display.Width()-W)/2,(_Display.Height()-H)/2 );

  Set_Name(Name);
  SetForegroundWindow( _Win );
  if (Show_Me) Show_Window();
  UpdateWindow( _Win );
  SetFocus( _Win );
}

void SKL_WIN_SYSTEM::Show_Window()
{
  if ( _Win==0 || _Win_Shown) return;
  ShowWindow( _Win, SW_SHOWNORMAL );
  //  if (FullScreen) while( ShowCursor(NULL)>= 0 );
  _Win_Shown = 1;
}

void SKL_WIN_SYSTEM::Hide_Window()
{
  if ( _Win==0 || !_Win_Shown ) return;
  ShowWindow( _Win, SW_HIDE );
  _Win_Shown = 0;
}

//////////////////////////////////////////////////////////

HWND SKL_WIN_SYSTEM::Init_Window(HINSTANCE HInst)
{
  HWND Win;
#if 1
  Win = CreateWindowEx(
    WS_EX_APPWINDOW,  // WS_EX_TOPMOST,
    SKL_CLASS_NAME,
    "Linux does it better",
    WS_SYSMENU|WS_OVERLAPPED|WS_CAPTION|WS_MINIMIZEBOX|WS_THICKFRAME,
    0,0, 0,0,
    NULL,
    NULL,
    HInst,
    NULL );
#else
  Win = CreateWindow(
    App_Name, _OUR_CLASS_NAME_,
    WS_CAPTION|WS_BORDER|WS_SYSMENU,
    CW_USEDEFAULT, CW_USEDEFAULT,
    CW_USEDEFAULT,CW_USEDEFAULT,
    NULL, NULL, HInst, NULL );
#endif
  return Win;
}

int SKL_WIN_SYSTEM::Init_Window()
{
  if (_Win!=0) Destroy_Window();
  _Win = Init_Window( _hInst );
  if (_Win==0) return 0;
  Hide_Window();
  return 1;
}

int SKL_WIN_SYSTEM::Init_Window(int W, int H, SKL_CST_STRING Name,
                                int Show_Me)
{
  if (_Win!=0) Destroy_Window();
  _Win = Init_Window( _hInst );
  if (_Win==0) return 0;
  Adjust_Window(W, H, Name, Show_Me);
  return 1;
}

void SKL_WIN_SYSTEM::Destroy_Window()
{
  if (_Win==0) return;
  Hide_Window();
  PostMessage( _Win, WM_CLOSE, 0, 0 );
  // Flush_Messages(0);
  CloseWindow( _Win );
  DestroyWindow( _Win );
  _Win = 0;
}

//////////////////////////////////////////////////////////

void SKL_WIN_SYSTEM::Print_Infos() const
{
  printf("W,H=%dx%d, FullScreen=%d; Ok=%d; Active=%d _Win=0x%x\n",
          _W, _H, _Fullscreen, _Win_Shown, _Is_Active, _Win);
}

void SKL_WIN_SYSTEM::Print_Msg(SKL_CST_STRING Msg, int x, int y) const
{
  if (_Win==0) return;
  HDC hDC = GetDC(_Win);
  RECT Rect; 
  GetClientRect( _Win, &Rect );
  if (x<0) x = (Rect.right-Rect.left)/2;
  if (y<0) y = (Rect.bottom-Rect.top)/2;
  SetTextColor( hDC, RGB( 255,0,0 ) );
  SetBkColor( hDC, RGB(0,0,0) );
  SetTextAlign( hDC, TA_CENTER );
  ExtTextOut(hDC, x, y,
             ETO_OPAQUE, NULL, 
             Msg, strlen(Msg), NULL );
  ReleaseDC(_Win, hDC);
}

void SKL_WIN_SYSTEM::Clear() const
{
  if (_Win==0) return;
  HDC hDC = GetDC(_Win);
  RECT Rect; 
  GetClientRect( _Win, &Rect );
  SetBkColor( hDC, RGB(0,0,0) );
  // PaintRgn( hDC, hRgn);
  ReleaseDC(_Win, hDC);
}

void SKL_WIN_SYSTEM::Get_Win_Infos(int &Xo, int &Yo,
                                   int &Width, int &Height) const
{
  RECT Rect;
  if ( _Fullscreen ) GetWindowRect(_Win, &Rect);
  else GetClientRect(_Win, &Rect);
  Xo = Rect.left;
  Yo = Rect.top;
  Width = Rect.right-Rect.left;
  Height = Rect.bottom-Rect.top;
}

//////////////////////////////////////////////////////////
// Statics (for main app.)

int SKL_WIN_SYSTEM::Init(HINSTANCE h)
{
  _hInst = h;
  _List = 0;
  WNDCLASS   wc;
//   wc.cbSize = sizeof( wc );
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = MainWndproc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = h;
  wc.hIcon = NULL; // LoadIcon( NULL, IDI_ICON );
  // LoadImage(h, _OUR_CLASS_NAME_, IMAGE_ICON, 0,0, LR_DEFAULTCOLOR );
  wc.hCursor = NULL; // LoadCursor( NULL, IDC_ARROW );
  wc.hbrBackground = NULL; // GetStockObject( BLACK_BRUSH );
  wc.lpszMenuName = NULL;
  wc.lpszClassName = SKL_CLASS_NAME;
  if ( !RegisterClass( &wc ) )
    return 1;
  ShowCursor( FALSE );
  Init_Main_Display(_Display);
  _App_Win = Init_Window(_hInst);
  return 0;
}

void SKL_WIN_SYSTEM::Finish() {
  Destroy_All_Windows();
}

void SKL_WIN_SYSTEM::Destroy_All_Windows()
{ 
  while( _List!=0 ) delete _List; // side effect!
  ShowCursor( TRUE );
}

SKL_WIN_SYSTEM *SKL_WIN_SYSTEM::Search(HWND w)
{
  if (_Last!=0 && _Last->Get_Win()==w) return _Last;
  for( SKL_WIN_SYSTEM *WS = _List; WS!=0; WS=WS->_Next)
    if (WS->Get_Win() == w) {
      _Last = WS;
      return WS;
    }
  return 0;
}

//////////////////////////////////////////////////////////

int SKL_WIN_SYSTEM::Init_Main_Display(SKL_BTM &Dsp)
{
  HWND Win = Init_Window(_hInst);
  HDC hdc = GetDC( Win );
  int Dsp_Bpp = GetDeviceCaps( hdc, PLANES ) * GetDeviceCaps( hdc, BITSPIXEL );
  ReleaseDC( Win, hdc );
  CloseWindow( Win );
  DestroyWindow( Win );

  SKL_FORMAT Dsp_Fmt = 0x10000;
  if (Dsp_Bpp==8) Dsp_Fmt = 0x10000;
  else if (Dsp_Bpp==15) Dsp_Fmt = 0x20555;
  else if (Dsp_Bpp==16) Dsp_Fmt = 0x20565;
  else if (Dsp_Bpp==24) Dsp_Fmt = 0x30888;    // ?!?
  else if (Dsp_Bpp==32) Dsp_Fmt = 0x40888;    // ?!?
  int Dsp_W = GetSystemMetrics(SM_CXSCREEN);
  int Dsp_H = GetSystemMetrics(SM_CYSCREEN);
  Dsp.Set_Virtual( Dsp_W, Dsp_H, Dsp_Fmt );
  return Dsp_Bpp;
}

//////////////////////////////////////////////////////////

#endif   // _WINDOWS
