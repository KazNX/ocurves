#ifndef OCURVESSCANNER_H_
#define OCURVESSCANNER_H_

#include "plotsconfig.h"

// fwd decl for flex
typedef struct yy_buffer_state *YY_BUFFER_STATE;
YY_BUFFER_STATE ocurves_scan_string(const char *str);
YY_BUFFER_STATE ocurves_scan_bytes(const char *bytes, int len);
void ocurves_delete_buffer(YY_BUFFER_STATE);

class PlotExpression;
class PlotFunctionRegister;

#include <QStringList>
#include <QVector>

/// @ingroup expr
/// Used as the parsing state object for Bison, essentially forming the build tree.
///
/// The state contains:
/// - The primary expression tree
/// - The function register for resolving user functions.
/// - An error list for error reporting.
struct ParseState
{
  PlotExpression *expression;           ///< The primary expression tree.
  const PlotFunctionRegister *funcs;    ///< User functions.
  QStringList errors;                   ///< Error report.
};

typedef QVector<PlotExpression *> ArgsList;

/// @ingroup expr
/// Add an error message to @p state.
/// @param state The parse state to modify.
/// @param msg The error message.
void ocurveserror(ParseState *state, const QString &msg);

#endif // OCURVESSCANNER_H_

