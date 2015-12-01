//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#include "plotgenerator.h"

#include "qwt_plot_curve.h"
#include "qwt_series_data.h"

#include <QColor>
#include <QFile>
#include <QMutex>
#include <QRegExp>
#include <QVector>

#include "plotfile.h"
#include "plotinstance.h"

#include "model/curves.h"

#include <cassert>
#include <cstdint>
#include <limits>
#include <memory>

PlotGenerator::PlotGenerator(Curves *curves)
  : _curves(curves)
  , _dataMutex(new QMutex)
  , _controlFlags(0)
  , _timeColumn(1)
  , _timeScale(1.0)
  , _abortFlag(false)
{
}


PlotGenerator::~PlotGenerator()
{
  delete _dataMutex;
}


void PlotGenerator::setTimeColumn(uint number)
{
  QMutexLocker lock(_dataMutex);
  _timeColumn = number;
}


uint PlotGenerator::timeColumn() const
{
  QMutexLocker lock(_dataMutex);
  return _timeColumn;
}

void PlotGenerator::setTimeScale(double scale)
{
  QMutexLocker lock(_dataMutex);
  _timeScale = scale;
}


double PlotGenerator::timeScale() const
{
  QMutexLocker lock(_dataMutex);
  return _timeScale;
}


void PlotGenerator::setControlFlags(uint flags)
{
  QMutexLocker lock(_dataMutex);
  _controlFlags = flags;
}


void PlotGenerator::setControlFlags(uint flags, bool on)
{
  QMutexLocker lock(_dataMutex);
  _controlFlags = (_controlFlags & ~flags) | (!!on * flags);
}


uint PlotGenerator::controlFlags() const
{
  QMutexLocker lock(_dataMutex);
  return _controlFlags;
}


void PlotGenerator::abortLoad()
{
  _abortFlag = true;
}
