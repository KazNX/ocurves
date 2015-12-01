//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2013
//
#include "plotbracketexpression.h"

#include <QTextStream>

double PlotBracketExpression::sample(double sampleTime) const
{
  return operand()->sample(sampleTime);
}


PlotExpression *PlotBracketExpression::clone() const
{
  return new PlotBracketExpression(operand()->clone());
}


QString PlotBracketExpression::stringExpression() const
{
  QString str;
  QTextStream stream(&str);
  stream << '(' << operand()->toString() << ')';
  return str;
}
