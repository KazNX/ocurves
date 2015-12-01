//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2013
//
#ifndef __EXPRESSIONVSIEW_H_
#define __EXPRESSIONVSIEW_H_

#include "ocurvesconfig.h"

#include <QWidget>

#include <QList>
#include <QVector>

class Expressions;
class FunctionDefinition;
class PlotExpression;
class QListWidgetItem;
class QTableWidget;

namespace Ui
{
  class ExpressionsView;
}

/// @ingroup ui
/// Manages and edits current expressions.
///
/// The view directly edits an @c Expressions object, modifying its content.
///
/// This view is designed to be used in a docking window.
class ExpressionsView : public QWidget
{
  Q_OBJECT

public:
  /// Constructor.
  /// @param expressions Details the @c Expressions to view and edit.
  /// @param parent The parent widget.
  explicit ExpressionsView(Expressions *expressions = nullptr, QWidget *parent = nullptr);

  /// Destructor.
  ~ExpressionsView();

  /// Sets the @c Expressions object to view and edit.
  ///
  /// The UI is updated to reflect the content of @c Expressions.
  /// @param expressions The object to view and edit. May be null.
  void setExpressions(Expressions *expressions);

  /// Returns the @c Expressions object being viewed and edited.
  const Expressions *expressions() const;

protected:
  /// Parses the text expression @c text returning a compiled @c PlotExpression.
  ///
  /// Parsing is performed by a @c PlotExpressionParser using the
  /// @c PlotFunctionRegister contained in the current @c Expressions object.
  /// The compiled expression is returned on success. On failure, the @c errors
  /// list is populated with the parsing issues encountered.
  ///
  /// If there is no current @c Expressions object, then the function returns null
  /// with nothing added to @c errors.
  ///
  /// @param text The expression string to parse.
  /// @param[in,out] errors Populated with any parsing errors encountered.
  /// @return The compiled expression on success, null on failure.
  PlotExpression *parseExpression(const QString &text, QStringList &errors) const;

  /// Attempts to add an expression based on the current UI state.
  ///
  /// The expression text is taken from the UI editing component.
  ///
  /// Note that this does not update the UI expression list directly,
  /// relying on signals from the @c Expressions object to do so.
  /// Parse errors are displayed in the appropriate UI component.
  ///
  /// @return A valid expression object on successes, null on failure.
  PlotExpression *addExpression();

protected slots:
  /// Handler for add button presses.
  ///
  /// Maps through to @c addExpression().
  void addExpr();

  /// Removes the currently selected expression (if any).
  void removeExpr();

  /// Attempts to update the currently selected expression with the current UI
  /// editor text.
  void updateExpr();

  /// Handles changes in the current expression selection, updating the editor text.
  /// @param current The current/new selection.
  /// @param previous The previous/old selection.
  void currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

  /// Handles explicit completion of editing in the expression editor (i.e., the enter key).
  ///
  /// This attempts to either update the current expression, or create a new one.
  void editingDone();

  /// Handles additions to the @c Expressions object, adding the expression to the display list.
  /// @param expression The new expression object.
  void expressionAdded(PlotExpression *expression);

  /// Handles removal from the @c Expressions object, removing the corresponding expression from the display list.
  /// @param expression The removed expression object.
  void expressionRemoved(const PlotExpression *expression);

protected:
  /// Clears and repopulates the expressions display list.
  ///
  /// Uses @c createItem() for each display entry.
  void showExpressions();

  /// Create a list widget for @p expression.
  /// @param expression The expression of interest.
  /// @return A display widget for @p expression.
  QListWidgetItem *createItem(PlotExpression *expression);

  /// Remove the expression associated with @p item.
  /// @param item The list item to remove the expression for.
  /// @return True on success, false if there is no associated expression.
  bool removeExpression(QListWidgetItem *item);

private:
  /// Creates a function display table for the functions tab widget.
  ///
  /// The functions tab widget contains a tab per function category, each containing a
  /// table of functions in that category. This function creates and populates the table
  /// widget to display in a tab.
  ///
  /// @param functions The list of functions in this category to display in the created
  ///     table.
  /// @param parent The parent widget for the table.
  /// @return The table populated with @c functions.
  QTableWidget *createFunctionsTable(const QVector<const FunctionDefinition *> &functions, QWidget *parent = nullptr);

  Ui::ExpressionsView *_ui;    ///< The UI components.
  Expressions *_expressions;  ///< The expressions model. May be null, but the UI is not useful without it.
  bool _suppressEvents;       ///< Used to ignore UI events (true) during certain code operations.
};


inline const Expressions *ExpressionsView::expressions() const
{
  return _expressions;
}

#endif // __EXPRESSIONSVIEW_H_
