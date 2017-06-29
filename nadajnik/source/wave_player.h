#pragma once
#ifndef WAVE_PLAYER_H_
#define WAVE_PLAYER_H_
#include <windows.h>
#include <mmsystem.h>
#include <cstdint>

/*
* some good values for block size and count
*/
#define BLOCK_SIZE 8192//8192
#define BLOCK_COUNT 20
#define BUFFER_SIZE 1024
/*
 * Timer set to fill the buffer id
 */
#define WRITE_DATA_TIMER_ID 1

 /*
 * function prototypes
 */
void CALLBACK waveOutProc(HWAVEOUT, UINT, DWORD, DWORD, DWORD);
VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD    dwTime);
WAVEHDR* allocateBlocks(int size, int count);
void freeBlocks(WAVEHDR* blockArray);
void writeAudio(HWAVEOUT hWaveOut, LPSTR data, int size);
void clearBuffer();


struct WavePlayer
{

  WavePlayer(HWND hWnd);
  VOID Play(LPCSTR lpPath);
  VOID PlayTest();
  VOID Stop();
  /*
  * module level variables
  */
  CRITICAL_SECTION waveCriticalSection;
  WAVEHDR* waveBlocks;
  volatile int waveFreeBlockCount;
  int waveCurrentBlock;
  HWND hMainWindow; /*Handle to parent window*/
  HWAVEOUT hWaveOut; /* device handle */
  HANDLE hFile;/* file handle */
  WAVEFORMATEX wfx; /* look this up in your documentation */
  int_fast16_t buffer[BUFFER_SIZE]; /* intermediate buffer for reading */
  int i;
  BOOL isPlaying;
};


_declspec(selectany) WavePlayer* wavePlayer;

#endif

