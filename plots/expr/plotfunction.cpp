//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2013
//
#include "plotfunction.h"

#include "functiondefinition.h"
#include "plotbindinfo.h"
#include "plotbindingtracker.h"

#include <QTextStream>


PlotFunction::PlotFunction(const FunctionDefinition *function, const QVector<PlotExpression *> &args)
  : _args(args)
  , _function(function)
  , _functionContext(nullptr)
{
}


PlotFunction::~PlotFunction()
{
  foreach (PlotExpression *e, _args)
  {
    delete e;
  }
}


BindResult PlotFunction::bind(const QList<PlotInstance *> &curves, PlotBindingTracker &bindTracker, PlotExpressionBindDomain &info, bool repeatLastBinding)
{
  _functionContext = _function->createContext();

  if (_args.empty())
  {
    // No args expected.
    return Bound;
  }

  _info.clear();
  info.setUnbounded();

  repeatLastBinding = repeatLastBinding || bindTracker.isHeld(this);
  std::vector<BindInfo> bindings(_args.count());
  for (int i = 0; i < _args.count(); ++i)
  {
    bindings[i].expression = _args[i];
  }

  BindResult bindRes;
  if (bindings.size() > 1)
  {
    bindRes = binding::bindMultiple(curves, bindTracker, bindings[0], &bindings[1], unsigned(bindings.size() - 1u), repeatLastBinding || bindTracker.isHeld(this));
  }
  else
  {
    bindRes = bindings[0].expression->bind(curves, bindTracker, bindings[0].domain, repeatLastBinding || bindTracker.isHeld(this));
  }

  if (bindRes > 0)
  {
    for (int i = 0; i < _args.count(); ++i)
    {
      domainUnion(info, bindings[i].domain);
    }
  }

  return bindRes;
}


void PlotFunction::unbind()
{
  _function->destroyContext(_functionContext);
  foreach (PlotExpression *e, _args)
  {
    e->unbind();
  }
}


double PlotFunction::sample(double sampleTime) const
{
  unsigned argc = unsigned(_args.count());
  if (argc && _function)
  {
    double *argv = (double *)alloca(sizeof(double) * argc);
    for (unsigned i = 0; i < argc; ++i)
    {
      argv[i] = _args[i]->sample(sampleTime);
    }
    PlotFunctionResult res;
    _function->evaluate(res, sampleTime, argc, argv, _info, _functionContext);
    _info.lastTime = sampleTime;
    _info.lastValue = res;
    _info.total += res.logicalValue;
    ++_info.count;
    return res.displayValue;
  }
  return 0;
}


QVector<PlotExpression *> PlotFunction::cloneArgs() const
{
  QVector<PlotExpression *> args;
  foreach (PlotExpression *e, _args)
  {
    args.push_back(e->clone());
  }
  return args;
}


PlotExpression *PlotFunction::clone() const
{
  return new PlotFunction(_function, cloneArgs());
}


QString PlotFunction::stringExpression() const
{
  QString str;
  QTextStream stream(&str);
  stream << _function->name() << '(';
  if (!_args.empty())
  {
    stream << _args.front()->toString();
    for (int i = 1; i < _args.size(); ++i)
    {
      stream << ", " << _args[i]->toString();
    }
  }
  stream << ')';
  return str;
}
