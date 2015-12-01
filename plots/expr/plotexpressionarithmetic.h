//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2013
//
#ifndef __PLOTEXPRESSIONARITHMETIC_H_
#define __PLOTEXPRESSIONARITHMETIC_H_

#include "plotsconfig.h"

#include "plotbinaryoperator.h"
#include "plotunaryoperator.h"

#include <functional>

/// @ingroup expr
/// A @c PlotExpression which adds the results of two other @c PlotExpression objects.
class PlotAdd : public PlotBinaryOperatorT<std::plus<double> > { public: PlotAdd(PlotExpression *left, PlotExpression *right) { setLeft(left); setRight(right); setOpStr(" + "); } };

/// @ingroup expr
/// A @c PlotExpression which subtracts the results one @c PlotExpression object from another.
class PlotSubtract : public PlotBinaryOperatorT<std::minus<double> > { public: PlotSubtract(PlotExpression *left, PlotExpression *right) { setLeft(left); setRight(right); setOpStr(" - "); } };

/// @ingroup expr
/// A @c PlotExpression which multiplies the results of two other @c PlotExpression objects.
class PlotMultiply : public PlotBinaryOperatorT<std::multiplies<double> > { public: PlotMultiply(PlotExpression *left, PlotExpression *right) { setLeft(left); setRight(right); setOpStr(" * "); } };

/// @ingroup expr
/// A @c PlotExpression which divides the results one @c PlotExpression object from another.
class PlotDivide : public PlotBinaryOperatorT<std::divides<double> > { public: PlotDivide(PlotExpression *left, PlotExpression *right) { setLeft(left); setRight(right); setOpStr(" / "); } };

/// @ingroup expr
/// A @c PlotExpression which adds the results of two other @c PlotExpression objects.
class PlotNegate : public PlotUnaryOperatorT<std::negate<double> > { public: PlotNegate(PlotExpression *operand) { setOperand(operand); setOpStr(" -"); } };

/// @ingroup expr
/// A @c binary_function object which raises the one value to the power of another.
template<class T>
struct Power : public std::binary_function<T, T, T>
{
  /// Expression effector calculating @p left raised to the power of @p right.
  /// @param left The base value.
  /// @param right The exponent.
  /// @return @p left raised to the power of @p right.
  inline T operator()(const T &left, const T &right) const { return T(pow(left, right)); }
};

/// @ingroup expr
/// A @c PlotExpression which raises the results of one @c PlotExpression object by another.
class PlotPower : public PlotBinaryOperatorT<Power<double> > { public : PlotPower(PlotExpression *left, PlotExpression *right) { setLeft(left); setRight(right); setOpStr("^"); } };

#endif // __PLOTEXPRESSIONARITHMETIC_H_
