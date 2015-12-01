//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef PLOTDATACURVE_H_
#define PLOTDATACURVE_H_

#include "ocurvesconfig.h"

#include <qwt_plot_curve.h>

#include "qwtrttiext.h"

class PlotInstance;

/// @ingroup ui
/// A specialisation of @c QwtPlotCurve used to visualisation @c PlotInstanceData.
///
/// This class allows the @c PlotInstance data to be shared across a number of active
/// plots.
class PlotDataCurve : public QwtPlotCurve
{
public:
  enum
  {
    /// The id for @c rtti() identifying a @c PlotDataCurve class.
    Rtti = Rtti_PlotDataCurve
  };

  /// Constructor.
  explicit PlotDataCurve(PlotInstance &curve);

  /// Returns the @c Rtti value for this object.
  /// @return The value @c Rtti.
  int rtti() const override;

  /// Invalidate this curve visualiser, requesting a replot.
  ///
  /// This invokes the @c QwtPlotSeriesItem::dataChanged() method.
  inline void invalidate() { dataChanged(); }

  /// Access the data being visualised.
  /// @return The visualised @c PlotInstance.
  inline PlotInstance &curve() { return *_curve; }

  /// Access the data being visualised.
  /// @return The visualised @c PlotInstance.
  inline const PlotInstance &curve() const { return *_curve; }

private:
  PlotInstance *_curve; ///< The data.
};


#endif // PLOTDATACURVE_H_
