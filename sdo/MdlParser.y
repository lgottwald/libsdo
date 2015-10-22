%{
#include <sstream>
#include <utility>
#include "ExpressionGraph.hpp"
using namespace sdo;
using NodePtr = sdo::ExpressionGraph::Node*;
using boost::get;

NodePtr get_smooth_node(ExpressionGraph& exprGraph, NodePtr smooth_node, NodePtr input, NodePtr delay_time, NodePtr initial_value);  
NodePtr get_delay1_node(ExpressionGraph& exprGraph, NodePtr delay1_node, NodePtr input, NodePtr delay_time, NodePtr initial_value);
NodePtr get_delay3_node(ExpressionGraph& exprGraph, NodePtr delay3_node, NodePtr input, NodePtr delay_time, NodePtr initial_value);
NodePtr get_smooth3_node(ExpressionGraph& exprGraph, NodePtr smooth3_node, NodePtr input, NodePtr delay_time, NodePtr initial_value);
std::pair<NodePtr,NodePtr> get_delay_p_node(ExpressionGraph& exprGraph, NodePtr delay_p_node, NodePtr input, NodePtr delay_time);

%}

%code requires {
#include "ExpressionGraph.hpp"
#include "Location.hpp"
#include <boost/variant.hpp>
using namespace sdo;

#define YYSTYPE MDL_STYPE

using MDL_STYPE = boost::variant<ExpressionGraph::Node*, LookupTable*, Symbol, std::vector<Symbol>, double, ExpressionGraph::Operator>;

#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

void Mdlerror(YYLTYPE *locp, sdo::ExpressionGraph &exprGraph, yyscan_t scanner, const std::string &fileName, const char *msg);
int Mdllex(YYSTYPE *lvalp, YYLTYPE *llocp, yyscan_t scanner);
}

%define api.pure
%locations
%parse-param { sdo::ExpressionGraph &exprGraph }
%param { yyscan_t scanner }
%parse-param { const std::string &fileName }

%token MDL_NUMBER
%token MDL_VARIABLE
%token MDL_COMMENTEND

%token MDL_INTEG
%token MDL_SMOOTH
%token MDL_SMOOTHI
%token MDL_SMOOTH3
%token MDL_SMOOTH3I
%token MDL_DELAY1
%token MDL_DELAY1I
%token MDL_DELAY3
%token MDL_DELAY3I
%token MDL_DELAYP
%token MDL_ACTIVE_INITIAL
%token MDL_INITIAL
%token MDL_IFSTMT
%token MDL_WLOOKUP
%token MDL_PULSETRAIN
%token MDL_PULSE
%token MDL_ABS
%token MDL_SIN
%token MDL_COS
%token MDL_TAN
%token MDL_TANH
%token MDL_SINH
%token MDL_COSH
%token MDL_ARCSIN
%token MDL_ARCCOS
%token MDL_ARCTAN
%token MDL_EXP
%token MDL_LN
%token MDL_LOG
%token MDL_MIN
%token MDL_MAX
%token MDL_POWER
%token MDL_MODULO
%token MDL_INTEGER
%token MDL_SQRT
%token MDL_RAMP
%token MDL_DELAY_FIXED
%token MDL_STEP
%token MDL_TIME
%token MDL_COLON

%token MDL_SQUAREOPEN
%token MDL_SQUARECLOSE
%token MDL_SEP
%token MDL_OP_G
%token MDL_OP_GE
%token MDL_OP_L
%token MDL_OP_LE
%token MDL_OP_EQ
%token MDL_OP_NEQ
%token MDL_RANDOM_UNIFORM
%token MDL_FINAL_TIME
%token MDL_INITIAL_TIME
%token MDL_TIME_STEP
%token MDL_SAVEPER
%token MDL_UTF_8

%nonassoc NO_COMMENT
%nonassoc MDL_COMMENT

%nonassoc MDL_WLOOKUP
%nonassoc MDL_INTEG
%nonassoc MDL_DELAY_FIXED

%left MDL_OR
%left MDL_AND
%left MDL_NOT

%left MDL_PLUS MDL_MINUS
%left MDL_MULT MDL_DIV
%left MDL_OPENPARA
%token MDL_CLOSEPARA
%left MDL_INFIX_POW UOP

%initial-action { new (&($$)) YYSTYPE(); }


%%
mdlfile:
    encoding definitions
    ;

encoding:
    MDL_UTF_8
    | /* epsilon */
    ;

definitions:
    definitions definition
    | definition
    ;

definition:
    MDL_VARIABLE MDL_OP_EQ MDL_INTEG MDL_OPENPARA expression MDL_SEP expression MDL_CLOSEPARA optional_commentblock {
      NodePtr node = exprGraph.getNode(ExpressionGraph::INTEG, get<NodePtr>($5), get<NodePtr>($7));
      node->usages.emplace_back(fileName, @3);
      exprGraph.addSymbol(get<Symbol>($1), node);
      exprGraph.addComments(get<Symbol>($1), get<std::vector<Symbol>>($9));
    }
    | MDL_VARIABLE MDL_OP_EQ MDL_SMOOTH MDL_OPENPARA expression MDL_SEP expression MDL_CLOSEPARA optional_commentblock {
      auto symbol = get<Symbol>($1);
      NodePtr input = get<NodePtr>($5);
      NodePtr smooth = get_smooth_node(exprGraph, exprGraph.getNode(symbol), input, get<NodePtr>($7), input);
      smooth->usages.emplace_back(fileName, @3);
      exprGraph.addSymbol(symbol, smooth);
      exprGraph.addComments(symbol, get<std::vector<Symbol>>($9));
    }
    | MDL_VARIABLE MDL_OP_EQ MDL_SMOOTHI MDL_OPENPARA expression MDL_SEP expression MDL_SEP expression MDL_CLOSEPARA optional_commentblock {
      auto symbol = get<Symbol>($1);
      NodePtr smooth = get_smooth_node(exprGraph, exprGraph.getNode(symbol), get<NodePtr>($5), get<NodePtr>($7), get<NodePtr>($9));
      smooth->usages.emplace_back(fileName, @3);
      exprGraph.addSymbol(symbol, smooth);
      exprGraph.addComments(symbol, get<std::vector<Symbol>>($11));
    }
    | MDL_VARIABLE MDL_OP_EQ MDL_DELAY1 MDL_OPENPARA expression MDL_SEP expression MDL_CLOSEPARA optional_commentblock {
      auto symbol = get<Symbol>($1);
      NodePtr input = get<NodePtr>($5);
      NodePtr delay = get_delay1_node(exprGraph, exprGraph.getNode(symbol), input, get<NodePtr>($7), input);
      delay->usages.emplace_back(fileName, @3);
      exprGraph.addSymbol(symbol, delay);
      exprGraph.addComments(symbol, get<std::vector<Symbol>>($9));
    }
    | MDL_VARIABLE MDL_OP_EQ MDL_DELAY1I MDL_OPENPARA expression MDL_SEP expression MDL_SEP expression MDL_CLOSEPARA optional_commentblock {
      auto symbol = get<Symbol>($1);
      NodePtr delay = get_delay1_node(exprGraph, exprGraph.getNode(symbol), get<NodePtr>($5), get<NodePtr>($7), get<NodePtr>($9));
      delay->usages.emplace_back(fileName, @3);
      exprGraph.addSymbol(symbol, delay);
      exprGraph.addComments(symbol, get<std::vector<Symbol>>($11));
    }
    | MDL_VARIABLE MDL_OP_EQ MDL_SMOOTH3 MDL_OPENPARA expression MDL_SEP expression MDL_CLOSEPARA optional_commentblock {
      auto symbol = get<Symbol>($1);
      NodePtr input = get<NodePtr>($5);
      NodePtr smooth = get_smooth3_node(exprGraph, exprGraph.getNode(symbol), input, get<NodePtr>($7), input);
      smooth->usages.emplace_back(fileName, @3);
      exprGraph.addSymbol(symbol, smooth);
      exprGraph.addComments(symbol, get<std::vector<Symbol>>($9));
    }
    | MDL_VARIABLE MDL_OP_EQ MDL_SMOOTH3I MDL_OPENPARA expression MDL_SEP expression MDL_SEP expression MDL_CLOSEPARA optional_commentblock {
      auto symbol = get<Symbol>($1);
      NodePtr smooth = get_smooth3_node(exprGraph, exprGraph.getNode(symbol), get<NodePtr>($5), get<NodePtr>($7), get<NodePtr>($9));
      smooth->usages.emplace_back(fileName, @3);
      exprGraph.addSymbol(symbol, smooth);
      exprGraph.addComments(symbol, get<std::vector<Symbol>>($11));
    }
    | MDL_VARIABLE MDL_OP_EQ MDL_DELAY3 MDL_OPENPARA expression MDL_SEP expression MDL_CLOSEPARA optional_commentblock {
      auto symbol = get<Symbol>($1);
      NodePtr input = get<NodePtr>($5);
      NodePtr delay = get_delay3_node(exprGraph, exprGraph.getNode(symbol), input, get<NodePtr>($7), input);
      delay->usages.emplace_back(fileName, @3);
      exprGraph.addSymbol(symbol, delay);
      exprGraph.addComments(symbol, get<std::vector<Symbol>>($9));
    }
    | MDL_VARIABLE MDL_OP_EQ MDL_DELAY3I MDL_OPENPARA expression MDL_SEP expression MDL_SEP expression MDL_CLOSEPARA optional_commentblock {
      auto symbol = get<Symbol>($1);
      NodePtr delay = get_delay3_node(exprGraph, exprGraph.getNode(symbol), get<NodePtr>($5), get<NodePtr>($7), get<NodePtr>($9));
      delay->usages.emplace_back(fileName, @3);
      exprGraph.addSymbol(symbol, delay);
      exprGraph.addComments(symbol, get<std::vector<Symbol>>($11));
    }
    | MDL_VARIABLE MDL_OP_EQ MDL_DELAYP MDL_OPENPARA expression MDL_SEP expression MDL_COLON MDL_VARIABLE MDL_CLOSEPARA optional_commentblock {
      auto delay_symbol = get<Symbol>($1);
      auto pipeline_symbol = get<Symbol>($9);
      auto node_pair = get_delay_p_node(exprGraph, exprGraph.getNode(delay_symbol), get<NodePtr>($5), get<NodePtr>($7));
      node_pair.first->usages.emplace_back(fileName, @3);
      node_pair.second->usages.emplace_back(fileName, @9);
      exprGraph.addSymbol(delay_symbol, node_pair.first);
      exprGraph.addSymbol(pipeline_symbol, node_pair.second);
      exprGraph.addComments(delay_symbol, get<std::vector<Symbol>>($11));
    }
    | MDL_VARIABLE MDL_OP_EQ expression optional_commentblock {
      exprGraph.addSymbol(get<Symbol>($1), get<NodePtr>($3));
      exprGraph.addComments(get<Symbol>($1), get<std::vector<Symbol>>($4));
    }
    | MDL_VARIABLE MDL_OP_EQ MDL_WLOOKUP MDL_OPENPARA expression MDL_SEP lookuptable MDL_CLOSEPARA optional_commentblock {
      NodePtr node = exprGraph.getNode(ExpressionGraph::APPLY_LOOKUP, get<NodePtr>($7), get<NodePtr>($5));
      node->usages.emplace_back(fileName, @3);
      exprGraph.addSymbol(get<Symbol>($1), node);
      exprGraph.addComments(get<Symbol>($1), get<std::vector<Symbol>>($9));
    }
    | MDL_VARIABLE lookuptable optional_commentblock {
      exprGraph.addSymbol(get<Symbol>($1), get<NodePtr>($2));
      exprGraph.addComments(get<Symbol>($1), get<std::vector<Symbol>>($3));
    }
    | MDL_FINAL_TIME MDL_OP_EQ expression optional_commentblock {
      Symbol symbol("FINAL TIME");
      exprGraph.addSymbol(Symbol("FINAL TIME"), get<NodePtr>($3));
      exprGraph.addComments(symbol, get<std::vector<Symbol>>($4));
    }
    | MDL_INITIAL_TIME MDL_OP_EQ expression optional_commentblock  {
      Symbol symbol("INITIAL TIME");
      exprGraph.addSymbol(symbol, get<NodePtr>($3));
      exprGraph.addComments(symbol, get<std::vector<Symbol>>($4));
    }
    | MDL_TIME_STEP MDL_OP_EQ expression optional_commentblock {
      Symbol symbol("TIME STEP");
      exprGraph.addSymbol(symbol, get<NodePtr>($3));
      exprGraph.addComments(symbol, get<std::vector<Symbol>>($4));
    }
    | MDL_SAVEPER MDL_OP_EQ expression optional_commentblock
    | MDL_VARIABLE MDL_OP_EQ error {
      std::string msg{"Skipping definition of '"};
      msg += get<Symbol>($1).get();
      msg += "' due to previous errors";
      exprGraph.error(fileName, @$, std::move(msg));
    }
    | error
    | commentblock
    ;

commentblock:
    comments MDL_COMMENTEND {
	$$ = std::move($1);
    }

optional_commentblock:
    comments MDL_COMMENTEND {
	$$ = std::move($1);
    }
    | /* epsilon */ %prec NO_COMMENT {
	$$ = std::vector<Symbol>();
    }
    | MDL_COMMENTEND {
	$$ = std::vector<Symbol>();
    }
    ;

comments:
    comments MDL_COMMENT {
        $$ = std::move($1);
    	get<std::vector<Symbol>>($$).emplace_back( std::move(get<Symbol>($2)));
    }
    | MDL_COMMENT {
        $$ = std::vector<Symbol>{ std::move(get<Symbol>($1)) };
    }
    ;

expression:
    MDL_IFSTMT MDL_OPENPARA logical_expression MDL_SEP expression MDL_SEP expression MDL_CLOSEPARA {
      auto cond =  get<NodePtr>($3);
      auto thenval =  get<NodePtr>($5);
      auto elseval =  get<NodePtr>($7);
      if(cond->child1 == thenval && cond->child2 == elseval)
      {
         if(cond->op == ExpressionGraph::G ||
            cond->op == ExpressionGraph::GE )
         {
            $$ = exprGraph.getNode( ExpressionGraph::MAX, thenval, elseval );
         }
         else if( cond->op == ExpressionGraph::L ||
                  cond->op == ExpressionGraph::LE )
         {
            $$ = exprGraph.getNode( ExpressionGraph::MIN, thenval, elseval );
         }
         else
         {
            $$ = exprGraph.getNode( ExpressionGraph::IF, cond, thenval, elseval );
         }
      }
      else if (cond->child2 == thenval && cond->child1 == elseval)
      {
         if(cond->op == ExpressionGraph::G ||
            cond->op == ExpressionGraph::GE )
         {
            $$ = exprGraph.getNode( ExpressionGraph::MIN, thenval, elseval );
         }
         else if( cond->op == ExpressionGraph::L ||
                  cond->op == ExpressionGraph::LE )
         {
            $$ = exprGraph.getNode( ExpressionGraph::MAX, thenval, elseval );
         }
         else
         {
            $$ = exprGraph.getNode( ExpressionGraph::IF, cond, thenval, elseval );
         }
      }
      else
      {
         $$ = exprGraph.getNode( ExpressionGraph::IF, cond, thenval, elseval );
      }
      get<NodePtr>($$)->usages.emplace_back(fileName, @$);
    }
    | expression MDL_PLUS expression {
      $$ = exprGraph.getNode(ExpressionGraph::PLUS, get<NodePtr>($1), get<NodePtr>($3));
      get<NodePtr>($$)->usages.emplace_back(fileName, @$);
    }
    | expression MDL_MINUS expression {
      $$ = exprGraph.getNode(ExpressionGraph::MINUS, get<NodePtr>($1), get<NodePtr>($3));
      get<NodePtr>($$)->usages.emplace_back(fileName, @$);
    }
    | expression MDL_MULT expression {
      $$ = exprGraph.getNode(ExpressionGraph::MULT, get<NodePtr>($1), get<NodePtr>($3));
      get<NodePtr>($$)->usages.emplace_back(fileName, @$);
    }
    | expression MDL_DIV expression {
      $$ = exprGraph.getNode(ExpressionGraph::DIV, get<NodePtr>($1), get<NodePtr>($3));
      get<NodePtr>($$)->usages.emplace_back(fileName, @$);
    }
    | expression MDL_INFIX_POW expression {
      $$ = exprGraph.getNode(ExpressionGraph::POWER, get<NodePtr>($1), get<NodePtr>($3));
      get<NodePtr>($$)->usages.emplace_back(fileName, @$);
    }
    | MDL_MINUS expression %prec UOP {
      $$ = exprGraph.getNode( ExpressionGraph::UMINUS, get<NodePtr>($2) );
      get<NodePtr>($$)->usages.emplace_back(fileName, @$);
    }
    | MDL_PLUS expression %prec UOP {
      $$ = get<NodePtr>($2);
    }
    | MDL_OPENPARA expression MDL_CLOSEPARA {
      $$ = get<NodePtr>($2);
    }
    | unary_prefix_op MDL_OPENPARA expression MDL_CLOSEPARA {
      $$ = exprGraph.getNode( get<ExpressionGraph::Operator>($1), get<NodePtr>($3) );
      get<NodePtr>($$)->usages.emplace_back(fileName, @$);
    }
    | binary_prefix_op MDL_OPENPARA expression MDL_SEP expression MDL_CLOSEPARA {
      $$ = exprGraph.getNode( get<ExpressionGraph::Operator>($1), get<NodePtr>($3), get<NodePtr>($5));
      get<NodePtr>($$)->usages.emplace_back(fileName, @$);
    }
    | ternary_prefix_op MDL_OPENPARA expression MDL_SEP expression MDL_SEP expression MDL_CLOSEPARA %prec UOP {
      $$ = exprGraph.getNode( get<ExpressionGraph::Operator>($1), get<NodePtr>($3), get<NodePtr>($5), get<NodePtr>($7));
      get<NodePtr>($$)->usages.emplace_back(fileName, @$);
    }
    | MDL_PULSETRAIN MDL_OPENPARA expression MDL_SEP expression MDL_SEP expression MDL_SEP expression MDL_CLOSEPARA %prec UOP {
      NodePtr pulse = exprGraph.getNode(ExpressionGraph::PULSE, get<NodePtr>($3), get<NodePtr>($5) );
      $$ = exprGraph.getNode(ExpressionGraph::PULSE_TRAIN, pulse, get<NodePtr>($7), get<NodePtr>($9));
      get<NodePtr>($$)->usages.emplace_back(fileName, @$);
    }
    | MDL_TIME {
      $$ = exprGraph.getTimeNode();
      get<NodePtr>($$)->usages.emplace_back(fileName, @$);
    }
    | MDL_VARIABLE {
      $$ = exprGraph.getNode( get<Symbol>($1) );
      get<NodePtr>($$)->usages.emplace_back(fileName, @$);
    }
    | MDL_FINAL_TIME {
      $$ = exprGraph.getNode(Symbol("FINAL TIME"));
      get<NodePtr>($$)->usages.emplace_back(fileName, @$);
    }
    | MDL_TIME_STEP {
      $$ = exprGraph.getNode(Symbol("TIME STEP"));
      get<NodePtr>($$)->usages.emplace_back(fileName, @$);
    }
    | MDL_INITIAL_TIME {
      $$ = exprGraph.getNode(Symbol("INITIAL TIME"));
      get<NodePtr>($$)->usages.emplace_back(fileName, @$);
    }
    | MDL_VARIABLE MDL_OPENPARA expression MDL_CLOSEPARA {
      NodePtr lkp_table = exprGraph.getNode(get<Symbol>($1));
      lkp_table->usages.emplace_back(fileName, @$);
      $$ = exprGraph.getNode( ExpressionGraph::APPLY_LOOKUP, lkp_table, get<NodePtr>($3));
      get<NodePtr>($$)->usages.emplace_back(fileName, @$);
    }
    | MDL_NUMBER {
        $$ = exprGraph.getNode(get<double>($1));
    }
    ;

unary_prefix_op:
    MDL_SIN { $$ = ExpressionGraph::SIN; } |
    MDL_COS { $$ = ExpressionGraph::COS; } |
    MDL_TAN { $$ = ExpressionGraph::TAN; } |
    MDL_SINH { $$ = ExpressionGraph::SINH; } |
    MDL_COSH { $$ = ExpressionGraph::COSH; } |
    MDL_TANH { $$ = ExpressionGraph::TANH; } |
    MDL_ARCCOS { $$ = ExpressionGraph::ARCCOS; } |
    MDL_ARCSIN { $$ = ExpressionGraph::ARCSIN; } |
    MDL_ARCTAN { $$ = ExpressionGraph::ARCTAN; } |
    MDL_LN { $$ = ExpressionGraph::LN; } |
    MDL_ABS { $$ = ExpressionGraph::ABS; } |
    MDL_SQRT { $$ = ExpressionGraph::SQRT; } |
    MDL_EXP { $$ = ExpressionGraph::EXP; } |
    MDL_INTEGER { $$ = ExpressionGraph::INTEGER; } |
    MDL_INITIAL { $$ = ExpressionGraph::INITIAL; }
    ;

binary_prefix_op:
    MDL_ACTIVE_INITIAL { $$ = ExpressionGraph::ACTIVE_INITIAL; } |
    MDL_MIN { $$ = ExpressionGraph::MIN; } |
    MDL_MAX { $$ = ExpressionGraph::MAX; } |
    MDL_LOG { $$ = ExpressionGraph::LOG; } |
    MDL_MODULO { $$ = ExpressionGraph::MODULO; } |
    MDL_POWER { $$ = ExpressionGraph::POWER; } |
    MDL_PULSE { $$ = ExpressionGraph::PULSE; } |
    MDL_STEP { $$ = ExpressionGraph::STEP; }
    ;

ternary_prefix_op:
    MDL_RAMP { $$ = ExpressionGraph::RAMP; } |
    MDL_DELAY_FIXED { $$ = ExpressionGraph::DELAY_FIXED; } |
    MDL_RANDOM_UNIFORM { $$ = ExpressionGraph::RANDOM_UNIFORM; }
    ;

rel_op:
    MDL_OP_G { $$ = ExpressionGraph::G; } |
    MDL_OP_GE { $$ = ExpressionGraph::GE; } |
    MDL_OP_L { $$ = ExpressionGraph::L; } |
    MDL_OP_LE { $$ = ExpressionGraph::LE; } |
    MDL_OP_NEQ { $$ = ExpressionGraph::NEQ; } |
    MDL_OP_EQ { $$ = ExpressionGraph::EQ; }
    ;

number:
    MDL_NUMBER {
      $$ = $1;
    }
    | MDL_MINUS MDL_NUMBER {
      $$ = -get<double>($2);
    }
    ;

lookuptable:
    MDL_OPENPARA lookupinterval MDL_SEP lookuppoints MDL_CLOSEPARA {
      $$ = exprGraph.getNode(get<LookupTable*>($4));
      get<NodePtr>($$)->usages.emplace_back(fileName, @$);
    }
    ;

lookuppoints:
    MDL_OPENPARA number MDL_SEP number MDL_CLOSEPARA {
      //create lookup table and add first point
      LookupTable* lkp = exprGraph.createLookupTable();
      lkp->addPoint(get<double>($2), get<double>($4));
      $$ = lkp;
    }
    | lookuppoints MDL_SEP MDL_OPENPARA number MDL_SEP number MDL_CLOSEPARA {
        // append next point
        LookupTable* lkp = get<LookupTable*>($1);
	    lkp->addPoint(get<double>($4), get<double>($6));
    	$$ = lkp;
    }
    ;

lookupinterval:
    MDL_SQUAREOPEN MDL_OPENPARA number MDL_SEP number MDL_CLOSEPARA MDL_MINUS MDL_OPENPARA number MDL_SEP number MDL_CLOSEPARA MDL_SQUARECLOSE
    ;

logical_expression:
    expression rel_op expression {
      $$ = exprGraph.getNode( get<ExpressionGraph::Operator>($2), get<NodePtr>($1), get<NodePtr>($3));
      get<NodePtr>($$)->usages.emplace_back(fileName, @$);
    }
    | logical_expression MDL_AND logical_expression {
      $$ = exprGraph.getNode( ExpressionGraph::AND, get<NodePtr>($1), get<NodePtr>($3) );
      get<NodePtr>($$)->usages.emplace_back(fileName, @$);
    }
    | logical_expression MDL_OR logical_expression {
      $$ = exprGraph.getNode( ExpressionGraph::OR, get<NodePtr>($1), get<NodePtr>($3) );
      get<NodePtr>($$)->usages.emplace_back(fileName, @$);
    }
    | MDL_NOT logical_expression {
      $$ = exprGraph.getNode( ExpressionGraph::NOT, get<NodePtr>($2) );
      get<NodePtr>($$)->usages.emplace_back(fileName, @$);
    }
    | MDL_OPENPARA logical_expression MDL_CLOSEPARA { $$ = get<NodePtr>($2); }
    ;

%%

NodePtr get_smooth_node(ExpressionGraph& exprGraph, NodePtr smooth_node, NodePtr input, NodePtr delay_time, NodePtr initial_value) {
  NodePtr inp_minus_smooth = exprGraph.getNode(ExpressionGraph::MINUS, input, smooth_node);
  NodePtr rate = exprGraph.getNode(ExpressionGraph::DIV, inp_minus_smooth, delay_time);
  return exprGraph.getNode(ExpressionGraph::INTEG, rate, initial_value);
}

NodePtr get_delay1_node(ExpressionGraph& exprGraph, NodePtr delay1_node, NodePtr input, NodePtr delay_time, NodePtr initial_value) {
  NodePtr lv_rate = exprGraph.getNode(ExpressionGraph::MINUS, input, delay1_node);
  NodePtr lv_initial = exprGraph.getNode(ExpressionGraph::MULT, initial_value, delay_time);
  NodePtr lv = exprGraph.getNode(ExpressionGraph::INTEG, lv_rate, lv_initial);
  return exprGraph.getNode(ExpressionGraph::DIV, lv, delay_time);
}

NodePtr get_delay3_node(ExpressionGraph& exprGraph, NodePtr delay3_node, NodePtr input, NodePtr delay_time, NodePtr initial_value) {
  NodePtr DL = exprGraph.getNode(ExpressionGraph::DIV, delay_time, exprGraph.getNode(3.));

  NodePtr lv1_tmp = exprGraph.createTmpNode();
  NodePtr lv2_tmp = exprGraph.createTmpNode();
  NodePtr lv3_tmp = exprGraph.createTmpNode();

  NodePtr rt1 = exprGraph.getNode(ExpressionGraph::DIV, lv1_tmp, DL);
  NodePtr rt2 = exprGraph.getNode(ExpressionGraph::DIV, lv2_tmp, DL);

  NodePtr lv1_rate = exprGraph.getNode(ExpressionGraph::MINUS, input, rt1);
  NodePtr lv2_rate = exprGraph.getNode(ExpressionGraph::MINUS, rt1, rt2);
  NodePtr lv3_rate =  exprGraph.getNode(ExpressionGraph::MINUS, rt2, delay3_node);

  NodePtr lv1 = exprGraph.getNode(ExpressionGraph::INTEG, lv1_rate, lv3_tmp);

  exprGraph.substituteTmpNode(lv1_tmp, lv1);

  NodePtr lv2 = exprGraph.getNode(ExpressionGraph::INTEG, lv2_rate, lv3_tmp);

  exprGraph.substituteTmpNode(lv2_tmp, lv2);

  NodePtr lv3_initial = exprGraph.getNode(ExpressionGraph::MULT, DL, initial_value);
  NodePtr lv3 = exprGraph.getNode(ExpressionGraph::INTEG, lv3_rate, lv3_initial);

  exprGraph.substituteTmpNode(lv3_tmp, lv3);

  return exprGraph.getNode(ExpressionGraph::DIV, lv3, DL);
}

NodePtr get_smooth3_node(ExpressionGraph& exprGraph, NodePtr smooth3_node, NodePtr input, NodePtr delay_time, NodePtr initial_value) {
  NodePtr tmp = exprGraph.createTmpNode();

  NodePtr DL = exprGraph.getNode(ExpressionGraph::DIV, delay_time, exprGraph.getNode(3.));
  NodePtr inp_minus_lv1 = exprGraph.getNode(ExpressionGraph::MINUS, input, tmp);
  NodePtr rate_lv1 = exprGraph.getNode(ExpressionGraph::DIV, inp_minus_lv1, DL);
  NodePtr lv1 = exprGraph.getNode(ExpressionGraph::INTEG, rate_lv1, initial_value);

  exprGraph.substituteTmpNode(tmp, lv1);
  tmp = exprGraph.createTmpNode();

  NodePtr lv1_minus_lv2 = exprGraph.getNode(ExpressionGraph::MINUS, lv1, tmp);
  NodePtr rate_lv2 = exprGraph.getNode(ExpressionGraph::DIV, lv1_minus_lv2, DL);
  NodePtr lv2 = exprGraph.getNode(ExpressionGraph::INTEG, rate_lv2, initial_value);

  exprGraph.substituteTmpNode(tmp, lv2);

  NodePtr lv2_minus_smooth = exprGraph.getNode(ExpressionGraph::MINUS, lv2, smooth3_node);
  NodePtr rate = exprGraph.getNode(ExpressionGraph::DIV, lv2_minus_smooth, DL);
  return exprGraph.getNode(ExpressionGraph::INTEG, rate, initial_value);
}

std::pair<NodePtr,NodePtr> get_delay_p_node(ExpressionGraph& exprGraph, NodePtr delay_p_node, NodePtr input, NodePtr delay_time) {
  NodePtr DL = exprGraph.getNode(ExpressionGraph::DIV, delay_time, exprGraph.getNode(3.));
  
  NodePtr lv1_tmp = exprGraph.createTmpNode();
  NodePtr lv2_tmp = exprGraph.createTmpNode();
  NodePtr lv3_tmp = exprGraph.createTmpNode();
  
  NodePtr rt1 = exprGraph.getNode(ExpressionGraph::DIV, lv1_tmp, DL);
  NodePtr rt2 = exprGraph.getNode(ExpressionGraph::DIV, lv2_tmp, DL);
  
  NodePtr lv1_rate = exprGraph.getNode(ExpressionGraph::MINUS, input, rt1);
  NodePtr lv2_rate = exprGraph.getNode(ExpressionGraph::MINUS, rt1, rt2);
  NodePtr lv3_rate =  exprGraph.getNode(ExpressionGraph::MINUS, rt2, delay_p_node);
  
  NodePtr lv1 = exprGraph.getNode(ExpressionGraph::INTEG, lv1_rate, lv3_tmp);
  
  exprGraph.substituteTmpNode(lv1_tmp, lv1);
  
  NodePtr lv2 = exprGraph.getNode(ExpressionGraph::INTEG, lv2_rate, lv3_tmp);
  
  exprGraph.substituteTmpNode(lv2_tmp, lv2);
  
  NodePtr lv3_initial = exprGraph.getNode(ExpressionGraph::MULT, DL, input);
  NodePtr lv3 = exprGraph.getNode(ExpressionGraph::INTEG, lv3_rate, lv3_initial);
  
  exprGraph.substituteTmpNode(lv3_tmp, lv3);
  NodePtr lv3_plus_lv2 = exprGraph.getNode(ExpressionGraph::PLUS, lv3, lv2);
  return std::make_pair(
    exprGraph.getNode(ExpressionGraph::DIV, lv3, DL),
    exprGraph.getNode(ExpressionGraph::PLUS, lv3_plus_lv2, lv1));
  
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void Mdlerror (YYLTYPE *locp, sdo::ExpressionGraph &exprGraph, yyscan_t scanner, const std::string &fileName, const char *msg) {
  exprGraph.error(fileName, *locp, msg);
}
#pragma GCC diagnostic pop
