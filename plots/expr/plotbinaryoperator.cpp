//
// author
//
// Copyright (c) CSIRO 2015
//
#include "plotbinaryoperator.h"

#include "plotbindinfo.h"
#include "plotbindingtracker.h"


PlotBinaryOperator::~PlotBinaryOperator()
{
  delete _left;
  delete _right;
}


BindResult PlotBinaryOperator::bind(const QList<PlotInstance *> &curves, PlotBindingTracker &bindTracker, PlotExpressionBindDomain &domain, bool repeatLastBinding)
{
  if (!_left || !_right)
  {
    return BindError;
  }

  BindInfo bindings[2];
  bindings[0].expression = _left;
  bindings[1].expression = _right;

  repeatLastBinding = repeatLastBinding || bindTracker.isHeld(this);
  BindResult bindResult = binding::bindMultiple(curves, bindTracker, bindings[0], &bindings[1], 1, repeatLastBinding);
  domainUnion(domain, bindings[0].domain, bindings[1].domain);
  return bindResult;
}



void PlotBinaryOperator::unbind()
{
  if (_left)
  {
    _left->unbind();
  }

  if (_right)
  {
    _right->unbind();
  }
}


bool PlotBinaryOperator::explicitTime() const
{
  if ((_left && _left->explicitTime()) || (_right && _right->explicitTime()))
  {
    return true;
  }

  return false;
}
