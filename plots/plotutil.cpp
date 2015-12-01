//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#include "plotutil.h"

#include <limits>

namespace plotutil
{
  double filter(double value, double filterResult, bool zeroInf, bool zeroNaN)
  {
    typedef std::numeric_limits<decltype(value)> Limits;

    if (value != value)
    {
      // NaN
      return (zeroNaN) ? 0.0 : filterResult;
    }
    else if (value == Limits::infinity() || value == -Limits::infinity())
    {
      // Infinite
      return (zeroInf) ? 0.0 : filterResult;
    }

    return value;
  }
}
