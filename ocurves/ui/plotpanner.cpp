//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#include "plotpanner.h"

PlotPanner::PlotPanner(QWidget *parent)
  : QwtPlotPanner(parent)
  , _ignoreEvents(false)
{
}


void PlotPanner::syncPan(int dx, int dy)
{
  if (!_ignoreEvents)
  {
    moveCanvas(dx, dy);
  }
}


void PlotPanner::moveCanvas(int dx, int dy)
{
  //QObject *send = sender();
  _ignoreEvents = true;
  Super::moveCanvas(dx, dy);
  _ignoreEvents = false;
}
