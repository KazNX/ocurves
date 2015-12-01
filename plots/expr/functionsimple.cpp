//
// author
//
// Copyright (c) CSIRO 2015
//
#include "functionsimple.h"

#include "plotfunctioninfo.h"

FunctionSimple::FunctionSimple(const ValueFunction &func, const QString &category, const QString &name, const QString &description)
  : FunctionDefinition(category, name, description, 1, false)
{
  _function = [func](PlotFunctionResult & result, double time, unsigned int argc, const double * argv, const PlotFunctionInfo & info)
  {
    result = func(argv[0]);
  };
}


FunctionSimple::FunctionSimple(const ExpandedFunction &ptr, const QString &category, const QString &name, unsigned argc, bool variadic)
  : FunctionDefinition(category, name, argc, variadic)
  , _function(ptr)
{
}


FunctionSimple::FunctionSimple(const ExpandedFunction &func, const QString &category, const QString &name, const QString &description, unsigned argc, bool variadic)
  : FunctionDefinition(category, name, description, argc, variadic)
  , _function(func)
{
}


void FunctionSimple::evaluate(PlotFunctionResult &result, double time, unsigned int argc, const double *argv, const PlotFunctionInfo &info, void * /*context*/) const
{
  _function(result, time, argc, argv, info);
}
