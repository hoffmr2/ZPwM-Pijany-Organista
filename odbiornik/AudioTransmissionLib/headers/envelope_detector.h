#pragma once
#ifndef ENVELOPE_DECTECTOR_H_
#define ENVELOPE_DETECTOR

#define ALPHA_MAX 1.0
#define ALPHA_MIN 0.0

class EnvelopeDetector
{
public:
  EnvelopeDetector();
  EnvelopeDetector(double alpha);
  ~EnvelopeDetector();

  void SetAlpha(double alpha) { alpha_ = alpha; }
  void ClearMemory();
  double GetEnvelopeValue(double sample);

private:
  double alpha_; //filter parameter from 0 to 1
  double memory_;
};

#endif

