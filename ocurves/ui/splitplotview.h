//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef SPLITPLOTVIEW_H_
#define SPLITPLOTVIEW_H_

#include "ocurvesconfig.h"

#include <QList>
#include <QWidget>

class Curves;
class PlotView;
class QSettings;
class QSplitter;

/// @ingroup ui
/// A special plotting view which can contain multiple splits, each with its own @c PlotView.
///
/// The split view always contains at least one @c PlotView. It may also contain a tree of
/// splitters branches with @c PlotView leaf nodes. This structure allows continued splitting
/// and re-splitting of views. On split, the current @c PlotView is duplicated, then the
/// original view and its duplicate are added to a @c QSplitter.
///
/// The @c SplitPlotView also manages event binding to synchronise panning and zooming between
/// the views as required, as well as synchronising the @c PlotView::ToolMode and
/// @c PlotZoomer::ZoomMode between child views.
///
/// This control can essentially be used as in place of a @c PlotView to support view splitting
/// and synchronisation.
class SplitPlotView : public QWidget
{
  Q_OBJECT
public:
  /// Create a split view containing exactly one @c PlotView.
  /// @param curves The curves model to visualise in the child views.
  /// @param parent The parent widgets.
  SplitPlotView(Curves *curves, QWidget *parent = nullptr);

  /// Load settings for the split view, as saved by @c saveSettings().
  /// @param settings The settings object to load from.
  /// @see @c saveSettings()
  void loadSettings(QSettings &settings);

  /// Saves settings for the split view supporting restoring to the same state.
  ///
  /// This creates a settings group labelled "root", then saves the
  /// splitter/view tree within this group. Each branch in the tree
  /// is either a @c QSplitter (branch) or a @c PlotView (leaf).
  ///
  /// A @c QSplitter stores the following:
  ///
  /// Key             | Description
  /// --------------- | ---------------------------------------------------------
  /// type            | "split"
  /// orientation     | Splitter orientation. @c Qt::Orientation as an integer
  /// left            | A new group, recursively storing the first child object.
  /// right           | A new group, recursively storing the second child object.
  /// geometry        | The splitter geometry.
  ///
  /// A @c PlotView stores the following:
  ///
  /// Key             | Description
  /// --------------- | ---------------------------------------------------------
  /// type            | "view"
  /// [View settings] | Calls @c PlotView::saveSettings() to save view settings.
  /// active          | "true" for the @c activeView()
  ///
  /// @param settings The settings object to save to.
  void saveSettings(QSettings &settings);

  /// Access the active view. This is the (last) focused view.
  ///
  /// There is always an active view.
  /// @return The currently active view.
  inline PlotView *activeView() const { return _activeView; }

  /// Does the view have any splits?
  /// @return True if the split view has any splits (multiple views).
  bool hasSplits() const;

  /// Re-plot all views within the splitter tree.
  void replot();

  /// Collate all contained views.
  /// @param[in,out] views Collated into here.
  void collate(QVector<PlotView *> &views);

  /// Collates the list of source and curve names being viewed by any active view.
  /// @param[in,out] sourceNames Sources are collated in this list. Does not get cleared first.
  /// @param[in,out] curveNames Curves are collated in this list. Does not get cleared first.
  /// @param exclude Optionally exclude this view from the collation process.
  void collateActive(QStringList &sourceNames, QStringList &curveNames, const PlotView *exclude = nullptr);

signals:

  /// Raised when a new view is created due to a new split.
  /// @param view The new view.
  void viewAdded(PlotView *view);
  //void viewRemoved(PlotView *view);

  /// Raised when the @c activeView() changes.
  /// @param newView The newly active view.
  /// @param oldView The previously active view.
  void activeViewChanged(PlotView *newView, PlotView *oldView);

  /// Signals panning from a child @c PlotView where @c PlotView::synchronised() is set.
  ///
  /// This is used to synchronise panning @c PlotView children. It is bound to
  /// the @c PlotPanner::syncPan() signal is bound to propagate to the
  /// @c PlotPanner::moveCanvas() slot.
  ///
  /// @param x The pan change in X.
  /// @param y The pan change in Y.
  void syncPan(int x, int y);

  /// Raised when the @c PlotView::ToolMode is changed to @c PlotView::MultiTool.
  void multiToolModeSet();

  /// Raised when the @c PlotView::ToolMode is changed to @c PlotView::PanTool.
  void panToolModeSet();

  /// Raised when the @c PlotView::ToolMode is changed to @c PlotView::ZoomTool.
  void zoomToolModeSet();

  /// Raised when the @c PlotZoomer::ZoomMode is changed to @c PlotZoomer::ZoomX.
  void zoomXSet();

  /// Raised when the @c PlotZoomer::ZoomMode is changed to @c PlotZoomer::ZoomY.
  void zoomYSet();

  /// Raised when the @c PlotZoomer::ZoomMode is changed to @c PlotZoomer::ZoomBoth.
  void zoomXYSet();

public slots:
  /// Split the active plot view vertically.
  void splitVertical();

  /// Split the active plot view horizontally.
  void splitHorizontal();

  /// Remove the current split.
  void splitRemove();

  /// Remove all splits, keeping only the current plot view.
  void splitRemoveAll();

  /// Set the current tool to @c PlotView::MultiTool. Affects all child views.
  void setMultiTool();

  /// Set the current tool to @c PlotView::PanTool. Affects all child views.
  void setPanTool();

  /// Set the current tool to @c PlotView::ZoomTool. Affects all child views.
  void setZoomTool();

  /// Set the current zoom mode to @c PlotZoomer::ZoomX. Affects all child views.
  void setZoomX();

  /// Set the current zoom mode to @c PlotZoomer::ZoomY. Affects all child views.
  void setZoomY();

  /// Set the current zoom mode to @c PlotZoomer::ZoomBoth. Affects all child views.
  void setZoomXY();

private slots:
  /// Handler for @c PlotView::focusGained(): changes the @c activeView(), signalling @c activeViewChanged().
  void viewFocusGained();

  /// Handles @c PlotView::synchronisedChanged() changes.
  ///
  /// This either binds or unbinds various pan and zoom signals and slots to ensure
  /// changing one synchronised view affects all synchronised views.
  ///
  /// Only works if the sender is a @c PlotView.
  ///
  /// @param sync True if synchronisation is being activated, thereby binding events, or
  ///   false to unbind events.
  void viewSyncChanged(bool sync);

  // Events only for internal propagation to child views.

  /// Handles tool mode changes from child views, keeping all views to the same mode.
  /// @param mode The new @c PlotView::ToolMode.
  void viewToolModeChanged(int mode);

  /// Handles zoom mode changes from child views, keeping all views to the same mode.
  /// @param mode The new @c PlotZoomer::ZoomMode.
  void viewZoomModeChanged(int mode);

private:
  /// Sets the active view, adjusting the highlights and raising @c activeViewChanged() as
  /// required.
  /// @param view The new view. Ignored if The same as @c activeView().
  void setActiveView(PlotView *view);

  /// Utility method recursively searching children of @p widget for the first @c PlotView.
  ///
  /// Uses a breadth first traversal.
  ///
  /// @param widget The widget to search. Generally a @c QSplitter.
  /// @return The first @c PlotView descendent of @p widget, or null if no view is found.
  static PlotView *findFirstView(QWidget *widget);

  /// Get the first child of @p split relevant to the view tree structure.
  ///
  /// This looks for other @c QSplitter objects or @c PlotView objects.
  /// Only searches immediately children of @p split.
  ///
  /// @param split The object to search the children of.
  /// @param index The index of the child of interest. Generally 0 or 1 (left or right).
  /// @return The @c QSplitter or @c PlotView child of @p split at the given @p index.
  ///   May be null if @p index is out of range or no such children can be found.
  static QWidget *relevantChild(const QSplitter *split, int index);

  /// Get the first or 'left' tree branch of @c split.
  /// @return The first @c QSplitter or @c PlotView child of @p split.
  static inline QWidget *left(const QSplitter *split) { return relevantChild(split, 0); }

  /// Get the second or 'right' tree branch of @c split.
  /// @return The second @c QSplitter or @c PlotView child of @p split.
  static inline QWidget *right(const QSplitter *split) { return relevantChild(split, 1); }

  /// Initialise @p view, binding appropriate events and setting the initial state.
  ///
  /// This duplicates the state of @p referenceView if provided.
  /// @param view The view to initialise.
  /// @param referenceView Optional reference view to duplicate the view settings of.
  void initialiseView(PlotView *view, const PlotView *referenceView = nullptr);

  /// Initialise @p split to internal settings. No views are added.
  ///
  /// For use when the child branches or leaves are yet to be determined.
  ///
  /// @param split The split to initialise.
  void initialiseSplit(QSplitter *split);

  /// Initialise @p split such that it contains two views based on @p view;
  /// the @p view object itself and a clone of @p view.
  ///
  /// For use in view splitting.
  ///
  /// @param split The splitter to initialise.
  /// @param view The view to wrap and duplicate in @p split.
  void initialiseSplit(QSplitter *split, PlotView *view);

  /// Save settings for @p branch. Handles @c QSplitter and @c PlotView objects.
  /// @param settings The settings to store values in.
  /// @param branch The view branch to store settings for.
  /// @see @c saveSettings()
  void saveBranch(QSettings &settings, QWidget *branch);

  /// Rebuilds the view tree based on @p settings.
  ///
  /// This restores the view tree by reading settings saved by @c saveBranch().
  /// The method recursively loads settings, creating @c QSplitter and @c PlotView
  /// objects as required.
  ///
  /// The first view marked as "active" is returned in @p activeView.
  ///
  /// @param settings The settings object to load from.
  /// @param parent The parent branch of the sub-tree. Always this for the first call.
  /// @param[out] activeView Set to the first @c PlotView marked as "active".
  ///   Correct de-serialsiation should always result in one view being active
  ///   in the view tree.
  /// @return The loaded branch.
  QWidget *loadTree(QSettings &settings, QWidget *parent, PlotView *&activeView);

  /// The root of the split tree. Each note is either a @c QSplitter, representing
  /// a branch, or a @c PlotView, representing a leaf. Each split will only have
  /// up to two children.
  QWidget *_root;
  PlotView *_activeView;  ///< The active @c PlotView.
  Curves *_curves;        ///< Curves data model.
  QList<PlotView *> _synchedViews;  ///< List of views marked as @c PlotView::synchronised()
  bool _suppressEvents;   ///< True to temporariliy ignore handling of various events.
};

#endif // SPLITPLOTVIEW_H_
