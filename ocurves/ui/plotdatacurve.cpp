//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#include "plotdatacurve.h"

#include "plotinstance.h"

PlotDataCurve::PlotDataCurve(PlotInstance &curve)
  : QwtPlotCurve(curve.name() + "|" + curve.source().name())
  , _curve(&curve)
{
}


int PlotDataCurve::rtti() const
{
  return Rtti;
}
