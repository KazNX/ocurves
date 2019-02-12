//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2013
//
#include "plotconstant.h"

#include <QTextStream>

double PlotConstant::sample(double /*sampleTime*/) const
{
  return _constant;
}


BindResult PlotConstant::bind(const QList<PlotInstance *> &/*curves*/, PlotBindingTracker &/*info*/, PlotExpressionBindDomain &domain, bool /*repeatLastBinding*/)
{
  domain.sampleCount = 1;
  return Bound;
}


PlotExpression *PlotConstant::clone() const
{
  return new PlotConstant(_constant);
}


QString PlotConstant::stringExpression() const
{
  QString str;
  QTextStream stream(&str);
  stream << _constant;
  return str;
}
