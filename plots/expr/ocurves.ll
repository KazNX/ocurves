%{
#include "plotsconfig.h"

#include <cstdio>
#include <QString>
#include "plotexpression.h"
#include "ocurvesparser.hpp"

#define YY_NO_UNISTD_H

#ifdef __APPLE__
// Avoid deprecation warning.
#define register
#endif // __APPLE__

%}

/*** Flex Declarations and Options ***/

/* enable c++ scanner class generation */
/* %option c++ */

/* change the name of the scanner class. results in "OCurvesFlexLexer" */
%option prefix="ocurves"

/* no support for include files is planned */
%option yywrap nounput

/* enables the use of start condition stacks */
/* %option stack */

%option bison-bridge

%option never-interactive

%% /*** Regular Expressions Part ***/

[ \t]+                  ;
-?[0-9]+\.[0-9]*([Ee]-?[0-9]+)?   { yylval->fval = atof(yytext); return DOUBLE; }
-?[0-9]*\.[0-9]+([Ee]-?[0-9]+)?   { yylval->fval = atof(yytext); return DOUBLE; }
-?[0-9]+[Ee]-?[0-9]+    { yylval->fval = atof(yytext); return DOUBLE; }
-?[0-9]+                { yylval->ival = atoi(yytext); return INTEGER; }
[a-zA-Z_][a-zA-Z0-9_]*  { yylval->sval = new QString(yytext); return IDENTIFIER; }
[rR]'(([^\'])|(\\\'))+'    {
  yylval->sval = new QString(yytext + 2);
  yylval->sval->remove(yylval->sval->length() - 1, 1);
  return REGX_SQ;
}
[rR]\"(([^\"])|(\\\"))+\"    {
  yylval->sval = new QString(yytext + 2);
  yylval->sval->remove(yylval->sval->length() - 1, 1);
  return REGX_DQ;
}
\'(([^\'])|(\\\'))+' {
  yylval->sval = new QString(yytext + 1);
  yylval->sval->remove(yylval->sval->length() - 1, 1);
  return IDENTIFIER_SQ;
}
\"(([^\"])|(\\\"))+\" {
  yylval->sval = new QString(yytext + 1);
  yylval->sval->remove(yylval->sval->length() - 1, 1);
  return IDENTIFIER_DQ;
}
"^"                     return '^';
"("                     return '(';
")"                     return ')';
"{"                     return '{';
"}"                     return '}';
","                     return ',';
"+"                     return '+';
"-"                     return '-';
"*"                     return '*';
"/"                     return '/';
"["                     return '[';
"]"                     return ']';
":"                     return ':';
"|"                     return '|';
\n                      { return END; }
.                       fprintf(stderr, "Unknown token!\n"); yyterminate();

%%

int ocurveswrap()
{
  return 1;
}
