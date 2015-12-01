//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#include "splitplotview.h"

#include "plotpanner.h"
#include "plotview.h"
#include "plotzoomer.h"

#include <QVBoxLayout>
#include <QSettings>
#include <QSplitter>

#include <qwt_plot.h>

#include <cassert>
#include <functional>

namespace
{
  void foreachView(QWidget *branch, const std::function<void(PlotView *)> &func)
  {
    if (PlotView *view = qobject_cast<PlotView *>(branch))
    {
      func(view);
    }
    else
    {
      for (QObject *child : branch->children())
      {
        if (QWidget *widget = qobject_cast<QWidget *>(child))
        {
          foreachView(widget, func);
        }
      }
    }
  }
}


SplitPlotView::SplitPlotView(Curves *curves, QWidget *parent)
  : QWidget(parent)
  , _root(nullptr)
  , _activeView(nullptr)
  , _curves(curves)
  , _suppressEvents(false)
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setObjectName(QStringLiteral("splitViewLayout"));
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  PlotView *firstView = new PlotView(curves, this);
  initialiseView(firstView);
  _root = firstView;
  layout->addWidget(firstView);
  setActiveView(firstView);
}


void SplitPlotView::loadSettings(QSettings &settings)
{
  splitRemoveAll();
  // Remove the root as well before recreating.
  if (_root)
  {
    _root->setVisible(false);
    _root->deleteLater();
  }

  _activeView = nullptr;

  settings.beginGroup("root");
  PlotView *active = nullptr;
  _root = loadTree(settings, this, active);
  settings.endGroup();

  if (_root)
  {
    layout()->addWidget(_root);
  }

  if (!active)
  {
    active = findFirstView(_root);
  }

  if (active)
  {
    setActiveView(active);
  }
}


void SplitPlotView::saveSettings(QSettings &settings)
{
  settings.beginGroup("root");
  saveBranch(settings, _root);
  settings.endGroup();
}


bool SplitPlotView::hasSplits() const
{
  return qobject_cast<QSplitter *>(_root) != nullptr;
}


void SplitPlotView::replot()
{
  foreachView(_root, [](PlotView * view)
  {
    view->plot()->replot();
  });
}


void SplitPlotView::collate(QVector<PlotView *> &views)
{
  foreachView(_root, [&views](PlotView * view)
  {
    views.push_back(view);
  });
}


void SplitPlotView::collateActive(QStringList &sourceNames, QStringList &curveNames, const PlotView *exclude)
{
  foreachView(_root, [&sourceNames, &curveNames, exclude](PlotView * view)
  {
    if (view != exclude)
    {
      for (const QString &sourceName : view->activeSourceNames())
      {
        if (!sourceNames.contains(sourceName))
        {
          sourceNames.push_back(sourceName);
        }
      }

      for (const QString &curveName : view->visibleCurveNames())
      {
        if (!curveNames.contains(curveName))
        {
          curveNames.push_back(curveName);
        }
      }
    }
  });
}


void SplitPlotView::splitVertical()
{
  QSplitter *newSplitter = new QSplitter(Qt::Vertical);
  initialiseSplit(newSplitter, _activeView);
}


void SplitPlotView::splitHorizontal()
{
  QSplitter *newSplitter = new QSplitter(Qt::Horizontal);
  initialiseSplit(newSplitter, _activeView);
}


void SplitPlotView::splitRemove()
{
  if (QSplitter *split = qobject_cast<QSplitter *>(_activeView->parentWidget()))
  {
    // Get the split's children and add them to its parent.
    // This assumes that each split only contains two items.
    QWidget *otherChild = nullptr;
    foreach (QObject *child, split->children())
    {
      if (child != _activeView)
      {
        if (QWidget *widget = qobject_cast<QWidget *>(child))
        {
          otherChild = widget;
          break;
        }
      }
    }

    if (QSplitter *grandparentSplit = qobject_cast<QSplitter *>(split->parentWidget()))
    {
      // We can merge into the grand parent.
      grandparentSplit->addWidget(otherChild);
    }
    else
    {
      // No grand parent split. Must be the root.
      assert(_root == split);
      _root = otherChild;
      layout()->addWidget(otherChild);
    }

    PlotView *removeView = _activeView;
    split->deleteLater();
    removeView->deleteLater();
    _synchedViews.removeOne(removeView);
    setActiveView(findFirstView(otherChild));
    //emit viewRemoved(removeView);
  }
}


void SplitPlotView::splitRemoveAll()
{
  if (_root != _activeView)
  {
    _synchedViews.clear();
    _root->setVisible(false);
    _root->deleteLater();
    _root = _activeView;
    _activeView->setParent(this);
    layout()->addWidget(_activeView);
    if (_activeView->synchronised() && _synchedViews.isEmpty())
    {
      _synchedViews.append(_activeView);
    }
  }
}


void SplitPlotView::setMultiTool()
{
  if (_suppressEvents)
  {
    return;
  }

  _suppressEvents = true;
  foreachView(_root, [](PlotView * view)
  {
    view->setMultiTool();
  });
  emit multiToolModeSet();
  _suppressEvents = false;
}


void SplitPlotView::setPanTool()
{
  if (_suppressEvents)
  {
    return;
  }

  _suppressEvents = true;
  foreachView(_root, [](PlotView * view)
  {
    view->setPanTool();
  });
  emit panToolModeSet();
  _suppressEvents = false;
}


void SplitPlotView::setZoomTool()
{
  if (_suppressEvents)
  {
    return;
  }

  _suppressEvents = true;
  foreachView(_root, [](PlotView * view)
  {
    view->setZoomTool();
  });
  emit zoomToolModeSet();
  _suppressEvents = false;
}


void SplitPlotView::setZoomX()
{
  if (_suppressEvents)
  {
    return;
  }

  _suppressEvents = true;
  foreachView(_root, [](PlotView * view)
  {
    view->zoomer()->setZoomModeX();
  });
  emit zoomXSet();
  _suppressEvents = false;
}


void SplitPlotView::setZoomY()
{
  if (_suppressEvents)
  {
    return;
  }

  _suppressEvents = true;
  foreachView(_root, [](PlotView * view)
  {
    view->zoomer()->setZoomModeY();
  });
  emit zoomYSet();
  _suppressEvents = false;
}


void SplitPlotView::setZoomXY()
{
  if (_suppressEvents)
  {
    return;
  }

  _suppressEvents = true;
  foreachView(_root, [](PlotView * view)
  {
    view->zoomer()->setZoomModeBoth();
  });
  emit zoomXYSet();
  _suppressEvents = false;
}


void SplitPlotView::viewFocusGained()
{
  PlotView *view = qobject_cast<PlotView *>(sender());
  if (view)
  {
    //qDebug() << view->objectName() << ": focus gained";
    setActiveView(view);
  }
}


void SplitPlotView::viewSyncChanged(bool sync)
{
  PlotView *view = qobject_cast<PlotView *>(sender());
  if (view)
  {
    // FIXME: can't connect PlotPanner::moved event to syncPan as it results in double movement.
    // It's more dynamic (responds during drag), but there's an event double up.
    // For now, get the feature working, and worry about niceties later.
    if (sync)
    {
      if (!_synchedViews.contains(view))
      {
        // Connect panning events
        for (PlotView *otherView : _synchedViews)
        {
          connect(otherView->panner(), &PlotPanner::panned, view->panner(), &PlotPanner::syncPan);
          connect(view->panner(), &PlotPanner::panned, otherView->panner(), &PlotPanner::syncPan);

          connect(otherView->zoomer(), &PlotZoomer::zoomed, view->zoomer(), &PlotZoomer::zoomSync);
          connect(view->zoomer(), &PlotZoomer::zoomed, otherView->zoomer(), &PlotZoomer::zoomSync);
        }

        _synchedViews.append(view);
      }
    }
    else
    {
      if (_synchedViews.removeOne(view))
      {
        // Connect panning events
        for (PlotView *otherView : _synchedViews)
        {
          disconnect(otherView->panner(), &PlotPanner::panned, view->panner(), &PlotPanner::syncPan);
          disconnect(view->panner(), &PlotPanner::panned, otherView->panner(), &PlotPanner::syncPan);

          disconnect(otherView->zoomer(), &PlotZoomer::zoomed, view->zoomer(), &PlotZoomer::zoomSync);
          disconnect(view->zoomer(), &PlotZoomer::zoomed, otherView->zoomer(), &PlotZoomer::zoomSync);
        }
      }
    }
  }
}


void SplitPlotView::viewToolModeChanged(int mode)
{
  if (!_suppressEvents)
  {
    _suppressEvents = true;
    foreachView(_root, [mode](PlotView * view)
    {
      view->setToolMode(PlotView::ToolMode(mode));
    });
    _suppressEvents = false;

    switch (mode)
    {
    case PlotView::MultiTool:
      emit multiToolModeSet();
      break;
    case PlotView::PanTool:
      emit panToolModeSet();
      break;
    case PlotView::ZoomTool:
      emit zoomToolModeSet();
      break;
    }
  }
}


void SplitPlotView::viewZoomModeChanged(int mode)
{
  if (!_suppressEvents)
  {
    _suppressEvents = true;
    foreachView(_root, [mode](PlotView * view)
    {
      view->zoomer()->setZoomMode(PlotZoomer::ZoomMode(mode));
    });
    _suppressEvents = false;

    switch (mode)
    {
    case PlotZoomer::ZoomBoth:
      emit zoomXYSet();
      break;
    case PlotZoomer::ZoomX:
      emit zoomXSet();
      break;
    case PlotZoomer::ZoomY:
      emit zoomYSet();
      break;
    }
  }
}


void SplitPlotView::setActiveView(PlotView *view)
{
  if (view != _activeView)
  {
    PlotView *oldView = _activeView;
    _activeView = view;
    _activeView->setFocus();
    if (oldView)
    {
      oldView->setHighlight(false);
    }
    if (view)
    {
      view->setHighlight(true);
    }
    emit activeViewChanged(view, oldView);
  }
}


PlotView *SplitPlotView::findFirstView(QWidget *widget)
{
  if (PlotView *view = qobject_cast<PlotView *>(widget))
  {
    return view;
  }

  // Breadth first traversal.
  for (QObject *child : widget->children())
  {
    if (PlotView *view = qobject_cast<PlotView *>(child))
    {
      return view;
    }
  }

  for (QObject *child : widget->children())
  {
    if (QWidget *childWidget = qobject_cast<QWidget *>(child))
    {
      if (PlotView *view = findFirstView(childWidget))
      {
        return view;
      }
    }
  }

  return nullptr;
}


QWidget *SplitPlotView::relevantChild(const QSplitter *split, int index)
{
  int i = 0;
  while (QWidget *child = split->widget(i++))
  {
    if (qobject_cast<PlotView *>(child) || qobject_cast<QSplitter *>(child))
    {
      if (index <= 0)
      {
        return child;
      }
      --index;
    }
  }

  return nullptr;
}


void SplitPlotView::initialiseView(PlotView *view, const PlotView *referenceView)
{
  connect(view, &PlotView::focusGained, this, &SplitPlotView::viewFocusGained);
  connect(view, &PlotView::synchronisedChanged, this, &SplitPlotView::viewSyncChanged);

  // Propagation handlers.
  connect(view, &PlotView::toolModeChanged, this, &SplitPlotView::viewToolModeChanged);
  connect(view->zoomer(), &PlotZoomer::zoomModeChanged, this, &SplitPlotView::viewZoomModeChanged);

  // Copy the displayed graphs.
  if (referenceView)
  {
    if (!referenceView->visibleCurveNames().empty())
    {
      view->updateActive(referenceView->activeSourceNames(), referenceView->visibleCurveNames());
      view->setSynchronised(referenceView->synchronised());
    }

    view->setToolMode(referenceView->toolMode());
    view->zoomer()->setZoomMode(referenceView->zoomer()->zoomMode());
  }

  emit viewAdded(view);
}


void SplitPlotView::initialiseSplit(QSplitter *split)
{
  split->setChildrenCollapsible(false);
}


void SplitPlotView::initialiseSplit(QSplitter *split, PlotView *view)
{
  if (QSplitter *viewParent = qobject_cast<QSplitter *>(view->parentWidget()))
  {
    // Find the index of view in split.
    int insertIndex = viewParent->indexOf(view);
    viewParent->insertWidget(insertIndex, split);
  }
  else if (view->parent() == this)
  {
    // This is the first split.
    _root = split;
    layout()->addWidget(split);
  }

  split->setChildrenCollapsible(false);
  split->addWidget(view);
  PlotView *newView = new PlotView(_curves, split);
  initialiseView(newView, view);
  split->addWidget(newView);
}


void SplitPlotView::saveBranch(QSettings &settings, QWidget *branch)
{
  if (PlotView *view = qobject_cast<PlotView *>(branch))
  {
    settings.setValue("type", "view");
    view->saveSettings(settings);
    settings.setValue("active", (view == _activeView));
  }
  else if (QSplitter *split = qobject_cast<QSplitter *>(branch))
  {
    settings.setValue("type", "split");
    settings.setValue("orientation", int(split->orientation()));
    settings.beginGroup("left");
    saveBranch(settings, left(split));
    settings.endGroup();
    settings.beginGroup("right");
    saveBranch(settings, right(split));
    settings.endGroup();
    settings.setValue("geometry", split->saveGeometry());
  }
}


QWidget *SplitPlotView::loadTree(QSettings &settings, QWidget *parent, PlotView *&activeView)
{
  QWidget *widget = nullptr;
  QString type = settings.value("type").toString();

  if (type.compare("view") == 0)
  {
    PlotView *view = new PlotView(_curves, parent);
    widget = view;
    initialiseView(view);
    bool active = settings.value("active", false).toBool();
    view->loadSettings(settings);

    if (active)
    {
      activeView = view;
    }
  }
  else if (type.compare("split") == 0)
  {
    Qt::Orientation orientation = Qt::Orientation(settings.value("orientation", int(Qt::Horizontal)).toInt());
    QSplitter *split = new QSplitter(orientation, parent);
    widget = split;
    initialiseSplit(split);

    settings.beginGroup("left");
    loadTree(settings, split, activeView);
    settings.endGroup();
    settings.beginGroup("right");
    loadTree(settings, split, activeView);
    settings.endGroup();

    split->restoreGeometry(settings.value("geometry").toByteArray());
  }

  if (widget)
  {
    if (QSplitter *splitParent = qobject_cast<QSplitter *>(parent))
    {
      splitParent->addWidget(widget);
    }
  }

  return widget;
}
