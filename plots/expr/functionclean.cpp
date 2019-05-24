//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#include "functionclean.h"

#include "plotfunctioninfo.h"
#include "plotfunctionresult.h"
#include "plotutil.h"

FunctionClean::FunctionClean(const QString &category)
  : FunctionDefinition(category, "clean", 1, true)
{
  setDesciption("Clean display of x by filtering out NaN and infinite values in x. Infinite and NaN values can be converted to zero or to "
                "the last display value depending on zeroInf and zeroNaN respectively. E.g., zeroInf=0 filters "
                "infinite values to the last display value while zeroInf=1 (any non-zero) filters to zero.");
  setDisplayName("clean(x[,zeroInf=0,zeroNaN=0])");
}


void FunctionClean::evaluate(PlotFunctionResult &result, double /*time*/, unsigned int argc, const double *argv, const PlotFunctionInfo &info, void * /*contextPtr*/) const
{
  const bool zeroInf = (argc > 1) ? (argv[1] != 0) : false;
  const bool zeroNaN = (argc > 2) ? (argv[2] != 0) : false;
  result = PlotFunctionResult(plotutil::filter(argv[0], info.lastValue.displayValue, zeroInf, zeroNaN), argv[0]);
}
