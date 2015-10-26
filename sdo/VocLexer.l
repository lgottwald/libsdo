%{
#include "VocParser.hpp"
#include "ReadFile.hpp"
#include <boost/lexical_cast.hpp>

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
%s VARIABLESTATE
%%
<INITIAL>{
  ":control"       				return VOC_CONTROL;
  ":piecewise control"				return VOC_PIECEWISE_CONTROL;
  ":integer" 				return VOC_INTEGER_VARIABLE;  
  ":dt"						return VOC_DT;
  (\+|-)?{FLOAT}				{
						  *yylval = boost::lexical_cast<double>(yytext);
						  return VOC_NUMBER;
						}
  "<="						return VOC_LE;
  "="						return VOC_EQ;
  "\n"						;
  {VARIABLE}					{*yylval = sdo::Symbol(yytext); return VOC_VARIABLE;}
  "\""						BEGIN(VARIABLESTATE);
  .						;
}

<VARIABLESTATE>{
    [^"\n\""]+                   {*yylval = sdo::Symbol(yytext); return VOC_VARIABLE;}
    "\""                         BEGIN(INITIAL);
}
%%
#pragma GCC diagnostic pop
namespace sdo {

  void parse_voc_file(const std::string &fileName, ExpressionGraph &exprGraph) {
    auto content = sdo::read_file(fileName);
    if(content.empty())
      return;
    yyscan_t scanner;
    YY_BUFFER_STATE buf;
    Voclex_init(&scanner);
    buf = Voc_scan_string(&content[0], scanner);
    Vocparse(exprGraph, scanner, fileName);
    Voc_delete_buffer(buf, scanner);
    Voclex_destroy(scanner);
    if(exprGraph.hasErrors())
      throw parse_error(exprGraph);
  }

}