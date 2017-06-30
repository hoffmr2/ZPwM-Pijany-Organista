#include <Windows.h>
#include "res.h"
#include <winuser.h>
#include <cstdio>
#include <portaudio.h>
#include <audio_demodulator.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "advapi32.lib")


/*
 * defines
 */
#define APP_NAME "Knur"
#define ERROR_MESSAGE "error with audio device"
#define SAMPLE_RATE 44100
#define TIMER_ACTUAL_BYTE 1
#define TIMER_REPAINT 2

#define RECT_START_X 50
#define RECT_START_Y 120
#define RECT_WIDTH 100
#define RECT_HEIGHT 30

/*
 * Var declarations
 */

HWND hwnd_main_window;
HWND hwnd_static_byte;
HDC hdc = NULL;
RECT rect;
PaError err;
PaStream *stream;
AudioDemodulator* audio_demodulator;
char actual_byte[30];
char c = '0';
/*
 * Function declarations
 */
void InitRect();
void ShowErrorDialog();
void OnWMmnitDialog();
void OnWmClose(HWND hwndDlg);
bool OnWmCommand(HWND hwndDlg, WPARAM wParam);
bool OnBnClicked(HWND hwndDlg, WPARAM wParam);
void OnIdcButtonHelp(HWND hwndDlg);

/*
 * Callback function for audio processing
 * makes program work with port audio library
 */

static int AudioProcessingCallback(const void *input,
  void *output,
  unsigned long frameCount,
  const PaStreamCallbackTimeInfo* timeInfo,
  PaStreamCallbackFlags statusFlags,
  void *userData);


VOID CALLBACK TimerProc(
  _In_ HWND     hwnd,
  _In_ UINT     uMsg,
  _In_ UINT_PTR idEvent,
  _In_ DWORD    dwTime
);

VOID CALLBACK TimerProcRepaint(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
  case WM_COMMAND:
    return OnWmCommand(hwndDlg, wParam);
  case WM_PAINT:
    hdc = GetDC(hwndDlg);
    if(audio_demodulator->GetClockOrStartState() != CLOCK_STATE_HIGH)
      FillRect(hdc, &rect,CreateSolidBrush(RGB(255,0,0)));
    else
      FillRect(hdc, &rect, CreateSolidBrush(RGB(0, 255, 0)));
    ReleaseDC(hwndDlg, hdc);
    return FALSE;
  case WM_INITDIALOG:
    hwnd_static_byte = GetDlgItem(hwndDlg, IDC_STATIC_RECEIVED);
    OnWMmnitDialog();
    return TRUE;
  case WM_CLOSE:
    OnWmClose(hwndDlg);
    return TRUE;
  }
  return FALSE;
}







int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{

  hwnd_main_window = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAINVIEW), NULL, DialogProc);
  ShowWindow(hwnd_main_window, iCmdShow);
  SetTimer(hwnd_main_window, TIMER_ACTUAL_BYTE, 1, TimerProc);
  SetTimer(hwnd_main_window, TIMER_REPAINT, 100, TimerProcRepaint);
  //pêtla obs³ugi komunikatów

  MSG msg = {};

  while(GetMessage(&msg,NULL,0,0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
 
  return 0;
}


void ShowErrorDialog()
{
  MessageBox(hwnd_main_window, ERROR_MESSAGE, APP_NAME, MB_OK | MB_ICONERROR);
  ExitProcess(0);
}

void OnWMmnitDialog()
{
  InitRect();
  err = Pa_Initialize();
  if (err != paNoError)
    ShowErrorDialog();

  PaStreamParameters in;
  int device_index = Pa_GetDefaultInputDevice();
  const PaDeviceInfo* device_info = Pa_GetDeviceInfo(device_index);
  in.channelCount = 1;
  in.device = Pa_GetDefaultInputDevice();
  in.hostApiSpecificStreamInfo = NULL;
  in.sampleFormat = paFloat32;
  in.suggestedLatency = device_info->defaultLowInputLatency;

  err = Pa_OpenStream(&stream,
    &in,          /* no input channels */
    NULL,
    SAMPLE_RATE,
    paFramesPerBufferUnspecified, 
    NULL,
    AudioProcessingCallback, /* this is your callback function */
    NULL); /*This is a pointer that will be passed to
           your callback*/
  if (err != paNoError)
    ShowErrorDialog();

  audio_demodulator = new AudioDemodulator();
  audio_demodulator->InitFilters(SAMPLE_RATE);

  err = Pa_StartStream(stream);
  if (err != paNoError)
    ShowErrorDialog();
  InvalidateRect(hwnd_main_window, NULL, TRUE);
  UpdateWindow(hwnd_main_window);

}

void OnWmClose(HWND hwndDlg)
{
  err = Pa_StopStream(stream);
  if (err != paNoError)
    ShowErrorDialog();

  err = Pa_Terminate();
  if (err != paNoError)
    ShowErrorDialog();

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
  case IDC_BUTTON_HELP:
    OnIdcButtonHelp(hwndDlg);
    return TRUE;

  default:
    return FALSE;
  }
}

void OnIdcButtonHelp(HWND hwndDlg)
{
  ShellExecute(NULL, "open", "..\\help\\help.pdf", NULL, NULL, SW_SHOWNORMAL);
}

int AudioProcessingCallback(const void* input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData)
{
  c= audio_demodulator->Demodulate((float*)input, frameCount);

  
  wsprintf(actual_byte,"Odebrano %c",c);

  return 0;
}

void __stdcall TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
  SetWindowText(hwnd_static_byte, actual_byte);
}

void __stdcall TimerProcRepaint(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
  hdc = GetDC(hwnd);
  if (audio_demodulator->GetClockOrStartState() != CLOCK_STATE_HIGH)
    FillRect(hdc, &rect, CreateSolidBrush(RGB(255, 0, 0)));
  else
    FillRect(hdc, &rect, CreateSolidBrush(RGB(0, 255, 0)));
  ReleaseDC(hwnd, hdc);

}


void InitRect()
{
  rect.left = RECT_START_X;
  rect.top = RECT_START_Y;
  rect.right = RECT_START_X + RECT_WIDTH;
  rect.bottom = RECT_START_Y + RECT_HEIGHT;
}