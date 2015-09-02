%{
#include "MdlParser.hpp"
#include <boost/lexical_cast.hpp>
#include "ReadFile.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wsign-compare"

#define YY_USER_ACTION yylloc->first_line = yylloc->last_line = yylineno; \
    yylloc->first_column = yycolumn; yylloc->last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng;

#define YY_USER_INIT yylineno = 1; yycolumn = 1;

%}
%option noyywrap
%option never-interactive
%option noinput
%option nounput
%option reentrant
%option bison-bridge
%option bison-locations
%option yylineno


DIGIT [0-9]
INT {DIGIT}+
FLOAT {INT}(\.{DIGIT}*)?((e|E)(\+|-)?{INT})?

CHAR [a-zA-ZÄÖÜäöüß]
VARIABLE {CHAR}({CHAR}|{DIGIT})*(" "({CHAR}|{DIGIT})+)*
A_FUNC_OF [^\|]*(?i:"A FUNCTION OF")[^\|]*\|
%s COMMENTSTATE VARIABLESTATE END
%%

<INITIAL>{
    (\*)+[\ \t\r]*\n[\ \t]*"\."{CHAR}({CHAR}|{DIGIT})*[\ \t\r]*\n(\*)+  ;
    (.*)"Sketch information"(.*)\n       {BEGIN(END); }
    "\n"                                 ;
    [\t\r\\]                             ;
    (?i:"{UTF-8}")                       return MDL_UTF_8;
    [\ \t\r]*~[\ \t\r]*                  BEGIN(COMMENTSTATE);
    {A_FUNC_OF}                          ;
    (?i:"INTEG")                         return MDL_INTEG;
    (?i:"ACTIVE INITIAL")                return MDL_ACTIVE_INITIAL;
    (?i:"INITIAL")                	 return MDL_INITIAL;
    (?i:"SMOOTHI")                       return MDL_SMOOTHI;
    (?i:"SMOOTH")                        return MDL_SMOOTH;
    (?i:"SMOOTH3I")                      return MDL_SMOOTH3I;
    (?i:"SMOOTH3")                       return MDL_SMOOTH3;
    (?i:"DELAY1I")                       return MDL_DELAY1I;
    (?i:"DELAY1")                        return MDL_DELAY1;
    (?i:"DELAY3I")                       return MDL_DELAY3I;
    (?i:"DELAY3")                        return MDL_DELAY3;
    (?i:"DELAYP")                        return MDL_DELAYP;
    (?i:"IF THEN ELSE")                  return MDL_IFSTMT;
    (?i:"WITH LOOKUP")                   return MDL_WLOOKUP;
    (?i:"PULSE TRAIN")                   return MDL_PULSETRAIN;
    (?i:"PULSE")                         return MDL_PULSE;
    (?i:"ABS")                           return MDL_ABS;
    (?i:"SIN")                           return MDL_SIN;
    (?i:"COS")                           return MDL_COS;
    (?i:"TAN")                           return MDL_TAN;
    (?i:"TANH")                          return MDL_TANH;
    (?i:"SINH")                          return MDL_SINH;
    (?i:"COSH")                          return MDL_COSH;
    (?i:"ARCTAN")                        return MDL_ARCTAN;
    (?i:"ARCSIN")                        return MDL_ARCSIN;
    (?i:"ARCCOS")                        return MDL_ARCCOS;
    (?i:"EXP")                           return MDL_EXP;
    (?i:"LN")                            return MDL_LN;
    (?i:"LOG")                           return MDL_LOG;
    (?i:"MIN")                           return MDL_MIN;
    (?i:"MAX")                           return MDL_MAX;
    (?i:"POWER")                         return MDL_POWER;
    (?i:"MODULO")                        return MDL_MODULO;
    (?i:"INTEGER")                       return MDL_INTEGER;
    (?i:"SQRT")                          return MDL_SQRT;
    (?i:"RAMP")                          return MDL_RAMP;
    (?i:"RANDOM UNIFORM")                return MDL_RANDOM_UNIFORM;
    (?i:"DELAY FIXED")                   return MDL_DELAY_FIXED;
    (?i:"STEP")                          return MDL_STEP;
    (?i:"TIME")                          return MDL_TIME;
    (?i:"FINAL TIME")                    return MDL_FINAL_TIME;
    (?i:"INITIAL TIME")                  return MDL_INITIAL_TIME;
    (?i:"TIME STEP")                     return MDL_TIME_STEP;
    (?i:"SAVEPER")                       return MDL_SAVEPER;
    (?i:":AND:")                         return MDL_AND;
    (?i:":OR:")                          return MDL_OR;
    (?i:":NOT:")                         return MDL_NOT;
    "("                                  return MDL_OPENPARA;
    ")"                                  return MDL_CLOSEPARA;
    "["                                  return MDL_SQUAREOPEN;
    "]"                                  return MDL_SQUARECLOSE;
    "*"                                  return MDL_MULT;
    "/"                                  return MDL_DIV;
    "+"                                  return MDL_PLUS;
    "-"                                  return MDL_MINUS;
    ","                                  return MDL_SEP;
    ":"				         return MDL_COLON;
    ">="                                 return MDL_OP_GE;
    "<="                                 return MDL_OP_LE;
    "<>"                                 return MDL_OP_NEQ;
    ">"                                  return MDL_OP_G;
    "<"                                  return MDL_OP_L;
    "="                                  return MDL_OP_EQ;
    "^"                                  return MDL_INFIX_POW;
    {FLOAT}                              {*yylval = boost::lexical_cast<double>(yytext); return MDL_NUMBER;}
    {VARIABLE}                           {*yylval = sdo::Symbol(yytext); return MDL_VARIABLE;}
    " "                                  ;
    "\""                                 BEGIN(VARIABLESTATE);
}

<VARIABLESTATE>{
    [^"\n\""]+                           {*yylval = sdo::Symbol(yytext); return MDL_VARIABLE;}
    "\""                                 BEGIN(INITIAL);
}

<COMMENTSTATE>{
    "|"                                  { BEGIN(INITIAL); return MDL_COMMENTEND;}
    [\ \t\r]*~[\ \t\r]*                  ;
    [^\|~\n]*[^\|~\ \n\t\r]              { *yylval = sdo::Symbol(yytext); return MDL_COMMENT;}
    [\ \t\r\n]*                          ;
}

<END>.|\n       ;
%%
#pragma GCC diagnostic pop
namespace sdo {

  void parse_mdl_file(const std::string &file, ExpressionGraph &exprGraph) {
    auto content = sdo::read_file(file);
    if(content.empty())
      return;
    yyscan_t scanner;
    YY_BUFFER_STATE buf;
    Mdllex_init(&scanner);
    buf = Mdl_scan_string(&content[0], scanner);
    Mdlparse(exprGraph, scanner, file);
    Mdl_delete_buffer(buf, scanner);
    Mdllex_destroy(scanner);
    if(exprGraph.hasErrors())
      throw parse_error(exprGraph);
  }

}
