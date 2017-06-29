

#ifndef AUDIO_MODULATOR_H_  
#define AUDIO_MODULATOR_H_   
#include "carrier_signals.h"
#include <array>

struct SineAngleParameters
{
  SineAngleParameters() : current_angle(0), delta_angle(0) {}

  double current_angle;
  double delta_angle;

};


class AudioModulator 
{
public:
  AudioModulator();
  AudioModulator(double sample_rate);
  ~AudioModulator();
  

  void SetSampleRate(double sample_rate);
  void Modulate(float* buffer, int buffer_size, char byte);
  void ModulateInt16(int* buffer, int buffer_size, char byte);
  void UpdateSyncSignalAngle();
  void UpdateStartSignalAngle();
  void SendSyncSignal(float* buffer, int buffer_size);
  void SendSyncSignalInt16(int* buffer, int buffer_size);
  void SendStartSignalInt16(int* buffer, int buffer_size);
  void ResetAngles();
private:
  void UpdateAngle(int carrier_index, int bit);
  bool IsBitZero(char byte, int mask, int j);
  std::array<SineAngleParameters, SIGNALS_NUMBER> carrier_angles_;
  double sample_rate_;


};

#endif  