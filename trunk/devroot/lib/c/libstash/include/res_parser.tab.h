typedef union {
  long integer;
  char * string;
  res_val_t * resource_value;
} YYSTYPE;
#define	LINE_NUM	258
#define	RESOURCE_NAME	259
#define	PP_TOKEN	260
#define	PP_VAR	261
#define	PP_MACRO_ARG	262
#define	FILENAME	263
#define	RESOURCE_VALUE	264
#define	PERIOD	265
#define	QUESTION	266
#define	COLON	267
#define	BACKSLASH	268
#define	CR	269
#define	DEFINE	270
#define	UNDEF	271
#define	INCLUDE	272
#define	IF	273
#define	IFDEF	274
#define	IFNDEF	275
#define	ELSE	276
#define	ENDIF	277
#define	LINE	278
#define	ELIF	279
#define	DEFINED	280
#define	POUND	281
#define	POUNDPOUND	282
#define	PRAGMA	283
#define	ERROR	284
#define	BACKSLASHCONT	285
#define	RES_INST	286
#define	RES_CLASS	287
#define	STAR	288
#define	COMMA	289
#define	LT	290
#define	GT	291
#define	OP	292
#define	CP	293


extern YYSTYPE yylval;
