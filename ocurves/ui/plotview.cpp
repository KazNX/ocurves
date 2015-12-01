//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#include "plotview.h"

#include "ui_plotview.h"

#include "plotdatacurve.h"
#include "plotinstance.h"
#include "plotinstancesampler.h"
#include "plotpanner.h"
#include "plotzoomer.h"

#include "model/curves.h"
#include "model/bookmarks.h"

#include "qwt_legend.h"
#include "qwt_plot.h"
#include "qwt_plot_canvas.h"
#include "qwt_plot_renderer.h"
#include "qwt_symbol.h"

#include <QApplication>
#include <QClipboard>
#include <QEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QSettings>

/// An extension of QwtPlot which monitors focus events.
///
/// Installing an event filter for the plot doesn't work (already in use?).
class FocusPlot : public QwtPlot
{
public:
  inline FocusPlot(PlotView *parent)
    : QwtPlot(parent)
    , _owner(parent)
    , _inFocus(false)
  {
  }


  QVariant itemToInfo(QwtPlotItem *plotItem) const override
  {
    QVariant itemInfo;
    if (plotItem->rtti() == PlotDataCurve::Rtti)
    {
      PlotDataCurve *curveItem = static_cast<PlotDataCurve *>(plotItem);
      void *curve = &curveItem->curve();
      itemInfo = qVariantFromValue(curve);
      int type = itemInfo.type();
      type = qMetaTypeId<void *>();
      type = itemInfo.userType();
    }
    return itemInfo;
  }


  QwtPlotItem *infoToItem(const QVariant &info) const override
  {
    if (info.type() == qMetaTypeId<void *>())
    {
      const PlotInstance *curve = static_cast<const PlotInstance *>(info.value<void *>());
      QwtPlotItemList items = itemList(PlotDataCurve::Rtti);
      for (QwtPlotItem *item : items)
      {
        PlotDataCurve *curveItem = static_cast<PlotDataCurve *>(item);
        if (curve == &curveItem->curve())
        {
          return item;
        }
      }
    }

    return nullptr;
  }


protected:
  bool eventFilter(QObject *target, QEvent *event)
  {
    switch (event->type())
    {
    case QEvent::FocusIn:
      if (!_inFocus)
      {
        _inFocus = true;
        _owner->viewFocusGained();
      }
      break;
    case QEvent::FocusOut:
      if (_inFocus)
      {
        _inFocus = false;
        _owner->viewFocusLost();
      }
      break;
    }
    return QwtPlot::eventFilter(target, event);
  }

  PlotView *_owner;
  bool _inFocus;
};


const int PlotView::SharedLegend = QwtPlot::TopLegend + 1;


PlotView::PlotView(Curves *curves, QWidget *parent)
  : QFrame(parent)
  , _plot(new FocusPlot(this))
  , _curves(curves)
  , _zoom(nullptr)
  , _panner(nullptr)
  , _ui(new Ui::PlotView)
  , _toolMode(MultiTool)
  , _synchronised(false)
  , _suppressEvents(false)
{
  _ui->setupUi(this);
  _ui->contentParent->layout()->addWidget(_plot);

#ifndef NDEBUG
  static int number = 1;
  setObjectName(QString("PlotView%1").arg(number++));
#endif // NDEBUG

  _plot->setAxisAutoScale(PlotZoomer::AxisX);
  _plot->setAxisAutoScale(PlotZoomer::AxisY);

  _zoom = new PlotZoomer(static_cast<QwtPlotCanvas *>(_plot->canvas()));
  _zoom->setTrackerMode(QwtPicker::AlwaysOn);

  _panner = new PlotPanner(_plot->canvas());
  _panner->setMouseButton(Qt::MidButton);

  _ui->zoomBothButton->setDefaultAction(_zoom->zoomBothAction());
  _ui->zoomXButton->setDefaultAction(_zoom->zoomXAction());
  _ui->zoomYButton->setDefaultAction(_zoom->zoomYAction());

  {
    Curves::CurveList curveList = _curves->curves();
    for (PlotInstance *curve : curveList)
    {
      addCurve(curve);
    }
    connect(curves, &Curves::curveAdded, this, &PlotView::addCurve);
    connect(curves, &Curves::curveRemoved, this, &PlotView::removeCurve);
    connect(curves, &Curves::curveDataChanged, this, &PlotView::curveDataChanged);
    connect(curves, &Curves::curvesCleared, this, &PlotView::curvesCleared);
    curveList.release();
  }

  _ui->panLockButton->setChecked(_synchronised);
  connect(_ui->panLockButton, &QToolButton::toggled, this, &PlotView::setSynchronised);
  connect(this, &PlotView::synchronisedChanged, _ui->panLockButton, &QToolButton::setChecked);

  _ui->legendCombo->setCurrentIndex(SharedLegend);
  connect(_ui->legendCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &PlotView::legendSelectionChanged);

  QAction *action;
  _toolModeActions[MultiTool] = action = _ui->actionMultiTool;
  _ui->multiToolButton->setDefaultAction(action);
  connect(action, &QAction::triggered, this, &PlotView::setMultiTool);
  connect(action, &QAction::toggled, this, &PlotView::toolActionToggled);
  _toolModeActions[PanTool] = action = _ui->actionPanTool;
  _ui->panToolButton->setDefaultAction(action);
  action->setCheckable(true);
  connect(action, &QAction::triggered, this, &PlotView::setPanTool);
  connect(action, &QAction::toggled, this, &PlotView::toolActionToggled);
  _toolModeActions[ZoomTool] = action = _ui->actionZoomTool;
  _ui->zoomToolButton->setDefaultAction(action);
  connect(action, &QAction::triggered, this, &PlotView::setZoomTool);
  connect(action, &QAction::toggled, this, &PlotView::toolActionToggled);

  _toolModeActions[_toolMode]->setChecked(true);

  // Hide buttons which have been moved to a global toolbar.
  // FIXME: This code should be preserved for external views, but the current usage
  // doesn't need them. We just hide the buttons for now, rather than loose the code.
  _ui->multiToolButton->setVisible(false);
  _ui->panToolButton->setVisible(false);
  _ui->zoomToolButton->setVisible(false);
  _ui->zoomXButton->setVisible(false);
  _ui->zoomYButton->setVisible(false);
  _ui->zoomBothButton->setVisible(false);

  setHighlight(false);
}


PlotView::~PlotView()
{
  QObject *parent = this->parent();
  for (PlotDataCurve *display : _displayCurves)
  {
    display->hide();
    display->detach();
    delete display;
  }

  delete _zoom;
  delete _ui;
}


void PlotView::showNone()
{
  for (PlotDataCurve *display : _displayCurves)
  {
    display->hide();
    display->detach();
  }
  _activeSourceNames.clear();
  _visibleCurveNames.clear();
}


void PlotView::loadSettings(QSettings &settings)
{
  _activeSourceNames = settings.value("sources").toStringList();
  _visibleCurveNames = settings.value("visible").toStringList();
  bool sync = settings.value("sync", false).toBool();
  setSynchronised(sync);
  int legendPos = settings.value("legend", int(SharedLegend)).toInt();
  _ui->legendCombo->setCurrentIndex(legendPos);
  int toolMode = settings.value("tool", int(_toolMode)).toInt();
  setToolMode(ToolMode(toolMode));

  _zoom->loadSettings(settings);

  updateActive(_activeSourceNames, _visibleCurveNames);
}


void PlotView::saveSettings(QSettings &settings)
{
  settings.setValue("sources", _activeSourceNames);
  settings.setValue("visible", _visibleCurveNames);
  settings.setValue("sync", _synchronised);
  settings.setValue("legend", _ui->legendCombo->currentIndex());
  settings.setValue("tool", int(_toolMode));
  _zoom->saveSettings(settings);
}


int PlotView::legendPosition() const
{
  return _ui->legendCombo->currentIndex();
}


void PlotView::setHighlight(bool active)
{
  if (active)
  {
    setFrameStyle(QFrame::Panel | QFrame::Sunken);
  }
  else
  {
    setFrameStyle(QFrame::Panel | QFrame::Raised);
  }
}


bool PlotView::isHighlighted() const
{
  const int style = frameStyle();
  if (style & QFrame::Sunken)
  {
    return true;
  }
  return false;
}


void PlotView::updateActive(const QStringList &sourceNames, const QStringList &curveNames)
{
  for (PlotDataCurve *display : _displayCurves)
  {
    const PlotInstance &curve = display->curve();
    bool show = sourceNames.contains(curve.source().name()) && curveNames.contains(curve.name());
    if (show)
    {
      QColor colour(curve.colour());
      if (!display->isVisible() || display->pen().color() != colour)
      {
        display->setPen(QPen(colour.rgb()));
        display->attach(_plot);
        display->show();
      }
    }
    else
    {
      display->detach();
      display->hide();
    }
    display->setItemAttribute(QwtPlotItem::Legend, show);
  }

  _activeSourceNames = sourceNames;
  _visibleCurveNames = curveNames;

  if (!_activeSourceNames.empty())
  {
    _zoom->fitIfAutoScaling();
  }
  else
  {
    _zoom->zoomToFit(false);
  }
}


void PlotView::zoomToFit()
{
  _zoom->zoomToFit();
}


void PlotView::setSynchronised(bool sync)
{
  _synchronised = sync;
  emit synchronisedChanged(sync);
}


void PlotView::setToolMode(ToolMode mode)
{
  if (mode != _toolMode)
  {
    ToolMode oldMode = _toolMode;
    _toolMode = mode;
    _toolModeActions[oldMode]->setChecked(false);
    _toolModeActions[_toolMode]->setChecked(true);

    switch (_toolMode)
    {
    case MultiTool:
      _zoom->setEnabled(true);
      _panner->setEnabled(true);
      _panner->setMouseButton(Qt::MidButton);
      break;

    case PanTool:
      _zoom->setEnabled(false);
      _panner->setEnabled(true);
      _panner->setMouseButton(Qt::LeftButton);
      break;

    case ZoomTool:
      _zoom->setEnabled(true);
      _panner->setEnabled(false);
      break;

    default:
      break;
    }

    emit toolModeChanged(mode);
  }
}


void PlotView::setLegendPosition(int pos)
{
  if (0 <= pos && pos < _ui->legendCombo->count())
  {
    _ui->legendCombo->setCurrentIndex(pos);
  }
}


void PlotView::copyToClipboard()
{
  if (QApplication::clipboard())
  {
    QPixmap pixmap(_plot->width(), _plot->height());
    pixmap.fill(Qt::transparent);
    QwtPlotRenderer painter;
    painter.renderTo(_plot, pixmap);
    QApplication::clipboard()->setPixmap(pixmap);
  }
}


void PlotView::addCurve(PlotInstance *curve)
{
  PlotDataCurve *displayCurve = new PlotDataCurve(*curve);
  displayCurve->setData(new PlotInstanceSampler(curve));
  displayCurve->setItemAttribute(QwtPlotItem::AutoScale, true);
  displayCurve->hide();
  _displayCurves.append(displayCurve);

  // Check if we should be displaying this item.
  if (_activeSourceNames.contains(curve->source().name()) && _visibleCurveNames.contains(curve->name()))
  {
    displayCurve->attach(_plot);
    displayCurve->show();
  }

  _zoom->fitIfAutoScaling();
}


void PlotView::removeCurve(const PlotInstance *curve)
{
  for (auto iter = _displayCurves.begin(); iter != _displayCurves.end(); ++iter)
  {
    PlotDataCurve *display = *iter;
    if (curve == &display->curve())
    {
      display->detach();
      delete display;
      _displayCurves.erase(iter);
      break;
    }
  }
}


void PlotView::curveDataChanged(const PlotInstance *curve)
{
  bool replot = false;
  for (auto iter = _displayCurves.begin(); iter != _displayCurves.end(); ++iter)
  {
    PlotDataCurve *display = *iter;
    if (curve == &display->curve())
    {
      // Setup display properties.
      // Generate a pen.
      QColor colour(curve->colour());
      bool colourChanged = display->pen().color() != colour;
      QPen pen(colour.rgb());
      pen.setWidth(curve->width());
      display->setPen(pen);

      // Set drawing style.
      if (QwtPlotCurve::NoCurve <= curve->style() && curve->style() <= QwtPlotCurve::Dots)
      {
        display->setStyle(QwtPlotCurve::CurveStyle(curve->style()));
      }
      else
      {
        display->setStyle(QwtPlotCurve::Lines);
      }

      // Set display symbol if current setting differs from current display.
      if (QwtSymbol::NoSymbol < curve->symbol() && curve->symbol() <= QwtSymbol::Hexagon)
      {
        if (colourChanged || !display->symbol() || display->symbol()->style() != curve->symbol() ||
            display->symbol()->size().width() != curve->symbolSize())
        {
          QwtSymbol *symbol = new QwtSymbol(QwtSymbol::Style(curve->symbol()));
          QColor penColour = colour;
          // Darken light colours and lighten dark for the outline colour.
          if (penColour.valueF() >= 0.5)
          {
            penColour = colour.darker();
          }
          else
          {
            penColour = colour.lighter(220);
          }
          symbol->setPen(QPen(penColour, 0.5));
          symbol->setBrush(QBrush(colour.rgb()));
          symbol->setSize(curve->symbolSize());
          display->setSymbol(symbol);
        }
      }
      else
      {
        display->setSymbol(nullptr);
      }

      if (display->isVisible())
      {
        PlotInstanceSampler *sampler = static_cast<PlotInstanceSampler *>(display->data());
        sampler->invalidateBoundingRect();
        display->invalidate();
        _zoom->fitIfAutoScaling();
        replot = true;
      }
      break;
    }
  }

  if (replot)
  {
    _plot->replot();
  }
}


void PlotView::curvesCleared()
{
  _zoom->zoomToFit(false);
}


void PlotView::legendSelectionChanged(int index)
{
  if (index < SharedLegend)
  {
    QwtLegend *legend = qobject_cast<QwtLegend *>(_plot->legend());
    if (!legend)
    {
      legend = new QwtLegend(_plot);
      legend->setDefaultItemMode(QwtLegendData::Clickable);
    }
    _plot->insertLegend(legend, QwtPlot::LegendPosition(index));
    emit legendChanged(legend, index);
  }
  else
  {
    _plot->insertLegend(nullptr);
    emit legendChanged(nullptr, index);
  }
}


void PlotView::toolActionToggled(bool checked)
{
  if (!checked)
  {
    for (int i = 0; i < ToolModeCount; ++i)
    {
      if (_toolModeActions[i] == sender())
      {
        if (_toolMode == i)
        {
          // Recheck the active zoom mode.
          _toolModeActions[i]->setChecked(true);
        }
        break;
      }
    }
  }
}


void PlotView::viewFocusGained()
{
  if (_suppressEvents)
  {
    return;
  }
  //qDebug() << objectName() << ": focus gained";
  emit focusGained();
}


void PlotView::viewFocusLost()
{
  if (_suppressEvents)
  {
    return;
  }
  //qDebug() << objectName() << ": focus lost";
  emit focusLost();
}
