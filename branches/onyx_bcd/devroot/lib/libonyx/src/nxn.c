/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
******************************************************************************
*
* <Copyright = jasone>
* <License>
*
******************************************************************************
*
* Version: <Version>
*
******************************************************************************/

#define	CW_NXN_C_

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
#ifdef CW_POSIX
    "atime",
#endif
    "begin",
    "bind",
#ifdef CW_POSIX
    "blksize",
    "blocks",
#endif
    "booleantype",
#ifdef CW_THREADS
    "broadcast",
#endif
    "bytesavailable",
    "catenate",
#ifdef CW_POSIX
    "cd",
    "chmod",
    "chown",
#endif
    "clear",
    "cleardstack",
    "cleartomark",
#ifdef CW_POSIX
    "close",
#endif
    "collect",
#ifdef CW_THREADS
    "condition",
    "conditiontype",
#endif
    "copy",
    "count",
    "countdstack",
    "countestack",
    "counttomark",
#ifdef CW_POSIX
    "ctime",
#endif
    "currentdict",
#ifdef CW_THREADS
    "currentlocking",
#endif
    "cve",
    "cvlit",
    "cvn",
    "cvrs",
    "cvs",
    "cvx",
    "def",
#ifdef CW_THREADS
    "detach",
#endif
#ifdef CW_POSIX
    "dev",
#endif
    "dict",
    "dicttype",
    "die",
#ifdef CW_POSIX
    "dirforeach",
#endif
    "div",
    "dstack",
    "dstackunderflow",
    "dup",
    "echeck",
#ifdef CW_POSIX
    "egid",
#endif
    "end",
#ifdef CW_POSIX
    "envdict",
#endif
    "eq",
    "errorname",
    "estack",
    "estackoverflow",
#ifdef CW_POSIX
    "euid",
#endif
    "eval",
    "exch",
#ifdef CW_POSIX
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
#ifdef CW_POSIX
    "fork",
#endif
    "gcdict",
    "ge",
    "get",
    "getinterval",
#ifdef CW_POSIX
    "gid",
#endif
    "globaldict",
    "gt",
    "hooktag",
    "hooktype",
    "if",
    "ifelse",
    "index",
#ifdef CW_POSIX
    "ino",
#endif
    "integertype",
    "invalidaccess",
    "invalidexit",
    "invalidfileaccess",
    "iobuf",
    "ioerror",
    "istack",
#ifdef CW_THREADS
    "join",
#endif
    "known",
#ifdef CW_THREADS
    "lcheck",
#endif
    "le",
    "length",
    "limitcheck",
#ifdef CW_POSIX
    "link",
#endif
    "load",
#ifdef CW_THREADS
    "lock",
#endif
    "loop",
    "lt",
    "mark",
    "marktype",
#ifdef CW_POSIX
    "mkdir",
#endif
    "mod",
#ifdef CW_POSIX
    "mode",
#endif
#ifdef HAVE_DLOPEN
    "modload",
#endif
#ifdef CW_THREADS
    "monitor",
#endif
#ifdef CW_POSIX
    "mtime",
#endif
    "mul",
#ifdef CW_THREADS
    "mutex",
    "mutextype",
#endif
    "nametype",
    "ndup",
    "ne",
    "neg",
    "newerror",
#ifdef CW_POSIX
    "nlink",
#endif
    "not",
    "npop",
#ifdef CW_POSIX
    "nsleep",
#endif
    "null",
    "nulltype",
#ifdef CW_POSIX
    "open",
#endif
    "operatortype",
    "or",
    "ostack",
#ifdef CW_THREADS
    "period",
#endif
#ifdef CW_POSIX
    "pid",
#endif
    "pmark",
    "pmarktype",
    "pop",
#ifdef CW_POSIX
    "ppid",
#endif
    "print",
    "product",
    "promptstring",
    "put",
    "putinterval",
#ifdef CW_POSIX
    "pwd",
#endif
    "quit",
    "rand",
    "rangecheck",
#ifdef CW_POSIX
    "rdev",
#endif
    "read",
    "readline",
#ifdef CW_POSIX
    "realtime",
    "rename",
#endif
    "repeat",
#ifdef CW_POSIX
    "rmdir",
#endif
    "roll",
    "sclear",
    "scleartomark",
    "scount",
    "scounttomark",
    "sdup",
#ifdef CW_POSIX
    "seek",
#endif
#ifdef CW_THREADS
    "self",
#endif
    "setactive",
#ifdef CW_POSIX
    "setegid",
    "setenv",
    "seteuid",
    "setgid",
#endif
    "setiobuf",
#ifdef CW_THREADS
    "setlocking",
    "setperiod",
#endif
    "setthreshold",
#ifdef CW_POSIX
    "setuid",
#endif
    "sexch",
    "shift",
#ifdef CW_THREADS
    "signal",
#endif
    "sindex",
#ifdef CW_POSIX
    "size",
#endif
    "spop",
    "spush",
#ifdef CW_POSIX
    "srand",
#endif
    "sroll",
    "stack",
    "stacktype",
    "stackunderflow",
    "start",
    "stats",
#ifdef CW_POSIX
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
#ifdef CW_POSIX
    "symlink",
#endif
    "syntaxerror",
    "system",
    "systemdict",
    "tell",
#ifdef CW_POSIX
    "test",
#endif
#ifdef CW_THREADS
    "thread",
    "threadtype",
#endif
    "threshold",
    "throw",
#ifdef CW_THREADS
    "timedwait",
#endif
    "token",
    "true",
#ifdef CW_POSIX
    "truncate",
#endif
#ifdef CW_THREADS
    "trylock",
#endif
    "type",
    "typecheck",
#ifdef CW_POSIX
    "uid",
#endif
    "undef",
    "undefined",
    "undefinedfilename",
    "undefinedresult",
#ifdef CW_POSIX
    "unlink",
#endif
#ifdef CW_THREADS
    "unlock",
#endif
    "unmatchedfino",
    "unmatchedmark",
    "unregistered",
#ifdef CW_POSIX
    "unsetenv",
#endif
    "version",
#ifdef CW_THREADS
    "wait",
#endif
#ifdef CW_POSIX
    "waitpid",
#endif
    "where",
    "write",
    "xcheck",
    "xor"
#ifdef CW_THREADS
    ,
    "yield"
#endif
};
