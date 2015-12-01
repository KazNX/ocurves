//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#include "curveproperties.h"

#include "model/curves.h"
#include "plotinstance.h"

#include "ui_curveproperties.h"

#include <QColorDialog>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QEvent>
#include <QMouseEvent>
#include <QResizeEvent>


CurveProperties::CurveProperties(Curves *curves, QWidget *parent)
  : QWidget(parent)
  , _ui(new Ui::CurveProperties)
  , _curves(curves)
  , _curve(nullptr)
  , _suppressEvents(false)
{
  _ui->setupUi(this);

  _timeColumnValidator = new QIntValidator(0, 1000000, _ui->timeColumnEdit);
  _ui->timeColumnEdit->setValidator(_timeColumnValidator);
  _ui->timeScaleEdit->setValidator(new QDoubleValidator(1e-20, 1e20, 6, _ui->timeScaleEdit));
  _ui->timeBaseEdit->setValidator(new QDoubleValidator(-1e20, 1e20, 6, _ui->timeBaseEdit));

  setColour(_ui->colourWidget, QColor(Qt::white).rgb());
  _ui->colourWidget->installEventFilter(this);

  connect(_ui->timeColumnEdit, &QLineEdit::editingFinished, this, &CurveProperties::timeColumnChanged);
  connect(_ui->timeScaleEdit, &QLineEdit::editingFinished, this, &CurveProperties::timeScaleChanged);
  connect(_ui->timeBaseEdit, &QLineEdit::editingFinished, this, &CurveProperties::timeBaseChanged);
  connect(_ui->restoreColourButton, &QToolButton::toggled, this, &CurveProperties::toggleExplicitColour);
  connect(_ui->widthSpin, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, static_cast<void(CurveProperties::*)(int)>(&CurveProperties::widthChanged));
  connect(_ui->styleCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, static_cast<void(CurveProperties::*)(int)>(&CurveProperties::styleChanged));
  connect(_ui->sizeSpin, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, static_cast<void(CurveProperties::*)(int)>(&CurveProperties::symbolSizeChanged));
  connect(_ui->symbolCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, static_cast<void(CurveProperties::*)(int)>(&CurveProperties::symbolChanged));
  connect(_ui->resetCurveButton, &QPushButton::pressed, this, &CurveProperties::resetCurveProperties);

  connect(curves, &Curves::curveDataChanged, this, &CurveProperties::curveChanged);
  connect(curves, &Curves::sourceDataChanged, this, &CurveProperties::sourceChanged);

  update(nullptr);
}


CurveProperties::~CurveProperties()
{
  delete _ui;
}



void CurveProperties::setActiveCurve(PlotInstance *curve)
{
  if (curve != _curve)
  {
    _suppressEvents = true;
    _curve = curve;
    update(_curve);
    _suppressEvents = false;
    emit activeCurveChanged(curve);
  }
}


void CurveProperties::curveChanged(const PlotInstance *curve)
{
  if (_suppressEvents)
  {
    return;
  }

  if (curve == _curve)
  {
    // Curve changed. Refresh.
    update(_curve);
  }
}


void CurveProperties::sourceChanged(const PlotSource *source)
{
  if (_suppressEvents)
  {
    return;
  }

  if (_curve && source == &_curve->source())
  {
    // Curve source has changed.
    update(_curve);
  }
}


void CurveProperties::timeColumnChanged()
{
  if (_suppressEvents || !_curve)
  {
    return;
  }

  const unsigned val = _ui->timeColumnEdit->text().toUInt();
  _curve->source().setTimeColumn(val);
  _ui->firstTimeEdit->setText(QString::number(_curve->source().firstTime()));
  invalidateSource(_curve->source());
}


void CurveProperties::timeScaleChanged()
{
  if (_suppressEvents || !_curve)
  {
    return;
  }

  const double val = _ui->timeScaleEdit->text().toDouble();
  _curve->source().setTimeScale(val);
  invalidateSource(_curve->source());
}


void CurveProperties::timeBaseChanged()
{
  if (_suppressEvents || !_curve)
  {
    return;
  }

  const double val = _ui->timeBaseEdit->text().toDouble();
  _curve->source().setTimeBase(val);
  invalidateSource(_curve->source());
}


void CurveProperties::toggleExplicitColour(bool checked)
{
  if (_suppressEvents || !_curve)
  {
    return;
  }

  if (!checked)
  {
    // Unlock explicit colouring.
    _curve->setColour(_curve->colour(), false);
    invalidateCurve(*_curve);
  }
}


void CurveProperties::widthChanged(int width)
{
  if (!_curve)
  {
    return;
  }

  _curve->setWidth(width);
  if (!_suppressEvents)
  {
    invalidateCurve(*_curve);
  }
}


void CurveProperties::styleChanged(int index)
{
  if (!_curve)
  {
    return;
  }

  _curve->setStyle(index - 1);
  if (!_suppressEvents)
  {
    invalidateCurve(*_curve);
  }
}


void CurveProperties::symbolSizeChanged(int size)
{
  if (!_curve)
  {
    return;
  }

  _curve->setSymbolSize(size);
  if (!_suppressEvents)
  {
    invalidateCurve(*_curve);
  }
}


void CurveProperties::symbolChanged(int index)
{
  if (!_curve)
  {
    return;
  }

  _curve->setSymbol(index - 1);
  if (!_suppressEvents)
  {
    invalidateCurve(*_curve);
  }
}


void CurveProperties::filterInfChecked(bool checked)
{
  if (_suppressEvents || !_curve)
  {
    return;
  }

  _curve->setFilterInf(_ui->filterInfCheck->isChecked());
  invalidateCurve(*_curve);
}


void CurveProperties::filterNaNChecked(bool checked)
{
  if (_suppressEvents || !_curve)
  {
    return;
  }

  _curve->setFilterInf(_ui->filterNaNCheck->isChecked());
  invalidateCurve(*_curve);
}


void CurveProperties::resetCurveProperties()
{
  if (_curve)
  {
    _curve->setDefaultProperties();
    invalidateCurve(*_curve);
    update(_curve);
  }
}


void CurveProperties::update(PlotInstance *curve)
{
  if (curve)
  {
    _ui->sourceNameEdit->setText(curve->source().name());
    _ui->fullNameEdit->setText(curve->source().fullName());
    _ui->curveNameEdit->setText(curve->name());
    _ui->timeColumnEdit->setText(QString::number(curve->source().timeColumn()));
    _ui->timeScaleEdit->setText(QString::number(curve->source().timeScale()));
    _ui->timeBaseEdit->setText(QString::number(curve->source().timeBase()));
    _ui->firstTimeEdit->setText(QString::number(curve->source().firstTime()));
    setColour(_ui->colourWidget, curve->colour());
    _ui->restoreColourButton->setChecked(curve->explicitColour());
    _ui->styleCombo->setCurrentIndex(curve->style() + 1);
    _ui->widthSpin->setValue(curve->width());
    _ui->symbolCombo->setCurrentIndex(curve->symbol() + 1);
    _ui->sizeSpin->setValue(curve->symbolSize());
    _ui->filterInfCheck->setChecked(curve->filterInf());
    _ui->filterNaNCheck->setChecked(curve->filterNaN());
    _timeColumnValidator->setTop(curve->source().curveCount());
  }
  else
  {
    _ui->sourceNameEdit->clear();
    _ui->fullNameEdit->clear();
    _ui->curveNameEdit->clear();
    _ui->timeColumnEdit->clear();
    _ui->timeScaleEdit->clear();
    _ui->timeBaseEdit->setText(QString::number(0.0));
    _ui->firstTimeEdit->setText(QString::number(0.0));
    setColour(_ui->colourWidget, QColor(Qt::white).rgb());
    _ui->restoreColourButton->setChecked(false);
    _ui->widthSpin->setValue(0);
    _ui->styleCombo->setCurrentIndex(1);
    _ui->sizeSpin->setValue(0);
    _ui->symbolCombo->setCurrentIndex(1);
    _ui->filterInfCheck->setChecked(false);
    _ui->filterNaNCheck->setChecked(false);
  }

  const bool enable = curve != nullptr;
  _ui->sourceNameEdit->setEnabled(enable);
  _ui->fullNameEdit->setEnabled(enable);
  _ui->curveNameEdit->setEnabled(enable);
  _ui->timeColumnEdit->setEnabled(enable);
  _ui->timeScaleEdit->setEnabled(enable);
  _ui->timeBaseEdit->setEnabled(enable);
  _ui->firstTimeEdit->setEnabled(enable);
  _ui->colourWidget->setEnabled(enable);
  _ui->restoreColourButton->setEnabled(enable);
  _ui->widthSpin->setEnabled(enable);
  _ui->styleCombo->setEnabled(enable);
  _ui->sizeSpin->setEnabled(enable);
  _ui->symbolCombo->setEnabled(enable);
  _ui->filterInfCheck->setEnabled(enable);
  _ui->filterNaNCheck->setEnabled(enable);
}


bool CurveProperties::eventFilter(QObject *target, QEvent *event)
{
  if (target == _ui->colourWidget && _curve)
  {
    if (event->type() == QEvent::MouseButtonPress)
    {
      QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
      if (mouseEvent->button() == Qt::LeftButton)
      {
        QColorDialog dialog(QColor(_curve->colour()));
        if (dialog.exec() == QDialog::Accepted)
        {
          _curve->setColour(dialog.selectedColor().rgb(), true);
          _ui->restoreColourButton->setChecked(true);
          setColour(_ui->colourWidget, dialog.selectedColor());
          invalidateCurve(*_curve);
        }
      }
      return true;
    }
  }

  return QWidget::eventFilter(target, event);
}


void CurveProperties::invalidateCurve(PlotInstance &curve)
{
  _suppressEvents = true;
  _curves->invalidate(&curve);
  _suppressEvents = false;
}


void CurveProperties::invalidateSource(PlotSource &source)
{
  _suppressEvents = true;
  _curves->invalidate(&source, true);
  _suppressEvents = false;
}


void CurveProperties::setColour(QWidget *widget, const QColor &colour)
{
  if (QLineEdit *edit = qobject_cast<QLineEdit *>(widget))
  {
    edit->setStyleSheet(
      QString("QLineEdit { background: rgb(%1, %2, %3); selection-background-color: rgb(%1, %2, %3); }")
      .arg(colour.red()).arg(colour.green()).arg(colour.blue()));
    return;
  }

  QPalette pal(widget->palette());

  // set black background
  pal.setColor(QPalette::Background, colour);
  widget->setAutoFillBackground(true);
  widget->setPalette(pal);
}
