//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef EXPRESSIONS_H_
#define EXPRESSIONS_H_

#include "ocurvesconfig.h"

#include <QObject>
#include <QList>

#include "expr/functiondefinition.h"
#include "expr/plotexpression.h"

class PlotFunctionRegister;
class QSettings;

Q_DECLARE_METATYPE(PlotExpression *)
Q_DECLARE_METATYPE(FunctionDefinition *)

/// @ingroup data
/// Maintains the current expressions available for generation and plotting.
///
/// This forms the model for use in the UI and for generating expressions from.
class Expressions : public QObject
{
  Q_OBJECT
public:
  /// Internally used in settings serialisation.
  static const unsigned missingTolerance = 50;

  /// Create an expressions data model object.
  /// @param parent The owning object.
  Expressions(QObject *parent = nullptr);

  /// Destructor.
  ~Expressions();

  /// Delete all expressions optionally suppressing event signals.
  /// @param suppressSignals True to suppress the @c expressionRemoved() signal, false
  ///   to allow it.
  void clear(bool suppressSignals = true);

  /// Load expressions from the given settings.
  ///
  /// Expressions are loaded from the current group in settings, so it should be primed
  /// before calling this method.
  ///
  /// @param settings The settings to load from.
  /// @param delaySignals True to delay the @c addExpression() and
  ///   @c expressionRemoved() signals until done.
  void loadExpressions(const QSettings &settings, bool delaySignals = true);

  /// Save expressions to the given settings.
  ///
  /// Expressions are saved with the tag 'exp#' where '#' is the expression number,
  /// starting from 1. At most @p expressionCountLimit expressions are stored.
  /// After writing existing expressions, the @p settings are checked for
  /// additional, contiguous 'exp#' patterns, which are removed.
  ///
  /// For example, we have three expressions, which are written as [ exp1, exp2, exp3 ].
  /// The settings previously contained six expressions, adding [ exp4, exp5, exp6 ].
  /// After saving three expressions, this function tests @p settings for the existence
  /// of 'exp4' and remove it. It then tests for 'exp5', etc and removes the key until
  /// key is not found in @p settings, in this case 'exp7'.
  ///
  /// Expressions are saved to the current group in @p settings.
  ///
  /// @param settings The settings to save to.
  void saveExpressions(QSettings &settings);

  /// Adds an expression.
  /// @param expression The expression to add.
  void addExpression(PlotExpression *expression);

  /// Remove @p expression if it is present. The caller takes ownership of @p expression.
  /// @param expression The expression to remove.
  /// @return True if the expression was found and removed.
  bool removeExpression(PlotExpression *expression);

  /// Check if @p expression is a known expression (check by pointer).
  /// @return True if the pointer @p expression references a known expression.
  bool contains(const PlotExpression *expression) const;

  /// Access the list of expressions.
  /// @return The current expressions.
  const QList<PlotExpression *> &expressions() const;

  /// Access the function register.
  /// @return The function register.
  const PlotFunctionRegister &functionRegister() const;

  /// Registers a function for use in expressions. See @c PlotFunction.
  ///
  /// This registers a function in the @c functionRegister() if possible and
  /// raises @c functionRegistered() on success.
  ///
  /// @param functionDef The definition of the function being registered.
  /// @param takeOwnership true to take ownership of the @p functionDef. This allows
  ///   it to be deleted by the function register on shutdown.
  bool registerFunction(FunctionDefinition *functionDef, bool takeOwnership = true);

signals:
  /// Raised when an expression is added.
  /// @param expression The added expression.
  void expressionAdded(PlotExpression *expression);
  /// Raised when an expression is removed.
  /// @param expression The removed expression. May be deleted if the signal is delayed.
  void expressionRemoved(const PlotExpression *expression);
  /// Raised when a function is successfully added via @c registerFunction().
  /// @param function The definition of the function added.
  void functionRegistered(const FunctionDefinition *function);

private:
  QList<PlotExpression *> _expressions; ///< Current expressions.
  PlotFunctionRegister *_functionRegister;  ///< The function register.
};

inline const QList<PlotExpression *> &Expressions::expressions() const
{
  return _expressions;
}


inline const PlotFunctionRegister &Expressions::functionRegister() const
{
  return *_functionRegister;
}


#endif // EXPRESSIONS_H_
