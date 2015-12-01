//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef PLOTEXPRESSIONGENERATOR_H_
#define PLOTEXPRESSIONGENERATOR_H_

#include "ocurvesconfig.h"

#include "plotgenerator.h"

class QMutex;

/// @ingroup gen
/// A @c PlotGenerator which generated plots by evaluating @c PlotExpression objects.
///
/// The generator is designed to run asynchronously. As such, it makes copies of the
/// data on which to operate in case these are destroyed. However, the
/// @c PlotInstance::expression member is always set as the original expression, not
/// the internal copy.
///
/// More explicitly, plots generated herein refer to the original @c PlotExpression
/// objects, but internal calculations use a copy of the expressions for thread
/// safety. The original expressions should remain valid and unmodified while the
/// generator executes, but failure to do so will not result in a failure within the
/// generator object. Instead, generated plots may be out of date or reference an
/// invalid expression.
///
/// Plot binding is exhaustive, supporting regular expression based @c PlotExpression
/// trees. However, this can lead to multiple bindings of the same data when referencing
/// regular expressions from different plots. For example, the expression:
/// @verbatim
///   r'samples-.*'|'value' - r'samples-.*'|'value'
/// @endverbatim
/// Consider where we have two files loaded 'samples-a' and 'samples-b'. This expression
/// will bind the following:
/// - 'samples-a'|'value' - 'samples-a'|'value'
/// - 'samples-a'|'value' - 'samples-a'|'value'
/// - 'samples-a'|'value' - 'samples-b'|'value'
/// - 'samples-b'|'value' - 'samples-b'|'value'
///
/// @note The @c PlotSource for @c PlotInstance objects generated here is the same as
/// the original source. Expression curves can be distinguished by the fact that
/// @c PlotInstance::expression() is not null.
class PlotExpressionGenerator : public PlotGenerator
{
  Q_OBJECT
public:
  /// Create an expression generator based on the given expressions and curves.
  ///
  /// The generator makes copies of the @c PlotExpression and @c PlotInstance
  /// objects for internal calculations. However, a copy of the original
  /// expression list is maintained so that the original expression objects are
  /// assigned to generated @c PlotInstance objects. See class comments.
  ///
  /// @param curves The @c Curves model containing existing curves from which to
  ///   generate new ones. New curves are added to this model.
  /// @param expressions The expressions to generate new curves from. These
  ///   objects must remain valid until the generator completes.
  /// @param sourceNames List of all the source files from which @p curves were
  ///   loaded.
  PlotExpressionGenerator(Curves *curves, const QList<PlotExpression *> &expressions, const QStringList &sourceNames);

  /// Destructor.
  ~PlotExpressionGenerator();

  /// Adds an expression to be generated even if already running. Thread-safe.
  ///
  /// The caller retains ownership of @p expression.
  /// @return True if generation of @p expression has been queued for the current run.
  ///   False if generation is not running or has completed.
  bool addExpression(PlotExpression *expression);

  /// Aborts generation of @p expression if it has yet to be generated. Thread-safe.
  ///
  /// @return True if the given expression has been removed or was not present. False if
  ///     the expression has already been generated.
  bool removeExpression(const PlotExpression *expression);

protected:
  /// Generation loop.
  void run() override;

  /// Check if the curve exists. This is used to duplicate binding.
  ///
  /// Looks in @c _existingCurves for an item with the same @c PlotSource::name()
  /// and @c PlotInstance::name().
  ///
  /// @param curve The curve to look for a duplicate of.
  /// @return True if a matching curves exists.
  bool curveExists(const PlotInstance &curve) const;

private:
  /// Pairing of the original expression and the clone used for binding and generation (giving thread-safety).
  struct ExpressionPair
  {
    PlotExpression *expression;         ///< Used in expression evaluation.
    const PlotExpression *original;     ///< Original expression Reference. May get deleted during generation.
  };

  QVector<ExpressionPair> _expressions;   ///< Expressions used for evaluation.
  QList<PlotInstance *> _existingCurves;  ///< A copy of existing curves when loading generated expressions.
  QStringList _sourceNames;               /// Only for use with plot expressions.
  struct GenerationMarker *_marker;       ///< Tracks generation progress to support @c addExpression() and @c removeExpression().
};

#endif // PLOTEXPRESSIONGENERATOR_H_
