//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2013
//
#ifndef __PLOTFUNCTION_H_
#define __PLOTFUNCTION_H_

#include "plotsconfig.h"

#include "plotexpression.h"

#include "plotfunctioninfo.h"
#include "plotfunctionresult.h"

#include <QVector>

class FunctionDefinition;

/// @ingroup expr
/// A function called to operation on any number of plot expressions.
/// This inserts a @c FunctionDefinition to the expression tree. It supports appropriate
/// binding and parameter evaluation for the definition.
///
/// Expression binding is managed by binding each of the @c args() expressions (if any)
/// with the binding domain as the union of the arguments' domain. Sampling is achieved by
/// sampling each of the @c args(), then calling @c FunctionDefinition::evaluate() with
/// the results.
///
/// The number of @c args() in the expression must match that of the @c FunctionDefinition.
class PlotFunction : public PlotExpression
{
public:
  /// Create a function expression. The number of @p args must match the minimum for
  /// the @p function. The @p function must outlive this object.
  ///
  /// @param function The function definition.
  /// @param args Expressions used to evaluate the arguments.
  PlotFunction(const FunctionDefinition *function, const QVector<PlotExpression *> &args = QVector<PlotExpression *>());

  /// Destructor, releasing all @p args().
  ~PlotFunction();

  /// The @c FunctionDefinition underpinning this expression object.
  /// @return The function definition.
  const FunctionDefinition *function() const { return _function; }

  /// Bind the argument expressions for the function.
  virtual BindResult bind(const QList<PlotInstance *> &curves, PlotBindingTracker &bindTracker, PlotExpressionBindDomain &info, bool repeatLastBinding = false);

  /// Unbind arguments.
  virtual void unbind();

  /// Sample at @p sampleTime. This samples all @c args() expressions at @p sampleTime, then filters the result through
  /// the @c function() object.
  virtual double sample(double sampleTime) const;

  /// Get the argument expressions.
  const QVector<PlotExpression *> &args() const { return _args; }

  /// Clone the arguments.
  QVector<PlotExpression *> cloneArgs() const;

  /// Deep clone, including arguments.
  virtual PlotExpression *clone() const;

private:
  /// Convert to string.
  virtual QString stringExpression() const;

  QVector<PlotExpression *> _args;      ///< Argument expressions.
  const FunctionDefinition *_function;  ///< Function definition.
  mutable PlotFunctionInfo _info;       ///< Binding info. Mutable :(
  void *_functionContext;               ///< Evaluation context object from @c FunctionDefinition::createContext().
};

#endif // __PLOTFUNCTION_H_
