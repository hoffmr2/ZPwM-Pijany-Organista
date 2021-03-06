#ifndef CARRIER_SIGNALS_H_
#define CARRIER_SIGNALS_H_

#define CARRIERS_NUMBER 8
#define SIGNALS_NUMBER 10
#define START_STOP_INDEX 9
#define FREQUENCY_NUMBER 16
#define ONE 1
#define ZERO 0

#define SYNC_FREQUENCY 440
#define START_STOP_FREQUENCY 659

#define ZERO_FREQUENCY_BIT0 6272//659//659
#define ONE_FREQUENCY_BIT0 7040//783

#define ZERO_FREQUENCY_BIT1 880
#define ONE_FREQUENCY_BIT1 1046

#define ZERO_FREQUENCY_BIT2 1174
#define ONE_FREQUENCY_BIT2 1318

#define ZERO_FREQUENCY_BIT3 1567
#define ONE_FREQUENCY_BIT3 1760

#define ZERO_FREQUENCY_BIT4 2093
#define ONE_FREQUENCY_BIT4 2349

#define ZERO_FREQUENCY_BIT5 2637
#define ONE_FREQUENCY_BIT5 3135

#define ZERO_FREQUENCY_BIT6 3520
#define ONE_FREQUENCY_BIT6 4186

#define ZERO_FREQUENCY_BIT7 4698
#define ONE_FREQUENCY_BIT7 5274

static const int frequency_table[FREQUENCY_NUMBER]{
  ZERO_FREQUENCY_BIT0,
  ONE_FREQUENCY_BIT0,
  ZERO_FREQUENCY_BIT1,
  ONE_FREQUENCY_BIT1,
  ZERO_FREQUENCY_BIT2,
  ONE_FREQUENCY_BIT2,
  ZERO_FREQUENCY_BIT3,
  ONE_FREQUENCY_BIT3,
  ZERO_FREQUENCY_BIT4,
  ONE_FREQUENCY_BIT4,
  ZERO_FREQUENCY_BIT5,
  ONE_FREQUENCY_BIT5,
  ZERO_FREQUENCY_BIT6,
  ONE_FREQUENCY_BIT6,
  ZERO_FREQUENCY_BIT7,
  ONE_FREQUENCY_BIT7,
};

#endif
