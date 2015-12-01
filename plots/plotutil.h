//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef PLOTUTIL_H_
#define PLOTUTIL_H_

#include "plotsconfig.h"

/// @ingroup plot
/// Utility functions for sampling and filtering plots and expression evaluation.
namespace plotutil
{
  /// Filters @p value for infinite and @c NaN results, returning a valid result instead.
  ///
  /// Invalid values are either returned as zero or as @c filterResult.
  /// Infinite values are converted according to @c zeroInf, while NaN values are
  /// converted according to @c zeroNaN. The result is zero when an invalid value
  /// is found and the respective control parameter is @c true. The result is
  /// @c filterResult when an invalid value is found and the control parameter is @c false.
  ///
  /// @param value The value to filter.
  /// @param filterResult The value to return when @c value is invalid and the
  ///   respective control parameter is @c false.
  /// @param zeroInf True to replace infinite values with zero. False to replace with
  ///   @c filterResult.
  /// @param zeroNaN True to replace NaN values with zero. False to replace with
  ///   @c filterResult.
  /// @return @c value when it is neither infinite nor NaN. Otherwise zero or
  ///   @c filterResult are returned.
  double filter(double value, double filterResult, bool zeroInf, bool zeroNaN);
}

#endif // PLOTUTIL_H_
