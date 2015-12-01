//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef FUNCTIONCLEAN_H_
#define FUNCTIONCLEAN_H_

#include "plotsconfig.h"

#include "functiondefinition.h"

#include <QList>
#include <QPointF>

/// @ingroup expr
/// A function which filters out infinite and NaN values.
///
/// Requires only one argument, but uses up to three as follows:
/// @code
///   clean(x, zeroInf=1, zeroNaN=1)
/// @endcode
///
/// The @c zeroInf and @c zeroNaN arguments control how infinite and NaN
/// values are respectively handled. When zero, such values are set to
/// maintain continuity in the display curve, otherwise invalid values are
/// set to zero.
class FunctionClean : public FunctionDefinition
{
public:
  /// Constructor.
  FunctionClean(const QString &category = QString());

  /// Evaluates the function, filtering infinite and NaN values.
  ///
  /// The @c PlotFunctionInfo is used as follows:
  /// - @c PlotFunctionInfo::displayValue - always contains the filtered values.
  /// - @c PlotFunctionInfo::logicalValue - always contains the original, unfiltered values.
  ///
  /// @param[out] result The evaluation result is written here.
  /// @param time The sample time being evaluated for.
  /// @param argc The number of arguments actually given.
  /// @param argv The list of calculated arguments, length @p argc. These are always logical values.
  /// @param info Additional information about the current sampling.
  /// @param context Additional context of the operation. May hold working data.
  ///     This is optained via @c createContext().
  void evaluate(PlotFunctionResult &result, double time, unsigned int argc, const double *argv, const PlotFunctionInfo &info, void *context) const override;
};

#endif // FUNCTIONCLEAN_H_
