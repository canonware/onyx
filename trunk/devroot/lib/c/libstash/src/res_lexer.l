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

#define _INC_STRING_H_
#include <inc_common.h>

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
		     _cw_checkptr(yylval.string);
                     strcpy(yylval.string, yytext);
		     return RES_INST;
                   }

 /* Resource class. */
[A-Z][A-Za-z0-9_]* {
                     yylval.string = (char *) malloc(strlen(yytext) + 1);
		     _cw_checkptr(yylval.string);
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
