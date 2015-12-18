//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2013
//
#include "expressionsview.h"
#include "ui_expressionsview.h"

#include "model/expressions.h"

#include "expr/plotexpression.h"
#include "expr/plotexpressionparser.h"
#include "expr/plotfunctionregister.h"

#include <QTableWidget>

ExpressionsView::ExpressionsView(Expressions *expressions, QWidget *parent)
  : QWidget(parent)
  , _ui(new Ui::ExpressionsView)
  , _expressions(nullptr)
  , _suppressEvents(false)
{
  _ui->setupUi(this);
  connect(_ui->addButton, &QPushButton::clicked, this, &ExpressionsView::addExpr);
  connect(_ui->updateButton, &QPushButton::clicked, this, &ExpressionsView::updateExpr);
  connect(_ui->removeButton, &QPushButton::clicked, this, static_cast<void (ExpressionsView::*)()>(&ExpressionsView::removeExpr));
  connect(_ui->expressionList, &QListWidget::currentItemChanged, this, &ExpressionsView::currentItemChanged);
  //connect(_ui->expressionEdit, &QLineEdit::editingFinished, this, &ExpressionsView::editingDone);
  setExpressions(expressions);
}


ExpressionsView::~ExpressionsView()
{
  delete _ui;
}


void ExpressionsView::setExpressions(Expressions *expressions)
{
  if (_expressions)
  {
    disconnect(_expressions, &Expressions::expressionAdded, this, &ExpressionsView::expressionAdded);
    disconnect(_expressions, &Expressions::expressionRemoved, this, &ExpressionsView::expressionRemoved);
  }

  _expressions = expressions;

  // Remove all function category tabs.
  while (_ui->functionsTabs->count())
  {
    QWidget *tab = _ui->functionsTabs->widget(0);
    _ui->functionsTabs->removeTab(0);
    delete tab;
  }

  if (_expressions)
  {
    connect(_expressions, &Expressions::expressionAdded, this, &ExpressionsView::expressionAdded);
    connect(_expressions, &Expressions::expressionRemoved, this, &ExpressionsView::expressionRemoved);

    QStringList categories = _expressions->functionRegister().categories();
    QVector<const FunctionDefinition *> functions;
    if (!categories.isEmpty())
    {
      //_expressions->functionsTabs
      for (const QString &category : _expressions->functionRegister().categories())
      {
        functions.clear();
        if (_expressions->functionRegister().getDefinitionsInCategory(category, functions))
        {
          // ensure the function list is sorted.
          qSort(functions.begin(), functions.end(),
                [](const FunctionDefinition * a, const FunctionDefinition * b)
          {
            return a->name() < b->name();
          });

          QTableWidget *functionsTable = createFunctionsTable(functions, _ui->functionsTabs);
          _ui->functionsTabs->addTab(functionsTable, category);
        }
      }
    }
  }

  showExpressions();
}


PlotExpression *ExpressionsView::parseExpression(const QString &text, QStringList &errors) const
{
  if (!_expressions)
  {
    return nullptr;
  }

  PlotExpressionParser parser(&_expressions->functionRegister());
  return parser.parse(text, errors);
}


PlotExpression *ExpressionsView::addExpression()
{
  if (!_expressions)
  {
    return nullptr;
  }

  QStringList errors;
  _ui->errorLine->clear();
  if (PlotExpression *expr = parseExpression(_ui->expressionEdit->text(), errors))
  {
    _expressions->addExpression(expr);
    return expr;
  }

  if (!errors.isEmpty())
  {
    _ui->errorLine->setText(errors.front());
  }
  else
  {
    _ui->errorLine->setText(tr("Parse error"));
  }
  return nullptr;
}


void ExpressionsView::addExpr()
{
  addExpression();
}


void ExpressionsView::removeExpr()
{
  if (!_expressions)
  {
    return;
  }

  removeExpression(_ui->expressionList->currentItem());
}


void ExpressionsView::updateExpr()
{
  if (!_expressions)
  {
    return;
  }

  QListWidgetItem *toUpdate = _ui->expressionList->currentItem();

  if (!toUpdate || toUpdate->text().compare(_ui->expressionEdit->text()) == 0)
  {
    // No change.
    return;
  }

  QStringList errors;
  _ui->errorLine->clear();
  if (PlotExpression *expr = parseExpression(_ui->expressionEdit->text(), errors))
  {
    PlotExpression *oldExpr = static_cast<PlotExpression *>(toUpdate->data(Qt::UserRole).value<void *>());
    void *ptr = expr;
    toUpdate->setData(Qt::UserRole, qVariantFromValue(ptr));
    toUpdate->setText(expr->toString());
    _suppressEvents = true;
    _expressions->addExpression(expr);
    _expressions->removeExpression(oldExpr);
    delete oldExpr;
    _suppressEvents = false;
  }
}


void ExpressionsView::currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
  // Does the current edit box match the previous?
  bool matchesPrevious = !previous && _ui->expressionEdit->text().isEmpty() ||
                         previous && previous->text().compare(_ui->expressionEdit->text()) == 0;

  // Change displayed text if we can. Don't if it has been edited.
  if (current)
  {
    // We have a new item. Check the text against the previous item to see if there are changes.
    if (matchesPrevious)
    {
      // No text and no previous selection, or text matches previous selection.
      // It's ok to change to the new item.
      _ui->expressionEdit->setText(current->text());
    }
  }
  else if (matchesPrevious)
  {
    // Clear the current text.
    _ui->expressionEdit->clear();
  }
}


void ExpressionsView::editingDone()
{
  // Editing done. We either update the currently selected item, or add a new item.
  if (_ui->expressionList->currentItem())
  {
    updateExpr();
  }
  else
  {
    addExpr();
  }
}


void ExpressionsView::expressionAdded(PlotExpression *expression)
{
  if (_suppressEvents)
  {
    return;
  }
  QListWidgetItem *item = createItem(expression);
  _ui->expressionList->addItem(item);
  item->setSelected(true);
}


void ExpressionsView::expressionRemoved(const PlotExpression *expression)
{
  if (_suppressEvents)
  {
    return;
  }
  for (int i = 0; i < _ui->expressionList->count(); ++i)
  {
    QListWidgetItem *item = _ui->expressionList->item(i);
    void *ptr = item->data(Qt::UserRole).value<void *>();
    PlotExpression *expr = static_cast<PlotExpression *>(item->data(Qt::UserRole).value<void *>());
    if (expr == expression)
    {
      _ui->expressionList->removeItemWidget(item);
      delete item;
      break;
    }
  }
}


void ExpressionsView::showExpressions()
{
  _ui->expressionList->clear();

  if (_expressions)
  {
    for (PlotExpression *exp : _expressions->expressions())
    {
      _ui->expressionList->addItem(createItem(exp));
    }
  }
}


QListWidgetItem *ExpressionsView::createItem(PlotExpression *expression)
{
  QListWidgetItem *item = new QListWidgetItem(expression->toString(), _ui->expressionList);
  void *ptr = expression;
  item->setData(Qt::UserRole, qVariantFromValue(ptr));
  return item;
}


bool ExpressionsView::removeExpression(QListWidgetItem *item)
{
  if (item)
  {
    void *ptr = item->data(Qt::UserRole).value<void *>();
    PlotExpression *expr = static_cast<PlotExpression *>(item->data(Qt::UserRole).value<void *>());
    _ui->expressionList->removeItemWidget(item);
    delete item;
    if (_expressions)
    {
      _expressions->removeExpression(expr);
    }
    return true;
  }
  return false;
}


QTableWidget *ExpressionsView::createFunctionsTable(const QVector<const FunctionDefinition *> &functions, QWidget *parent)
{
  QTableWidget *functionsTable = new QTableWidget(parent);
  if (functionsTable->columnCount() < 2)
  {
    functionsTable->setColumnCount(2);
  }
  QTableWidgetItem *column = new QTableWidgetItem();

  functionsTable->setHorizontalHeaderItem(0, column);
  column->setText(tr("Function"));
  column = new QTableWidgetItem();
  column->setText(tr("Description"));
  functionsTable->setHorizontalHeaderItem(1, column);
  functionsTable->horizontalHeader()->setStretchLastSection(true);

  int row = 0;
  for (const FunctionDefinition *def : functions)
  {
    functionsTable->insertRow(row);
    functionsTable->setItem(row, 0, new QTableWidgetItem(def->displayName()));
    functionsTable->setItem(row, 1, new QTableWidgetItem(def->description()));
    ++row;
  }

  return functionsTable;
}
