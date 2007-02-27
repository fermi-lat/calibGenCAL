#ifndef singlex16_h
#define singlex16_h

// $Header $

/** @file Specification of online 'singlex16' test scriptd data
   @author fewtrell
 */

// LOCAL INCLUDES

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"

// EXTLIB INCLUDES

// STD INCLUDES

namespace singlex16 {
  static const float CIDAC_TEST_VALS[] =
  {
    0,    2,     4,      6,      8,      10,      12,      14,      16,      18,      20,     22,    24,    26,    28,
    30,
    32,
    34,   36,    38,     40,     42,     44,      46,      48,      50,      52,      54,     56,    58,    60,    62,
    64,
    80,   96,    112,    128,    144,    160,     176,     192,     208,     224,     240,    256,   272,   288,
    304,  320,   336,    352,    368,    384,     400,     416,     432,     448,     464,    480,   496,   512,
    543,  575,   607,    639,    671,    703,     735,     767,     799,     831,     863,    895,   927,   959,
    991,  1023,  1055,   1087,   1119,   1151,    1183,    1215,    1247,    1279,
    1311, 1343,  1375,   1407,   1439,   1471,    1503,    1535,    1567,    1599,
    1631, 1663,  1695,   1727,   1759,   1791,    1823,    1855,    1887,    1919,
    1951, 1983,  2015,   2047,   2079,   2111,    2143,    2175,    2207,    2239,
    2271, 2303,  2335,   2367,   2399,   2431,    2463,    2495,    2527,    2559,
    2591, 2623,  2655,   2687,   2719,   2751,    2783,    2815,    2847,    2879,
    2911, 2943,  2975,   3007,   3039,   3071,    3103,    3135,    3167,    3199,
    3231, 3263,  3295,   3327,   3359,   3391,    3423,    3455,    3487,    3519,
    3551, 3583,  3615,   3647,   3679,   3711,    3743,    3775,    3807,    3839,
    3871, 3903,  3935,   3967,   3999,   4031,    4063,    4095
  };

  /// number of CIDAC values tested
  static const unsigned short N_CIDAC_VALS           = sizeof(CIDAC_TEST_VALS)/sizeof(*CIDAC_TEST_VALS);
  /// n pulses (events) per CIDAC val
  static const unsigned short N_PULSES_PER_DAC       = 50;
  /// n total pulsees per xtal (or column)
  static const unsigned N_PULSES_PER_XTAL      = N_CIDAC_VALS * N_PULSES_PER_DAC;

  /// total number of pulses in broadcast mode singlex16
  static const unsigned TOTAL_PULSES_BCAST     = N_PULSES_PER_XTAL;

  /// total number of pulses in column-wise singlex16
  static const unsigned TOTAL_PULSES_COLWISE   = N_PULSES_PER_XTAL*CalUtil::ColNum::N_VALS;
};

#endif
