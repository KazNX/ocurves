//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#include "plotinstancesampler.h"

#include "plotinstance.h"
#include "plotutil.h"

namespace
{
  // From qwt_series_data.cpp
  inline QRectF qwtBoundingRect(const QPointF &sample)
  {
    return QRectF(sample.x(), sample.y(), 0.0, 0.0);
  }
}



PlotInstanceSampler::PlotInstanceSampler(const PlotInstance *curveData)
  : _curve(curveData)
  , _lastRingHead(0)
  , _lastRingSize(0)
{
}


void PlotInstanceSampler::setCurve(const PlotInstance *curveData)
{
  _curve = curveData;
  _boundingRect = QRectF(0, 0, 0, 0);
}


size_t PlotInstanceSampler::size() const
{
  return _curve->data().size();
}


QPointF PlotInstanceSampler::sample(size_t i) const
{
  if (!_curve->data().empty())
  {
    // Fetch the initial sample.
    typedef std::numeric_limits<qreal> Limits;
    QPointF sample = _curve->sample(i);

    // Filter NaN and infinite results.
    if (_curve->flags() & (PlotInstance::FilterNaN | PlotInstance::FilterInf))
    {
      if (_curve->filterNaN() && sample.y() != sample.y() ||
          _curve->filterInf() && (sample.y() == Limits::infinity() || sample.y() == -Limits::infinity()))
      {
        sample.setY(0);
      }
    }

    // Adjust the X value (time) for the sample. This looks up the shared source
    // and its timing information: time column, scaling and time shift.
    sample.setX(lookupSampleTime(sample.x(), i));

    return sample;
  }
  return QPointF(0, 0);
}


QRectF PlotInstanceSampler::boundingRect() const
{
  if (_boundingRect.width() == 0 || _lastRingHead != _curve->ringHead() || _lastRingSize != _curve->data().size())
  {
    _boundingRect = calculateBoundingRect();
  }

  _lastRingHead = _curve->ringHead();
  _lastRingSize = _curve->data().size();
  return _boundingRect;
}


void PlotInstanceSampler::invalidateBoundingRect()
{
  _boundingRect = QRectF(0, 0, 0, 0);
}


QRectF PlotInstanceSampler::calculateBoundingRect(size_t from, size_t to) const
{
  // From qwt_series_data.cpp
  QRectF boundingRect(1.0, 1.0, -2.0, -2.0); // invalid;

  if (to == ~(size_t)(0u))
  {
    to = size() - 1;
  }

  if (!size() || to < from)
  {
    return boundingRect;
  }

  size_t i;
  for (i = from; i <= to; i++)
  {
    const QRectF rect = qwtBoundingRect(sample(i));
    if (rect.width() >= 0.0 && rect.height() >= 0.0)
    {
      boundingRect = rect;
      i++;
      break;
    }
  }

  for (; i <= to; i++)
  {
    const QRectF rect = qwtBoundingRect(sample(i));
    if (rect.width() >= 0.0 && rect.height() >= 0.0)
    {
      boundingRect.setLeft(qMin(boundingRect.left(), rect.left()));
      boundingRect.setRight(qMax(boundingRect.right(), rect.right()));
      boundingRect.setTop(qMin(boundingRect.top(), rect.top()));
      boundingRect.setBottom(qMax(boundingRect.bottom(), rect.bottom()));
    }
  }

  return boundingRect;
}


double PlotInstanceSampler::lookupSampleTime(double initialTime, size_t i) const
{
  double time = initialTime;
  const PlotSource &source = _curve->source();
  // Lookup the requested sample in the time column if required.
  if (PlotInstance *timeCurve = source.timeColumnCurve())
  {
    time = timeCurve->sample(i).y();
  }

  // Time shift before scaling.
  time -= source.timeBase();
  // Scale.
  time *= source.timeScale();
  return time;
}
