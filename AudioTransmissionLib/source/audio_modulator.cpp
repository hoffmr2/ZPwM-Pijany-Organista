#include "audio_modulator.h"
#include <assert.h>
#include <boost/math/constants/constants.hpp>
#define PI boost::math::constants::pi<double>();

AudioModulator::AudioModulator() : sample_rate_(0)
{
}

AudioModulator::AudioModulator(double sample_rate): sample_rate_(sample_rate)
{
}


AudioModulator::~AudioModulator()
{


}

void AudioModulator::SetSampleRate(double sample_rate)
{
  assert(sample_rate > 0);
  sample_rate_ = sample_rate;
}


bool AudioModulator::IsBitZero(char byte, int mask, int j)
{
  return (byte & (mask << j)) == 0;
}

/*
 * Modulates the float based buffer
 */
void AudioModulator::Modulate(float* buffer,int buffer_size,char byte)
{
  int mask = 1;
  for(int i=0;i<buffer_size;++i)
  {
    buffer[i] = 0;
    for(int j=0;j<carrier_angles_.size()-2;++j)
    {
      auto bit = IsBitZero(byte, mask, j) ? ZERO : ONE;
      UpdateAngle(j, bit);;
      buffer[i] += float(std::sin(carrier_angles_[j].current_angle)) / static_cast<float>(CARRIERS_NUMBER);
      carrier_angles_[j].current_angle += carrier_angles_[j].delta_angle;
      
    }
  }
}
/*
 * Modulates the int16 buffer
 */
void AudioModulator::ModulateInt16(int* buffer, int buffer_size, char byte)
{
  static const int int16_max = 16384;
  int mask = 1;
  for (int i = 0; i<buffer_size; ++i)
  {
    buffer[i] = 0;
    for (int j = 0; j<carrier_angles_.size() - 2; ++j)
    {
      auto bit = IsBitZero(byte, mask, j) ? ZERO : ONE;
      UpdateAngle(j, bit);
      buffer[i] += (float(int16_max)*(std::sin(carrier_angles_[j].current_angle))) / static_cast<float>(CARRIERS_NUMBER);
      carrier_angles_[j].current_angle += carrier_angles_[j].delta_angle;

    }
  }
}

void AudioModulator::UpdateSyncSignalAngle()
{
  carrier_angles_[CARRIERS_NUMBER].current_angle += carrier_angles_[CARRIERS_NUMBER].delta_angle;
  const auto cycles_per_sample = static_cast<double>(SYNC_FREQUENCY) / sample_rate_;
  carrier_angles_[CARRIERS_NUMBER].delta_angle = cycles_per_sample*2.0*PI;
}

void AudioModulator::UpdateStartSignalAngle()
{
  carrier_angles_[START_STOP_INDEX].current_angle += carrier_angles_[START_STOP_INDEX].delta_angle;
  const auto cycles_per_sample = static_cast<double>(START_STOP_FREQUENCY) / sample_rate_;
  carrier_angles_[START_STOP_INDEX].delta_angle = cycles_per_sample*2.0*PI;
}

void AudioModulator::SendSyncSignal(float* buffer, int buffer_size)
{
  for (int i = 0; i < buffer_size; ++i)
  {
    buffer[i] = float(std::sin(carrier_angles_[CARRIERS_NUMBER].current_angle));
    UpdateSyncSignalAngle();
  }
}

void AudioModulator::SendSyncSignalInt16(int* buffer, int buffer_size)
{
  auto pi = PI;
  static const int int16_max = 16000;
  for (int i = 0; i < buffer_size; ++i)
  {
    buffer[i] = int16_max*((std::sin(carrier_angles_[CARRIERS_NUMBER].current_angle)));//*(0.5-0.5*std::cos(2.0*i/(buffer_size-1)*pi)));
    UpdateSyncSignalAngle();
  }
}

void AudioModulator::SendStartSignalInt16(int* buffer, int buffer_size)
{
  auto pi = PI;
  static const int int16_max = 16000;
  for (int i = 0; i < buffer_size; ++i)
  {
    buffer[i] = int16_max*((std::sin(carrier_angles_[START_STOP_INDEX].current_angle)));//*(0.5-0.5*std::cos(2.0*i/(buffer_size-1)*pi)));
    UpdateStartSignalAngle();
  }
}

void AudioModulator::ResetAngles()
{
  for(auto& carrier : carrier_angles_)
  {
    carrier.current_angle = 0;
  }
}

void AudioModulator::UpdateAngle(int carrier_index, int bit)
{
  assert(sample_rate_ != 0);
  assert(bit == ONE || bit == ZERO);
  assert(carrier_index <= CARRIERS_NUMBER);
 
    const auto cycles_per_sample = static_cast<double>(frequency_table[2*carrier_index+ bit]) / sample_rate_;
    carrier_angles_[carrier_index].delta_angle = cycles_per_sample*2.0*PI;
  
}

