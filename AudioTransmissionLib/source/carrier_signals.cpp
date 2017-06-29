#include "carrier_signals.h"
#include <cassert>
/*
double pi() { return std::atan(1) * 4; }

CarrierSignals::CarrierSignals() : carriers_(CARRIERS_NUMBER,nullptr)
{
}


CarrierSignals::~CarrierSignals()
{
  for (auto carrier : carriers_)
  {
    if (carrier != nullptr)
      delete[] carrier;
  }
}

void CarrierSignals::SetBufferSize(int buffer_size)
{
  assert(buffer_size > 0);
  buffer_size_ = buffer_size;
}

void CarrierSignals::SetSampleRate(double sample_rate)
{
  assert(sample_rate > 0);
  sample_rate_ = sample_rate;
}

void CarrierSignals::InitCarriers(int buffer_size, double sample_rate)
{
  SetBufferSize(buffer_size);
  SetSampleRate(sample_rate);
  InitCarriers();
}

void CarrierSignals::CreateCarrier(int i)
{
  if(carriers_[i] != nullptr)
  {
    delete carriers_[i];
    carriers_[i] = nullptr;
  }
  carriers_[i] = new float[buffer_size_];
}

void CarrierSignals::InitCarriers()
{
  assert(buffer_size_ != 0);
  assert(sample_rate_ != 0);
  for (int i = 0; i<CARRIERS_NUMBER; ++i)
  {
    const auto cycles_per_sample = (start_carrier_frequency_ + i*step_carrier_frequency) / sample_rate_;
    delta_angles_[i] = cycles_per_sample*2.0 *pi();
    CreateCarrier(i);
    GenerateCarrier(i);
  }
}

void CarrierSignals::GenerateCarrier(int index)
{
  for (int j = 0; j < buffer_size_; ++j)
  {
    carriers_[index][j] = float(std::sin(current_angles_[index]));
    current_angles_[index] += delta_angles_[index];
  }
}
*/