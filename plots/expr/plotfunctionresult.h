//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef PLOTFUNCTIONRESULT_H_
#define PLOTFUNCTIONRESULT_H_

#include "plotsconfig.h"

/// Result value from a @c PlotFunction.
///
/// This structure allows a distinction between the value displayed by the function
/// - the @c displayValue - and that used in future evaluations - the @c logicalValue.
///
/// The @c displayValue is what is returned in as the current sample value. The logical
/// can prevent bad results in certain calculates, such as the @c total() function.
/// Actual use depends on the function implementation. In most cases, the
/// @c logicalValue should be treated as authoritative.
///
/// Note that the @c logicalValue is used in calculating @c PlotFunctionInfo::total.
///
/// The results of evaluation are generally assigned using the assignment operator.
/// This writes to both the display and logical value. In some cases, the logical value
/// may mark an invalid result (NaN or infinite), in which case a different display
/// value may be written (e.g., 0).
struct PlotFunctionResult
{
  double displayValue;  ///< Value to display.
  double logicalValue;  ///< Value referenced in further evaluations of the function.

  /// Constructor - both display and logical values are set to @p value.
  /// @param value The value to set.
  inline PlotFunctionResult(double value = 0) : displayValue(value), logicalValue(value) {}

  /// Constructor accepting distinct display and logical values.
  /// @param display The @c displayValue to set.
  /// @param logical The @c logicalValue to set.
  inline PlotFunctionResult(double display, double logical) : displayValue(display), logicalValue(logical) {}

  /// Copy constructor.
  /// @param other The object to copy values from.
  inline PlotFunctionResult(const PlotFunctionResult &other)
  {
    *this = other;
  }

  /// Assignment - both display and logical values are set to @p value.
  /// @param value The value to set.
  inline PlotFunctionResult &operator=(double value)
  {
    displayValue = logicalValue = value;
    return *this;
  }

  /// Assignment - copy.
  /// @param other The object to copy values from.
  inline PlotFunctionResult &operator=(const PlotFunctionResult &other)
  {
    displayValue = other.displayValue;
    logicalValue = other.logicalValue;
    return *this;
  }
};

#endif // PLOTFUNCTIONRESULT_H_
