/* -*-mode:c-*- */
%{
/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * Current revision: $Revision: 86 $
 * Last modified: $Date: 1998-06-23 17:40:29 -0700 (Tue, 23 Jun 1998) $
 *
 * Description: 
 *              
 *              
 *              
 *              
 ****************************************************************************
 */

#define _INC_STRING_H_
#include <inc_common.h>
#include <res_parser.tab.h>
  
extern unsigned g_line_num;
extern YYSTYPE yylval;
%}

%%

 /* Suck up whitespace. */
[ \t]+         ;

\n             {
                 g_line_num++;
               }

"*" {return STAR;}
"." {return PERIOD;}
"?" {return QUESTION;}
":" {return COLON;}
"\\" {return BACKSLASH;}
"define" {return DEFINE;}
"undef" {return UNDEF;}
"include" {return INCLUDE;}
"if" {return IF;}
"ifdef" {return IFDEF;}
"ifndef" {return IFNDEF;}
"else" {return ELSE;}
"endif" {return ENDIF;}
"line" {return LINE;}
"elif" {return ELIF;}
"defined" {return DEFINED;}
"#" {return POUND;}
"##" {return POUNDPOUND;}
"pragma" {return PRAGMA;}
"error" {return ERROR;}
\\[ \t]*\n {return BACKSLASHCONT;}
"<" {return LT;}
">" {return GT;}
"(" {return OP;}
")" {return CP;}

 /* ??? Should _ be allowed in instances and classes? */

 /* Resource instance. */
[a-z][A-Za-z0-9_]* {
                     yylval.string = (char *) malloc(strlen(yytext) + 1);
		     _cw_check_ptr(yylval.string);
                     strcpy(yylval.string, yytext);
		     return RES_INST;
                   }

 /* Resource class. */
[A-Z][A-Za-z0-9_]* {
                     yylval.string = (char *) malloc(strlen(yytext) + 1);
		     _cw_check_ptr(yylval.string);
                     strcpy(yylval.string, yytext);
		     return RES_CLASS;
                   }

 /* filename */
[A-Za-z0-9][ ] {} /* ??? Need to look this up. */
 /* ??? Does this cause a possible confict with the resource specifiers? */

 /* Catch-all */
. {ECHO; leprintf(__FILE__, __LINE__, NULL,
		  "Possible error: catch-all hit in flex grammar.\n");
  }

%%
