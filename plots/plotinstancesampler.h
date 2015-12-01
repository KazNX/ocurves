//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef PLOTINSTANCESAMPLER_H_
#define PLOTINSTANCESAMPLER_H_

#include "plotsconfig.h"

#include "qwt_series_data.h"

class PlotInstance;

/// @ingroup plot
/// An adaptor class converting from @c PlotInstance data to @c QwtSeriesData.
///
/// The sampler provides two primary functions: adapting the @c PlotInstance for
/// display in Qwt widgets and resolving the time values for samples as
/// dictated by the @c PlotSource. This includes accessing the time column,
/// adjusting the time-base and time scaling (in that order).
///
/// A @c PlotInstance must outlive all its samplers.
class PlotInstanceSampler : public QwtSeriesData<QPointF>
{
public:
  /// Create a sampler for the give plot.
  /// @param curveData The plot to sample.
  PlotInstanceSampler(const PlotInstance *curveData);

  /// Sets the curve, replacing the existing curve.
  void setCurve(const PlotInstance *curveData);

  /// Access the curve to sample.
  /// @return The curve to sample.
  inline const PlotInstance *curve() const { return _curve; }

  /// Returns the number of samples in the @c PlotInstance.
  /// @return The curve sample count.
  size_t size() const override;

  /// Samples the @p ith element of the plot. Respects timing conversions.
  ///
  /// @param i The sample number to request: [0, @c size()).
  /// @return The plot sample. The y value is the sample value, while the x
  ///   value is the sample index or adjusted time.
  QPointF sample(size_t i) const override;

  /// Overridden to recalculate the as required bounds.
  /// @return The curve bounds.
  QRectF boundingRect() const override;

  /// Invalidate the bounding rectangle, to be recalculated.
  void invalidateBoundingRect();

  /// Calculate the bounds.
  /// @return The curve bounds.
  QRectF calculateBoundingRect(size_t from = 0, size_t to = ~(size_t)(0u)) const;

private:
  /// Resolves sample time for the @p ith element.
  /// @param initialTime The initial time value as reported by the @c PlotInstance.
  /// @return The adjusted time value.
  double lookupSampleTime(double initialTime, size_t i) const;

  const PlotInstance *_curve;   ///< Curve to sample.
  mutable QRectF _boundingRect; ///< Cache bounds.
  mutable size_t _lastRingHead; ///< Last ring buffer element head.
  mutable size_t _lastRingSize; ///< last ring buffer size.
};

#endif // PLOTINSTANCESAMPLER_H_
