//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef PLOTVIEWTOOLBAR_H_
#define PLOTVIEWTOOLBAR_H_

#include "ocurvesui.h"

#include <QToolBar>

/// @ingroup ui
/// A toolbar for use with @c PlotView objects.
///
/// The toolbar supports operations which directly affect a @c PlotView. The toolbar may be added to
/// a main view and associated with a @p SplitView or directly associated with a @c PlotView. In
/// either case, it is the user's responsibility to bind the toolbar actions appropriately.
///
/// The toolbar automatically ensures that one and only one action in each @p GroupId is checked.
class PlotViewToolbar : public QToolBar
{
  Q_OBJECT
public:
  /// Various tool groupings for actions on the toolbar.
  enum GroupId
  {
    GroupNone = -1,
    GroupZoom = 0,  ///< Various zoom settings.
    GroupTool,      ///< Exclusive interaction tools.
    GroupCount
  };

  /// IDs for the various actions on the toolbar.
  enum ActionId
  {
    ActionInvalid = -1,
    ActionZoomXY, ///< Set the zoom mode to X and Y.
    ActionZoomX,  ///< Set zoom to X axis only.
    ActionZoomY,  ///< Set zoom to Y axis only.

    ActionToolMulti,  ///< Select zoom and pan multi-tool.
    ActionToolZoom,   ///< Select zoom only tool.
    ActionToolPan,    ///< Select pan only tool.

    ActionCount
  };

  /// Constructor.
  /// @param parent The owner of the toolbar.
  explicit PlotViewToolbar(QWidget *parent = 0);
  /// Destructor.
  ~PlotViewToolbar();

  /// Access an action on the toolbar by @c ActionId.
  /// @param actionId The ID of the requested action.
  /// @return The requested action or null @p actionId is invalid.
  QAction *action(ActionId actionId) const;

  /// Get the @c ActionId of the given @p action.
  /// @param action The action of interest.
  /// @return The @c ActionId associated with @p action or @c ActionInvalid if
  ///   @c action is null or invalid.
  ActionId actionId(const QAction *action) const;

public slots:
  /// Set the action associated with @p actionId to be active. Other actions in the
  /// same group are made inactive.
  /// @param actionId The ID of the action of interest.
  /// @return True if @p actionId was successfully resolve to an action.
  bool setActive(ActionId actionId);

signals:
  /// Signal emitted when an action becomes active.
  /// @param actionId The ID of the activated action.
  void active(ActionId actionId);

private slots:
  /// Handle signalling @c active() and ensure at least one item in the check group stays checked.
  /// @param checked True if the action is checked.
  void actionChecked(bool checked);

private:
  /// Group definition structure.
  struct CheckGroup
  {
    enum { Max = 4 };
    QAction *actions[Max];  ///< Actions in the group.

    /// Constructor, marking the first item as invalid.
    inline CheckGroup()
    {
      actions[0] = nullptr;
    }
  };

  /// Creates an initialises the actions in @c ActionId, adding them to the toolbar.
  /// Also resolves action groupings.
  void initialiseActions();

  /// Create an initialise a single action.
  ///
  /// This resolves the @c ActionId and @p GroupId,.
  /// @param action The action to initialise. Not null.
  /// @param actionId The ID for the action.
  /// @param groupId Optional group ID for the action. The action is only added to the
  ///   group if the group is not already full.
  /// @return A pointer to @p action.
  QAction *initialiseAction(QAction *action, ActionId actionId, GroupId groupId = GroupNone);

  /// Resolves the @c CheckGroup for @p action.
  /// @param action The action of interest (null handled).
  /// @return A pointer to the group to which @p action belongs, or null when @p action
  ///   is not associated with a group.
  CheckGroup *checkGroupFor(QAction *action);

  /// Ensures that the given @p group has at least one action checked.
  ///
  /// The function has no effect if @p defaultActionId is not in @p group.
  /// @param group The group to operate on.
  /// @param defaultActionId ID of the action to check if no action is currently
  /// checked.
  void ensureChecked(CheckGroup &group, ActionId defaultActionId);

  CheckGroup _checkGroups[GroupCount];  ///< Group definitions.
};

#endif // PLOTVIEWTOOLBAR_H_
