//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef PLOTPANNER_H_
#define PLOTPANNER_H_

#include "ocurvesconfig.h"

#include "qwt_plot_panner.h"

/// @ingroup ui
/// Extension of @c QwtPlotPanner to expose @c moveCanvas() for synchronised panning.
class PlotPanner : public QwtPlotPanner
{
  Q_OBJECT
public:
  /// Alias for the parent class.
  typedef QwtPlotPanner Super;

  /// Constructor.
  /// @param parent The panner parent (generally a @c QwtPlotCanvas).
  explicit PlotPanner(QWidget *parent = nullptr);

public slots:
  /// Synchronised panning event. Calls through to @c moveCanvas().
  ///
  /// Note that @c moveCanvas() if the event hasn't originated from the
  /// @c moveCanvas() event in the first place. However, this only works when
  /// using direct signalling.
  ///
  /// @param dx Pan change along X.
  /// @param dy Pan change along Y.
  void syncPan(int dx, int dy);

protected:
  /// Override to mark ignoring of @c syncPan() events.
  /// @param dx Pan change along X.
  /// @param dy Pan change along Y.
  void moveCanvas(int dx, int dy) override;

private:
  bool _ignoreEvents; ///< Set to ignore @c syncPan() events.
};

#endif // PLOTPANNER_H_
