//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#include "plotsource.h"

#include "plotinstance.h"

#include <QDir>
#include <QFileInfo>
#include <QMutex>
#include <QRegExp>
#include <QStringList>

PlotSource::PlotSource(int sourceType, const QString &fullName, unsigned curveCount)
  : _name(fullName)
  , _fullName(fullName)
  , _type(sourceType)
  , _timeColumn(0)
  , _timeScale(0)
  , _timeBase(0)
  , _curvesMutex(new QMutex)
{
  if (curveCount)
  {
    _curves.reserve(curveCount);
  }
}


PlotSource::~PlotSource()
{
  delete _curvesMutex;
}


void PlotSource::addCurve(PlotInstance *curve)
{
  QMutexLocker lock(_curvesMutex);
  _curves.push_back(curve);
}


double PlotSource::firstTime() const
{
  if (const PlotInstance *curve = timeColumnCurve())
  {
    if (!curve->data().empty())
    {
      return curve->data()[0].y();
    }
  }

  return 0;
}


unsigned PlotSource::curveCount() const
{
  QMutexLocker lock(_curvesMutex);
  return _curves.count();
}


PlotInstance *PlotSource::curve(unsigned index) const
{
  QMutexLocker lock(_curvesMutex);
  if (index < unsigned(_curves.count()))
  {
    return _curves[index];
  }

  return nullptr;
}


PlotInstance *PlotSource::timeColumnCurve() const
{
  QMutexLocker lock(_curvesMutex);
  if (_timeColumn && _timeColumn <= unsigned(_curves.count()))
  {
    return _curves[_timeColumn - 1];
  }
  return nullptr;
}


void PlotSource::deriveName()
{
  QFileInfo fileRef(_fullName);
  fileRef = fileRef.fileName();
  _name = fileRef.baseName();
}


bool PlotSource::lengthenName()
{
  QRegExp delim("[/\\]");
  int targetTokens = _name.split(delim).count() + 1;
  QStringList tokens = QFileInfo(_fullName).baseName().split(delim);

  if (targetTokens > tokens.size())
  {
    targetTokens = tokens.size();
  }

  if (targetTokens == 0)
  {
    return false;
  }

  QString originalName = _name;
  QStringList::iterator iter = tokens.end() - 1;
  _name = *iter;
  for (int i = 0; i < targetTokens; ++i, --iter)
  {
    _name = QString("%1/%2").arg(*iter).arg(_name);
  }

  return originalName.compare(_name) != 0;
}
