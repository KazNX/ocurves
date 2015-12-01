//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#include "plotbindingtracker.h"

void PlotBindingTracker::setMarker(const PlotExpression *expr, unsigned marker)
{
  _markers[expr] = marker;
}


bool PlotBindingTracker::markerFor(const PlotExpression *expr, unsigned &marker) const
{
  auto search = _markers.find(expr);
  if (search != _markers.end())
  {
    marker = *search;
    return true;
  }

  return false;
}


bool PlotBindingTracker::contains(const PlotExpression *expr) const
{
  return _markers.contains(expr);
}


bool PlotBindingTracker::clear(const PlotExpression *expr)
{
  auto search = _markers.find(expr);
  if (search != _markers.end())
  {
    _markers.erase(search);
    return true;
  }

  return false;
}


void PlotBindingTracker::setHold(const PlotExpression *expr, bool hold)
{
  _hold[expr] = hold;
}


bool PlotBindingTracker::isHeld(const PlotExpression *expr) const
{
  return _hold[expr];
}
