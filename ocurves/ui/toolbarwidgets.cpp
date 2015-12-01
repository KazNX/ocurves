//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#include "toolbarwidgets.h"
#include "ui_toolbarwidgets.h"

#include <QDoubleValidator>

ToolbarWidgets::ToolbarWidgets(QWidget *parent)
  : QWidget(parent)
  , _ui(new Ui::ToolbarWidgets)
{
  _ui->setupUi(this);
  _ui->timeScaleEdit->setValidator(new QDoubleValidator(0, std::numeric_limits<double>::max(), 6, _ui->timeScaleEdit));
}


ToolbarWidgets::~ToolbarWidgets()
{
  delete _ui;
}


QSpinBox *ToolbarWidgets::maxSamplesSpin()
{
  return _ui->targetSamplesSpin;
}


QLineEdit *ToolbarWidgets::timeScaleEdit()
{
  return _ui->timeScaleEdit;
}


QCheckBox *ToolbarWidgets::timeColumnCheck()
{
  return _ui->timeColumnCheck;
}


QSpinBox *ToolbarWidgets::timeColumnSpin()
{
  return _ui->timeColumnSpin;
}


QCheckBox *ToolbarWidgets::relativeTimeCheck()
{
  return _ui->relativeTimeCheck;
}
