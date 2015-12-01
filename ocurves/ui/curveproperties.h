//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef CURVEPROPERTIES_H_
#define CURVEPROPERTIES_H_

#include "ocurvesconfig.h"

#include <QWidget>

namespace Ui
{
  class CurveProperties;
}

class Curves;
class PlotInstance;
class PlotSource;
class QIntValidator;

/// @ingroup ui
/// UI for editing @c PlotSource and @c PlotInstance properties.
class CurveProperties : public QWidget
{
  Q_OBJECT
public:
  /// Constructor.
  /// @param curves The curves model.
  /// @param parent Owning widget.
  CurveProperties(Curves *curves, QWidget *parent = nullptr);

  /// Destructor.
  ~CurveProperties();

  /// Access the curve being viewed and edited.
  /// @return The current curve.
  inline PlotInstance *curve() { return _curve; }

  /// Access the curve being viewed and edited.
  /// @return The current curve.
  inline const PlotInstance *curve() const { return _curve; }

public slots:
  /// Sets the active curve being edited. May be null.
  /// @param curve The curve to edit.
  void setActiveCurve(PlotInstance *curve);

  /// Bound to @c Curves::curveDataChanged(). Updates the UI.
  /// @param curve The changed curve.
  void curveChanged(const PlotInstance *curve);

  /// Bound to @c Curves::sourceDataChanged(). Updates the UI.
  /// @param source The changed curve source.
  void sourceChanged(const PlotSource *source);

signals:
  /// Emitted when the active curve changed.
  /// @param curve The new curve being edited.
  void activeCurveChanged(PlotInstance *curve);

private slots:
  /// UI event handler.
  void timeColumnChanged();
  /// UI event handler.
  void timeScaleChanged();
  /// UI event handler.
  void timeBaseChanged();
  /// UI event handler.
  void toggleExplicitColour(bool checked);
  /// UI event handler.
  void widthChanged(int width);
  /// UI event handler.
  void styleChanged(int index);
  /// UI event handler.
  void symbolSizeChanged(int width);
  /// UI event handler.
  void symbolChanged(int index);
  /// UI event handler.
  void filterInfChecked(bool checked);
  /// UI event handler.
  void filterNaNChecked(bool checked);
  /// UI event handler.
  void resetCurveProperties();

private:
  /// Update UI reflect @p curve. Handles null.
  /// @param curve The curve to display.
  void update(PlotInstance *curve);
  /// Filters event for colour editing.
  bool eventFilter(QObject *target, QEvent *event) override;

  /// Invalidate @p curve via @c Curves. Events suppressed during signal.
  /// @param curve The curve to invalidate.
  void invalidateCurve(PlotInstance &curve);

  /// Invalidate @p source via @c Curves. Events suppressed during signal.
  /// @param source The source to invalidate.
  void invalidateSource(PlotSource &source);

  /// Set the background colour of @p widget.
  /// @param widget The widget to set the background colour on.
  /// @param colour The colour to set.
  static void setColour(QWidget *widget, const QColor &colour);

  Ui::CurveProperties *_ui;
  Curves *_curves;
  PlotInstance *_curve;
  QIntValidator *_timeColumnValidator;
  bool _suppressEvents;
};

#endif // CURVEPROPERTIES_H_
