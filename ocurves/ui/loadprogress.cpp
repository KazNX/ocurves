//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2012
//
#include "loadprogress.h"
#include "ui_loadprogress.h"

LoadProgress::LoadProgress(QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::LoadProgress)
{
  ui->setupUi(this);
  ui->itemProgress->setRange(0, CurrentItemMax);
  ui->itemProgress->setValue(0);
  ui->overallProgress->setRange(0, 0);
  ui->overallProgress->setValue(0);

  connect(ui->cancelButton, &QPushButton::clicked, this, &LoadProgress::onCancel);
}

LoadProgress::~LoadProgress()
{
  delete ui;
}


void LoadProgress::itemName(const QString &name)
{
  if (!name.isEmpty())
  {
    ui->currentEdit->setText(name);
  }
  else
  {
    ui->currentEdit->setText(tr("Progress"));
  }
}


void LoadProgress::itemProgress(int current)
{
  ui->itemProgress->setValue(current);
}


void LoadProgress::overallProgress(int current, int total)
{
  if (total && total != ui->overallProgress->maximum())
  {
    ui->overallProgress->setRange(current, total);
  }
  ui->overallProgress->setValue(current);
}


void LoadProgress::loadComplete(int /*overallCount*/)
{
  hide();
  deleteLater();
}


void LoadProgress::onCancel()
{
  emit cancel();
}
