/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
******************************************************************************
*
* <Copyright = jasone>
* <License>
*
******************************************************************************
*
* Version: Onyx <Version = onyx>
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
#ifdef CW_POSIX
    "accept",
#endif
    "active",
    "add",
    "and",
    "argv",
    "array",
    "arraytype",
#ifdef CW_REAL
    "atan",
#endif
#ifdef CW_POSIX
    "atime",
#endif
    "begin",
    "bind",
#ifdef CW_POSIX
    "bindsocket",
    "blksize",
    "blocks",
#endif
    "booleantype",
    "bpop",
#ifdef CW_THREADS
    "broadcast",
#endif
    "bytesavailable",
    "cat",
#ifdef CW_POSIX
    "cd",
#endif
#ifdef CW_REAL
    "ceiling",
#endif
#ifdef CW_POSIX
    "chmod",
    "chown",
    "chroot",
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
#ifdef CW_POSIX
    "connect",
#endif
    "copy",
#ifdef CW_REAL
    "cos",
#endif
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
#ifdef CW_REAL
    "cvds",
#endif
    "cve",
#ifdef CW_REAL
    "cves",
#endif
    "cvlit",
    "cvn",
    "cvrs",
    "cvs",
    "cvx",
    "dec",
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
#ifdef CW_REAL
    "div",
#endif
    "dn",
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
#ifdef CW_POSIX
    "fcntl",
    "fifo",
#endif
    "filetype",
    "finotype",
#ifdef CW_REAL
    "floor",
#endif
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
#ifdef CW_THREADS
    "gstderr",
    "gstdin",
    "gstdout",
#endif
    "gt",
    "hooktag",
#ifdef CW_POSIX
    "hostbyname",
#endif
    "hooktype",
    "idiv",
    "idup",
    "if",
    "ifelse",
    "inc",
#ifdef CW_POSIX
    "ino",
#endif
    "integertype",
    "invalidaccess",
    "invalidexit",
    "invalidfileaccess",
    "iobuf",
#ifdef CW_POSIX
    "ioctl",
#endif
    "ioerror",
    "ipop",
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
    "listen",
#endif
#ifdef CW_REAL
    "ln",
#endif
    "load",
#ifdef CW_THREADS
    "lock",
#endif
#ifdef CW_REAL
    "log",
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
    "nbpop",
    "ndn",
    "ndup",
    "ne",
    "neg",
    "newerror",
    "nip",
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
    "nup",
    "onyxdict",
#ifdef CW_POSIX
    "open",
#endif
    "operatortype",
    "or",
    "ostack",
    "over",
#ifdef CW_POSIX
    "peername",
#endif
#ifdef CW_THREADS
    "period",
#endif
#ifdef CW_POSIX
    "pid",
#endif
    "pmark",
    "pmarktype",
#ifdef CW_POSIX
    "pipe",
    "poll",
    "POLLERR",
    "POLLHUP",
    "POLLIN",
    "POLLNVAL",
    "POLLOUT",
    "POLLPRI",
    "POLLRDBAND",
    "POLLRDNORM",
    "POLLWRNORM",
    "POLLWRBAND",
#endif
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
    "readlink",
    "realtime",
#endif
#ifdef CW_REAL
    "realtype",
#endif
#ifdef CW_POSIX
    "recv",
    "rename",
#endif
    "repeat",
#ifdef CW_POSIX
    "rmdir",
#endif
    "roll",
#ifdef CW_REAL
    "round",
#endif
    "sbpop",
    "sclear",
    "scleartomark",
    "scount",
    "scounttomark",
    "sdn",
    "sdup",
#ifdef CW_POSIX
    "seek",
#endif
#ifdef CW_THREADS
    "self",
#endif
#ifdef CW_POSIX
    "send",
#endif
    "setactive",
#ifdef CW_POSIX
    "setegid",
    "setenv",
    "seteuid",
    "setgid",
#endif
#ifdef CW_THREADS
    "setgstderr",
    "setgstdin",
    "setgstdout",
#endif
    "setiobuf",
#ifdef CW_THREADS
    "setlocking",
    "setperiod",
#endif
#ifdef CW_POSIX
    "setsockopt",
#endif
    "setstderr",
    "setstdin",
    "setstdout",
    "setthreshold",
#ifdef CW_POSIX
    "setuid",
    "setumask",
#endif
    "sexch",
    "shift",
    "sidup",
#ifdef CW_THREADS
    "signal",
#endif
#ifdef CW_REAL
    "sin",
#endif
    "sipop",
#ifdef CW_POSIX
    "size",
#endif
    "snbpop",
    "sndn",
    "sndup",
    "snip",
    "snpop",
    "snup",
#ifdef CW_POSIX
    "socket",
    "socketpair",
    "sockname",
    "sockopt",
#endif
    "sover",
    "spop",
    "spush",
#ifdef CW_REAL
    "sqrt",
#endif
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
    "stuck",
    "sub",
    "sunder",
    "sup",
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
    "threadsdict",
    "threadstate",
    "threadtype",
#endif
    "threshold",
    "throw",
#ifdef CW_THREADS
    "timedwait",
#endif
    "token",
    "true",
#ifdef CW_REAL
    "trunc",
#endif
#ifdef CW_POSIX
    "truncate",
#endif
#ifdef CW_THREADS
    "trylock",
#endif
    "tstack",
    "tuck",
    "type",
    "typecheck",
#ifdef CW_POSIX
    "uid",
    "umask",
#endif
    "undef",
    "undefined",
    "undefinedfilename",
    "undefinedresult",
    "under",
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
    "until",
    "up",
    "version",
#ifdef CW_THREADS
    "wait",
#endif
#ifdef CW_POSIX
    "waitpid",
#endif
    "where",
    "while",
    "write",
    "xcheck",
    "xor"
#ifdef CW_THREADS
    ,
    "yield"
#endif
};
