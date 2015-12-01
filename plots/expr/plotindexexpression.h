//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2013
//
#ifndef PLOTINDEXEXPRESSION_H
#define PLOTINDEXEXPRESSION_H

#include "plotsconfig.h"

#include "plotbinaryoperator.h"

/// @ingroup expr
/// An expression to index into an expression. This essentially uses the results from
/// one expression as the sample time for another. @c PlotBinaryOperator is derived
/// for convenience and the @c right expression makes the index or sample value, while
/// the @c left expression is the indexed expression.
///
/// Out of range sample times are returned as a zero value.
class PlotIndexExpression : public PlotBinaryOperator
{
public:
  /// Creates an index expression using @p indexer to index into @p indexee.
  /// @param indexee Expression to index into. Stored as @c left().
  /// @param indexer Expression to calculate the index. Stored as @c right().
  inline PlotIndexExpression(PlotExpression *indexee, PlotExpression *indexer)
    : PlotBinaryOperator(indexee, indexer)
  {}

  /// Samples the index expression for @p sampleTime. This resolves the value
  /// of @p right() using @p sampleTime and uses the result as a sample time
  /// into @p left().
  /// @param sampleTime The sample time to for @c right().
  /// @return A sample, calculated as:
  /// <code>left()->sample(right()->sample(sampleTime))</code>
  virtual double sample(double sampleTime) const;

  /// Clones this expression.
  /// @return A deep copy of this expression.
  virtual PlotExpression *clone() const;

private:
  /// Writes this expression back to string.
  /// @return A string formated as:
  /// <verbatim><left-string>[<right-string>]</endverbatim>
  virtual QString stringExpression() const;
};

#endif // PLOTINDEXEXPRESSION_H
