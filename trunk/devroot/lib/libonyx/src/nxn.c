/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#define	_NXN_C_

#include "../include/libonyx/libonyx.h"

/* Alphabetical order. */
const cw_uint8_t *cw_g_nx_names[] = {
	"",
	"!#",
	"#!",
	"(",
	")",
	"<",
	">",
	"[",
	"]",
	"abs",
	"active",
	"add",
	"and",
	"argv",
	"array",
	"arraytype",
	"atime",
	"begin",
	"bind",
	"blksize",
	"blocks",
	"booleantype",
#ifdef _CW_THREADS
	"broadcast",
#endif
	"bytesavailable",
	"catenate",
	"cd",
	"chmod",
	"chown",
	"clear",
	"cleardstack",
	"cleartomark",
	"close",
	"collect",
#ifdef _CW_THREADS
	"condition",
	"conditiontype",
#endif
	"copy",
	"count",
	"countdstack",
	"countestack",
	"counttomark",
	"ctime",
	"currentdict",
#ifdef _CW_THREADS
	"currentlocking",
#endif
	"cve",
	"cvlit",
	"cvn",
	"cvrs",
	"cvs",
	"cvx",
	"def",
#ifdef _CW_THREADS
	"detach",
#endif
	"dev",
	"dict",
	"dicttype",
	"die",
	"dirforeach",
	"div",
	"dstack",
	"dstackunderflow",
	"dup",
	"echeck",
	"egid",
	"end",
	"envdict",
	"eq",
	"errorname",
	"estack",
	"estackoverflow",
	"euid",
	"eval",
	"exch",
	"exec",
	"exit",
	"exp",
	"false",
	"filetype",
	"finotype",
	"flush",
	"flushfile",
	"for",
	"foreach",
	"fork",
	"gcdict",
	"ge",
	"get",
	"getinterval",
	"gid",
	"globaldict",
	"gt",
	"hooktag",
	"hooktype",
	"if",
	"ifelse",
	"index",
	"ino",
	"integertype",
	"invalidaccess",
	"invalidexit",
	"invalidfileaccess",
	"ioerror",
	"istack",
#ifdef _CW_THREADS
	"join",
#endif
	"known",
#ifdef _CW_THREADS
	"lcheck",
#endif
	"le",
	"length",
	"limitcheck",
	"link",
	"load",
#ifdef _CW_THREADS
	"lock",
#endif
	"loop",
	"lt",
	"mark",
	"marktype",
	"mkdir",
	"mod",
	"mode",
#ifdef _CW_THREADS
	"monitor",
#endif
	"mtime",
	"mul",
#ifdef _CW_THREADS
	"mutex",
	"mutextype",
#endif
	"nametype",
	"ndup",
	"ne",
	"neg",
	"newerror",
	"nlink",
	"not",
	"npop",
	"nsleep",
	"null",
	"nulltype",
	"open",
	"operatortype",
	"or",
	"ostack",
#ifdef _CW_THREADS
	"period",
#endif
	"pid",
	"pmark",
	"pmarktype",
	"pop",
	"ppid",
	"print",
	"product",
	"promptstring",
	"put",
	"putinterval",
	"pwd",
	"quit",
	"rand",
	"rangecheck",
	"rdev",
	"read",
	"readline",
	"realtime",
	"rename",
	"repeat",
	"rmdir",
	"roll",
	"sclear",
	"scleartomark",
	"scount",
	"scounttomark",
	"sdup",
	"seek",
#ifdef _CW_THREADS
	"self",
#endif
	"setactive",
	"setegid",
	"setenv",
	"seteuid",
	"setgid",
#ifdef _CW_THREADS
	"setlocking",
	"setperiod",
#endif
	"setthreshold",
	"setuid",
	"sexch",
	"shift",
#ifdef _CW_THREADS
	"signal",
#endif
	"sindex",
	"size",
	"spop",
	"spush",
	"srand",
	"sroll",
	"stack",
	"stacktype",
	"stackunderflow",
	"start",
	"stats",
	"status",
	"stdin",
	"stderr",
	"stdout",
	"stop",
	"stopped",
	"string",
	"stringtype",
	"sub",
	"symlink",
	"syntaxerror",
	"system",
	"systemdict",
	"tell",
	"test",
#ifdef _CW_THREADS
	"thread",
	"threadtype",
#endif
	"threshold",
	"throw",
#ifdef _CW_THREADS
	"timedwait",
#endif
	"token",
	"true",
	"truncate",
#ifdef _CW_THREADS
	"trylock",
#endif
	"type",
	"typecheck",
	"uid",
	"undef",
	"undefined",
	"undefinedfilename",
	"undefinedresult",
	"unlink",
#ifdef _CW_THREADS
	"unlock",
#endif
	"unmatchedfino",
	"unmatchedmark",
	"unregistered",
	"unsetenv",
	"version",
#ifdef _CW_THREADS
	"wait",
#endif
	"waitpid",
	"where",
	"write",
	"xcheck",
	"xor"
#ifdef _CW_THREADS
	,
	"yield"
#endif
};
