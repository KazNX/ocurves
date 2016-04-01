//
// author
//
// Copyright (c) CSIRO 2015
//
#include "plotzoomer.h"

#include <qwt_plot_canvas.h>
#include <qwt_plot.h>

#include <QMenu>
#include <QMouseEvent>
#include <QSettings>
#include <QTimer>

namespace
{
  void write(QSettings &settings, const QRectF &rect)
  {
    settings.setValue("x", rect.x());
    settings.setValue("y", rect.y());
    settings.setValue("w", rect.width());
    settings.setValue("h", rect.height());
  }


  void read(QSettings &settings, QRectF &rect)
  {
    rect.setX(settings.value("x", 0.0).toReal());
    rect.setY(settings.value("y", 0.0).toReal());
    rect.setWidth(settings.value("w", 0.0).toReal());
    rect.setHeight(settings.value("h", 0.0).toReal());
  }
}


PlotZoomer::PlotZoomer(QwtPlotCanvas *plot, bool doReplot)
  : QwtPlotZoomer(plot, doReplot)
  , _zoomMode(ZoomBoth)
  , _contextMenuTimeMs(200)
  , _contextMenuSequenceNumber(0)
  , _contextMenuExpectedSeq(0)
  , _contextMenu(new QMenu)
  , _ignoreSync(false)
{
  QAction *action;
  _zoomActions[ZoomBoth] = action = _contextMenu->addAction(tr("Zoom &Both"));
  action->setCheckable(true);
  action->setChecked(true);
  connect(action, &QAction::triggered, this, &PlotZoomer::setZoomModeBoth);
  connect(this, &PlotZoomer::zoomModeBoth, action, &QAction::setChecked);
  _zoomActions[ZoomX] = action = _contextMenu->addAction(tr("Zoom &X"));
  action->setCheckable(true);
  connect(action, &QAction::triggered, this, &PlotZoomer::setZoomModeX);
  connect(this, &PlotZoomer::zoomModeX, action, &QAction::setChecked);
  _zoomActions[ZoomY] = action = _contextMenu->addAction(tr("Zoom &Y"));
  action->setCheckable(true);
  connect(action, &QAction::triggered, this, &PlotZoomer::setZoomModeY);
  connect(this, &PlotZoomer::zoomModeY, action, &QAction::setChecked);

  for (int i = 0; i < ZoomModeCount; ++i)
  {
    connect(_zoomActions[i], &QAction::toggled, this, &PlotZoomer::zoomActionToggled);
  }
}


PlotZoomer::~PlotZoomer()
{
  delete _contextMenu;
}


void PlotZoomer::loadSettings(QSettings &settings)
{
  settings.beginGroup("zoom");

  int zoomMode = settings.value("mode", int(PlotZoomer::ZoomBoth)).toInt();
  setZoomMode(PlotZoomer::ZoomMode(zoomMode));

  int zoomIndex = settings.value("index", -1).toInt();
  int stackSize = settings.beginReadArray("stack");
  QStack<QRectF> stack;
  stack.resize(stackSize);
  for (int i = 0; i < stackSize; ++i)
  {
    settings.setArrayIndex(i);
    read(settings, stack[i]);
  }
  settings.endArray();

  setZoomStack(stack, zoomIndex);

  if (stackSize <= 1)
  {
    zoomToFit(true);
  }

  settings.endGroup();
}


void PlotZoomer::saveSettings(QSettings &settings)
{
  settings.beginGroup("zoom");
  settings.setValue("mode", int(_zoomMode));

  settings.setValue("index", zoomRectIndex());
  settings.beginWriteArray("stack", zoomStack().size());
  int i = 0;
  for (const QRectF &zoom : zoomStack())
  {
    settings.setArrayIndex(i++);
    write(settings, zoom);
  }
  settings.endArray();

  settings.endGroup();
}


void PlotZoomer::fitIfAutoScaling()
{
  QwtPlot *plotter = plot();
  if (plotter->axisAutoScale(AxisY) && plotter->axisAutoScale(AxisX))
  {
    zoomToFit(true);
  }
}


void PlotZoomer::zoomToFit(bool replot)
{
  if (zoomRectIndex() <= 0)
  {
    // Force zoom signal for synchronisation.
    zoom(0);
    emit zoomed(zoomRect());
  }
  else
  {
    zoom(0);
  }
  if (QwtPlot *plotter = plot())
  {
    plotter->setAxisAutoScale(AxisX);
    plotter->setAxisAutoScale(AxisY);
    plotter->updateAxes();
  }
  setZoomBase(replot);
}


void PlotZoomer::zoom(const QRectF &rect)
{
  _ignoreSync = true;
  Super::zoom(rect);
  _ignoreSync = false;
}


void PlotZoomer::zoom(int up)
{
  // FIXME: Save the current zoom window in case it's been panned.
  int zoomIndex = zoomRectIndex();
  if (zoomIndex > 0)
  {
    QStack<QRectF> stack = zoomStack();
    if (zoomIndex < stack.size())
    {
      QRectF r = scaleRect();
      stack[zoomIndex] = r;
      setZoomStack(stack, zoomIndex);
    }
  }

  Super::zoom(up);
  if (zoomRectIndex() == 0)
  {
    if (QwtPlot *plotter = plot())
    {
      plotter->setAxisAutoScale(AxisX);
      plotter->setAxisAutoScale(AxisY);
      plotter->updateAxes();
    }
  }
}


QwtText PlotZoomer::trackerTextF(const QPointF &pos) const
{
  QString text;
  switch (rubberBand())
  {
  case HLineRubberBand:
    text.sprintf("%.6g", pos.y());
    break;
  case VLineRubberBand:
    text.sprintf("%.6g", pos.x());
    break;
  default:
    text.sprintf("%.6g, %.6g", pos.x(), pos.y());
  }
  return QwtText(text);
}


void PlotZoomer::setZoomMode(ZoomMode mode)
{
  if (_zoomMode != mode)
  {
    ZoomMode oldMode = _zoomMode;
    _zoomMode = mode;
    emit zoomModeChanged(mode);
    switch (oldMode)
    {
    case ZoomBoth:
      emit zoomModeBoth(false);
      break;
    case ZoomX:
      emit zoomModeX(false);
      break;
    case ZoomY:
      emit zoomModeY(false);
      break;
    default:
      break;
    }
    switch (_zoomMode)
    {
    case ZoomBoth:
      emit zoomModeBoth(true);
      break;
    case ZoomX:
      emit zoomModeX(true);
      break;
    case ZoomY:
      emit zoomModeY(true);
      break;
    default:
      break;
    }
  }
}


void PlotZoomer::zoomSync(const QRectF &rect)
{
  if (!_ignoreSync)
  {
//    if (QwtPlotZoomer *zoomer = qobject_cast<QwtPlotZoomer *>(sender()))
//    {
//      setZoomStack(zoomer->zoomStack(), zoomer->zoomRectIndex());
//    }
//    else
//    {
//      zoom(rect);
//    }
    // Adjust the zoom rect according to the zoom mode.
    QRectF adjustedRect = rect;
    switch (_zoomMode)
    {
    default:
    case ZoomBoth:
      break;
    case ZoomX:
      adjustedRect.setY(zoomRect().y());
      adjustedRect.setHeight(zoomRect().height());
      break;
    case ZoomY:
      adjustedRect.setX(zoomRect().x());
      adjustedRect.setWidth(zoomRect().width());
      break;
    }
    zoom(adjustedRect);
  }
}


void PlotZoomer::zoomActionToggled(bool checked)
{
  if (!checked)
  {
    for (int i = 0; i < ZoomModeCount; ++i)
    {
      if (_zoomActions[i] == sender())
      {
        if (_zoomMode == i)
        {
          // Recheck the active zoom mode.
          _zoomActions[i]->setChecked(true);
        }
        break;
      }
    }
  }
}


void PlotZoomer::widgetMousePressEvent(QMouseEvent *event)
{
  if (mouseMatch(MouseSelect2, event))
  {
    _contextMenuExpectedSeq = ++_contextMenuSequenceNumber;
    //QTimer::singleShot(_contextMenuTimeMs, this, &PlotZoomer::contextMenuTimeout);
    QTimer::singleShot(_contextMenuTimeMs, this, SLOT(contextMenuTimeout()));
  }
  Super::widgetMousePressEvent(event);
}


void PlotZoomer::widgetMouseReleaseEvent(QMouseEvent *event)
{
  if (mouseMatch(MouseSelect3, event))
  {
    // Panner handles middle button.
    return;
  }

  QwtPlotZoomer::widgetMouseReleaseEvent(event);
  if (mouseMatch(MouseSelect2, event))
  {
    if (_contextMenuExpectedSeq == _contextMenuSequenceNumber)
    {
      ++_contextMenuSequenceNumber;
      zoomToFit();
    }
  }
}

void PlotZoomer::widgetWheelEvent(QWheelEvent *event)
{
  if (event->delta())
  {
    int numSteps = event->delta() > 0 ? 1 : -1;
    zoom(numSteps);
  }
}


bool PlotZoomer::end(bool ok)
{
  // Code here is taken from QwtPlotZoomer. The only modification is around the _zoomMode handling.
  ok = QwtPlotPicker::end(ok);
  if (!ok)
  {
    return false;
  }

  QwtPlot *plot = QwtPlotZoomer::plot();
  if (!plot)
  {
    return false;
  }

  const QPolygon &pa = selection();
  if (pa.count() < 2)
  {
    return false;
  }

  QRect rect = QRect(pa[0], pa[int(pa.count() - 1)]);
  rect = rect.normalized();

  QRectF currentZoomRect = zoomRect();
  QRectF zoomRect = invTransform(rect).normalized();

  switch (_zoomMode)
  {
  default:
  case ZoomBoth:
    // nothing.
    break;

  case ZoomX:
    // Maintain current zoom y and height.
    zoomRect.setY(currentZoomRect.y());
    zoomRect.setHeight(currentZoomRect.height());
    break;

  case ZoomY:
    // Maintain current zoom x and width.
    zoomRect.setX(currentZoomRect.x());
    zoomRect.setWidth(currentZoomRect.width());
    break;
  }

  const QSizeF minSize = minZoomSize();
  if (minSize.isValid())
  {
    const QPointF center = zoomRect.center();
    zoomRect.setSize(zoomRect.size().expandedTo(minZoomSize()));
    zoomRect.moveCenter(center);
  }

  zoom(zoomRect);

  return true;
}


void PlotZoomer::contextMenuTimeout()
{
  if (_contextMenu && _contextMenuExpectedSeq == _contextMenuSequenceNumber)
  {
    _contextMenuExpectedSeq = 0;
    _contextMenu->popup(QCursor::pos());
  }
}
