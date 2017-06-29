#include "wave_player.h"
#include <cassert>
#include <cstring>
#include <cstdio>


WavePlayer::WavePlayer(HWND hWnd)
{
  hMainWindow = hWnd;

  /*
  * initialise the module variables
  */

  waveBlocks = allocateBlocks(BLOCK_SIZE, BLOCK_COUNT);
  waveFreeBlockCount = BLOCK_COUNT;
  waveCurrentBlock = 0;

  InitializeCriticalSection(&waveCriticalSection);

  /*
   * it requiers modification to read this
   * data from the wav file in the future
   */
  wfx.nSamplesPerSec = 44100; /* sample rate */
  wfx.wBitsPerSample = 16; /* sample size */
  wfx.nChannels = 2; /* channels*/
  wfx.cbSize = 0; /* size of _extra_ info */
  wfx.wFormatTag = WAVE_FORMAT_PCM;
  wfx.nBlockAlign = (wfx.wBitsPerSample * wfx.nChannels) >> 3;
  wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

  isPlaying = FALSE;
}

void WavePlayer::Play(LPCSTR lpPath)
{
  if ((hFile = CreateFile(
    lpPath,
    GENERIC_READ,
    FILE_SHARE_READ,
    NULL,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL,
    NULL
  )) == INVALID_HANDLE_VALUE) {
    fprintf(stderr, "%s: unable to open file \n", "knur");
    return;
  }
  /*
  * try to open the default wave device. WAVE_MAPPER is
  * a constant defined in mmsystem.h, it always points to the
  * default wave device on the system (some people have 2 or
  * more sound cards).
  */
  // path = LPCSTR("jazz.wav");

  if (waveOutOpen(
    &hWaveOut,
    WAVE_MAPPER,
    &wfx,
    (DWORD_PTR)waveOutProc,
    (DWORD_PTR)&waveFreeBlockCount,
    CALLBACK_FUNCTION
  ) != MMSYSERR_NOERROR) {
    fprintf(stderr, "%s: unable to open wave mapper device\n", lpPath);
  }

  isPlaying = TRUE;
}

void WavePlayer::PlayTest()
{
  hFile = NULL;
  hFile = CreateFile("..//res//test.txt",
    GENERIC_READ,
    0,
    (LPSECURITY_ATTRIBUTES)0,
    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);

  if ((hFile) == INVALID_HANDLE_VALUE) {
    fprintf(stderr, "%s: unable to open file \n", "knur");
    return;
  }
  /*
  * try to open the default wave device. WAVE_MAPPER is
  * a constant defined in mmsystem.h, it always points to the
  * default wave device on the system (some people have 2 or
  * more sound cards).
  */
  // path = LPCSTR("jazz.wav");

  if (waveOutOpen(
    &hWaveOut,
    WAVE_MAPPER,
    &wfx,
    (DWORD_PTR)waveOutProc,
    (DWORD_PTR)&waveFreeBlockCount,
    CALLBACK_FUNCTION
  ) != MMSYSERR_NOERROR) {
    fprintf(stderr, " unable to open wave mapper device\n");
  }

  isPlaying = TRUE;
}


void WavePlayer::Stop()
{
 // freeBlocks(waveBlocks);
 // waveOutClose(hWaveOut);
  KillTimer(hMainWindow, WRITE_DATA_TIMER_ID);
  isPlaying = FALSE;
  CloseHandle(hFile);
}


void CALLBACK waveOutProc(
  HWAVEOUT hWaveOut,
  UINT uMsg,
  DWORD dwInstance,
  DWORD dwParam1,
  DWORD dwParam2
)
{
  /*
  * pointer to free block counter
  */
  int* freeBlockCounter = (int*)dwInstance;
  /*
  * ignore calls that occur due to openining and closing the
  * device.
  */
  switch (uMsg)
  {
  case WOM_DONE:
    EnterCriticalSection(&wavePlayer->waveCriticalSection);

    (*freeBlockCounter)++;
    LeaveCriticalSection(&wavePlayer->waveCriticalSection);
    break;
  case MM_WOM_OPEN:
    EnterCriticalSection(&wavePlayer->waveCriticalSection);
    assert(wavePlayer->hMainWindow != NULL);
    SetTimer(wavePlayer->hMainWindow, WRITE_DATA_TIMER_ID, 1, TimerProc);
    LeaveCriticalSection(&wavePlayer->waveCriticalSection);
    break;
  default:
    break;
  }
}



WAVEHDR* allocateBlocks(int size, int count)
{
  unsigned char* buffer;
  int i;
  WAVEHDR* blocks;
  DWORD totalBufferSize = (size + sizeof(WAVEHDR)) * count;
  /*
  * allocate memory for the entire set in one go
  */
  if ((buffer = (BYTE*)HeapAlloc(
    GetProcessHeap(),
    HEAP_ZERO_MEMORY,
    totalBufferSize
  )) == NULL) {
    fprintf(stderr, "Memory allocation error\n");
    ExitProcess(1);
  }
  /*
  * and set up the pointers to each bit
  */
  blocks = (WAVEHDR*)buffer;
  buffer += sizeof(WAVEHDR) * count;
  for (i = 0; i < count; i++) {
    blocks[i].dwBufferLength = size;
    blocks[i].lpData = (LPSTR)buffer;
    buffer += size;
  }
  return blocks;
}
void freeBlocks(WAVEHDR* blockArray)
{
  /*
  * and this is why allocateBlocks works the way it does
  */
  HeapFree(GetProcessHeap(), 0, blockArray);
}

void writeAudio(HWAVEOUT hWaveOut, LPSTR data, int size)
{
  WAVEHDR* current;
  int remain;
  current = &wavePlayer->waveBlocks[wavePlayer->waveCurrentBlock];
  while (size > 0) {
    /*
    * first make sure the header we're going to use is unprepared
    */
    if (current->dwFlags & WHDR_PREPARED)
      waveOutUnprepareHeader(hWaveOut, current, sizeof(WAVEHDR));
    if (size < (int)(BLOCK_SIZE - current->dwUser)) {
      memcpy(current->lpData + current->dwUser, data, size);
      current->dwUser += size;
      break;
    }
    remain = BLOCK_SIZE - current->dwUser;
    memcpy(current->lpData + current->dwUser, data, remain);
    size -= remain;
    data += remain;
    current->dwBufferLength = BLOCK_SIZE;
    waveOutPrepareHeader(hWaveOut, current, sizeof(WAVEHDR));
    waveOutWrite(hWaveOut, current, sizeof(WAVEHDR));
    EnterCriticalSection(&wavePlayer->waveCriticalSection);
    wavePlayer->waveFreeBlockCount--;
    LeaveCriticalSection(&wavePlayer->waveCriticalSection);
    /*
    * wait for a block to become free
    */
    while (!wavePlayer->waveFreeBlockCount)
      Sleep(10);
    /*
    * point to the next block
    */
    wavePlayer->waveCurrentBlock++;
    wavePlayer->waveCurrentBlock %= BLOCK_COUNT;
    current = &wavePlayer->waveBlocks[wavePlayer->waveCurrentBlock];
    current->dwUser = 0;
  }
}

void clearBuffer()
{
  std::memset(wavePlayer->buffer, 0, sizeof wavePlayer->buffer);
}
