//
// author Kazys Stepanas
//
// Copyright (c) 2012
//
#include "plotexpressionparser.h"

#include "plotbracketexpression.h"
#include "plotconstant.h"
#include "plotexpression.h"
#include "plotexpressionarithmetic.h"
#include "plotfunction.h"
#include "plotfunctionregister.h"
#include "plotindexexpression.h"
#include "plotsample.h"

#include "ocurvesparser.hpp"
#include "plotparseprivate.h"

#include "qwt_series_data.h"

#include <iostream>

#include <QStringList>

#include <algorithm>
#include <cmath>
#include <sstream>

int ocurvesparse(ParseState *state);

void ocurveserror(ParseState *state, const QString &message)
{
  state->errors.push_back(message);
}

class PlotParserImp
{
public:
  PlotParserImp(const PlotFunctionRegister *functionRegister);
  ~PlotParserImp();

  inline const PlotFunctionRegister &functionRegister() const { return *reg; }

private:
  const PlotFunctionRegister *reg;
  bool ownRegister;
};


PlotExpressionParser::PlotExpressionParser(const PlotFunctionRegister *functionRegister)
  : _parser(new PlotParserImp(functionRegister))
{

}


PlotExpressionParser::~PlotExpressionParser()
{
  delete _parser;
}



const PlotFunctionRegister &PlotExpressionParser::functionRegister() const
{
  return _parser->functionRegister();
}


PlotExpression *PlotExpressionParser::parse(const QString &expression, QStringList &errors)
{
  bool parsed = false;
  std::string stdExpr = qPrintable(expression);

  ParseState state;
  state.expression = nullptr;
  state.funcs = &_parser->functionRegister();
  // create a scan buffer
  YY_BUFFER_STATE buffer = ocurves_scan_string(stdExpr.c_str());
  int err = 0;
  if (buffer)
  {
    err = ocurvesparse(&state);
    ocurves_delete_buffer(buffer);
  }

  if (err != 0 || !state.errors.empty())
  {
    errors = state.errors;
    delete state.expression;
    state.expression = nullptr;
  }

//  if (state.expression)
//  {
//    std::cout << state.expression->toString() << std::endl;
//  }

  errors << QString("Error : parse error. Failed to parse expression");

  return state.expression;
}


// At end of the file because it messes with auto formatting. Who knows why?
PlotParserImp::PlotParserImp(const PlotFunctionRegister *functionRegister)
  : reg(functionRegister)
  , ownRegister(false)
{
  if (!reg)
  {
    reg = new PlotFunctionRegister();
    ownRegister = true;
  }
}


PlotParserImp::~PlotParserImp()
{
  if (ownRegister)
  {
    delete reg;
  }
}
