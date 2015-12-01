//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#include "curves.h"

#include "plotinstance.h"

#include <QMutex>
#include <QThread>
#include <QTimer>

Curves::CurveList::CurveList(QMutex &mutex, const QList<PlotInstance *> &curves)
  : _mutex(&mutex)
  , _curves(&curves)
{
  _mutex->lock();
}


Curves::CurveList::~CurveList()
{
  release();
}


void Curves::CurveList::release()
{
  if (_mutex)
  {
    _mutex->unlock();
    _mutex = nullptr;
    _curves = nullptr;
  }
}


Curves::Curves(QObject *parent)
  : QObject(parent)
  , _curvesMutex(new QMutex)
  , _loadingMutex(new QMutex)
  , _realTimeMutex(new QMutex)
  , _deathRowMutex(new QMutex)
{
}


Curves::~Curves()
{
  for (PlotInstance *curve : _curves)
  {
    delete curve;
  }

  for (const PlotInstance *curve : _deathRow)
  {
    delete curve;
  }
  delete _deathRowMutex;
  delete _realTimeMutex;
  delete _loadingMutex;
  delete _curvesMutex;
}


Curves::CurveList Curves::curves() const
{
  return CurveList(*_curvesMutex, _curves);
}


Curves::CurveList Curves::loadingCurves() const
{
  return CurveList(*_loadingMutex, _loadingCurves);
}


Curves::CurveList Curves::realTimeCurves() const
{
  return CurveList(*_realTimeMutex, _realTimeCurves);
}


unsigned Curves::removeUsingExpression(const PlotExpression *expression)
{
  if (!expression)
  {
    return 0;
  }

  QMutexLocker lock(_curvesMutex);
  unsigned removed = 0;
  for (auto iter = _curves.begin(); iter != _curves.end();)
  {
    PlotInstance *curve = *iter;
    if (curve->expression() == expression)
    {
      // OCurvesUI uses the expression we are removing. Remove the plot.
      iter = _curves.erase(iter);
      {
        QMutexLocker llock(_loadingMutex);
        _loadingCurves.removeOne(curve);
      }
      // Don't check real-time curves. They don't support expressions.
      emit curveRemoved(curve);

      delete curve;
      curve = nullptr;  // Clear dead pointer.
      ++removed;
    }
    else
    {
      ++iter;
    }
  }

  return removed;
}


void Curves::enumerateFileSources(QStringList &filePaths) const
{
  CurveList curvesList = curves();
  for (const PlotInstance *curve : curvesList)
  {
    if (!filePaths.contains(curve->source().fullName()))
    {
      filePaths.append(curve->source().fullName());
    }
  }
}


void Curves::enumerateSources(QList<PlotSource *> &sources, unsigned type) const
{
  CurveList curvesList = curves();
  for (PlotInstance *curve : curvesList)
  {
    PlotSource *source = &curve->source();
    if (source->type() == type && !sources.contains(source))
    {
      sources.append(source);
    }
  }
}


void Curves::setCurvePropertiesMap(VariantMap &map)
{
  // Lock loading (first) and current lists to ensure we don't miss anything.
  QMutexLocker lock1(_loadingMutex);
  QMutexLocker lock2(_curvesMutex);

  QList<PlotInstance *> modifiedList;
  _curvePropertiesMap = map;
  // Iterate existing curves and modify if present.
  for (PlotInstance *curve : _curves)
  {
    if (restoreProperties(*curve))
    {
      modifiedList.append(curve);
    }
  }

  // Unlock.
  lock2.unlock();
  lock1.unlock();

  // Notify changes.
  for (PlotInstance *curve : modifiedList)
  {
    emit curveDataChanged(curve);
  }
}


void Curves::newCurve(PlotInstance *curve)
{
  QMutexLocker lock(_curvesMutex);
  if (!_curves.contains(curve))
  {
    _curves.append(curve);
    if (curve->source().type() != PlotSource::RealTime)
    {
      QMutexLocker llock(_loadingMutex);
      _loadingCurves.append(curve);
    }
    else
    {
      QMutexLocker rtlock(_realTimeMutex);
      _realTimeCurves.append(curve);
    }
    // Restore properties
    restoreProperties(*curve);
    lock.unlock();
    emit curveAdded(curve);
  }
}


void Curves::completeLoading(PlotInstance *curve)
{
  QMutexLocker llock(_loadingMutex);
  // Note: curve could have already been deleted as cross thread messages catch up.
  // If deleted, it should no longer be present in loadingCurves.
  if (_loadingCurves.removeOne(curve))
  {
    llock.unlock();

    if (QThread::currentThread() != this->thread())
    {
      // Can't migrate from here as this may be a background thread.
      // We have to just emit the signal.
      _completedCurves.append(curve);
    }
    else
    {
      // On the main thread. migrate whatever data are left to display most current curve.
      if (curve->migrateBuffer())
      {
        emit curveDataChanged(curve);
      }
      curve->setComplete();
      emit curveComplete(curve);
    }

    llock.relock();
    if (_loadingCurves.empty())
    {
      emit loadingComplete();
    }
  }
  else
  {
    llock.unlock();
    QMutexLocker rtlock(_realTimeMutex);
    if (_realTimeCurves.removeOne(curve))
    {
      rtlock.unlock();
      if (curve->migrateBuffer())
      {
        emit curveDataChanged(curve);
      }
      curve->setComplete();
      emit curveComplete(curve);

      rtlock.relock();
      if (_realTimeCurves.empty() && _loadingCurves.empty())
      {
        emit loadingComplete();
      }
    }
  }
}


bool Curves::removeCurve(const PlotInstance *curve)
{
  QMutexLocker lock(_curvesMutex);
  QMutexLocker llock(_loadingMutex);
  QMutexLocker rtlock(_realTimeMutex);

  // Remove from loading list.
  // Const cast so we can use removeOne().
  PlotInstance *c = const_cast<PlotInstance *>(curve);
  _loadingCurves.removeOne(c);
  _realTimeCurves.removeOne(c);
  if (_curves.removeOne(c))
  {
    rtlock.unlock();
    llock.unlock();
    lock.unlock();
    emit curveRemoved(curve);
    addToDeathRow(curve);
    return true;
  }

  return false;
}


void Curves::invalidate(const PlotInstance *curve)
{
  if (curve)
  {
    emit curveDataChanged(curve);
  }
}


void Curves::invalidate(const PlotSource *source, bool invalidateCurves)
{
  if (!source)
  {
    return;
  }

  emit sourceDataChanged(source);

  if (invalidateCurves)
  {
    for (unsigned i = 0; i < source->curveCount(); ++i)
    {
      if (const PlotInstance *curve = source->curve(i))
      {
        invalidate(curve);
      }
    }
  }
}

void Curves::clearCurves()
{
  QMutexLocker lock(_curvesMutex);
  QMutexLocker llock(_loadingMutex);
  QMutexLocker rtlock(_realTimeMutex);

  llock.unlock();
  rtlock.unlock();
  while (!_curves.empty())
  {
    PlotInstance *curve = _curves.back();
    _curves.pop_back();
    llock.relock();
    rtlock.relock();
    _loadingCurves.removeOne(curve);
    _realTimeCurves.removeOne(curve);
    llock.unlock();
    rtlock.unlock();
    lock.unlock();
    emit curveRemoved(curve);
    addToDeathRow(curve);
    lock.relock();
  }

  lock.unlock();

  emit curvesCleared();
}


bool Curves::migrateLoadingData()
{
  QMutexLocker llock(_loadingMutex);
  bool migrated = false;
  for (PlotInstance *curve : _loadingCurves)
  {
    if (curve->migrateBuffer())
    {
      emit curveDataChanged(curve);
      migrated = true;
    }
  }

  for (PlotInstance *curve : _completedCurves)
  {
    curve->setComplete();
    if (curve->migrateBuffer())
    {
      emit curveDataChanged(curve);
      migrated = true;
    }
    emit curveComplete(curve);
  }
  _completedCurves.clear();
  llock.unlock();

  QMutexLocker rtlock(_realTimeMutex);
  for (PlotInstance *curve : _realTimeCurves)
  {
    if (curve->migrateBuffer())
    {
      emit curveDataChanged(curve);
      migrated = true;
    }
  }

  return migrated;
}


bool Curves::isLoading(const PlotInstance *curve) const
{
  QMutexLocker llock(_loadingMutex);
  for (const PlotInstance *c : _loadingCurves)
  {
    if (c == curve)
    {
      return true;
    }
  }
  return false;
}


namespace
{
  template <typename T>
  T lookup(const VariantMap &map, const QString &key, const T &currentValue, bool &changed)
  {
    QVariant var = map[key];
    if (var.isNull())
    {
      return currentValue;
    }

    T val = var.value<T>();
    if (val != currentValue)
    {
      changed = true;
    }

    return val;
  }
}

bool Curves::restoreProperties(PlotInstance &curve) const
{
  // Generate the key prefix.
  QString key = QString("%1|%2|%3").arg(curve.source().name(), curve.name());

  bool changed = false;
  // Look for values.
  QVariant var;
  var = _curvePropertiesMap[key.arg("colour")];
  if (!var.isNull())
  {
    curve.setColour(var.toUInt());
    changed = true;
  }

  curve.setStyle(lookup<int>(_curvePropertiesMap, key.arg("style"), curve.style(), changed));
  curve.setWidth(lookup<unsigned>(_curvePropertiesMap, key.arg("width"), curve.width(), changed));
  curve.setSymbol(lookup<int>(_curvePropertiesMap, key.arg("symbol"), curve.symbol(), changed));
  curve.setSymbolSize(lookup<unsigned>(_curvePropertiesMap, key.arg("symbol-size"), curve.symbolSize(), changed));

  curve.setFilterInf(lookup<bool>(_curvePropertiesMap, key.arg("filter-inf"), curve.filterInf(), changed));
  curve.setFilterNaN(lookup<bool>(_curvePropertiesMap, key.arg("filter-nan"), curve.filterNaN(), changed));

  return changed;
}


void Curves::addToDeathRow(const PlotInstance *curve)
{
  QMutexLocker lock(_deathRowMutex);
  _deathRow.append(curve);
  lock.unlock();
  // Setup the event to clear death row.
  //QTimer::singleShot(0, this, &Curves::clearDeathRow);
  QTimer::singleShot(0, this, SLOT(clearDeathRow()));
}


void Curves::clearDeathRow()
{
  QMutexLocker lock(_deathRowMutex);
  for (const PlotInstance *curve : _deathRow)
  {
    delete curve;
  }
  _deathRow.clear();
}
