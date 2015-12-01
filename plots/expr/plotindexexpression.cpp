//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2013
//
#include "plotindexexpression.h"

#include <QTextStream>

double PlotIndexExpression::sample(double sampleTime) const
{
  const double index = right()->sample(sampleTime);
  return left()->sample(index);
}


PlotExpression *PlotIndexExpression::clone() const
{
  return new PlotIndexExpression(left()->clone(), right()->clone());
}


QString PlotIndexExpression::stringExpression() const
{
  QString str;
  QTextStream stream(&str);
  stream << left()->toString() << '[' << right()->toString() << ']';
  return str;
}
