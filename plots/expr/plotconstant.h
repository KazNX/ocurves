//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2013
//
#ifndef __PLOTCONSTANT_H_
#define __PLOTCONSTANT_H_

#include "plotsconfig.h"

#include "plotexpression.h"

/// @ingroup expr
/// An expression yielding a constant numeric value.
///
/// The constant has no specific domain and its range depends on other domains in the expression tree.
class PlotConstant : public PlotExpression
{
public:
  /// Create a constant expression.
  /// @param constant The value to return from @c sample().
  inline PlotConstant(double constant) : _constant(constant) {}

  /// Sets the constant value.
  /// @param constant The value to return from @c sample().
  inline void setConstant(double constant) { _constant = constant; }

  /// Get the constant value.
  /// @return The value to return from @c sample().
  inline double constant() const { return _constant; }

  /// Return the constant.
  /// @return @c constant().
  virtual double sample(double sampleTime) const;

  /// Always bound, but sets the domain sample count to 1.
  virtual BindResult bind(const QList<PlotInstance *> &curves, PlotBindingTracker &bindTracker, PlotExpressionBindDomain &info, bool repeatLastBinding = false);

  /// Deep clone.
  virtual PlotExpression *clone() const;

private:
  /// String representation.
  virtual QString stringExpression() const;

  double _constant; ///< The value.
};

#endif // __PLOTCONSTANT_H_
