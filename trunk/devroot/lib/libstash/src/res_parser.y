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

/* To keep the compiler quiet. */
int yyerror(char * arg_error_str);
int yylex();

#include <inc_common.h>

%}

%union {
  long integer;
  char * string;
  res_val_t * resource_value;
}

/* %pure_parser */

%token <integer> LINE_NUM
%token <string> RESOURCE_NAME PP_TOKEN PP_VAR PP_MACRO_ARG FILENAME
%token <resource_value> RESOURCE_VALUE

/* %token STAR COMMA */
%token PERIOD QUESTION COLON BACKSLASH CR
%token DEFINE UNDEF INCLUDE IF IFDEF IFNDEF ELSE ENDIF LINE ELIF DEFINED
%token POUND POUNDPOUND PRAGMA ERROR
%token BACKSLASHCONT
/* %token LT GT OP CP */
%token RES_INST RES_CLASS

%left STAR
%right COMMA
%left LT GT
%right OP
%left CP

%%
 /* Unnecessary rule */
res_file: stmt_list
        ;

stmt_list: pp_stmt
         | res_stmt
         | pp_stmt stmt_list
         | res_stmt stmt_list
         ;

pp_stmt: pp_def
       | UNDEF PP_VAR
       | INCLUDE LT FILENAME GT
       | pp_def stmt_list ENDIF
       | pp_def stmt_list pp_else ENDIF
       | LINE LINE_NUM
       ;

pp_def: DEFINE PP_VAR
      | DEFINE pp_macro pp_macro_body
      ;

pp_macro: OP pp_macro_arg_list CP
        ;

pp_macro_arg_list : PP_MACRO_ARG
{
  lprintf("%s\n", $1);
}
                  | PP_MACRO_ARG COMMA pp_macro_arg_list
{
  lprintf("%s\n", $1);
}
		  ;

pp_macro_body: PP_TOKEN CR
{
  lprintf("%s\n", $1);
}
             | PP_TOKEN pp_macro_body
{
  lprintf("%s\n", $1);
}
             | BACKSLASHCONT pp_macro_body
             ;

pp_expr:
       ;

pp_def: IF pp_expr
      | IFDEF PP_VAR
      | IFNDEF PP_VAR
      ;

pp_else: ELSE stmt_list
       | ELIF pp_expr stmt_list
       | ELIF pp_expr stmt_list pp_else
       ;

res_stmt:
        ;

res_name:
        ;

res_val:
       ;

rn_s1:
     ;

rn_s2:
     ;

rn_s3:
     ;

rn_s4:
     ;

rn_s5:
     ;

%%
int
yyerror(char * arg_error_str)
{
  leprintf(__FILE__, __LINE__, "yyerror", "%s\n", arg_error_str);
  return 0;
}
