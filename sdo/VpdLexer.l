%{
#include "VpdParser.hpp"
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
FLOAT {DIGIT}+(\.{DIGIT}+)?
INT {DIGIT}+
CHAR [a-zA-ZÄÖÜäöüß]
VARIABLE {CHAR}({CHAR}|{DIGIT})*(" "({CHAR}|{DIGIT})+)*
%s VARIABLESTATE
%%
<INITIAL>{
  (?i:":maximize")    return VPD_MAXIMIZE;
  (?i:":minimize")    return VPD_MINIMIZE;
  (?i:":Mayer")       return VPD_MAYER;
  (?i:":Lagrange")    return VPD_LAGRANGE;
  "/"                 return VPD_SLASH;
  {FLOAT}(e\+{INT})?  {*yylval = boost::lexical_cast<double>(yytext); return VPD_NUMBER;}
  {VARIABLE}          {*yylval = sdo::Symbol(yytext); return VPD_VARIABLE;}
  "\n"                ;
  "\""                BEGIN(VARIABLESTATE);
  .                   ;
}

<VARIABLESTATE>{
    [^"\n\""]+        {*yylval = sdo::Symbol(yytext); return VPD_VARIABLE;}
    "\""              BEGIN(INITIAL);
}
%%
#pragma GCC diagnostic pop
namespace sdo {

void parse_vpd_file(const std::string &fileName, Objective &obj) {
  auto content = sdo::read_file(fileName);
  if(content.empty())
    return;
  yyscan_t scanner;
  YY_BUFFER_STATE buf;
  Vpdlex_init(&scanner);
  buf = Vpd_scan_string(&content[0], scanner);
  Vpdparse(obj, scanner, fileName);
  Vpd_delete_buffer(buf, scanner);
  Vpdlex_destroy(scanner);
  if(obj.hasErrors())
      throw parse_error(obj);
}

}