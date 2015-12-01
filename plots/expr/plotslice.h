//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef __PLOTSLICE_H_
#define __PLOTSLICE_H_

#include "plotsconfig.h"

#include "plotexpression.h"

class PlotSample;

/// @ingroup expr
/// The slice expression supports taking a slice out of another expression.
///
/// A slice has indexer start and end expressions, both of which are optional,
/// to define the slice range. While both are expressions, only one sample is
/// ever requested from each one. This occurs at the time of binding. The
/// results may modify the sampling range of the indexee expression.
///
/// In effect, plot binding is effected by first attempting normal binding on
/// the indexee expression. If this succeeds, then the start and end expressions
/// are check to try an restrict the sampling range of the indexee.
///
/// The index start expression limits the domain minimum. The minimum is defined
/// by binding the index start expression, then sampling that expression at its
/// domain minimum. This is illustrated by the pseudo code below, executed in
/// the context of the @c bind() method:
/// @verbatim
///   define startInfo as PlotExpressionBindInfo
///   call indexerStart.bind(curves, startInfo)
///   set startTime = indexerStart.sample(startInfo.domainMin)
/// @endverbatim
///
/// Note now the indexer start expression is sampled only at its domain minimum.
/// The slice domain minimum is then defined as the maximum of @c startTime
/// (above) and the domain minimum of @c indexee, but the minimum of the result
/// and the domain maximum of @c indexee. That is the domain minimum of the
/// indexee is only modified if @c startTime is greater than its original minimum.
///
/// The same process is used to modify the domain maximum, with the following changes:
/// - Sample @c indexerEnd at its domain maximum instead of minimum.
/// - Only modify the domain maximum of @c indexee if the end time is less.
///
/// In some cases the resulting domain minimum and maximum may be invalid, with
/// maximum having value less than the minimum. This is an error condition and binding
/// fails.
///
/// In practise, it is expected that in most cases the start and end expressions
/// are restricted to time value literals (@c PlotConstant).
class PlotSlice : public PlotExpression
{
public:
  /// Defines the slice expression. Start and end expressions are optional.
  ///
  /// Note that defining a slice with no start or end expression is technically
  /// valid and essentially aliases the @p indexee expression. However, it is
  /// expected that at least one or the other be set.
  ///
  /// @param indexee The expression to sample a slice of. Must not be null.
  /// @param indexerStart The expression defining evaluation of the start time
  ///   for the slice. May be null to use the start time of @c indexee.
  /// @param indexerEnd The expression defining evaluation of the end time
  ///   for the slice. May be null to use the end time of @c indexee.
  PlotSlice(PlotExpression *indexee, PlotExpression *indexerStart, PlotExpression *indexerEnd);

  /// Destructor.
  ~PlotSlice();

  /// Accessor for the indexee expression.
  /// @return The expression from which a slice is being taken.
  inline const PlotExpression *indexee() const { return _indexee; }
  /// Accessor for the index start expression.
  /// @return The expression used to calculate the start time. May be null.
  inline const PlotExpression *indexerStart() const { return _indexerStart; }
  /// Accessor for the index end expression.
  /// @return The expression used to calculate the end time. May be null.
  inline const PlotExpression *indexerEnd() const { return _indexerEnd; }

  /// Samples the @c indexee() at @p sampleTime.
  /// @param sampleTime The time to sample at. Expected to be in range.
  /// @return The sample at @p sampleTime.
  double sample(double sampleTime) const override;

  /// Attempts to binds the slice expression.
  ///
  /// This binds the supporting expressions and determines the slice range.
  /// The range is set by modifying @p info.
  ///
  /// Binding fails if:
  /// - Any of the three (non-null) expressions fails to bind.
  /// - The calculated domain minimum is greater than the domain maximum.
  ///
  /// @param curves The set of existing curves available for sampling. These are the curves
  ///   which were loaded from the source named @p sourceFile.
  /// @param bindTracker Tracks the last binding on a per expression basis.
  /// @param[out] domain To be populated with bound domain details if a successful binding can be
  ///   made.
  /// @param repeatLastBinding True to request that the function rebind whatever it bound
  ///   last time it was called, according to the @p bindTracker. Bind the first possibility
  ///   if there is no such last binding.
  /// @return True if binding is successful.
  BindResult bind(const QList<PlotInstance *> &curves, PlotBindingTracker &bindTracker, PlotExpressionBindDomain &domain, bool repeatLastBinding = false) override;

  /// Unbinds the sampling state.
  void unbind() override;

  /// Performs a deep clone.
  /// @return A clone of the expression.
  PlotExpression *clone() const override;

private:
  /// Internal implementation of the string conversion of @c PlotExpression.
  /// @return The string representation of the expression.
  QString stringExpression() const override;

  PlotExpression *_indexee;       ///< Expression to sample a slice of.
  PlotExpression *_indexerStart;  ///< Expression defining the slice start.
  PlotExpression *_indexerEnd;    ///< Expression defining the slice end.
};


#endif // __PLOTSLICE_H_
