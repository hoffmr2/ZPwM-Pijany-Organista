#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include "res.h"
#include <thread>

/*
 * Wave player takes care
 * of generating audio
 */
#include "wave_player.h"
#include "open_file_dialog.h"
#include <audio_modulator.h>
#include <cassert>


/*
 * Constant defines
 */

#define START_TRANSMISSION 0
#define SEND_DATA 1
#define END_TRANSMISSION 2
#define SYNC_SIGNAL_DURATION 6 //duration in data blocks
#define SYMBOL_DURATION 14
#define TIMER_FINALIZE 44

struct MainPicturePosition
{
  const int offset_x=30;
  const int offset_y=10;
  const int width = 500;
  const int height = 230;
};


/*
 * User globals
 */
HWND hwnd_main_window;
AudioModulator* audio_modulator;
HBITMAP main_image;
MainPicturePosition main_picture_position = MainPicturePosition();
HINSTANCE h_instance;
CHAR edit_box_text[200];

int  timer = 0;
bool send_sync = false;
bool is_finished = false;
const std::string message = "KNUR w knurowni";


static char c = 'A';
static int state_indicator = START_TRANSMISSION;//informs about actual transmitter state


/*
 * User functions
 */
void InitTransmission();
void OnWmPaint(HWND hwndDlg);
void OnWmInitDialog(HWND hwndDlg);
void OnWmClose(HWND hwndDlg);
bool OnWmCommand(HWND hwndDlg, WPARAM wParam);
bool OnBnClicked(HWND hwndDlg, WPARAM wParam);
void OnIdcButtonStart(HWND hwndDlg);
void OnIdcButtonPlay(HWND hwndDlg);
void OnIdcButtonTestSequence(HWND hwndDlg);


void SendSyncSignals();
void SendDataSignals();
void ClearBuffer();
void IncTimer();
bool IsTimeForSyncSignal();
bool IsTimeForDataSignal();
void GetNextByte();



/*
 * Implementation of function declarated
 * in wave_player.h
 */
VOID CALLBACK TimerProcFinalize(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD    dwTime)
{
  if (uMsg != WM_TIMER)
    return;
  if (idEvent != TIMER_FINALIZE)
    return;
  EnableWindow(GetDlgItem(hwnd_main_window, IDC_BUTTON_PLAY), true);
  EnableWindow(GetDlgItem(hwnd_main_window, IDC_BUTTON_TEST_SEQUENCE), true);
  KillTimer(hwnd, TIMER_FINALIZE);

}

VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD    dwTime)
{
  if (uMsg != WM_TIMER)
    return;
  if (idEvent != WRITE_DATA_TIMER_ID)
    return;
  EnterCriticalSection(&wavePlayer->waveCriticalSection);


  /*
  * Write data to buffer
  */
  if (timer == 0)
  {
    GetNextByte();
  }

  IncTimer();

  if ( IsTimeForSyncSignal())
  {
    SendSyncSignals();
  }
  else
  {
    if (IsTimeForDataSignal() )
    {
      SendDataSignals();
    }
    else
    {
      ClearBuffer();
    }
  }
  

  /*
  * Write filled buffer
  */
  LeaveCriticalSection(&wavePlayer->waveCriticalSection);
  writeAudio(wavePlayer->hWaveOut, (char*)wavePlayer->buffer, sizeof(wavePlayer->buffer));


}



LPCSTR path;
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
  case WM_COMMAND:
   return OnWmCommand(hwndDlg, wParam);
  case WM_INITDIALOG:
    OnWmInitDialog(hwndDlg);
    return TRUE;
  case WM_CLOSE:
    OnWmClose(hwndDlg);
    return TRUE;

  case WM_PAINT:
    OnWmPaint(hwndDlg);
      return FALSE;

  }
  return FALSE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{

  h_instance = hInstance;
  hwnd_main_window = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAINVIEW), NULL, DialogProc);
  ShowWindow(hwnd_main_window, iCmdShow);
  MSG msg = {};

  while (GetMessage(&msg, NULL, 0, 0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return 0;
}


void InitTransmission()
{
  timer = 0;
  send_sync = false;
  is_finished = false;

}

void OnWmPaint(HWND hwndDlg)
{
  HDC hdc = GetDC(hwndDlg);
  BITMAP bmp = { 0 };
  GetObject(main_image, sizeof(bmp), &bmp);

  auto hdc_bitmap = CreateCompatibleDC(hdc);
  SelectObject(hdc_bitmap, main_image);
  StretchBlt(hdc, main_picture_position.offset_x,
    main_picture_position.offset_y,
    main_picture_position.width,
    main_picture_position.height,
    hdc_bitmap, 0, 0,
    bmp.bmWidth,
    bmp.bmHeight, SRCCOPY);
  DeleteDC(hdc_bitmap);
  ReleaseDC(hwndDlg, hdc);
}

void OnWmInitDialog(HWND hwndDlg)
{
  wavePlayer = new WavePlayer(hwndDlg);
  audio_modulator = new AudioModulator(wavePlayer->wfx.nSamplesPerSec);
  audio_modulator->ResetAngles(); //Prepare sine generators
  path = "../jazz.wav";
  main_image = LoadBitmap(h_instance, MAKEINTRESOURCE(IDB_MAIN_IMAGE));
  InvalidateRect(hwndDlg, NULL, TRUE);
  UpdateWindow(hwndDlg);
}

void OnWmClose(HWND hwndDlg)
{
  delete wavePlayer;
  delete audio_modulator;
  DestroyWindow(hwndDlg);
  PostQuitMessage(0);

}

bool OnWmCommand(HWND hwndDlg, WPARAM wParam)
{
  switch (HIWORD(wParam))
  {
  case BN_CLICKED:
    return OnBnClicked(hwndDlg, wParam);
  default:
    return FALSE;
  }
}

bool OnBnClicked(HWND hwndDlg, WPARAM wParam)
{
  switch (LOWORD(wParam))
  {
  case IDC_BUTTON_START:
    //This case  finalizes transmission
    //It's name is quite misleading
    OnIdcButtonStart(hwndDlg);
    return TRUE;

  case IDC_BUTTON_PLAY:
    OnIdcButtonPlay(hwndDlg);
    return TRUE;
  case IDC_BUTTON_TEST_SEQUENCE:
    OnIdcButtonTestSequence(hwndDlg);
    return TRUE;
  default:
    return FALSE;
  }
}

void OnIdcButtonStart(HWND hwndDlg)
{
  wavePlayer->Stop();
  SetTimer(hwndDlg, TIMER_FINALIZE, 1000, TimerProcFinalize);

}

void OnIdcButtonPlay(HWND hwndDlg)
{
  OpenFileDialog* openFileDialog1 = new OpenFileDialog();
  openFileDialog1->FilterIndex = 1;
  openFileDialog1->Flags |= OFN_SHOWHELP;
  openFileDialog1->InitialDir = _T("");
  openFileDialog1->Title = _T("Open File");

  if (!openFileDialog1->ShowDialog())
  {
    return;
  }
  InitTransmission();
  wavePlayer->Play(openFileDialog1->FileName);
  EnableWindow(GetDlgItem(hwnd_main_window, IDC_BUTTON_PLAY), false);
  EnableWindow(GetDlgItem(hwnd_main_window, IDC_BUTTON_TEST_SEQUENCE), false);


  delete openFileDialog1;
}

void OnIdcButtonTestSequence(HWND hwndDlg)
{
  InitTransmission();
  wavePlayer->Play(TEXT("..\\res\\test.txt"));
  if (wavePlayer->isPlaying)
  {
    EnableWindow(GetDlgItem(hwnd_main_window, IDC_BUTTON_PLAY), false);
    EnableWindow(GetDlgItem(hwnd_main_window, IDC_BUTTON_TEST_SEQUENCE), false);
  }
}

void SendSyncSignals()
{
  switch (state_indicator)
  {
  case SEND_DATA:
    if (send_sync)
      audio_modulator->SendSyncSignalInt16(wavePlayer->buffer, BUFFER_SIZE);
    else
    {
      audio_modulator->SendStartSignalInt16(wavePlayer->buffer, BUFFER_SIZE);
    }
    break;
  case END_TRANSMISSION:
    audio_modulator->SendStartSignalInt16(wavePlayer->buffer, BUFFER_SIZE);
    break;
  default:
    assert(true);
  }
}

void SendDataSignals()
{
  send_sync = true;
  switch (state_indicator)
  {

  case SEND_DATA:
    audio_modulator->ModulateInt16(wavePlayer->buffer, BUFFER_SIZE, c);
    break;
  case END_TRANSMISSION:
    ClearBuffer();
    break;
  default:
    assert(true);
  }
}

void ClearBuffer()
{
  for (auto i = 0; i < BUFFER_SIZE; ++i)
  {
    wavePlayer->buffer[i] = 0;
  }
}

void IncTimer()
{
  timer = (timer + 1) % SYMBOL_DURATION;
}

bool IsTimeForSyncSignal()
{
  return timer > 0 && timer < SYNC_SIGNAL_DURATION;
}

bool IsTimeForDataSignal()
{
  return timer >= SYNC_SIGNAL_DURATION && timer < SYMBOL_DURATION;
}

void GetNextByte()
{
  //read  next byte
  DWORD nBytesRead = 0;
  BOOL bResult = ReadFile(wavePlayer->hFile, &c, 1, &nBytesRead, NULL);
  if ((bResult && nBytesRead) != 0)
  {
    state_indicator = SEND_DATA;
  }
  else
  {
    if (is_finished)
    {
      is_finished = false;
      send_sync = false;
      SendMessage(hwnd_main_window, WM_COMMAND, MAKEWPARAM(IDC_BUTTON_START, BN_CLICKED), (LPARAM)(NULL));
    }
    else
    {
      state_indicator = END_TRANSMISSION;
      is_finished = true;
    }

  }
}