//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#include "plotbindinfo.h"

#include "plotbindingtracker.h"
#include "plotexpression.h"

BindResult binding::bindMultiple(const QList<PlotInstance *> &curves, PlotBindingTracker &bindTracker, BindInfo &head, BindInfo *tail, unsigned tailLength, bool repeatLast)
{
  if (tailLength == 0)
  {
    return head.expression->bind(curves, bindTracker, head.domain, repeatLast || bindTracker.isHeld(head.expression));
  }

  BindResult bindHead = head.expression->bind(curves, bindTracker, head.domain, repeatLast || bindTracker.isHeld(head.expression));

  if (bindHead <= 0)
  {
    // Failed to bind the head.
    bindTracker.clear(head.expression);
    bindTracker.setHold(head.expression, false);
    return bindHead;
  }

  // Failed to bind the head. No need to continue.
  if (bindHead <= 0)
  {
    return bindHead;
  }

  // We have a head binding. Recurse on the tail.
  BindResult bindTail = bindMultiple(curves, bindTracker, tail[0], tail + 1, tailLength - 1, repeatLast);

  switch (bindHead)
  {
  // -----------------------------
  case Bound:
    switch (bindTail)
    {
    case Bound:
      // Both bound with no repeat bindings available.
      return Bound;

    case BoundMaybeMore:
      // Left bound, tail may have more. Hold head and report possible repeats.
      bindTracker.setHold(head.expression, true);
      return BoundMaybeMore;

    default:
      // Left bound, right failed. Overall failure.
      head.expression->unbind();
      return bindTail;
    }
    break;

  // -----------------------------
  case BoundMaybeMore:
    switch (bindTail)
    {
    case Bound:
      // Both bound, head may repeat, tail won't.
      return BoundMaybeMore;

    case BoundMaybeMore:
      // Head and tail bound and both may repeat. Hold tail and report possible repeats.
      bindTracker.setHold(head.expression, true);
      return BoundMaybeMore;

    default:
      // Head bound with potentially more bindings, tail failed.
      // Release head hold and try again.
      head.expression->unbind();
      if (bindTracker.isHeld(head.expression))
      {
        bindTracker.setHold(head.expression, false);
        return bindMultiple(curves, bindTracker, head, tail, tailLength, repeatLast);
      }
      return bindTail;
    }
    break;

  // -----------------------------
  default:
    // Head failed to bind. Stop. This is actually handled above, but it doesn't hurt to
    // have it here against future changed.
    bindTracker.clear(head.expression);
    bindTracker.setHold(head.expression, false);
    if (bindTail > 0)
    {
      for (unsigned i = 0; i < tailLength; ++i)
      {
        tail[i].expression->unbind();
        bindTracker.clear(tail[i].expression);
        bindTracker.setHold(tail[i].expression, false);
      }
    }

    return bindHead;
  }

  // Shouldn't get here, but I'm paranoid.
  if (bindHead > 0)
  {
    bindTracker.clear(head.expression);
    bindTracker.setHold(head.expression, false);
    head.expression->unbind();
  }

  if (bindTail > 0)
  {
    for (unsigned i = 0; i < tailLength; ++i)
    {
      tail[i].expression->unbind();
      bindTracker.clear(tail[i].expression);
      bindTracker.setHold(tail[i].expression, false);
    }
  }

  return BindError;
}
