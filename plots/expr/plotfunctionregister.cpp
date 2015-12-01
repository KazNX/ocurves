//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2013
//
#include "plotfunctionregister.h"

#include "functionclean.h"
#include "functionmavg.h"
#include "functionunwrap.h"
#include "plotexpression.h"

#include <cmath>

namespace
{
  // Function implementations.
  void atan2Func(PlotFunctionResult &res, double /*time*/, unsigned int /*argc*/, const double *argv, const PlotFunctionInfo &/*prog*/)
  {
    res = std::atan2(argv[0], argv[1]);
  }


  void abserrFunc(PlotFunctionResult &res, double /*time*/, unsigned int /*argc*/, const double *argv, const PlotFunctionInfo &/*prog*/)
  {
    res = std::abs(argv[0] - argv[1]);
  }


  void relerrFunc(PlotFunctionResult &res, double /*time*/, unsigned int /*argc*/, const double *argv, const PlotFunctionInfo &/*prog*/)
  {
    const double a = argv[0];
    const double b = argv[1];
    double error = a - b;
    error = (b) ? error / b : error;
    error = (error >= 0) ? error : -error;
    res = error;
  }


  void nowFunc(PlotFunctionResult &res, double time, unsigned int /*argc*/, const double * /*argv*/, const PlotFunctionInfo &/*prog*/)
  {
    res = PlotFunctionResult(time);
  }


  void totalFunc(PlotFunctionResult &res, double /*time*/, unsigned int /*argc*/, const double *argv, const PlotFunctionInfo &prog)
  {
    // Show current + last value. Add only current to total.
    res = argv[0] + prog.lastValue.logicalValue;
  }


  void avgFunc(PlotFunctionResult &res, double /*time*/, unsigned int /*argc*/, const double *argv, const PlotFunctionInfo &prog)
  {
    // Show (current + total) / count. Add only current to total.
    res = argv[0] + prog.total / double(prog.count + 1u);
  }


  void deltaFunc(PlotFunctionResult &res, double /*time*/, unsigned int /*argc*/, const double *argv, const PlotFunctionInfo &prog)
  {
    // Show current from last logical. Track current.
    res = PlotFunctionResult(argv[0] - prog.lastValue.logicalValue, argv[0]);
  }


  void clipFunc(PlotFunctionResult &res, double /*time*/, unsigned int /*argc*/, const double *argv, const PlotFunctionInfo &prog)
  {
    // Bound to a range.
    const double &val = argv[0];
    const double &minclip = argv[1];
    const double &maxclip = argv[2];
    res = std::max(minclip, std::min(val, maxclip));
  }


  void clipmaxFunc(PlotFunctionResult &res, double /*time*/, unsigned int /*argc*/, const double *argv, const PlotFunctionInfo &prog)
  {
    // Bound to a maximum value.
    const double &val = argv[0];
    const double &maxclip = argv[1];
    res = std::min(val, maxclip);
  }


  void clipminFunc(PlotFunctionResult &res, double /*time*/, unsigned int /*argc*/, const double *argv, const PlotFunctionInfo &prog)
  {
    // Bound to a minimum value.
    const double &val = argv[0];
    const double &minclip = argv[1];
    res = std::max(val, minclip);
  }


  void maxofFunc(PlotFunctionResult &res, double /*time*/, unsigned int argc, const double *argv, const PlotFunctionInfo &/*prog*/)
  {
    if (!argc)
    {
      res = 0;
      return;
    }

    double mx = argv[0];
    for (unsigned int i = 1; i < argc; ++i)
    {
      mx = std::max(mx, argv[i]);
    }
    res = mx;
  }


  void minofFunc(PlotFunctionResult &res, double /*time*/, unsigned int argc, const double *argv, const PlotFunctionInfo &/*prog*/)
  {
    if (!argc)
    {
      res = 0;
      return;
    }

    double mn = argv[0];
    for (unsigned int i = 1; i < argc; ++i)
    {
      mn = std::min(mn, argv[i]);
    }
    res = mn;
  }


  void maxFunc(PlotFunctionResult &res, double /*time*/, unsigned int /*argc*/, const double *argv, const PlotFunctionInfo &prog)
  {
    if (prog.count == 0)
    {
      res = argv[0];
      return;
    }

    res = std::max(argv[0], prog.lastValue.logicalValue);
  }


  void minFunc(PlotFunctionResult &res, double /*time*/, unsigned int /*argc*/, const double *argv, const PlotFunctionInfo &prog)
  {
    if (prog.count == 0)
    {
      res = argv[0];
      return;
    }

    res = std::min(argv[0], prog.lastValue.logicalValue);
  }


  void fmodFunc(PlotFunctionResult &res, double /*time*/, unsigned int /*argc*/, const double *argv, const PlotFunctionInfo &prog)
  {
    res = std::fmod(argv[0], argv[1]);
  }


  void remainderFunc(PlotFunctionResult &res, double /*time*/, unsigned int /*argc*/, const double *argv, const PlotFunctionInfo &prog)
  {
    res = std::remainder(argv[0], argv[1]);
  }


  void isNanFunc(PlotFunctionResult &res, double /*time*/, unsigned int /*argc*/, const double *argv, const PlotFunctionInfo &prog)
  {
    res = std::isnan(argv[0]);
  }


  void isInfFunc(PlotFunctionResult &res, double /*time*/, unsigned int /*argc*/, const double *argv, const PlotFunctionInfo &prog)
  {
    res = std::isinf(argv[0]);
  }


  void signFunc(PlotFunctionResult &res, double /*time*/, unsigned int /*argc*/, const double *argv, const PlotFunctionInfo &prog)
  {
    res = std::signbit(argv[0]) ? -1.0 : 1.0;
  }


  void piFunc(PlotFunctionResult &res, double /*time*/, unsigned int /*argc*/, const double *argv, const PlotFunctionInfo &prog)
  {
    res = M_PI;
  }
}


PlotFunctionRegister::PlotFunctionRegister(bool registerDefaults)
{
  if (registerDefaults)
  {
    registerDefaultFunctions();
  }
}


PlotFunctionRegister::~PlotFunctionRegister()
{
  auto diter = _definitions.begin();
  auto owniter = _ownership.begin();
  for (; owniter != _ownership.end() && diter != _definitions.end(); ++owniter, ++diter)
  {
    if (*owniter)
    {
      delete *diter;
    }
  }
}


FunctionSimple *PlotFunctionRegister::add(FunctionSimple::ValueFunctionPtr function, const QString &category, const QString &name, const QString &description)
{
  FunctionSimple *func = new FunctionSimple(FunctionSimple::ValueFunction(function), category, name, description);
  if (!add(func))
  {
    delete func;
    return nullptr;
  }

  return func;
}


FunctionSimple *PlotFunctionRegister::add(FunctionSimple::ExpandedFunctionPtr function, const QString &category, const QString &name, const QString &description, unsigned argc, bool variadic)
{
  FunctionSimple *func = new FunctionSimple(function, category, name, description, argc, variadic);
  if (!add(func))
  {
    delete func;
    return nullptr;
  }

  return func;
}


FunctionDefinition *PlotFunctionRegister::add(FunctionDefinition *functionDef, bool takeOwnership)
{
  if (!functionDef || find(functionDef->name()))
  {
    return nullptr;
  }

  _definitions.push_back(functionDef);
  _ownership.push_back(takeOwnership);
  ensureCategoryIsPresent(functionDef->category());
  return functionDef;
}


const FunctionDefinition *PlotFunctionRegister::find(const QString &name) const
{
  for (const FunctionDefinition *def : _definitions)
  {
    if (name.compare(def->name()) == 0)
    {
      return def;
    }
  }
  return nullptr;
}


void PlotFunctionRegister::registerDefaultFunctions()
{
  FunctionDefinition *def = nullptr;
  QString category;

  // Core fundamentals
  category = "general";
  add(static_cast<double (*)(double)>(&std::abs), category, "abs", "Absolute value of x.");
  add(new FunctionClean(category));
  def = add(&clipFunc, category, "clip", "Clip x to to the range [min,max]. All values are clipped to this range.", 3);
  def->setDisplayName("clip(x,min,max)");
  def = add(&clipmaxFunc, category, "clipmax", "Clip x to a maximum value. Any value above max is displayed as min.", 2);
  def->setDisplayName("clipmax(x,max)");
  def = add(&clipminFunc, category, "clipmin", "Clip x to a minimum value. Any value below min is displayed as min.", 2);
  def->setDisplayName("clipmin(x,min)");
  add(&deltaFunc, category, "delta", "Delta from the last value of x.", 1);
  add(static_cast<double (*)(double)>(&std::exp), category, "exp", "Calculate e (2.718...) to the power of x.");
  add(&isNanFunc, category, "isnan", "Test if the input value is NaN. 1 if so, zero otherwise.", 1);
  add(&isInfFunc, category, "isinf", "Test if the input value is +/- infinity. 1 if so, zero otherwise.", 1);
  add(static_cast<double(*)(double)>(&std::log), category, "log", "Natural logarithm of x.");
  add(static_cast<double(*)(double)>(&std::log10), category, "log10", "Base 10 logarithm of x.");
  add(&maxFunc, category, "max", "Most recent maximum value of x.", 1);
  add(&minFunc, category, "min", "Most recent minimum value of x.", 1);
  add(static_cast<double (*)(double)>(&std::sqrt), category, "sqrt", "Sqrt of the value of x.");
  add(&nowFunc, category, "now", "Returns the current sample time.", 0);
  add(new FunctionUnwrap(category));

  category = "rounding";
  add(static_cast<double(*)(double)>(&std::ceil), category, "ceil", "Nearest integer not less than x.");
  add(static_cast<double(*)(double)>(&std::floor), category, "floor", "Nearest integer not greater than x.");
  add(static_cast<double(*)(double)>(&std::trunc), category, "trunc", "Nearest integer not greater in magnitude than x.");
  add(static_cast<double(*)(double)>(&std::round), category, "round", "Nearest integer rounding up (in magnitude) from halfway cases.");
  add(&fmodFunc, category, "fmod", "Floating point remainder of the division x/y.", 2);
  add(&remainderFunc, category, "remainder", "Integer remainder (closest) of the division x/y. Sign may differ from fmod.", 2);
  add(&signFunc, category, "sign", "Return the sign of x. -1 if negative, 1 otherwise.", 1);

  // Statistics
  category = "statistics";
  add(&abserrFunc, category, "abserr", "Absolute error between x and y.", 2);
  add(&avgFunc, category, "avg", "Running average value of x.", 1);
  add(new FunctionMAvg(category));
  add(&maxofFunc, category, "maxof", "Maximum value of any number of graphs.", 1, true);
  add(&minofFunc, category, "minof", "Minimum value of any number of graphs.", 1, true);
  add(&relerrFunc, category, "relerr", "Relative error between x and y.", 2);
  add(&totalFunc, category, "total", "Running sum of x.", 1);

  // Trigonometry
  category = "trigonometry";
  add(static_cast<double(*)(double)>(&std::acos), category, "acos", "Trigonometric arccos function of x");
  add(static_cast<double(*)(double)>(&std::asin), category, "asin", "Trigonometric arcsin function of x");
  add(static_cast<double(*)(double)>(&std::atan), category, "atan", "Trigonometric arctan function of x");
  def = add(&atan2Func, category, "atan2", "Trigonometric artan of y/x with quandrant selection.");
  if (def)
  {
    def->setDisplayName("atan2(y,x)");
  }
  add(static_cast<double(*)(double)>(&std::cos), category, "cos", "Trigonometric cos function of x");
  add(static_cast<double(*)(double)>(&std::sin), category, "sin", "Trigonometric sin function of x");
  add(static_cast<double(*)(double)>(&std::tan), category, "tan", "Trigonometric tan function of x");
  add(static_cast<double(*)(double)>(&std::cosh), category, "cosh", "Hyperbolic cos function of x");
  add(static_cast<double(*)(double)>(&std::sinh), category, "sinh", "Hyperbolic sin function of x");
  add(static_cast<double(*)(double)>(&std::tanh), category, "tanh", "Hyperbolic tan function of x");
  add(static_cast<double(*)(double)>(&std::acosh), category, "acosh", "Hyperbolic arccos function of x");
  add(static_cast<double(*)(double)>(&std::asinh), category, "asinh", "Hyperbolic arcsin function of x");
  add(static_cast<double(*)(double)>(&std::atanh), category, "atanh", "Hyperbolic arctan function of x");
  add(&piFunc, category, "pi", "Pi constant (3.141...).", 0);
}


unsigned PlotFunctionRegister::getDefinitionsInCategory(const QString &categoryName, QVector<const FunctionDefinition *> &definitions) const
{
  unsigned added = 0;
  for (const FunctionDefinition *def : _definitions)
  {
    if (def->category().compare(categoryName) == 0)
    {
      definitions.push_back(def);
      ++added;
    }
  }
  return added;
}


void PlotFunctionRegister::ensureCategoryIsPresent(const QString &categoryName)
{
  for (const QString &cat : _categories)
  {
    if (cat.compare(categoryName) == 0)
    {
      return;
    }
  }

  _categories.push_back(categoryName);
}
