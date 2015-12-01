//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2012
//
#ifndef __LOADPROGRESS_
#define __LOADPROGRESS_

#include "ocurvesconfig.h"

#include <QWidget>

namespace Ui
{
  class LoadProgress;
}

/// @ingroup ui
/// A widget for showing the progress of loading.
///
/// Contains:
/// - A cancel button.
/// - A label showing to describe the current operation.
/// - An overall progress bar.
/// - A progress bar for the current item.
///
/// The current item progress range
class LoadProgress : public QWidget
{
  Q_OBJECT

public:
  enum
  {
    /// Maximum value for the current progress item.
    ///
    /// Considered to be 100% done when the current progress is at this value.
    CurrentItemMax = 1000
  };

  /// Constructor.
  /// @param parent The parent for this widget.
  explicit LoadProgress(QWidget *parent = nullptr);
  /// Destructor.
  ~LoadProgress();

signals:
  /// Raised when the cancel button is pressed.
  void cancel();

public slots:
  /// Sets the name of the current item or operation.
  /// @param name The item name to display.
  void itemName(const QString &name);

  /// Sets the current item progress [0, @c CurrentItemMax].
  /// @param current The current progress, expected in the range [0, @c CurrentItemMax].
  void itemProgress(int current);

  /// Updates the overall item progress bar [0, @c total].
  ///
  /// @param current The current progress in the range [0, @c total].
  /// @param total The total target progress.
  void overallProgress(int current, int total);

  /// To be invoked on completion. This closes and deletes this dialog.
  /// @param overallCount The overall number of items loaded. Ignored.
  void loadComplete(int overallCount);

private slots:
  /// Handles the cancel button by raising the @c cancel() signal.
  void onCancel();

private:
  Ui::LoadProgress *ui; ///< UI widgets.
};

#endif // __LOADPROGRESS_
