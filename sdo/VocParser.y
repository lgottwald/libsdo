%{
#include <sstream>
%}

%code requires {
#include "ExpressionGraph.hpp"
#include "Location.hpp"
#include <boost/variant.hpp>

using boost::get;
using namespace sdo;

#define YYSTYPE VOC_STYPE

using NodePtr = ExpressionGraph::Node*;

using VOC_STYPE = boost::variant<Symbol, NodePtr, double>;


#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

void Vocerror (YYLTYPE *locp, sdo::ExpressionGraph &exprGraph, yyscan_t scanner, const std::string &fileName, const char *msg);
extern int Voclex(YYSTYPE * lvalp, YYLTYPE * llocp, yyscan_t scanner);
}

%define api.pure
%locations
%parse-param { sdo::ExpressionGraph &exprGraph }
%param { yyscan_t scanner }
%parse-param { const std::string &fileName }

%token VOC_CONTROL
%token VOC_PIECEWISE_CONTROL
%token VOC_INTEGER_VARIABLE
%token VOC_DT
%token VOC_NUMBER
%token VOC_LE
%token VOC_EQ
%token VOC_VARIABLE

%%

options:
    options option
    | option
    | error
    ;

option:
    VOC_INTEGER_VARIABLE VOC_VARIABLE {
      NodePtr var = exprGraph.getNode(get<Symbol>($2));
      if(var->op == ExpressionGraph::NIL)
	  	exprGraph.error(fileName, yylloc, std::string("Expected variable ") + get<Symbol>($2).get() + std::string(" to be already defined") );
      var->integer = true;
    }
    | VOC_CONTROL VOC_VARIABLE {
      NodePtr ctrl = exprGraph.getNode(get<Symbol>($2));
      if(ctrl->op == ExpressionGraph::NIL) {
	ctrl = exprGraph.getNode(ExpressionGraph::CONTROL, nullptr, nullptr, nullptr);
	exprGraph.addSymbol(get<Symbol>($2), ctrl);
      }
      ctrl->control_size = 1;
    }
    | VOC_PIECEWISE_CONTROL VOC_VARIABLE VOC_EQ VOC_NUMBER {
      NodePtr ctrl = exprGraph.getNode(get<Symbol>($2));
      if(ctrl->op == ExpressionGraph::NIL) {
	ctrl = exprGraph.getNode(ExpressionGraph::CONTROL, nullptr, nullptr, nullptr);
	exprGraph.addSymbol(get<Symbol>($2), ctrl);
      }
      ctrl->control_size = static_cast<int>(get<double>($4));
    }
    | VOC_DT VOC_NUMBER {
      exprGraph.addSymbol(Symbol("TIME STEP"), exprGraph.getNode(get<double>($2)));
    }
    | bound_lo VOC_VARIABLE bound_l bound_up {
      NodePtr lo = get<NodePtr>($1);
      NodePtr l = get<NodePtr>($3);
      NodePtr up = get<NodePtr>($4);
      if(!lo)
	exprGraph.error(fileName, yylloc, "Expected lower bound for control");
      if(!up)
	exprGraph.error(fileName, yylloc, "Expected upper bound for control");
      NodePtr ctrl = exprGraph.getNode(get<Symbol>($2));
      if(ctrl->op == ExpressionGraph::NIL) {
	ctrl = exprGraph.getNode(ExpressionGraph::CONTROL, lo, l, up);
	ctrl->control_size = 0;
	exprGraph.addSymbol(get<Symbol>($2), ctrl);
      } else {
	if(ctrl->child1 && lo)
	  exprGraph.error(fileName, yylloc, "Lower bound of control already set");
	else if(lo)
	  ctrl->child1 = lo;

	if(ctrl->child2 && l)
	  exprGraph.error(fileName, yylloc, "Level of control already set");
	else if(l)
	  ctrl->child2 = l;

	if(ctrl->child3 && up)
	  exprGraph.error(fileName, yylloc, "Upper bound of control already set");
	else if(up)
	  ctrl->child3 = up;
      }
    }
    ;

bound_lo:
	number VOC_LE {
	  $$ = $1;
	}
	| /* epsilon */ {
	  $$ = NodePtr(nullptr);
	}
	;

bound_up:
	VOC_LE number {
	  $$ = $2;
	}
	| /* epsilon */ {
	  $$ = NodePtr(nullptr);
	}
	;

bound_l:
	VOC_EQ number {
	  $$ = $2;
	}
	| /* epsilon */ {
	  $$ = NodePtr(nullptr);
	}
	;
number:
	VOC_NUMBER {
	  $$ = exprGraph.getNode(get<double>($1));
	}
	;
%%


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void Vocerror(YYLTYPE *locp, sdo::ExpressionGraph &exprGraph, yyscan_t scanner, const std::string &fileName, const char *msg) {
  exprGraph.error(fileName, *locp, std::string(msg));
}
#pragma GCC diagnostic pop
