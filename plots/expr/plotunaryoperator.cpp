//
// author
//
// Copyright (c) CSIRO 2015
//
#include "plotunaryoperator.h"

#include "plotbindingtracker.h"

PlotUnaryOperator::~PlotUnaryOperator()
{
  delete _operand;
}


BindResult PlotUnaryOperator::bind(const QList<PlotInstance*> &curves, PlotBindingTracker &bindTracker, PlotExpressionBindDomain &domain, bool repeatLastBinding)
{
  if (_operand)
  {
    repeatLastBinding = repeatLastBinding || bindTracker.isHeld(this);
    return _operand->bind(curves, bindTracker, domain, repeatLastBinding);
  }

  return BindError;
}


void PlotUnaryOperator::unbind()
{
  if (_operand)
  {
    _operand->unbind();
  }
}
