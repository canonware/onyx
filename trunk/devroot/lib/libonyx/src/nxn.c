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
#ifdef _CW_POSIX
	"atime",
#endif
	"begin",
	"bind",
#ifdef _CW_POSIX
	"blksize",
	"blocks",
#endif
	"booleantype",
#ifdef _CW_THREADS
	"broadcast",
#endif
	"bytesavailable",
	"catenate",
#ifdef _CW_POSIX
	"cd",
	"chmod",
	"chown",
#endif
	"clear",
	"cleardstack",
	"cleartomark",
#ifdef _CW_POSIX
	"close",
#endif
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
#ifdef _CW_POSIX
	"ctime",
#endif
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
#ifdef _CW_POSIX
	"dev",
#endif
	"dict",
	"dicttype",
	"die",
#ifdef _CW_POSIX
	"dirforeach",
#endif
	"div",
	"dstack",
	"dstackunderflow",
	"dup",
	"echeck",
#ifdef _CW_POSIX
	"egid",
#endif
	"end",
#ifdef _CW_POSIX
	"envdict",
#endif
	"eq",
	"errorname",
	"estack",
	"estackoverflow",
#ifdef _CW_POSIX
	"euid",
#endif
	"eval",
	"exch",
#ifdef _CW_POSIX
	"exec",
#endif
	"exit",
	"exp",
	"false",
	"filetype",
	"finotype",
	"flush",
	"flushfile",
	"for",
	"foreach",
#ifdef _CW_POSIX
	"fork",
#endif
	"gcdict",
	"ge",
	"get",
	"getinterval",
#ifdef _CW_POSIX
	"gid",
#endif
	"globaldict",
	"gt",
	"hooktag",
	"hooktype",
	"if",
	"ifelse",
	"index",
#ifdef _CW_POSIX
	"ino",
#endif
	"integertype",
	"invalidaccess",
	"invalidexit",
	"invalidfileaccess",
	"iobuf",
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
#ifdef _CW_POSIX
	"link",
#endif
	"load",
#ifdef _CW_THREADS
	"lock",
#endif
	"loop",
	"lt",
	"mark",
	"marktype",
#ifdef _CW_POSIX
	"mkdir",
#endif
	"mod",
#ifdef _CW_POSIX
	"mode",
#endif
#ifdef HAVE_DLOPEN
	"modload",
#endif
#ifdef _CW_THREADS
	"monitor",
#endif
#ifdef _CW_POSIX
	"mtime",
#endif
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
#ifdef _CW_POSIX
	"nlink",
#endif
	"not",
	"npop",
#ifdef _CW_POSIX
	"nsleep",
#endif
	"null",
	"nulltype",
#ifdef _CW_POSIX
	"open",
#endif
	"operatortype",
	"or",
	"ostack",
#ifdef _CW_THREADS
	"period",
#endif
#ifdef _CW_POSIX
	"pid",
#endif
	"pmark",
	"pmarktype",
	"pop",
#ifdef _CW_POSIX
	"ppid",
#endif
	"print",
	"product",
	"promptstring",
	"put",
	"putinterval",
#ifdef _CW_POSIX
	"pwd",
#endif
	"quit",
	"rand",
	"rangecheck",
#ifdef _CW_POSIX
	"rdev",
#endif
	"read",
	"readline",
#ifdef _CW_POSIX
	"realtime",
	"rename",
#endif
	"repeat",
#ifdef _CW_POSIX
	"rmdir",
#endif
	"roll",
	"sclear",
	"scleartomark",
	"scount",
	"scounttomark",
	"sdup",
#ifdef _CW_POSIX
	"seek",
#endif
#ifdef _CW_THREADS
	"self",
#endif
	"setactive",
#ifdef _CW_POSIX
	"setegid",
	"setenv",
	"seteuid",
	"setgid",
#endif
	"setiobuf",
#ifdef _CW_THREADS
	"setlocking",
	"setperiod",
#endif
	"setthreshold",
#ifdef _CW_POSIX
	"setuid",
#endif
	"sexch",
	"shift",
#ifdef _CW_THREADS
	"signal",
#endif
	"sindex",
#ifdef _CW_POSIX
	"size",
#endif
	"spop",
	"spush",
#ifdef _CW_POSIX
	"srand",
#endif
	"sroll",
	"stack",
	"stacktype",
	"stackunderflow",
	"start",
	"stats",
#ifdef _CW_POSIX
	"status",
#endif
	"stdin",
	"stderr",
	"stdout",
	"stop",
	"stopped",
	"string",
	"stringtype",
	"sub",
#ifdef _CW_POSIX
	"symlink",
#endif
	"syntaxerror",
	"system",
	"systemdict",
	"tell",
#ifdef _CW_POSIX
	"test",
#endif
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
#ifdef _CW_POSIX
	"truncate",
#endif
#ifdef _CW_THREADS
	"trylock",
#endif
	"type",
	"typecheck",
#ifdef _CW_POSIX
	"uid",
#endif
	"undef",
	"undefined",
	"undefinedfilename",
	"undefinedresult",
#ifdef _CW_POSIX
	"unlink",
#endif
#ifdef _CW_THREADS
	"unlock",
#endif
	"unmatchedfino",
	"unmatchedmark",
	"unregistered",
#ifdef _CW_POSIX
	"unsetenv",
#endif
	"version",
#ifdef _CW_THREADS
	"wait",
#endif
#ifdef _CW_POSIX
	"waitpid",
#endif
	"where",
	"write",
	"xcheck",
	"xor"
#ifdef _CW_THREADS
	,
	"yield"
#endif
};
