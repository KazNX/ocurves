//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#include "expressions.h"

#include <QSettings>

#include "expr/plotexpression.h"
#include "expr/plotexpressionparser.h"
#include "expr/plotfunctionregister.h"

Expressions::Expressions(QObject *parent)
  : QObject(parent)
  , _functionRegister(new PlotFunctionRegister(true))
{
}


Expressions::~Expressions()
{
  clear(false);
  delete _functionRegister;
}


void Expressions::clear(bool suppressSignals)
{
  for (PlotExpression *expr : _expressions)
  {
    if (!suppressSignals)
    {
      emit expressionRemoved(expr);
    }
    delete expr;
  }
  _expressions.clear();
}


void Expressions::loadExpressions(const QSettings &settings, bool delaySignals)
{
  PlotExpressionParser parser(_functionRegister);
  QList<PlotExpression *> initialExpressions;
  if (!delaySignals)
  {
    clear(false);
  }
  else
  {
    initialExpressions = _expressions;
    _expressions.clear();
  }

   // We'll allow some missing expressions.
  unsigned missing = 0;
  unsigned count = 0;
  QString key = QString("exp%1").arg(++count);
  while (missing < missingTolerance)
  {
    QString expStr = settings.value(key).toString();

    if (!expStr.isEmpty())
    {
      missing = 0;
      QStringList errors;
      PlotExpression *exp = parser.parse(expStr, errors);
      if (exp)
      {
        _expressions.append(exp);
        if (!delaySignals)
        {
          emit expressionAdded(exp);
        }
      }
    }
    else
    {
      ++missing;
    }

    key = QString("exp%1").arg(++count);
  }

  if (delaySignals)
  {
    for (PlotExpression *expr : initialExpressions)
    {
      emit expressionRemoved(expr);
      delete expr;
    }

    for (PlotExpression *expr : _expressions)
    {
      emit expressionAdded(expr);
    }
  }
}


void Expressions::saveExpressions(QSettings &settings)
{
  unsigned int count = 0;
  for (PlotExpression *exp : _expressions)
  {
    settings.setValue(QString("exp%1").arg(++count), exp->toString());
  }

  // Clear out additional expression entries (handles deletes).
  // We'll allow some missing expressions.
  unsigned missing = 0;
  QString key = QString("exp%1").arg(++count);
  while (missing < missingTolerance)
  {
    if (!settings.value(key).toString().isEmpty())
    {
      missing = 0;
      settings.remove(key);
    }
    else
    {
      ++missing;
    }
    key = QString("exp%1").arg(++count);
  }
}


void Expressions::addExpression(PlotExpression *expression)
{
  _expressions << expression;
  emit expressionAdded(expression);
}


bool Expressions::removeExpression(PlotExpression *expression)
{
  if (_expressions.removeOne(expression))
  {
    emit expressionRemoved(expression);
    return true;
  }
  return false;
}


bool Expressions::contains(const PlotExpression *expression) const
{
  for (const PlotExpression *e : _expressions)
  {
    if (e == expression)
    {
      return true;
    }
  }
  return false;
}


bool Expressions::registerFunction(FunctionDefinition *functionDef, bool takeOwnership)
{
  if (_functionRegister->add(functionDef, takeOwnership))
  {
    emit functionRegistered(functionDef);
    return true;
  }

  return false;
}
