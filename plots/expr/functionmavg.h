//
// author
//
// Copyright (c) CSIRO 2015
//
#ifndef FUNCTIONMAVG_H_
#define FUNCTIONMAVG_H_

#include "plotsconfig.h"

#include "functiondefinition.h"

#include <QList>
#include <QPointF>

/// @ingroup expr
/// A function which evaluates a moving average.
///
/// This functions uses context data to maintain a windowed history, reporting the average
/// of the window.
class FunctionMAvg : public FunctionDefinition
{
public:
  /// Constructor.
  /// @param category Sorting category.
  FunctionMAvg(const QString &category = QString());

  /// Updates the window and evaluates the average on that window.
  void evaluate(PlotFunctionResult &result, double time, unsigned int argc, const double *argv, const PlotFunctionInfo &info, void *context) const override;

  /// Creates the history object.
  void *createContext() const override;

  /// Destroys the history object.
  void destroyContext(void *context) const override;

private:
  /// Context for @c createContext()
  struct Context
  {
    QList<QPointF> window;    ///< History.
    double total;             ///< Running total.
  };

  /// Add to the @c context window and evaluate.
  /// @param time Current sample time.
  /// @param value Current value (added to window).
  /// @param windowSizeAndValue Specifies the requested (maximum) window size.
  /// @param context The window.
  /// @return The average of the @c Context window.
  static double addWindowAndAverage(double time, double value, double windowSizeValue, Context &context);
};

#endif // FUNCTIONMAVG_H_
