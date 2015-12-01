%{
#include "plotsconfig.h"

#include <cstdio>
#include <iostream>
#include <vector>

#include "functiondefinition.h"
#include "plotexpression.h"
#include "plotbracketexpression.h"
#include "plotconstant.h"
#include "plotexpressionarithmetic.h"
#include "plotfunction.h"
#include "plotfunctionregister.h"
#include "plotindexexpression.h"
#include "plotsample.h"
#include "plotslice.h"

#include "plotparseprivate.h"

#include <algorithm>

#include <QTextStream>
#include <QString>
#include <QVector>

class PlotFunctionRegister;

void deleteArgsList(ArgsList *argsList);

%}

/*** yacc/bison Declarations ***/

/* Require bison 2.3 or later */
%require "2.3"

/* start symbol is named "expression" */
%start start

/* write out a header file containing the token defines */
%defines

/* use newer C++ skeleton file */
/* %skeleton "lalr1.cc" */

/* namespace to enclose parser in */
%name-prefix="ocurves"

/* variables. */
%parse-param { ParseState *state }

%pure-parser

%union {
  int ival;
  float fval;
  QString *sval;
  struct PlotSampleId *idval;
  PlotExpression *expval;
  QVector<PlotExpression*> *argsval;
};

%token END 0 "end of file"
%token <ival> INTEGER "integer"
%token <fval> DOUBLE  "double"
%token <sval> IDENTIFIER "identifier"
%token <sval> IDENTIFIER_SQ "identifier_sq"
%token <sval> IDENTIFIER_DQ "identifier_dq"
%token <sval> REGX_SQ "regx_sq"
%token <sval> REGX_DQ "regx_dq"

%type <expval>  start
%type <expval>  expression
%type <expval>  product power
%type <expval>  sample term simpleTerm
%type <expval>  func
%type <argsval> arg_list
%type <idval>   sample_identifier

%destructor { delete $$; } IDENTIFIER IDENTIFIER_SQ IDENTIFIER_DQ
%destructor { delete $$; } REGX_SQ REGX_DQ
%destructor { delete $$; } sample_identifier
%destructor { delete $$; } expression
%destructor { delete $$; } product power
%destructor { delete $$; } sample term
%destructor { delete $$; } func
%destructor { deleteArgsList($$); } arg_list

%{
int ocurveslex(YYSTYPE * yytype);
%}

%%

start:                        { state->expression = nullptr; }
  | expression                { state->expression = $1; }
  ;

expression:
    product                   { $$ = $1; }
  | expression '+' product    { $$ = new PlotAdd($1, $3); }
  | expression '-' product    { $$ = new PlotSubtract($1, $3); }
  ;

product:
    power                     { $$ = $1; }
    | product '/' power       { $$ = new PlotDivide($1, $3); }
    | product '*' power       { $$ = new PlotMultiply($1, $3); }
  ;

power:
    term                      { $$ = $1; }
  | term '^' power            { $$ = new PlotPower($1, $3); }
  ;

arg_list:
    expression                { $$ = new QVector<PlotExpression*>(); $$->push_back($1); }
  | arg_list ',' expression   { $$ = $1; $$->push_back($3); $1 = nullptr; }
  ;

func:
     IDENTIFIER '(' ')'       {
                                QString funcName = *$1;
                                delete $1;
                                const FunctionDefinition *func = nullptr;
                                QString errStr;
                                QTextStream err(&errStr);
                                if (!(func = state->funcs->find(funcName)))
                                {
                                  err << "Unknown function '" << funcName << "'";
                                  yyerror(state, errStr);
                                  YYABORT;
                                }
                                else
                                {
                                  if (func->argc() != 0)
                                  {
                                    err << "Too many arguments for function '" << funcName << "'. Expected " << func->argc() << ", read 0";
                                    yyerror(state, errStr);
                                    YYABORT;
                                  }
                                  else
                                  {
                                    $$ = new PlotFunction(func);
                                  }
                                }
                              }
  | IDENTIFIER '(' arg_list ')'
                              {
                                QString funcName = *$1;
                                delete $1;
                                const FunctionDefinition *func = nullptr;
                                QString errStr;
                                QTextStream err(&errStr);
                                if (!(func = state->funcs->find(funcName)))
                                {
                                  err << "Unknown function '" << funcName << "'";
                                  yyerror(state, errStr);
                                  deleteArgsList($3);
                                  $3 = nullptr;
                                  YYABORT;
                                }
                                else
                                {
                                  if (func->argc() < unsigned($3->size()) && !func->variadic())
                                  {
                                    err << "Too many arguments for function '" << funcName << "'. Expected " << func->argc() << ", read 0";
                                    yyerror(state, errStr);
                                    deleteArgsList($3);
                                    $3 = nullptr;
                                    YYABORT;
                                  }
                                  else if (func->argc() > unsigned($3->size()))
                                  {
                                    err << "Too few arguments for function '" << funcName << "'. Expected " << func->argc() << ", read 0";
                                    yyerror(state, errStr);
                                    deleteArgsList($3);
                                    $3 = nullptr;
                                    YYABORT;
                                  }
                                  else
                                  {
                                    $$ = new PlotFunction(func, *$3);
                                    delete $3;
                                    $3 = nullptr;
                                  }
                                }
                              }
  ;

sample_identifier:
    IDENTIFIER                { $$ = new PlotSampleId(*$1); delete $1; $1 = nullptr; }
  | IDENTIFIER_SQ             { $$ = new PlotSampleId(*$1, '\''); delete $1; $1 = nullptr; }
  | IDENTIFIER_DQ             { $$ = new PlotSampleId(*$1, '"'); delete $1; $1 = nullptr; }
  | REGX_SQ                   { $$ = new PlotSampleId(*$1, '\'', true); delete $1; $1 = nullptr; }
  | REGX_DQ                   { $$ = new PlotSampleId(*$1, '"', true); delete $1; $1 = nullptr; }

sample:
    sample_identifier '|' sample_identifier {
      $$ = new PlotSample(*$3, *$1);
      delete $1;
      delete $3;
      $1 = $3 = nullptr;
    }
  | sample_identifier         { $$ = new PlotSample(*$1); delete $1; $1 = nullptr; }
  ;

term:
    simpleTerm '[' expression ':' ']'
                              { $$ = new PlotSlice($1, $3, nullptr); }
  | simpleTerm '[' ':' expression ']'
                              { $$ = new PlotSlice($1, nullptr, $4); }
  | simpleTerm '[' expression ':' expression ']'
                              { $$ = new PlotSlice($1, $3, $5); }
  | simpleTerm '[' expression ']'
                              { $$ = new PlotIndexExpression($1, $3); }
  | '-' simpleTerm            { $$ = new PlotNegate($2); }
  | '+' simpleTerm            { $$ = $2; }
  | simpleTerm                { $$ = $1; }

simpleTerm:
    func                      { $$ = $1; }
  | sample                    { $$ = $1; }
  | DOUBLE                    { $$ = new PlotConstant($1); }
  | INTEGER                   { $$ = new PlotConstant(double($1)); }
  | '(' expression ')'        { $$ = new PlotBracketExpression($2); }
  ;

%%

void deleteArgsList(ArgsList *argsList)
{
  if (argsList)
  {
    for (ArgsList::iterator iter = argsList->begin(); iter != argsList->end(); ++iter)
    {
      delete *iter;
    }
    delete argsList;
  }
}
