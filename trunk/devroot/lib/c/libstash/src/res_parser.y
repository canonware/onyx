/* -*-mode:c-*- */
%{
/* -*-mode:c-*-
 ****************************************************************************
 *
 * Copyright (C) 1996-1997 Jason Evans <jasone@canonware.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You can get a copy of the GNU General Public License at
 * http://www.fsf.org/copyleft/gpl.html, or by writing to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * Current revision: $Revision: 4 $
 * Last modified: $Date: 1997-12-14 22:01:41 -0800 (Sun, 14 Dec 1997) $
 *
 * Description: 
 *              
 *              
 *              
 *              
 ****************************************************************************
 */

#include <inc_common.h>

%}

%union
{
  long * integer;
  char * string;
  res_val_t * resource_value;
}

%token <integer> LINE_NUM
%token <string> RESOURCE_NAME PP_TOKEN PP_VAR FILENAME
%token <resource_value> RESOURCE_VALUE

%token STAR PERIOD QUESTION COLON BACKSLASH
%token DEFINE UNDEF INCLUDE IF IFDEF IFNDEF ELSE ENDIF LINE ELIF DEFINED
%token POUND POUNDPOUND PRAGMA ERROR
%token LT GT OP CP

%%
 /* Unnecessary rule */
res_file: stmt_list

stmt_list: pp_stmt
         | res_stmt
         | pp_stmt stmt_list
         | res_stmt stmt_list

pp_stmt: pp_def
       | UNDEF PP_VAR
       | INCLUDE LT FILENAME GT
       | pp_def stmt_list ENDIF
       | pp_def stmt_list pp_else ENDIF
       | LINE LINE_NUM

pp_def: DEFINE PP_VAR
      | DEFINE pp_macro pp_macro_body

pp_macro: OP pp_macro_arg_list CP

pp_macro_arg_list : PP_MACRO_ARG
                  | PP_MACRO_ARG COMMA pp_macro_arg_list

pp_macro_body: PP_TOKEN CR
             | PP_TOKEN pp_macro_body
             | BACKSLASH CR pp_macro_body

pp_expr:

pp_def: IF pp_expr
      | IFDEF PP_VAR
      | IFNDEF PP_VAR

pp_else:  ELSE stmt_list
        | ELIF pp_expr stmt_list
        | ELIF pp_expr stmt_list pp_else

res_stmt:
res_name:
res_val:
rn_s1:
rn_s2:
rn_s3:
rn_s4:
rn_s5:
