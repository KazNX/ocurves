//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#include "plotexpressiongenerator.h"

#include "expr/plotbindingtracker.h"
#include "expr/plotexpression.h"
#include "plotfile.h"
#include "plotinstance.h"

#include "model/curves.h"

struct GenerationMarker
{
  bool complete;
  int index;
};

PlotExpressionGenerator::PlotExpressionGenerator(Curves *curves, const QList<PlotExpression *> &expressions, const QStringList &sourceNames)
  : PlotGenerator(curves)
  , _marker(new GenerationMarker( { true, 0 }))
{
  _sourceNames = sourceNames;

  for (const PlotExpression *exp : expressions)
  {
    _expressions.append({ exp->clone(), exp });
  }

  for (const PlotInstance *curve : curves->curves())
  {
    PlotInstance *c = new PlotInstance(*curve);
    _existingCurves.push_back(c);
  }
}


PlotExpressionGenerator::~PlotExpressionGenerator()
{
  for (ExpressionPair &e : _expressions)
  {
    delete e.expression;
  }

  for (PlotInstance *curve : _existingCurves)
  {
    delete curve;
  }

  delete _marker;
}


bool PlotExpressionGenerator::addExpression(PlotExpression *expression)
{
  QMutexLocker lock(_dataMutex);
  if (!_marker->complete)
  {
    _expressions.append({ expression->clone(), expression });
    return true;
  }
  return false;
}


bool PlotExpressionGenerator::removeExpression(const PlotExpression *expression)
{
  QMutexLocker lock(_dataMutex);
  if (!_marker->complete)
  {
    for (int i = 0; i < _expressions.count(); ++i)
    {
      if (_expressions[i].original == expression)
      {
        // Found curve to remove.
        // Have we processed it yet?
        if (_marker->index < i)
        {
          // No. We can remove this item.
          delete  _expressions[i].expression;
          _expressions.removeAt(i);
          return true;
        }

        // Too late.
        return false;
      }
    }

    // Didn't find it to remove.
    return true;
  }

  return false;
}


void PlotExpressionGenerator::run()
{
  int generatedCount = 0;
  QMutexLocker lock(_dataMutex);
  *_marker = { false, 0 };
  if (!_expressions.empty())
  {
    emit overallProgress(0, _expressions.count());
    int processedCount = 0;

    QList<PlotInstance *> newCurves;
    const int itemCount = std::max(1, _sourceNames.count());

    for (; _marker->index < _expressions.count(); ++_marker->index)
    {
      int i = _marker->index;
      lock.unlock();

      if (_abortFlag)
      {
        break;
      }

      PlotExpression *exp = _expressions[i].expression;
      const PlotExpression *originalExpression = _expressions[i].original;
      int itemIndex = 0;
      emit itemProgress(0);
      emit itemName(exp->toString());
      PlotExpressionBindDomain domain;
      PlotBindingTracker bindTracker;
      BindResult bindResult;

      bindResult = exp->bind(_existingCurves, bindTracker, domain);
      while (bindResult > 0)
      {
        // Expression binds. We can create a curve for this. Use the original source if possible.
        PlotSource *source = nullptr;

        // There will be a 'first plot' only when we have PlotSample references,
        // which relate to a source (file).
        if (bindTracker.firstPlot())
        {
          source = &bindTracker.firstPlot()->source();
        }
        else
        {
          source = new PlotSource(PlotSource::Expression, exp->toString());
          source->setTimeColumn(0);
          source->setTimeBase(0);
        }

        PlotInstance *c = new PlotInstance(source);
        c->setName(exp->toString());
        c->setExpression(originalExpression);

        // We may be generating a duplicate curve. This can occur when we load
        // a file, generate expression curves, then load another file and generate
        // new expression curves. We may rebind on the first set of curves.
        if (!curveExists(*c))
        {
          newCurves.append(c);
          emit beginNewCurves();
          _curves->newCurve(c);
          emit endNewCurves();

          // Note: if this sampling loop is changed, then the comments on
          // PlotExpressionBindDomain must be adjusted to reflect the changes.
          // Be sure to keep the logic and comments in sync.
          const double startTime = domain.domainMin;
          for (uint i = 0; i < domain.sampleCount; ++i)
          {
            double time = std::min(startTime + i * domain.sampleDelta, domain.domainMax);
            double val = exp->sample(time);
            c->addPoint(QPointF(time, val));
          }
        }
        else
        {
          delete c;
          c = nullptr;
        }

        exp->unbind();
        if (bindResult == BoundMaybeMore)
        {
          bindTracker.clearFirstPlot();
          bindResult = exp->bind(_existingCurves, bindTracker, domain);
        }
        else
        {
          bindResult = BindFailure;
        }
        emit itemProgress((100 * ++itemIndex) / (100 * itemCount));
      }

      emit overallProgress(++processedCount, _expressions.count());
      lock.relock();
    }

    _marker->complete = true;

    // Done reading.
    if (!_abortFlag)
    {
      for (int i = 0; i < newCurves.count(); ++i)
      {
        // Add to the owning source and notify completion.
        PlotInstance *c = newCurves[i];
        c->source().addCurve(c);
        _curves->completeLoading(c);
      }

      generatedCount += newCurves.size();
    }
    else
    {
      for (int i = 0; i < newCurves.count(); ++i)
      {
        // Add to allocated list.
        PlotInstance *c = newCurves[i];
        _curves->removeCurve(c);
      }
    }
  }
  else
  {
    _marker->complete = true;
  }

  lock.unlock();
  emit loadComplete(generatedCount);
}


bool PlotExpressionGenerator::curveExists(const PlotInstance &curve) const
{
  for (const PlotInstance *other : _existingCurves)
  {
    if (curve.source().fullName() == other->source().fullName() &&
        curve.name() == other->name())
    {
      return true;
    }
  }

  return false;
}
