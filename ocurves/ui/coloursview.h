//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2013
//
#ifndef COLOURSVIEW_H
#define COLOURSVIEW_H

#include "ocurvesconfig.h"

#include <QDialog>

#include <QVector>
#include <QRgb>

namespace Ui
{
  class ColoursView;
}

class QMenu;
class QTableWidgetItem;

/// @ingroup ui
/// Dialog used to edit the colours used to display plots.
///
/// Displays colours in a table with a hexadecimal cell and a cell filled with the
/// final colour. Colours can be added and removed, but there must always be at
/// least one colour.
///
/// After editing, the final colour list is available via @c colours().
class ColoursView : public QDialog
{
  Q_OBJECT

public:
  /// Column index definitions.
  enum Columns
  {
    ColHex,
    ColColour
  };

  /// Constructor.
  /// @param parent Parent widget.
  explicit ColoursView(QWidget *parent = nullptr);

  /// Destructor.
  ~ColoursView();

  /// Sets the list of colours to edit.
  /// @param colours The initial list of available colours.
  inline void setColours(const QVector<QRgb> &colours) { _colours = colours; }

  /// Retrieves the list of colours. This is the edited list after the dialog is closed.
  /// @return The edited colour list.
  inline const QVector<QRgb> &colours() const { return _colours; }

public slots:
  /// Opens a colour dialog to edit the colour at the given index. Does nothing if @p colourIndex is out of range.
  ///
  /// This opens a colour selection dialog initialised with the colour at
  /// @p colourIndex. The colour is unmodified if the editing dialog is cancelled.
  /// @param colourIndex The index of the colour to edit. Ignores out of range values.
  void editColour(int colourIndex);

protected slots:
  /// Adds a new colour entry to the colour list, opening a colour selection dialog.
  ///
  /// Bound to the add button.
  void addItem();

  /// Removes the currently selected colour from the list.
  ///
  /// Bound to the remove button.
  void removeItem();

  /// Reset to the default colour set.
  ///
  /// The default colours are taken from @c ocurves::DefaultColours.
  void resetColours();

  /// Handler for actions on the reset context menu.
  ///
  /// Resets to the colour set associated with the triggering action.
  void resetActionTriggered();

  /// Remaps selection of @c ColColour to the corresponding @c ColHex.
  ///
  /// This method ensures that the @c ColColour cannot remain selected by
  /// deselecting cells in this column, and selecting the corresponding @c ColHex
  /// cell. This is simply to prevent drawing the colour column with the wrong
  /// colour.
  ///
  /// For example, on Windows platforms the selected cell is always drawn with a
  /// system colour (commonly blue). This would result in the colour cell showing
  /// the wrong colour (blue) instead of the colour it represents. We avoid this
  /// by remapping the selection.
  void cellSelectionChange();

  /// Handles editing of entries in the colour table (hexadecimal column only).
  ///
  /// Only accepts editing of the hexadecimal value column, ignoring any others.
  ///
  /// Bound to the table's @c itemChanged() signal.
  ///
  /// @param item The table item being edited.
  void edited(QTableWidgetItem *item);

  /// Respond to cell double clicks on colours by showing the colour dialog.
  ///
  /// Only responds to clicks on the colour column. Bound to the table's
  /// @c cellDoubleClicked() signal.
  ///
  /// @param row The row index of the clicked cell.
  /// @param column The column index of the clicked cell.
  void cellDoubleClick(int row, int column);

protected:
  /// Handles the dialog show event by populating the colours table.
  /// @param event Event details.
  virtual void showEvent(QShowEvent *event);

  /// Populate the colours UI control from @c _colours.
  void populateColourTable();

private:
  Ui::ColoursView *_ui;   ///< User interface
  QVector<QRgb> _colours; ///< Colours to edit.
  QMenu *_resetSelectionMenu; ///< Menu shown to reset to a default colour set.
  bool _ignoreChanges;    ///< Used to ignore events (true) during certain code manipulations.
};

#endif // COLOURSVIEW_H
