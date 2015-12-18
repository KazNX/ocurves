//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2013
//
#include "plotsample.h"

#include "plotbindinfo.h"
#include "plotbindingtracker.h"
#include "plotinstance.h"
#include "plotinstancesampler.h"

#include <QTextStream>
#include <QRegExp>

#include <qwt_series_data.h>

namespace
{
  double sample(double sampleTime, const QPointF &from, const QPointF &to)
  {
    double normalTime = (sampleTime - from.x()) / (to.x() - from.x());
    return from.y() + normalTime * (to.y() - from.y());
  }
}


QTextStream &operator << (QTextStream &stream, const PlotSampleId &sid)
{
  if (!sid.name.isEmpty())
  {
    if (sid.regex)
    {
      stream << 'r';
    }

    if (sid.quote)
    {
      stream << sid.quote;
    }

    stream << sid.name;

    if (sid.quote)
    {
      stream << sid.quote;
    }
  }
  return stream;
}


PlotSample::PlotSample(const QString &curveName, bool curveRegularExpression)
  : _curveId(curveName, curveRegularExpression)
  , _sampler(new PlotInstanceSampler(nullptr))
  , _previousSample(0)
{
}


PlotSample::PlotSample(const QString &curveName, const QString &fileName, bool curveRegularExpression, bool fileRegularExpression)
  : _curveId(curveName, curveRegularExpression)
  , _fileId(fileName, fileRegularExpression)
  , _sampler(new PlotInstanceSampler(nullptr))
  , _previousSample(0)
{
}


PlotSample::PlotSample(const PlotSampleId &curveId, const PlotSampleId &fileId)
  : _curveId(curveId)
  , _fileId(fileId)
  , _sampler(new PlotInstanceSampler(nullptr))
  , _previousSample(0)
{
}


PlotSample::PlotSample(const PlotSample &other)
  : _curveId(other._curveId)
  , _fileId(other._fileId)
  , _sampler(new PlotInstanceSampler(other._sampler->curve()))
  , _previousSample(0)
{
}


PlotSample::~PlotSample()
{
  delete _sampler;
}


double PlotSample::sample(double sampleTime) const
{
  QPointF from;
  QPointF to;

  if (!_sampler->curve())
  {
    return 0;
  }

  if (_previousSample + 1 < _sampler->size())
  {
    from = _sampler->sample(_previousSample);
    if (from.x() > sampleTime)
    {
      _previousSample = 0u;
    }
  }
  else
  {
    _previousSample = 0u;
  }

  // FIXME: change to a binary search. For, put in a range check to ensure we aren't searching
  // exhaustively for naught.
  if (_sampler->size())
  {
    // Range check.
    if (_sampler->sample(0).x() <= sampleTime && sampleTime <= _sampler->sample(_sampler->size() - 1).x())
    {
      // Try sampling from the current position.
      while (_previousSample + 1 < _sampler->size())
      {
        from = _sampler->sample(_previousSample);
        to = _sampler->sample(_previousSample + 1);

        if (from.x() <= sampleTime && sampleTime <= to.x())
        {
          return ::sample(sampleTime, from, to);
        }

        ++_previousSample;
      }
    }
  }

  return 0.0;
}


BindResult PlotSample::bind(const QList<PlotInstance *> &curves, PlotBindingTracker &bindTracker, PlotExpressionBindDomain &info, bool repeatLastBinding)
{
  _previousSample = 0u;
  QRegExp fileRegEx(_fileId.name);
  QRegExp *fileREP = _fileId.regex ? &fileRegEx : nullptr;
  QRegExp nameRegEx(_curveId.name);
  QRegExp *nameREP = _curveId.regex ? &nameRegEx : nullptr;

  // Support multiple bindings.
  unsigned skipTo = 0;
  auto curveIter = curves.begin();
  if (bindTracker.markerFor(this, skipTo))
  {
    if (!repeatLastBinding && !bindTracker.isHeld(this))
    {
      ++skipTo;
    }
    curveIter += skipTo;
    bindTracker.clear(this);
  }

  unsigned index = skipTo;
  for (; curveIter != curves.end(); ++curveIter, ++index)
  {
    PlotInstance *curve = *curveIter;
    if (nameMatch(curve->source().name(), _fileId.name, fileREP))
    {
      if (nameMatch(curve->name(), _curveId.name, nameREP))
      {
        _boundName = makeBoundName(*curve);

        _sampler->setCurve(curve);
        if (!curve->data().empty())
        {
          info.domainMin = _sampler->sample(0).x();
          info.domainMax = _sampler->sample(_sampler->size() - 1).x();
          info.minSet = info.maxSet = true;
          double step = (info.domainMax - info.domainMin) / ((_sampler->size() > 1) ? _sampler->size() - 1 : 1);
          info.sampleDelta = (info.sampleDelta == 0.0 || info.sampleDelta > step) ? step : info.sampleDelta;
          info.sampleCount = (info.sampleCount >= _sampler->size()) ? info.sampleCount : _sampler->size();
        }

        // Record as first binding source if required.
        bindTracker.setFirstPlotIf(curve);

        bindTracker.setMarker(this, index);
        if (index + 1 < unsigned(curves.count()))
        {
          return BoundMaybeMore;
        }

        // No more bindings available.
        return Bound;
      }
    }
  }

  return BindFailure;
}


void PlotSample::unbind()
{
  _boundName = QString();
  _sampler->setCurve(nullptr);
}


PlotExpression *PlotSample::clone() const
{
  return new PlotSample(*this);
}


QString PlotSample::stringExpression() const
{
  QString str;
  QTextStream stream(&str);

  if (!_boundName.isEmpty())
  {
    return _boundName;
  }

  if (_fileId)
  {
    stream << _fileId << '|';
  }
  stream << _curveId;

  return str;
}


QString PlotSample::makeBoundName(const PlotInstance &plot) const
{
  QString str;
  QTextStream stream(&str);

  PlotSampleId curveId(_curveId);

  if (_fileId)
  {
    PlotSampleId fileId(_fileId);
    fileId.name = plot.source().name();
    stream << fileId;
    stream << '|';
  }

  curveId.name = plot.name();
  stream << curveId;
  return str;
}


bool PlotSample::nameMatch(const QString &name, const QString &searchName, const QRegExp *re)
{
  if (re)
  {
    return re->exactMatch(name);
  }

  if (!searchName.isEmpty())
  {
    return name.compare(searchName) == 0;
  }

  return true;
}

