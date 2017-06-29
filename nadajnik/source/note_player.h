#pragma once

#include <Windows.h>
#include <map>
#include <winuser.h>
#include <math.h>
#include <boost/math/constants/constants.hpp>
#include <cstdio>
#include <vector>

#pragma comment(lib,"winmm.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "advapi32.lib")


class NotePlayer
{
public:
  NotePlayer();
  ~NotePlayer();
  BOOL InitNotePlayer(WORD channels,DWORD sample_rate,WORD bits_per_sample);
  void Play(double duration, double frequency);
  void Play(double duration, std::string note);
  void GetRythmValuesFormString(std::string rythm, std::vector<std::string> splited_rythm, std::vector<int>& rythm_values);
  void PlayMelody(std::string notes, std::string rythm, double tempo_bpm);
private:
  WAVEFORMATEX pcm_wave_format;
  BYTE* audio_buffer;
  HWAVEOUT waveout;
  MMRESULT mm_result;
  std::map<std::string, double> tone_frequencies;

  void InitTones();


};

