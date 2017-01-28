%{
#include <sstream>
%}

%code requires {
#include "VopFile.hpp"
#include "Location.hpp"
#include <vector>
#include <string>

using namespace sdo;

#define YYSTYPE VOP_STYPE

typedef std::string VOP_STYPE;

	
#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

void Voperror(YYLTYPE *locp, VopFile &vopFile, yyscan_t scanner, const std::string &fileName, const char *msg);
extern int Voplex(YYSTYPE * lvalp, YYLTYPE * llocp, yyscan_t scanner);

}

%define api.pure
%locations
%parse-param { VopFile &vopFile }
%param { yyscan_t scanner }
%parse-param { const std::string &fileName }
%initial-action { new (&($$)) std::string(); }

%token VOP_MODEL 
%token VOP_OBJECTIVE 
%token VOP_CONTROL
%token VOP_BOUNDS 
%token VOP_CATEGORY 
%token VOP_STRING 
%token VOP_SEP 

%%

options:
    options option
    | option
    | error
    ;

option:
    VOP_MODEL VOP_STRING {
        vopFile.setModelFile($2);
    }
    | VOP_OBJECTIVE VOP_STRING {
    	vopFile.setObjectiveFile($2);
    }
    | VOP_CONTROL VOP_STRING {
    	vopFile.setControlFile($2);
    }
    | VOP_BOUNDS VOP_STRING {
    	vopFile.setBoundFile($2);
    }
    
    | VOP_CATEGORY categories
    ;
categories:
    categories VOP_SEP VOP_STRING {
    	vopFile.addCategory($3);

    }
    | VOP_STRING {
    	vopFile.addCategory($1);
    }
%%

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void Voperror(YYLTYPE *locp, VopFile &vopFile, yyscan_t scanner, const std::string &fileName, const char *msg) {
  vopFile.error(fileName, *locp, msg);
}
#pragma GCC diagnostic pop
