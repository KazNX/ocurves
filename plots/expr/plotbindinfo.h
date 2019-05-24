//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef __BINDINFO_H_
#define __BINDINFO_H_

#include "plotsconfig.h"

#include "plotexpressionbinddomain.h"

#include <QString>
#include <QList>
#include <QVector>

class PlotExpression;
class PlotBindingTracker;
class PlotInstance;

/// @ingroup expr
/// A structure tracking the domain for an expression during binding.
struct BindInfo
{
  PlotExpressionBindDomain domain;  ///< The domain of @p expression.
  PlotExpression *expression;       ///< The expression to track.
};

/// @ingroup expr
/// Contains utility functions for binding expression tree branches.
///
/// Tree based @c PlotExpression objects may require binding on multiple branches.
/// Each branch may in turn require binding multiple times. For example, consider
/// the following expression:
/// @code{.unparsed}
///   r'speed.*' - r'speed.*'
/// @endcode
///
/// This creates the following expression tree:
/// @code{.unparsed}
///           PlotSubtract
///             /   \ .
///   PlotSample    PlotSample
/// @endcode
///
/// Both @c PlotSample nodes are using regular expressions. Now, if the available curves are:
/// [ 'speed_measured', 'speed_reported' ], then the resulting bindings should result in a
/// comparison of 'speed_measured' against 'speed_reported'. The @c bindMultiple() functions
/// handle this sort of exhaustive binding, resulting in four bindings in this case:
/// - 'speed_measured' - 'speed_measured'
/// - 'speed_reported' - 'speed_reported'
/// - 'speed_measured' - 'speed_reported'
/// - 'speed_reported' - 'speed_measured'
///
/// While this is excessive, it ensures all required bindings are made.
///
/// The @c bindMultiple() functions should be used by any @c PlotExpression which has
/// two or more child expressions. For such an expresison, the @c bind() method should
/// look somewhat as follows:
/// @code
/// BindResult bind(const QList<PlotInstance *> &curves, PlotBindingTracker &bindTracker, PlotExpressionBindDomain &domain, bool repeatLastBinding)
/// {
///   if (!_left || !_right)
///   {
///     return BindError;
///   }
///
///   BindInfo bindings[2];
///   bindings[0].expression = _left;
///   bindings[1].expression = _right;
///
///   repeatLastBinding = repeatLastBinding || bindTracker.isHeld(this);
///   BindResult bindResult = binding::bindMultiple(curves, bindTracker, bindings[0], &bindings[1], 1, repeatLastBinding);
///   domainUnion(domain, bindings[0].domain, bindings[1].domain);
///   return bindResult;
///}
/// @endcode
///
/// This is a binary binding. A ternary binding would have additional elements in @c bindings.
namespace binding
{
  /// Bind multiple expressions, ensuring exhaustive binding combinations.
  ///
  /// This binds the expression at @p head, then recurses to bind the remaining items in @p tail.
  /// The overall bind result is a combination of the head and tail results as follows:
  ///
  /// Head Result     | Tail Result     | Final Result
  /// --------------- | --------------- | --------------
  /// Any             | BindError       | BindError
  /// Any             | BindFailure     | BindFailure
  /// BindError       | Any             | BindError
  /// BindFailure     | Any             | BindFailure
  /// Bound           | Bound           | Bound
  /// Bound           | BoundMaybeMore  | BoundMaybeMore
  /// BoundMaybeMore  | Bound           | BoundMaybeMore
  /// BoundMaybeMore  | BoundMaybeMore  | BoundMaybeMore
  ///
  /// @param curves The curves available for binding.
  /// @param bindTracker Tracks bind state for later bindings.
  /// @param head The head of the list of items to bind.
  /// @param tail A list of remaining expression requiring binding.
  /// @param tailLength Number of elements in @p tail.
  /// @param repeatLast True to remake the last binding if possible (as defined in @p bindTracker).
  /// @return The overall binding result as described in the table above.
  BindResult bindMultiple(const QList<PlotInstance *> &curves, PlotBindingTracker &bindTracker, BindInfo &head, BindInfo *tail, unsigned tailLength, bool repeatLast);
}

#endif // __BINDINFO_H_
