//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef PLOTUNARYOPERATOR_H_
#define PLOTUNARYOPERATOR_H_

#include "plotsconfig.h"

#include "plotexpression.h"

#include <QTextStream>

/// @ingroup expr
/// An extension of @p PlotExpression defining a unary operation.
///
/// This tracks the child and manages binding, but derivations are required
/// to implement the @c sample() function.
class PlotUnaryOperator : public PlotExpression
{
public:
  /// Create a binary operator on the given @p left and @p right expressions.
  /// @param operand The child expression.
  inline PlotUnaryOperator(PlotExpression *operand) : _operand(operand) {}

  /// Destructor, destroying the child.
  ~PlotUnaryOperator();

  /// Set the operand expression. Does not release the existing expression.
  /// @param operand The new operand.
  inline void setOperand(PlotExpression *operand) { _operand = operand; }

  /// Get the operand expression.
  /// @return The operand.
  inline const PlotExpression *operand() const { return _operand; }

  /// Binds the operand expression.
  virtual BindResult bind(const QList<PlotInstance *> &curves, PlotBindingTracker &bindTracker, PlotExpressionBindDomain &domain, bool repeatLastBinding = false);

  /// Unbind the operand.
  virtual void unbind();

private:
  PlotExpression *_operand; ///< Operand expression.
};


/// @ingroup expr
/// A template unary operator, using a functional object to evaluate the expression.
///
/// This class extends the @c PlotUnaryOperator to create a concrete derivation
/// which uses @p Operator evaluate the expression. The @p Operator type must
/// be an object supporting the bracket operator accepting one @c double
/// argument and returning a @c double. The operator performs the expression
/// calculations. For example, @c std::negate\<double\>.
///
/// As a convenience, the object supports a @c opStr(), which is used when
/// @c toString() is called. Essentially, the complete string is formed by:
/// <tt>opStr() + operand()->toString()</tt>.
///
/// @tparam Operator The functional object used to evaluate the expression.
template <class Operator>
class PlotUnaryOperatorT : public PlotUnaryOperator
{
public:
  /// Default constructor: operand is null.
  inline PlotUnaryOperatorT() : PlotUnaryOperator(0) {}

  /// Constructor.
  /// @param operand Operand expression.
  inline PlotUnaryOperatorT(PlotExpression *operand) : PlotUnaryOperator(operand) {}

  /// Sample the operand and evaluate the @c Operator.
  ///
  /// Evaluates @c operand at @p sampleTime, and passes the
  /// result to @c Operator for evaluation.
  ///
  /// @param sampleTime The time to sample at.
  /// @return The operand samples filtered through @c Operator.
  virtual inline double sample(double sampleTime) const
  {
    return Operator()(operand()->sample(sampleTime));
  }

  /// Return the string used to prefix the operand.
  /// @return The operation string (e.g., "-" for negation).
  inline const QString &opStr() const { return _opStr; }

  /// Sets the string used to prefix the operand.
  /// @param str The operation string.
  inline void setOpStr(const QString &str) { _opStr = str; }

  /// Deep clone.
  virtual PlotExpression *clone() const
  {
    PlotUnaryOperatorT<Operator> *op = new PlotUnaryOperatorT<Operator>(operand()->clone());
    op->setOpStr(_opStr);
    return op;
  }

private:
  /// Convert to string. See above.
  /// @return The expression string.
  virtual QString stringExpression() const
  {
    QString str;
    QTextStream stream(&str);
    stream << _opStr << operand()->toString();
    return str;
  }

  QString _opStr; ///< Operation string.
};

#endif // PLOTUNARYOPERATOR_H_
