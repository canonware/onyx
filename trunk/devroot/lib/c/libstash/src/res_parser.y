/* -*-mode:c-*- */
%{
/* -*-mode:c-*-
 ****************************************************************************
 *
 * Copyright (c) 1996-1998
 * Jason Evans <jasone@canonware.com>.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer as
 *    the first lines of this file unmodified.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY JASON EVANS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL JASON EVANS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * Current revision: $Revision: 6 $
 * Last modified: $Date: 1998-01-16 00:20:41 -0800 (Fri, 16 Jan 1998) $
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
%token BACKSLASHCONT
%token LT GT OP CP
%token RES_INST RES_CLASS

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
