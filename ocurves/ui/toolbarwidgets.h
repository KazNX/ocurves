//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef TOOLBARWIDGETS_H
#define TOOLBARWIDGETS_H

#include "ocurvesconfig.h"

#include <QWidget>

class QSpinBox;
class QCheckBox;
class QLineEdit;

namespace Ui
{
  class ToolbarWidgets;
}

/// @ingroup ui
/// Widgets added to the toolbar, supporting non-button widgets.
///
/// Features no event binding, but the widgets are laid out.
class ToolbarWidgets : public QWidget
{
  Q_OBJECT
public:
  /// Constructor.
  /// @param parent The parent widget.
  explicit ToolbarWidgets(QWidget *parent = nullptr);

  /// Destructor.
  ~ToolbarWidgets();

  /// Access the spin box for setting the maximum number of samples.
  /// @return The max samples widget.
  QSpinBox *maxSamplesSpin();

  /// Access the editor for setting the time scaling.
  /// @return The time scale widget.
  QLineEdit *timeScaleEdit();

  /// Access the check box for specifying use of a time column.
  /// @return The time column option widget.
  QCheckBox *timeColumnCheck();

  /// Access the spin box for setting the time column index (1 based) number of samples.
  /// @return The time column index widget.
  QSpinBox *timeColumnSpin();

  /// Access the check box for setting relative time values.
  /// @return The relative time check widget.
  QCheckBox *relativeTimeCheck();

private:
  Ui::ToolbarWidgets *_ui;  ///< Internal UI.
};

#endif // TOOLBARWIDGETS_H
