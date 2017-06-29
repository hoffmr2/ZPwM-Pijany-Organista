#include "note_player.h"
#include<climits>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <vector>

#define BOOST boost::algorithm

NotePlayer::NotePlayer(): pcm_wave_format({0}), audio_buffer(NULL), waveout(NULL), mm_result(0)
{
  InitTones();
}


NotePlayer::~NotePlayer()
{
}

BOOL NotePlayer::InitNotePlayer(WORD channels, DWORD sample_rate, WORD bits_per_sample)
{
  pcm_wave_format.wFormatTag = WAVE_FORMAT_PCM;
  pcm_wave_format.nChannels = channels;
  pcm_wave_format.nSamplesPerSec = sample_rate;
  pcm_wave_format.wBitsPerSample = bits_per_sample;
  pcm_wave_format.nAvgBytesPerSec = pcm_wave_format.nSamplesPerSec*pcm_wave_format.wBitsPerSample / 8;
  pcm_wave_format.nBlockAlign = 1;
  pcm_wave_format.cbSize = 0;

  UINT devId;

  for (devId = 0; devId<waveOutGetNumDevs(); ++devId)
  {
    mm_result = waveOutOpen(&waveout, devId, &pcm_wave_format, 0, 0, CALLBACK_NULL);
    if (mm_result == MMSYSERR_NOERROR)//gdy nie udalo sie otworzyc
    {
      return TRUE;
    }
  }

  if (mm_result != MMSYSERR_NOERROR)
  {
    return FALSE;
  }
}

void NotePlayer::Play(double duration, double frequency)
{
  float scale_factor =  CHAR_MAX-20; //sizeof(pcm_wave_format.wBitsPerSample) / 2;
  int length = duration*pcm_wave_format.nAvgBytesPerSec;
  if (audio_buffer != NULL)
    delete[] audio_buffer;
  audio_buffer = new BYTE[length];
  for (int i = 0; i<length; ++i)
  {
    float t = i / (float)pcm_wave_format.nSamplesPerSec;
    audio_buffer[i] = scale_factor * std::sin(2 * boost::math::double_constants::pi*t*frequency) + scale_factor;
  }

  WAVEHDR whdr;
  ZeroMemory(&whdr, sizeof(WAVEHDR));
  whdr.lpData = LPSTR(audio_buffer);
  whdr.dwBufferLength = length;

  mm_result = waveOutPrepareHeader(waveout, &whdr, sizeof(WAVEHDR));
  mm_result = waveOutWrite(waveout, &whdr, sizeof(WAVEHDR));

  while ((whdr.dwFlags & WHDR_DONE) != WHDR_DONE)Sleep(100);


}

void NotePlayer::Play(double duration, std::string note)
{
  if (tone_frequencies.find(note)._Ptr == NULL)
    return;
  else
  {
    Play(duration, tone_frequencies[note]);
  }
}

void NotePlayer::GetRythmValuesFormString(std::string rythm, std::vector<std::string> splited_rythm, std::vector<int>& rythm_values)
{
  BOOST::split(splited_rythm, rythm,boost::is_any_of( ";"), boost::token_compress_on);
  for(int i=0;i<splited_rythm.size();++i)
  {
    rythm_values.push_back(std::stoi(splited_rythm[i]));
  }
}

void NotePlayer::PlayMelody(std::string notes, std::string rythm, double tempo_bpm)
{
  auto full_note_duration = 240/ tempo_bpm;
  std::vector<std::string>splited_notes;
  std::vector<std::string> splited_rythm;
  std::vector<int>rythm_values;
  BOOST::split(splited_notes, notes,boost::is_any_of( ";"), boost::token_compress_on);

  GetRythmValuesFormString(rythm, splited_rythm, rythm_values);
  if (splited_notes.size() != rythm_values.size())
    return;

  for(int i=0;i<splited_notes.size();++i)
  {
    if (tone_frequencies.find(splited_notes[i])._Ptr == NULL)
      return;
    Play(full_note_duration / rythm_values[i], tone_frequencies[splited_notes[i]]);
  }
}

void NotePlayer::InitTones()
{
  const int octave_number = 3;
  const int half_tones = 12;
  auto current_octave = 0;
  std::string notes[12] =
  { "A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#" };

  double ratio = pow(2.0, 1.0 / 12.0);
  double a = 110;
  for (int i = 0; i <= octave_number*half_tones; i++)
  {
    auto text = notes[i % 12];
    tone_frequencies.insert(std::pair<std::string,int>(text.append(std::to_string(i/12)) ,a * pow(ratio, i)));
  }
}

