//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef __PLOTEXPRESSIONBINDDOMAIN_H_
#define __PLOTEXPRESSIONBINDDOMAIN_H_

#include "plotsconfig.h"

#include <cstddef>

/// @ingroup expr
/// Return values for the @c PlotExpression::bind() method.
enum BindResult
{
  BindError = -1,   ///< Binding failed due to an error.
  BindFailure = 0,  ///< Binding failed due to a failure to match curves.
  Bound,            ///< Binding succeeded, no further attempts required.
  BoundMaybeMore    ///< Binding succeeded, multiple bindings may be possible.
};

/// @ingroup expr
/// Core data for binding a @c PlotExpression. This defines the sampling range.
///
/// This is populated by calls to @c PlotExpression::bind() and used to generate
/// the sample times passed to @c PlotExpression::sample().
///
/// The sampling loop is defined by the following pseudo code:
/// @verbatim
///   define info as PlotExpressionBindInfo
///   define expr as PlotExpression
///   for i = [0, info.sampleCount)     // Does not include info.sampleCount
///     set sampleTime = info.domainMin + i * info.sampleDelta
///     if sampleTime > info.domainMax
///       set sampleTime = info.domainMax
///     expr->sample(sampleTime)
/// @endverbatim
///
/// An unbounded domain can be defined by calling @c setUnbounded() or tested for
/// via @c isUnbounded(). For example, a constant value has an unbounded domain.
/// An unbounded domain has the following characteristics:
/// - @c domainMin, @c domainMax and @c domainDelta are zero (for simplicity).
/// - @c sampleCount is zero.
/// - A union with a bounded domain results in the bounded domain.
///
/// The test for an unbounded domain only examines the @c sampleCount and may yield
/// false positives.
struct PlotExpressionBindDomain
{
  double domainMin;   ///< The minimum value to be passed to @c PlotExpression::sample().
  double domainMax;   ///< The maximum value to be passed to @c PlotExpression::sample().
  double sampleDelta; ///< Time delta between calls to @c PlotExpression::sample().
  size_t sampleCount; ///< The number of samples to be generated.
  bool minSet;        ///< @c domainMin is set and relevant.
  bool maxSet;        ///< @c domainMax is set and relevant.

  /// Constructor. Creates an unbounded domain.
  inline PlotExpressionBindDomain() : domainMin(0), domainMax(0), sampleDelta(0), sampleCount(0), minSet(false), maxSet(false) {}

  /// Sets the domain to be unbounded.
  void setUnbounded();

  /// Returns true if the domain is unbounded.
  ///
  /// This only checks the @c sampleCount and may yield false positives.
  /// @return True if unbounded.
  inline bool isUnbounded() const { return sampleCount == 0u; }

  /// Checks if @p sampleTime lies within the domain.
  ///
  /// Validation respects the @p minSet and @p maxSet values such that an unbounded curve
  /// contains any @p sampleTime value. The test may be made open or closed
  /// as determined by the @p closedMin and @p closedMax values. If @p closedMin is true
  /// then <tt>sampleTime == domainMin</tt> is considered to be part of the domain, otherwise
  /// it is not part of the domain. @p closedMax corresponds to @c sampleMax in the same way.
  ///
  /// The default behaviour is closed at both ends.
  ///
  /// @param sampleTime The time value to test within the domain.
  /// @param closedMin Close the interval at the minimum value?
  /// @param closedMax Close the interval at the maximum value?
  /// @return True if @p sampleTime lies within the domain.
  inline bool contains(double sampleTime, bool closedMin = true, bool closedMax = true) const;
};

/// @ingroup expr
/// Finds the union of two domains, @p a and @p b.
///
/// The new domain has a minimum defined as the smaller of the two operand minimums
/// and a maximum defined as the larger of the two operand maximums. The result may
/// cover a section which neither original domain covered.
///
/// The delta is taken as the smaller, positive value of the two domain deltas.
/// Otherwise, the sample delta is set to the domain range. The sample count
/// is dependent on the sample delta, but is at least two for a bounded domain.
///
/// If either @p a or @p b are unbounded, then the @p result is set to the other.
/// Thus the result can only be unbounded if both @p a and @p b are unbounded.
///
/// @param[out] result Set to the union of @p a and @p b.
/// @param a First operand.
/// @param b Second operand.
void domainUnion(PlotExpressionBindDomain &result, const PlotExpressionBindDomain &a, const PlotExpressionBindDomain &b);

/// @ingroup expr
/// Find the union of the domains @p a and @p b, and store in @p a.
/// See the overload for details.
/// @param[in,out] a The first operand and the result.
/// @param b The second operand.
inline void domainUnion(PlotExpressionBindDomain &a, const PlotExpressionBindDomain &b)
{
  return domainUnion(a, a, b);
}


inline void PlotExpressionBindDomain::setUnbounded()
{
  domainMin = domainMax = sampleDelta = 0;
  sampleCount = 0u;
  minSet = maxSet = false;
}


inline bool PlotExpressionBindDomain::contains(double sampleTime, bool closedMin, bool closedMax) const
{
  if (minSet && ((closedMin && sampleTime < domainMin) || (!closedMin && sampleTime <= domainMin)))
  {
    return false;
  }

  if (maxSet && ((closedMax && sampleTime > domainMax) || (!closedMax  && sampleTime >= domainMax)))
  {
    return false;
  }

  return true;
}

#endif // __PLOTEXPRESSIONBINDDOMAIN_H_
