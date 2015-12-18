//
// author Kazys Stepanas
//
// Copyright (c) 2012
//
#ifndef __PLOT_H_
#define __PLOT_H_

#include "ocurvesconfig.h"

#include "timesampling.h"

#include <QMainWindow>
#include <QList>
#include <QRgb>
#include <QStringList>
#include <QVector>

class QElapsedTimer;
class QMenu;
class QMimeData;
class QSettings;
class QSplitter;
class QwtLegend;

class Curves;
class CurveProperties;
class PlotExpression;
class PlotGenerator;
class PlotInstance;
class PlotSource;
class PlotViewToolbar;
class LoadProgress;
class RealTimePlot;
class ToolbarWidgets;
class Expressions;
class ExpressionsView;
class PlotView;
class SplitPlotView;

namespace Ui
{
  class OCurvesUI;
}

#define REFERENCE_SETTINGS(s) QSettings s(QSettings::IniFormat, QSettings::UserScope, "ocurves");

/// @ingroup ui
/// The main Open Curves UI window.
///
/// The main UI holds the major UI components as well as the main data model classes.
/// It also manages background loading and progressive plotting data sets.
///
/// The main UI consists of the following components:
/// - The @c splitPlotView() for plotting and splitting.
/// - The @c ExpressionsView.
/// - The shared legend.
/// - The source and curve list widgets.
///
/// The data components are:
/// - @c Curves
/// - @c Expressions
class OCurvesUI : public QMainWindow
{
  Q_OBJECT

public:
  /// Constructor.
  /// @param parent The widget parent.
  explicit OCurvesUI(QWidget *parent = nullptr);

  /// Destructor.
  ~OCurvesUI();

  /// Access the primary @c Curves data.
  /// @return The complete @c Curves set
  inline Curves *curves() const { return _curves; }

  /// Access the @c Expressions data.
  /// @return The complete @c Expressions set.
  inline Expressions *expressions() const { return _expressions; }

  /// Access the view for expressions.
  /// @return The @c ExpressionsView.
  inline ExpressionsView *expressionsView() const { return _expressionsView; }

  /// Access the main plot view. Supports splitting.
  /// @return The @c SplitPlotView.
  inline SplitPlotView *splitPlotView() const { return _splitView; }

  /// Loads saved settings saved by @c saveSettings() and applies them.
  ///
  /// Geometry settings may be excluded as they cannot be properly restored
  /// from the constructor. Use @c loadWindowSettings() to do so manually.
  ///
  /// @param settings The settings object to load from.
  /// @param loadGeometry Restore geometry settings?
  void loadSettings(QSettings &settings, bool loadGeometry = true);

  /// Loads and restores window geometry and other settings from the default settings.
  void loadWindowsSettings();

  /// Loads and restores window geometry and other settings.
  ///
  /// This cannot be done from the constructor.
  ///
  /// @param settings The settings object to load from.
  void loadWindowsSettings(QSettings &settings);

  /// Saves the application state and settings including both UI and data settings.
  ///
  /// The UI state for all windows and children is saved. It serialises the current
  /// colour set and makes calls to save the state of the @c SplitPlotView and @c Expressions.
  ///
  /// The current state may be optionally bookmarked as "bookmark0" which allows the user to
  /// restore the last session.
  ///
  /// @param settings The settings object to save to.
  /// @param bookmarkCurrent True to bookmark the currently open files, selections, etc as bookmark0.
  void saveSettings(QSettings &settings, bool bookmarkCurrent = false);

public slots:
  /// Request loading of the given file.
  ///
  /// May optionally append to any currently loading file list when @p append is true.
  /// When false (default), any current load process is aborted.
  ///
  /// @param plotFile The file to load.
  /// @param append True to append to the active list of files to load.
  void load(const QString &plotFile, bool append = false);

  /// Request loading of the given list of files.
  ///
  /// May optionally append to any currently loading file list when @p append is true.
  /// When false (default), any current load process is aborted.
  ///
  /// @param plotFiles List of files to load.
  /// @param append True to append to the active list of files to load.
  /// @param plotTiming Optionally provides @c TimeSampling control on a per file basis.
  ///     For each element in @p plotFiles, the corresponding @p plotTiming entry overrides
  ///     the time column, scale base and flags. May be null, and insufficient entries are handled.
  ///     Additional entries are ignored.
  void load(const QStringList &plotFiles, bool append = false, QVector<TimeSampling> *plotTiming = nullptr);

  /// Show a dialog allowing the user to select data files to open.
  ///
  /// Aborts current loading.
  void openDataFiles();

  /// Show a dialog allowing the user to select a real time source XML file to load.
  ///
  /// Aborts current loading.
  ///
  /// @todo Add a reference to the real time source specifications here.
  void connectToRealtimeSource();

  /// Trigger a reload of a currently loaded sources. Does not include ones pending load.
  void reloadPlots();

  /// Clears the display, removing all sources.
  void clearPlots();

  /// Handler for selecting a single source by model index.
  /// @param index Index into the source list view model for the source to select.
  void sourceSelectOnlyIndex(const QModelIndex &index);

  /// Select only the source at @p row.
  /// @param row The row of the source to select. Range checked.
  void sourceSelectOnly(int row);

  /// Handler for selecting a single curve name by model index.
  /// @param index Index into the plot list view model for the source to select.
  void plotsSelectOnlyIndex(const QModelIndex &index);

  /// Select only the curve name at @p row.
  /// @param row The row of the curve name to select. Range checked.
  void plotsSelectOnly(int row);

  /// Copy the active view content to the system clipboard (if available).
  ///
  /// Invokes @c PlotView::copyToClipboard()
  void copyActiveView();

  /// Shows the colour set editing dialog.
  void editColours();

private slots:
  /// Handler of changes to the selected items in the sources UI list.
  void sourcesSelectionChanged();

  /// Context menu handler for the sources UI list.
  /// @param where Location to show the context menu (local).
  void sourcesContextMenu(const QPoint &where);

  /// Handler of changes to the selected items in the plots UI list.
  void plotsSelectionChanged();

  /// Context menu handler for the plots UI list.
  /// @param where Location to show the context menu (local).
  void plotsContextMenu(const QPoint &where);

  /// New curve defined (data pending).
  /// @param curve The new curve.
  void curveAdded(PlotInstance *curve);

  /// Begin defining new curves.
  void beginNewCurves();

  /// End defining new curves.
  void endNewCurves();

  /// Load complete
  /// @param curve Completed curve.
  void curveComplete(PlotInstance *curve);

  /// End marking curves as completed.
  void curveLoadingComplete();

  /// A source has had its data changed. Trigger regeneration of expression curves
  /// with explicit time values.
  /// @param source The changed source.
  void sourceDataChanged(const PlotSource *source);

  /// Loading complete from a @c PlotGenerator.
  ///
  /// This triggers the next @c PostLoaderAction.
  ///
  /// @param curveCount The number of curves added.
  void loadComplete(int curveCount);

  /// Show properties for the curve associated with the selected legend item.
  ///
  /// The @p itemInfo is converted to a @c PlotDataCurve via the owning
  /// @c QwtPlot. The owner is determined by either the parent of the
  /// legend which send the event, or by querying the active view.
  ///
  /// Properties for the associated @c PlotInstance are shown if a
  /// @c PlotDataCurve can be resolved, otherwise the call is ignored.
  ///
  /// @param itemInfo Data describing the selected item.
  /// @param index Not used.
  void legendItemClicked(const QVariant &itemInfo, int index);

  /// Select only source at the mouse location.
  void sourcesSelectOnlyCurrent();

  /// Reload the selected plot source. Only works for file sources.
  void sourcesReloadCurrent();

  /// Remove (unload) the selected source.
  void sourcesRemoveCurrent();

  /// Select only the curve at the mouse location.
  void plotsSelectOnlyCurrent();

  /// Handle expressions being added by queuing generation of curves from the expression.
  /// @param expression The new expression.
  void expressionAdded(PlotExpression *expression);

  /// Handle expressions being removed by destroying associated plots and preventing generation.
  /// @param expression The removed expression.
  void expressionRemoved(const PlotExpression *expression);

  /// Accept a new view, connecting it to the legend.
  /// @param view The new view.
  void viewAdded(PlotView *view);

  /// Handles changes to the active view, update the UI to suit.
  /// @param newView The newly active view.
  /// @param oldView The previously active view, if any.
  void activeViewChanged(PlotView *newView, PlotView *oldView);

  /// Handle changes to the legend for a @c PlotView.
  /// @param legend The new legend (if any).
  /// @param position The position of the legend as per @c QwtPlot::LegendPosition or
  ///   @c PlotView::SharedLegend.
  void viewLegendChanged(QwtLegend *legend, int position);

  /// Goto to the bookmark associated with the sender.
  void gotoBookmark();

  /// Set the bookmark associated with the sender. Shows naming dialog first.
  void setBookmark();

  /// Clear the bookmark associated with the sender. No confirmation required.
  void clearBookmark();

  /// Clear the bookmark with ID @p id. No confirmation.
  /// @param id The id of the bookmark to clear. Range checked.
  void clearBookmark(int id);

  /// Clear all bookmarks (except the last sesssion bookmark0).
  void clearBookmarks();

  /// Export bookmarks to INI file.
  ///
  /// Requests destination of the user.
  void exportBookmarks();

  /// Import bookmarks from INI file.
  ///
  /// Requests source file from the user.
  void importBookmarks();

  /// Restore the last session (from bookmark0).
  void restoreLastSession();

  /// View the user manual if available.
  void viewHelp();

  /// Show the "about" dialog.
  void showAbout();

protected:
  /// Removes all curves which have a @c PlotSource::name() mathcing @c sourceName.
  /// This function does not perform UI cleanup and UI components may need to be
  /// further updated to reflect the removal of curves.
  ///
  /// Loading will be stopped if the source is curently being loaded.
  ///
  /// @param sourceName The source name to match.
  void removeCurvesWithSource(const QString &sourceName);

  /// Recolours all visible @c PlotInstance objects by cycling through the colour set.
  ///
  /// This affects all visible @p PlotInstance objects in all views. Colours are assigned
  /// in turn from the current colour set, cycling back to the beginning of the set if
  /// there are more curves than colours.
  ///
  /// To be visible, a @c PlotInstance must have a @c PlotSource::name() which is
  /// present in the @p activeSources list and its own @c PlotInstance::name()
  /// must be in @p activeCurves.
  ///
  /// Curves with an explicit colour set are skipped.
  ///
  /// @param activeSources The list of sources from which to display curves.
  /// @param activeCurves The list of curves to show.
  void updateColours(const QStringList &activeSources, const QStringList &activeCurves);

  /// Periodic event used to migrate background loaded data.
  ///
  /// Calls @c Curves::migrateLoadingData().
  virtual void timerEvent(QTimerEvent *) override;

  /// Handles reveling the window by restoring stored settings.
  virtual void showEvent(QShowEvent *) override;

  /// Ensures loading is stopped and settings are saved.
  virtual void closeEvent(QCloseEvent *) override;

private:
  /// Action to take once the @c PlotGenerator completes.
  enum PostLoaderAction
  {
    PLA_None,
    PLA_GenerateExpressions,  ///< Run expression plot generator.
    PLA_CheckExpressions  ///< Check the loaded expressions. Remove invalid ones.
  };

  /// Initialise the toolbars.
  void setupToolbars();

  /// Setup bookmark events.
  void setupBookmarks();

  /// Generate a UI menu display name for a bookmark.
  /// @param id The bookmark ID. Only designed to deal with [1, 10].
  /// @param name Optional name for the bookmark.
  QString bookmarkMenuName(unsigned id, const QString &name = QString());

  /// Find the goto bookmark action for the given id.
  /// @param id The id of the bookmark of interest.
  /// @return The associated action or null if @p id is invalid.
  QAction *findGotoBookmarkAction(int id);

  /// Find the set bookmark action for the given id.
  /// @param id The id of the bookmark of interest.
  /// @return The associated action or null if @p id is invalid.
  QAction *findSetBookmarkAction(int id);

  /// Find the clear bookmark action for the given id.
  /// @param id The id of the bookmark of interest.
  /// @return The associated action or null if @p id is invalid.
  QAction *findClearBookmarkAction(int id);

  /// Request replotting of all active views and plots.
  ///
  /// The function includes a time delay to prevent excessive replotting.
  /// It will only effect the request if @p force is true, or if sufficient
  /// time has elapsed since the last call.
  /// @param force True to force replotting.
  void replot(bool force = false);

  /// Cancel any pending data load from any source. Includes files, expressions and real-time sources.
  void stopLoad();

  /// Completing loading of data from @p loader.
  /// @param load The loader to clean up loading for.
  /// @return The next action which should be taken.
  PostLoaderAction completeLoad(PlotGenerator *loader);

  /// Clear all existing curves.
  void clearCurves();

  /// Add @p displayName to the list of visible sources.
  ///
  /// The source is added only if it is not already present. The source is selected
  /// in the @c activeView() if this is the first source and the active view has no
  /// sources selected.
  ///
  /// @param displayName The display name for the source.
  /// @return True if the source has been newly added, false if it already existed.
  bool addToSourceList(const QString &displayName);

  /// Recolour curves based on all current views.
  /// @see @c updateColours()
  void recolourCurves();

  /// Update the active plot view from the UI list components.
  /// @param updateSources True to update the view's source list from the UI list selections.
  /// @param updatePlots True to update the view's curves list from the UI list selections.
  void updateActivePlotView(bool updateSources, bool updatePlots);

  /// Update the selected sources list UI display from the active plot view.
  void updateSelectedSources();

  /// Populate plots UI list to show all curves from active sources.
  void populatePlotsList();
  /// Update the displayed and selected plots list from the active plot view.
  void updateSelectedPlots();

  /// End any current real-time source streaming.
  void endStreams();

  /// Set time column, scaling and relative flag on @p generator.
  /// @param generator The loader to set time data for.
  void setTimeControls(PlotGenerator *generator);

  /// Set the active loade.
  /// @param newLoader The active loader.
  /// @param nextAction The action to take once @c newLoader successfully completes.
  void activateLoader(PlotGenerator *newLoader, PostLoaderAction nextAction);

  /// Connect a @c PlotGenerator events to this object and optionally to a progress display.
  /// @param loader The generator of interest.
  /// @param loader Optional progress control to connect.
  void connectLoader(PlotGenerator *loader, LoadProgress *progress = nullptr);

  /// Prime file drag and drop.
  /// @param event Event details.
  void dragEnterEvent(QDragEnterEvent *event) override;

  /// Handle file drag and drop, queuing loading.
  /// @param event Event details.
  void dropEvent(QDropEvent *event) override;

  // Support functions.

  /// Check if we can accept a drop event for @p mimeData.
  /// @param mimeData The data to check.
  /// @return True if we can handle a drop event with this data.
  bool canAcceptDrop(const QMimeData &mimeData) const;

  /// Extract a list of files from a drop event.
  /// @param mimeData Dropped data details.
  /// @return A list of file names (possibly empty).
  QStringList extractDroppedFileList(const QMimeData &mimeData) const;

  Ui::OCurvesUI *_ui;                 ///< Main UI widgets.
  ToolbarWidgets *_toolbarWidgets;    ///< Widgets to add to the main toolbar.
  PlotViewToolbar *_viewToolbar;      ///< Shared view toolbar.
  SplitPlotView *_splitView;          ///< Split data view.
  QwtLegend *_legend;                 ///< Shared legend.
  Curves *_curves;                    ///< Curve data.
  QVector<QRgb> _colours;             ///< Current colour set.
  ExpressionsView *_expressionsView;  ///< Expression editor.
  bool _suppressEvents;               ///< True to ignore signals from certain UI events.
  PostLoaderAction _postLoaderAction; ///< Action to take when @c _loader completes.
  PlotGenerator *_loader;             ///< Current data loader if any.
  QMenu *_sourcesContextMenu;         ///< Context menu for the sources UI list.
  QMenu *_plotsContextMenu;           ///< Context menu for the plots UI list.
  QPoint _lastContextPos;             ///< Stores where the context menu was opened.
  Expressions *_expressions;          ///< Expressions data model.
  QElapsedTimer *_timeSinceLastPlot;  ///< Timer tracking calls to @c replot().
  RealTimePlot *_streams;             ///< Streams loader.
  CurveProperties *_properties;       ///< Properties editor for a curve.

  QString _loadDirectory; ///< The directory open in with the load operation. Stores the last directory used. Serialised to/from settings.
  QString _loadFilter;  ///< Last file filter applied to the load dialog.

  QString _streamDirectory; ///< The directory open in when connecting to a stream.
  QString _streamFilter;  ///< Last file filter applied to the stream dialog.

  QString _originalWindowTitle; ///< Original window title, used to display things like "about".

  int _activeBookmark;  ///< The active bookmark id. Zero for none.
};

#endif // __PLOT_H_
