//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2013
//
#include "coloursview.h"
#include "ui_coloursview.h"

#include "defaultcolours.h"

#include <QColorDialog>
#include <QMenu>
#include <QRegExp>

namespace
{
  QString toHexString(const QRgb rgb)
  {
    return QString("%1%2%3").arg(qRed(rgb), 2, 16, QChar('0'))
           .arg(qGreen(rgb), 2, 16, QChar('0'))
           .arg(qBlue(rgb), 2, 16, QChar('0'));
  }

  bool parseColour(const QString &parseString, QRgb &rgb)
  {
    QRegExp hexExp("([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})");
    QRegExp rgbExp("([1-2]{,1}[0-9]{1,2})\\s*,\\s*([1-2]{,1}[0-9]{1,2})\\s*,\\s*([1-2]{,1}[0-9]{1,2})");

    QString str = parseString.trimmed();
    if (hexExp.exactMatch(str))
    {
      auto c = hexExp.capturedTexts();
      rgb = qRgb(hexExp.cap(1).toInt(nullptr, 16), hexExp.cap(2).toInt(nullptr, 16), hexExp.cap(3).toInt(nullptr, 16));
      return true;
    }
    else if (rgbExp.exactMatch(str))
    {
      auto c = rgbExp.capturedTexts();
      rgb = qRgb(rgbExp.cap(1).toInt(), rgbExp.cap(2).toInt(), rgbExp.cap(3).toInt());
      return true;
    }

    return false;
  }

  bool greaterThan(int a, int b)
  {
    return a > b;
  }
}

ColoursView::ColoursView(QWidget *parent)
  : QDialog(parent)
  , _ui(new Ui::ColoursView)
  , _resetSelectionMenu(nullptr)
  , _ignoreChanges(false)
{
  _ui->setupUi(this);
  connect(_ui->colourTable, &QTableWidget::itemChanged, this, &ColoursView::edited);
  connect(_ui->colourTable, &QTableWidget::cellDoubleClicked, this, &ColoursView::cellDoubleClick);
  connect(_ui->addButton, &QPushButton::clicked, this, &ColoursView::addItem);
  connect(_ui->removeButton, &QPushButton::clicked, this, &ColoursView::removeItem);
  connect(_ui->resetButton, &QPushButton::clicked, this, &ColoursView::resetColours);
  connect(_ui->colourTable, &QTableWidget::itemSelectionChanged, this, &ColoursView::cellSelectionChange);

  // Build the reset context menu.
  _resetSelectionMenu = new QMenu(this);
  for (unsigned i = 0; i < ocurves::ColourSetCount; ++i)
  {
    QAction *action = _resetSelectionMenu->addAction(tr(ocurves::DefaultColourSetNames[i]));
    action->setData(i);
    connect(action, &QAction::triggered, this, &ColoursView::resetActionTriggered);
  }
}

ColoursView::~ColoursView()
{
  delete _ui;
}


void ColoursView::editColour(int colourIndex)
{
  if (colourIndex < 0 && colourIndex >= _colours.size())
  {
    return;
  }

  QColor c(_colours[colourIndex]);
  QColorDialog dialog(c);
  if (dialog.exec() == QDialog::Accepted)
  {
    QRgb colour = dialog.currentColor().rgb();
    _colours[colourIndex] = colour;
    _ui->colourTable->item(colourIndex, ColHex)->setText(toHexString(colour));
    _ui->colourTable->item(colourIndex, ColColour)->setBackgroundColor(colour);
  }
}


void ColoursView::addItem()
{
  _ignoreChanges = true;
  QRgb colour = qRgb(0, 0, 0);
  _colours.push_back(colour);
  int row = _ui->colourTable->rowCount();
  _ui->colourTable->setRowCount(row + 1);
  QTableWidgetItem *hex = new QTableWidgetItem(toHexString(colour));
  QTableWidgetItem *col = new QTableWidgetItem();
  col->setFlags(col->flags() & ~(Qt::ItemIsEditable));
  col->setBackgroundColor(colour);
  _ui->colourTable->setItem(row, ColHex, hex);
  _ui->colourTable->setItem(row, ColColour, col);
  _ignoreChanges = false;
  editColour(row);
}


void ColoursView::removeItem()
{
  _ignoreChanges = true;
  // Remove items, but preserve at least one item in the table.
  QVector<int> removeIndices;
  QList<QTableWidgetItem *> selection = _ui->colourTable->selectedItems();
  // Build row index list. This may have duplicates due to cells being selected from the same rows.
  foreach (QTableWidgetItem *item, selection)
  {
    if (!removeIndices.contains(item->row()))
    {
      removeIndices.push_back(item->row());
    }
  }

  // Now remove the colours internally. We sort them to remove in reverse order
  // (the greatest index first), which will ensure correct removal.
  qSort(removeIndices.begin(), removeIndices.end(), greaterThan);

  if (removeIndices.size() == _ui->colourTable->rowCount())
  {
    // Everything selected. Keep something.
    removeIndices.removeLast();
  }

  for (int i = 0; i < removeIndices.size(); ++i)
  {
    _ui->colourTable->removeRow(removeIndices[i]);
    _colours.removeAt(removeIndices[i]);
  }

  _ignoreChanges = false;
}


void ColoursView::resetColours()
{
  // Show colour set selection menu.
  if (_resetSelectionMenu)
  {
    QPoint pos = mapToGlobal(_ui->resetButton->frameGeometry().topRight());
    _resetSelectionMenu->popup(pos);
  }
}


void ColoursView::resetActionTriggered()
{
  if (QAction *action = qobject_cast<QAction *>(sender()))
  {
    bool ok = false;
    unsigned setId = action->data().toUInt(&ok);
    if (ok && setId < ocurves::ColourSetCount)
    {
      _colours.clear();

      for (unsigned i = 0; i < ocurves::DefaultColourSetCounts[setId]; ++i)
      {
        _colours.append(ocurves::WebSafeColours[ocurves::DefaultColourSets[setId][i]]);
      }

      populateColourTable();
    }
  }
}


void ColoursView::cellSelectionChange()
{
  if (_ignoreChanges)
  {
    return;
  }

  QList<QTableWidgetItem *> selection = _ui->colourTable->selectedItems();
  QVector<int> reselect;
  _ignoreChanges = true;
  for (QTableWidgetItem *item : selection)
  {
    if (item->column() == ColColour)
    {
      reselect.push_back(item->row());
      item->setSelected(false);
      _ui->colourTable->item(item->row(), ColHex)->setSelected(true);
    }
  }
  _ignoreChanges = false;
}


void ColoursView::edited(QTableWidgetItem *item)
{
  if (_ignoreChanges || item->column() != 0)
  {
    return;
  }

  QRgb rgb = 0;
  if (parseColour(item->text(), rgb))
  {
    QTableWidgetItem *col = _ui->colourTable->item(item->row(), 1);
    col->setBackgroundColor(rgb);
  }
  else
  {
    rgb = _colours[item->row()];
  }
  _ignoreChanges = true;
  item->setText(toHexString(rgb));
  _ignoreChanges = false;
}


void ColoursView::cellDoubleClick(int row, int column)
{
  if (column == ColColour)
  {
    editColour(row);
  }
}


void ColoursView::showEvent(QShowEvent *event)
{
  // Refresh colour table.
  populateColourTable();
}


void ColoursView::populateColourTable()
{
  int row = 0;
  _ignoreChanges = true;
  _ui->colourTable->clearContents();
  if (!_colours.isEmpty())
  {
    QString str("%1%2%3");
    str = str.arg(255, 2, 16, QChar('0'));
    str = str.arg(0, 2, 16, QChar('0'));
    str = str.arg(0, 2, 16, QChar('0'));
    _ui->colourTable->setRowCount(_colours.count());
    foreach (QRgb colour, _colours)
    {
      QTableWidgetItem *hex = new QTableWidgetItem(toHexString(colour));
      QTableWidgetItem *col = new QTableWidgetItem();
      col->setFlags(col->flags() & ~(Qt::ItemIsEditable));
      col->setBackgroundColor(colour);
      _ui->colourTable->setItem(row, ColHex, hex);
      _ui->colourTable->setItem(row, ColColour, col);
      ++row;
    }
  }
  _ignoreChanges = false;
}
