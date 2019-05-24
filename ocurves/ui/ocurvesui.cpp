//
// author Kazys Stepanas
//
// Copyright (c) 2012
//
#include "ocurvesui.h"
#include "ui_ocurvesui.h"

#include "ocurvesver.h"

#include "coloursview.h"
#include "curveproperties.h"
#include "defaultcolours.h"
#include "expr/plotexpression.h"
#include "expr/plotexpressionparser.h"
#include "expr/plotfunctionregister.h"
#include "expressionsview.h"
#include "loadprogress.h"
#include "model/bookmarks.h"
#include "model/curves.h"
#include "model/expressions.h"
#include "ocurvesutil.h"
#include "plotdatacurve.h"
#include "plotexpressiongenerator.h"
#include "plotfileloader.h"
#include "plotinstance.h"
#include "plotview.h"
#include "plotviewtoolbar.h"
#include "plotzoomer.h"
#include "realtimeplot.h"
#include "splitplotview.h"
#include "toolbarwidgets.h"

#include "qwt_legend.h"
#include "qwt_plot.h"
#include "qwt_series_data.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QDebug>
#include <QDesktopServices>
#include <QDragEnterEvent>
#include <QElapsedTimer>
#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QRegExp>
#include <QRgb>
#include <QSettings>
#include <QSpinBox>
#include <QToolBar>

#define REFRESH_INTERVAL (1000/30)

OCurvesUI::OCurvesUI(QWidget *parent)
  : QMainWindow(parent)
  , _ui(new Ui::OCurvesUI)
  , _toolbarWidgets(nullptr)
  , _viewToolbar(nullptr)
  , _splitView(nullptr)
  , _legend(nullptr)
  , _curves(new Curves(this))
  , _expressionsView(nullptr)
  , _suppressEvents(false)
  , _postLoaderAction(PLA_None)
  , _loader(nullptr)
  , _sourcesContextMenu(nullptr)
  , _plotsContextMenu(nullptr)
  , _expressions(new Expressions)
  , _timeSinceLastPlot(new QElapsedTimer)
  , _streams(nullptr)
  , _properties(nullptr)
  , _activeBookmark(0)
{
  _timeSinceLastPlot->start();
  _ui->setupUi(this);
  setupToolbars();

  setWindowIcon(QIcon(":/icons/ocurves.png"));
  _originalWindowTitle = windowTitle();
  setWindowTitle(QString("%1 %2").arg(_originalWindowTitle).arg(ocutil::versionString()));

  _splitView = new SplitPlotView(_curves, this);
  _ui->plotView->layout()->addWidget(_splitView);

  // Use placeholder to size the legend (minimum).
  _legend = new QwtLegend(_ui->legendDock);
  _legend->setDefaultItemMode(QwtLegendData::Clickable);
  _legend->setMinimumWidth(_ui->legendPlaceholder->minimumWidth());

  connect(_legend, &QwtLegend::clicked, this, &OCurvesUI::legendItemClicked);
  _ui->legendPlaceholder->setEnabled(false);
  _ui->legendDock->setWidget(_legend);

  if (_splitView->activeView())
  {
    viewAdded(_splitView->activeView());
  }

  _properties = new CurveProperties(_curves, _ui->propertiesDock);
  _ui->propertiesDock->setWidget(_properties);
  _ui->propertiesDock->close();

  connect(_splitView, &SplitPlotView::viewAdded, this, &OCurvesUI::viewAdded);
  connect(_splitView, &SplitPlotView::activeViewChanged, this, &OCurvesUI::activeViewChanged);

  connect(_viewToolbar->action(PlotViewToolbar::ActionZoomXY), &QAction::triggered, _splitView, &SplitPlotView::setZoomXY);
  connect(_viewToolbar->action(PlotViewToolbar::ActionZoomX), &QAction::triggered, _splitView, &SplitPlotView::setZoomX);
  connect(_viewToolbar->action(PlotViewToolbar::ActionZoomY), &QAction::triggered, _splitView, &SplitPlotView::setZoomY);
  connect(_splitView, &SplitPlotView::zoomXYSet, _viewToolbar->action(PlotViewToolbar::ActionZoomXY), &QAction::trigger);
  connect(_splitView, &SplitPlotView::zoomXSet, _viewToolbar->action(PlotViewToolbar::ActionZoomX), &QAction::trigger);
  connect(_splitView, &SplitPlotView::zoomYSet, _viewToolbar->action(PlotViewToolbar::ActionZoomY), &QAction::trigger);

  connect(_viewToolbar->action(PlotViewToolbar::ActionToolMulti), &QAction::triggered, _splitView, &SplitPlotView::setMultiTool);
  connect(_viewToolbar->action(PlotViewToolbar::ActionToolZoom), &QAction::triggered, _splitView, &SplitPlotView::setZoomTool);
  connect(_viewToolbar->action(PlotViewToolbar::ActionToolPan), &QAction::triggered, _splitView, &SplitPlotView::setPanTool);
  connect(_splitView, &SplitPlotView::multiToolModeSet, _viewToolbar->action(PlotViewToolbar::ActionToolMulti), &QAction::trigger);
  connect(_splitView, &SplitPlotView::zoomToolModeSet, _viewToolbar->action(PlotViewToolbar::ActionToolZoom), &QAction::trigger);
  connect(_splitView, &SplitPlotView::panToolModeSet, _viewToolbar->action(PlotViewToolbar::ActionToolPan), &QAction::trigger);

  connect(_ui->actionOpen, &QAction::triggered, this, &OCurvesUI::openDataFiles);
  connect(_ui->actionConnect, &QAction::triggered, this, &OCurvesUI::connectToRealtimeSource);
  connect(_ui->actionReload, &QAction::triggered, this, &OCurvesUI::reloadPlots);
  connect(_ui->actionClear, &QAction::triggered, this, &OCurvesUI::clearPlots);
  connect(_ui->actionEditColours, &QAction::triggered, this, &OCurvesUI::editColours);
  connect(_ui->actionSplitVertical, &QAction::triggered, _splitView, &SplitPlotView::splitVertical);
  connect(_ui->actionSplitHorizontal, &QAction::triggered, _splitView, &SplitPlotView::splitHorizontal);
  connect(_ui->actionSplitRemove, &QAction::triggered, _splitView, &SplitPlotView::splitRemove);
  connect(_ui->actionSplitRemoveAll, &QAction::triggered, _splitView, &SplitPlotView::splitRemoveAll);
  connect(_ui->actionCopyActiveView, &QAction::triggered, this, &OCurvesUI::copyActiveView);
  connect(_ui->actionExportBookmarks, &QAction::triggered, this, &OCurvesUI::exportBookmarks);
  connect(_ui->actionImportBookmarks, &QAction::triggered, this, &OCurvesUI::importBookmarks);
  connect(_ui->actionRestoreLastSession, &QAction::triggered, this, &OCurvesUI::restoreLastSession);
  connect(_ui->actionViewHelp, &QAction::triggered, this, &OCurvesUI::viewHelp);
  connect(_ui->actionAbout, &QAction::triggered, this, &OCurvesUI::showAbout);

  connect(_ui->sourcesList, &QListWidget::itemSelectionChanged, this, &OCurvesUI::sourcesSelectionChanged);
  connect(_ui->sourcesList, &QListWidget::doubleClicked, this, &OCurvesUI::sourceSelectOnlyIndex);
  connect(_ui->sourcesList, &QListWidget::customContextMenuRequested, this, &OCurvesUI::sourcesContextMenu);
  connect(_ui->plotsList, &QListWidget::itemSelectionChanged, this, &OCurvesUI::plotsSelectionChanged);
  connect(_ui->plotsList, &QListWidget::doubleClicked, this, &OCurvesUI::plotsSelectOnlyIndex);
  connect(_ui->plotsList, &QListWidget::customContextMenuRequested, this, &OCurvesUI::plotsContextMenu);

  connect(_curves, &Curves::curveAdded, this, &OCurvesUI::curveAdded);
  connect(_curves, &Curves::curveComplete, this, &OCurvesUI::curveComplete);
  connect(_curves, &Curves::loadingComplete, this, &OCurvesUI::curveLoadingComplete);
  connect(_curves, &Curves::sourceDataChanged, this, &OCurvesUI::sourceDataChanged);

  _sourcesContextMenu = new QMenu();
  QAction *action;
  action = _sourcesContextMenu->addAction(tr("Select &Only"));
  connect(action, &QAction::triggered, this, &OCurvesUI::sourcesSelectOnlyCurrent);
  action = _sourcesContextMenu->addAction(tr("Select &All"));
  connect(action, &QAction::triggered, _ui->sourcesList, &QListWidget::selectAll);
  action = _sourcesContextMenu->addAction(tr("Select &None"));
  connect(action, &QAction::triggered, _ui->sourcesList, &QListWidget::clearSelection);
  action = _sourcesContextMenu->addAction(tr("&Reload"));
  connect(action, &QAction::triggered, this, &OCurvesUI::sourcesReloadCurrent);
  action = _sourcesContextMenu->addAction(tr("Remo&ve"));
  connect(action, &QAction::triggered, this, &OCurvesUI::sourcesRemoveCurrent);

  _plotsContextMenu = new QMenu();
  action = _plotsContextMenu->addAction(tr("Select &Only"));
  connect(action, &QAction::triggered, this, &OCurvesUI::plotsSelectOnlyCurrent);
  action = _plotsContextMenu->addAction(tr("Select &All"));
  connect(action, &QAction::triggered, _ui->plotsList, &QListWidget::selectAll);
  action = _plotsContextMenu->addAction(tr("Select &None"));
  connect(action, &QAction::triggered, _ui->plotsList, &QListWidget::clearSelection);

  connect(_expressions, &Expressions::expressionAdded, this, &OCurvesUI::expressionAdded);
  connect(_expressions, &Expressions::expressionRemoved, this, &OCurvesUI::expressionRemoved);

  _expressionsView = new ExpressionsView(_expressions, this);
  _ui->expressionsDock->setWidget(_expressionsView);

  setupBookmarks();

  // Can't restore window state settings from constructor.
  REFERENCE_SETTINGS(settings);
  _ui->actionRestoreLastSession->setEnabled(bookmarks::exists(settings, 0));
  loadSettings(settings, false);
  installEventFilter(this);
  startTimer(REFRESH_INTERVAL);
}


OCurvesUI::~OCurvesUI()
{
  endStreams();
  stopLoad();
  clearCurves();
  delete _expressions;
  delete _plotsContextMenu;
  delete _sourcesContextMenu;
  _plotsContextMenu = nullptr;
  delete _ui;
  delete _timeSinceLastPlot;
  delete _curves;
}


void OCurvesUI::loadSettings(QSettings &settings, bool loadGeometry)
{
  if (loadGeometry)
  {
    loadWindowsSettings(settings);
  }

  settings.beginGroup("load");
  _loadDirectory = settings.value("dir", "").toString();
  _loadFilter = settings.value("filter", "").toString();
  _toolbarWidgets->timeColumnCheck()->setChecked(settings.value("useTimeColumn", "true").toBool());
  _toolbarWidgets->timeColumnSpin()->setValue(settings.value("timeColumn", 1).toUInt());
  _toolbarWidgets->maxSamplesSpin()->setValue(settings.value("targetSamples", 20000).toUInt());
  _toolbarWidgets->timeScaleEdit()->setText(QString("%1").arg(settings.value("timeScale", 1).toDouble()));
  _toolbarWidgets->relativeTimeCheck()->setChecked(settings.value("relativeTime", "false").toBool());
  settings.endGroup(); // load

  settings.beginGroup("stream");
  _streamDirectory = settings.value("dir", "").toString();
  _streamFilter = settings.value("filter", "").toString();
  settings.endGroup(); // stream

  settings.beginGroup("plot");
  QString coloursString = settings.value("colours", "").toString();
  _colours.clear();
  if (!coloursString.isEmpty())
  {
    QStringList coloursList = coloursString.split(QRegExp("[ \t,]+"), QString::SkipEmptyParts);
    foreach (const QString &colStr, coloursList)
    {
      bool ok = false;
      QRgb c = colStr.toUInt(&ok, 16) | qRgba(0, 0, 0, 255);
      if (ok)
      {
        _colours.append(c);
      }
    }
  }
  settings.endGroup(); // plot

  if (_colours.isEmpty())
  {
    int setId = ocurves::StandardColours;
    for (unsigned i = 0; i < ocurves::DefaultColourSetCounts[setId]; ++i)
    {
      _colours.append(ocurves::WebSafeColours[ocurves::DefaultColourSets[setId][i]]);
    }
  }

  settings.beginGroup("expressions");
  _expressions->loadExpressions(settings);
  settings.endGroup();
}


void OCurvesUI::loadWindowsSettings()
{
  REFERENCE_SETTINGS(settings);
  loadWindowsSettings(settings);
}


void OCurvesUI::loadWindowsSettings(QSettings &settings)
{
  settings.beginGroup("ui");
  restoreGeometry(settings.value("geometry").toByteArray());
  restoreState(settings.value("windowState").toByteArray());
  settings.endGroup(); // ui
}


void OCurvesUI::saveSettings(QSettings &settings, bool bookmarkCurrent)
{
  settings.beginGroup("ui");
  settings.setValue("geometry", saveGeometry());
  settings.setValue("windowState", saveState());
  settings.endGroup(); // ui

  settings.beginGroup("load");
  settings.setValue("dir", _loadDirectory);
  settings.setValue("filter", _loadFilter);
  settings.setValue("useTimeColumn", _toolbarWidgets->timeColumnCheck()->isChecked());
  settings.setValue("timeColumn", _toolbarWidgets->timeColumnSpin()->value());
  settings.setValue("targetSamples", _toolbarWidgets->maxSamplesSpin()->value());
  settings.setValue("timeScale", _toolbarWidgets->timeScaleEdit()->text());
  settings.setValue("relativeTime", _toolbarWidgets->relativeTimeCheck()->isChecked());
  settings.endGroup(); // load

  settings.beginGroup("stream");
  settings.setValue("dir", _streamDirectory);
  settings.setValue("filter", _streamFilter);
  settings.endGroup(); // stream

  settings.beginGroup("plot");
  if (!_colours.empty())
  {
    QString coloursString;
    coloursString = QString::number(_colours[0] & ~qRgba(0, 0, 0, 255), 16);
    for (int i = 1; i < _colours.count(); ++i)
    {
      coloursString.append(",");

      coloursString.append(QString::number(_colours[i] & ~qRgba(0, 0, 0, 255), 16));
    }
    settings.setValue("colours", coloursString);
  }
  else
  {
    settings.setValue("colours", "");
  }
  settings.endGroup(); // plot

  settings.beginGroup("expressions");
  _expressions->saveExpressions(settings);
  settings.endGroup();

  if (bookmarkCurrent)
  {
    bookmarks::setBookmark(settings, 0, this, true);
  }
}


void OCurvesUI::load(const QString &plotFile, bool append)
{
  QStringList fileList;
  fileList << plotFile;
  load(fileList, append);
}


void OCurvesUI::load(const QStringList &plotFiles, bool append, QVector<TimeSampling> *plotTiming)
{
  if (append)
  {
    if (PlotFileLoader *fileLoader = qobject_cast<PlotFileLoader *>(_loader))
    {
      // Ensure we have the correct time setup.
      setTimeControls(fileLoader);
      if (fileLoader->append(plotFiles, plotTiming))
      {
        // Successfully appended. We are done.
        return;
      }
    }
  }

  // We are here for one of the following reasons:
  // - append is false.
  // - The current loader is not a file loader.
  // = Appending to the file loader failed. It completed before we could append.

  PlotFileLoader *fileLoader = new PlotFileLoader(_curves, plotFiles, plotTiming);
  fileLoader->setTargetSampleCount(_toolbarWidgets->maxSamplesSpin()->value());
  activateLoader(fileLoader, PLA_GenerateExpressions);
}


void OCurvesUI::openDataFiles()
{
  stopLoad();

  QString fileFilter = QString("%1 (*.txt);;%3 (*.*)")
                       .arg(tr("Text Files"))
                       .arg(tr("All Files"));
  QString activeFilter = QString("%1 (*.*)").arg(tr("All Files"));

  if (!_loadFilter.isEmpty())
  {
    //  QString lastFilterString = QString("%0 (%1);;").arg(tr("Last Filter")).arg(_loadFilter);
    //  fileFilter = lastFilterString + fileFilter;
    activeFilter = _loadFilter;
  }

  QFileDialog filesDialog(this, tr("Open Files"), _loadDirectory, fileFilter);
  filesDialog.setAcceptMode(QFileDialog::AcceptOpen);
  filesDialog.setFileMode(QFileDialog::ExistingFiles);
  filesDialog.selectNameFilter(activeFilter);

  if (filesDialog.exec() == QDialog::Accepted)
  {
    _loadDirectory = filesDialog.directory().absolutePath();
    _loadFilter = filesDialog.selectedNameFilter();
    QStringList fileList = filesDialog.selectedFiles();
    load(fileList);
  }
}


void OCurvesUI::connectToRealtimeSource()
{
  stopLoad();

  QString fileFilter = QString("%0 (*.xml);;%3 (*.*)")
                       .arg(tr("Realtime-spec Files"))
                       .arg(tr("All Files"));
  QString activeFilter = QString("%1 (*.xml)").arg(tr("Realtime-spec Files"));

  QFileDialog filesDialog(this, tr("Open Realtime Files"), _streamDirectory, fileFilter);
  filesDialog.setAcceptMode(QFileDialog::AcceptOpen);
  filesDialog.setFileMode(QFileDialog::ExistingFile);
  filesDialog.selectNameFilter(activeFilter);

  if (!_streamFilter.isEmpty())
  {
    activeFilter = _loadFilter;
  }

  if (filesDialog.exec() == QDialog::Accepted)
  {
    _streamDirectory = filesDialog.directory().absolutePath();
    _streamFilter = filesDialog.selectedNameFilter();

    // Load streams.
    QStringList fileList = filesDialog.selectedFiles();
    if (!_streams)
    {
      _streams = new RealTimePlot(_curves, fileList);
      _streams->start();
      connectLoader(_streams);
    }
    else
    {
      _streams->appendLoad(fileList);
    }
  }
}


void OCurvesUI::reloadPlots()
{
  stopLoad();
  // Collect the list of plot files.
  QStringList fileList;
  Curves::CurveList curves = _curves->curves();
  for (PlotInstance *curve : curves)
  {
    if (!curve->source().fullName().isEmpty())
    {
      if (!fileList.contains(curve->source().fullName()))
      {
        fileList << curve->source().fullName();
      }
    }
  }
  curves.release();

  // #TODO: Leave real-time plots active. Possibly reconnect them.
  clearPlots();
  load(fileList);
}


void OCurvesUI::clearPlots()
{
  stopLoad();
  clearCurves();
}



void OCurvesUI::sourceSelectOnlyIndex(const QModelIndex &index)
{
  sourceSelectOnly(index.row());
}


void OCurvesUI::sourceSelectOnly(int row)
{
  // Select only the item at the given row.
  if (QListWidgetItem *item = _ui->sourcesList->item(row))
  {
    _ui->sourcesList->clearSelection();
    item->setSelected(true);
  }
}


void OCurvesUI::plotsSelectOnlyIndex(const QModelIndex &index)
{
  plotsSelectOnly(index.row());
}


void OCurvesUI::plotsSelectOnly(int row)
{
  // Select only the item at the given row.
  if (QListWidgetItem *item = _ui->plotsList->item(row))
  {
    _ui->plotsList->clearSelection();
    item->setSelected(true);
  }
}


void OCurvesUI::copyActiveView()
{
  if (PlotView *view = _splitView->activeView())
  {
    view->copyToClipboard();
  }
}


void OCurvesUI::editColours()
{
  ColoursView coloursView;
  coloursView.setColours(_colours);
  coloursView.setModal(true);
  if (coloursView.exec() == QDialog::Accepted)
  {
    _colours = coloursView.colours();
    REFERENCE_SETTINGS(settings);
    saveSettings(settings);
    recolourCurves();
  }
}


void OCurvesUI::sourcesSelectionChanged()
{
  if (_suppressEvents)
  {
    return;
  }

  updateActivePlotView(true, false);
  populatePlotsList();
  updateSelectedPlots();
  recolourCurves();
  replot();
}


void OCurvesUI::sourcesContextMenu(const QPoint &where)
{
  QWidget *w = qobject_cast<QWidget *>(sender());
  QPoint pos = where;
  if (w)
  {
    pos = w->mapToGlobal(where);
  }
  _lastContextPos = pos;

  if (_sourcesContextMenu)
  {
    _sourcesContextMenu->popup(pos);
  }
}


void OCurvesUI::plotsSelectionChanged()
{
  if (_suppressEvents)
  {
    return;
  }

  if (_ui->plotsList->selectedItems().isEmpty())
  {
    _ui->propertiesDock->close();
  }

  updateActivePlotView(false, true);
  recolourCurves();
  replot();
}


void OCurvesUI::plotsContextMenu(const QPoint &where)
{
  QWidget *w = qobject_cast<QWidget *>(sender());
  QPoint pos = where;
  if (w)
  {
    pos = w->mapToGlobal(where);
  }
  _lastContextPos = pos;

  if (_plotsContextMenu)
  {
    _plotsContextMenu->popup(pos);
  }
}


void OCurvesUI::curveAdded(PlotInstance *curve)
{
  addToSourceList(curve->source().name());
  updateSelectedSources();
  populatePlotsList();
  updateSelectedPlots();
}


void OCurvesUI::beginNewCurves()
{
}


void OCurvesUI::endNewCurves()
{
  populatePlotsList();
  updateSelectedPlots();
  recolourCurves();
  replot();
}


void OCurvesUI::curveComplete(PlotInstance * /*curve*/)
{
  replot();
}


void OCurvesUI::curveLoadingComplete()
{
  // Select all curves if nothing currently selected.
  if (_ui->plotsList->selectedItems().count() == 0)
  {
    for (int i = 0; i < _ui->plotsList->count(); ++i)
    {
      QListWidgetItem *item = _ui->plotsList->item(i);
      item->setSelected(true);
    }
  }
  replot();
}


void OCurvesUI::sourceDataChanged(const PlotSource *source)
{
  // Look for explicit time expressions in this source.
  QList<const PlotExpression *> regenExpressions;
  for (unsigned i = 0; i < source->curveCount(); ++i)
  {
    const PlotInstance *curve = source->curve(i);
    if (curve && curve->explicitTime() && curve->expression())
    {
      regenExpressions << curve->expression();
    }
  }

  // Remove all curves using the collated expressions.
  for (const PlotExpression *exp : regenExpressions)
  {
    _curves->removeUsingExpression(exp);
  }

  if (!regenExpressions.empty())
  {
    bool createGenerator = false;
    bool regenAll = false;
    if (PlotExpressionGenerator *expressionGenerator = qobject_cast<PlotExpressionGenerator *>(_loader))
    {
      // Add to the current generator.
      switch (expressionGenerator->addExpressions(regenExpressions))
      {
      case PlotExpressionGenerator::AER_QueuedPartial:
        regenAll = true;  // Regenerate all expressions to be sure.
      // No break.
        // FALLTHROUGH

      case PlotExpressionGenerator::AER_AlreadyComplete:
      default:
        // Failed to queue all expressions or unknown state.
        // Trigger restart of expression generation.
        createGenerator = true;
        break;

      case PlotExpressionGenerator::AER_Queued:
        createGenerator = false;
        break;
      }
      createGenerator = !expressionGenerator->addExpressions(regenExpressions);
    }
    else if (_loader)
    {
      // Ensure we will regenerate expressions.
      _postLoaderAction = PLA_GenerateExpressions;
    }
    else
    {
      createGenerator = true;
    }

    if (createGenerator)
    {
      // Need to create a new PlotExpressionGenerator and install it now.
      QStringList sourceFiles;
      _curves->enumerateFileSources(sourceFiles);
      PlotGenerator *newLoader;
      if (!regenAll)
      {
        newLoader = new PlotExpressionGenerator(_curves, regenExpressions, sourceFiles);
      }
      else
      {
        newLoader = new PlotExpressionGenerator(_curves, _expressions->expressions(), sourceFiles);
      }
      activateLoader(newLoader, PLA_CheckExpressions);
    }
  }
}


void OCurvesUI::loadComplete(int curveCount)
{
  PlotGenerator *source = qobject_cast<PlotGenerator *>(sender());
  if (!source)
  {
    // Sender has already been deleted. Ignore the event.
    return;
  }

  (void)curveCount;

  PostLoaderAction nextAction = completeLoad(source);

  if (nextAction == PLA_GenerateExpressions && !_expressions->expressions().isEmpty())
  {
    QStringList sourceFiles;
    _curves->enumerateFileSources(sourceFiles);
    PlotGenerator *newLoader = new PlotExpressionGenerator(_curves, _expressions->expressions(), sourceFiles);
    activateLoader(newLoader, PLA_CheckExpressions);
  }
  else if (nextAction == PLA_CheckExpressions)
  {
    bool refreshPlots = false;
    // Generally here because an expression was removed after it had already been generated.
    // Iterate the generated plots. Remove any which are no longer in _expressions.
    // Must duplicate the curve list because we'll be modifying the curves object.
    QList<PlotInstance *> curveList = _curves->curves().list();
    for (auto iter = curveList.begin(); iter != curveList.end(); ++iter)
    {
      PlotInstance *curve = *iter;
      if (curve->expression())
      {
        // Expression based curve. Is it still valid?
        if (!_expressions->contains(curve->expression()))
        {
          // Dead expression. Remove the plot.
          _curves->removeCurve(curve);
          curve = nullptr;  // Clear dead pointer.
          refreshPlots = true;
        }
      }
    }

    if (refreshPlots)
    {
      populatePlotsList();
      updateSelectedPlots();
      recolourCurves();
    }
  }
}


void OCurvesUI::legendItemClicked(const QVariant &itemInfo, int /*index*/)
{
  QwtLegend *legend = qobject_cast<QwtLegend *>(sender());

  if (!legend)
  {
    return;
  }

  QwtPlotItem *item = nullptr;
  if (legend == _legend)
  {
    item = (_splitView->activeView()) ? _splitView->activeView()->plot()->infoToItem(itemInfo) : nullptr;
  }
  else if (QwtPlot *plot = qobject_cast<QwtPlot *>(legend->parent()))
  {
    item = plot->infoToItem(itemInfo);
  }

  if (!item || item->rtti() != PlotDataCurve::Rtti)
  {
    return;
  }

  PlotDataCurve *curveItem = static_cast<PlotDataCurve *>(item);
  if (!curveItem)
  {
    return;
  }

  PlotInstance &curve = curveItem->curve();
  _properties->setActiveCurve(&curve);
  _ui->propertiesDock->setVisible(true);
}


void OCurvesUI::sourcesSelectOnlyCurrent()
{
  // Select only the source under the cursor.
  QPoint p = _ui->sourcesList->mapFromGlobal(_lastContextPos);
  if (QListWidgetItem *item = _ui->sourcesList->itemAt(p))
  {
    _ui->sourcesList->clearSelection();
    item->setSelected(true);
  }
}


void OCurvesUI::sourcesReloadCurrent()
{
  QPoint p = _ui->sourcesList->mapFromGlobal(_lastContextPos);
  if (QListWidgetItem *item = _ui->sourcesList->itemAt(p))
  {
    QString reloadName = item->text();
    QStringList reloadList;
    // Search for file curves matching reloadName w
    Curves::CurveList curves = _curves->curves();
    for (PlotInstance *curve : curves)
    {
      const PlotSource &source = curve->source();
      if (source.type() == PlotSource::File && source.name().compare(reloadName) == 0)
      {
        if (!reloadList.contains(source.fullName()))
        {
          reloadList << source.fullName();
        }
      }
    }
    curves.release();

    if (!reloadList.empty())
    {
      // Remove the current selection and reload.
      removeCurvesWithSource(reloadName);
      // Reload items.
      load(reloadList);
    }
  }
}

void OCurvesUI::sourcesRemoveCurrent()
{
  // Select only the plot under the cursor.
  QPoint p = _ui->sourcesList->mapFromGlobal(_lastContextPos);
  if (QListWidgetItem *item = _ui->sourcesList->itemAt(p))
  {
    removeCurvesWithSource(item->text());
    delete item;
  }
}


void OCurvesUI::plotsSelectOnlyCurrent()
{
  // Select only the plot under the cursor.
  QPoint p = _ui->plotsList->mapFromGlobal(_lastContextPos);
  if (QListWidgetItem *item = _ui->plotsList->itemAt(p))
  {
    _ui->plotsList->clearSelection();
    item->setSelected(true);
  }
}


void OCurvesUI::expressionAdded(PlotExpression *expression)
{
  if (PlotExpressionGenerator *expressionLoader = qobject_cast<PlotExpressionGenerator *>(_loader))
  {
    if (expressionLoader->addExpression(expression))
    {
      // Will generate the new plot.
      return;
    }
  }

  if (_loader)
  {
    // Already have a loader which is not an expression generator. Let it complete and expressions
    // will be generated afterwards.
    _postLoaderAction = PLA_GenerateExpressions;
    return;
  }

  // Failed to add to existing loader or there is no loader.
  // Generate the new expression.
  QStringList sourceFiles;
  _curves->enumerateFileSources(sourceFiles);
  //  if (!sourceFiles.empty())
  {
    QList<PlotExpression *> explist;
    explist << expression;
    PlotGenerator *newLoader = new PlotExpressionGenerator(_curves, explist, sourceFiles);
    activateLoader(newLoader, PLA_CheckExpressions);
  }
}


void OCurvesUI::expressionRemoved(const PlotExpression *expression)
{
  bool canRemoveNow = true;
  if (PlotExpressionGenerator *expressionLoader = qobject_cast<PlotExpressionGenerator *>(_loader))
  {
    if (!expressionLoader->removeExpression(expression))
    {
      // Remove generated plots.
      _postLoaderAction = PLA_CheckExpressions;
      canRemoveNow = false;
    }
  }

  if (canRemoveNow)
  {
    if (_curves->removeUsingExpression(expression))
    {
      populatePlotsList();
      updateSelectedPlots();
      recolourCurves();
    }
  }
}


void OCurvesUI::setupToolbars()
{
  // Initialise the main toolbar.
  QToolBar *toolbar = addToolBar(tr("Main Toolbar"));
  toolbar->setObjectName("mainToolbar");
  toolbar->addAction(_ui->actionOpen);
  toolbar->addAction(_ui->actionConnect);
  toolbar->addAction(_ui->actionReload);
  toolbar->addAction(_ui->actionClear);
  _toolbarWidgets = new ToolbarWidgets(this);
  toolbar->insertWidget(nullptr, _toolbarWidgets);

  _viewToolbar = new PlotViewToolbar(this);
  _viewToolbar->setObjectName("viewToolbar");
  _viewToolbar->setWindowTitle(tr("View Toolbar"));
  addToolBar(_viewToolbar);
}


void OCurvesUI::setupBookmarks()
{
  // Add 10 bookmark each for various key presses:
  // - Control + [0-9] : goto
  // - Control + Shift + [0-9] : set

  REFERENCE_SETTINGS(settings);
  for (int i = 0; i < 10; ++i)
  {
    const int bookmarkId = i + 1;
    QString bookmarkName;
    bool enabled = bookmarks::name(bookmarkName, settings, bookmarkId);
    QString shortDisplay = bookmarkMenuName(bookmarkId);
    QString displayName = bookmarkMenuName(bookmarkId, bookmarkName);

    QAction *gotoAction = _ui->menuGotoBookmark->addAction(displayName);
    QAction *setAction = _ui->menuSetBookmark->addAction(shortDisplay);
    QAction *clearAction = _ui->menuClearBookmark->addAction(displayName);

    gotoAction->setEnabled(enabled);
    clearAction->setEnabled(enabled);

    const int keyIndex = (i + 1) % 10;
    int primaryKey = Qt::Key_0 + keyIndex;
    gotoAction->setShortcut(QKeySequence(Qt::CTRL + primaryKey));
    setAction->setShortcut(QKeySequence(Qt::CTRL + Qt::ALT + primaryKey));

    gotoAction->setData(bookmarkId);
    setAction->setData(bookmarkId);
    clearAction->setData(bookmarkId);

    connect(gotoAction, &QAction::triggered, this, &OCurvesUI::gotoBookmark);
    connect(setAction, &QAction::triggered, this, &OCurvesUI::setBookmark);
    connect(clearAction, &QAction::triggered, this, static_cast<void (OCurvesUI::*)()>(&OCurvesUI::clearBookmark));
  }

  connect(_ui->actionClearBookmarks, &QAction::triggered, this, &OCurvesUI::clearBookmarks);
}


namespace
{
  QAction *findMenuActionWithDataId(QMenu *menu, int id)
  {
    for (QObject *child : menu->children())
    {
      if (QAction *action = qobject_cast<QAction *>(child))
      {
        bool ok = false;
        int actionId = action->data().toInt(&ok);
        if (ok && actionId == id)
        {
          return action;
        }
      }
    }

    return nullptr;
  }
}


QString OCurvesUI::bookmarkMenuName(unsigned id, const QString &name)
{
  QString displayName;
  if (id != 10)
  {
    displayName = tr("Bookmark &%1").arg(id);
  }
  else
  {
    displayName = tr("Bookmark 1&0");
  }

  if (!name.isEmpty())
  {
    displayName += ":" + name;
  }

  return displayName;
}


QAction *OCurvesUI::findGotoBookmarkAction(int id)
{
  return findMenuActionWithDataId(_ui->menuGotoBookmark, id);
}


QAction *OCurvesUI::findSetBookmarkAction(int id)
{
  return findMenuActionWithDataId(_ui->menuSetBookmark, id);
}


QAction *OCurvesUI::findClearBookmarkAction(int id)
{
  return findMenuActionWithDataId(_ui->menuClearBookmark, id);
}


void OCurvesUI::replot(bool force)
{
  if (_timeSinceLastPlot->elapsed() > REFRESH_INTERVAL || force)
  {
    _splitView->replot();
    _timeSinceLastPlot->restart();
  }
}


void OCurvesUI::stopLoad()
{
  completeLoad(_loader);
}


OCurvesUI::PostLoaderAction OCurvesUI::completeLoad(PlotGenerator *loader)
{
  PostLoaderAction nextAction = PLA_None;
  if (loader)
  {
    if (loader == _loader)
    {
      _loader = nullptr;
      nextAction = _postLoaderAction;
      _postLoaderAction = PLA_None;
    }
    loader->abortLoad();
    if (!loader->wait(2000))
    {
      loader->terminate();
    }
    loader->deleteLater();
  }

  return nextAction;
}


void OCurvesUI::clearCurves()
{
  endStreams();
  _curves->clearCurves();
  _ui->sourcesList->clear();
}


bool OCurvesUI::addToSourceList(const QString &displayName)
{
  // Search for a matching item.
  bool found = false;
  for (int i = 0; !found && i < _ui->sourcesList->count(); ++i)
  {
    if (_ui->sourcesList->item(i)->text().compare(displayName) == 0)
    {
      found = true;
    }
  }

  if (!found)
  {
    _ui->sourcesList->addItem(displayName);

    // Select this item if it's the first item and we aren't restoring a view.
    PlotView *activeView = _splitView->activeView();
    if (_ui->sourcesList->count() == 1 && (activeView == nullptr || activeView->activeSourceNames().isEmpty()))
    {
      // First item. Select it.
      _ui->sourcesList->setItemSelected(_ui->sourcesList->item(0), true);
    }
  }

  return !found;
}


void OCurvesUI::recolourCurves()
{
  QStringList fullActiveList;
  QStringList fullCurveNameList;
  _splitView->collateActive(fullActiveList, fullCurveNameList);
  updateColours(fullActiveList, fullCurveNameList);
}


void OCurvesUI::updateActivePlotView(bool updateSources, bool updatePlots)
{
  // Only affects the active view.
  if (PlotView *view = _splitView->activeView())
  {
    QStringList activeSources, activeCurves;
    if (updateSources)
    {
      // Updating the active sources. Read the UI control.
      for (int i = 0; i < _ui->sourcesList->count(); ++i)
      {
        QListWidgetItem *item = _ui->sourcesList->item(i);
        if (item->isSelected())
        {
          activeSources.append(item->text());
        }
      }
    }
    else
    {
      // Use existing sources.
      activeSources = view->activeSourceNames();
    }

    if (updatePlots)
    {
      // Updating the active curves. Read the UI control.
      for (int i = 0; i < _ui->plotsList->count(); ++i)
      {
        QListWidgetItem *item = _ui->plotsList->item(i);
        if (item->isSelected())
        {
          activeCurves.append(item->text());
        }
      }
    }
    else
    {
      // Use existing curves.
      activeCurves = view->visibleCurveNames();
    }

    view->updateActive(activeSources, activeCurves);
  }
}


void OCurvesUI::updateSelectedSources()
{
  // Read from the active view.
  if (PlotView *view = _splitView->activeView())
  {
    bool changed = false;
    QStringList activeSources = view->activeSourceNames();
    // Write to the plots list.
    _suppressEvents = true;
    for (int i = 0; i < _ui->sourcesList->count(); ++i)
    {
      QListWidgetItem *item = _ui->sourcesList->item(i);
      bool shouldSelect = activeSources.contains(item->text());
      bool selected = item->isSelected();
      if (selected != shouldSelect)
      {
        changed = true;
        item->setSelected(shouldSelect);
      }
    }
    _suppressEvents = false;

    if (changed)
    {
      populatePlotsList();
    }
  }
}


void OCurvesUI::populatePlotsList()
{
  QStringList selectedSources;
  for (QListWidgetItem *item : _ui->sourcesList->selectedItems())
  {
    selectedSources.append(item->text());
  }

  _suppressEvents = true;
  _ui->plotsList->clear();
  _suppressEvents = false;
  QStringList plotNames;
  for (const PlotInstance *curve : _curves->curves())
  {
    if (selectedSources.contains(curve->source().name()))
    {
      // Show this item.
      if (!plotNames.contains(curve->name()))
      {
        plotNames.append(curve->name());
      }
    }
  }

  for (const QString &name : plotNames)
  {
    _ui->plotsList->addItem(name);
  }
}


void OCurvesUI::updateSelectedPlots()
{
  // Read from the active view.
  if (PlotView *view = _splitView->activeView())
  {
    QStringList activeCurves = view->visibleCurveNames();
    // Write to the plots list.
    _suppressEvents = true;
    for (int i = 0; i < _ui->plotsList->count(); ++i)
    {
      QListWidgetItem *item = _ui->plotsList->item(i);
      bool shouldSelect = activeCurves.contains(item->text());
      if (shouldSelect != item->isSelected())
      {
        item->setSelected(activeCurves.contains(item->text()));
      }
    }
    _suppressEvents = false;
  }
}


void OCurvesUI::endStreams()
{
  if (_streams)
  {
    // FIXME: this generates signals which will be processed later,
    // but we are to delete curves immediately below. Need to resolve
    // PlotInstance sharing.
    // A reference count and wrapped pointer may help.
    _streams->stop();
    _streams->deleteLater();
    _streams = nullptr;
  }
}


void OCurvesUI::setTimeControls(PlotGenerator *generator)
{
  if (_toolbarWidgets->timeColumnCheck()->isChecked())
  {
    generator->setTimeColumn(_toolbarWidgets->timeColumnSpin()->value());
  }
  else
  {
    generator->setTimeColumn(0);
  }
  generator->setTimeScale(_toolbarWidgets->timeScaleEdit()->text().toDouble());
  generator->setTimeScale(_toolbarWidgets->timeScaleEdit()->text().toDouble());
  generator->setControlFlags(PlotGenerator::RelativeTime, _toolbarWidgets->relativeTimeCheck()->isChecked());
}


void OCurvesUI::activateLoader(PlotGenerator *newLoader, PostLoaderAction nextAction)
{
  stopLoad();
  _loader = newLoader;
  setTimeControls(_loader);

  LoadProgress *progress = new LoadProgress();
  this->statusBar()->layout()->addWidget(progress);
  connectLoader(_loader, progress);

  _postLoaderAction = nextAction;
  _loader->start();
  replot();
}


void OCurvesUI::connectLoader(PlotGenerator *loader, LoadProgress *progress)
{
  connect(loader, &PlotGenerator::beginNewCurves, this, &OCurvesUI::beginNewCurves);
  connect(loader, &PlotGenerator::endNewCurves, this, &OCurvesUI::endNewCurves);
  connect(loader, &PlotGenerator::loadComplete, this, &OCurvesUI::loadComplete);

  if (progress)
  {
    connect(loader, &PlotGenerator::itemName, progress, &LoadProgress::itemName);
    connect(loader, &PlotGenerator::itemProgress, progress, &LoadProgress::itemProgress);
    connect(loader, &PlotGenerator::overallProgress, progress, &LoadProgress::overallProgress);
    connect(loader, &PlotGenerator::loadComplete, progress, &LoadProgress::loadComplete);

    connect(progress, &LoadProgress::cancel, loader, &PlotGenerator::abortLoad);
  }
}


void OCurvesUI::viewAdded(PlotView *view)
{
  connect(view->plot(), &QwtPlot::legendDataChanged, _legend, &QwtLegend::updateLegend);
  connect(view, &PlotView::legendChanged, this, &OCurvesUI::viewLegendChanged);
}


void OCurvesUI::activeViewChanged(PlotView *newView, PlotView * /*oldView*/)
{
  if (newView)
  {
    // Update the UI lists to show the active plot view selections.
    updateSelectedSources();
    updateSelectedPlots();
    newView->plot()->updateLegend();
  }
}


void OCurvesUI::viewLegendChanged(QwtLegend *legend, int /*position*/)
{
  if (legend)
  {
    // Disconnect in case already connected.
    disconnect(legend, &QwtLegend::clicked, this, &OCurvesUI::legendItemClicked);
    connect(legend, &QwtLegend::clicked, this, &OCurvesUI::legendItemClicked);
  }
}


void OCurvesUI::gotoBookmark()
{
  if (QAction *action = qobject_cast<QAction *>(sender()))
  {
    bool ok = false;
    int bookmarkId = action->data().toInt(&ok);
    if (ok)
    {
      _activeBookmark = bookmarkId;
      REFERENCE_SETTINGS(settings);
      VariantMap curveProperties;
      bookmarks::restoreBookmak(settings, bookmarkId, this, &curveProperties);
      _curves->setCurvePropertiesMap(curveProperties);
      replot(true);
    }
  }
}


void OCurvesUI::setBookmark()
{
  if (QAction *action = qobject_cast<QAction *>(sender()))
  {
    bool ok = false;
    int bookmarkId = action->data().toInt(&ok);
    if (ok)
    {
      QAction *action = findGotoBookmarkAction(bookmarkId);
      if (action)
      {
        action->setEnabled(true);
      }

      action = findClearBookmarkAction(bookmarkId);
      if (action)
      {
        action->setEnabled(true);
      }

      // Ask for confirmation.
      REFERENCE_SETTINGS(settings);
      QString name;
      QInputDialog dialog(this);
      dialog.setInputMode(QInputDialog::TextInput);

      if (bookmarks::name(name, settings, bookmarkId))
      {
        dialog.setWindowTitle(tr("Overwrite Bookmark"));
        dialog.setLabelText(tr("Enter a new name (optional) and click 'OK' to overwrite.").arg(bookmarkId));
        dialog.setTextValue(name);
      }
      else
      {
        dialog.setWindowTitle(tr("Set Bookmark"));
        dialog.setLabelText(tr("Setting bookmark %1. Optionally name the bookmark.").arg(bookmarkId));
      }

      if (dialog.exec() != QDialog::Accepted)
      {
        // Cancelled.
        return;
      }
      name = dialog.textValue();
      bookmarks::setBookmark(settings, bookmarkId, name, this);

      QString displayName = bookmarkMenuName(bookmarkId, name);
      if (name.isEmpty())
      {
        QMessageBox::information(this, tr("Set Bookmark"), tr("Bookmark %1 set").arg(bookmarkId));
      }
      else
      {
        QMessageBox::information(this, tr("Set Bookmark"), tr("Bookmark %1 set").arg(name));
      }

      QAction *gotoAction, *clearAction;
      gotoAction = findGotoBookmarkAction(bookmarkId);
      clearAction = findClearBookmarkAction(bookmarkId);
      gotoAction->setText(displayName);
      clearAction->setText(displayName);
    }
  }
}


void OCurvesUI::clearBookmark()
{
  if (QAction *action = qobject_cast<QAction *>(sender()))
  {
    bool ok = false;
    int bookmarkId = action->data().toInt(&ok);
    if (ok)
    {
      clearBookmark(bookmarkId);
      QMessageBox::information(this, tr("Clear Bookmark").arg(bookmarkId), tr("Bookmark %1 cleared").arg(bookmarkId));
    }
  }
}


void OCurvesUI::clearBookmark(int id)
{
  REFERENCE_SETTINGS(settings);
  bookmarks::clearBookmark(settings, id);

  QAction *action = findGotoBookmarkAction(id);
  if (action)
  {
    action->setEnabled(false);
  }

  action = findClearBookmarkAction(id);
  if (action)
  {
    action->setEnabled(false);
  }
}


void OCurvesUI::clearBookmarks()
{
  for (int i = 0; i < 10; ++i)
  {
    clearBookmark(i + 1);
  }
}


void OCurvesUI::exportBookmarks()
{
  QString fileFilter = QString("%1 (*.ini);;%3 (*.*)")
                       .arg(tr("Ini Files"))
                       .arg(tr("All Files"));
  QString activeFilter = QString("%1 (*.ini)").arg(tr("Ini Files"));

  QFileDialog filesDialog(this, tr("Export Bookmarks"), QString(), fileFilter);
  filesDialog.setAcceptMode(QFileDialog::AcceptSave);
  filesDialog.setFileMode(QFileDialog::AnyFile);
  filesDialog.selectNameFilter(activeFilter);

  if (filesDialog.exec() == QDialog::Accepted)
  {
    REFERENCE_SETTINGS(settings);
    QSettings bksettings(filesDialog.selectedFiles().front(), QSettings::IniFormat);
    bookmarks::migrate(bksettings, settings, 10);
  }
}


void OCurvesUI::importBookmarks()
{
  QString fileFilter = QString("%1 (*.ini);;%3 (*.*)")
                       .arg(tr("Ini Files"))
                       .arg(tr("All Files"));
  QString activeFilter = QString("%1 (*.ini)").arg(tr("Ini Files"));

  QFileDialog filesDialog(this, tr("Import Bookmarks"), QString(), fileFilter);
  filesDialog.setAcceptMode(QFileDialog::AcceptOpen);
  filesDialog.setFileMode(QFileDialog::ExistingFile);
  filesDialog.selectNameFilter(activeFilter);

  if (filesDialog.exec() == QDialog::Accepted)
  {
    REFERENCE_SETTINGS(settings);
    QSettings bksettings(filesDialog.selectedFiles().front(), QSettings::IniFormat);
    bookmarks::migrate(settings, bksettings, 10);
    settings.sync();

    for (int i = 0; i < 10; ++i)
    {
      const unsigned bookmarkId = i + 1;
      QString name, displayName;
      bool isSet = bookmarks::name(name, settings, bookmarkId);
      displayName = bookmarkMenuName(bookmarkId, name);

      QAction *gotoAction = findGotoBookmarkAction(bookmarkId);
      QAction *clearAction = findClearBookmarkAction(bookmarkId);

      if (gotoAction)
      {
        gotoAction->setText(displayName);
        gotoAction->setEnabled(isSet);
      }
      if (clearAction)
      {
        clearAction->setText(displayName);
        clearAction->setEnabled(isSet);
      }
    }
  }
}


void OCurvesUI::restoreLastSession()
{
  REFERENCE_SETTINGS(settings);
  VariantMap curveProperties;
  bookmarks::restoreBookmak(settings, 0, this, &curveProperties);
  _curves->setCurvePropertiesMap(curveProperties);
}


void OCurvesUI::viewHelp()
{
  // Find the target file.
  QDir dir(qApp->applicationDirPath());
  QUrl helpUrl;

  // Try as a sub-directory of the application path.
  // Primarily a Windows based pattern.
  if (QFile::exists(dir.filePath("userdoc/index.html")))
  {
    helpUrl = QUrl::fromLocalFile(dir.filePath("userdoc/index.html"));
  }

  // Try up a directory. Primarily a Linux style installation (exectuable in a 'bin' directory).
  // FIXME: needs to be resolved when the installation paths for LINUX are standardised.
  if (helpUrl.isEmpty())
  {
    dir.cdUp();
    if (QFile::exists(dir.filePath("userdoc/index.html")))
    {
      helpUrl = QUrl::fromLocalFile(dir.filePath("userdoc/index.html"));
    }
  }

  // Try for a MacOS bundle installation pattern.
  if (helpUrl.isEmpty())
  {
    if (QFile::exists(dir.filePath("Resources/userdoc/index.html")))
    {
      helpUrl = QUrl::fromLocalFile(dir.filePath("Resources/userdoc/index.html"));
    }
  }

  if (!helpUrl.isEmpty())
  {
    QDesktopServices::openUrl(helpUrl);
  }
  else
  {
    QMessageBox::warning(this, tr("Missing Help"), tr("User documentation is unavailable."));
  }
}


void OCurvesUI::showAbout()
{
  QString versionPart;
  QString infoPart;

  versionPart = QString("%1 %2").arg(tr("Version ")).arg(ocutil::versionString());

  // TODO:
  //infoPart;

  QMessageBox::about(this, tr("About %1").arg(_originalWindowTitle),
                     QString("%1\n%2").arg(versionPart).arg(infoPart)
                    );
}


void OCurvesUI::removeCurvesWithSource(const QString &sourceName)
{
  bool loadingSelectedSource = false;
  for (const PlotInstance *curve : _curves->loadingCurves())
  {
    if (curve->source().name() == sourceName)
    {
      loadingSelectedSource = true;
      break;
    }
  }

  if (loadingSelectedSource)
  {
    // Source is currently being loaded. Stop loading.
    stopLoad();
  }

  // Build remove list to avoid thread deadlock.
  QList<const PlotInstance *> removeList;
  for (const PlotInstance *curve : _curves->curves())
  {
    if (curve->source().name() == sourceName)
    {
      removeList.append(curve);
    }
  }

  for (const PlotInstance *curve : removeList)
  {
    _curves->removeCurve(curve);
  }
}


void OCurvesUI::updateColours(const QStringList &activeSources, const QStringList &activeCurves)
{
  if (_colours.isEmpty())
  {
    return;
  }

  // Assign colours based on active and selected curves.
  Curves::CurveList curveList = _curves->curves();
  int colourIndex = 0;
  for (PlotInstance *curve : curveList)
  {
    if (activeSources.contains(curve->source().name()) && activeCurves.contains(curve->name()))
    {
      if (!curve->explicitColour())
      {
        curve->setColour(_colours[colourIndex], false);
        colourIndex = (colourIndex + 1) % _colours.count();
        _curves->invalidate(curve);
      }
    }
  }
}


void OCurvesUI::timerEvent(QTimerEvent *)
{
  if (_curves->migrateLoadingData())
  {
    replot();
  }
}


void OCurvesUI::showEvent(QShowEvent *)
{
  REFERENCE_SETTINGS(settings);
  loadSettings(settings, true);
}


void OCurvesUI::closeEvent(QCloseEvent *)
{
  endStreams();
  stopLoad();
  REFERENCE_SETTINGS(settings);
  saveSettings(settings, true);
}


void OCurvesUI::dragEnterEvent(QDragEnterEvent *event)
{
  if (canAcceptDrop(*event->mimeData()))
  {
    event->acceptProposedAction();
  }
}


void OCurvesUI::dropEvent(QDropEvent *event)
{
  QStringList fileList = extractDroppedFileList(*event->mimeData());
  load(fileList, true);
}


bool OCurvesUI::canAcceptDrop(const QMimeData &mimeData) const
{
  return mimeData.hasUrls() || mimeData.hasText();
}


QStringList OCurvesUI::extractDroppedFileList(const QMimeData &mimeData) const
{
  QStringList fileList;
  if (mimeData.hasUrls())
  {
    foreach (const QUrl &url, mimeData.urls())
    {
      if (url.isLocalFile())
      {
        fileList << url.toLocalFile();
      }
    }
  }
  else if (mimeData.hasText())
  {
    QStringList proposedList = mimeData.text().split(QChar(';'), QString::SkipEmptyParts);
    foreach (const QString &proposedFile, proposedList)
    {
      if (QFile::exists(proposedFile))
      {
        fileList << proposedFile;
      }
    }
  }

  return fileList;
}
