//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#include "plotslice.h"

#include "plotbindinfo.h"
#include "plotbindingtracker.h"

#include <QTextStream>

#include <limits>

PlotSlice::PlotSlice(PlotExpression *indexee, PlotExpression *indexerStart, PlotExpression *indexerEnd)
  : _indexee(indexee)
  , _indexerStart(indexerStart)
  , _indexerEnd(indexerEnd)
{
}


PlotSlice::~PlotSlice()
{
  delete _indexee;
  delete _indexerStart;
  delete _indexerEnd;
}


double PlotSlice::sample(double sampleTime) const
{
  // Index start/end members only define the range and don't affect sampling within that range.
  return _indexee->sample(sampleTime);
}


BindResult PlotSlice::bind(const QList<PlotInstance *> &curves, PlotBindingTracker &bindTracker, PlotExpressionBindDomain &info, bool repeatLastBinding)
{
//  PlotExpressionBindDomain startInfo, endInfo;
  if (!_indexee)
  {
    return BindError;
  }

  unsigned bindCount = 0;
  BindInfo bindings[3];
  bindings[bindCount++].expression = _indexee;
  if (_indexerStart)
  {
    bindings[bindCount++].expression = _indexerStart;
  }
  if (_indexerEnd)
  {
    bindings[bindCount++].expression = _indexerEnd;
  }

  repeatLastBinding = repeatLastBinding || bindTracker.isHeld(this);
  BindResult bindResult = binding::bindMultiple(curves, bindTracker, bindings[0], &bindings[1], bindCount - 1, repeatLastBinding);

  if (bindResult <= 0)
  {
    return bindResult;
  }

  info = bindings[0].domain;
  // Binding ok. Resolve slice start and end limits.
  if (_indexerStart)
  {
    // Sample at the domain start time.
    const PlotExpressionBindDomain &startDomain = bindings[1].domain;
    double startTime = _indexerStart->sample(startDomain.domainMin);
    if (startTime > info.domainMin)
    {
      info.domainMin = std::min(startTime, info.domainMax);
      info.minSet = true;
    }
  }

  if (_indexerEnd)
  {
    // Sample at the domain end time.
    const PlotExpressionBindDomain &endDomain = bindings[bindCount - 1].domain;
    double endTime = _indexerEnd->sample(endDomain.domainMax);
    if (endTime < info.domainMax)
    {
      info.domainMax = std::max(endTime, info.domainMin);
      info.maxSet = true;
    }
  }

  if (info.domainMin > info.domainMax || !info.minSet || !info.maxSet)
  {
    _indexee->unbind();
    if (_indexerStart) { _indexerStart->unbind(); }
    if (_indexerEnd) { _indexerEnd->unbind(); }
    return BindError;
  }

  info.sampleCount = 1u + std::max<size_t>(size_t((info.domainMax - info.domainMin) / info.sampleDelta), 1u);
  info.sampleDelta = (info.domainMax - info.domainMin) / (info.sampleCount - 1u);

  return bindResult;
}


void PlotSlice::unbind()
{
  if (_indexee)
  {
    _indexee->unbind();
  }
  if (_indexerStart)
  {
    _indexerStart->unbind();
  }
  if (_indexerEnd)
  {
    _indexerEnd->unbind();
  }
}


PlotExpression *PlotSlice::clone() const
{
  return new PlotSlice(_indexee ? _indexee->clone() : nullptr,
                       _indexerStart ? _indexerStart->clone() : nullptr,
                       _indexerEnd ? _indexerEnd->clone() : nullptr);
}


QString PlotSlice::stringExpression() const
{
  QString str;
  QTextStream stream(&str);
  stream << indexee()->toString() << '[';
  if (indexerStart())
  {
    stream << indexerStart()->toString();
  }
  stream << ':';
  if (indexerEnd())
  {
    stream << indexerEnd()->toString();
  }
  stream << ']';
  return str;
}
