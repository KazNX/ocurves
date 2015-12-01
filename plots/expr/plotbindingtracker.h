//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef PLOTBINDINGTRACKER_H_
#define PLOTBINDINGTRACKER_H_

#include "plotsconfig.h"

#include <QHash>

class PlotExpression;
class PlotInstance;

/// @ingroup expr
/// Supports multiple bindings of the same expression to different curves.
///
/// This class tracks the state of expressions supporting multiple bindings,
/// allowing each expression to maintain its own binding marker, or index.
///
/// The class supports multiple binding by:
/// - Setting @c firstPlot() as the first bound @c PlotInstance in the tree.
/// - Associating an arbitrary marker with each @c PlotExpression in the tree.
/// - Allowing a @c PlotExpression to mark itself as "held", implying it should
///   keep its binding and marker the same as for its last binding.
///
/// The marker usage depends on the @c PlotExpression implementation. A simple
/// implementation is to use the marker as the index of the last curve bound
/// in the @c PlotExpression::bind() @c curves argument. The marker should only
/// be progressed if @c isHeld() is false for an expression.
///
/// See @c PlotExpression for further details on multi-binding.
class PlotBindingTracker
{
public:
  /// Create an empty biding.
  inline PlotBindingTracker() : _firstPlot(nullptr) {}

  /// Request the first bound @c PlotInstance in the tree.
  /// @return The first bound @c PlotInstance.
  inline PlotInstance *firstPlot() const { return _firstPlot; }

  /// Set the first bound @c PlotInstance in the tree. Prefer to use @c setFirstPlotIf().
  /// @param plot The new plot.
  inline void setFirstPlot(PlotInstance *plot) { _firstPlot = plot; }

  /// Set the first bound @c PlotInstance so long as it is not already set.
  /// @param plot The new plot.
  inline void setFirstPlotIf(PlotInstance *plot) { if (!_firstPlot) { _firstPlot = plot; } }

  /// Clear @c firstPlot().
  inline void clearFirstPlot() { _firstPlot = nullptr; }

  /// Set the arbitrary marker value for @p expr.
  /// @param expr The expression to set the marker for.
  /// @param marker The marker value.
  void setMarker(const PlotExpression *expr, unsigned marker);

  /// Request the marker value for @p expr.
  /// @param expr The expression to get the marker for.
  /// @param[out] marker Set to the marker value of @p expr if available.
  /// @return True if a marker had been set and @p marker is valid.
  bool markerFor(const PlotExpression *expr, unsigned &marker) const;

  /// Checks if a marker is present for @p expr.
  /// @param expr The expression to search for a marker for.
  /// @return True if there is a marker set for @p expr.
  bool contains(const PlotExpression *expr) const;

  /// Set the flag indicating @p expr should not progress its marker.
  /// @param expr The expression of interest.
  /// @param hold True to hold, false to release.
  void setHold(const PlotExpression *expr, bool hold = true);

  /// Is @p expr being held?
  /// @param expr The expression of interest.
  /// @return True if held.
  bool isHeld(const PlotExpression *expr) const;

  /// Clear marker and hold flag for @p expr.
  /// @param expr The expression of interest.
  bool clear(const PlotExpression *expr);

private:
  PlotInstance *_firstPlot;       ///< @c firstPlot()
  QHash<const PlotExpression *, unsigned> _markers; ///< Marker hash.
  QHash<const PlotExpression *, bool> _hold;        ///< Hold flags.
};

#endif // PLOTBINDINGTRACKER_H_
