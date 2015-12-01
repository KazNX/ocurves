//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#include "functionunwrap.h"

#include "plotfunctioninfo.h"

#include <QString>

#include <cmath>

#define DEFAULT_TOLERANCE 0.75

FunctionUnwrap::FunctionUnwrap(const QString &category)
  : FunctionDefinition(category, "unwrap", 3, true)
{
  setDisplayName(QString("unwrap(x,min,max,tolerance=%1)").arg(DEFAULT_TOLERANCE));
}


void FunctionUnwrap::evaluate(PlotFunctionResult &result, double time, unsigned int argc, const double *argv, const PlotFunctionInfo &info, void *context) const
{
  const double rangeMin = argv[1];
  const double rangeMax = argv[2];
  const double range = rangeMax - rangeMin;
  const double tolerance = (argc > 3) ? argv[3] : DEFAULT_TOLERANCE;

  result.logicalValue = argv[0];
  double delta = argv[0] - info.lastValue.logicalValue;
  if (std::abs(delta) > tolerance * range)
  {
    // Big change. We need to wrap it.
    if (delta < 0)
    {
      delta = rangeMax - info.lastValue.logicalValue + argv[0] - rangeMin;
    }
    else
    {
      delta = rangeMin - info.lastValue.logicalValue + argv[0] - rangeMax;
    }
  }

  result.displayValue = info.lastValue.displayValue + delta;
}
