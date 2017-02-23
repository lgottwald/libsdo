%{
#include "VopParser.hpp"
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
%s QUOTED
%%
<INITIAL>{
	":model"         return VOP_MODEL;
	":objective"     return VOP_OBJECTIVE;
	":control"       return VOP_CONTROL;
	":bounds"        return VOP_BOUNDS;
	":category"      return VOP_CATEGORY;
	","              return VOP_SEP;
	"\""			 {BEGIN(QUOTED);}
	[^"\n ,\""]+	 {*yylval = yytext; return VOP_STRING;}
	"\n"             ;
	.                ;
}
<QUOTED>{
	[^"\""]*		{*yylval = yytext;  return VOP_STRING;}
	"\""			{BEGIN(INITIAL);}
}
%%
#pragma GCC diagnostic pop

namespace sdo {

VopFile parse_vop_file(const std::string &fileName) {
  VopFile vopFile;
  auto content = sdo::read_file(fileName);
  if(content.empty())
    return VopFile();
  yyscan_t scanner;
  YY_BUFFER_STATE buf;
  Voplex_init(&scanner);
  buf = Vop_scan_string(&content[0], scanner);
  Vopparse(vopFile, scanner, fileName);
  Vop_delete_buffer(buf, scanner);
  Voplex_destroy(scanner);
  if(vopFile.hasErrors())
      throw parse_error(vopFile);

  boost::filesystem::path base( fileName );
  base.remove_filename();

  if( !vopFile.getModelFile().empty() )
  {
      boost::filesystem::path mPath( base );
      mPath /= vopFile.getModelFile();
      vopFile.setModelFile(mPath.string());
  }

  if( !vopFile.getObjectiveFile().empty() )
  {
      boost::filesystem::path oPath( base );
      oPath /= vopFile.getObjectiveFile();
      vopFile.setObjectiveFile(oPath.string());
  }

  if( !vopFile.getControlFile().empty() )
  {
      boost::filesystem::path cPath( base );
      cPath /= vopFile.getControlFile();
      vopFile.setControlFile(cPath.string());
  }
  
  if( !vopFile.getBoundFile().empty() )
  {
      boost::filesystem::path cPath( base );
      cPath /= vopFile.getBoundFile();
      vopFile.setBoundFile(cPath.string());
  }
  
  return vopFile;
}

}
