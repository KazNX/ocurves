//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2013
//
#ifndef PLOTBRACKETEXPRESSION_H
#define PLOTBRACKETEXPRESSION_H

#include "plotsconfig.h"

#include "plotunaryoperator.h"

/// @ingroup expr
/// A unary bracket operator, ensuring operation precedence.
class PlotBracketExpression : public PlotUnaryOperator
{
public:
  /// Constructor, wrapping @p operand in brackets.
  /// @param operand Operand to wrap in brackets.
  inline PlotBracketExpression(PlotExpression *operand) : PlotUnaryOperator(operand) {}

  /// Samples the bracketed expression.
  /// @param sampleTime Time at which to sample.
  /// @return The sample of @p operand() at time @p sampleTime.
  virtual double sample(double sampleTime) const;

  /// Clones this expression.
  /// @return A deep copy of this expression.
  virtual PlotExpression *clone() const;

private:
  /// Returns a bracketed version of the operand string.
  /// @return A bracketed expression string.
  virtual QString stringExpression() const;
};

#endif // PLOTBRACKETEXPRESSION_H
