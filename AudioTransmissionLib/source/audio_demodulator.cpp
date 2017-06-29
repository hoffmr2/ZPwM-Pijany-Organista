/*
  ==============================================================================

    audio_demodulator.cpp
    Created: 22 Apr 2017 10:49:18pm
    Author:  MHofffmann

  ==============================================================================
*/

#include "audio_demodulator.h"
#include <thread>
#include "file_write_win.h"

AudioDemodulator::AudioDemodulator() : sync_flag_(false), sync_timer(0), sample_rate_(0), state_(Receiving)
{
  InitEnvelopeDetectors();
}

void AudioDemodulator::DeleteFilters()
{
  for(auto filter : band_pass_filters_)
  {
    if(filter != nullptr)
    {
      delete filter;
      filter = nullptr;
    }
  }
  if(sync_filter_ != nullptr)
  {
    delete sync_filter_;
    sync_filter_ = nullptr;
  }
}

void AudioDemodulator::DeleteEnvelopeDetectors()
{
  for (auto envelope_detector : envelope_detectors_)
  {
    if(envelope_detector != nullptr)
    {
      delete envelope_detector;
      envelope_detector = nullptr;
    }
  }
}

AudioDemodulator::~AudioDemodulator()
{

  DeleteFilters();
  DeleteEnvelopeDetectors();
}

void AudioDemodulator::GetByte(char& byte, int bit)
{
  for (int j = 0; j < FREQUENCY_NUMBER; j += 2)
  {
    if (accumulators_[j] > accumulators_[j + 1])
    {
      bit = ZERO;
    }
    else

    {
      bit = ONE;
    }
    byte += bit << (j / 2);
  }
}

void AudioDemodulator::DemodulateByte(char& byte)
{
  int bit = 0;


  for (int j = 0; j < band_pass_filters_.size(); ++j)
  {
    band_pass_filters_[j]->InitMemory();
    envelope_detectors_[j]->ClearMemory();
  }

  GetByte(byte, bit);
  file_buffer_.push_back(byte);
  for (auto j = 0; j < FREQUENCY_NUMBER; ++j)
    accumulators_[j] = 0;
}

void AudioDemodulator::ReceiveStartStopSignal(char byte, double sample)
{
  if (sample > TRESHOLD_HIGH)
  {
    start_state_ = CLOCK_STATE_HIGH;
  }
  else
  {
    if (sample < TRESHOLD_LOW)
      start_state_ = CLOCK_STATE_LOW;
    if (prev_start_state_ == CLOCK_STATE_HIGH && start_state_ == CLOCK_STATE_LOW)
    {
      if (!start_received_)
      {
        for (auto j = 0; j < FREQUENCY_NUMBER; ++j)
          accumulators_[j] = 0;
        start_received_ = true;
      }
      else
      {
        start_received_ = false;
        DemodulateByte(byte);
        SaveFile();
        file_buffer_.clear();
      }
    }
  }
}

void AudioDemodulator::ReceiveData(float* buffer, char byte, double& sample, int i)
{
  if (start_received_)
  {
    sample = sync_filter_->FilterOutputLeft(buffer[i]);
    sample = envelope_detectors_[FREQUENCY_NUMBER]->GetEnvelopeValue(sample);


    prev_clock_state_ = clock_state_;
    if (sample > TRESHOLD_HIGH)
    {
      clock_state_ = CLOCK_STATE_HIGH;
    }
    else
    {
      if (sample < TRESHOLD_LOW)
        clock_state_ = CLOCK_STATE_LOW;
      if (prev_clock_state_ == CLOCK_STATE_HIGH && clock_state_ == CLOCK_STATE_LOW)
      {

        DemodulateByte(byte);
      }
    }
  }
}

char AudioDemodulator::Demodulate(float* buffer, int buffer_size)
{
  char byte = 0;

  auto sample = 0.0;
  for (int i = 0; i < buffer_size; ++i)
  {
    prev_start_state_ = start_state_;
    sample = start_filter_->FilterOutputLeft(buffer[i]);
    sample = envelope_detectors_[FREQUENCY_NUMBER + 1]->GetEnvelopeValue(sample);

    ReceiveStartStopSignal(byte, sample);
    ReceiveData(buffer, byte, sample, i);
    for (int j = 0; j < band_pass_filters_.size(); ++j)
    {
      accumulators_[j] += envelope_detectors_[j]->GetEnvelopeValue(band_pass_filters_[j]->FilterOutputLeft(buffer[i]));
    }
  }
  if (state_ == Receiving)
  {
    int bit = 0;
    GetByte(byte, bit);
  }
  return byte;
}

  



void AudioDemodulator::InitFilters()
{
  assert(sample_rate_ > 0);
  for(int i=0;i<FREQUENCY_NUMBER;++i)
  {
      band_pass_filters_[i] = new HoffFilters::FirPolyphaseBandpassFilter(sample_rate_, frequency_table[i], frequency_table[i] - BANDWIDTH,
        frequency_table[i], frequency_table[i] + BANDWIDTH, 20,8);
  }

  sync_filter_ = new HoffFilters::FirPolyphaseBandpassFilter(sample_rate_, SYNC_FREQUENCY-10, SYNC_FREQUENCY - SYNC_BANDWIDTH,
    SYNC_FREQUENCY+10, SYNC_FREQUENCY + SYNC_BANDWIDTH, 20,8);

  start_filter_ = new HoffFilters::FirPolyphaseBandpassFilter(sample_rate_, START_STOP_FREQUENCY - 10, START_STOP_FREQUENCY - SYNC_BANDWIDTH,
    START_STOP_FREQUENCY + 10, START_STOP_FREQUENCY + SYNC_BANDWIDTH, 20, 8);
}

void AudioDemodulator::InitFilters(double sample_rate)
{
  SetSampleRate(sample_rate);
  InitFilters();
}

void AudioDemodulator::SetSampleRate(double sample_rate)
{
  assert(sample_rate > 0);
  sample_rate_ = sample_rate;
}

void AudioDemodulator::SaveFile()
{
  FileWriteWin* file_write_win = new FileWriteWin();
  file_write_win->SaveFile(&file_buffer_);
}

void AudioDemodulator::InitEnvelopeDetectors()
{
  for (auto i=0;i<envelope_detectors_.size();++i)
  {
    envelope_detectors_[i] = new EnvelopeDetector(ALPHA);
  }

}
