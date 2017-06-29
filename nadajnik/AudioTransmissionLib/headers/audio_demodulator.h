/*
  ==============================================================================

    audio_demodulator.h
    Created: 22 Apr 2017 10:49:18pm
    Author:  MHofffmann

  ==============================================================================
*/

#ifndef AUDIO_DEMODULATOR_H_INCLUDED
#define AUDIO_DEMODULATOR_H_INCLUDED

#include "carrier_signals.h"
#include "digital_filters.h"
#include "envelope_detector.h"
#include <array>
#include <vector>
#include <fstream>

#define Q_FACTOR 0.05
#define MIN_VALUE 0.00001
#define BANDWIDTH 25
#define SYNC_BANDWIDTH 80
#define STOP_BAND 600
#define MIN_SYNC_VALUE 0.004

//0.00025
#define MIN_SYNC_TIME 1500
//700
//2000

/*
 * sync values
 */
//#define ALPHA 0.00045
#define ALPHA 0.001
#define TRESHOLD_HIGH 0.0022//0.0022
#define TRESHOLD_LOW 0.0019//0.0019
#define CLOCK_STATE_LOW 0
#define CLOCK_STATE_HIGH 1


enum State
{
  NonSync,
  SyncInProgress,
  WaitingTimeout,
  Receiving,
  EndingTransmission
};


class AudioDemodulator
{
public:
  AudioDemodulator();
  ~AudioDemodulator();


  char Demodulate(float* buffer, int buffer_size);
  void InitFilters();
  void InitFilters(double sample_rate);
  void SetSampleRate(double sample_rate);
  void SaveFile();

  int inline GetClockOrStartState()
   { return (clock_state_ == CLOCK_STATE_HIGH || start_state_ == CLOCK_STATE_HIGH) ? CLOCK_STATE_HIGH : CLOCK_STATE_LOW; }
private:
  std::vector<char> file_buffer_;
  std::vector<double> clock_;
  std::fstream file_;
  State state_;
  bool sync_flag_;
  bool is_sync_in_progress;
  int sync_timer;
  double sample_rate_;
  std::string text_to_write;
  std::array<HoffFilters::FirPolyphaseBandpassFilter*, FREQUENCY_NUMBER> band_pass_filters_;
  std::array<EnvelopeDetector*, FREQUENCY_NUMBER+2> envelope_detectors_;
  HoffFilters::FirPolyphaseBandpassFilter* sync_filter_;
  HoffFilters::FirPolyphaseBandpassFilter* start_filter_;

  bool start_received_ = false;
  double accumulators_[FREQUENCY_NUMBER]{ 0 };
  int clock_state_ = 0;
  int start_state_ = 0;
  int prev_clock_state_ = 0;
  int prev_start_state_ = 0;


  void InitEnvelopeDetectors();
  void DeleteFilters();
  void DeleteEnvelopeDetectors();
  void DemodulateByte(char& byte);
  void ReceiveStartStopSignal(char byte, double sample);
  void ReceiveData(float* buffer, char byte, double& sample, int i);
  void GetByte(char& byte, int bit);
};



#endif  // AUDIO_DEMODULATOR_H_INCLUDED
