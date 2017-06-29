#include "envelope_detector.h"
#include <assert.h>
#include <cmath>


EnvelopeDetector::EnvelopeDetector() : alpha_(0), memory_(0)
{
}

EnvelopeDetector::EnvelopeDetector(double alpha) : alpha_(alpha),memory_(0)
{
}


EnvelopeDetector::~EnvelopeDetector()
{
}

void EnvelopeDetector::ClearMemory()
{
  memory_ = 0;
}

double EnvelopeDetector::GetEnvelopeValue(double sample)
{
  sample = std::abs(sample);
  assert(alpha_ >= ALPHA_MIN && alpha_ < ALPHA_MAX);
//  sample *= alpha_/2;
 // double out = (1 - alpha_)*memory_ + sample;

  double out = (sample + (1 - alpha_)*memory_);

  memory_ = out;
  return out*alpha_/2;
}
