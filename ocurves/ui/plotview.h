//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef PLOTVIEW_H_
#define PLOTVIEW_H_

#include "ocurvesconfig.h"

#include <QColor>
#include <QFrame>
#include <QStringList>
#include <QVector>

class Curves;
class PlotDataCurve;
class PlotInstance;
class PlotPanner;
class PlotZoomer;

class QSettings;
class QwtLegend;
class QwtPlot;

namespace Ui
{
  class PlotView;
}

/// @ingroup ui
/// An plot view viewing a @c Curves model.
///
/// The view maintains its own visibility status including list of visible files, curves
/// zoom level and position.
class PlotView : public QFrame
{
  Q_OBJECT

public:
  /// Parent class alias.
  typedef QFrame Super;

  static const int SharedLegend;  ///< Special index for shared legend.

  /// Modes for various interaction tools.
  enum ToolMode
  {
    MultiTool,  ///< Pan/zoom combined tool.
    PanTool,    ///< Pan only tool.
    ZoomTool,   ///< Zoom only tool.

    ToolModeCount
  };

  /// Create a view into @c curves.
  PlotView(Curves *curves, QWidget *parent = nullptr);

  /// Destructor.
  ~PlotView();

  /// Clear viewing of all items. No notification triggered.
  void showNone();

  /// Load settings for this view.
  /// @param settings The settings object to use.
  /// @see @c saveSettings()
  void loadSettings(QSettings &settings);

  /// Save settings for this view.
  ///
  /// Stores the following items:
  /// Key           | Data Item
  /// ------------- | --------------------------------------
  /// @c sources    | list of @c activeSourceNames()
  /// @c visible    | list of @c visibleCurveNames()
  /// @c sync       | @c synchronised() state
  /// @c legend     | The @c legendPosition()
  /// @c tool       | The @c toolMode()
  /// Zoom settings | See @c PlotZoomer::saveSettings()
  ///
  /// @param settings The settings object to use.
  void saveSettings(QSettings &settings);

  /// Access the primary plotting widget. The widget is a child of this object.
  /// @return The @c QwtPlot used to display the curves.
  inline QwtPlot *plot() { return _plot; }

  /// Access the primary plotting widget. The widget is a child of this object.
  /// @return The @c QwtPlot used to display the curves.
  inline const QwtPlot *plot() const { return _plot; }

  /// Get the current tool mode.
  /// @return The current tool mode.
  inline ToolMode toolMode() const { return _toolMode; }

  /// Request the current legend position.
  ///
  /// The result corresponds to @c QwtPlot::LegendPosition with an additional possible
  /// value of @c SharedLegend.
  ///
  /// The @c SharedLeged is not handled by this view as the semantics imply higher
  /// level handling at a single, shared level.
  ///
  /// @return The current legend position, as indicated by @c QwtPlot::LegendPosition or
  ///     @c SharedLegend.
  int legendPosition() const;

  /// Get the synchronisation state.
  ///
  /// A synchronised plot view is expected to keep its panning and zoom levels
  /// in sync with one or more other @c PlotView objects. This flag simply indicates
  /// the desired internal state and synchronisation is managed at a higher level.
  ///
  /// @return True if the view should be synchronised with other views.
  inline bool synchronised() const { return _synchronised; }

  /// Access the zooming tool.
  /// @return The zooming tool belonging to this view.
  inline PlotZoomer *zoomer() const { return _zoom; }

  /// Access the panning tool.
  /// @return The panning tool belonging to this view.
  inline PlotPanner *panner() const { return _panner; }

  /// Set to display in the highlight or not. This affects the display border.
  /// @param active True to activate the highlight.
  void setHighlight(bool active);

  /// Request the current highlight state (as per @c setHighlight()).
  /// @return True if the highlight is active.
  bool isHighlighted() const;

  /// Updates which plots and files this view is showing.
  ///
  /// To display, a curve must have a source name matching one of the @p sourceNames
  /// and a curve name matching one of the @c curveNames.
  ///
  /// The @p allActiveSources may be used as the collated list of sources being displayed
  /// by any active view. This ensures all curves are colour shifted the same way when provided
  /// to all views.
  ///
  /// @param sourceNames The list of sources to display.
  /// @param curveNames The list of curves to display from the sources.
  void updateActive(const QStringList &sourceNames, const QStringList &curveNames);

  /// Retrieves the list of active sources this view is displaying, or expecting to display.
  ///
  /// This is set in @c updateActive().
  /// @return A list of source names this view can display.
  const QStringList &activeSourceNames() const { return _activeSourceNames; }

  /// Retrieves the list of visible curves this view is displaying, or expecting to display.
  ///
  /// This is set in @c updateActive().
  /// @return A list of curve names this view can display.
  const QStringList &visibleCurveNames() const { return _visibleCurveNames; }

public slots:
  /// Change the current zoom level to fit the currently selected plots.
  ///
  /// This resets the zoom-stack and enabled auto-scaling on both axes.
  ///
  /// Raises the @c PlotZoomer::zoomed() signal from @c zoomer().
  void zoomToFit();

  /// Set or clear the synchronisation flag for this view.
  ///
  /// Raises the @c synchronisedChanged() signal.
  /// @param sync True to enable view synchronisation.
  /// @see @c synchronised()
  void setSynchronised(bool sync);

  /// Set the tool mode for this view.
  ///
  /// Raises the @c toolModeChanged() signal.
  /// @param mode The mode to set.
  /// @see @c ToolMode
  void setToolMode(ToolMode mode);

  /// Set the tool mode to @c MultiTool.
  inline void setMultiTool() { setToolMode(MultiTool); }

  /// Set the tool mode to @c PanTool.
  inline void setPanTool() { setToolMode(PanTool); }

  /// Set the tool mode to @c ZoomTool.
  inline void setZoomTool() { setToolMode(ZoomTool); }

  /// Set the legend position.
  ///
  /// The @p pos must be in range or the call is ignored.
  ///
  /// On changing the legend position, the @c legendChanged() signal is raised.
  ///
  /// @param pos The new legend position. Must be one of @c QwtLegend::LegendPosition
  ///   values, or @c SharedLegend.
  void setLegendPosition(int pos);

  /// Copies the current plot view content to the clipboard.
  ///
  /// The current curves and legend (if not shared) are rendered to a bitmap and
  /// added to the system clipboard. The size of the bitmap matches the @c plot()
  /// size.
  ///
  /// This call does nothing if there is no system clipboard, determined by whether
  /// @c QApplication::clipboard() is null or not.
  void copyToClipboard();

signals:
  /// Emitted when this @c PlotView gains focus. This intercepts focusing of the
  /// contained @c plot().
  void focusGained();

  /// Emitted when this @c PlotView looses focus. This based on focus loss of the
  /// contained @c plot().
  void focusLost();

  /// Emitted when the @c synchronised() flag changed.
  /// @param sync The new flag value.
  void synchronisedChanged(bool sync);

  /// Emitted when the current tool mode is changed.
  /// @param mode The new @c ToolMode.
  void toolModeChanged(ToolMode mode);

  /// The legend or its position has been changed, possibly to null.
  /// @param legend The new legend object, or null if @p position is @c SharedLegend.
  /// @param position The new legend position; one of QwtLegend::LegendPosition or @c SharedLegend.
  void legendChanged(QwtLegend *legend, int position);

protected slots:
  /// Handles a new curve definition by creating an appropriate display interface.
  ///
  /// The new curve is initially visible only if its source and name are present in
  /// @c activeSourceNames() and @c visibleCurveNames() respectively.
  ///
  /// Handler for @c Curves::curveAdded()
  /// @param curve The new curve.
  void addCurve(PlotInstance *curve);

  /// Handles curve removal or deletion, removing the corresponding display adaptor.
  ///
  /// The @c visibleCurveNames() remains unchanged.
  ///
  /// Handler for @c Curves::curveRemoved()
  /// @param curve The curve being removed (and likely to be deleted).
  void removeCurve(const PlotInstance *curve);

  /// Handles changes to curve data.
  ///
  /// This updates the display adaptor for @p curve and may replot if the
  /// curve is visible. The method assumes all data may be invalidated as there
  /// is no specification for what has changed.
  /// @param curve The curve which has been modified or invalidated.
  void curveDataChanged(const PlotInstance *curve);

  /// Handles removal of all curves.
  ///
  /// The @c activeSourceNames() and @c visibleCurveNames() remain unchanged.
  ///
  /// Handler for @c Curves::curvesCleared()
  void curvesCleared();

  /// Handles changes to the legend position combo box.
  ///
  /// May emit @c legendChanged()
  /// @param index The new legend position index.
  void legendSelectionChanged(int index);

  /// Ensures the current tool action stays checked. Handler for all tool mode
  /// actions.
  ///
  /// @todo Validate if this is made obsolete by the @c PlotViewToolbar.
  /// @param checked The new checked state for the @c QAction sending the signal.
  void toolActionToggled(bool checked);

private:
  friend class FocusPlot;

  /// Called when the @c plot() gains focus. May emit the @c focusGained() event
  /// if events are not being suppressed.
  void viewFocusGained();

  /// Called when the @c plot() looses focus. May emit the @c focusLost() event
  /// if events are not being suppressed.
  void viewFocusLost();

  QwtPlot *_plot;       ///< The internal plot view.
  Curves *_curves;      ///< Curves data model.
  PlotZoomer *_zoom;    ///< Zooming UI interface.
  PlotPanner *_panner;  ///< Panning UI interface.
  QStringList _activeSourceNames; ///< List of @c PlotSource objects which are active in this view.
  QStringList _visibleCurveNames; ///< List of @c PlotInstance objects which are active in this view.
  QList<PlotDataCurve *> _displayCurves;  ///< Display adaptors for @c PlotInstance objects in this view. Includes non-visible curves.
  Ui::PlotView *_ui;    ///< The UI components for this view.
  ToolMode _toolMode;   ///< Current tool mode.
  bool _synchronised;   ///< See @c synchronised()
  bool _suppressEvents; ///< True if event signalling is disabled. Set during certain UI changes.

  /// List of actions used to modify the current @c ToolMode. One is always kept checked.
  QAction *_toolModeActions[ToolModeCount];
};

#endif // PLOTVIEW_H_
