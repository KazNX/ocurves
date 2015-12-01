//
// author
//
// Copyright (c) CSIRO 2015
//
#ifndef PLOTZOOMER_H_
#define PLOTZOOMER_H_

#include "ocurvesconfig.h"

#include <qwt_plot_zoomer.h>

class QAction;
class QMenu;
class QSettings;
class QwtPlotCanvas;

/// @ingroup ui
/// Custom zoom handler for @c Plot objects. Extends zoom functionality to handle
/// mouse wheel cycling through zoom levels, middle mouse panning and right mouse
/// reset.
class PlotZoomer : public QwtPlotZoomer
{
  Q_OBJECT

public:
  /// Parent class.
  typedef QwtPlotZoomer Super;

  /// Zooming mode.
  enum ZoomMode
  {
    /// Zoom both axes.
    ZoomBoth,
    /// Zoom only X axis.
    ZoomX,
    /// Zoom only Y axis.
    ZoomY,

    ZoomModeCount
  };

  /// YAxis reference within @c QwtPlot.
  static const unsigned int AxisY = 0;
  /// XAxis reference within @c QwtPlot.
  static const unsigned int AxisX = 2;

  /// Constructor, operating on the given @p plot.
  /// @param plot The plot to operate on.
  /// @param doReplot Call QwtPlot::replot() for the attached plot before initializing
  ///   the zoomer with its scales. This might be necessary,
  ///   when the plot is in a state with pending scale changes.
  explicit PlotZoomer(QwtPlotCanvas *plot, bool doReplot = true);

  /// Destructor.
  ~PlotZoomer();

  /// Access the current @c ZoomMode.
  /// @return The current @c ZoomMode.
  inline ZoomMode zoomMode() const { return _zoomMode; }

  /// Fetch the action used to select the @c ZoomBoth mode.
  /// @return The @c ZoomBoth action.
  QAction *zoomBothAction() const { return _zoomActions[ZoomBoth]; }

  /// Fetch the action used to select the @c ZoomX mode.
  /// @return The @c ZoomX action.
  QAction *zoomXAction() const { return _zoomActions[ZoomX]; }

  /// Fetch the action used to select the @c ZoomY mode.
  /// @return The @c ZoomY action.
  QAction *zoomYAction() const { return _zoomActions[ZoomY]; }

  /// Load zoom settings stored by @c saveSettings().
  ///
  /// Note: @c zoomToFit() is called if the stack size is 1 or empty.
  /// This enforces auto scaling when there is no active stack.
  /// @param settings The object to load from.
  /// @see @c saveSettings()
  void loadSettings(QSettings &settings);

  /// Save zoom settings to @c settings.
  ///
  /// Settings are stored in a group labelled "zoom". The following settings
  /// are stored:
  ///
  /// Key    | Description
  /// ------ | -------------------------------------------------------
  /// mode   | The @c zoomMode()
  /// index  | The current @c zoomRectIndex() in the @c zoomStack()
  /// stack  | The @c zoomStack() as a array of rectangles (see below)
  ///
  /// Each zoom rectangle is written as:
  ///
  /// Key    | Description
  /// ------ | --------------------
  /// x      | The X position
  /// y      | The Y position
  /// w      | The rectangle width
  /// h      | The rectangle height
  ///
  /// @param settings The object to save to.
  void saveSettings(QSettings &settings);

  /// Set current zoom to fit if auto scaling is enabled.
  void fitIfAutoScaling();

  /// Zoom to fit even if auto-scaling is off.
  /// @param replot Force replot after the zoom?
  void zoomToFit(bool replot = true);

  /// Override to prevent catastrophic event propagation on synchronised zoom response.
  /// @param rect The zoom rectangle.
  void zoom(const QRectF &rect) override;

  /// Override to set auto scaling when reaching zoom level zero.
  /// @param up The number of zoom steps to make.
  void zoom(int up) override;

public slots:
  /// Set the active @c ZoomMode.
  ///
  /// Raises the @c zoomModeChanged() signal if the mode differs from the current mode.
  /// @param mode The mode to set.
  void setZoomMode(ZoomMode mode);

  /// Set the zoom mode to @c ZoomBoth.
  inline void setZoomModeBoth() { setZoomMode(ZoomBoth); }

  /// Set the zoom mode to @c ZoomX.
  inline void setZoomModeX() { setZoomMode(ZoomX); }

  /// Set the zoom mode to @c ZoomY.
  inline void setZoomModeY() { setZoomMode(ZoomY); }

  /// Event for zoom synchronisation.
  ///
  /// The whole zoom stack is synchronised if the sender is a @c QwtPlotZoomer.
  void zoomSync(const QRectF &rect);

signals:
  /// Notifies changes in @c zoomMode().
  void zoomModeChanged(int mode);

  /// Signal for changes to/from @c ZoomBoth.
  /// @param isset True if @c ZoomBoth is the new zoom mode.
  void zoomModeBoth(bool isset);

  /// Signal for changes to/from @c ZoomX.
  /// @param isset True if @c ZoomX is the new zoom mode.
  void zoomModeX(bool isset);

  /// Signal for changes to/from @c ZoomY.
  /// @param isset True if @c ZoomY is the new zoom mode.
  void zoomModeY(bool isset);

protected slots:
  /// Bound to zoom actions simply to ensure one is kept active.
  void zoomActionToggled(bool checked);

protected:
  /// End panning or reset the zoom level on right mouse button.
  /// @param event The mouse event.
  void widgetMousePressEvent(QMouseEvent *event) override;

  /// Ignore middle click - handled by panner - resolve right click zoom restore or context menu.
  /// @param event The mouse event.
  void widgetMouseReleaseEvent(QMouseEvent *event) override;

  /// Cycle zoom levels on wheel.
  /// @param event The mouse event.
  void widgetWheelEvent(QWheelEvent *event) override;

  /// Overload to handle @c ZoomMode.
  bool end(bool ok = true) override;

private slots:
  /// Handles displaying the context menu. See remarks.
  ///
  /// The zoomer uses right click events for both resettings the zoom level (@c zoomToFit())
  /// and to display a context menu to select the @c ZoomMode. They are differentiated by
  /// a timeout; a quick right click implies a zoom reset, while a hold implies showing
  /// the context menu. This slot is used as the called to make after the timeout elapses
  /// in the latter case.
  ///
  /// There is a slight complication in that this signal is set up immediately on a right
  /// mouse button press which may be followed by a quick release (zoom reset). We further
  /// distinguish between the two by using a sequence number. The sequence number
  /// @c _contextMenuSequenceNumber is updated whenever we setup this timeout signal, or
  /// when the mouse release has been handled (zoom reset or context menu). The correct handling
  /// is determined by matching @c _contextMenuSequenceNumber against @c _contextMenuExpectedSeq.
  /// Whichever event occurs first will find the two values match and can handle the event.
  /// Otherwise the event is ignored. The handler also adjusts the value of
  /// @c _contextMenuSequenceNumber to ensure it no longer matches @c _contextMenuExpectedSeq.
  void contextMenuTimeout();

private:
  ZoomMode _zoomMode; ///< Active zooming mode.
  unsigned _contextMenuTimeMs;  ///< Time to elapse before showing the right click context menu.
  /// Sequence number for context menu. See @c contextMenuTimeout().
  quint16 _contextMenuSequenceNumber;
  /// Expected sequence number for context menu. See @c contextMenuTimeout().
  quint16 _contextMenuExpectedSeq;

  QMenu *_contextMenu;  ///< @c ZoomMode context menu.
  QAction *_zoomActions[ZoomModeCount]; ///< @c ZoomMode actions.

  bool _ignoreSync; ///< Set when ignoring UI based signals.
};


#endif // PLOTZOOMER_H_
