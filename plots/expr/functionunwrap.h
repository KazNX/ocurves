//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef FUNCTIONUNWRAP_H_
#define FUNCTIONUNWRAP_H_

#include "plotsconfig.h"

#include "functiondefinition.h"

/// @ingroup expr
/// A function designed to unwrap angles, but capable of unwrapping any range.
///
/// Unwrap turns wrapped plots into continuous plots. For example, consider
/// a rotation in the range [-180, 180] degrees. There can be discontinuities
/// rotating through 180, jumping down to -180. Unwrap converts this into a
/// continuous plot.
///
/// The @c PlotFunctionResult::logicalValue is used to hold the original, wrapped values,
/// while @c PlotFunctionResult::displayValue is unwrapped.
class FunctionUnwrap : public FunctionDefinition
{
public:
  /// Constructor.
  /// @param category The sorting category for this function.
  FunctionUnwrap(const QString &category = QString());

  /// Evaluation.
  void evaluate(PlotFunctionResult &result, double time, unsigned int argc, const double *argv, const PlotFunctionInfo &info, void *context) const override;
};

#endif // FUNCTIONUNWRAP_H_
