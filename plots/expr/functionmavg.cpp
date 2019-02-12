//
// author
//
// Copyright (c) CSIRO 2015
//
#include "functionmavg.h"

#include "plotfunctionresult.h"

FunctionMAvg::FunctionMAvg(const QString &category)
  : FunctionDefinition(category, "mavg", 2)
{
  setDisplayName("mavg(x,window)");
  setDesciption("Moving average of x. The 'window' specifies the number of previous samples to retain.");
}


void FunctionMAvg::evaluate(PlotFunctionResult &result, double time, unsigned int /*argc*/, const double *argv, const PlotFunctionInfo &/*info*/, void *contextPtr) const
{
  Context &context = *static_cast<Context *>(contextPtr);
  result = addWindowAndAverage(time, argv[0], argv[1], context);
}


void *FunctionMAvg::createContext() const
{
  return new Context;
}


void FunctionMAvg::destroyContext(void *context) const
{
  delete static_cast<Context *>(context);
}


double FunctionMAvg::addWindowAndAverage(double time, double value, double windowSizeValue, Context &context)
{
  unsigned windowSize = unsigned(windowSizeValue >= 0 ? windowSizeValue : 0);
  unsigned currCount = context.window.count();

  // Pop old values.
  while (currCount > windowSize)
  {
    context.window.pop_front();
    --currCount;
  }

  // Push new value. Handle NaN and infinite, replacing with zero.
  value = (value == value && !std::isinf(value)) ? value : 0;
  context.window.push_back(QPointF(time, value));

  context.total = 0;
  // FIXME: include the time delta in the average calculation (weighted average).
  for (const QPointF &val : context.window)
  {
    context.total += val.y();
  }

  return context.total / context.window.count();
}
