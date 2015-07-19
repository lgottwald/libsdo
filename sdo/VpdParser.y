%{
#include <sstream>
%}

%code requires {
#include "Objective.hpp"
#include "Location.hpp"
#include <boost/variant.hpp>

using boost::get;
using namespace sdo;

#define YYSTYPE VPD_STYPE
using VPD_STYPE = boost::variant<Symbol, double>;

#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
	typedef void* yyscan_t;
#endif

void Vpderror(YYLTYPE *locp, Objective &objective, yyscan_t scanner, const std::string &fileName, const char *msg);
extern int Vpdlex(YYSTYPE * lvalp, YYLTYPE * llocp, yyscan_t scanner);


}

%define api.pure
%locations
%parse-param { Objective &objective }
%param { yyscan_t scanner }
%parse-param { const std::string &fileName }


%token VPD_MAXIMIZE
%token VPD_MINIMIZE
%token VPD_MAYER
%token VPD_LAGRANGE
%token VPD_SLASH
%token VPD_NUMBER
%token VPD_VARIABLE

%%

options:
	options option
	| option
	| error
	;

option:
	VPD_MAXIMIZE {
		objective.setMaximized(true);
	}
	| VPD_MINIMIZE {
		objective.setMinimized(true);
	}
	| VPD_LAGRANGE lagrange_summands
	| VPD_MAYER mayer_summands
	;

lagrange_summands:
	lagrange_summands VPD_VARIABLE VPD_SLASH VPD_NUMBER {
		objective.addSummand(Objective::Summand::LAGRANGE,
			get<Symbol>($2), get<double>($4));

	}
	| VPD_VARIABLE VPD_SLASH VPD_NUMBER {
		objective.addSummand(Objective::Summand::LAGRANGE,
			get<Symbol>($1), get<double>($3));
	}
	;

mayer_summands:
	mayer_summands VPD_VARIABLE VPD_SLASH VPD_NUMBER {
		objective.addSummand(Objective::Summand::MAYER,
			get<Symbol>($2), get<double>($4));

	}
	| VPD_VARIABLE VPD_SLASH VPD_NUMBER {
		objective.addSummand(Objective::Summand::MAYER,
			get<Symbol>($1), get<double>($3));
	}
	;

%%

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void Vpderror(YYLTYPE *locp, Objective &objective, yyscan_t scanner, const std::string &fileName, const char *msg) {
  objective.error(fileName, *locp, msg);
}
#pragma GCC diagnostic pop
