//
// author Kazys Stepanas
//
// Copyright (c) 2012
//
#ifndef __PLOTEXPRESSIONPARSER_H_
#define __PLOTEXPRESSIONPARSER_H_

#include "plotsconfig.h"

#include <QList>
#include <QString>

class QwtPointSeriesData;

class PlotInstance;
class PlotExpression;
class PlotFunctionRegister;

/// @ingroup expr
/// Parses plot expressions into a @c PlotExpression that can be bound and executed.
/// Supports single line expressions using +, -, *, /, bracketing, negation and
/// constants (double precision numbers). The operands are the names of existing
/// plot curves. Bracketing operations and precedence are also supported.
class PlotExpressionParser
{
public:
  /// Create an expression parser.
  /// @param functionRegister Optional function register to use. Uses the default
  ///   implementation if not specified.
  PlotExpressionParser(const PlotFunctionRegister *functionRegister = nullptr);

  /// Destructor.
  ~PlotExpressionParser();

  /// Returns the function register. This is either the one provided on construction,
  /// or the default implementation.
  /// @return The parser's function register.
  const PlotFunctionRegister &functionRegister() const;

  /// Attempt to parse the expression and return the operation.
  ///
  /// On failure, error messages are available in @p errors.
  ///
  /// @param expression The expression string to parser.
  /// @param errors Errors messages are added here.
  /// @return A valid @c PlotExpression on success, or null on failure.
  PlotExpression *parse(const QString &expression, QStringList &errors);

private:
  class PlotParserImp *_parser;   ///< Implementation.
};

#endif // __PLOTEXPRESSIONPARSER_H_
