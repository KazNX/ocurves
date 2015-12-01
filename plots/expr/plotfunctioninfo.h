//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef PLOTFUNCTIONINFO_H_
#define PLOTFUNCTIONINFO_H_

#include "plotsconfig.h"

#include "plotfunctionresult.h"

/// @ingroup expr
/// Plot sampling progress information for @c PlotFunction. Tracks meta data about
/// the sampling which can be used in @c FunctionDefinition evaluation.
struct PlotFunctionInfo
{
  double lastTime;  ///< Time of the last sample.
  PlotFunctionResult lastValue; ///< Last value at time @c lastTime.
  double total; ///< Running total of the samples.
  unsigned int count; ///< Number of samples taken so far.
  void *context; ///< Function specific context data.

  /// Constructor.
  inline PlotFunctionInfo() { clear(); }

  /// Clear sampling info.
  inline void clear()
  {
    lastValue = lastTime = total = 0.0;
    count = 0u;
    context = nullptr;
  }
};

#endif // PLOTFUNCTIONINFO_H_
