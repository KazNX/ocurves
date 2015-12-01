//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef PLOTBINARYOPERATOR_H_
#define PLOTBINARYOPERATOR_H_

#include "plotsconfig.h"

#include "plotexpression.h"

#include <QTextStream>

/// @ingroup expr
/// An extension of @p PlotExpression defining a binary operation.
/// This is essentially a binary in the expression tree.
///
/// This tracks the branches and manages binding, but derivations are required
/// to implement the @c sample() function.
class PlotBinaryOperator : public PlotExpression
{
public:
  /// Create a binary operator on the given @p left and @p right expressions.
  /// @param left The left expression.
  /// @param right The right expression.
  inline PlotBinaryOperator(PlotExpression *left, PlotExpression *right)
    : _left(left), _right(right) {}

  /// Destructor, destroying left and right.
  ~PlotBinaryOperator();

  /// Set the left branch. Does not release the existing branch.
  /// @param left The new branch.
  inline void setLeft(PlotExpression *left) { _left = left; }

  /// Get the left branch.
  /// @return The left branch.
  inline const PlotExpression *left() const { return _left; }

  /// Set the right branch. Does not release the existing branch.
  /// @param right The new branch.
  inline void setRight(PlotExpression *right) { _right = right; }

  /// Get the right branch.
  /// @return The right branch.
  inline const PlotExpression *right() const { return _right; }

  /// Binds the left and right branch expressions.
  ///
  /// Handles exhaustive binding.
  virtual BindResult bind(const QList<PlotInstance *> &curves, PlotBindingTracker &bindTracker, PlotExpressionBindDomain &domain, bool repeatLastBinding = false);

  /// Unbind left and right branches.
  virtual void unbind();

private:
  PlotExpression *_left;  ///< Left branch.
  PlotExpression *_right; ///< Right branch.
};


/// @ingroup expr
/// A template binary operator, using a functional object to evaluate the expression.
///
/// This class extends the @c PlotBinaryOperator to create a concrete derivation
/// which uses @p Operator evaluate the expression. The @p Operator type must
/// be an object supporting the bracket operator accepting two @c double
/// arguments and returning a @c double. The operator performs the expression
/// calculations. For example, @c std::plus\<double\>.
///
/// As a convenience, the object supports a @c opStr(), which is used when
/// @c toString() is called. Essentially, the complete string is formed by:
/// <tt>left->toString() + opStr() + right->toString()</tt>.
///
/// @tparam Operator The functional object used to evaluate the expression.
template <class Operator>
class PlotBinaryOperatorT : public PlotBinaryOperator
{
public:
  /// Default constructor: left and right are null.
  inline PlotBinaryOperatorT() : PlotBinaryOperator(nullptr, nullptr) {}

  /// Branched constructor.
  /// @param left The left branch.
  /// @param right The right branch.
  /// @param opstr The string used to combine the branches when @c toString() is called.
  inline PlotBinaryOperatorT(PlotExpression *left, PlotExpression *right, const QString &opstr = QString())
    : PlotBinaryOperator(left, right)
    , _opStr(opstr) {}

  /// Sample the branches and evaluate the results.
  ///
  /// Evaluates @c left and @c right branches at @p sampleTime, and passes the
  /// results to @c Operator for evaluation. Returns the result.
  ///
  /// @param sampleTime The time to sample at.
  /// @return The left and right samples combined with @c Operator.
  virtual inline double sample(double sampleTime) const
  {
    return Operator()(left()->sample(sampleTime), right()->sample(sampleTime));
  }

  /// Return the string used to combine left and right branches.
  /// @return The operation string (e.g., "+" for addition).
  inline const QString &opStr() const { return _opStr; }

  /// Sets the string used to combine left and right branches.
  /// @param str The operation string.
  inline void setOpStr(const QString &str) { _opStr = str; }

  /// Deep clone.
  virtual PlotExpression *clone() const
  {
    PlotBinaryOperatorT<Operator> *op = new PlotBinaryOperatorT<Operator>(left()->clone(), right()->clone());
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
    stream << left()->toString() << _opStr << right()->toString();
    return str;
  }

  QString _opStr; ///< Operation string.
};

#endif // PLOTBINARYOPERATOR_H_
