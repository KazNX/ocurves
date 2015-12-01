//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#include "plotviewtoolbar.h"

#include <QAction>

PlotViewToolbar::PlotViewToolbar(QWidget *parent)
  : QToolBar(parent)
{
  initialiseActions();
}


PlotViewToolbar::~PlotViewToolbar()
{
}


QAction *PlotViewToolbar::action(ActionId actionId) const
{
  for (QAction *action : actions())
  {
    bool ok = false;
    ActionId id = ActionId(action->data().toInt(&ok));
    if (ok && id == actionId)
    {
      return action;
    }
  }

  return nullptr;
}


PlotViewToolbar::ActionId PlotViewToolbar::actionId(const QAction *action) const
{
  ActionId id = ActionInvalid;

  if (action)
  {
    bool ok = false;
    id = ActionId(action->data().toInt(&ok));
    if (!ok)
    {
      id = ActionInvalid;
    }
  }

  return id;
}


bool PlotViewToolbar::setActive(ActionId actionId)
{
  if (QAction *action = this->action(actionId))
  {
    action->setChecked(true);
    return true;
  }
  return false;
}


void PlotViewToolbar::actionChecked(bool checked)
{
  if (QAction *action = qobject_cast<QAction *>(sender()))
  {
    ActionId id = actionId(action);
    CheckGroup *group = checkGroupFor(action);
    if (group)
    {
      if (!checked)
      {
        ensureChecked(*group, id);
      }
      else
      {
        // Uncheck other actions in the group.
        for (unsigned i = 0; i < CheckGroup::Max && group->actions[i]; ++i)
        {
          if (group->actions[i] != action)
          {
            group->actions[i]->setChecked(false);
          }
        }
      }
    }

    if (checked)
    {
      emit active(id);
    }
  }
}


void PlotViewToolbar::initialiseActions()
{
  QAction *action = nullptr;
  action = initialiseAction(addAction(QIcon(":/icons/multi-tool.png"), "&Multi Tool"), ActionToolMulti, GroupTool);
  action->setToolTip(tr("Pan/Zoom multi tool"));
  action = initialiseAction(addAction(QIcon(":/icons/rubber-band.png"), "&Zoom Tool"), ActionToolZoom, GroupTool);
  action->setToolTip(tr("Zoom tool"));
  action = initialiseAction(addAction(QIcon(":/icons/move.png"), "&Pan Tool"), ActionToolPan, GroupTool);
  action->setToolTip(tr("Pan tool"));

  action = initialiseAction(addAction("X/Y"), ActionZoomXY, GroupZoom);
  action->setToolTip(tr("Zoom X and Y axes"));
  action = initialiseAction(addAction("&X"), ActionZoomX, GroupZoom);
  action->setToolTip(tr("Zoom X axis"));
  action = initialiseAction(addAction("&Y"), ActionZoomY, GroupZoom);
  action->setToolTip(tr("Zoom Y axis"));

  ensureChecked(_checkGroups[GroupZoom], ActionZoomXY);
  ensureChecked(_checkGroups[GroupTool], ActionToolMulti);
}


QAction *PlotViewToolbar::initialiseAction(QAction *action, ActionId actionId, GroupId groupId)
{
  action->setData(int(actionId));
  connect(action, &QAction::toggled, this, &PlotViewToolbar::actionChecked);
  if (groupId >= 0)
  {
    action->setCheckable(true);
    if (groupId < GroupCount)
    {
      CheckGroup &group = _checkGroups[groupId];
      for (int i = 0; i < CheckGroup::Max; ++i)
      {
        if (!group.actions[i])
        {
          group.actions[i] = action;
          if (i + 1 < CheckGroup::Max)
          {
            group.actions[i + 1] = nullptr;
          }
          break;
        }
      }
    }
  }

  return action;
}


PlotViewToolbar::CheckGroup *PlotViewToolbar::checkGroupFor(QAction *action)
{
  if (!action)
  {
    return nullptr;
  }

  for (int g = 0; g < GroupCount; ++g)
  {
    CheckGroup &group = _checkGroups[g];
    for (int i = 0; i < CheckGroup::Max && group.actions[i] != nullptr; ++i)
    {
      if (group.actions[i] == action)
      {
        return &group;
      }
    }
  }

  return nullptr;
}


void PlotViewToolbar::ensureChecked(CheckGroup &group, ActionId defaultActionId)
{
  bool checked = false;
  QAction *defaultAction = nullptr;
  for (int i = 0; i < CheckGroup::Max && group.actions[i]; ++i)
  {
    checked = checked || group.actions[i]->isChecked();
    if (actionId(group.actions[i]) == defaultActionId)
    {
      defaultAction = group.actions[i];
    }
  }

  if (!checked && defaultAction)
  {
    defaultAction->setChecked(true);
  }
}
