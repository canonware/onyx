/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <Copyright = mordor>
 * <License>
 *
 ******************************************************************************
 *
 * Version: Onyx <Version = onyx>
 *
 ******************************************************************************/

#define CW_SYSTEMDICT_C_

#include "../include/libonyx/libonyx.h"

#include <unistd.h>
#include <sys/time.h> /* For realtime operator. */
#include <time.h> /* For nanosleep() and localtime_r(). */
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <dirent.h> /* For dirforeach operator. */
#ifdef CW_SOCKET
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/un.h>
#ifndef CW_HAVE_SOCKLEN_T
/* socklen_t is missing on Mac OS X <= 10.2. */
typedef int socklen_t;
#endif
#endif
#ifdef CW_REAL
#ifndef HAVE_ASPRINTF
#include "asprintf.c"
#endif
#endif
#ifdef CW_MODULES
#include <dlfcn.h> /* For modload operator. */
#endif
#ifdef CW_POSIX
#ifdef HAVE_POLL
#include <sys/poll.h>
#else
#include "poll.c"
#endif
#endif
#ifdef CW_REAL
#include <math.h>
#endif

#include "../include/libonyx/nxa_l.h"
#include "../include/libonyx/nxo_l.h"
#include "../include/libonyx/nxo_array_l.h"
#include "../include/libonyx/nxo_operator_l.h"
#ifdef CW_REGEX
#include "../include/libonyx/nxo_regex_l.h"
#endif
#include "../include/libonyx/nxo_thread_l.h"

#ifdef CW_SOCKET
struct cw_systemdict_name_arg
{
    cw_nxn_t nxn;
    int arg;
};

/* Globals. */
#if (defined(CW_SOCKET) && defined(CW_THREADS))
cw_mtx_t cw_g_gethostbyname_mtx;
cw_mtx_t cw_g_getprotobyname_mtx;
cw_mtx_t cw_g_getservbyname_mtx;
#endif

/* Compare a_name to the names in a_arg.  If successful, return the index of the
 * matching element.  Otherwise, return a_argcnt. */
static cw_uint32_t
systemdict_p_name_arg(cw_nxo_t *a_name,
		      const struct cw_systemdict_name_arg *a_arg,
		      cw_uint32_t a_argcnt)
{
    const cw_uint8_t *str;
    cw_uint32_t len, i;

    cw_assert(nxo_type_get(a_name) == NXOT_NAME);

    str = nxo_name_str_get(a_name);
    len = nxo_name_len_get(a_name);
    for (i = 0; i < a_argcnt; i++)
    {
	if (nxn_len(a_arg[i].nxn) == len
	    && memcmp(nxn_str(a_arg[i].nxn), str, len) == 0)
	{
	    break;
	}
    }

    return i;
}
#endif

struct cw_systemdict_entry
{
    cw_nxn_t nxn;
    cw_op_t *op_f;
};

#define ENTRY(name) {NXN_##name, systemdict_##name}

/* Array of operators in systemdict. */
static const struct cw_systemdict_entry systemdict_ops[] = {
    ENTRY(abs),
#ifdef CW_SOCKET
    ENTRY(accept),
#endif
#ifdef CW_REAL
    ENTRY(acos),
    ENTRY(acosh),
#endif
    ENTRY(add),
    ENTRY(adn),
    ENTRY(and),
    ENTRY(array),
#ifdef CW_REAL
    ENTRY(asin),
    ENTRY(asinh),
    ENTRY(atan),
    ENTRY(atan2),
    ENTRY(atanh),
#endif
    ENTRY(aup),
    ENTRY(bdup),
    ENTRY(begin),
    ENTRY(bind),
#ifdef CW_SOCKET
    ENTRY(bindsocket),
#endif
    ENTRY(bpop),
#ifdef CW_THREADS
    ENTRY(broadcast),
#endif
    ENTRY(bytesavailable),
    ENTRY(cat),
#ifdef CW_POSIX
    ENTRY(cd),
#endif
#ifdef CW_REAL
    ENTRY(ceiling),
#endif
#ifdef CW_POSIX
    ENTRY(chmod),
    ENTRY(chown),
    ENTRY(chroot),
#endif
    ENTRY(clear),
    ENTRY(cleartomark),
#ifdef CW_POSIX
    ENTRY(close),
#endif
#ifdef CW_THREADS
    ENTRY(condition),
#endif
#ifdef CW_SOCKET
    ENTRY(connect),
#endif
    ENTRY(copy),
#ifdef CW_REAL
    ENTRY(cos),
    ENTRY(cosh),
#endif
    ENTRY(count),
    ENTRY(countdstack),
    ENTRY(countestack),
    ENTRY(counttomark),
    ENTRY(currentdict),
#ifdef CW_THREADS
    ENTRY(currentlocking),
#endif
#ifdef CW_REAL
    ENTRY(cvds),
#endif
    ENTRY(cve),
#ifdef CW_REAL
    ENTRY(cves),
#endif
    ENTRY(cvlit),
    ENTRY(cvn),
    ENTRY(cvrs),
    ENTRY(cvs),
    ENTRY(cvx),
    ENTRY(dec),
    ENTRY(def),
#ifdef CW_THREADS
    ENTRY(detach),
#endif
    ENTRY(dict),
    ENTRY(die),
#ifdef CW_POSIX
    ENTRY(dirforeach),
#endif
#ifdef CW_REAL
    ENTRY(div),
#endif
    ENTRY(dn),
    ENTRY(dstack),
    ENTRY(dup),
    ENTRY(echeck),
#ifdef CW_POSIX
    ENTRY(egid),
#endif
    ENTRY(end),
    ENTRY(eq),
    ENTRY(estack),
#ifdef CW_POSIX
    ENTRY(euid),
#endif
    ENTRY(eval),
    ENTRY(exch),
#ifdef CW_POSIX
    ENTRY(exec),
#endif
    ENTRY(exit),
#ifdef CW_REAL
    ENTRY(exp),
    ENTRY(floor),
#endif
    ENTRY(flush),
    ENTRY(flushfile),
    ENTRY(for),
    ENTRY(foreach),
#ifdef CW_POSIX
    ENTRY(forkexec),
#endif
    ENTRY(ge),
    ENTRY(get),
    ENTRY(getinterval),
#ifdef CW_POSIX
    ENTRY(gid),
#endif
#ifdef CW_THREADS
    ENTRY(gstderr),
    ENTRY(gstdin),
    ENTRY(gstdout),
#endif
    ENTRY(gt),
#ifdef CW_HANDLE
    ENTRY(handletag),
#endif
    ENTRY(ibdup),
    ENTRY(ibpop),
    ENTRY(idiv),
    ENTRY(idup),
    ENTRY(if),
    ENTRY(ifelse),
    ENTRY(inc),
    ENTRY(iobuf),
    ENTRY(ipop),
    ENTRY(istack),
#ifdef CW_THREADS
    ENTRY(join),
#endif
    ENTRY(known),
#ifdef CW_THREADS
    ENTRY(lcheck),
#endif
    ENTRY(le),
    ENTRY(length),
#ifdef CW_POSIX
    ENTRY(link),
#endif
#ifdef CW_SOCKET
    ENTRY(listen),
#endif
#ifdef CW_REAL
    ENTRY(ln),
#endif
    ENTRY(load),
#ifdef CW_POSIX
    ENTRY(localtime),
#endif
#ifdef CW_THREADS
    ENTRY(lock),
#endif
#ifdef CW_REAL
    ENTRY(log),
#endif
    ENTRY(loop),
    ENTRY(lt),
#ifdef CW_REGEX
    ENTRY(match),
#endif
#ifdef CW_POSIX
    ENTRY(mkdir),
    ENTRY(mkfifo),
#endif
    ENTRY(mod),
#ifdef CW_MODULES
    ENTRY(modload),
#endif
#ifdef CW_THREADS
    ENTRY(monitor),
#endif
    ENTRY(mul),
#ifdef CW_THREADS
    ENTRY(mutex),
#endif
    ENTRY(nbpop),
    ENTRY(ncat),
    ENTRY(ndn),
    ENTRY(ndup),
    ENTRY(ne),
    ENTRY(neg),
    ENTRY(nip),
    ENTRY(nonblocking),
    ENTRY(not),
    ENTRY(npop),
#ifdef CW_POSIX
    ENTRY(nsleep),
#endif
    ENTRY(nup),
#ifdef CW_REGEX
    ENTRY(offset),
#endif
#ifdef CW_POSIX
    ENTRY(open),
#endif
    ENTRY(or),
    ENTRY(ostack),
    ENTRY(over),
#ifdef CW_SOCKET
    ENTRY(peername),
#endif
#ifdef CW_POSIX
    ENTRY(pid),
    ENTRY(pipe),
    ENTRY(poll),
#endif
    ENTRY(pop),
    ENTRY(pow),
#ifdef CW_POSIX
    ENTRY(ppid),
#endif
    ENTRY(print),
    ENTRY(put),
    ENTRY(putinterval),
#ifdef CW_POSIX
    ENTRY(pwd),
#endif
    ENTRY(quit),
    ENTRY(rand),
    ENTRY(read),
    ENTRY(readline),
#ifdef CW_POSIX
    ENTRY(readlink),
    ENTRY(realtime),
#endif
#ifdef CW_SOCKET
    ENTRY(recv),
#endif
#ifdef CW_REGEX
    ENTRY(regex),
    ENTRY(regsub),
#endif
#ifdef CW_POSIX
    ENTRY(rename),
#endif
    ENTRY(repeat),
#ifdef CW_POSIX
    ENTRY(rmdir),
#endif
    ENTRY(roll),
    ENTRY(rot),
#ifdef CW_REAL
    ENTRY(round),
#endif
    ENTRY(sadn),
    ENTRY(saup),
    ENTRY(sbdup),
    ENTRY(sbpop),
    ENTRY(sbpush),
    ENTRY(sclear),
    ENTRY(scleartomark),
    ENTRY(scount),
    ENTRY(scounttomark),
    ENTRY(sdn),
    ENTRY(sdup),
#ifdef CW_POSIX
    ENTRY(seek),
#endif
    ENTRY(self),
#ifdef CW_SOCKET
    ENTRY(send),
    ENTRY(serviceport),
#endif
#ifdef CW_POSIX
    ENTRY(setegid),
    ENTRY(setenv),
    ENTRY(seteuid),
    ENTRY(setgid),
#endif
#ifdef CW_THREADS
    ENTRY(setgstderr),
    ENTRY(setgstdin),
    ENTRY(setgstdout),
#endif
    ENTRY(setiobuf),
#ifdef CW_THREADS
    ENTRY(setlocking),
#endif
    ENTRY(setnonblocking),
#ifdef CW_SOCKET
    ENTRY(setsockopt),
#endif
    ENTRY(setstderr),
    ENTRY(setstdin),
    ENTRY(setstdout),
#ifdef CW_POSIX
    ENTRY(setuid),
#endif
    ENTRY(sexch),
    ENTRY(shift),
    ENTRY(sibdup),
    ENTRY(sibpop),
    ENTRY(sidup),
#ifdef CW_THREADS
    ENTRY(signal),
#endif
#ifdef CW_REAL
    ENTRY(sin),
    ENTRY(sinh),
#endif
    ENTRY(sipop),
    ENTRY(snbpop),
    ENTRY(sndn),
    ENTRY(sndup),
    ENTRY(snip),
    ENTRY(snpop),
    ENTRY(snup),
#ifdef CW_SOCKET
    ENTRY(socket),
    ENTRY(socketpair),
    ENTRY(sockname),
    ENTRY(sockopt),
#endif
    ENTRY(sover),
#ifdef CW_REGEX
    ENTRY(split),
#endif
    ENTRY(spop),
    ENTRY(spush),
#ifdef CW_REAL
    ENTRY(sqrt),
#endif
#ifdef CW_POSIX
    ENTRY(srand),
#endif
    ENTRY(sroll),
    ENTRY(srot),
    ENTRY(stack),
    ENTRY(start),
#ifdef CW_POSIX
    ENTRY(status),
#endif
    ENTRY(stderr),
    ENTRY(stdin),
    ENTRY(stdout),
    ENTRY(stop),
    ENTRY(stopped),
    ENTRY(string),
    ENTRY(stuck),
    ENTRY(sub),
#ifdef CW_REGEX
    ENTRY(submatch),
    ENTRY(subst),
#endif
    ENTRY(sunder),
    ENTRY(sup),
    ENTRY(sym_lp),
    ENTRY(sym_rp),
    ENTRY(sym_gt),
    ENTRY(sym_rb),
#ifdef CW_POSIX
    ENTRY(symlink),
#endif
#ifdef CW_REAL
    ENTRY(tan),
    ENTRY(tanh),
#endif
    ENTRY(tell),
#ifdef CW_POSIX
    ENTRY(test),
#endif
#ifdef CW_THREADS
    ENTRY(thread),
#endif
    ENTRY(threaddstack),
    ENTRY(threadestack),
    ENTRY(threadistack),
    ENTRY(threadostack),
#ifdef CW_THREADS
    ENTRY(threadsdict),
    ENTRY(timedwait),
#endif
    ENTRY(token),
#ifdef CW_REAL
    ENTRY(trunc),
#endif
#ifdef CW_POSIX
    ENTRY(truncate),
#endif
#ifdef CW_THREADS
    ENTRY(trylock),
#endif
    ENTRY(tuck),
    ENTRY(type),
#ifdef CW_POSIX
    ENTRY(uid),
    ENTRY(umask),
#endif
    ENTRY(undef),
    ENTRY(under),
    ENTRY(unless),
#ifdef CW_POSIX
    ENTRY(unlink),
#endif
#ifdef CW_THREADS
    ENTRY(unlock),
#endif
#ifdef CW_POSIX
    ENTRY(unsetenv),
#endif
    ENTRY(until),
    ENTRY(up),
#ifdef CW_THREADS
    ENTRY(wait),
#endif
#ifdef CW_POSIX
    ENTRY(waitpid),
#endif
    ENTRY(where),
    ENTRY(while),
    ENTRY(write),
    ENTRY(xcheck),
    ENTRY(xor)
#ifdef CW_THREADS
    ,
    ENTRY(yield)
#endif
};

void
systemdict_l_init(void)
{
#ifdef CW_POSIX
    /* Ignore SIGPIPE, so that writing to a closed socket won't crash the
     * program. */
    signal(SIGPIPE, SIG_IGN);
#endif

#if (defined(CW_SOCKET) && defined(CW_THREADS))
    /* Initialize mutexes that protect non-reentrant functions. */
    mtx_new(&cw_g_gethostbyname_mtx);
    mtx_new(&cw_g_getprotobyname_mtx);
    mtx_new(&cw_g_getservbyname_mtx);
#endif
}

void
systemdict_l_shutdown(void)
{
#if (defined(CW_SOCKET) && defined(CW_THREADS))
    mtx_delete(&cw_g_getservbyname_mtx);
    mtx_delete(&cw_g_getprotobyname_mtx);
    mtx_delete(&cw_g_gethostbyname_mtx);
#endif
}

void
systemdict_l_populate(cw_nxo_t *a_dict, cw_nxo_t *a_tname, cw_nxo_t *a_tvalue,
		      cw_nx_t *a_nx)
{
    cw_uint32_t i;

/* Number of names that are defined below, but not as operators. */
#ifdef CW_POSIX
#define NEXTRA 12
#else
#define NEXTRA 11
#endif
#define NOPS								\
	(sizeof(systemdict_ops) / sizeof(struct cw_systemdict_entry))

    nxo_dict_new(a_dict, TRUE,
		 NOPS + NEXTRA + CW_LIBONYX_SYSTEMDICT_HASH_SPARE);

    /* Operators. */
    for (i = 0; i < NOPS; i++)
    {
	nxo_name_new(a_tname, nxn_str(systemdict_ops[i].nxn),
		     nxn_len(systemdict_ops[i].nxn), TRUE);
	nxo_operator_new(a_tvalue, systemdict_ops[i].op_f,
			 systemdict_ops[i].nxn);
	nxo_attr_set(a_tvalue, NXOA_EXECUTABLE);

	nxo_dict_def(a_dict, a_tname, a_tvalue);
    }

    /* Initialize entries that are not operators. */

    /* globaldict. */
    nxo_name_new(a_tname, nxn_str(NXN_globaldict), nxn_len(NXN_globaldict),
		 TRUE);
    nxo_dup(a_tvalue, nx_globaldict_get(a_nx));
    nxo_dict_def(a_dict, a_tname, a_tvalue);

    /* systemdict. */
    nxo_name_new(a_tname, nxn_str(NXN_systemdict), nxn_len(NXN_systemdict),
		 TRUE);
    nxo_dict_def(a_dict, a_tname, a_dict);

    /* gcdict. */
    nxo_name_new(a_tname, nxn_str(NXN_gcdict), nxn_len(NXN_gcdict), TRUE);
    nxo_dup(a_tvalue, nx_gcdict_get(a_nx));
    nxo_dict_def(a_dict, a_tname, a_tvalue);

#ifdef CW_POSIX
    /* envdict. */
    nxo_name_new(a_tname, nxn_str(NXN_envdict), nxn_len(NXN_envdict), TRUE);
    nxo_dup(a_tvalue, libonyx_envdict_get());
    nxo_dict_def(a_dict, a_tname, a_tvalue);
#endif

    /* onyxdict. */
    nxo_name_new(a_tname, nxn_str(NXN_onyxdict), nxn_len(NXN_onyxdict), TRUE);
    nxo_dict_new(a_tvalue, TRUE, CW_LIBONYX_ONYXDICT_HASH);
    nxo_dict_def(a_dict, a_tname, a_tvalue);

    /* argv. */
    nxo_name_new(a_tname, nxn_str(NXN_argv), nxn_len(NXN_argv), TRUE);
    nxo_dup(a_tvalue, libonyx_argv_get());
    nxo_dict_def(a_dict, a_tname, a_tvalue);

    /* true. */
    nxo_name_new(a_tname, nxn_str(NXN_true), nxn_len(NXN_true), TRUE);
    nxo_boolean_new(a_tvalue, TRUE);
    nxo_dict_def(a_dict, a_tname, a_tvalue);

    /* false. */
    nxo_name_new(a_tname, nxn_str(NXN_false), nxn_len(NXN_false), TRUE);
    nxo_boolean_new(a_tvalue, FALSE);
    nxo_dict_def(a_dict, a_tname, a_tvalue);

    /* mark. */
    nxo_name_new(a_tname, nxn_str(NXN_mark), nxn_len(NXN_mark), TRUE);
    nxo_mark_new(a_tvalue);
    nxo_dict_def(a_dict, a_tname, a_tvalue);

    /* <. */
    nxo_name_new(a_tname, nxn_str(NXN_sym_lt), nxn_len(NXN_sym_lt), TRUE);
    nxo_mark_new(a_tvalue);
    nxo_dict_def(a_dict, a_tname, a_tvalue);

    /* [. */
    nxo_name_new(a_tname, nxn_str(NXN_sym_lb), nxn_len(NXN_sym_lb), TRUE);
    nxo_mark_new(a_tvalue);
    nxo_dict_def(a_dict, a_tname, a_tvalue);

    /* null. */
    nxo_name_new(a_tname, nxn_str(NXN_null), nxn_len(NXN_null), TRUE);
    nxo_null_new(a_tvalue);
    nxo_dict_def(a_dict, a_tname, a_tvalue);

    cw_assert(nxo_dict_count(a_dict) == NOPS + NEXTRA);
#undef NOPS
#undef NEXTRA
}

void
systemdict_abs(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *a;

    ostack = nxo_thread_ostack_get(a_thread);
	
    NXO_STACK_GET(a, ostack, a_thread);
    switch (nxo_type_get(a))
    {
	case NXOT_INTEGER:
	{
	    if (nxo_integer_get(a) < 0)
	    {
		nxo_integer_set(a, -nxo_integer_get(a));
	    }
	    break;
	}
#ifdef CW_REAL
	case NXOT_REAL:
	{
	    if (nxo_real_get(a) < 0)
	    {
		nxo_real_set(a, -nxo_real_get(a));
	    }
	    break;
	}
#endif
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }
}

#ifdef CW_SOCKET
static cw_bool_t
systemdict_p_sock_family(cw_nxo_t *a_thread, int a_fd, cw_bool_t a_peer,
			 sa_family_t *r_family)
{
    cw_bool_t retval;
    int error;
/* Stevens hard codes 128 in his unp.h header.  This seems sketchy to me, but
 * should work fine for the limited types of sockets Onyx supports. */
#define CW_MAXSOCKADDR 128
    union
    {
	struct sockaddr sa;
	char pad[CW_MAXSOCKADDR];
    } u;
    socklen_t len;

    len = CW_MAXSOCKADDR;
#undef CW_MAXSOCKADDR
    if (a_peer)
    {
	error = getpeername(a_fd, &u.sa, &len);
    }
    else
    {
	error = getsockname(a_fd, &u.sa, &len);
    }
    if (error == -1)
    {
	switch (errno)
	{
	    case EBADF:
	    {
		nxo_thread_nerror(a_thread, NXN_ioerror);
		retval = TRUE;
		goto RETURN;
	    }
	    case ECONNRESET:
	    case ENOTCONN:
	    {
		nxo_thread_nerror(a_thread, NXN_neterror);
		retval = TRUE;
		goto RETURN;
	    }
	    case ENOTSOCK:
	    {
		nxo_thread_nerror(a_thread, NXN_argcheck);
		retval = TRUE;
		goto RETURN;
	    }
	    case ENOBUFS:
	    {
		xep_throw(CW_ONYXX_OOM);
		/* Not reached. */
	    }
	    default:
	    {
		nxo_thread_nerror(a_thread, NXN_unregistered);
		retval = TRUE;
		goto RETURN;
	    }
	}
    }

    *r_family = u.sa.sa_family;

    retval = FALSE;
    RETURN:
    return retval;
}

void
systemdict_accept(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *sock;
    sa_family_t family;
    int sockfd;
    socklen_t sockaddrlen;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(sock, ostack, a_thread);
    if (nxo_type_get(sock) != NXOT_FILE)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    
    /* Get the socket family. */
    if (systemdict_p_sock_family(a_thread, nxo_file_fd_get(sock), FALSE,
				 &family))
    {
	return;
    }

    switch (family)
    {
	case AF_INET:
	{
	    struct sockaddr_in sockaddr;

	    sockaddrlen = sizeof(sockaddr);
	    sockfd = accept(nxo_file_fd_get(sock), (struct sockaddr *)&sockaddr,
			    &sockaddrlen);
	    break;
	}
	case AF_LOCAL:
	{
	    struct sockaddr_un sockaddr;

	    sockaddrlen = sizeof(sockaddr);
	    sockfd = accept(nxo_file_fd_get(sock), (struct sockaddr *)&sockaddr,
			    &sockaddrlen);
	    break;
	}
	default:
	{
	    cw_not_reached();
	}
    }

    if (sockfd == -1)
    {
	switch (errno)
	{
	    case ECONNABORTED:
	    {
		nxo_thread_nerror(a_thread, NXN_neterror);
		return;
	    }
	    case EINTR:
	    case EWOULDBLOCK:
	    {
		nxo_thread_nerror(a_thread, NXN_ioerror);
		return;
	    }
	    case ENOTSOCK:
	    case EOPNOTSUPP:
	    {
		nxo_thread_nerror(a_thread, NXN_argcheck);
		return;
	    }
	    case EPERM:
	    {
		nxo_thread_nerror(a_thread, NXN_invalidfileaccess);
		return;
	    }
	    case EBADF:
	    case EFAULT:
	    case EINVAL:
	    case EMFILE:
	    case ENFILE:
	    case ENOBUFS:
	    case ENOMEM:
	    default:
	    {
		nxo_thread_nerror(a_thread, NXN_unregistered);
		return;
	    }
	}
    }

    nxo_file_new(sock, nxo_thread_currentlocking(a_thread));
    nxo_file_fd_wrap(sock, sockfd, TRUE);
}
#endif

#ifdef CW_REAL
void
systemdict_acos(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;
    cw_nxor_t real;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    switch (nxo_type_get(nxo))
    {
	case NXOT_INTEGER:
	{
	    real = (cw_nxor_t) nxo_integer_get(nxo);
	    break;
	}
	case NXOT_REAL:
	{
	    real = nxo_real_get(nxo);
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }

    if (fabs(real) > 1.0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    nxo_real_new(nxo, acos(real));
}
#endif

#ifdef CW_REAL
void
systemdict_acosh(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;
    cw_nxor_t real;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    switch (nxo_type_get(nxo))
    {
	case NXOT_INTEGER:
	{
	    real = (cw_nxor_t) nxo_integer_get(nxo);
	    break;
	}
	case NXOT_REAL:
	{
	    real = nxo_real_get(nxo);
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }

    if (real < 1.0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    nxo_real_new(nxo, acosh(real));
}
#endif

void
systemdict_add(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo_a, *nxo_b;
    cw_nxoi_t integer_a, integer_b;
#ifdef CW_REAL
    cw_bool_t do_real;
    cw_nxor_t real_a, real_b;
#endif

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo_b, ostack, a_thread);
    NXO_STACK_DOWN_GET(nxo_a, ostack, a_thread, nxo_b);
    switch (nxo_type_get(nxo_a))
    {
	case NXOT_INTEGER:
	{
#ifdef CW_REAL
	    do_real = FALSE;
#endif
	    integer_a = nxo_integer_get(nxo_a);
	    break;
	}
#ifdef CW_REAL
	case NXOT_REAL:
	{
	    do_real = TRUE;
	    real_a = nxo_real_get(nxo_a);
	    break;
	}
#endif
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }
    switch (nxo_type_get(nxo_b))
    {
	case NXOT_INTEGER:
	{
#ifdef CW_REAL
	    if (do_real)
	    {
		real_b = (cw_nxor_t) nxo_integer_get(nxo_b);
	    }
	    else
#endif
	    {
		integer_b = nxo_integer_get(nxo_b);
	    }
	    break;
	}
#ifdef CW_REAL
	case NXOT_REAL:
	{
	    real_b = nxo_real_get(nxo_b);
	    if (do_real == FALSE)
	    {
		do_real = TRUE;
		real_a = (cw_nxor_t) integer_a;
	    }
	    break;
	}
#endif
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }

#ifdef CW_REAL
    if (do_real)
    {
	/* nxo_a may be an integer, so use nxo_real_new() rather than
	 * nxo_real_set(). */
	nxo_real_new(nxo_a, real_a + real_b);
    }
    else
#endif
    {
	nxo_integer_set(nxo_a, integer_a + integer_b);
    }

    nxo_stack_pop(ostack);
}

void
systemdict_adn(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo, *bnxo;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_BGET(bnxo, ostack, a_thread);
    nxo = nxo_stack_push(ostack);
    nxo_dup(nxo, bnxo);
    nxo_stack_bpop(ostack);
}

void
systemdict_and(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo_a, *nxo_b;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(nxo_b, ostack, a_thread);
    NXO_STACK_DOWN_GET(nxo_a, ostack, a_thread, nxo_b);

    if (nxo_type_get(nxo_a) == NXOT_BOOLEAN
	&& nxo_type_get(nxo_b) == NXOT_BOOLEAN)
    {
	cw_bool_t and;

	if (nxo_boolean_get(nxo_a) && nxo_boolean_get(nxo_b))
	{
	    and = TRUE;
	}
	else
	{
	    and = FALSE;
	}
	nxo_boolean_new(nxo_a, and);
    }
    else if (nxo_type_get(nxo_a) == NXOT_INTEGER
	     && nxo_type_get(nxo_b) ==   NXOT_INTEGER)
    {
	nxo_integer_set(nxo_a, nxo_integer_get(nxo_a) & nxo_integer_get(nxo_b));
    }
    else
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    nxo_stack_pop(ostack);
}

void
systemdict_array(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;
    cw_nxoi_t len;

    ostack = nxo_thread_ostack_get(a_thread);
	
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    len = nxo_integer_get(nxo);
    if (len < 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    nxo_array_new(nxo, nxo_thread_currentlocking(a_thread), len);
}

#ifdef CW_REAL
void
systemdict_asin(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;
    cw_nxor_t real;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    switch (nxo_type_get(nxo))
    {
	case NXOT_INTEGER:
	{
	    real = (cw_nxor_t) nxo_integer_get(nxo);
	    break;
	}
	case NXOT_REAL:
	{
	    real = nxo_real_get(nxo);
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }

    if (fabs(real) > 1.0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    nxo_real_new(nxo, asin(real));
}
#endif

#ifdef CW_REAL
void
systemdict_asinh(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;
    cw_nxor_t real;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    switch (nxo_type_get(nxo))
    {
	case NXOT_INTEGER:
	{
	    real = (cw_nxor_t) nxo_integer_get(nxo);
	    break;
	}
	case NXOT_REAL:
	{
	    real = nxo_real_get(nxo);
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }

    nxo_real_new(nxo, asinh(real));
}
#endif

#ifdef CW_REAL
void
systemdict_atan(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;
    cw_nxor_t real;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    switch (nxo_type_get(nxo))
    {
	case NXOT_INTEGER:
	{
	    real = (cw_nxor_t) nxo_integer_get(nxo);
	    break;
	}
	case NXOT_REAL:
	{
	    real = nxo_real_get(nxo);
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }

    nxo_real_new(nxo, atan(real));
}
#endif

#ifdef CW_REAL
void
systemdict_atan2(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo_x, *nxo_y;
    cw_nxor_t x, y;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo_x, ostack, a_thread);
    NXO_STACK_DOWN_GET(nxo_y, ostack, a_thread, nxo_x);
    switch (nxo_type_get(nxo_y))
    {
	case NXOT_INTEGER:
	{
	    y = (cw_nxor_t) nxo_integer_get(nxo_y);
	    break;
	}
	case NXOT_REAL:
	{
	    y = nxo_real_get(nxo_y);
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }
    switch (nxo_type_get(nxo_x))
    {
	case NXOT_INTEGER:
	{
	    x = (cw_nxor_t) nxo_integer_get(nxo_x);
	    break;
	}
	case NXOT_REAL:
	{
	    x = nxo_real_get(nxo_x);
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }
    nxo_real_new(nxo_y, atan2(y, x));

    nxo_stack_pop(ostack);
}
#endif

#ifdef CW_REAL
void
systemdict_atanh(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;
    cw_nxor_t real;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    switch (nxo_type_get(nxo))
    {
	case NXOT_INTEGER:
	{
	    real = (cw_nxor_t) nxo_integer_get(nxo);
	    break;
	}
	case NXOT_REAL:
	{
	    real = nxo_real_get(nxo);
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }

    if (fabs(real) > 1.0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    nxo_real_new(nxo, atanh(real));
}
#endif

void
systemdict_aup(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo, *bnxo;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    bnxo = nxo_stack_bpush(ostack);
    nxo_dup(bnxo, nxo);
    nxo_stack_pop(ostack);
}

void
systemdict_bdup(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *orig, *dup;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_BGET(orig, ostack, a_thread);
    dup = nxo_stack_push(ostack);
    nxo_dup(dup, orig);
}

void
systemdict_begin(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *dstack;
    cw_nxo_t *nxo, *dict;

    dstack = nxo_thread_dstack_get(a_thread);
    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(dict, ostack, a_thread);
    if (nxo_type_get(dict) != NXOT_DICT)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    nxo = nxo_stack_push(dstack);
    nxo_dup(nxo, dict);
    nxo_stack_pop(ostack);
}

static void
systemdict_p_bind(cw_nxo_t *a_proc, cw_nxo_t *a_thread)
{
    cw_nxo_t *tstack;
    cw_nxo_t *el, *val;
    cw_uint32_t i, count;
    cw_nxot_t type;
    cw_nxoa_t attr;

    tstack = nxo_thread_tstack_get(a_thread);

    val = nxo_stack_push(tstack);
    el = nxo_stack_push(tstack);

    nxo_l_array_bound_set(a_proc);

    for (i = 0, count = nxo_array_len_get(a_proc); i < count; i++)
    {
	nxo_array_el_get(a_proc, i, el);
	attr = nxo_attr_get(el);
	if (attr == NXOA_LITERAL)
	{
	    continue;
	}

	switch (nxo_type_get(el))
	{
	    case NXOT_ARRAY:
	    {
		if (nxo_l_array_bound_get(el) == FALSE)
		{
		    systemdict_p_bind(el, a_thread);
		}
		break;
	    }
	    case NXOT_NAME:
	    {
		if (attr == NXOA_EVALUABLE)
		{
		    /* Do not bind evaluable names. */
		    continue;
		}

		if (nxo_thread_dstack_search(a_thread, el, val) == FALSE)
		{
		    type = nxo_type_get(val);

		    /* Bind under any of the following conditions:
		     *
		     * 1) Literal object.
		     *
		     * 2) Operator.
		     *
		     * 3) Handle.
		     *
		     * 4) Array.  (Set attribute to evaluable.) */
		    if (nxo_attr_get(val) == NXOA_LITERAL
			|| type == NXOT_OPERATOR
#ifdef CW_HANDLE
			|| type == NXOT_HANDLE
#endif
			)
		    {
			nxo_array_el_set(a_proc, val, i);
		    }
		    else if (type == NXOT_ARRAY)
		    {
			nxo_attr_set(val, NXOA_EVALUABLE);
			nxo_array_el_set(a_proc, val, i);
		    }
		}
	    }
	    default:
	    {
		break;
	    }
	}
    }

    nxo_stack_npop(tstack, 2);
}

void
systemdict_bind(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *array;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(array, ostack, a_thread);
    if (nxo_type_get(array) != NXOT_ARRAY)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    if (nxo_l_array_bound_get(array) == FALSE)
    {
	systemdict_p_bind(array, a_thread);
    }
}

#ifdef CW_SOCKET
void
systemdict_bindsocket(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack, *sock, *addr, *taddr;
    cw_uint32_t npop;
    sa_family_t family;
    int sockport, error;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    NXO_STACK_GET(addr, ostack, a_thread);
    if (nxo_type_get(addr) == NXOT_INTEGER)
    {
	sockport = nxo_integer_get(addr);
	NXO_STACK_DOWN_GET(addr, ostack, a_thread, addr);
	npop = 3;
    }
    else
    {
	/* Let the OS choose a port. */
	sockport = 0;
	npop = 2;
    }
    NXO_STACK_DOWN_GET(sock, ostack, a_thread, addr);
    if (nxo_type_get(sock) != NXOT_FILE || nxo_type_get(addr) != NXOT_STRING)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    /* Get the socket family. */
    if (systemdict_p_sock_family(a_thread, nxo_file_fd_get(sock), FALSE,
				 &family))
    {
	return;
    }

    /* Create a '\0'-terminated copy of addr. */
    taddr = nxo_stack_push(tstack);
    nxo_string_cstring(taddr, addr, a_thread);

    switch (family)
    {
	case AF_INET:
	{
	    struct sockaddr_in sockaddr;

	    /* Begin initialization of sockaddr. */
	    memset(&sockaddr, 0, sizeof(struct sockaddr_in));
	    sockaddr.sin_family = family;
	    sockaddr.sin_port = htons(sockport);

	    error = inet_pton(family, nxo_string_get(taddr),
			      &sockaddr.sin_addr);
	    if (error < 0)
	    {
		nxo_thread_nerror(a_thread, NXN_argcheck);
		goto ERROR;
	    }
	    else if (error == 0)
	    {
		struct hostent *ent;
		struct in_addr *iaddr;

		/* Not a dotted number IP address.  Try it as a hostname. */
#ifdef CW_THREADS
		mtx_lock(&cw_g_gethostbyname_mtx);
#endif
		ent = gethostbyname(nxo_string_get(taddr));
		if (ent == NULL)
		{
#ifdef CW_THREADS
		    mtx_unlock(&cw_g_gethostbyname_mtx);
#endif
		    nxo_thread_nerror(a_thread, NXN_argcheck);
		    goto ERROR;
		}

		iaddr = (struct in_addr *) ent->h_addr_list[0];

		sockaddr.sin_addr = *iaddr;
	    }

#ifdef CW_THREADS
	    mtx_unlock(&cw_g_gethostbyname_mtx);
#endif

	    /* Bind. */
	    error = bind(nxo_file_fd_get(sock),
			 (struct sockaddr *) &sockaddr,
			 sizeof(sockaddr));

	    break;
	}
	case AF_LOCAL:
	{
	    struct sockaddr_un sockaddr;

	    if (npop == 3)
	    {
		/* Port shouldn't be specified for a Unix domain socket. */
		nxo_thread_nerror(a_thread, NXN_typecheck);
		goto ERROR;
	    }

	    if (nxo_string_len_get(taddr) > sizeof(sockaddr.sun_path))
	    {
		/* Not enough room for path. */
		nxo_thread_nerror(a_thread, NXN_rangecheck);
		goto ERROR;
	    }

	    /* Initialize sockaddr. */
	    memset(&sockaddr, 0, sizeof(struct sockaddr_un));
	    sockaddr.sun_family = family;
	    memcpy(sockaddr.sun_path, nxo_string_get(taddr),
		   nxo_string_len_get(taddr));

	    /* Bind. */
	    error = bind(nxo_file_fd_get(sock),
			 (struct sockaddr *) &sockaddr,
			 sizeof(sockaddr));

	    break;
	}
	default:
	{
	    cw_not_reached();
	}
    }

    /* Check for bind() error. */
    if (error == -1)
    {
	switch (errno)
	{
	    case EADDRINUSE:
	    case EADDRNOTAVAIL:
	    {
		nxo_thread_nerror(a_thread, NXN_neterror);
		goto ERROR;
	    }
	    case EACCES:
	    case EINVAL:
	    case EIO:
	    case EISDIR:
	    case ELOOP:
	    case ENAMETOOLONG:
	    case ENOENT:
	    case EROFS:
	    case ENOTDIR:
	    case ENOTSOCK:
	    {
		nxo_thread_nerror(a_thread, NXN_invalidfileaccess);
		goto ERROR;
	    }
	    case EAGAIN:
	    case EBADF:
	    case EFAULT:
	    case ENOMEM:
	    default:
	    {
		nxo_thread_nerror(a_thread, NXN_unregistered);
		goto ERROR;
	    }
	}
    }

    nxo_stack_npop(ostack, npop);
    ERROR:
    nxo_stack_pop(tstack);
}
#endif

void
systemdict_bpop(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_BPOP(ostack, a_thread);
}

#ifdef CW_THREADS
void
systemdict_broadcast(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *condition;
	
    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(condition, ostack, a_thread);
    if (nxo_type_get(condition) != NXOT_CONDITION)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    nxo_condition_broadcast(condition);

    nxo_stack_pop(ostack);
}
#endif

void
systemdict_bytesavailable(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *file;
    cw_uint32_t bytes;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(file, ostack, a_thread);
	
    if (nxo_type_get(file) != NXOT_FILE)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    bytes = nxo_file_buffer_count(file);
    nxo_integer_new(file, bytes);
}

void
systemdict_cat(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *a, *b, *r;
    cw_uint32_t i, len_a, len_b;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(b, ostack, a_thread);
    NXO_STACK_DOWN_GET(a, ostack, a_thread, b);
    if (nxo_type_get(a) != nxo_type_get(b)
	|| (nxo_type_get(a) != NXOT_ARRAY
	    && nxo_type_get(a) != NXOT_STACK
	    && nxo_type_get(a) != NXOT_STRING))
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    r = nxo_stack_under_push(ostack, a);

    switch (nxo_type_get(a))
    {
	case NXOT_ARRAY:
	{
	    cw_nxo_t *tstack, *tnxo;
		
	    tstack = nxo_thread_tstack_get(a_thread);
	    tnxo = nxo_stack_push(tstack);

	    len_a = nxo_array_len_get(a);
	    len_b = nxo_array_len_get(b);

	    nxo_array_new(r, nxo_thread_currentlocking(a_thread),
			  len_a + len_b);

	    for (i = 0; i < len_a; i++)
	    {
		nxo_array_el_get(a, i, tnxo);
		nxo_array_el_set(r, tnxo, i);
	    }
	    for (i = 0; i < len_b; i++)
	    {
		nxo_array_el_get(b, i, tnxo);
		nxo_array_el_set(r, tnxo, i + len_a);
	    }

	    nxo_stack_pop(tstack);

	    break;
	}
	case NXOT_STACK:
	{
	    cw_nxo_t *fr, *to;

	    nxo_stack_new(r, nxo_thread_currentlocking(a_thread));

	    for (fr = nxo_stack_get(b);
		 fr != NULL;
		 fr = nxo_stack_down_get(b, fr))
	    {
		to = nxo_stack_bpush(r);
		nxo_dup(to, fr);
	    }
	    for (fr = nxo_stack_get(a);
		 fr != NULL;
		 fr = nxo_stack_down_get(a, fr))
	    {
		to = nxo_stack_bpush(r);
		nxo_dup(to, fr);
	    }

	    break;
	}
	case NXOT_STRING:
	{
	    len_a = nxo_string_len_get(a);
	    len_b = nxo_string_len_get(b);

	    nxo_string_new(r, nxo_thread_currentlocking(a_thread),
			   len_a + len_b);

	    nxo_string_lock(a);
	    nxo_string_set(r, 0, nxo_string_get(a), len_a);
	    nxo_string_unlock(a);

	    nxo_string_lock(b);
	    nxo_string_set(r, len_a, nxo_string_get(b), len_b);
	    nxo_string_unlock(b);

	    break;
	}
	default:
	{
	    cw_not_reached();
	}
    }

    nxo_stack_npop(ostack, 2);
}

#ifdef CW_POSIX
void
systemdict_cd(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack, *path, *tpath;
    int error;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    NXO_STACK_GET(path, ostack, a_thread);
    if (nxo_type_get(path) != NXOT_STRING)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    /* Create a copy of the path with an extra byte to store a '\0'
     * terminator. */
    tpath = nxo_stack_push(tstack);
    nxo_string_cstring(tpath, path, a_thread);

    error = chdir(nxo_string_get(tpath));
    if (error == -1)
    {
	nxo_string_unlock(tpath);
	switch (errno)
	{
	    case EIO:
	    {
		nxo_thread_nerror(a_thread, NXN_ioerror);
		break;
	    }
	    default:
	    {
		nxo_thread_nerror(a_thread, NXN_invalidaccess);
	    }
	}
	goto ERROR;
    }

    nxo_stack_pop(ostack);

    ERROR:
    nxo_stack_pop(tstack);
}
#endif

#ifdef CW_REAL
void
systemdict_ceiling(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    switch (nxo_type_get(nxo))
    {
	case NXOT_INTEGER:
	{
	    break;
	}
	case NXOT_REAL:
	{
	    nxo_integer_new(nxo, (cw_nxoi_t) ceil(nxo_real_get(nxo)));
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }
}
#endif

#ifdef CW_POSIX
void
systemdict_chmod(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *file, *mode;
    int error;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(mode, ostack, a_thread);
    NXO_STACK_DOWN_GET(file, ostack, a_thread, mode);
    if ((nxo_type_get(mode) != NXOT_INTEGER)
	|| (nxo_type_get(file) != NXOT_FILE
	    && nxo_type_get(file) != NXOT_STRING))
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    if (nxo_integer_get(mode) < 0 || nxo_integer_get(mode) > 0xfff)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    if (nxo_type_get(file) == NXOT_FILE)
    {
	int fd;

	fd = nxo_file_fd_get(file);
	if (fd < 0)
	{
	    nxo_thread_nerror(a_thread, NXN_invalidfileaccess);
	    return;
	}

	error = fchmod(fd, nxo_integer_get(mode));
    }
    else
    {
	cw_nxo_t *tstack, *tfile;

	tstack = nxo_thread_tstack_get(a_thread);

	/* Create a copy of file with an extra byte to store a '\0'
	 * terminator. */
	tfile = nxo_stack_push(tstack);
	nxo_string_cstring(tfile, file, a_thread);

	error = chmod(nxo_string_get(tfile), nxo_integer_get(mode));

	nxo_stack_pop(tstack);
    }

    if (error == -1)
    {
	switch (errno)
	{
	    case EIO:
	    case EROFS:
	    {
		nxo_thread_nerror(a_thread, NXN_ioerror);
		break;
	    }
	    case EACCES:
#ifdef EFTYPE
	    case EFTYPE:
#endif
	    case EINVAL:
	    case ELOOP:
	    case ENAMETOOLONG:
	    case ENOENT:
	    case ENOTDIR:
	    case EPERM:
	    {
		nxo_thread_nerror(a_thread, NXN_invalidfileaccess);
		break;
	    }
	    case EBADF:
	    case EFAULT:
	    default:
	    {
		nxo_thread_nerror(a_thread, NXN_unregistered);
	    }
	}
	return;
    }

    nxo_stack_npop(ostack, 2);
}
#endif

#ifdef CW_POSIX
void
systemdict_chown(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *file, *uid, *gid;
    int error;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(gid, ostack, a_thread);
    NXO_STACK_DOWN_GET(uid, ostack, a_thread, gid);
    NXO_STACK_DOWN_GET(file, ostack, a_thread, uid);
    if ((nxo_type_get(file) != NXOT_FILE && nxo_type_get(file) != NXOT_STRING)
	|| nxo_type_get(gid) != NXOT_INTEGER
	|| nxo_type_get(uid) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    if (nxo_integer_get(uid) < 0 || nxo_integer_get(gid) < 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    if (nxo_type_get(file) == NXOT_FILE)
    {
	int fd;

	fd = nxo_file_fd_get(file);
	if (fd < 0)
	{
	    nxo_thread_nerror(a_thread, NXN_invalidfileaccess);
	    return;
	}

	error = fchown(fd, nxo_integer_get(uid), nxo_integer_get(gid));
    }
    else
    {
	cw_nxo_t *tstack, *tfile;

	tstack = nxo_thread_tstack_get(a_thread);

	/* Create a copy of file with an extra byte to store a '\0'
	 * terminator. */
	tfile = nxo_stack_push(tstack);
	nxo_string_cstring(tfile, file, a_thread);

	error = chown(nxo_string_get(tfile), nxo_integer_get(uid),
		      nxo_integer_get(gid));

	nxo_stack_pop(tstack);
    }

    if (error == -1)
    {
	switch (errno)
	{
	    case EIO:
	    case EROFS:
	    {
		nxo_thread_nerror(a_thread, NXN_ioerror);
		break;
	    }
	    case EACCES:
	    case EINVAL:
	    case ELOOP:
	    case ENAMETOOLONG:
	    case ENOENT:
	    case ENOTDIR:
	    case EPERM:
	    {
		nxo_thread_nerror(a_thread, NXN_invalidfileaccess);
		break;
	    }
	    case EBADF:
	    case EFAULT:
	    default:
	    {
		nxo_thread_nerror(a_thread, NXN_unregistered);
	    }
	}
	return;
    }

    nxo_stack_npop(ostack, 3);
}
#endif

#ifdef CW_POSIX
void
systemdict_chroot(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack, *path, *tpath;
    int error;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    NXO_STACK_GET(path, ostack, a_thread);
    if (nxo_type_get(path) != NXOT_STRING)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    /* Create a copy of the path with an extra byte to store a '\0'
     * terminator. */
    tpath = nxo_stack_push(tstack);
    nxo_string_cstring(tpath, path, a_thread);

    error = chroot(nxo_string_get(tpath));
    if (error == -1)
    {
	nxo_string_unlock(tpath);
	switch (errno)
	{
	    case EIO:
	    {
		nxo_thread_nerror(a_thread, NXN_ioerror);
		break;
	    }
	    default:
	    {
		nxo_thread_nerror(a_thread, NXN_invalidaccess);
	    }
	}
	goto ERROR;
    }

    nxo_stack_pop(ostack);

    ERROR:
    nxo_stack_pop(tstack);
}
#endif

void
systemdict_clear(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_uint32_t count;

    ostack = nxo_thread_ostack_get(a_thread);
    count = nxo_stack_count(ostack);
    if (count > 0)
    {
	nxo_stack_npop(ostack, count);
    }
}

void
systemdict_cleartomark(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;
    cw_uint32_t i, depth;

    ostack = nxo_thread_ostack_get(a_thread);

    for (i = 0, depth = nxo_stack_count(ostack), nxo = NULL; i < depth; i++)
    {
	nxo = nxo_stack_down_get(ostack, nxo);
	if (nxo_type_get(nxo) == NXOT_MARK)
	{
	    break;
	}
    }
    if (i == depth)
    {
	nxo_thread_nerror(a_thread, NXN_unmatchedmark);
	return;
    }

    nxo_stack_npop(ostack, i + 1);
}

#ifdef CW_POSIX
void
systemdict_close(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;
    cw_nxn_t error;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_FILE)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    error = nxo_file_close(nxo);
    if (error)
    {
	nxo_thread_nerror(a_thread, NXN_ioerror);
	return;
    }

    nxo_stack_pop(ostack);
}
#endif

#ifdef CW_THREADS
void
systemdict_condition(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *condition;

    ostack = nxo_thread_ostack_get(a_thread);
    condition = nxo_stack_push(ostack);
    nxo_condition_new(condition);
}
#endif

#ifdef CW_SOCKET
void
systemdict_connect(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack, *sock, *addr, *taddr;
    cw_uint32_t npop;
    sa_family_t family;
    int sockport, error;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    NXO_STACK_GET(addr, ostack, a_thread);
    if (nxo_type_get(addr) == NXOT_INTEGER)
    {
	sockport = nxo_integer_get(addr);
	NXO_STACK_DOWN_GET(addr, ostack, a_thread, addr);
	npop = 3;
    }
    else
    {
	npop = 2;
    }
    NXO_STACK_DOWN_GET(sock, ostack, a_thread, addr);
    if (nxo_type_get(sock) != NXOT_FILE || nxo_type_get(addr) != NXOT_STRING)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    /* Get the socket family. */
    if (systemdict_p_sock_family(a_thread, nxo_file_fd_get(sock), FALSE,
				 &family))
    {
	return;
    }

    /* Create a '\0'-terminated copy of addr. */
    taddr = nxo_stack_push(tstack);
    nxo_string_cstring(taddr, addr, a_thread);

    switch (family)
    {
	case AF_INET:
	{
	    struct sockaddr_in sockaddr;

	    if (npop != 3)
	    {
		/* Port must be specified for an IP connection. */
		nxo_thread_nerror(a_thread, NXN_typecheck);
		goto ERROR;
	    }

	    /* Begin initialization of sockaddr. */
	    memset(&sockaddr, 0, sizeof(struct sockaddr_in));
	    sockaddr.sin_family = family;
	    sockaddr.sin_port = htons(sockport);

	    error = inet_pton(family, nxo_string_get(taddr),
			      &sockaddr.sin_addr);
	    if (error < 0)
	    {
		nxo_thread_nerror(a_thread, NXN_argcheck);
		goto ERROR;
	    }
	    else if (error == 0)
	    {
		struct hostent *ent;
		struct in_addr *iaddr;

		/* Not a dotted number IP address.  Try it as a hostname. */
#ifdef CW_THREADS
		mtx_lock(&cw_g_gethostbyname_mtx);
#endif
		ent = gethostbyname(nxo_string_get(taddr));
		if (ent == NULL)
		{
#ifdef CW_THREADS
		    mtx_unlock(&cw_g_gethostbyname_mtx);
#endif
		    nxo_thread_nerror(a_thread, NXN_argcheck);
		    goto ERROR;
		}

		iaddr = (struct in_addr *) ent->h_addr_list[0];

		sockaddr.sin_addr = *iaddr;
	    }

#ifdef CW_THREADS
	    mtx_unlock(&cw_g_gethostbyname_mtx);
#endif

	    /* Connect. */
	    error = connect(nxo_file_fd_get(sock),
			    (struct sockaddr *) &sockaddr,
			    sizeof(sockaddr));

	    break;
	}
	case AF_LOCAL:
	{
	    struct sockaddr_un sockaddr;

	    if (npop == 3)
	    {
		/* Port shouldn't be specified for a Unix domain connection. */
		nxo_thread_nerror(a_thread, NXN_typecheck);
		goto ERROR;
	    }

	    if (nxo_string_len_get(taddr) > sizeof(sockaddr.sun_path))
	    {
		/* Not enough room for path. */
		nxo_thread_nerror(a_thread, NXN_rangecheck);
		goto ERROR;
	    }

	    /* Initialize sockaddr. */
	    memset(&sockaddr, 0, sizeof(struct sockaddr_un));
	    sockaddr.sun_family = family;
	    memcpy(sockaddr.sun_path, nxo_string_get(taddr),
		   nxo_string_len_get(taddr));

	    /* Connect. */
	    error = connect(nxo_file_fd_get(sock),
			    (struct sockaddr *) &sockaddr,
			    sizeof(sockaddr));

	    break;
	}
	default:
	{
	    cw_not_reached();
	}
    }

    /* Check for connect() error. */
    if (error == -1)
    {
	switch (errno)
	{
	    case EADDRINUSE:
	    case ECONNREFUSED:
	    case ETIMEDOUT:
	    case ENETUNREACH:
	    {
		nxo_thread_nerror(a_thread, NXN_neterror);
		goto ERROR;
	    }
	    case EINPROGRESS:
	    case EALREADY:
	    {
		nxo_thread_nerror(a_thread, NXN_ioerror);
		goto ERROR;
	    }
	    case EAFNOSUPPORT:
	    {
		nxo_thread_nerror(a_thread, NXN_argcheck);
		goto ERROR;
	    }
	    case EACCES:
	    case EISCONN:
	    case ENOTSOCK:
	    case EPERM:
	    {
		nxo_thread_nerror(a_thread, NXN_invalidfileaccess);
		goto ERROR;
	    }
	    case EAGAIN:
	    case EBADF:
	    case EFAULT:
	    default:
	    {
		nxo_thread_nerror(a_thread, NXN_unregistered);
		goto ERROR;
	    }
	}
    }

    nxo_stack_npop(ostack, npop);
    ERROR:
    nxo_stack_pop(tstack);
}
#endif

void
systemdict_copy(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);

    switch (nxo_type_get(nxo))
    {
	case NXOT_ARRAY:
	{
	    cw_nxo_t *orig;

	    NXO_STACK_DOWN_GET(orig, ostack, a_thread, nxo);
	    if (nxo_type_get(orig) != NXOT_ARRAY)
	    {
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	    }
	    if (nxo_array_len_get(nxo) < nxo_array_len_get(orig))
	    {
		nxo_thread_nerror(a_thread, NXN_rangecheck);
		return;
	    }

	    nxo_array_copy(nxo, orig);

	    nxo_stack_remove(ostack, orig);
	    break;
	}
	case NXOT_DICT:
	{
	    cw_nxo_t *orig;

	    NXO_STACK_DOWN_GET(orig, ostack, a_thread, nxo);
	    if (nxo_type_get(orig) != NXOT_DICT)
	    {
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	    }

	    nxo_dict_copy(nxo, orig);
		
	    nxo_stack_remove(ostack, orig);
	    break;
	}
	case NXOT_STACK:
	{
	    cw_nxo_t *orig;

	    NXO_STACK_DOWN_GET(orig, ostack, a_thread, nxo);
	    if (nxo_type_get(orig) != NXOT_STACK)
	    {
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	    }

	    nxo_stack_copy(nxo, orig);

	    nxo_stack_remove(ostack, orig);
	    break;
	}
	case NXOT_STRING:
	{
	    cw_nxo_t *orig;

	    NXO_STACK_DOWN_GET(orig, ostack, a_thread, nxo);
	    if (nxo_type_get(orig) != NXOT_STRING)
	    {
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	    }
	    if (nxo_string_len_get(nxo) < nxo_string_len_get(orig))
	    {
		nxo_thread_nerror(a_thread, NXN_rangecheck);
		return;
	    }

	    nxo_string_copy(nxo, orig);

	    nxo_stack_remove(ostack, orig);
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }
}

#ifdef CW_REAL
void
systemdict_cos(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;
    cw_nxor_t real;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    switch (nxo_type_get(nxo))
    {
	case NXOT_INTEGER:
	{
	    real = (cw_nxor_t) nxo_integer_get(nxo);
	    break;
	}
	case NXOT_REAL:
	{
	    real = nxo_real_get(nxo);
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }
    nxo_real_new(nxo, cos(real));
}
#endif

#ifdef CW_REAL
void
systemdict_cosh(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;
    cw_nxor_t real;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    switch (nxo_type_get(nxo))
    {
	case NXOT_INTEGER:
	{
	    real = (cw_nxor_t) nxo_integer_get(nxo);
	    break;
	}
	case NXOT_REAL:
	{
	    real = nxo_real_get(nxo);
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }
    nxo_real_new(nxo, cosh(real));
}
#endif

void
systemdict_count(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;

    ostack = nxo_thread_ostack_get(a_thread);

    nxo = nxo_stack_push(ostack);
    nxo_integer_new(nxo, nxo_stack_count(ostack) - 1);
}

void
systemdict_countdstack(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *dstack;
    cw_nxo_t *nxo;

    dstack = nxo_thread_dstack_get(a_thread);
    ostack = nxo_thread_ostack_get(a_thread);

    nxo = nxo_stack_push(ostack);
    nxo_integer_new(nxo, nxo_stack_count(dstack));
}

void
systemdict_countestack(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *estack;
    cw_nxo_t *nxo;

    estack = nxo_thread_estack_get(a_thread);
    ostack = nxo_thread_ostack_get(a_thread);

    nxo = nxo_stack_push(ostack);
    nxo_integer_new(nxo, nxo_stack_count(estack));
}

void
systemdict_counttomark(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;
    cw_uint32_t i, depth;

    ostack = nxo_thread_ostack_get(a_thread);

    for (i = 0, depth = nxo_stack_count(ostack), nxo = NULL; i < depth; i++)
    {
	nxo = nxo_stack_down_get(ostack, nxo);
	if (nxo_type_get(nxo) == NXOT_MARK)
	{
	    break;
	}
    }
    if (i == depth)
    {
	nxo_thread_nerror(a_thread, NXN_unmatchedmark);
	return;
    }

    nxo = nxo_stack_push(ostack);
    nxo_integer_new(nxo, i);
}

void
systemdict_currentdict(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *dstack;
    cw_nxo_t *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    dstack = nxo_thread_dstack_get(a_thread);

    nxo = nxo_stack_push(ostack);
    nxo_dup(nxo, nxo_stack_get(dstack));
}

#ifdef CW_THREADS
void
systemdict_currentlocking(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    nxo = nxo_stack_push(ostack);
    nxo_boolean_new(nxo, nxo_thread_currentlocking(a_thread));
}
#endif

#ifdef CW_REAL
void
systemdict_cvds(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *real, *precision;
    char *result;
    cw_sint32_t len;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(precision, ostack, a_thread);
    NXO_STACK_DOWN_GET(real, ostack, a_thread, precision);
    if (nxo_type_get(precision) != NXOT_INTEGER
	|| nxo_type_get(real) != NXOT_REAL)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    if ((int) nxo_integer_get(precision) < 0)
    {
	len = asprintf(&result, "%.*g", -(int)nxo_integer_get(precision),
		       nxo_real_get(real));
    }
    else
    {
	len = asprintf(&result, "%.*f", (int) nxo_integer_get(precision),
		       nxo_real_get(real));
    }
    if (len == -1)
    {
	xep_throw(CW_ONYXX_OOM);
    }

    nxo_string_new(real, nxo_thread_currentlocking(a_thread), len);
    nxo_string_lock(real);
    nxo_string_set(real, 0, result, len);
    nxo_string_unlock(real);
    free(result);

    nxo_stack_pop(ostack);
}
#endif

void
systemdict_cve(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    nxo_attr_set(nxo, NXOA_EVALUABLE);
}

#ifdef CW_REAL
void
systemdict_cves(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *real, *precision;
    char *result;
    cw_sint32_t len;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(precision, ostack, a_thread);
    NXO_STACK_DOWN_GET(real, ostack, a_thread, precision);
    if (nxo_type_get(precision) != NXOT_INTEGER
	|| nxo_type_get(real) != NXOT_REAL)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    len = asprintf(&result, "%.*e", (int) nxo_integer_get(precision),
		   nxo_real_get(real));
    if (len == -1)
    {
	xep_throw(CW_ONYXX_OOM);
    }

    nxo_string_new(real, nxo_thread_currentlocking(a_thread), len);
    nxo_string_lock(real);
    nxo_string_set(real, 0, result, len);
    nxo_string_unlock(real);
    free(result);

    nxo_stack_pop(ostack);
}
#endif

void
systemdict_cvlit(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    nxo_attr_set(nxo, NXOA_LITERAL);
}

void
systemdict_cvn(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack;
    cw_nxo_t *nxo, *tnxo;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);

    if (nxo_type_get(nxo) != NXOT_STRING)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    tnxo = nxo_stack_push(tstack);
    nxo_dup(tnxo, nxo);

    nxo_name_new(nxo, nxo_string_get(tnxo), nxo_string_len_get(tnxo), FALSE);
    nxo_attr_set(nxo, nxo_attr_get(tnxo));

    nxo_stack_pop(tstack);
}

static cw_uint32_t
systemdict_p_integer_render(cw_nxoi_t a_integer, cw_uint32_t a_base,
			    cw_uint8_t *r_buf)
{
    cw_uint32_t retval, i;
    cw_uint64_t t = (cw_uint64_t) a_integer;
    cw_bool_t negative;
    cw_uint8_t *syms = "0123456789abcdefghijklmnopqrstuvwxyz";
    /* Since we're printing a signed integer, the most we ever need is 1 sign
     * byte and 63 digit bytes. */
    cw_uint8_t *result, s_result[65] =
	"0000000000000000000000000000000000000000000000000000000000000000";

    /* Leave space for a leading sign. */
    result = &s_result[1];

    /* Print the sign if a negative number. */
    if ((t & (((cw_uint64_t) 1) << 63)) != 0)
    {
	/* Convert two's complement to positive.  We have to use an unsigned
	 * integer and do all this manually in case the integer is the minimum
	 * negative value, which is not representable as a negated (to positive)
	 * signed integer. */
	t ^= 0xffffffffffffffffLL;
	t++;

	negative = TRUE;
    }
    else
    {
	negative = FALSE;
    }

    /* Render. */
    if (t == 0)
    {
	result += 62;
    }
    else
    {
	if (a_base == 16)
	{
	    for (i = 62; t != 0; i--)
	    {
		result[i] = syms[t & 0xf];
		t >>= 4;
	    }
	}
	else
	{
	    for (i = 62; t != 0; i--)
	    {
		result[i] = syms[t % a_base];
		t /= a_base;
	    }
	}
	result += i + 1;
    }

    /* If negative, show the sign. */
    if (negative)
    {
	result--;
	result[0] = '-';
    }

    /* Calculate the length an copy to the return buffer. */
    retval = &s_result[64] - result;
    memcpy(r_buf, result, retval);

    return retval;
}

void
systemdict_cvrs(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *num, *radix;
    cw_uint64_t val;
    cw_uint32_t rlen, base;
    cw_uint8_t *str;
    cw_uint8_t result[66]; /* Sign, 64 bits, terminator. */

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(radix, ostack, a_thread);
    NXO_STACK_DOWN_GET(num, ostack, a_thread, radix);
    if ((nxo_type_get(num) != NXOT_INTEGER)
	|| (nxo_type_get(radix) != NXOT_INTEGER))
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    base = nxo_integer_get(radix);
    if (base < 2 || base > 36)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }
    val = nxo_integer_get(num);

    rlen = systemdict_p_integer_render(val, base, result);
    cw_assert(rlen <= 65);

    nxo_string_new(num, nxo_thread_currentlocking(a_thread), rlen);

    str = nxo_string_get(num);
    nxo_string_lock(num);
    memcpy(str, result, rlen);
    nxo_string_unlock(num);

    nxo_stack_pop(ostack);
}

void
systemdict_cvs(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;
	
    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);

    switch (nxo_type_get(nxo))
    {
	case NXOT_BOOLEAN:
	{
	    cw_onyx_code(a_thread, "{`true'} {`false'} ifelse");
	    break;
	}
	case NXOT_INTEGER:
	{
	    cw_uint8_t result[21];
	    cw_sint32_t len;

	    len = systemdict_p_integer_render(nxo_integer_get(nxo), 10,
					      result);

	    nxo_string_new(nxo, nxo_thread_currentlocking(a_thread), len);
	    nxo_string_lock(nxo);
	    nxo_string_set(nxo, 0, result, len);
	    nxo_string_unlock(nxo);
	    break;
	}
	case NXOT_NAME:
	{
	    cw_nxo_t *tstack;
	    cw_nxo_t *tnxo;

	    tstack = nxo_thread_tstack_get(a_thread);
	    tnxo = nxo_stack_push(tstack);
	    nxo_dup(tnxo, nxo);

	    nxo_string_new(nxo, nxo_thread_currentlocking(a_thread),
			   nxo_name_len_get(tnxo));
	    nxo_string_lock(nxo);
	    nxo_string_set(nxo, 0, nxo_name_str_get(tnxo),
			   nxo_name_len_get(tnxo));
	    nxo_string_unlock(nxo);

	    nxo_stack_pop(tstack);
	    break;
	}
	case NXOT_OPERATOR:
	{
	    cw_nxn_t nxn;

	    nxn = nxo_l_operator_nxn_get(nxo);
	    if (nxn == NXN_ZERO)
	    {
		cw_onyx_code(a_thread, "pop `-operator-'");
	    }
	    else
	    {
		cw_nxo_t *tstack;
		cw_nxo_t *tnxo;

		cw_assert(nxn <= NXN_LAST);
		tstack = nxo_thread_tstack_get(a_thread);
		tnxo = nxo_stack_push(tstack);
		nxo_dup(tnxo, nxo);

		nxo_string_new(nxo, nxo_thread_currentlocking(a_thread),
			       nxn_len(nxn));
		nxo_string_lock(nxo);
		nxo_string_set(nxo, 0, nxn_str(nxn), nxn_len(nxn));
		nxo_string_unlock(nxo);

		nxo_stack_pop(tstack);
	    }
	    break;
	}
#ifdef CW_REAL
	case NXOT_REAL:
	{
	    cw_uint8_t result[15]; /* "-9.999999e+307\0" */
	    cw_sint32_t len;

	    len = snprintf(result, sizeof(result), "%e", nxo_real_get(nxo));

	    nxo_string_new(nxo, nxo_thread_currentlocking(a_thread), len);
	    nxo_string_lock(nxo);
	    nxo_string_set(nxo, 0, result, len);
	    nxo_string_unlock(nxo);
	    break;
	}
#endif
	case NXOT_STRING:
	{
	    cw_nxo_t *tstack, *tnxo;
	    cw_uint32_t i, j, len, newlen;
	    cw_uint8_t *str, *newstr;
	    cw_uint8_t syms[] = "0123456789abcdef";

	    /* The source is already a string, but here we convert non-printing
	     * characters. */

	    tstack = nxo_thread_tstack_get(a_thread);
	    tnxo = nxo_stack_push(tstack);
	    nxo_dup(tnxo, nxo);

	    str = nxo_string_get(tnxo);
	    len = nxo_string_len_get(tnxo);

	    /* The source string must not change between the first and second
	     * passes.  The destination string need not be locked, since no
	     * other threads have a possible way of accessing it yet. */

	    /* Calculate the length of the new string. */
	    for (i = 0, newlen = 2; i < len; i++)
	    {
		switch (str[i])
		{
		    case '\0': case '\x1b': /* \e */
		    case '\a': case '\b': case '\f': case '\n': case '\r':
		    case '\t': case '\\': case '`': case '\'':
		    {
			newlen += 2;
			break;
		    }
		    case '\x01': case '\x02': case '\x03': case '\x04':
		    case '\x05': case '\x06':
		    /* case '\x07': \a */
		    /* case '\x08': \b */
		    /* case '\x09': \t */
		    /* case '\x0a': \n */
		    case '\x0b':
		    /* case '\x0c': \f */
		    /* case '\x0d': \r */
		    case '\x0e': case '\x0f': case '\x10': case '\x11':
		    case '\x12': case '\x13': case '\x14': case '\x15':
		    case '\x16': case '\x17': case '\x18': case '\x19':
		    case '\x1a':
		    {
			newlen += 3;
			break;
		    }
		    default:
		    {
			if (isprint(str[i]))
			{
			    newlen++;
			}
			else
			{
			    newlen += 4;
			}
			break;
		    }
		}
	    }

	    /* Create new string. */
	    nxo_string_new(nxo, nxo_thread_currentlocking(a_thread), newlen);
	    newstr = nxo_string_get(nxo);

	    /* Convert old string to new string. */
	    newstr[0] = '`';
	    newstr[newlen - 1] = '\'';
	    for (i = 0, j = 1; i < len; i++)
	    {
		switch (str[i])
		{
		    case '\0':
		    {
			newstr[j] = '\\';
			newstr[j + 1] = '0';
			j += 2;
			break;
		    }
		    case '\x1b':
		    {
			newstr[j] = '\\';
			newstr[j + 1] = 'e';
			j += 2;
			break;
		    }
		    case '\a':
		    {
			newstr[j] = '\\';
			newstr[j + 1] = 'a';
			j += 2;
			break;
		    }
		    case '\b':
		    {
			newstr[j] = '\\';
			newstr[j + 1] = 'b';
			j += 2;
			break;
		    }
		    case '\f':
		    {
			newstr[j] = '\\';
			newstr[j + 1] = 'f';
			j += 2;
			break;
		    }
		    case '\n':
		    {
			newstr[j] = '\\';
			newstr[j + 1] = 'n';
			j += 2;
			break;
		    }
		    case '\r':
		    {
			newstr[j] = '\\';
			newstr[j + 1] = 'r';
			j += 2;
			break;
		    }
		    case '\t':
		    {
			newstr[j] = '\\';
			newstr[j + 1] = 't';
			j += 2;
			break;
		    }
		    case '\\':
		    {
			newstr[j] = '\\';
			newstr[j + 1] = '\\';
			j += 2;
			break;
		    }
		    case '`':
		    {
			newstr[j] = '\\';
			newstr[j + 1] = '`';
			j += 2;
			break;
		    }
		    case '\'':
		    {
			newstr[j] = '\\';
			newstr[j + 1] = '\'';
			j += 2;
			break;
		    }
		    case '\x01': case '\x02': case '\x03': case '\x04':
		    case '\x05': case '\x06':
		    /* case '\x07': \a */
		    /* case '\x08': \b */
		    /* case '\x09': \t */
		    /* case '\x0a': \n */
		    case '\x0b':
		    /* case '\x0c': \f */
		    /* case '\x0d': \r */
		    case '\x0e': case '\x0f': case '\x10': case '\x11':
		    case '\x12': case '\x13': case '\x14': case '\x15':
		    case '\x16': case '\x17': case '\x18': case '\x19':
		    case '\x1a':
		    {
			newstr[j] = '\\';
			newstr[j + 1] = 'c';
			newstr[j + 2] = str[i] - 1 + 'a';
			j += 3;
			break;
		    }
		    default:
		    {
			if (isprint(str[i]))
			{
			    newstr[j] = str[i];
			    j++;
			}
			else
			{
			    newstr[j] = '\\';
			    newstr[j + 1] = 'x';
			    newstr[j + 2] = syms[str[i] >> 4];
			    newstr[j + 3] = syms[str[i] & 0xf];
			    j += 4;
			}
			break;
		    }
		}
	    }

	    nxo_stack_pop(tstack);
	    break;
	}
	case NXOT_ARRAY:
#ifdef CW_THREADS
	case NXOT_CONDITION:
#endif
	case NXOT_DICT:
	case NXOT_FILE:
	case NXOT_FINO:
#ifdef CW_HANDLE
	case NXOT_HANDLE:
#endif
	case NXOT_MARK:
#ifdef CW_THREADS
	case NXOT_MUTEX:
#endif
	case NXOT_NULL:
	case NXOT_PMARK:
#ifdef CW_REGEX
	case NXOT_REGEX:
	case NXOT_REGSUB:
#endif
	case NXOT_STACK:
	case NXOT_THREAD:
	{
	    cw_onyx_code(a_thread, "pop `--nostringval--'");
	    break;
	}
	case NXOT_NO:
	default:
	{
	    cw_not_reached();
	}
    }
}

void
systemdict_cvx(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    nxo_attr_set(nxo, NXOA_EXECUTABLE);
}

void
systemdict_dec(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    
    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    nxo_integer_set(nxo, nxo_integer_get(nxo) - 1);
}

void
systemdict_def(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *dstack;
    cw_nxo_t *dict, *key, *val;

    ostack = nxo_thread_ostack_get(a_thread);
    dstack = nxo_thread_dstack_get(a_thread);

    dict = nxo_stack_get(dstack);
    NXO_STACK_GET(val, ostack, a_thread);
    NXO_STACK_DOWN_GET(key, ostack, a_thread, val);

    nxo_dict_def(dict, key, val);

    nxo_stack_npop(ostack, 2);
}

#ifdef CW_THREADS
void
systemdict_detach(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *thread;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(thread, ostack, a_thread);
    if (nxo_type_get(thread) != NXOT_THREAD)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    nxo_thread_detach(thread);

    nxo_stack_pop(ostack);
}
#endif

void
systemdict_dict(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *dict;

    ostack = nxo_thread_ostack_get(a_thread);

    dict = nxo_stack_push(ostack);
    nxo_dict_new(dict, nxo_thread_currentlocking(a_thread),
		 CW_SYSTEMDICT_DICT_SIZE);
}

void
systemdict_die(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *code;
    cw_nxoi_t ecode;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(code, ostack, a_thread);
    if (nxo_type_get(code) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    ecode = nxo_integer_get(code);
    if (ecode < 0 || ecode > 255)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    _exit(ecode);
}

#ifdef CW_POSIX
void
systemdict_dirforeach(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *estack, *tstack;
    cw_nxo_t *nxo, *tnxo, *path, *proc, *entry;
    cw_bool_t currentlocking, dot;
    DIR *dir;
    cw_uint32_t edepth, tdepth;
    struct dirent *entp;
#ifndef CW_HAVE_DIRENT_NAMLEN
	size_t namlen;
#endif
#ifdef HAVE_READDIR_R
    struct dirent ent;
    int error;
#endif

    ostack = nxo_thread_ostack_get(a_thread);
    estack = nxo_thread_estack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    edepth = nxo_stack_count(estack);
    tdepth = nxo_stack_count(tstack);
    currentlocking = nxo_thread_currentlocking(a_thread);

    NXO_STACK_GET(tnxo, ostack, a_thread);
    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, tnxo);
    if (nxo_type_get(nxo) != NXOT_STRING)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    /* Make a copy of the procedure to execute. */
    proc = nxo_stack_push(tstack);
    nxo_dup(proc, tnxo);

    /* Create a copy of the path with an extra byte to store a '\0'
     * terminator. */
    path = nxo_stack_push(tstack);
    nxo_string_cstring(path, nxo, a_thread);

    /* Open the directory. */
    dir = opendir(nxo_string_get(path));
    if (dir == NULL)
    {
	nxo_stack_npop(tstack, 2);
	nxo_thread_nerror(a_thread, NXN_invalidaccess);
	return;
    }

    /* Pop the path and proc off ostack before going into the loop. */
    nxo_stack_npop(ostack, 2);

    xep_begin();
    xep_try
    {
	/* Iterate through the directory. */
#ifdef HAVE_READDIR_R
	while ((error = readdir_r(dir, &ent, &entp) == 0) && entp == &ent)
#else
	errno = 0;
	while ((entp = readdir(dir)) != NULL)
#endif
	{
	    /* Ignore "." and "..". */
#ifdef CW_HAVE_DIRENT_NAMLEN
	    switch (entp->d_namlen)
#else
	    namlen = strlen(entp->d_name);
	    switch (namlen)
#endif
	    {
		case 2:
		{
		    if (entp->d_name[1] != '.')
		    {
			dot = FALSE;
			break;
		    }
		    /* Fall through. */
		}
		case 1:
		{
		    if (entp->d_name[0] != '.')
		    {
			dot = FALSE;
			break;
		    }
		    else
		    {
			dot = TRUE;
		    }
		    break;
		}
		default:
		{
		    dot = FALSE;
		}
	    }
	    if (dot == FALSE)
	    {
		/* Push a string onto ostack that represents the directory
		 * entry. */
		entry = nxo_stack_push(ostack);
#ifdef CW_HAVE_DIRENT_NAMLEN
		nxo_string_new(entry, currentlocking, entp->d_namlen);
		nxo_string_set(entry, 0, entp->d_name, entp->d_namlen);
#else
		nxo_string_new(entry, currentlocking, namlen);
		nxo_string_set(entry, 0, entp->d_name, namlen);
#endif

		/* Evaluate proc. */
		nxo = nxo_stack_push(estack);
		nxo_dup(nxo, proc);
		nxo_thread_loop(a_thread);
	    }
	}
#ifdef HAVE_READDIR_R
	if (error && entp != NULL)
#else
	if (errno != 0)
#endif
	{
	    /* The loop terminated due to an error. */
	    nxo_thread_nerror(a_thread, NXN_ioerror);
	}
    }
    xep_catch(CW_ONYXX_EXIT)
    {
	cw_nxo_t *istack;

	xep_handled();

	/* Clean up estack and istack. */
	nxo_stack_npop(estack, nxo_stack_count(estack) - edepth);
	istack = nxo_thread_istack_get(a_thread);
	nxo_stack_npop(istack, nxo_stack_count(istack) -
		       nxo_stack_count(estack));
    }
    xep_acatch
    {
	/* Close the directory. */
	closedir(dir);
    }
    xep_end();

    /* Close the directory. */
    closedir(dir);

    /* Clean up tstack. */
    nxo_stack_npop(tstack, nxo_stack_count(tstack) - tdepth);
}
#endif

#ifdef CW_REAL
void
systemdict_div(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo_a, *nxo_b;
    cw_nxor_t real_a, real_b;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo_b, ostack, a_thread);
    NXO_STACK_DOWN_GET(nxo_a, ostack, a_thread, nxo_b);
    switch (nxo_type_get(nxo_a))
    {
	case NXOT_INTEGER:
	{
	    real_a = (cw_nxor_t) nxo_integer_get(nxo_a);
	    break;
	}
	case NXOT_REAL:
	{
	    real_a = nxo_real_get(nxo_a);
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }
    switch (nxo_type_get(nxo_b))
    {
	case NXOT_INTEGER:
	{
	    real_b = (cw_nxor_t) nxo_integer_get(nxo_b);
	    break;
	}
	case NXOT_REAL:
	{
	    real_b = nxo_real_get(nxo_b);
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }

    if (real_b == 0)
    {
	nxo_thread_nerror(a_thread, NXN_undefinedresult);
	return;
    }

    nxo_real_new(nxo_a, real_a / real_b);
    nxo_stack_pop(ostack);
}
#endif

void
systemdict_dn(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;

    ostack = nxo_thread_ostack_get(a_thread);
    if (nxo_stack_count(ostack) < 3)
    {
	nxo_thread_nerror(a_thread, NXN_stackunderflow);
	return;
    }

    nxo_stack_roll(ostack, 3, -1);
}

void
systemdict_dstack(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *dstack, *stack;

    ostack = nxo_thread_ostack_get(a_thread);
    dstack = nxo_thread_dstack_get(a_thread);

    stack = nxo_stack_push(ostack);
    nxo_stack_new(stack, nxo_thread_currentlocking(a_thread));
    nxo_stack_copy(stack, dstack);
}

void
systemdict_dup(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *orig, *dup;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(orig, ostack, a_thread);
    dup = nxo_stack_push(ostack);
    nxo_dup(dup, orig);
}

void
systemdict_echeck(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
	
    if (nxo_attr_get(nxo) == NXOA_EVALUABLE)
    {
	nxo_boolean_new(nxo, TRUE);
    }
    else
    {
	nxo_boolean_new(nxo, FALSE);
    }
}

#ifdef CW_POSIX
void
systemdict_egid(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
	
    ostack = nxo_thread_ostack_get(a_thread);
    nxo = nxo_stack_push(ostack);
    nxo_integer_new(nxo, getegid());
}
#endif

void
systemdict_end(cw_nxo_t *a_thread)
{
    cw_nxo_t *dstack;

    dstack = nxo_thread_dstack_get(a_thread);

    NXO_STACK_POP(dstack, a_thread);
}

void
systemdict_eq(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo_a, *nxo_b;
    cw_sint32_t result;
    cw_bool_t eq;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(nxo_b, ostack, a_thread);
    NXO_STACK_DOWN_GET(nxo_a, ostack, a_thread, nxo_b);

    result = nxo_compare(nxo_a, nxo_b);
    if (result == 0)
    {
	eq = TRUE;
    }
    else
    {
	eq = FALSE;
    }

    nxo_boolean_new(nxo_a, eq);

    nxo_stack_pop(ostack);
}

void
systemdict_estack(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *estack, *stack;

    ostack = nxo_thread_ostack_get(a_thread);
    estack = nxo_thread_estack_get(a_thread);

    stack = nxo_stack_push(ostack);
    nxo_stack_new(stack, nxo_thread_currentlocking(a_thread));
    nxo_stack_copy(stack, estack);
}

#ifdef CW_POSIX
void
systemdict_euid(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
	
    ostack = nxo_thread_ostack_get(a_thread);
    nxo = nxo_stack_push(ostack);
    nxo_integer_new(nxo, geteuid());
}
#endif

void
systemdict_eval(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *estack;
    cw_nxo_t *orig, *new;

    ostack = nxo_thread_ostack_get(a_thread);
    estack = nxo_thread_estack_get(a_thread);

    NXO_STACK_GET(orig, ostack, a_thread);
    new = nxo_stack_push(estack);
    nxo_dup(new, orig);
    nxo_stack_pop(ostack);

    nxo_thread_loop(a_thread);
}

void
systemdict_exch(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;

    ostack = nxo_thread_ostack_get(a_thread);

    if (nxo_stack_exch(ostack))
    {
	nxo_thread_nerror(a_thread, NXN_stackunderflow);
    }
}

#ifdef CW_POSIX
static cw_bool_t
systemdict_p_exec_prepare(cw_nxo_t *a_thread, char **r_path, char ***r_argv,
			  char ***r_envp)
{
    cw_nxo_t *ostack, *tstack, *array, *el, *key, *val;
    cw_uint32_t i, slen, argc, dcount, key_len, val_len;
    char *path, **argv, **envp, *entry;
    cw_nxn_t error;

    /* This function does a bunch of memory allocation, which must be done with
     * great care, since an exception will cause us to leak all of the allocated
     * memory.  For example, a user can cause an exception in
     * nxo_thread_nerror(), so we must do all cleanup before throwing the
     * error. */

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    el = nxo_stack_push(tstack);

    array = nxo_stack_get(ostack);
    if (array == NULL)
    {
	error = NXN_stackunderflow;
	goto VALIDATION_ERROR;
    }
    if (nxo_type_get(array) != NXOT_ARRAY)
    {
	error = NXN_typecheck;
	goto VALIDATION_ERROR;
    }
    argc = nxo_array_len_get(array);
    if (argc < 1)
    {
	error = NXN_rangecheck;
	goto VALIDATION_ERROR;
    }
    for (i = 0; i < argc; i++)
    {
	nxo_array_el_get(array, i, el);
	if (nxo_type_get(el) != NXOT_STRING)
	{
	    error = NXN_typecheck;
	    goto VALIDATION_ERROR;
	}
    }

    /* Construct path. */
    nxo_array_el_get(array, 0, el);
    if (nxo_type_get(el) != NXOT_STRING)
    {
	error = NXN_typecheck;
	goto PATH_ERROR;
    }
    slen = nxo_string_len_get(el);
    path = (char *) cw_malloc(slen + 1);
    nxo_string_lock(el);
    memcpy(path, nxo_string_get(el), slen);
    nxo_string_unlock(el);
    path[slen] = '\0';

    /* Construct argv. */
    argv = (char **) cw_calloc(argc + 1, sizeof(char *));
    for (i = 0; i < argc; i++)
    {
	nxo_array_el_get(array, i, el);
	if (nxo_type_get(el) != NXOT_STRING)
	{
	    error = NXN_typecheck;
	    goto ARGV_ERROR;
	}
	slen = nxo_string_len_get(el);
	argv[i] = (char *) cw_malloc(slen + 1);
	nxo_string_lock(el);
	memcpy(argv[i], nxo_string_get(el), slen);
	nxo_string_unlock(el);
	argv[i][slen] = '\0';
    }
    argv[i] = NULL;

    /* Construct envp. */
    key = el;
    val = nxo_stack_push(tstack);

    dcount = nxo_dict_count(libonyx_envdict_get());
    envp = (char **) cw_calloc(dcount + 1, sizeof(char *));
    for (i = 0; i < dcount; i++)
    {
	/* Get key and val. */
	nxo_dict_iterate(libonyx_envdict_get(), key);
	nxo_dict_lookup(libonyx_envdict_get(), key, val);
	if (nxo_type_get(key) != NXOT_NAME || nxo_type_get(val) != NXOT_STRING)
	{
	    error = NXN_typecheck;
	    goto ENVP_ERROR;
	}

	/* Create string that looks like "<key>=<val>\0". */
	key_len = nxo_name_len_get(key);
	val_len = nxo_string_len_get(val);
	entry = (char *) cw_malloc(key_len + val_len + 2);

	memcpy(entry, nxo_name_str_get(key), key_len);
	entry[key_len] = '=';
	nxo_string_lock(val);
	memcpy(&entry[key_len + 1], nxo_string_get(val), val_len);
	nxo_string_unlock(val);
	entry[key_len + 1 + val_len] = '\0';

	envp[i] = entry;
    }
    envp[i] = NULL;

    nxo_stack_npop(tstack, 2);

    *r_path = path;
    *r_argv = argv;
    *r_envp = envp;
    return FALSE;

    ENVP_ERROR:
    nxo_stack_npop(tstack, 2);
    for (i = 0; envp[i] != NULL; i++)
    {
	cw_free(envp[i]);
    }
    cw_free(envp);
    ARGV_ERROR:
    for (i = 0; argv[i] != NULL; i++)
    {
	cw_free(argv[i]);
    }
    cw_free(argv);
    cw_free(path);
    PATH_ERROR:
    VALIDATION_ERROR:
    nxo_stack_pop(tstack);
    nxo_thread_nerror(a_thread, error);

    return TRUE;
}
void
systemdict_exec(cw_nxo_t *a_thread)
{
    char *path, **argv, **envp;

    if (systemdict_p_exec_prepare(a_thread, &path, &argv, &envp) == FALSE)
    {
	execve(path, argv, envp);
	/* If we get here, then the execve() call failed.  Get an error back to
	 * the parent. */
	_exit(1);
    }
}
#endif

void
systemdict_exit(cw_nxo_t *a_thread)
{
    xep_throw(CW_ONYXX_EXIT);
}

#ifdef CW_REAL
void
systemdict_exp(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;
    cw_nxor_t real;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    switch (nxo_type_get(nxo))
    {
	case NXOT_INTEGER:
	{
	    real = (cw_nxor_t) nxo_integer_get(nxo);
	    break;
	}
	case NXOT_REAL:
	{
	    real = nxo_real_get(nxo);
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }

    nxo_real_new(nxo, exp(real));
}
#endif

#ifdef CW_REAL
void
systemdict_floor(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    switch (nxo_type_get(nxo))
    {
	case NXOT_INTEGER:
	{
	    break;
	}
	case NXOT_REAL:
	{
	    nxo_integer_new(nxo, (cw_nxoi_t) floor(nxo_real_get(nxo)));
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }
}
#endif

void
systemdict_flush(cw_nxo_t *a_thread)
{
    cw_nxn_t error;

    error = nxo_file_buffer_flush(nxo_thread_stdout_get(a_thread));
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
    }
}

void
systemdict_flushfile(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *file;
    cw_nxn_t error;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(file, ostack, a_thread);
	
    if (nxo_type_get(file) != NXOT_FILE)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    error = nxo_file_buffer_flush(file);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
	
    nxo_stack_pop(ostack);
}

void
systemdict_for(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *estack, *tstack;
    cw_nxo_t *exec, *onxo, *enxo, *tnxo;
    cw_nxoi_t i, inc, limit, edepth, tdepth;

    ostack = nxo_thread_ostack_get(a_thread);
    estack = nxo_thread_estack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);

    NXO_STACK_GET(exec, ostack, a_thread);

    NXO_STACK_DOWN_GET(onxo, ostack, a_thread, exec);
    if (nxo_type_get(onxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    limit = nxo_integer_get(onxo);

    NXO_STACK_DOWN_GET(onxo, ostack, a_thread, onxo);
    if (nxo_type_get(onxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    inc = nxo_integer_get(onxo);

    NXO_STACK_DOWN_GET(onxo, ostack, a_thread, onxo);
    if (nxo_type_get(onxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    i = nxo_integer_get(onxo);

    /* Move the object to be executed to tstack. */
    tnxo = nxo_stack_push(tstack);
    nxo_dup(tnxo, exec);
    nxo_stack_npop(ostack, 4);

    /* Record stack depths so that we can clean up if necessary. */
    edepth = nxo_stack_count(estack);
    tdepth = nxo_stack_count(tstack);

    /* Catch an exit exception, if thrown, but do not continue executing the
     * loop. */
    xep_begin();
    xep_try
    {
	if (inc >= 0)
	{
	    for (; i <= limit; i += inc)
	    {
		/* Dup the object to execute onto the execution stack. */
		enxo = nxo_stack_push(estack);
		nxo_dup(enxo, tnxo);

		/* Push the control variable onto the data stack. */
		onxo = nxo_stack_push(ostack);
		nxo_integer_new(onxo, i);

		nxo_thread_loop(a_thread);
	    }
	}
	else
	{
	    for (; i >= limit; i += inc)
	    {
		/* Dup the object to execute onto the execution stack. */
		enxo = nxo_stack_push(estack);
		nxo_dup(enxo, tnxo);

		/* Push the control variable onto the data stack. */
		onxo = nxo_stack_push(ostack);
		nxo_integer_new(onxo, i);

		nxo_thread_loop(a_thread);
	    }
	}
    }
    xep_catch(CW_ONYXX_EXIT)
    {
	cw_nxo_t *istack;

	xep_handled();

	/* Clean up stacks. */
	nxo_stack_npop(estack, nxo_stack_count(estack) - edepth);
	istack = nxo_thread_istack_get(a_thread);
	nxo_stack_npop(istack, nxo_stack_count(istack)
		       - nxo_stack_count(estack));
	nxo_stack_npop(tstack, nxo_stack_count(tstack) - tdepth);
    }
    xep_end();

    /* An object is pushed before tdepth is stored, so we can unconditionally
     * pop it here. */
    nxo_stack_pop(tstack);
}

void
systemdict_foreach(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *estack, *tstack;
    cw_nxo_t *nxo, *what, *proc;
    cw_uint32_t edepth, tdepth;
    cw_nxoi_t i, count;

    ostack = nxo_thread_ostack_get(a_thread);
    estack = nxo_thread_estack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);

    NXO_STACK_GET(proc, ostack, a_thread);
    NXO_STACK_DOWN_GET(what, ostack, a_thread, proc);
    switch (nxo_type_get(what))
    {
	case NXOT_ARRAY:
	case NXOT_DICT:
	case NXOT_STACK:
	case NXOT_STRING:
	{
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }

    /* Record stack depths so that we can clean up if necessary. */
    edepth = nxo_stack_count(estack);
    tdepth = nxo_stack_count(tstack);

    xep_begin();
    xep_try
    {
	switch (nxo_type_get(what))
	{
	    case NXOT_ARRAY:
	    {
		cw_nxo_t *el;

		/* Move proc and array to tstack. */
		nxo = nxo_stack_push(tstack);
		nxo_dup(nxo, proc);
		proc = nxo;

		nxo = nxo_stack_push(tstack);
		nxo_dup(nxo, what);
		what = nxo;

		nxo_stack_npop(ostack, 2);

		/* Iterate through the array, push each element onto ostack, and
		 * execute proc. */
		el = nxo_stack_push(tstack);
		for (i = 0, count = nxo_array_len_get(what); i < count; i++)
		{
		    nxo_array_el_get(what, i, el);
		    nxo = nxo_stack_push(ostack);
		    nxo_dup(nxo, el);

		    nxo = nxo_stack_push(estack);
		    nxo_dup(nxo, proc);
		    nxo_thread_loop(a_thread);
		}
		break;
	    }
	    case NXOT_DICT:
	    {
		cw_nxo_t *key, *val;

		/* Move proc and dict to tstack. */
		nxo = nxo_stack_push(tstack);
		nxo_dup(nxo, proc);
		proc = nxo;

		nxo = nxo_stack_push(tstack);
		nxo_dup(nxo, what);
		what = nxo;

		nxo_stack_npop(ostack, 2);

		for (i = 0, count = nxo_dict_count(what); i < count; i++)
		{
		    /* Push key and val onto ostack. */
		    key = nxo_stack_push(ostack);
		    val = nxo_stack_push(ostack);

		    /* Get next key. */
		    nxo_dict_iterate(what, key);

		    /* Use key to get val. */
		    nxo_dict_lookup(what, key, val);

		    /* Push proc onto estack and execute it. */
		    nxo = nxo_stack_push(estack);
		    nxo_dup(nxo, proc);
		    nxo_thread_loop(a_thread);
		}
		break;
	    }
	    case NXOT_STACK: {
		cw_nxo_t *el;

		/* Move proc to tstack. */
		nxo = nxo_stack_push(tstack);
		nxo_dup(nxo, proc);
		proc = nxo;

		/* Copy the contents of stack onto tstack. */
		nxo_stack_copy(tstack, what);

		nxo_stack_npop(ostack, 2);

		/* Iterate through the stack element (sitting on tstack), push
		 * each one onto ostack, and execute proc. */
		for (el = nxo_stack_get(tstack);
		     el != proc;
		     el = nxo_stack_get(tstack))
		{
		    nxo = nxo_stack_push(ostack);
		    nxo_dup(nxo, el);
		    nxo_stack_pop(tstack);

		    nxo = nxo_stack_push(estack);
		    nxo_dup(nxo, proc);
		    nxo_thread_loop(a_thread);
		}
		break;
	    }
	    case NXOT_STRING:
	    {
		cw_uint8_t el;

		/* Move proc and array to tstack. */
		nxo = nxo_stack_push(tstack);
		nxo_dup(nxo, proc);
		proc = nxo;

		nxo = nxo_stack_push(tstack);
		nxo_dup(nxo, what);
		what = nxo;

		nxo_stack_npop(ostack, 2);

		/* Iterate through the string, push each element onto ostack,
		 * and execute proc. */
		for (i = 0, count = nxo_string_len_get(what); i < count; i++)
		{
		    nxo_string_el_get(what, i, &el);
		    nxo = nxo_stack_push(ostack);
		    nxo_integer_new(nxo, (cw_nxoi_t) el);

		    nxo = nxo_stack_push(estack);
		    nxo_dup(nxo, proc);
		    nxo_thread_loop(a_thread);
		}
		break;
	    }
	    default:
	    {
		cw_not_reached();
	    }
	}
    }
    xep_catch(CW_ONYXX_EXIT)
    {
	cw_nxo_t *istack;

	xep_handled();

	/* Clean up estack.  tstack is handled later, so don't bother cleaning
	 * it up here. */
	nxo_stack_npop(estack, nxo_stack_count(estack) - edepth);
	istack = nxo_thread_istack_get(a_thread);
	nxo_stack_npop(istack, nxo_stack_count(istack) -
		       nxo_stack_count(estack));
    }
    xep_end();

    nxo_stack_npop(tstack, nxo_stack_count(tstack) - tdepth);
}

#ifdef CW_POSIX
void
systemdict_forkexec(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;
    char *path, **argv, **envp;
    pid_t pid;
    cw_uint32_t i;

    if (systemdict_p_exec_prepare(a_thread, &path, &argv, &envp) == FALSE)
    {
	pid = fork();
	if (pid == 0)
	{
	    /* Child. */
	    execve(path, argv, envp);
	    /* If we get here, then the execve() call failed.  Get an error back
	     * to the parent. */
	    _exit(1);
	}
	else
	{
	    /* Parent. */

	    /* Clean up memory allocation. */
	    for (i = 0; envp[i] != NULL; i++)
	    {
		cw_free(envp[i]);
	    }
	    cw_free(envp);

	    for (i = 0; argv[i] != NULL; i++)
	    {
		cw_free(argv[i]);
	    }
	    cw_free(argv);

	    cw_free(path);

	    if (pid == -1)
	    {
		/* Error, related to some form of resource exhaustion. */
		nxo_thread_nerror(a_thread, NXN_limitcheck);
		return;
	    }

	    ostack = nxo_thread_ostack_get(a_thread);
	    nxo = nxo_stack_get(ostack);
	    nxo_integer_new(nxo, pid);
	}
    }
}
#endif

void
systemdict_ge(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo_a, *nxo_b;
    cw_sint32_t result;
    cw_bool_t ge;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(nxo_b, ostack, a_thread);
    NXO_STACK_DOWN_GET(nxo_a, ostack, a_thread, nxo_b);

    switch (nxo_type_get(nxo_a))
    {
	case NXOT_INTEGER:
#ifdef CW_REAL
	case NXOT_REAL:
#endif
	case NXOT_STRING:
	{
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }
    switch (nxo_type_get(nxo_b))
    {
	case NXOT_INTEGER:
#ifdef CW_REAL
	case NXOT_REAL:
#endif
	case NXOT_STRING:
	{
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }

    result = nxo_compare(nxo_a, nxo_b);
    if (result == 2)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    else if (result >= 0)
    {
	ge = TRUE;
    }
    else
    {
	ge = FALSE;
    }

    nxo_boolean_new(nxo_a, ge);

    nxo_stack_pop(ostack);
}

void
systemdict_get(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *from, *with;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(with, ostack, a_thread);
    NXO_STACK_DOWN_GET(from, ostack, a_thread, with);

    switch (nxo_type_get(from))
    {
	case NXOT_ARRAY:
	{
	    cw_nxoi_t index;

	    if (nxo_type_get(with) != NXOT_INTEGER)
	    {
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	    }
	    index = nxo_integer_get(with);

	    if (index < 0 || index >= nxo_array_len_get(from))
	    {
		nxo_thread_nerror(a_thread, NXN_rangecheck);
		return;
	    }
	    nxo_array_el_get(from, index, with);

	    nxo_stack_remove(ostack, from);
	    break;
	}
	case NXOT_DICT:
	{
	    cw_nxo_t *val;

	    val = nxo_stack_push(ostack);
	    if (nxo_dict_lookup(from, with, val))
	    {
		nxo_stack_pop(ostack);
		nxo_thread_nerror(a_thread, NXN_undefined);
		return;
	    }
	    nxo_stack_remove(ostack, with);
	    nxo_stack_remove(ostack, from);
	    break;
	}
	case NXOT_STRING:
	{
	    cw_nxoi_t index;
	    cw_uint8_t el;

	    if (nxo_type_get(with) != NXOT_INTEGER)
	    {
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	    }
	    index = nxo_integer_get(with);

	    if (index < 0 || index >= nxo_string_len_get(from))
	    {
		nxo_thread_nerror(a_thread, NXN_rangecheck);
		return;
	    }
	    nxo_string_el_get(from, index, &el);
	    nxo_integer_set(with, (cw_nxoi_t) el);

	    nxo_stack_remove(ostack, from);
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }
}

void
systemdict_getinterval(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *from, *with, *count;
    cw_nxoi_t index, len;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(count, ostack, a_thread);
    NXO_STACK_DOWN_GET(with, ostack, a_thread, count);
    NXO_STACK_DOWN_GET(from, ostack, a_thread, with);

    if ((nxo_type_get(with) != NXOT_INTEGER)
	|| (nxo_type_get(count) != NXOT_INTEGER))
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    index = nxo_integer_get(with);
    len = nxo_integer_get(count);
    if (index < 0 || len < 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    switch (nxo_type_get(from))
    {
	case NXOT_ARRAY:
	{
	    if (index + len > nxo_array_len_get(from))
	    {
		nxo_thread_nerror(a_thread, NXN_rangecheck);
		return;
	    }
	    nxo_array_subarray_new(count, from, index, len);
	    break;
	}
	case NXOT_STRING:
	{
	    if (index + len > nxo_string_len_get(from))
	    {
		nxo_thread_nerror(a_thread, NXN_rangecheck);
		return;
	    }
	    nxo_string_substring_new(count, from, index, len);
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }

    nxo_stack_remove(ostack, with);
    nxo_stack_remove(ostack, from);
}

#ifdef CW_POSIX
void
systemdict_gid(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
	
    ostack = nxo_thread_ostack_get(a_thread);
    nxo = nxo_stack_push(ostack);
    nxo_integer_new(nxo, getgid());
}
#endif

#ifdef CW_THREADS
void
systemdict_gstderr(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    nxo = nxo_stack_push(ostack);
    nxo_dup(nxo, nx_stderr_get(nxo_thread_nx_get(a_thread)));
}
#endif

#ifdef CW_THREADS
void
systemdict_gstdin(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    nxo = nxo_stack_push(ostack);
    nxo_dup(nxo, nx_stdin_get(nxo_thread_nx_get(a_thread)));
}
#endif

#ifdef CW_THREADS
void
systemdict_gstdout(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    nxo = nxo_stack_push(ostack);
    nxo_dup(nxo, nx_stdout_get(nxo_thread_nx_get(a_thread)));
}
#endif

void
systemdict_gt(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo_a, *nxo_b;
    cw_sint32_t result;
    cw_bool_t gt;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(nxo_b, ostack, a_thread);
    NXO_STACK_DOWN_GET(nxo_a, ostack, a_thread, nxo_b);

    switch (nxo_type_get(nxo_a))
    {
	case NXOT_INTEGER:
#ifdef CW_REAL
	case NXOT_REAL:
#endif
	case NXOT_STRING:
	{
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }
    switch (nxo_type_get(nxo_b))
    {
	case NXOT_INTEGER:
#ifdef CW_REAL
	case NXOT_REAL:
#endif
	case NXOT_STRING:
	{
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }

    result = nxo_compare(nxo_a, nxo_b);
    if (result == 2)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    else if (result == 1)
    {
	gt = TRUE;
    }
    else
    {
	gt = FALSE;
    }

    nxo_boolean_new(nxo_a, gt);

    nxo_stack_pop(ostack);
}

#ifdef CW_HANDLE
void
systemdict_handletag(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack, *nxo, *tnxo, *tag;
	
    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_HANDLE)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    tnxo = nxo_stack_push(tstack);
    nxo_dup(tnxo, nxo);

    tag = nxo_handle_tag_get(tnxo);
    nxo_dup(nxo, tag);

    nxo_stack_pop(tstack);
}
#endif

void
systemdict_ibdup(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo, *orig;
    cw_nxoi_t index;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    index = nxo_integer_get(nxo);
    if (index < 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }
    if (index >= nxo_stack_count(ostack) - 1)
    {
	nxo_thread_nerror(a_thread, NXN_stackunderflow);
	return;
    }

    orig = nxo_stack_nbget(ostack, index);
    nxo_dup(nxo, orig);
}

void
systemdict_ibpop(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxoi_t index;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    index = nxo_integer_get(nxo);
    if (index < 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }
    if (index >= nxo_stack_count(ostack) - 1)
    {
	nxo_thread_nerror(a_thread, NXN_stackunderflow);
	return;
    }

    nxo = nxo_stack_nbget(ostack, index);
    nxo_stack_remove(ostack, nxo);
    nxo_stack_pop(ostack);
}

void
systemdict_idiv(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *a, *b;

    ostack = nxo_thread_ostack_get(a_thread);
	
    NXO_STACK_GET(b, ostack, a_thread);
    NXO_STACK_DOWN_GET(a, ostack, a_thread, b);
    if (nxo_type_get(a) != NXOT_INTEGER || nxo_type_get(b) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    if (nxo_integer_get(b) == 0)
    {
	nxo_thread_nerror(a_thread, NXN_undefinedresult);
	return;
    }

    nxo_integer_set(a, nxo_integer_get(a) / nxo_integer_get(b));
    nxo_stack_pop(ostack);
}

void
systemdict_idup(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo, *orig;
    cw_nxoi_t index;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    index = nxo_integer_get(nxo);
    if (index < 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    NXO_STACK_NGET(orig, ostack, a_thread, index + 1);
    nxo_dup(nxo, orig);
}

void
systemdict_if(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *cond, *exec;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(exec, ostack, a_thread);
    NXO_STACK_DOWN_GET(cond, ostack, a_thread, exec);
    if (nxo_type_get(cond) != NXOT_BOOLEAN)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    if (nxo_boolean_get(cond))
    {
	cw_nxo_t *estack;
	cw_nxo_t *nxo;

	estack = nxo_thread_estack_get(a_thread);
	nxo = nxo_stack_push(estack);
	nxo_dup(nxo, exec);
	nxo_stack_npop(ostack, 2);
	nxo_thread_loop(a_thread);
    }
    else
    {
	nxo_stack_npop(ostack, 2);
    }
}

void
systemdict_ifelse(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *estack;
    cw_nxo_t *cond, *exec_if, *exec_else, *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    estack = nxo_thread_estack_get(a_thread);

    NXO_STACK_GET(exec_else, ostack, a_thread);

    NXO_STACK_DOWN_GET(exec_if, ostack, a_thread, exec_else);

    NXO_STACK_DOWN_GET(cond, ostack, a_thread, exec_if);
    if (nxo_type_get(cond) != NXOT_BOOLEAN)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    nxo = nxo_stack_push(estack);
    if (nxo_boolean_get(cond))
    {
	nxo_dup(nxo, exec_if);
    }
    else
    {
	nxo_dup(nxo, exec_else);
    }
    nxo_stack_npop(ostack, 3);
    nxo_thread_loop(a_thread);
}

void
systemdict_inc(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    
    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    nxo_integer_set(nxo, nxo_integer_get(nxo) + 1);
}

void
systemdict_iobuf(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_FILE)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    nxo_integer_new(nxo, nxo_file_buffer_size_get(nxo));
}

void
systemdict_ipop(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxoi_t index;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    index = nxo_integer_get(nxo);
    if (index < 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    NXO_STACK_NGET(nxo, ostack, a_thread, index + 1);
    nxo_stack_remove(ostack, nxo);
    nxo_stack_pop(ostack);
}

void
systemdict_istack(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *istack, *stack;

    ostack = nxo_thread_ostack_get(a_thread);
    istack = nxo_thread_istack_get(a_thread);

    stack = nxo_stack_push(ostack);
    nxo_stack_new(stack, nxo_thread_currentlocking(a_thread));
    nxo_stack_copy(stack, istack);
}

#ifdef CW_THREADS
void
systemdict_join(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *thread;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(thread, ostack, a_thread);
    if (nxo_type_get(thread) != NXOT_THREAD)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    nxo_thread_join(thread);

    nxo_stack_pop(ostack);
}
#endif

void
systemdict_known(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *dict, *key;
    cw_bool_t known;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(key, ostack, a_thread);
    NXO_STACK_DOWN_GET(dict, ostack, a_thread, key);
    if (nxo_type_get(dict) != NXOT_DICT)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    known = !nxo_dict_lookup(dict, key, NULL);
    nxo_boolean_new(dict, known);

    nxo_stack_pop(ostack);
}

#ifdef CW_THREADS
void
systemdict_lcheck(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;
    cw_bool_t locking;
	
    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);

    switch (nxo_type_get(nxo))
    {
	case NXOT_BOOLEAN:
	case NXOT_CONDITION:
	case NXOT_FINO:
#ifdef CW_HANDLE
	case NXOT_HANDLE:
#endif
	case NXOT_INTEGER:
	case NXOT_MARK:
	case NXOT_MUTEX:
	case NXOT_NAME:
	case NXOT_NULL:
	case NXOT_OPERATOR:
	case NXOT_PMARK:
#ifdef CW_REAL
	case NXOT_REAL:
#endif
#ifdef CW_REGEX
	case NXOT_REGEX:
	case NXOT_REGSUB:
#endif
	case NXOT_THREAD:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
	case NXOT_ARRAY:
	case NXOT_DICT:
	case NXOT_FILE:
	case NXOT_STACK:
	case NXOT_STRING:
	{
	    locking = nxo_lcheck(nxo);
	    break;
	}
	case NXOT_NO:
	default:
	{
	    cw_not_reached();
	}
    }
    nxo_boolean_new(nxo, locking);
}
#endif

void
systemdict_le(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo_a, *nxo_b;
    cw_sint32_t result;
    cw_bool_t le;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(nxo_b, ostack, a_thread);
    NXO_STACK_DOWN_GET(nxo_a, ostack, a_thread, nxo_b);

    switch (nxo_type_get(nxo_a))
    {
	case NXOT_INTEGER:
#ifdef CW_REAL
	case NXOT_REAL:
#endif
	case NXOT_STRING:
	{
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }
    switch (nxo_type_get(nxo_b))
    {
	case NXOT_INTEGER:
#ifdef CW_REAL
	case NXOT_REAL:
#endif
	case NXOT_STRING:
	{
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }

    result = nxo_compare(nxo_a, nxo_b);
    if (result == 2)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    else if (result <= 0)
    {
	le = TRUE;
    }
    else
    {
	le = FALSE;
    }

    nxo_boolean_new(nxo_a, le);

    nxo_stack_pop(ostack);
}

void
systemdict_length(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;
    cw_nxoi_t len;

    ostack = nxo_thread_ostack_get(a_thread);
	
    NXO_STACK_GET(nxo, ostack, a_thread);
    switch (nxo_type_get(nxo))
    {
	case NXOT_ARRAY:
	{
	    len = nxo_array_len_get(nxo);
	    break;
	}
	case NXOT_DICT:
	{
	    len = nxo_dict_count(nxo);
	    break;
	}
	case NXOT_NAME:
	{
	    len = nxo_name_len_get(nxo);
	    break;
	}
	case NXOT_STRING:
	{
	    len = nxo_string_len_get(nxo);
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }

    nxo_integer_new(nxo, len);
}

#ifdef CW_POSIX
void
systemdict_link(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack;
    cw_nxo_t *filename, *linkname, *tfilename, *tlinkname;
    int error;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    NXO_STACK_GET(linkname, ostack, a_thread);
    NXO_STACK_DOWN_GET(filename, ostack, a_thread, linkname);
    if (nxo_type_get(filename) != NXOT_STRING
	|| nxo_type_get(linkname) != NXOT_STRING)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    /* Create a copy of filename with an extra byte to store a '\0'
     * terminator. */
    tfilename = nxo_stack_push(tstack);
    nxo_string_cstring(tfilename, filename, a_thread);

    /* Create a copy of linkname with an extra byte to store a '\0'
     * terminator. */
    tlinkname = nxo_stack_push(tstack);
    nxo_string_cstring(tlinkname, linkname, a_thread);

    error = link(nxo_string_get(tfilename), nxo_string_get(tlinkname));
    nxo_stack_npop(tstack, 2);
    if (error == -1)
    {
	switch (errno)
	{
	    case EIO:
	    case EDQUOT:
	    case EMLINK:
	    case ENOSPC:
	    case EROFS:
	    {
		nxo_thread_nerror(a_thread, NXN_ioerror);
		break;
	    }
	    case EEXIST:
	    case ENOENT:
	    case ENOTDIR:
	    case EOPNOTSUPP:
	    {
		nxo_thread_nerror(a_thread, NXN_undefinedfilename);
		break;
	    }
	    case EACCES:
	    case ENAMETOOLONG:
	    case ELOOP:
	    case EPERM:
	    case EXDEV:
	    {
		nxo_thread_nerror(a_thread, NXN_invalidfileaccess);
		break;
	    }
	    case EFAULT:
	    default:
	    {
		nxo_thread_nerror(a_thread, NXN_unregistered);
	    }
	}
	return;
    }

    nxo_stack_npop(ostack, 2);
}
#endif

#ifdef CW_SOCKET
void
systemdict_listen(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *sock;
    cw_uint32_t npop;
    int backlog, error;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(sock, ostack, a_thread);
    if (nxo_type_get(sock) == NXOT_INTEGER)
    {
	backlog = (int) nxo_integer_get(sock);
	NXO_STACK_DOWN_GET(sock, ostack, a_thread, sock);
	npop = 2;
    }
    else
    {
	/* Maximum backlog. */
	backlog = -1;
	npop = 1;
    }
    if (nxo_type_get(sock) != NXOT_FILE)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    error = listen(nxo_file_fd_get(sock), backlog);
    if (error == -1)
    {
	switch (errno)
	{
	    case EADDRINUSE:
	    case EOPNOTSUPP:
	    {
		nxo_thread_nerror(a_thread, NXN_neterror);
		return;
	    }
	    case ENOTSOCK:
	    {
		nxo_thread_nerror(a_thread, NXN_invalidfileaccess);
		return;
	    }
	    case EBADF:
	    default:
	    {
		nxo_thread_nerror(a_thread, NXN_unregistered);
		return;
	    }
	}
    }

    nxo_stack_npop(ostack, npop);
}
#endif

#ifdef CW_REAL
void
systemdict_ln(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;
    cw_nxor_t real;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    switch (nxo_type_get(nxo))
    {
	case NXOT_INTEGER:
	{
	    real = (cw_nxor_t) nxo_integer_get(nxo);
	    break;
	}
	case NXOT_REAL:
	{
	    real = nxo_real_get(nxo);
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }
    if (real <= 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }
    nxo_real_new(nxo, log(real));
}
#endif

void
systemdict_load(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack;
    cw_nxo_t *key, *val;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);

    NXO_STACK_GET(key, ostack, a_thread);
    val = nxo_stack_push(tstack);

    if (nxo_thread_dstack_search(a_thread, key, val))
    {
	nxo_stack_pop(tstack);
	nxo_thread_nerror(a_thread, NXN_undefined);
	return;
    }
    nxo_dup(key, val);
    nxo_stack_pop(tstack);
}

#ifdef CW_POSIX
void
systemdict_localtime(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack, *nxo, *name, *value;
    cw_bool_t currentlocking;
    cw_nxoi_t realtime;
    struct tm tm;
    time_t time;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    currentlocking = nxo_thread_currentlocking(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    realtime = nxo_integer_get(nxo);
    if (realtime < 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    time = realtime / 1000000000LL;
    localtime_r(&time, &tm);

    name = nxo_stack_push(tstack);
    value = nxo_stack_push(tstack);

    nxo_dict_new(nxo, currentlocking, 11);

    /* sec. */
    nxo_name_new(name, nxn_str(NXN_sec), nxn_len(NXN_sec), TRUE);
    nxo_integer_new(value, tm.tm_sec);
    nxo_dict_def(nxo, name, value);

    /* min. */
    nxo_name_new(name, nxn_str(NXN_min), nxn_len(NXN_min), TRUE);
    nxo_integer_new(value, tm.tm_min);
    nxo_dict_def(nxo, name, value);

    /* hour. */
    nxo_name_new(name, nxn_str(NXN_hour), nxn_len(NXN_hour), TRUE);
    nxo_integer_new(value, tm.tm_hour);
    nxo_dict_def(nxo, name, value);

    /* mday. */
    nxo_name_new(name, nxn_str(NXN_mday), nxn_len(NXN_mday), TRUE);
    nxo_integer_new(value, tm.tm_mday);
    nxo_dict_def(nxo, name, value);

    /* mon. */
    nxo_name_new(name, nxn_str(NXN_mon), nxn_len(NXN_mon), TRUE);
    nxo_integer_new(value, tm.tm_mon);
    nxo_dict_def(nxo, name, value);

    /* year. */
    nxo_name_new(name, nxn_str(NXN_year), nxn_len(NXN_year), TRUE);
    nxo_integer_new(value, tm.tm_year + 1900);
    nxo_dict_def(nxo, name, value);

    /* wday. */
    nxo_name_new(name, nxn_str(NXN_wday), nxn_len(NXN_wday), TRUE);
    nxo_integer_new(value, tm.tm_wday);
    nxo_dict_def(nxo, name, value);

    /* yday. */
    nxo_name_new(name, nxn_str(NXN_yday), nxn_len(NXN_yday), TRUE);
    nxo_integer_new(value, tm.tm_yday);
    nxo_dict_def(nxo, name, value);

    /* isdst. */
    nxo_name_new(name, nxn_str(NXN_isdst), nxn_len(NXN_isdst), TRUE);
    nxo_boolean_new(value, tm.tm_isdst ? TRUE : FALSE);
    nxo_dict_def(nxo, name, value);

    /* zone. */
    nxo_name_new(name, nxn_str(NXN_zone), nxn_len(NXN_zone), TRUE);
    nxo_string_new(value, currentlocking, strlen(tm.tm_zone));
    nxo_string_set(value, 0, tm.tm_zone, nxo_string_len_get(value));
    nxo_dict_def(nxo, name, value);

    /* gmtoff. */
    nxo_name_new(name, nxn_str(NXN_gmtoff), nxn_len(NXN_gmtoff), TRUE);
    nxo_integer_new(value, tm.tm_gmtoff);
    nxo_dict_def(nxo, name, value);

    nxo_stack_npop(tstack, 2);
}
#endif

#ifdef CW_THREADS
void
systemdict_lock(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *mutex;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(mutex, ostack, a_thread);
    if (nxo_type_get(mutex) != NXOT_MUTEX)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    nxo_mutex_lock(mutex);

    nxo_stack_pop(ostack);
}
#endif

#ifdef CW_REAL
void
systemdict_log(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;
    cw_nxor_t real;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    switch (nxo_type_get(nxo))
    {
	case NXOT_INTEGER:
	{
	    real = (cw_nxor_t) nxo_integer_get(nxo);
	    break;
	}
	case NXOT_REAL:
	{
	    real = nxo_real_get(nxo);
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }
    if (real <= 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }
    nxo_real_new(nxo, log10(real));
}
#endif

void
systemdict_loop(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *estack, *istack, *tstack;
    cw_nxo_t *exec, *nxo, *tnxo;
    cw_uint32_t edepth, tdepth;

    ostack = nxo_thread_ostack_get(a_thread);
    estack = nxo_thread_estack_get(a_thread);
    istack = nxo_thread_istack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);

    NXO_STACK_GET(exec, ostack, a_thread);

    /* Record stack depths so that we can clean up later. */
    edepth = nxo_stack_count(estack);
    tdepth = nxo_stack_count(tstack);

    /* Move the object to be executed to tstack. */
    tnxo = nxo_stack_push(tstack);
    nxo_dup(tnxo, exec);
    nxo_stack_pop(ostack);

    /* Catch an exit exception, if thrown, but do not continue executing the
     * loop. */
    xep_begin();
    xep_try
    {
	for (;;)
	{
	    nxo = nxo_stack_push(estack);
	    nxo_dup(nxo, tnxo);
	    nxo_thread_loop(a_thread);
	}
    }
    xep_catch(CW_ONYXX_EXIT)
    {
	xep_handled();
    }
    xep_end();

    /* Clean up stacks. */
    nxo_stack_npop(estack, nxo_stack_count(estack) - edepth);
    nxo_stack_npop(istack, nxo_stack_count(istack) - nxo_stack_count(estack));
    nxo_stack_npop(tstack, nxo_stack_count(tstack) - tdepth);
}

void
systemdict_lt(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo_a, *nxo_b;
    cw_sint32_t result;
    cw_bool_t lt;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(nxo_b, ostack, a_thread);
    NXO_STACK_DOWN_GET(nxo_a, ostack, a_thread, nxo_b);

    switch (nxo_type_get(nxo_a))
    {
	case NXOT_INTEGER:
#ifdef CW_REAL
	case NXOT_REAL:
#endif
	case NXOT_STRING:
	{
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }
    switch (nxo_type_get(nxo_b))
    {
	case NXOT_INTEGER:
#ifdef CW_REAL
	case NXOT_REAL:
#endif
	case NXOT_STRING:
	{
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }

    result = nxo_compare(nxo_a, nxo_b);
    if (result == 2)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    else if (result == -1)
    {
	lt = TRUE;
    }
    else
    {
	lt = FALSE;
    }

    nxo_boolean_new(nxo_a, lt);

    nxo_stack_pop(ostack);
}

#ifdef CW_REGEX
/* Get flags in preparation for calling nxo_regex_new(). */
CW_P_INLINE cw_nxn_t
systemdict_p_regex_flags_get(cw_nxo_t *a_flags, cw_nxo_t *a_thread,
			     cw_bool_t *r_cont, cw_bool_t *r_global,
			     cw_bool_t *r_insensitive, cw_bool_t *r_multiline,
			     cw_bool_t *r_singleline)
{
    cw_nxn_t retval;
    cw_nxo_t *tstack, *tkey, *tval;

    tstack = nxo_thread_tstack_get(a_thread);
    tkey = nxo_stack_push(tstack);
    tval = nxo_stack_push(tstack);

#define REGEX_FLAG_GET(a_nxn, a_var)					\
    do									\
    {									\
	if ((a_var) != NULL)						\
	{								\
	    nxo_name_new(tkey, nxn_str(a_nxn), nxn_len(a_nxn),	\
			 TRUE);						\
	    if (nxo_dict_lookup(a_flags, tkey, tval))			\
	    {								\
		*(a_var) = FALSE;					\
	    }								\
	    else							\
	    {								\
	        if (nxo_type_get(tval) != NXOT_BOOLEAN)			\
	        {							\
		    retval = NXN_typecheck;				\
		    goto RETURN;					\
	        }							\
									\
		*(a_var) = nxo_boolean_get(tval);			\
	    }								\
	}								\
    } while (0)

    REGEX_FLAG_GET(NXN_c, r_cont);
    REGEX_FLAG_GET(NXN_g, r_global);
    REGEX_FLAG_GET(NXN_i, r_insensitive);
    REGEX_FLAG_GET(NXN_m, r_multiline);
    REGEX_FLAG_GET(NXN_s, r_singleline);

#undef REGEX_FLAG_GET

    retval = NXN_ZERO;
    RETURN:
    nxo_stack_npop(tstack, 2);
    return retval;
}

void
systemdict_match(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo, *flags, *pattern, *input;
    cw_uint32_t npop;
    cw_bool_t match;
    cw_nxn_t error;

    ostack = nxo_thread_ostack_get(a_thread);

    npop = 0;
    NXO_STACK_GET(nxo, ostack, a_thread);
    switch (nxo_type_get(nxo))
    {
	case NXOT_DICT:
	{
	    npop++;
	    flags = nxo;
	    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
	    if (nxo_type_get(nxo) != NXOT_STRING)
	    {
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	    }

	    /* Fall through. */
	}
	case NXOT_STRING:
	{
	    cw_bool_t cont, global, insensitive, multiline, singleline;

	    /* Get regex flags. */
	    if (npop != 0)
	    {
		error = systemdict_p_regex_flags_get(flags, a_thread,
						     &cont, &global,
						     &insensitive, &multiline,
						     &singleline);
		if (error)
		{
		    nxo_thread_nerror(a_thread, error);
		    return;
		}
	    }
	    else
	    {
		cont = FALSE;
		global = FALSE;
		insensitive = FALSE;
		multiline = FALSE;
		singleline = FALSE;
	    }

	    /* Get pattern. */
	    pattern = nxo;

	    npop++;

	    /* Get input string. */
	    NXO_STACK_DOWN_GET(input, ostack, a_thread, nxo);
	    if (nxo_type_get(input) != NXOT_STRING)
	    {
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	    }

	    nxo_string_lock(pattern);
	    error = nxo_regex_nonew_match(a_thread, nxo_string_get(pattern),
					  nxo_string_len_get(pattern), cont,
					  global, insensitive, multiline,
					  singleline, input, &match);
	    nxo_string_unlock(pattern);
	    if (error)
	    {
		nxo_thread_nerror(a_thread, error);
		return;
	    }

	    break;
	}
	case NXOT_REGEX:
	{
	    cw_nxo_t *regex;

	    npop++;
	    regex = nxo;

	    /* Get input string. */
	    NXO_STACK_DOWN_GET(input, ostack, a_thread, nxo);
	    if (nxo_type_get(input) != NXOT_STRING)
	    {
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	    }

	    nxo_regex_match(regex, a_thread, input, &match);

	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }

    nxo_boolean_new(input, match);
    nxo_stack_npop(ostack, npop);
}
#endif

#ifdef CW_POSIX
void
systemdict_mkdir(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack;
    cw_nxo_t *nxo, *tnxo;
    cw_uint32_t npop;
    mode_t mode;
    int error;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);

    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) == NXOT_INTEGER)
    {
	/* Mode specified. */
	npop = 2;
	mode = (mode_t) nxo_integer_get(nxo);
	if ((mode & 0777) != mode)
	{
	    nxo_thread_nerror(a_thread, NXN_rangecheck);
	    return;
	}
	NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    }
    else
    {
	npop = 1;
	mode = 0777;
    }
    if (nxo_type_get(nxo) != NXOT_STRING)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    tnxo = nxo_stack_push(tstack);
    nxo_string_cstring(tnxo, nxo, a_thread);

    error = mkdir(nxo_string_get(tnxo), mode);
    nxo_stack_pop(tstack);

    if (error == -1)
    {
	switch (errno)
	{
	    case EIO:
	    case EDQUOT:
	    case ENOSPC:
	    case EROFS:
	    {
		nxo_thread_nerror(a_thread, NXN_ioerror);
		break;
	    }
	    case EACCES:
	    case EEXIST:
	    case ELOOP:
	    case ENOENT:
	    case ENOTDIR:
	    case ENAMETOOLONG:
	    {
		nxo_thread_nerror(a_thread, NXN_invalidfileaccess);
		break;
	    }
	    case EFAULT:
	    default:
	    {
		nxo_thread_nerror(a_thread, NXN_unregistered);
	    }
	}
	return;
    }

    nxo_stack_npop(ostack, npop);
}
#endif

#ifdef CW_POSIX
void
systemdict_mkfifo(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack;
    cw_nxo_t *nxo, *tnxo;
    cw_uint32_t npop;
    mode_t mode;
    int error;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);

    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) == NXOT_INTEGER)
    {
	/* Mode specified. */
	npop = 2;
	mode = (mode_t) nxo_integer_get(nxo);
	if ((mode & 0777) != mode)
	{
	    nxo_thread_nerror(a_thread, NXN_rangecheck);
	    return;
	}
	NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    }
    else
    {
	npop = 1;
	mode = 0777;
    }
    if (nxo_type_get(nxo) != NXOT_STRING)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    tnxo = nxo_stack_push(tstack);
    nxo_string_cstring(tnxo, nxo, a_thread);

    error = mkfifo(nxo_string_get(tnxo), mode);
    nxo_stack_pop(tstack);

    if (error == -1)
    {
	switch (errno)
	{
	    case ENOSPC:
	    case EROFS:
	    {
		nxo_thread_nerror(a_thread, NXN_ioerror);
		return;
	    }
	    case EACCES:
	    case EEXIST:
	    case ENOTDIR:
	    case ENOENT:
	    case ENAMETOOLONG:
	    {
		nxo_thread_nerror(a_thread, NXN_invalidfileaccess);
		return;
	    }
	    default:
	    {
		nxo_thread_nerror(a_thread, NXN_unregistered);
		return;
	    }
	}
    }

    nxo_stack_npop(ostack, npop);
}
#endif

void
systemdict_mod(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *a, *b;

    ostack = nxo_thread_ostack_get(a_thread);
	
    NXO_STACK_GET(b, ostack, a_thread);
    NXO_STACK_DOWN_GET(a, ostack, a_thread, b);
    switch (nxo_type_get(a))
    {
	case NXOT_INTEGER:
	{
	    switch (nxo_type_get(b))
	    {
		case NXOT_INTEGER:
		{
		    if (nxo_integer_get(b) == 0)
		    {
			nxo_thread_nerror(a_thread, NXN_undefinedresult);
			return;
		    }
		    nxo_integer_set(a, nxo_integer_get(a) % nxo_integer_get(b));
		    break;
		}
#ifdef CW_REAL
		case NXOT_REAL:
		{
		    if (nxo_real_get(b) == 0.0)
		    {
			nxo_thread_nerror(a_thread, NXN_undefinedresult);
			return;
		    }
		    nxo_real_new(a, fmod(nxo_integer_get(a), nxo_real_get(b)));
		    break;
		}
#endif
		default:
		{
		    nxo_thread_nerror(a_thread, NXN_typecheck);
		    return;
		}
	    }
	    break;
	}
#ifdef CW_REAL
	case NXOT_REAL:
	{
	    switch (nxo_type_get(b))
	    {
		case NXOT_INTEGER:
		{
		    if (nxo_integer_get(b) == 0)
		    {
			nxo_thread_nerror(a_thread, NXN_undefinedresult);
			return;
		    }
		    nxo_real_new(a, fmod(nxo_real_get(a), nxo_integer_get(b)));
		    break;
		}
		case NXOT_REAL:
		{
		    if (nxo_real_get(b) == 0.0)
		    {
			nxo_thread_nerror(a_thread, NXN_undefinedresult);
			return;
		    }
		    nxo_real_set(a, fmod(nxo_real_get(a), nxo_real_get(b)));
		    break;
		}
		default:
		{
		    nxo_thread_nerror(a_thread, NXN_typecheck);
		    return;
		}
	    }
	    break;
	}
#endif
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }

    nxo_stack_pop(ostack);
}

#ifdef CW_MODULES
/* #define CW_MODLOAD_VERBOSE */
static cw_nxmod_t *
systemdict_p_nxmod_new(void *a_handle)
{
    cw_nxmod_t *retval;

    retval = (cw_nxmod_t *) nxa_malloc(sizeof(cw_nxmod_t));
    /* Set the default iteration for module destruction to 1.  This number can
     * be overridden on a per-module basis in the module initialization code. */
    retval->iter = 1;
    retval->dlhandle = a_handle;

    return retval;
}

static cw_nxoe_t *
systemdict_p_nxmod_ref_iter(void *a_data, cw_bool_t a_reset)
{
    return NULL;
}

static cw_bool_t
systemdict_p_nxmod_delete(void *a_data, cw_uint32_t a_iter)
{
    cw_bool_t retval;
    cw_nxmod_t *nxmod = (cw_nxmod_t *) a_data;

    if (a_iter != nxmod->iter)
    {
	retval = TRUE;
	goto RETURN;
    }

#ifdef CW_MODLOAD_VERBOSE
    fprintf(stderr, "dlclose(%p)\n", nxmod->dlhandle);
#endif
    dlclose(nxmod->dlhandle);
    nxa_free(a_data, sizeof(cw_nxmod_t));
	
    retval = FALSE;
    RETURN:
    return retval;
}

void
systemdict_modload(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *estack, *tstack;
    cw_nxo_t *path, *sym, *nxo;
    cw_nxmod_t *nxmod;
    cw_uint8_t *str;
    void *symbol, *handle = NULL;

    ostack = nxo_thread_ostack_get(a_thread);
    estack = nxo_thread_estack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    NXO_STACK_GET(sym, ostack, a_thread);
    NXO_STACK_DOWN_GET(path, ostack, a_thread, sym);
    if (nxo_type_get(path) != NXOT_STRING)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    /* Create '\0'-terminated copy of path. */
    nxo = nxo_stack_push(tstack);
    nxo_string_cstring(nxo, path, a_thread);
    str = nxo_string_get(nxo);

    /* Try to dlopen(). */
    handle = dlopen(str, RTLD_LAZY);
    if (handle == NULL)
    {
#ifdef CW_DBG
	fprintf(stderr, "dlopen() error: %s\n", dlerror());
#endif
	nxo_stack_pop(tstack);
	nxo_thread_nerror(a_thread, NXN_invalidfileaccess);
	return;
    }
#ifdef CW_MODLOAD_VERBOSE
    fprintf(stderr, "dlopen(\"%s\") handle: %p\n", str, handle);
#endif

    /* Create '\0'-terminated copy of sym. */
    nxo_string_cstring(nxo, sym, a_thread);
    str = nxo_string_get(nxo);

    /* Look up symbol. */
#ifdef CW_MODLOAD_VERBOSE
    fprintf(stderr, "dlsym(\"%s\")\n", str);
#endif
    symbol = dlsym(handle, str);

    /* Pop nxo. */
    nxo_stack_pop(tstack);

    if (symbol == NULL)
    {
	/* Couldn't find the symbol. */
#ifdef CW_DBG
	fprintf(stderr, "dlsym() error: %s\n", dlerror());
#endif
	dlclose(handle);
	nxo_thread_nerror(a_thread, NXN_undefined);
	return;
    }

    /* Create a handle whose data pointer is a (cw_nxmod_t), and whose
     * evaluation function is the symbol we just looked up. */
    nxmod = systemdict_p_nxmod_new(handle);
    nxo = nxo_stack_push(estack);
    nxo_handle_new(nxo, nxmod, symbol, systemdict_p_nxmod_ref_iter,
		   systemdict_p_nxmod_delete);
    nxo_dup(nxo_handle_tag_get(nxo), sym);
    nxo_attr_set(nxo, NXOA_EXECUTABLE);

    /* Pop the arguments before recursing. */
    nxo_stack_npop(ostack, 2);

    /* Recurse on the handle. */
    nxo_thread_loop(a_thread);
}
#endif

#ifdef CW_THREADS
void
systemdict_monitor(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *estack, *tstack, *proc, *mutex, *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    estack = nxo_thread_estack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    NXO_STACK_GET(proc, ostack, a_thread);
    NXO_STACK_DOWN_GET(mutex, ostack, a_thread, proc);
    if (nxo_type_get(mutex) != NXOT_MUTEX)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    /* Dup proc to estack. */
    nxo = nxo_stack_push(estack);
    nxo_dup(nxo, proc);

    /* Dup mutex to tstack. */
    nxo = nxo_stack_push(tstack);
    nxo_dup(nxo, mutex);

    /* Remove args from ostack. */
    nxo_stack_npop(ostack, 2);

    /* Lock mutex. */
    nxo_mutex_lock(nxo);

    /* Make sure that the mutex gets unlocked, even in the face of
     * exceptions. */
    xep_begin();
    xep_try
    {
	/* Execute proc. */
	nxo_thread_loop(a_thread);
    }
    xep_acatch
    {
	/* Don't handle the exception, but unlock mutex. */
	nxo_mutex_unlock(nxo);
    }
    xep_end();

    /* Unlock mutex. */
    nxo_mutex_unlock(nxo);

    /* Pop mutex off of tstack. */
    nxo_stack_pop(tstack);
}
#endif

void
systemdict_mul(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo_a, *nxo_b;
    cw_nxoi_t integer_a, integer_b;
#ifdef CW_REAL
    cw_bool_t do_real;
    cw_nxor_t real_a, real_b;
#endif

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo_b, ostack, a_thread);
    NXO_STACK_DOWN_GET(nxo_a, ostack, a_thread, nxo_b);
    switch (nxo_type_get(nxo_a))
    {
	case NXOT_INTEGER:
	{
#ifdef CW_REAL
	    do_real = FALSE;
#endif
	    integer_a = nxo_integer_get(nxo_a);
	    break;
	}
#ifdef CW_REAL
	case NXOT_REAL:
	{
	    do_real = TRUE;
	    real_a = nxo_real_get(nxo_a);
	    break;
	}
#endif
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }
    switch (nxo_type_get(nxo_b))
    {
	case NXOT_INTEGER:
	{
#ifdef CW_REAL
	    if (do_real)
	    {
		real_b = (cw_nxor_t) nxo_integer_get(nxo_b);
	    }
	    else
#endif
	    {
		integer_b = nxo_integer_get(nxo_b);
	    }
	    break;
	}
#ifdef CW_REAL
	case NXOT_REAL:
	{
	    real_b = nxo_real_get(nxo_b);
	    if (do_real == FALSE)
	    {
		do_real = TRUE;
		real_a = (cw_nxor_t) integer_a;
	    }
	    break;
	}
#endif
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }

#ifdef CW_REAL
    if (do_real)
    {
	/* nxo_a may be an integer, so use nxo_real_new() rather than
	 * nxo_real_set(). */
	nxo_real_new(nxo_a, real_a * real_b);
    }
    else
#endif
    {
	nxo_integer_set(nxo_a, integer_a * integer_b);
    }

    nxo_stack_pop(ostack);
}

#ifdef CW_THREADS
void
systemdict_mutex(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *mutex;

    ostack = nxo_thread_ostack_get(a_thread);
    mutex = nxo_stack_push(ostack);
    nxo_mutex_new(mutex);
}
#endif

void
systemdict_nbpop(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxoi_t count;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    count = nxo_integer_get(nxo);
    if (count < 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }
    if (count > nxo_stack_count(ostack) - 1)
    {
	nxo_thread_nerror(a_thread, NXN_stackunderflow);
	return;
    }

    /* Pop the argument off as well as the count. */
    nxo_stack_pop(ostack);
    nxo_stack_nbpop(ostack, count);
}

void
systemdict_ncat(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo, *tnxo, *r;
    cw_nxoi_t count;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    count = nxo_integer_get(nxo);
    if (count < 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    if (count > 0)
    {
	cw_nxot_t type;
	cw_uint32_t i, j;

	tnxo = nxo;
	NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
	type = nxo_type_get(nxo);
	nxo = tnxo;
	switch (type)
	{
	    case NXOT_ARRAY:
	    {
		cw_uint32_t len, off, nelms = 0;

		/* Check argument type and add up the total number of
		 * elements. */
		for (i = 0; i < count; i++)
		{
		    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
		    if (nxo_type_get(nxo) != NXOT_ARRAY)
		    {
			nxo_thread_nerror(a_thread, NXN_typecheck);
			return;
		    }
		    nelms += nxo_array_len_get(nxo);
		}

		/* Allocate the array. */
		r = nxo_stack_under_push(ostack, nxo);
		nxo_array_new(r, nxo_thread_currentlocking(a_thread), nelms);

		/* Fill in the array. */
		for (i = off = 0; i < count; i++)
		{
		    len = nxo_array_len_get(nxo);
		    for (j = 0; j < len; j++)
		    {
			nxo_array_el_get(nxo, j, tnxo);
			nxo_array_el_set(r, tnxo, off);
			off++;
		    }

		    nxo = nxo_stack_up_get(ostack, nxo);
		}

		break;
	    }
	    case NXOT_STACK:
	    {
		cw_nxo_t *tstack, *to, *fr;

		/* Iterate over the stacks to be catenated in one pass, since
		 * there is no actual need for a first pass to calculate the
		 * final stack size, unlike with arrays and strings. */

		tstack = nxo_thread_tstack_get(a_thread);

		/* Allocate the stack. */
		r = nxo_stack_push(tstack);
		nxo_stack_new(r, nxo_thread_currentlocking(a_thread));

		/* Fill in the stack. */
		for (i = 0; i < count; i++)
		{
		    nxo = nxo_stack_down_get(ostack, nxo);
		    if (nxo == NULL)
		    {
			nxo_stack_pop(tstack);
			nxo_thread_nerror(a_thread, NXN_stackunderflow);
			return;
		    }
		    if (nxo_type_get(nxo) != NXOT_STACK)
		    {
			nxo_stack_pop(tstack);
			nxo_thread_nerror(a_thread, NXN_typecheck);
			return;
		    }

		    for (fr = nxo_stack_get(nxo);
			 fr != NULL;
			 fr = nxo_stack_down_get(nxo, fr))
		    {
			to = nxo_stack_bpush(r);
			nxo_dup(to, fr);
		    }
		}

		nxo = nxo_stack_under_push(ostack, nxo);
		nxo_dup(nxo, r);
		nxo_stack_pop(tstack);

		break;
	    }
	    case NXOT_STRING:
	    {
		cw_uint32_t len, off, nelms = 0;

		/* Check argument type and add up the total number of
		 * elements. */
		for (i = 0; i < count; i++)
		{
		    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
		    if (nxo_type_get(nxo) != NXOT_STRING)
		    {
			nxo_thread_nerror(a_thread, NXN_typecheck);
			return;
		    }
		    nelms += nxo_string_len_get(nxo);
		}

		/* Allocate the string. */
		r = nxo_stack_under_push(ostack, nxo);
		nxo_string_new(r, nxo_thread_currentlocking(a_thread), nelms);

		/* Fill in the string. */
		for (i = off = 0; i < count; i++)
		{
		    len = nxo_string_len_get(nxo);
		    nxo_string_lock(nxo);
		    nxo_string_set(r, off, nxo_string_get(nxo), len);
		    nxo_string_unlock(nxo);
		    off += len;

		    nxo = nxo_stack_up_get(ostack, nxo);
		}

		break;
	    }
	    default:
	    {
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	    }
	}
    }

    nxo_stack_npop(ostack, count + 1);
}

void
systemdict_ndn(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxoi_t count;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    count = nxo_integer_get(nxo);
    if (count < 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }
    if (count > nxo_stack_count(ostack) - 1)
    {
	nxo_thread_nerror(a_thread, NXN_stackunderflow);
	return;
    }

    nxo_stack_pop(ostack);
    /* nxo_stack_roll() doesn't allow a count of 0. */
    if (count > 0)
    {
	nxo_stack_roll(ostack, count, -1);
    }
}

void
systemdict_ndup(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo, *dup;
    cw_uint32_t i;
    cw_nxoi_t count;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    count = nxo_integer_get(nxo);
    if (count < 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }
    if (count > nxo_stack_count(ostack) - 1)
    {
	nxo_thread_nerror(a_thread, NXN_stackunderflow);
	return;
    }
    nxo_stack_pop(ostack);

    /* Iterate down the stack, creating dup's along the way.  Since we're going
     * down, it's necessary to use nxo_stack_under_push() to preserve order. */
    for (i = 0, nxo = NULL, dup = NULL; i < count; i++)
    {
	nxo = nxo_stack_down_get(ostack, nxo);
	dup = nxo_stack_under_push(ostack, dup);
	nxo_dup(dup, nxo);
    }
}

void
systemdict_ne(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo_a, *nxo_b;
    cw_sint32_t result;
    cw_bool_t ne;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(nxo_b, ostack, a_thread);
    NXO_STACK_DOWN_GET(nxo_a, ostack, a_thread, nxo_b);

    result = nxo_compare(nxo_a, nxo_b);
    if (result == 0)
    {
	ne = FALSE;
    }
    else
    {
	ne = TRUE;
    }

    nxo_boolean_new(nxo_a, ne);

    nxo_stack_pop(ostack);
}

void
systemdict_neg(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *a;

    ostack = nxo_thread_ostack_get(a_thread);
	
    NXO_STACK_GET(a, ostack, a_thread);
    switch (nxo_type_get(a))
    {
	case NXOT_INTEGER:
	{
	    nxo_integer_set(a, -nxo_integer_get(a));
	    break;
	}
#ifdef CW_REAL
	case NXOT_REAL:
	{
	    nxo_real_set(a, -nxo_real_get(a));
	    break;
	}
#endif
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }
}

void
systemdict_nip(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_NGET(nxo, ostack, a_thread, 1);
    nxo_stack_remove(ostack, nxo);
}

void
systemdict_nonblocking(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_bool_t nonblocking;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_FILE)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    nonblocking = nxo_file_nonblocking_get(nxo);
    nxo_boolean_new(nxo, nonblocking);
}

void
systemdict_not(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(nxo, ostack, a_thread);

    if (nxo_type_get(nxo) == NXOT_BOOLEAN)
    {
	nxo_boolean_set(nxo, !nxo_boolean_get(nxo));
    }
    else if (nxo_type_get(nxo) == NXOT_INTEGER)
    {
	nxo_integer_set(nxo, ~nxo_integer_get(nxo));
    }
    else
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
    }
}

void
systemdict_npop(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxoi_t count;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    count = nxo_integer_get(nxo);
    if (count < 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    /* Pop the argument off as well as the count. */
    NXO_STACK_NPOP(ostack, a_thread, count + 1);
}

#ifdef CW_POSIX
void
systemdict_nsleep(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;
    int error;
    struct timespec sleeptime, remainder;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(nxo, ostack, a_thread);

    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    if (nxo_integer_get(nxo) <= 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    sleeptime.tv_sec = nxo_integer_get(nxo) / 1000000000LL;
    sleeptime.tv_nsec = nxo_integer_get(nxo) % 1000000000LL;

    for (;;)
    {
	error = nanosleep(&sleeptime, &remainder);
	if (error == 0)
	{
	    /* We've slept the entire time. */
	    break;
	}
	/* A signal interrupted us.  Sleep some more. */
	memcpy(&sleeptime, &remainder, sizeof(struct timespec));
    }
    nxo_stack_pop(ostack);
}
#endif

void
systemdict_nup(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxoi_t count;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    count = nxo_integer_get(nxo);
    if (count < 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }
    if (count > nxo_stack_count(ostack) - 1)
    {
	nxo_thread_nerror(a_thread, NXN_stackunderflow);
	return;
    }

    nxo_stack_pop(ostack);
    /* nxo_stack_roll() doesn't allow a count of 0. */
    if (count > 0)
    {
	nxo_stack_roll(ostack, count, 1);
    }
}

#ifdef CW_REGEX
void
systemdict_offset(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *input, *submatch;
    cw_uint8_t *instr, *inend, *smstr, *smend;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(submatch, ostack, a_thread);
    NXO_STACK_DOWN_GET(input, ostack, a_thread, submatch);
    if (nxo_type_get(input) != NXOT_STRING
	|| nxo_type_get(submatch) != NXOT_STRING)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    /* Get pointers that bound the input string. */
    instr = nxo_string_get(input);
    inend = &instr[nxo_string_len_get(input)];

    /* Get substring. */
    smstr = nxo_string_get(submatch);
    smend = &smstr[nxo_string_len_get(submatch)];

    /* Make sure smstr really is a substring of instr (or equal in position and
     * length). */
    if (smstr < instr || smstr >= inend || smend > inend)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    /* Calculate the substring offset and store it on ostack. */
    nxo_integer_new(input, (cw_nxoi_t) (smstr - instr));

    /* Clean up ostack. */
    nxo_stack_pop(ostack);
}
#endif

#ifdef CW_POSIX
void
systemdict_open(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack;
    cw_nxo_t *name, *flags, *file;
    cw_uint32_t npop;
    mode_t mode;
    cw_nxn_t error;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);

    NXO_STACK_GET(flags, ostack, a_thread);
    if (nxo_type_get(flags) == NXOT_INTEGER)
    {
	/* Mode specified. */
	npop = 2;
	mode = (mode_t) nxo_integer_get(flags);
	if ((mode & 0777) != mode)
	{
	    nxo_thread_nerror(a_thread, NXN_rangecheck);
	    return;
	}
	NXO_STACK_DOWN_GET(flags, ostack, a_thread, flags);
    }
    else
    {
	npop = 1;
	mode = 0777;
    }
    NXO_STACK_DOWN_GET(name, ostack, a_thread, flags);
    if (nxo_type_get(name) != NXOT_STRING || nxo_type_get(flags) != NXOT_STRING)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    file = nxo_stack_push(tstack);
    nxo_file_new(file, nxo_thread_currentlocking(a_thread));
    nxo_string_lock(name);
    error = nxo_file_open(file, nxo_string_get(name),
			  nxo_string_len_get(name), nxo_string_get(flags),
			  nxo_string_len_get(flags), mode);
    nxo_string_unlock(name);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }

    nxo_file_buffer_size_set(file, CW_LIBONYX_FILE_BUFFER_SIZE);

    nxo_stack_npop(ostack, npop);
    nxo_dup(name, file);
    nxo_stack_pop(tstack);
}
#endif

void
systemdict_or(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo_a, *nxo_b;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(nxo_b, ostack, a_thread);
    NXO_STACK_DOWN_GET(nxo_a, ostack, a_thread, nxo_b);

    if (nxo_type_get(nxo_a) == NXOT_BOOLEAN
	&& nxo_type_get(nxo_b) == NXOT_BOOLEAN)
    {
	cw_bool_t or;

	if (nxo_boolean_get(nxo_a) || nxo_boolean_get(nxo_b))
	or = TRUE;
	else
	or = FALSE;
	nxo_boolean_new(nxo_a, or);
    }
    else if (nxo_type_get(nxo_a) == NXOT_INTEGER
	     && nxo_type_get(nxo_b) == NXOT_INTEGER)
    {
	nxo_integer_set(nxo_a, nxo_integer_get(nxo_a) | nxo_integer_get(nxo_b));
    }
    else
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    nxo_stack_pop(ostack);
}

void
systemdict_ostack(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *stack;

    ostack = nxo_thread_ostack_get(a_thread);
    stack = nxo_stack_push(ostack);
    nxo_stack_new(stack, nxo_thread_currentlocking(a_thread));
    nxo_stack_copy(stack, ostack);

    /* Pop the top element off the stack, since it's a reference to the stack
     * itself. */
    nxo_stack_pop(stack);
}

void
systemdict_over(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *under, *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_NGET(under, ostack, a_thread, 1);
    nxo = nxo_stack_push(ostack);
    nxo_dup(nxo, under);
}

#ifdef CW_SOCKET
static void
systemdict_p_peername(cw_nxo_t *a_thread, cw_bool_t a_peer)
{
    cw_nxo_t *ostack, *tstack, *nxo, *tkey, *tval;
    sa_family_t family;
    int len, error;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);

    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_FILE)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    /* Get the socket family. */
    if (systemdict_p_sock_family(a_thread, nxo_file_fd_get(nxo), a_peer,
				 &family))
    {
	return;
    }

    switch (family)
    {
	case AF_INET:
	{
	    struct sockaddr_in sa;

	    len = sizeof(sa);
	    if (a_peer)
	    {
		error = getpeername(nxo_file_fd_get(nxo),
				    (struct sockaddr *) &sa, &len);
	    }
	    else
	    {
		error = getsockname(nxo_file_fd_get(nxo),
				    (struct sockaddr *) &sa, &len);
	    }
	    if (error == -1)
	    {
		switch (errno)
		{
		    case EBADF:
		    {
			nxo_thread_nerror(a_thread, NXN_ioerror);
			return;
		    }
		    case ENOTSOCK:
		    {
			nxo_thread_nerror(a_thread, NXN_argcheck);
			return;
		    }
		    case ENOBUFS:
		    {
			xep_throw(CW_ONYXX_OOM);
			/* Not reached. */
		    }
		    default:
		    {
			nxo_thread_nerror(a_thread, NXN_unregistered);
			return;
		    }
		}
	    }

	    tkey = nxo_stack_push(tstack);
	    tval = nxo_stack_push(tstack);

	    nxo_dict_new(nxo, nxo_thread_currentlocking(a_thread), 3);

	    /* family. */
	    nxo_name_new(tkey, nxn_str(NXN_family), nxn_len(NXN_family), TRUE);
	    nxo_name_new(tval, nxn_str(NXN_AF_INET), nxn_len(NXN_AF_INET),
			 TRUE);
	    nxo_dict_def(nxo, tkey, tval);

	    /* address. */
	    nxo_name_new(tkey, nxn_str(NXN_address), nxn_len(NXN_address),
			 TRUE);
	    nxo_integer_new(tval, ntohl(sa.sin_addr.s_addr));
	    nxo_dict_def(nxo, tkey, tval);

	    /* port. */
	    nxo_name_new(tkey, nxn_str(NXN_port), nxn_len(NXN_port), TRUE);
	    nxo_integer_new(tval, ntohs(sa.sin_port));
	    nxo_dict_def(nxo, tkey, tval);

	    nxo_stack_npop(tstack, 2);
	    break;
	}
	case AF_LOCAL:
	{
	    struct sockaddr_un sa;
	    cw_uint32_t pathlen;

	    len = sizeof(sa);
	    if (a_peer)
	    {
		error = getpeername(nxo_file_fd_get(nxo),
				    (struct sockaddr *) &sa, &len);
	    }
	    else
	    {
		error = getsockname(nxo_file_fd_get(nxo),
				    (struct sockaddr *) &sa, &len);
	    }
	    if (error == -1)
	    {
		switch (errno)
		{
		    case EBADF:
		    {
			nxo_thread_nerror(a_thread, NXN_ioerror);
			return;
		    }
		    case ENOTSOCK:
		    {
			nxo_thread_nerror(a_thread, NXN_argcheck);
			return;
		    }
		    case ENOBUFS:
		    {
			xep_throw(CW_ONYXX_OOM);
			/* Not reached. */
		    }
		    default:
		    {
			nxo_thread_nerror(a_thread, NXN_unregistered);
			return;
		    }
		}
	    }

	    tkey = nxo_stack_push(tstack);
	    tval = nxo_stack_push(tstack);

	    nxo_dict_new(nxo, nxo_thread_currentlocking(a_thread), 2);

	    /* family. */
	    nxo_name_new(tkey, nxn_str(NXN_family), nxn_len(NXN_family), TRUE);
	    nxo_name_new(tval, nxn_str(NXN_AF_LOCAL), nxn_len(NXN_AF_LOCAL),
			 TRUE);
	    nxo_dict_def(nxo, tkey, tval);

	    /* path. */
	    pathlen = strlen(sa.sun_path);
	    nxo_name_new(tkey, nxn_str(NXN_path), nxn_len(NXN_path), TRUE);
	    nxo_string_new(tval, nxo_thread_currentlocking(a_thread), pathlen);
	    nxo_string_set(tval, 0, sa.sun_path, pathlen);
	    nxo_dict_def(nxo, tkey, tval);

	    nxo_stack_npop(tstack, 2);
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_unregistered);
	    return;
	}
    }
}

void
systemdict_peername(cw_nxo_t *a_thread)
{
    systemdict_p_peername(a_thread, TRUE);
}
#endif

#ifdef CW_POSIX
void
systemdict_pid(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    nxo = nxo_stack_push(ostack);
    nxo_integer_new(nxo, getpid());
}
#endif

#ifdef CW_POSIX
void
systemdict_pipe(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    int filedes[2], error;

    ostack = nxo_thread_ostack_get(a_thread);

    error = pipe(filedes);
    if (error == -1)
    {
	switch (errno)
	{
	    case EMFILE:
	    case ENFILE:
	    {
		nxo_thread_nerror(a_thread, NXN_ioerror);
		return;
	    }
	    case EFAULT:
	    default:
	    {
		nxo_thread_nerror(a_thread, NXN_unregistered);
		return;
	    }
	}
    }

    /* Read fd. */
    nxo = nxo_stack_push(ostack);
    nxo_file_new(nxo, nxo_thread_currentlocking(a_thread));
    nxo_file_fd_wrap(nxo, filedes[0], TRUE);
    
    /* Write fd. */
    nxo = nxo_stack_push(ostack);
    nxo_file_new(nxo, nxo_thread_currentlocking(a_thread));
    nxo_file_fd_wrap(nxo, filedes[1], TRUE);
}
#endif

#ifdef CW_POSIX
void
systemdict_poll(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack;
    cw_nxo_t *nxo, *dict, *file, *flags, *flag;
    cw_nxo_t *pollin, *pollrdnorm, *pollrdband, *pollpri;
    cw_nxo_t *pollout, *pollwrnorm, *pollwrband;
    cw_nxo_t *pollerr, *pollhup, *pollnval;
    cw_nxo_t boolean_true, boolean_false;
    struct pollfd *fds;
    unsigned i, nfds;
    int nready;
    cw_uint32_t j, nflags;
    cw_bool_t changed;
    cw_uint32_t tcount;
    cw_nxoi_t timeout;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    tcount = nxo_stack_count(tstack);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    timeout = nxo_integer_get(nxo);
    if (timeout < -1 || timeout > INT_MAX)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }
    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    if (nxo_type_get(nxo) != NXOT_DICT)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    dict = nxo_stack_push(tstack);
    nxo_dup(dict, nxo);

    nxo_boolean_new(&boolean_true, TRUE);
    nxo_boolean_new(&boolean_false, FALSE);

    /* Create names for flags. */
#define POLLFLAGNAME(a_name, a_nxn)					\
    do									\
    {									\
	(a_name) = nxo_stack_push(tstack);				\
	nxo_name_new((a_name), nxn_str(a_nxn),				\
		     nxn_len(a_nxn), TRUE);				\
    } while (0)

    POLLFLAGNAME(pollin, NXN_POLLIN);
    POLLFLAGNAME(pollrdnorm, NXN_POLLRDNORM);
    POLLFLAGNAME(pollrdband, NXN_POLLRDBAND);
    POLLFLAGNAME(pollpri, NXN_POLLPRI);
    POLLFLAGNAME(pollout, NXN_POLLOUT);
    POLLFLAGNAME(pollwrnorm, NXN_POLLWRNORM);
    POLLFLAGNAME(pollwrband, NXN_POLLWRBAND);
    POLLFLAGNAME(pollerr, NXN_POLLERR);
    POLLFLAGNAME(pollhup, NXN_POLLHUP);
    POLLFLAGNAME(pollnval, NXN_POLLNVAL);
#undef POLLFLAGNAME

    file = nxo_stack_push(tstack);
    flag = nxo_stack_push(tstack);
    flags = nxo_stack_push(tstack);

    /* Convert the dict of file/flags pairs to an array of struct pollfd
     * structures. */
    nfds = nxo_dict_count(dict);
    if (nfds == 0)
    {
	nxo_stack_npop(tstack, nxo_stack_count(tstack) - tcount);
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }
    fds = nxa_malloc(nfds * sizeof(struct pollfd));

    /* Iterate through files. */
    for (i = 0; i < nfds; i++)
    {
	nxo_dict_iterate(dict, file);
	nxo_dict_lookup(dict, file, flags);
	if (nxo_type_get(file) != NXOT_FILE || nxo_type_get(flags) != NXOT_DICT)
	{
	    nxa_free(fds, nfds * sizeof(struct pollfd));
	    nxo_stack_npop(tstack, nxo_stack_count(tstack) - tcount);
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}

	fds[i].fd = nxo_file_fd_get(file);
	fds[i].events = 0;
	nflags = nxo_dict_count(flags);
	if (nflags == 0)
	{
	    nxa_free(fds, nfds * sizeof(struct pollfd));
	    nxo_stack_npop(tstack, nxo_stack_count(tstack) - tcount);
	    nxo_thread_nerror(a_thread, NXN_rangecheck);
	    return;
	}

	/* Iterate through flags. */
	for (j = 0; j < nflags; j++)
	{
	    nxo_dict_iterate(flags, flag);
	    if (nxo_type_get(flag) == NXOT_NAME)
	    {
#define ADDPOLLFLAG(a_name, a_flag)					\
		if (nxo_compare(flag, (a_name)) == 0)			\
		{							\
		    fds[i].events |= (a_flag);				\
		    nxo_dict_def(flags, flag, &boolean_false);		\
		}
#define CLEARPOLLERROR(a_name, a_flag)					\
		if (nxo_compare(flag, (a_name)) == 0)			\
		{							\
		    nxo_dict_def(flags, flag, &boolean_false);		\
		}

		/* List these in order of most to least used. */
		ADDPOLLFLAG(pollin, POLLIN)
		else ADDPOLLFLAG(pollout, POLLOUT)
		else ADDPOLLFLAG(pollrdnorm, POLLRDNORM)
		else ADDPOLLFLAG(pollwrnorm, POLLWRNORM)
		else ADDPOLLFLAG(pollrdband, POLLRDBAND)
		else ADDPOLLFLAG(pollwrband, POLLWRBAND)
		else ADDPOLLFLAG(pollpri, POLLPRI)
		else CLEARPOLLERROR(pollerr, POLLERR)
		else CLEARPOLLERROR(pollhup, POLLHUP)
		else CLEARPOLLERROR(pollnval, POLLNVAL);
#undef ADDPOLLFLAG
#undef CLEARPOLLERROR
	    }
	}
    }

    /* Call poll(). */
    while ((nready = poll(fds, nfds, (int) timeout)) == -1 && errno == EINTR)
    {
	/* EINTR; try again. */
    }
    if (nready == -1)
    {
	switch (errno)
	{
	    case ENOMEM:
	    {
		xep_throw(CW_ONYXX_OOM);
	    }
	    default:
	    {
		cw_not_reached();
	    }
	}
    }

    /* Translate the results. */
    nxo_array_new(nxo, nxo_thread_currentlocking(a_thread), nready);
    for (i = j = 0, changed = FALSE; i < nfds && j < nready; i++)
    {
	nxo_dict_iterate(dict, file);
	nxo_dict_lookup(dict, file, flags);
	cw_assert(nxo_file_fd_get(file) == fds[i].fd);

#define CHECKPOLLFLAG(a_var, a_flag)					\
	if ((fds[i].events & (a_flag)) && (fds[i].revents & (a_flag)))	\
	{								\
	    nxo_dict_def(flags, (a_var), &boolean_true);		\
	    nxo_array_el_set(nxo, file, j);				\
	    changed = TRUE;						\
	}
#define CHECKPOLLERROR(a_var, a_flag)					\
	if (fds[i].revents & (a_flag))					\
	{								\
	    nxo_dict_def(flags, (a_var), &boolean_true);		\
	    nxo_array_el_set(nxo, file, j);				\
	    changed = TRUE;						\
	}

	CHECKPOLLFLAG(pollin, POLLIN);
	CHECKPOLLFLAG(pollrdnorm, POLLRDNORM);
	CHECKPOLLFLAG(pollrdband, POLLRDBAND);
	CHECKPOLLFLAG(pollpri, POLLPRI);
	CHECKPOLLFLAG(pollout, POLLOUT);
	CHECKPOLLFLAG(pollwrnorm, POLLWRNORM);
	CHECKPOLLFLAG(pollwrband, POLLWRBAND);
	CHECKPOLLERROR(pollerr, POLLERR);
	CHECKPOLLERROR(pollhup, POLLHUP);
	CHECKPOLLERROR(pollnval, POLLNVAL);

#undef CHECKPOLLFLAG
#undef CHECKPOLLERROR

	    if (changed)
	{
	    changed = FALSE;
	    j++;
	}
    }

    nxo_stack_pop(ostack);
    nxa_free(fds, nfds * sizeof(struct pollfd));
    nxo_stack_npop(tstack, nxo_stack_count(tstack) - tcount);
}
#endif

void
systemdict_pop(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_POP(ostack, a_thread);
}

void
systemdict_pow(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *base, *exp;
    cw_nxoi_t integer_base, integer_exp;
#ifdef CW_REAL
    cw_bool_t do_real;
    cw_nxor_t real_base, real_exp;
#endif

    ostack = nxo_thread_ostack_get(a_thread);
	
    NXO_STACK_GET(exp, ostack, a_thread);
    NXO_STACK_DOWN_GET(base, ostack, a_thread, exp);
    switch (nxo_type_get(base))
    {
	case NXOT_INTEGER:
	{
#ifdef CW_REAL
	    do_real = FALSE;
#endif
	    integer_base = nxo_integer_get(base);
	    break;
	}
#ifdef CW_REAL
	case NXOT_REAL:
	{
	    do_real = TRUE;
	    real_base = nxo_real_get(base);
	    break;
	}
#endif
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }
    switch (nxo_type_get(exp))
    {
	case NXOT_INTEGER:
	{
#ifdef CW_REAL
	    if (do_real)
	    {
		real_exp = (cw_nxor_t) nxo_integer_get(exp);
	    }
	    else
#endif
	    {
		integer_exp = nxo_integer_get(exp);
		if (integer_exp < 0)
		{
#ifdef CW_REAL
		    do_real = TRUE;
		    real_base = (cw_nxor_t) integer_base;
		    real_exp = (cw_nxor_t) integer_exp;
#else
		    /* The rangecheck error for exp is not documented in the
		     * reference manual, since a non-standard configuration is
		     * required for this code to be compiled in. */
		    nxo_thread_nerror(a_thread, NXN_rangecheck);
		    return;
#endif
		}
	    }
	    break;
	}
#ifdef CW_REAL
	case NXOT_REAL:
	{
	    real_exp = nxo_real_get(exp);
	    if (do_real == FALSE)
	    {
		do_real = TRUE;
		real_base = (cw_nxor_t) integer_base;
	    }
	    break;
	}
#endif
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }

#ifdef CW_REAL
    if (do_real)
    {
	/* base may be an integer, so use nxo_real_new() rather than
	 * nxo_real_set(). */
	nxo_real_new(base, pow(real_base, real_exp));
    }
    else
#endif
    {
	cw_uint32_t i;
	cw_nxoi_t r;

	if (integer_exp != 0)
	{
	    for (i = 1, r = integer_base; i < integer_exp; i++)
	    {
		r *= integer_base;
	    }
	}
	else
	{
	    /* base^0 --> 1. */
	    r = 1;
	}

	nxo_integer_set(base, r);
    }

    nxo_stack_pop(ostack);
}

#ifdef CW_POSIX
void
systemdict_ppid(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    nxo = nxo_stack_push(ostack);
    nxo_integer_new(nxo, getppid());
}
#endif

void
systemdict_print(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo, *stdout_nxo;
    cw_nxn_t error;
    cw_bool_t nonblocking;

    ostack = nxo_thread_ostack_get(a_thread);
    stdout_nxo = nxo_thread_stdout_get(a_thread);

    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_STRING)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    nonblocking = nxo_file_nonblocking_get(stdout_nxo);
    if (nonblocking)
    {
	nxo_file_nonblocking_set(stdout_nxo, FALSE);
    }

    nxo_string_lock(nxo);
    error = nxo_file_write(stdout_nxo, nxo_string_get(nxo),
			   nxo_string_len_get(nxo), NULL);
    nxo_string_unlock(nxo);

    if (nonblocking)
    {
	nxo_file_nonblocking_set(stdout_nxo, TRUE);
    }
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }

    nxo_stack_pop(ostack);
}

void
systemdict_put(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *into, *with, *what;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(what, ostack, a_thread);
    NXO_STACK_DOWN_GET(with, ostack, a_thread, what);
    NXO_STACK_DOWN_GET(into, ostack, a_thread, with);

    switch (nxo_type_get(into))
    {
	case NXOT_ARRAY:
	{
	    cw_nxoi_t index;

	    if (nxo_type_get(with) != NXOT_INTEGER)
	    {
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	    }
	    index = nxo_integer_get(with);

	    if (index < 0 || index >= nxo_array_len_get(into))
	    {
		nxo_thread_nerror(a_thread, NXN_rangecheck);
		return;
	    }
	    nxo_array_el_set(into, what, index);
	    break;
	}
	case NXOT_DICT:
	{
	    nxo_dict_def(into, with, what);
	    break;
	}
	case NXOT_STRING:
	{
	    cw_nxoi_t index;
	    cw_uint8_t val;

	    if ((nxo_type_get(with) != NXOT_INTEGER)
		|| nxo_type_get(what) != NXOT_INTEGER)
	    {
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	    }
	    index = nxo_integer_get(with);
	    val = nxo_integer_get(what);

	    if (index < 0 || index >= nxo_string_len_get(into))
	    {
		nxo_thread_nerror(a_thread, NXN_rangecheck);
		return;
	    }
	    nxo_string_el_set(into, val, index);
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }
    nxo_stack_npop(ostack, 3);
}

void
systemdict_putinterval(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *into, *with, *what;
    cw_nxoi_t index;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(what, ostack, a_thread);
    NXO_STACK_DOWN_GET(with, ostack, a_thread, what);
    NXO_STACK_DOWN_GET(into, ostack, a_thread, with);

    if (nxo_type_get(with) != NXOT_INTEGER
	|| nxo_type_get(what) != nxo_type_get(into))
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    index = nxo_integer_get(with);
    if (index < 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    switch (nxo_type_get(into))
    {
	case NXOT_ARRAY:
	{
	    cw_nxo_t *tstack;
	    cw_nxo_t *el;
	    cw_uint32_t i, len;

	    tstack = nxo_thread_tstack_get(a_thread);
	    el = nxo_stack_push(tstack);
	    len = nxo_array_len_get(what);
	    if (index + len > nxo_array_len_get(into))
	    {
		nxo_thread_nerror(a_thread, NXN_rangecheck);
		nxo_stack_pop(tstack);
		return;
	    }
	    for (i = 0; i < len; i++)
	    {
		nxo_array_el_get(what, i, el);
		nxo_array_el_set(into, el, i + index);
	    }
	    nxo_stack_pop(tstack);
	    break;
	}
	case NXOT_STRING:
	{
	    cw_uint8_t *str;
	    cw_uint32_t len;

	    str = nxo_string_get(what);
	    len = nxo_string_len_get(what);
	    if (index + len > nxo_string_len_get(into))
	    {
		nxo_thread_nerror(a_thread, NXN_rangecheck);
		return;
	    }

	    nxo_string_lock(what);
	    nxo_string_lock(into);
	    nxo_string_set(into, index, str, len);
	    nxo_string_unlock(into);
	    nxo_string_unlock(what);
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }
    nxo_stack_npop(ostack, 3);
}

#ifdef CW_POSIX
void
systemdict_pwd(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    char *str;

    str = getcwd(NULL, 0);
    if (str == NULL)
    {
	nxo_thread_nerror(a_thread, NXN_invalidaccess);
	return;
    }

    ostack = nxo_thread_ostack_get(a_thread);
    nxo = nxo_stack_push(ostack);

    nxo_string_new(nxo, nxo_thread_currentlocking(a_thread), strlen(str));
    nxo_string_lock(nxo);
    nxo_string_set(nxo, 0, str, nxo_string_len_get(nxo));
    nxo_string_unlock(nxo);

    free(str);
}
#endif

void
systemdict_quit(cw_nxo_t *a_thread)
{
    xep_throw(CW_ONYXX_QUIT);
}

void
systemdict_rand(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *num;

    ostack = nxo_thread_ostack_get(a_thread);

    num = nxo_stack_push(ostack);
    /* random() returns 31 bits. */
    nxo_integer_new(num,
		    ((cw_nxoi_t) (random() & 1))
		    | (((cw_nxoi_t) random()) << 1)
		    | (((cw_nxoi_t) random()) << 32));
}

void
systemdict_read(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *file;
    cw_uint8_t val;
    cw_sint32_t nread;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(file, ostack, a_thread);
    switch (nxo_type_get(file))
    {
	case NXOT_FILE:
	{
	    cw_nxo_t *value, *code;

	    /* Character read. */
	    value = nxo_stack_push(ostack);
	    code = nxo_stack_push(ostack);
		
	    nread = nxo_file_read(file, 1, &val);
	    if (nread == -1)
	    {
		nxo_stack_npop(ostack, 2);
		nxo_thread_nerror(a_thread, NXN_ioerror);
		return;
	    }

	    if (nread == 0)
	    {
		nxo_integer_new(value, 0);
		nxo_boolean_new(code, TRUE);
	    }
	    else
	    {
		nxo_integer_new(value, (cw_nxoi_t) val);
		nxo_boolean_new(code, FALSE);
	    }

	    nxo_stack_remove(ostack, file);
	    break;
	}
	case NXOT_STRING:
	{
	    cw_nxo_t *string;

	    /* String read. */
	    string = file;
	    NXO_STACK_DOWN_GET(file, ostack, a_thread, string);
	    if (nxo_type_get(file) != NXOT_FILE)
	    {
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	    }

	    nxo_string_lock(string);
	    nread = nxo_file_read(file, nxo_string_len_get(string),
				  nxo_string_get(string));
	    nxo_string_unlock(string);
	    if (nread == -1)
	    {
		nxo_thread_nerror(a_thread, NXN_ioerror);
		return;
	    }

	    if (nread == 0)
	    {
		/* EOF. */
		nxo_boolean_new(file, TRUE);
		nxo_string_new(string, nxo_thread_currentlocking(a_thread), 0);
		nxo_stack_exch(ostack);
	    }
	    else if (nread < nxo_string_len_get(string))
	    {
		cw_nxo_t *value, *code;

		/* We didn't fill the string, so we can't just use it as the
		 * result.  Create a copy. */
		value = nxo_stack_under_push(ostack, file);
		nxo_string_substring_new(value, string, 0, nread);
		code = nxo_stack_under_push(ostack, file);
		nxo_boolean_new(code, FALSE);

		nxo_stack_npop(ostack, 2);
	    }
	    else
	    {
		/* The string is full, so doesn't need modified. */
		nxo_boolean_new(file, FALSE);
		nxo_stack_exch(ostack);
	    }
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }
}

void
systemdict_readline(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack;
    cw_nxo_t *nxo, *tfile;
    cw_nxn_t error;
    cw_bool_t eof;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);

    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_FILE)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    tfile = nxo_stack_push(tstack);
    nxo_dup(tfile, nxo);
    error = nxo_file_readline(tfile, nxo_thread_currentlocking(a_thread), nxo,
			      &eof);
    if (error)
    {
	nxo_stack_pop(tstack);
	nxo_thread_nerror(a_thread, error);
	return;
    }
    nxo_stack_pop(tstack);

    nxo = nxo_stack_push(ostack);
    nxo_boolean_new(nxo, eof);
}

#ifdef CW_POSIX
void
systemdict_readlink(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack, *nxo, *tnxo, *link;
    struct stat sb;
    int error;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_STRING)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    /* Create a copy of the string with an extra byte to store a '\0'
     * terminator. */
    tnxo = nxo_stack_push(tstack);
    nxo_string_cstring(tnxo, nxo, a_thread);

    error = lstat(nxo_string_get(tnxo), &sb);
    if (error == -1)
    {
	nxo_stack_pop(tstack);

	switch (errno)
	{
	    case EACCES:
	    {
		nxo_thread_nerror(a_thread, NXN_invalidaccess);
		return;
	    }
	    case ENOENT:
	    case ENOTDIR:
	    {
		nxo_thread_nerror(a_thread, NXN_undefinedfilename);
		return;
	    }
	    case EIO:
	    {
		nxo_thread_nerror(a_thread, NXN_ioerror);
		return;
	    }
	    case ELOOP:
	    case ENAMETOOLONG:
	    {
		nxo_thread_nerror(a_thread, NXN_invalidfileaccess);
		return;
	    }
	    case EOVERFLOW:
	    default:
	    {
		nxo_thread_nerror(a_thread, NXN_unregistered);
		return;
	    }
	}
    }

    link = nxo_stack_push(ostack);
    nxo_string_new(link, nxo_thread_currentlocking(a_thread), sb.st_size);

    error = readlink(nxo_string_get(tnxo), nxo_string_get(link), sb.st_size);
    if (error == -1)
    {
	nxo_stack_pop(ostack);
	nxo_stack_pop(tstack);

	switch (errno)
	{
	    case EACCES:
	    {
		nxo_thread_nerror(a_thread, NXN_invalidaccess);
		return;
	    }
	    case ENOENT:
	    case ENOTDIR:
	    case EINVAL:
	    {
		nxo_thread_nerror(a_thread, NXN_undefinedfilename);
		return;
	    }
	    case EIO:
	    {
		nxo_thread_nerror(a_thread, NXN_ioerror);
		return;
	    }
	    case ELOOP:
	    case ENAMETOOLONG:
	    {
		nxo_thread_nerror(a_thread, NXN_invalidfileaccess);
		return;
	    }
	    case EFAULT:
	    case ENOMEM:
	    default:
	    {
		nxo_thread_nerror(a_thread, NXN_unregistered);
		return;
	    }
	}
    }

    nxo_stack_remove(ostack, nxo);
    nxo_stack_pop(tstack);
}
#endif

#ifdef CW_POSIX
void
systemdict_realtime(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;
    struct timeval tv;

    ostack = nxo_thread_ostack_get(a_thread);
    nxo = nxo_stack_push(ostack);

    gettimeofday(&tv, NULL);
    nxo_integer_new(nxo, (((cw_nxoi_t) tv.tv_sec * (cw_nxoi_t) 1000000000)
			  + ((cw_nxoi_t) tv.tv_usec * (cw_nxoi_t) 1000)));
}
#endif

#ifdef CW_SOCKET
static const struct cw_systemdict_name_arg send_recv_flag[] =
{
    {NXN_MSG_OOB, MSG_OOB},
    {NXN_MSG_PEEK, MSG_PEEK},
    {NXN_MSG_WAITALL, MSG_WAITALL}
};

void
systemdict_recv(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *sock, *nxo;
    cw_uint32_t npop;
    int flags, nread;

    ostack = nxo_thread_ostack_get(a_thread);

    /* Process flags array, if specified. */
    NXO_STACK_GET(nxo, ostack, a_thread);
    flags = 0;
    if (nxo_type_get(nxo) == NXOT_ARRAY)
    {
	cw_nxo_t *tstack, *el;
	cw_uint32_t i, count, argcnt, argind;

	tstack = nxo_thread_tstack_get(a_thread);
	el = nxo_stack_push(tstack);

	argcnt = sizeof(send_recv_flag) / sizeof(struct cw_systemdict_name_arg);
	for (i = 0, count = nxo_array_len_get(nxo); i < count; i++)
	{
	    nxo_array_el_get(nxo, i, el);
	    argind = systemdict_p_name_arg(nxo, send_recv_flag, argcnt);
	    if (argind == argcnt)
	    {
		nxo_stack_pop(tstack);
		nxo_thread_nerror(a_thread, NXN_argcheck);
		return;
	    }
	    flags |= send_recv_flag[argind].arg;
	}

	nxo_stack_pop(tstack);

	/* Get string. */
	NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
	npop = 2;
    }
    else
    {
	npop = 1;
    }
    NXO_STACK_DOWN_GET(sock, ostack, a_thread, nxo);
    if (nxo_type_get(sock) != NXOT_FILE || nxo_type_get(nxo) != NXOT_STRING)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    nxo_string_lock(nxo);
    while ((nread = recv(nxo_file_fd_get(sock), nxo_string_get(nxo),
		 nxo_string_len_get(nxo), flags)) == -1 && errno == EINTR)
    {
	/* EINTR; try again. */
    }
    nxo_string_unlock(nxo);

    if (nread == -1)
    {
	switch (errno)
	{
	    case EWOULDBLOCK:
	    {
		/* Failed non-blocking read. */
		nread = 0;
		break;
	    }
	    case ENOTCONN:
	    {
		nxo_thread_nerror(a_thread, NXN_neterror);
		return;
	    }
	    case ENOTSOCK:
	    {
		nxo_thread_nerror(a_thread, NXN_argcheck);
		return;
	    }
	    case EBADF:
	    case EFAULT:
	    case EINVAL:
	    default:
	    {
		nxo_thread_nerror(a_thread, NXN_unregistered);
		return;
	    }
	}
    }

    if (nread < nxo_string_len_get(nxo))
    {
	/* Create a substring. */
	nxo_string_substring_new(sock, nxo, 0, (cw_uint32_t) nread);
    }
    else
    {
	/* Entire string filled; return it as-is. */
	nxo_dup(sock, nxo);
    }

    nxo_stack_npop(ostack, npop);
}
#endif

#ifdef CW_POSIX
void
systemdict_rename(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack;
    cw_nxo_t *from, *to, *tfrom, *tto;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    NXO_STACK_GET(to, ostack, a_thread);
    NXO_STACK_DOWN_GET(from, ostack, a_thread, to);

    if (nxo_type_get(from) != NXOT_STRING
	|| nxo_type_get(to) != NXOT_STRING)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    if (nxo_string_len_get(from) >= PATH_MAX
	|| nxo_string_len_get(to) >= PATH_MAX)
    {
	nxo_thread_nerror(a_thread, NXN_limitcheck);
	return;
    }

    tto = nxo_stack_push(tstack);
    nxo_string_cstring(tto, to, a_thread);
	
    tfrom = nxo_stack_push(tstack);
    nxo_string_cstring(tfrom, from, a_thread);

    if (rename(nxo_string_get(tfrom), nxo_string_get(tto)) == -1)
    {
	switch (errno)
	{
	    case EACCES:
	    case EPERM:
	    case EROFS:
	    case EINVAL:
	    {
		nxo_thread_nerror(a_thread, NXN_invalidfileaccess);
	    }
	    case ENAMETOOLONG:
	    case ENOENT:
	    case ENOTDIR:
	    {
		nxo_thread_nerror(a_thread, NXN_undefinedfilename);
	    }
	    default:
	    {
		nxo_thread_nerror(a_thread, NXN_ioerror);
	    }
	}
	nxo_stack_npop(tstack, 2);
	return;
    }

    nxo_stack_npop(tstack, 2);
    nxo_stack_npop(ostack, 2);
}
#endif

#ifdef CW_REGEX
void
systemdict_regex(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *pattern, *nxo;
    cw_nxn_t error;
    cw_uint32_t npop;
    cw_bool_t cont, global, insensitive, multiline, singleline;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(pattern, ostack, a_thread);
    switch (nxo_type_get(pattern))
    {
	case NXOT_DICT:
	{
	    cw_nxo_t *flags;

	    flags = pattern;
	    npop = 2;
	    NXO_STACK_DOWN_GET(pattern, ostack, a_thread, pattern);
	    if (nxo_type_get(pattern) != NXOT_STRING)
	    {
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	    }

	    error = systemdict_p_regex_flags_get(flags, a_thread, &cont,
						 &global, &insensitive,
						 &multiline, &singleline);
	    if (error)
	    {
		nxo_thread_nerror(a_thread, error);
		return;
	    }
	    break;
	}
	case NXOT_STRING:
	{
	    npop = 1;

	    cont = FALSE;
	    global = FALSE;
	    insensitive = FALSE;
	    multiline = FALSE;
	    singleline = FALSE;
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }

    /* Create the regex object. */
    nxo = nxo_stack_under_push(ostack, pattern);
    nxo_string_lock(pattern);
    error = nxo_regex_new(nxo, nxo_string_get(pattern),
			  nxo_string_len_get(pattern), cont, global,
			  insensitive, multiline, singleline);
    nxo_string_unlock(pattern);
    if (error)
    {
	nxo_stack_remove(ostack, nxo);
	nxo_thread_nerror(a_thread, error);
	return;
    }

    nxo_stack_npop(ostack, npop);
}
#endif

#ifdef CW_REGEX
void
systemdict_regsub(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *pattern, *template, *nxo;
    cw_nxn_t error;
    cw_uint32_t npop;
    cw_bool_t global, insensitive, multiline, singleline;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(template, ostack, a_thread);
    switch (nxo_type_get(template))
    {
	case NXOT_DICT:
	{
	    cw_nxo_t *flags;

	    flags = template;
	    npop = 3;
	    NXO_STACK_DOWN_GET(template, ostack, a_thread, template);
	    if (nxo_type_get(template) != NXOT_STRING)
	    {
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	    }

	    error = systemdict_p_regex_flags_get(flags, a_thread, NULL,
						 &global, &insensitive,
						 &multiline, &singleline);
	    if (error)
	    {
		nxo_thread_nerror(a_thread, error);
		return;
	    }
	    break;
	}
	case NXOT_STRING:
	{
	    npop = 2;

	    global = FALSE;
	    insensitive = FALSE;
	    multiline = FALSE;
	    singleline = FALSE;
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }
    NXO_STACK_DOWN_GET(pattern, ostack, a_thread, template);
    if (nxo_type_get(pattern) != NXOT_STRING)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    /* Create the regex object. */
    nxo = nxo_stack_under_push(ostack, pattern);
    nxo_string_lock(pattern);
    nxo_string_lock(template);
    error = nxo_regsub_new(nxo, nxo_string_get(pattern),
			   nxo_string_len_get(pattern), global,
			   insensitive, multiline, singleline,
			   nxo_string_get(template),
			   nxo_string_len_get(template));
    nxo_string_unlock(template);
    nxo_string_unlock(pattern);
    if (error)
    {
	nxo_stack_remove(ostack, nxo);
	nxo_thread_nerror(a_thread, error);
	return;
    }

    nxo_stack_npop(ostack, npop);
}
#endif

void
systemdict_repeat(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *estack, *tstack;
    cw_nxo_t *count, *exec, *nxo, *tnxo;
    cw_nxoi_t i, cnt;
    cw_uint32_t edepth, tdepth;

    ostack = nxo_thread_ostack_get(a_thread);
    estack = nxo_thread_estack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);

    NXO_STACK_GET(exec, ostack, a_thread);
    NXO_STACK_DOWN_GET(count, ostack, a_thread, exec);
    if (nxo_type_get(count) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    cnt = nxo_integer_get(count);
    if (cnt < 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    tnxo = nxo_stack_push(tstack);
    nxo_dup(tnxo, exec);

    nxo_stack_npop(ostack, 2);

    /* Record stack depths so that we can clean up if necessary. */
    edepth = nxo_stack_count(estack);
    tdepth = nxo_stack_count(tstack);

    /* Catch an exit exception, if thrown, but do not continue executing the
     * loop. */
    xep_begin();
    xep_try
    {
	for (i = 0; i < cnt; i++)
	{
	    nxo = nxo_stack_push(estack);
	    nxo_dup(nxo, tnxo);
	    nxo_thread_loop(a_thread);
	}
    }
    xep_catch(CW_ONYXX_EXIT)
    {
	cw_nxo_t *istack;

	xep_handled();

	/* Clean up stacks. */
	nxo_stack_npop(estack, nxo_stack_count(estack) - edepth);
	istack = nxo_thread_istack_get(a_thread);
	nxo_stack_npop(istack, nxo_stack_count(istack)
		       - nxo_stack_count(estack));
	nxo_stack_npop(tstack, nxo_stack_count(tstack) - tdepth);
    }
    xep_end();

    /* An object is pushed before tdepth is stored, so we can unconditionally
     * pop it here. */
    nxo_stack_pop(tstack);
}

#ifdef CW_POSIX
void
systemdict_rmdir(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack, *path, *tpath;
    int error;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    NXO_STACK_GET(path, ostack, a_thread);
    if (nxo_type_get(path) != NXOT_STRING)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    /* Create a copy of path with an extra byte to store a '\0' terminator. */
    tpath = nxo_stack_push(tstack);
    nxo_string_cstring(tpath, path, a_thread);

    error = rmdir(nxo_string_get(tpath));

    nxo_stack_pop(tstack);

    if (error == -1)
    {
	switch (errno)
	{
	    case EBUSY:
	    case EIO:
	    case ENOTEMPTY:
	    case EROFS:
	    {
		nxo_thread_nerror(a_thread, NXN_ioerror);
		break;
	    }
	    case EACCES:
	    case ELOOP:
	    case ENOENT:
	    case ENOTDIR:
	    case ENAMETOOLONG:
	    case EPERM:
	    {
		nxo_thread_nerror(a_thread, NXN_invalidfileaccess);
		break;
	    }
	    case EFAULT:
	    default:
	    {
		nxo_thread_nerror(a_thread, NXN_unregistered);
	    }
	}
	return;
    }

    nxo_stack_pop(ostack);
}
#endif

void
systemdict_roll(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;
    cw_nxoi_t count, amount;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    amount = nxo_integer_get(nxo);
    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    count = nxo_integer_get(nxo);
    if (count < 1)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    nxo_stack_npop(ostack, 2);
    if (nxo_stack_roll(ostack, count, amount))
    {
	cw_nxo_t *nxo;

	/* Stack underflow.  Restore the stack to its original state, then throw
	 * an error. */
	nxo = nxo_stack_push(ostack);
	nxo_integer_new(nxo, count);
	nxo = nxo_stack_push(ostack);
	nxo_integer_new(nxo, amount);

	nxo_thread_nerror(a_thread, NXN_stackunderflow);
    }
}

void
systemdict_rot(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxoi_t amount;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    amount = nxo_integer_get(nxo);
    if (nxo_stack_count(ostack) < 2)
    {
	nxo_thread_nerror(a_thread, NXN_stackunderflow);
	return;
    }
    nxo_stack_pop(ostack);
    nxo_stack_rot(ostack, amount);
}

#ifdef CW_REAL
void
systemdict_round(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    switch (nxo_type_get(nxo))
    {
	case NXOT_INTEGER:
	{
	    break;
	}
	case NXOT_REAL:
	{
	    nxo_integer_new(nxo, (cw_nxoi_t) rint(nxo_real_get(nxo)));
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }
}
#endif

void
systemdict_sadn(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *stack, *nxo, *bnxo;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(stack, ostack, a_thread);
    if (nxo_type_get(stack) != NXOT_STACK)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    NXO_STACK_BGET(bnxo, stack, a_thread);
    nxo = nxo_stack_push(stack);
    nxo_dup(nxo, bnxo);
    nxo_stack_bpop(stack);

    nxo_stack_pop(ostack);
}

void
systemdict_saup(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *stack, *nxo, *bnxo;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(stack, ostack, a_thread);
    if (nxo_type_get(stack) != NXOT_STACK)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    
    NXO_STACK_GET(nxo, stack, a_thread);
    bnxo = nxo_stack_bpush(stack);
    nxo_dup(bnxo, nxo);
    nxo_stack_pop(stack);

    nxo_stack_pop(ostack);
}

void
systemdict_sbdup(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *stack, *orig, *dup;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(stack, ostack, a_thread);
    if (nxo_type_get(stack) != NXOT_STACK)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    NXO_STACK_BGET(orig, stack, a_thread);
    dup = nxo_stack_push(stack);
    nxo_dup(dup, orig);

    nxo_stack_pop(ostack);
}

void
systemdict_sbpop(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *stack, *snxo, *onxo;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(stack, ostack, a_thread);
    if (nxo_type_get(stack) != NXOT_STACK)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    NXO_STACK_BGET(snxo, stack, a_thread);
    onxo = nxo_stack_under_push(ostack, stack);
    nxo_dup(onxo, snxo);

    nxo_stack_bpop(stack);
    nxo_stack_pop(ostack);
}

void
systemdict_sbpush(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo, *stack, *nnxo;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    NXO_STACK_DOWN_GET(stack, ostack, a_thread, nxo);
    if (nxo_type_get(stack) != NXOT_STACK)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    nnxo = nxo_stack_bpush(stack);
    nxo_dup(nnxo, nxo);
    nxo_stack_npop(ostack, 2);
}

void
systemdict_sclear(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *stack;
    cw_uint32_t count;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(stack, ostack, a_thread);
    if (nxo_type_get(stack) != NXOT_STACK)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    count = nxo_stack_count(stack);
    if (count > 0)
    {
	nxo_stack_npop(stack, count);
    }

    nxo_stack_pop(ostack);
}

void
systemdict_scleartomark(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *stack, *nxo;
    cw_uint32_t i, depth;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(stack, ostack, a_thread);
    if (nxo_type_get(stack) != NXOT_STACK)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    for (i = 0, depth = nxo_stack_count(stack), nxo = NULL; i < depth; i++)
    {
	nxo = nxo_stack_down_get(stack, nxo);
	if (nxo == NULL)
	{
	    nxo_thread_nerror(a_thread, NXN_unmatchedmark);
	    return;
	}
	if (nxo_type_get(nxo) == NXOT_MARK)
	{
	    break;
	}
    }
    if (i == depth)
    {
	nxo_thread_nerror(a_thread, NXN_unmatchedmark);
	return;
    }

    nxo_stack_npop(stack, i + 1);

    nxo_stack_pop(ostack);
}

void
systemdict_scount(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_STACK)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    nxo_integer_new(nxo, nxo_stack_count(nxo));
}

void
systemdict_scounttomark(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *stack, *nxo;
    cw_uint32_t i;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(stack, ostack, a_thread);
    if (nxo_type_get(stack) != NXOT_STACK)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    for (i = 0, nxo = nxo_stack_get(stack);
	 nxo != NULL && nxo_type_get(nxo) != NXOT_MARK;
	 i++, nxo = nxo_stack_down_get(stack, nxo))
    {
	/* Do nothing. */
    }

    if (nxo == NULL)
    {
	nxo_thread_nerror(a_thread, NXN_unmatchedmark);
	return;
    }

    nxo_integer_new(stack, i);
}

void
systemdict_sdn(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *stack;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(stack, ostack, a_thread);
    if (nxo_type_get(stack) != NXOT_STACK)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    if (nxo_stack_count(stack) < 3)
    {
	nxo_thread_nerror(a_thread, NXN_stackunderflow);
	return;
    }

    nxo_stack_roll(stack, 3, -1);

    nxo_stack_pop(ostack);
}

void
systemdict_sdup(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *stack, *orig, *dup;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(stack, ostack, a_thread);
    if (nxo_type_get(stack) != NXOT_STACK)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    NXO_STACK_GET(orig, stack, a_thread);
    dup = nxo_stack_push(stack);
    nxo_dup(dup, orig);

    nxo_stack_pop(ostack);
}

#ifdef CW_POSIX
void
systemdict_seek(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *file, *position;
    cw_nxn_t error;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(position, ostack, a_thread);
    NXO_STACK_DOWN_GET(file, ostack, a_thread, position);
	
    if (nxo_type_get(file) != NXOT_FILE
	|| nxo_type_get(position) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    error = nxo_file_position_set(file, nxo_integer_get(position));
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }

    nxo_stack_npop(ostack, 2);
}
#endif

void
systemdict_self(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *thread;

    ostack = nxo_thread_ostack_get(a_thread);
    thread = nxo_stack_push(ostack);
    nxo_dup(thread, a_thread);
}

#ifdef CW_SOCKET
void
systemdict_send(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *sock, *nxo;
    cw_uint8_t *str;
    cw_uint32_t npop, len;
    int fd, flags;
    ssize_t count, nwritten;
    cw_bool_t blocking;

    ostack = nxo_thread_ostack_get(a_thread);

    /* Process flags array, if specified. */
    NXO_STACK_GET(nxo, ostack, a_thread);
    flags = 0;
    if (nxo_type_get(nxo) == NXOT_ARRAY)
    {
	cw_nxo_t *tstack, *el;
	cw_uint32_t i, count, argcnt, argind;

	tstack = nxo_thread_tstack_get(a_thread);
	el = nxo_stack_push(tstack);

	argcnt = sizeof(send_recv_flag) / sizeof(struct cw_systemdict_name_arg);
	for (i = 0, count = nxo_array_len_get(nxo); i < count; i++)
	{
	    nxo_array_el_get(nxo, i, el);
	    argind = systemdict_p_name_arg(nxo, send_recv_flag, argcnt);
	    if (argind == argcnt)
	    {
		nxo_stack_pop(tstack);
		nxo_thread_nerror(a_thread, NXN_argcheck);
		return;
	    }
	    flags |= send_recv_flag[argind].arg;
	}

	nxo_stack_pop(tstack);

	/* Get string. */
	NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
	npop = 2;
    }
    else
    {
	npop = 1;
    }
    NXO_STACK_DOWN_GET(sock, ostack, a_thread, nxo);
    if (nxo_type_get(sock) != NXOT_FILE || nxo_type_get(nxo) != NXOT_STRING)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    fd = nxo_file_fd_get(sock);
    blocking = (fcntl(fd, F_GETFL, O_NONBLOCK) & O_NONBLOCK) ? TRUE : FALSE;
    str = nxo_string_get(nxo);
    len = nxo_string_len_get(nxo);
    nwritten = 0;
    nxo_string_lock(nxo);
    do
    {
	while ((count = send(fd, &str[nwritten], len - nwritten, flags)) == -1)
	{
	    switch (errno)
	    {
		case EINTR:
		{
		    /* Interrupted system call. */
		    break;
		}
		case EWOULDBLOCK:
		{
		    /* Failed non-blocking write. */
		    goto OUT;
		}
		case ECONNREFUSED:
		case EHOSTDOWN:
		case EHOSTUNREACH:
		case EMSGSIZE:
		case ENOTCONN:
		{
		    nxo_string_unlock(nxo);
		    nxo_thread_nerror(a_thread, NXN_neterror);
		    return;
		}
		case ENOTSOCK:
		{
		    nxo_string_unlock(nxo);
		    nxo_thread_nerror(a_thread, NXN_argcheck);
		    return;
		}
		case EBADF:
		case EFAULT:
		case ENOBUFS:
		case EINVAL:
		case ENOMEM:
		case EPIPE:
		default:
		{
		    nxo_string_unlock(nxo);
		    nxo_thread_nerror(a_thread, NXN_unregistered);
		    return;
		}
	    }
	}
	nwritten += count;
    } while (nwritten < len && blocking);
    OUT:
    nxo_string_unlock(nxo);

    nxo_integer_new(sock, (cw_nxoi_t) nwritten);

    nxo_stack_npop(ostack, npop);
}
#endif

#ifdef CW_SOCKET
void
systemdict_serviceport(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack, *nxo, *tnxo;
    struct servent *ent;
    cw_nxoi_t port;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_STRING)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    tnxo = nxo_stack_push(tstack);
    nxo_string_cstring(tnxo, nxo, a_thread);

#ifdef CW_THREADS
    mtx_lock(&cw_g_getservbyname_mtx);
#endif
    setservent(0);
    ent = getservbyname(nxo_string_get(tnxo), NULL);
    if (ent == NULL)
    {
	port = 0;
    }
    else
    {
	port = ntohs(ent->s_port);
    }
    endservent();
#ifdef CW_THREADS
    mtx_unlock(&cw_g_getservbyname_mtx);
#endif
    
    nxo_stack_pop(tstack);
    nxo_integer_new(nxo, port);
}
#endif

#ifdef CW_POSIX
void
systemdict_setegid(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxoi_t egid;
    int error;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);;
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    egid = nxo_integer_get(nxo);
    if (egid < 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    error = setegid((gid_t) egid);
    nxo_boolean_new(nxo, error == 0 ? FALSE : TRUE);
}
#endif

#ifdef CW_POSIX
void
systemdict_setenv(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack, *envdict;
    cw_nxo_t *key, *val, *tnxo;
    cw_uint32_t klen, vlen;
    const cw_uint8_t *str;
    cw_uint8_t *tstr;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    envdict = libonyx_envdict_get();
    NXO_STACK_GET(val, ostack, a_thread);
    NXO_STACK_DOWN_GET(key, ostack, a_thread, val);
    if (nxo_type_get(key) != NXOT_NAME)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    if (nxo_type_get(val) != NXOT_STRING)
    {
	systemdict_cvs(a_thread);
	/* Get val again, since it may have changed. */
	val = nxo_stack_get(ostack);
    }

    /* Create a new string big enough for: KEY=val\0 */
    klen = nxo_name_len_get(key);
    vlen = nxo_string_len_get(val);
    tnxo = nxo_stack_push(tstack);
    nxo_string_new(tnxo, nxo_thread_currentlocking(a_thread),
		   klen + vlen + 2);

    /* Copy the key and value. */
    tstr = nxo_string_get(tnxo);
    str = nxo_name_str_get(key);
    memcpy(tstr, str, klen);

    tstr[klen] = '=';

    str = nxo_string_get(val);
    nxo_string_lock(val);
    memcpy(&tstr[klen + 1], str, vlen);
    nxo_string_unlock(val);

    tstr[klen + vlen + 1] = '\0';

    /* Do the putenv(). */
    if (putenv(tstr) == -1)
    {
	xep_throw(CW_ONYXX_OOM);
    }
    nxo_stack_pop(tstack);

    /* Insert the key/value pair into envdict. */
    nxo_dict_def(envdict, key, val);

    nxo_stack_npop(ostack, 2);
}
#endif

#ifdef CW_POSIX
void
systemdict_seteuid(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxoi_t euid;
    int error;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);;
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    euid = nxo_integer_get(nxo);
    if (euid < 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    error = seteuid((uid_t) euid);
    nxo_boolean_new(nxo, error == 0 ? FALSE : TRUE);
}
#endif

#ifdef CW_POSIX
void
systemdict_setgid(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxoi_t gid;
    int error;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);;
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    gid = nxo_integer_get(nxo);
    if (gid < 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    error = setgid((gid_t) gid);
    nxo_boolean_new(nxo, error == 0 ? FALSE : TRUE);
}
#endif

#ifdef CW_THREADS
void
systemdict_setgstderr(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *file;
    
    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(file, ostack, a_thread);
    if (nxo_type_get(file) != NXOT_FILE)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    
    nx_stderr_set(nxo_thread_nx_get(a_thread), file);

    nxo_stack_pop(ostack);
}
#endif

#ifdef CW_THREADS
void
systemdict_setgstdin(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *file;
    
    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(file, ostack, a_thread);
    if (nxo_type_get(file) != NXOT_FILE)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    
    nx_stdin_set(nxo_thread_nx_get(a_thread), file);

    nxo_stack_pop(ostack);
}
#endif

#ifdef CW_THREADS
void
systemdict_setgstdout(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *file;
    
    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(file, ostack, a_thread);
    if (nxo_type_get(file) != NXOT_FILE)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    
    nx_stdout_set(nxo_thread_nx_get(a_thread), file);

    nxo_stack_pop(ostack);
}
#endif

void
systemdict_setiobuf(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *file, *iobuf;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(iobuf, ostack, a_thread);
    NXO_STACK_DOWN_GET(file, ostack, a_thread, iobuf);
    if (nxo_type_get(file) != NXOT_FILE || nxo_type_get(iobuf) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    nxo_file_buffer_size_set(file, nxo_integer_get(iobuf));
    nxo_stack_npop(ostack, 2);
}

#ifdef CW_THREADS
void
systemdict_setlocking(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_BOOLEAN)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    nxo_thread_setlocking(a_thread, nxo_boolean_get(nxo));
    nxo_stack_pop(ostack);
}
#endif

void
systemdict_setnonblocking(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *file, *nonblocking;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nonblocking, ostack, a_thread);
    NXO_STACK_DOWN_GET(file, ostack, a_thread, nonblocking);
    if (nxo_type_get(file) != NXOT_FILE
	|| nxo_type_get(nonblocking) != NXOT_BOOLEAN)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    if (nxo_file_nonblocking_set(file, nxo_boolean_get(nonblocking)))
    {
	nxo_thread_nerror(a_thread, NXN_ioerror);
	return;
    }

    nxo_stack_npop(ostack, 2);
}

#ifdef CW_SOCKET
static const struct cw_systemdict_name_arg sock_opt[] =
{
    {NXN_SO_DEBUG, SO_DEBUG},
    {NXN_SO_REUSEADDR, SO_REUSEADDR},
#ifdef SO_REUSEPORT
    {NXN_SO_REUSEPORT, SO_REUSEPORT},
#endif
    {NXN_SO_KEEPALIVE, SO_KEEPALIVE},
    {NXN_SO_DONTROUTE, SO_DONTROUTE},
    {NXN_SO_LINGER, SO_LINGER},
    {NXN_SO_BROADCAST, SO_BROADCAST},
    {NXN_SO_OOBINLINE, SO_OOBINLINE},
    {NXN_SO_SNDBUF, SO_SNDBUF},
    {NXN_SO_RCVBUF, SO_RCVBUF},
    {NXN_SO_SNDLOWAT, SO_SNDLOWAT},
    {NXN_SO_RCVLOWAT, SO_RCVLOWAT},
    {NXN_SO_SNDTIMEO, SO_SNDTIMEO},
    {NXN_SO_RCVTIMEO, SO_RCVTIMEO},
    {NXN_SO_TYPE, SO_TYPE},
    {NXN_SO_ERROR, SO_ERROR}
};

static const struct cw_systemdict_name_arg sock_opt_level[] =
{
    {NXN_SOL_SOCKET, SOL_SOCKET}
};

static void
systemdict_p_sockopt(cw_nxo_t *a_thread, cw_bool_t a_set)
{
    cw_nxo_t *ostack, *nxo, *nxoval;
    cw_uint32_t argcnt, argind, npop;
    int opt, level;
    union
    {
	int i;
	struct linger l;
	struct timeval t;
    } optval;

    ostack = nxo_thread_ostack_get(a_thread);

    if (a_set)
    {
	/* Get option value. */
	NXO_STACK_GET(nxoval, ostack, a_thread);
	NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxoval);
	npop = 1;
    }
    else
    {
	NXO_STACK_GET(nxo, ostack, a_thread);
	npop = 0;
    }

    /* Get option name. */
    if (nxo_type_get(nxo) != NXOT_NAME)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    argcnt = sizeof(sock_opt) / sizeof(struct cw_systemdict_name_arg);
    argind = systemdict_p_name_arg(nxo, sock_opt, argcnt);
    if (argind == argcnt)
    {
	nxo_thread_nerror(a_thread, NXN_argcheck);
	return;
    }
    opt = sock_opt[argind].arg;

    /* Get level. */
    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    if (nxo_type_get(nxo) == NXOT_NAME)
    {
	/* If level is $SOL_SOCKET, simply use SOL_SOCKET.  Otherwise, use the
	 * name as a protocol to look up the associated protocol number. */
	argcnt = sizeof(sock_opt_level) / sizeof(struct cw_systemdict_name_arg);
	argind = systemdict_p_name_arg(nxo, sock_opt_level, argcnt);
	if (argind != argcnt)
	{
	    level = SOL_SOCKET;
	}
	else
	{
	    cw_nxo_t *tstack, *tnxo;
	    struct protoent *ent;

	    tstack = nxo_thread_tstack_get(a_thread);
	    tnxo = nxo_stack_push(tstack);
	    nxo_string_cstring(tnxo, nxo, a_thread);

#ifdef CW_THREADS
	    mtx_lock(&cw_g_getprotobyname_mtx);
#endif
	    setprotoent(0);
	    ent = getprotobyname(nxo_string_get(tnxo));

	    nxo_stack_pop(tstack);

	    if (ent == NULL)
	    {
		/* Not a socket protocol. */
		endprotoent();
#ifdef CW_THREADS
		mtx_unlock(&cw_g_getprotobyname_mtx);
#endif
		nxo_thread_nerror(a_thread, NXN_argcheck);
		return;
	    }

	    level = ent->p_proto;
	}

	NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
	if (nxo_type_get(nxo) != NXOT_FILE)
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
	npop += 2;
    }
    else if (nxo_type_get(nxo) == NXOT_FILE)
    {
	level = SOL_SOCKET;
	npop++;
    }
    else
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    /* nxo is the socket. */

    if (a_set)
    {
	npop++;

	switch (opt)
	{
	    case SO_DEBUG:
	    case SO_REUSEADDR:
#ifdef SO_REUSEPORT
	    case SO_REUSEPORT:
#endif
	    case SO_KEEPALIVE:
	    case SO_DONTROUTE:
	    case SO_BROADCAST:
	    case SO_OOBINLINE:
	    case SO_SNDBUF:
	    case SO_RCVBUF:
	    case SO_SNDLOWAT:
	    case SO_RCVLOWAT:
	    case SO_TYPE:
	    case SO_ERROR:
	    {
		if (nxo_type_get(nxoval) != NXOT_INTEGER)
		{
		    nxo_thread_nerror(a_thread, NXN_typecheck);
		    return;
		}

		optval.i = nxo_integer_get(nxoval);

		break;
	    }
	    case SO_LINGER:
	    {
		cw_nxo_t *tstack, *tkey, *tval;

		if (nxo_type_get(nxoval) != NXOT_DICT)
		{
		    nxo_thread_nerror(a_thread, NXN_typecheck);
		    return;
		}

		tstack = nxo_thread_tstack_get(a_thread);
		tkey = nxo_stack_push(tstack);
		tval = nxo_stack_push(tstack);

		
		/* on. */
		nxo_name_new(tkey, nxn_str(NXN_on), nxn_len(NXN_on), TRUE);
		if (nxo_dict_lookup(nxoval, tkey, tval))
		{
		    nxo_stack_npop(tstack, 2);
		    nxo_thread_nerror(a_thread, NXN_argcheck);
		    return;
		}
		if (nxo_type_get(tval) != NXOT_BOOLEAN)
		{
		    nxo_stack_npop(tstack, 2);
		    nxo_thread_nerror(a_thread, NXN_typecheck);
		    return;
		}		    
		optval.l.l_onoff = nxo_boolean_get(tval);

		/* time. */
		nxo_name_new(tkey, nxn_str(NXN_time), nxn_len(NXN_time), TRUE);
		if (nxo_dict_lookup(nxoval, tkey, tval))
		{
		    nxo_stack_npop(tstack, 2);
		    nxo_thread_nerror(a_thread, NXN_argcheck);
		    return;
		}
		if (nxo_type_get(tval) != NXOT_INTEGER)
		{
		    nxo_stack_npop(tstack, 2);
		    nxo_thread_nerror(a_thread, NXN_typecheck);
		    return;
		}		    
		optval.l.l_linger = nxo_integer_get(tval);

		nxo_stack_npop(tstack, 2);
		break;
	    }
	    case SO_SNDTIMEO:
	    case SO_RCVTIMEO:
	    {
		if (nxo_type_get(nxoval) != NXOT_INTEGER)
		{
		    nxo_thread_nerror(a_thread, NXN_typecheck);
		    return;
		}

		optval.t.tv_sec = nxo_integer_get(nxoval) / 1000000000LL;
		optval.t.tv_usec = (nxo_integer_get(nxoval) % 1000000000LL)
		    / 1000;
		break;
	    }
	    default:
	    {
		cw_not_reached();
	    }
	}

	if (setsockopt(nxo_file_fd_get(nxo), level, opt, &optval,
		       sizeof(optval)) == -1)
	{
	    switch (errno)
	    {
		case ENOTSOCK:
		case ENOPROTOOPT:
		{
		    nxo_thread_nerror(a_thread, NXN_argcheck);
		    break;
		}
		case EBADF:
		case EFAULT:
		default:
		{
		    nxo_thread_nerror(a_thread, NXN_unregistered);
		}
	    }
	    return;
	}
    }
    else
    {
	socklen_t optlen;

	optlen = sizeof(optval);
	if (getsockopt(nxo_file_fd_get(nxo), level, opt, &optval, &optlen)
	    == -1)
	{
	    switch (errno)
	    {
		case ENOTSOCK:
		case ENOPROTOOPT:
		{
		    nxo_thread_nerror(a_thread, NXN_argcheck);
		    break;
		}
		case EBADF:
		case EFAULT:
		default:
		{
		    nxo_thread_nerror(a_thread, NXN_unregistered);
		}
	    }
	    return;
	}

	switch (opt)
	{
	    case SO_DEBUG:
	    case SO_REUSEADDR:
#ifdef SO_REUSEPORT
	    case SO_REUSEPORT:
#endif
	    case SO_KEEPALIVE:
	    case SO_DONTROUTE:
	    case SO_BROADCAST:
	    case SO_OOBINLINE:
	    case SO_SNDBUF:
	    case SO_RCVBUF:
	    case SO_SNDLOWAT:
	    case SO_RCVLOWAT:
	    case SO_TYPE:
	    case SO_ERROR:
	    {
		nxo_integer_new(nxo, optval.i);
		break;
	    }
	    case SO_LINGER:
	    {
		cw_nxo_t *tstack, *tkey, *tval;

		tstack = nxo_thread_tstack_get(a_thread);
		tkey = nxo_stack_push(tstack);
		tval = nxo_stack_push(tstack);

		nxo_dict_new(nxo, nxo_thread_currentlocking(a_thread), 2);

		/* on. */
		nxo_name_new(tkey, nxn_str(NXN_on), nxn_len(NXN_on), TRUE);
		nxo_boolean_new(tval, optval.l.l_onoff ? TRUE : FALSE);
		nxo_dict_def(nxo, tkey, tval);

		/* time. */
		nxo_name_new(tkey, nxn_str(NXN_time), nxn_len(NXN_time), TRUE);
		nxo_integer_new(tval, optval.l.l_linger);
		nxo_dict_def(nxo, tkey, tval);

		nxo_stack_npop(tstack, 2);
		break;
	    }
	    case SO_SNDTIMEO:
	    case SO_RCVTIMEO:
	    {
		nxo_integer_new(nxo,
				(((cw_nxoi_t) optval.t.tv_sec
				  * (cw_nxoi_t) 1000000000)
				 + ((cw_nxoi_t) optval.t.tv_usec
				    * (cw_nxoi_t) 1000)));
		break;
	    }
	    default:
	    {
		cw_not_reached();
	    }
	}
    }

    nxo_stack_npop(ostack, npop);
}

void
systemdict_setsockopt(cw_nxo_t *a_thread)
{
    systemdict_p_sockopt(a_thread, TRUE);
}
#endif

void
systemdict_setstderr(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *file;
    
    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(file, ostack, a_thread);
    if (nxo_type_get(file) != NXOT_FILE)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    
    nxo_thread_stderr_set(a_thread, file);

    nxo_stack_pop(ostack);
}

void
systemdict_setstdin(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *file;
    
    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(file, ostack, a_thread);
    if (nxo_type_get(file) != NXOT_FILE)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    
    nxo_thread_stdin_set(a_thread, file);

    nxo_stack_pop(ostack);
}

void
systemdict_setstdout(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *file;
    
    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(file, ostack, a_thread);
    if (nxo_type_get(file) != NXOT_FILE)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    
    nxo_thread_stdout_set(a_thread, file);

    nxo_stack_pop(ostack);
}

#ifdef CW_POSIX
void
systemdict_setuid(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxoi_t uid;
    int error;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);;
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    uid = nxo_integer_get(nxo);
    if (uid < 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    error = setuid((uid_t) uid);
    nxo_boolean_new(nxo, error == 0 ? FALSE : TRUE);
}
#endif

void
systemdict_sexch(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *stack;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(stack, ostack, a_thread);
    if (nxo_type_get(stack) != NXOT_STACK)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    if (nxo_stack_exch(stack))
    {
	nxo_thread_nerror(a_thread, NXN_stackunderflow);
    }

    nxo_stack_pop(ostack);
}

void
systemdict_shift(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *integer, *shift;
    cw_nxoi_t nshift;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(shift, ostack, a_thread);
    NXO_STACK_DOWN_GET(integer, ostack, a_thread, shift);

    if (nxo_type_get(integer) != NXOT_INTEGER
	|| nxo_type_get(shift) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    nshift = nxo_integer_get(shift);

    /* Specially handle situations where the shift amount is more than 63
     * bits. */
    if (nshift < -63)
    {
	nxo_integer_set(integer, 0);
    }
    else if (nshift < 0)
    {
	nxo_integer_set(integer,
			nxo_integer_get(integer) >> -nxo_integer_get(shift));
    }
    else if (nshift > 63)
    {
	nxo_integer_set(integer, 0);
    }
    else if (nshift > 0)
    {
	nxo_integer_set(integer,
			nxo_integer_get(integer) << nxo_integer_get(shift));
    }

    nxo_stack_pop(ostack);
}

void
systemdict_sibdup(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo, *stack, *orig;
    cw_nxoi_t index;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    NXO_STACK_DOWN_GET(stack, ostack, a_thread, nxo);
    if (nxo_type_get(nxo) != NXOT_INTEGER || nxo_type_get(stack) != NXOT_STACK)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    index = nxo_integer_get(nxo);
    if (index < 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    NXO_STACK_NBGET(orig, stack, a_thread, index);
    nxo = nxo_stack_push(stack);
    nxo_dup(nxo, orig);

    nxo_stack_npop(ostack, 2);
}

void
systemdict_sibpop(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo, *stack, *inxo;
    cw_nxoi_t index;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    NXO_STACK_DOWN_GET(stack, ostack, a_thread, nxo);
    if (nxo_type_get(nxo) != NXOT_INTEGER || nxo_type_get(stack) != NXOT_STACK)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    index = nxo_integer_get(nxo);
    if (index < 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    NXO_STACK_NBGET(inxo, stack, a_thread, index);
    nxo_dup(nxo, inxo);
    nxo_stack_remove(stack, inxo);

    nxo_stack_remove(ostack, stack);
}

void
systemdict_sidup(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo, *stack, *orig;
    cw_nxoi_t index;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    NXO_STACK_DOWN_GET(stack, ostack, a_thread, nxo);
    if (nxo_type_get(nxo) != NXOT_INTEGER || nxo_type_get(stack) != NXOT_STACK)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    index = nxo_integer_get(nxo);
    if (index < 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    NXO_STACK_NGET(orig, stack, a_thread, index);
    nxo = nxo_stack_push(stack);
    nxo_dup(nxo, orig);

    nxo_stack_npop(ostack, 2);
}

#ifdef CW_THREADS
void
systemdict_signal(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *condition;
	
    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(condition, ostack, a_thread);
    if (nxo_type_get(condition) != NXOT_CONDITION)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    nxo_condition_signal(condition);

    nxo_stack_pop(ostack);
}
#endif

#ifdef CW_REAL
void
systemdict_sin(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;
    cw_nxor_t real;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    switch (nxo_type_get(nxo))
    {
	case NXOT_INTEGER:
	{
	    real = (cw_nxor_t) nxo_integer_get(nxo);
	    break;
	}
	case NXOT_REAL:
	{
	    real = nxo_real_get(nxo);
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }
    nxo_real_new(nxo, sin(real));
}
#endif

#ifdef CW_REAL
void
systemdict_sinh(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;
    cw_nxor_t real;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    switch (nxo_type_get(nxo))
    {
	case NXOT_INTEGER:
	{
	    real = (cw_nxor_t) nxo_integer_get(nxo);
	    break;
	}
	case NXOT_REAL:
	{
	    real = nxo_real_get(nxo);
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }
    nxo_real_new(nxo, sinh(real));
}
#endif

void
systemdict_sipop(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo, *stack, *inxo;
    cw_nxoi_t index;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    NXO_STACK_DOWN_GET(stack, ostack, a_thread, nxo);
    if (nxo_type_get(nxo) != NXOT_INTEGER || nxo_type_get(stack) != NXOT_STACK)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    index = nxo_integer_get(nxo);
    if (index < 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    NXO_STACK_NGET(inxo, stack, a_thread, index);
    nxo_dup(nxo, inxo);
    nxo_stack_remove(stack, inxo);

    nxo_stack_remove(ostack, stack);
}

void
systemdict_snbpop(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo, *stack, *snxo;
    cw_nxoi_t count;
    cw_uint32_t i;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    NXO_STACK_DOWN_GET(stack, ostack, a_thread, nxo);
    if (nxo_type_get(nxo) != NXOT_INTEGER || nxo_type_get(stack) != NXOT_STACK)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    count = nxo_integer_get(nxo);
    if (count < 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }
    if (count > nxo_stack_count(stack))
    {
	nxo_thread_nerror(a_thread, NXN_stackunderflow);
	return;
    }

    nxo_array_new(nxo, nxo_thread_currentlocking(a_thread), count);

    /* Iteratively create dup's and bpop.. */
    for (i = 0, snxo = NULL; i < count; i++)
    {
	snxo = nxo_stack_bget(stack);
	nxo_array_el_set(nxo, snxo, i);
	nxo_stack_bpop(stack);
    }

    nxo_stack_remove(ostack, stack);
}

void
systemdict_sndn(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *stack, *nxo;
    cw_nxoi_t count;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    NXO_STACK_DOWN_GET(stack, ostack, a_thread, nxo);
    if (nxo_type_get(nxo) != NXOT_INTEGER || nxo_type_get(stack) != NXOT_STACK)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    count = nxo_integer_get(nxo);
    if (count < 1)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }
    if (count > nxo_stack_count(stack))
    {
	nxo_thread_nerror(a_thread, NXN_stackunderflow);
	return;
    }

    nxo_stack_roll(stack, count, -1);
    nxo_stack_npop(ostack, 2);
}

void
systemdict_sndup(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *stack, *nxo, *dup;
    cw_uint32_t i;
    cw_nxoi_t count;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    NXO_STACK_DOWN_GET(stack, ostack, a_thread, nxo);
    if (nxo_type_get(nxo) != NXOT_INTEGER || nxo_type_get(stack) != NXOT_STACK)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    count = nxo_integer_get(nxo);
    if (count < 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }
    if (count > nxo_stack_count(stack))
    {
	nxo_thread_nerror(a_thread, NXN_stackunderflow);
	return;
    }

    /* Iterate down the stack, creating dup's along the way.  Since we're going
     * down, it's necessary to use nxo_stack_under_push() to preserve order. */
    for (i = 0, nxo = NULL, dup = NULL; i < count; i++)
    {
	nxo = nxo_stack_down_get(stack, nxo);
	if (nxo == NULL)
	{
	    /* This is very bad, and means that another thread is mucking with
	     * the stack.  The right thing to do is try to restore the stack
	     * state, but there's no way to know for sure what state the stack
	     * is in.  Throw an error. */
	    nxo_thread_nerror(a_thread, NXN_unregistered);
	    return;
	}
	dup = nxo_stack_under_push(stack, dup);
	nxo_dup(dup, nxo);
    }

    nxo_stack_npop(ostack, 2);
}

void
systemdict_snip(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *stack, *snxo, *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(stack, ostack, a_thread);
    if (nxo_type_get(stack) != NXOT_STACK)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    NXO_STACK_NGET(snxo, stack, a_thread, 1);
    nxo = nxo_stack_push(ostack);
    nxo_dup(nxo, snxo);
    nxo_stack_remove(stack, snxo);
    nxo_stack_remove(ostack, stack);
}

void
systemdict_snpop(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo, *stack, *snxo;
    cw_nxoi_t count;
    cw_uint32_t i;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    NXO_STACK_DOWN_GET(stack, ostack, a_thread, nxo);
    if (nxo_type_get(nxo) != NXOT_INTEGER || nxo_type_get(stack) != NXOT_STACK)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    count = nxo_integer_get(nxo);
    if (count < 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }
    if (count > nxo_stack_count(stack))
    {
	nxo_thread_nerror(a_thread, NXN_stackunderflow);
	return;
    }

    nxo_array_new(nxo, nxo_thread_currentlocking(a_thread), count);

    /* Iteratively create dup's and pop. */
    for (i = 0, snxo = NULL; i < count; i++)
    {
	snxo = nxo_stack_get(stack);
	nxo_array_el_set(nxo, snxo, count - 1 - i);
	nxo_stack_pop(stack);
    }

    nxo_stack_remove(ostack, stack);
}

void
systemdict_snup(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *stack, *nxo;
    cw_nxoi_t count;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    NXO_STACK_DOWN_GET(stack, ostack, a_thread, nxo);
    if (nxo_type_get(nxo) != NXOT_INTEGER || nxo_type_get(stack) != NXOT_STACK)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    count = nxo_integer_get(nxo);
    if (count < 1)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }
    if (count > nxo_stack_count(stack))
    {
	nxo_thread_nerror(a_thread, NXN_stackunderflow);
	return;
    }

    nxo_stack_roll(stack, count, 1);
    nxo_stack_npop(ostack, 2);
}

#ifdef CW_SOCKET
static const struct cw_systemdict_name_arg socket_family[] =
{
    {NXN_AF_INET, AF_INET},
    {NXN_AF_LOCAL, AF_LOCAL}
};

static const struct cw_systemdict_name_arg socket_type[] =
{
    {NXN_SOCK_STREAM, SOCK_STREAM},
    {NXN_SOCK_DGRAM, SOCK_DGRAM}
};

static void
systemdict_p_socket(cw_nxo_t *a_thread, cw_bool_t a_pair)
{
    cw_nxo_t *ostack, *nxo;
    cw_uint32_t argcnt, argind, npop;
    int family, type, protocol = 0;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack,  a_thread);
    if (nxo_type_get(nxo) != NXOT_NAME)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    /* See if nxo is a socket type. */
    argcnt = sizeof(socket_type) / sizeof(struct cw_systemdict_name_arg);
    argind = systemdict_p_name_arg(nxo, socket_type, argcnt);
    if (argind != argcnt)
    {
	/* Socket type. */
	type = socket_type[argind].arg;

	npop = 2;
    }
    else
    {
	cw_nxo_t *tstack, *tnxo;
	struct protoent *ent;

	/* Not a socket type.  Try it as a socket protocol. */

	tstack = nxo_thread_tstack_get(a_thread);
	tnxo = nxo_stack_push(tstack);
	nxo_string_cstring(tnxo, nxo, a_thread);

#ifdef CW_THREADS
	mtx_lock(&cw_g_getprotobyname_mtx);
#endif
	setprotoent(0);
	ent = getprotobyname(nxo_string_get(tnxo));

	nxo_stack_pop(tstack);

	if (ent == NULL)
	{
	    /* Not a socket protocol. */
	    endprotoent();
#ifdef CW_THREADS
	    mtx_unlock(&cw_g_getprotobyname_mtx);
#endif
	    nxo_thread_nerror(a_thread, NXN_argcheck);
	    return;
	}

	protocol = ent->p_proto;
	endprotoent();
#ifdef CW_THREADS
	mtx_unlock(&cw_g_getprotobyname_mtx);
#endif

	/* The next argument must be the socket type. */
	NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
	if (nxo_type_get(nxo) != NXOT_NAME)
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
	argcnt = sizeof(socket_type)
	    / sizeof(struct cw_systemdict_name_arg);
	argind = systemdict_p_name_arg(nxo, socket_type, argcnt);
	if (argind != argcnt)
	{
	    /* Socket type. */
	    type = socket_type[argind].arg;
	}
	else
	{
	    /* Not a socket type. */
	    nxo_thread_nerror(a_thread, NXN_argcheck);
	    return;
	}

	npop = 3;
    }

    /* Get the socket family. */
    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    if (nxo_type_get(nxo) != NXOT_NAME)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    argcnt = sizeof(socket_family) / sizeof(struct cw_systemdict_name_arg);
    argind = systemdict_p_name_arg(nxo, socket_family, argcnt);
    if (argind != argcnt)
    {
	/* Socket family. */
	family = socket_family[argind].arg;
    }
    else
    {
	/* Not a socket family. */
	nxo_thread_nerror(a_thread, NXN_argcheck);
	return;
    }

    if (a_pair == FALSE)
    {
	int sockfd;

	sockfd = socket(family, type, protocol);
	if (sockfd == -1)
	{
	    goto ERROR;
	}
	/* Wrap sockfd. */
	nxo = nxo_stack_under_push(ostack, nxo);
	nxo_file_new(nxo, nxo_thread_currentlocking(a_thread));
	nxo_file_fd_wrap(nxo, sockfd, TRUE);
    }
    else
    {
	cw_uint32_t i;
	int sockfds[2];

	if (socketpair(family, type, protocol, sockfds))
	{
	    goto ERROR;
	}

	/* Wrap sockfds. */
	for (i = 0; i < 2; i++) {
	    nxo = nxo_stack_under_push(ostack, nxo);
	    nxo_file_new(nxo, nxo_thread_currentlocking(a_thread));
	    nxo_file_fd_wrap(nxo, sockfds[i], TRUE);
	}
    }

    /* Pop inputs. */
    nxo_stack_npop(ostack, npop);
    return;

    ERROR:
    switch (errno)
    {
	case EAFNOSUPPORT:
	case EPFNOSUPPORT:
	case EOPNOTSUPP:
	case EPROTONOSUPPORT:
	case ESOCKTNOSUPPORT:
	{
	    nxo_thread_nerror(a_thread, NXN_argcheck);
	    return;
	}
	case EACCES:
	{
	    nxo_thread_nerror(a_thread, NXN_invalidaccess);
	    return;
	}
	case ENFILE:
	case EMFILE:
	case ENOBUFS:
	case ENOMEM:
	{
	    xep_throw(CW_ONYXX_OOM);
	}
	case EINVAL:
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_unregistered);
	    return;
	}
    }
}
#endif

#ifdef CW_SOCKET
void
systemdict_socket(cw_nxo_t *a_thread)
{
    systemdict_p_socket(a_thread, FALSE);
}
#endif

#ifdef CW_SOCKET
void
systemdict_socketpair(cw_nxo_t *a_thread)
{
    systemdict_p_socket(a_thread, TRUE);
}
#endif

#ifdef CW_SOCKET
void
systemdict_sockopt(cw_nxo_t *a_thread)
{
    systemdict_p_sockopt(a_thread, FALSE);
}
#endif

#ifdef CW_SOCKET
void
systemdict_sockname(cw_nxo_t *a_thread)
{
    systemdict_p_peername(a_thread, FALSE);
}
#endif

void
systemdict_sover(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *stack, *sunder, *snxo;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(stack, ostack, a_thread);
    if (nxo_type_get(stack) != NXOT_STACK)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    NXO_STACK_NGET(sunder, stack, a_thread, 1);
    snxo = nxo_stack_push(stack);
    nxo_dup(snxo, sunder);

    nxo_stack_pop(ostack);
}

#ifdef CW_REGEX
void
systemdict_split(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo, *flags, *pattern, *input, *output;
    cw_uint32_t npop;
    cw_nxn_t error;
    cw_nxoi_t limit;

    ostack = nxo_thread_ostack_get(a_thread);

    /* Get limit, if specified. */
    npop = 1;
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) == NXOT_INTEGER)
    {
	npop++;
	limit = nxo_integer_get(nxo);
	if (limit < 0LL)
	{
	    nxo_thread_nerror(a_thread, NXN_rangecheck);
	    return;
	}
	NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    }
    else
    {
	limit = 0;
    }

    flags = NULL;
    switch (nxo_type_get(nxo))
    {
	case NXOT_DICT:
	{
	    npop++;
	    flags = nxo;
	    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
	    if (nxo_type_get(nxo) != NXOT_STRING)
	    {
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	    }

	    /* Fall through. */
	}
	case NXOT_STRING:
	{
	    cw_bool_t insensitive, multiline, singleline;

	    /* Get regex flags. */
	    if (flags != NULL)
	    {
		error = systemdict_p_regex_flags_get(flags, a_thread,
						     NULL, NULL,
						     &insensitive, &multiline,
						     &singleline);
		if (error)
		{
		    nxo_thread_nerror(a_thread, error);
		    return;
		}
	    }
	    else
	    {
		insensitive = FALSE;
		multiline = FALSE;
		singleline = FALSE;
	    }

	    /* Get pattern. */
	    pattern = nxo;

	    /* Get input string. */
	    npop++;
	    NXO_STACK_DOWN_GET(input, ostack, a_thread, nxo);
	    if (nxo_type_get(input) != NXOT_STRING)
	    {
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	    }

	    output = nxo_stack_under_push(ostack, input);
	    nxo_string_lock(pattern);
	    error = nxo_regex_nonew_split(a_thread, nxo_string_get(pattern),
					  nxo_string_len_get(pattern),
					  insensitive, multiline, singleline,
					  (cw_uint32_t) limit, input, output);
	    nxo_string_unlock(pattern);
	    if (error)
	    {
		nxo_stack_remove(ostack, output);
		nxo_thread_nerror(a_thread, error);
		return;
	    }

	    break;
	}
	case NXOT_REGEX:
	{
	    cw_nxo_t *regex;

	    regex = nxo;

	    /* Get input string. */
	    npop++;
	    NXO_STACK_DOWN_GET(input, ostack, a_thread, nxo);
	    if (nxo_type_get(input) != NXOT_STRING)
	    {
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	    }

	    output = nxo_stack_under_push(ostack, input);
	    nxo_regex_split(regex, a_thread, (cw_uint32_t) limit, input,
			    output);

	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }

    nxo_stack_npop(ostack, npop);
}
#endif

void
systemdict_spop(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *stack, *snxo, *onxo;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(stack, ostack, a_thread);
    if (nxo_type_get(stack) != NXOT_STACK)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    NXO_STACK_GET(snxo, stack, a_thread);
    onxo = nxo_stack_under_push(ostack, stack);
    nxo_dup(onxo, snxo);

    nxo_stack_pop(stack);
    nxo_stack_pop(ostack);
}

void
systemdict_spush(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo, *stack, *nnxo;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    NXO_STACK_DOWN_GET(stack, ostack, a_thread, nxo);
    if (nxo_type_get(stack) != NXOT_STACK)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    nnxo = nxo_stack_push(stack);
    nxo_dup(nnxo, nxo);
    nxo_stack_npop(ostack, 2);
}

#ifdef CW_REAL
void
systemdict_sqrt(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;
    cw_nxor_t real;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    switch (nxo_type_get(nxo))
    {
	case NXOT_INTEGER:
	{
	    real = (cw_nxor_t) nxo_integer_get(nxo);
	    break;
	}
	case NXOT_REAL:
	{
	    real = nxo_real_get(nxo);
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }
    if (real < 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }
    nxo_real_new(nxo, sqrt(real));
}
#endif

#ifdef CW_POSIX
void
systemdict_srand(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;
    cw_nxoi_t seed;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    seed = nxo_integer_get(nxo);
    if (seed < 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    srandom((unsigned long)seed);
    nxo_stack_pop(ostack);
}
#endif

void
systemdict_sroll(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *stack, *nxo;
    cw_nxoi_t count, amount;

    ostack = nxo_thread_ostack_get(a_thread);
	
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    amount = nxo_integer_get(nxo);
    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
    NXO_STACK_DOWN_GET(stack, ostack, a_thread, nxo);
    if (nxo_type_get(nxo) != NXOT_INTEGER || nxo_type_get(stack) != NXOT_STACK)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    count = nxo_integer_get(nxo);
    if (count < 1)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    if (nxo_stack_roll(stack, count, amount))
    {
	nxo_thread_nerror(a_thread, NXN_stackunderflow);
	return;
    }

    nxo_stack_npop(ostack, 3);
}

void
systemdict_srot(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *stack, *nxo;
    cw_nxoi_t amount;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    NXO_STACK_DOWN_GET(stack, ostack, a_thread, nxo);
    if (nxo_type_get(nxo) != NXOT_INTEGER || nxo_type_get(stack) != NXOT_STACK)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    amount = nxo_integer_get(nxo);

    if (nxo_stack_count(stack) < 1)
    {
	nxo_thread_nerror(a_thread, NXN_stackunderflow);
	return;
    }
    nxo_stack_rot(stack, amount);
    nxo_stack_npop(ostack, 2);
}

void
systemdict_stack(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nstack;

    ostack = nxo_thread_ostack_get(a_thread);
    nstack = nxo_stack_push(ostack);
    nxo_stack_new(nstack, nxo_thread_currentlocking(a_thread));
}

void
systemdict_start(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *estack, *istack, *tstack;
    cw_nxo_t *onxo, *enxo;
    cw_uint32_t edepth, tdepth;

    ostack = nxo_thread_ostack_get(a_thread);
    estack = nxo_thread_estack_get(a_thread);
    istack = nxo_thread_istack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);

    /* Record stack depths so that we can clean up later. */
    edepth = nxo_stack_count(estack);
    tdepth = nxo_stack_count(tstack);

    NXO_STACK_GET(onxo, ostack, a_thread);
    enxo = nxo_stack_push(estack);
    nxo_dup(enxo, onxo);
    nxo_stack_pop(ostack);

    xep_begin();
    xep_try
    {
	nxo_thread_loop(a_thread);
    }
    xep_catch(CW_ONYXX_EXIT)
    xep_mcatch(CW_ONYXX_QUIT)
    xep_mcatch(CW_ONYXX_STOP)
    {
	xep_handled();
    }
    xep_end();

    /* Pop all objects off estack, istack, and tstack that weren't there before
     * entering this function. */
    nxo_stack_npop(estack, nxo_stack_count(estack) - edepth);
    nxo_stack_npop(istack, nxo_stack_count(istack) - nxo_stack_count(estack));
    nxo_stack_npop(tstack, nxo_stack_count(tstack) - tdepth);
}

#ifdef CW_POSIX
void
systemdict_status(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack, *file;
    cw_nxo_t *dict, *name, *value;
    int error;
    struct stat sb;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    NXO_STACK_GET(file, ostack, a_thread);
    if (nxo_type_get(file) != NXOT_FILE && nxo_type_get(file) != NXOT_STRING)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    if (nxo_type_get(file) == NXOT_FILE)
    {
	int fd;

	fd = nxo_file_fd_get(file);
	if (fd < 0)
	{
	    nxo_thread_nerror(a_thread, NXN_invalidfileaccess);
	    return;
	}

	error = fstat(fd, &sb);
    }
    else
    {
	cw_nxo_t *tfile;

	/* Create a copy of file with an extra byte to store a '\0'
	 * terminator. */
	tfile = nxo_stack_push(tstack);
	nxo_string_cstring(tfile, file, a_thread);

	error = stat(nxo_string_get(tfile), &sb);

	nxo_stack_pop(tstack);
    }

    if (error == -1)
    {
	switch (errno)
	{
	    case EIO:
	    {
		nxo_thread_nerror(a_thread, NXN_ioerror);
		break;
	    }
	    case EACCES:
	    case ELOOP:
	    case ENAMETOOLONG:
	    case ENOENT:
	    case ENOTDIR:
	    {
		nxo_thread_nerror(a_thread, NXN_invalidfileaccess);
		break;
	    }
	    case EBADF:
	    case EFAULT:
	    default:
	    {
		nxo_thread_nerror(a_thread, NXN_unregistered);
	    }
	}
	return;
    }

    nxo_stack_pop(ostack);

    /* We now have a valid stat buffer.  Create a dictionary that represents
     * it. */
    dict = nxo_stack_push(ostack);
    nxo_dict_new(dict, nxo_thread_currentlocking(a_thread), 13);

    name = nxo_stack_push(tstack);
    value = nxo_stack_push(tstack);

    /* dev. */
    nxo_name_new(name, nxn_str(NXN_dev), nxn_len(NXN_dev), TRUE);
    nxo_integer_new(value, sb.st_dev);
    nxo_dict_def(dict, name, value);

    /* ino. */
    nxo_name_new(name, nxn_str(NXN_ino), nxn_len(NXN_ino), TRUE);
    nxo_integer_new(value, sb.st_ino);
    nxo_dict_def(dict, name, value);

    /* mode. */
    nxo_name_new(name, nxn_str(NXN_mode), nxn_len(NXN_mode), TRUE);
    nxo_integer_new(value, sb.st_mode);
    nxo_dict_def(dict, name, value);

    /* nlink. */
    nxo_name_new(name, nxn_str(NXN_nlink), nxn_len(NXN_nlink), TRUE);
    nxo_integer_new(value, sb.st_nlink);
    nxo_dict_def(dict, name, value);

    /* uid. */
    nxo_name_new(name, nxn_str(NXN_uid), nxn_len(NXN_uid), TRUE);
    nxo_integer_new(value, sb.st_uid);
    nxo_dict_def(dict, name, value);

    /* gid. */
    nxo_name_new(name, nxn_str(NXN_gid), nxn_len(NXN_gid), TRUE);
    nxo_integer_new(value, sb.st_gid);
    nxo_dict_def(dict, name, value);

    /* rdev. */
    nxo_name_new(name, nxn_str(NXN_rdev), nxn_len(NXN_rdev), TRUE);
    nxo_integer_new(value, sb.st_rdev);
    nxo_dict_def(dict, name, value);

    /* size. */
    nxo_name_new(name, nxn_str(NXN_size), nxn_len(NXN_size), TRUE);
    nxo_integer_new(value, sb.st_size);
    nxo_dict_def(dict, name, value);

    /* atime. */
    nxo_name_new(name, nxn_str(NXN_atime), nxn_len(NXN_atime), TRUE);
#ifdef CW_LIBONYX_USE_STAT_ATIMESPEC
    nxo_integer_new(value,
		    ((cw_nxoi_t) sb.st_atimespec.tv_sec *
		     (cw_nxoi_t) 1000000000)
		    + (cw_nxoi_t) sb.st_atimespec.tv_nsec);
#else
    nxo_integer_new(value, ((cw_nxoi_t) sb.st_atime * (cw_nxoi_t) 1000000000));
#endif
    nxo_dict_def(dict, name, value);

    /* mtime. */
    nxo_name_new(name, nxn_str(NXN_mtime), nxn_len(NXN_mtime), TRUE);
#ifdef CW_LIBONYX_USE_STAT_MTIMESPEC
    nxo_integer_new(value,
		    ((cw_nxoi_t) sb.st_mtimespec.tv_sec
		     * (cw_nxoi_t) 1000000000)
		    + (cw_nxoi_t) sb.st_mtimespec.tv_nsec);
#else
    nxo_integer_new(value, ((cw_nxoi_t) sb.st_mtime * (cw_nxoi_t) 1000000000));
#endif
    nxo_dict_def(dict, name, value);

    /* ctime. */
    nxo_name_new(name, nxn_str(NXN_ctime), nxn_len(NXN_ctime), TRUE);
#ifdef CW_LIBONYX_USE_STAT_CTIMESPEC
    nxo_integer_new(value,
		    ((cw_nxoi_t) sb.st_ctimespec.tv_sec
		     * (cw_nxoi_t) 1000000000)
		    + (cw_nxoi_t) sb.st_ctimespec.tv_nsec);
#else
    nxo_integer_new(value, ((cw_nxoi_t) sb.st_ctime * (cw_nxoi_t) 1000000000));
#endif
    nxo_dict_def(dict, name, value);

    /* blksize. */
    nxo_name_new(name, nxn_str(NXN_blksize), nxn_len(NXN_blksize), TRUE);
    nxo_integer_new(value, sb.st_blksize);
    nxo_dict_def(dict, name, value);

    /* blocks. */
    nxo_name_new(name, nxn_str(NXN_blocks), nxn_len(NXN_blocks), TRUE);
    nxo_integer_new(value, sb.st_blocks);
    nxo_dict_def(dict, name, value);

    nxo_stack_npop(tstack, 2);
}
#endif

void
systemdict_stderr(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    nxo = nxo_stack_push(ostack);
    nxo_dup(nxo, nxo_thread_stderr_get(a_thread));
}

void
systemdict_stdin(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    nxo = nxo_stack_push(ostack);
    nxo_dup(nxo, nxo_thread_stdin_get(a_thread));
}

void
systemdict_stdout(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    nxo = nxo_stack_push(ostack);
    nxo_dup(nxo, nxo_thread_stdout_get(a_thread));
}

void
systemdict_stop(cw_nxo_t *a_thread)
{
    xep_throw(CW_ONYXX_STOP);
}

void
systemdict_stopped(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *estack, *tstack;
    cw_nxo_t *exec, *nxo;
    cw_bool_t result = FALSE;
    cw_uint32_t edepth, tdepth;

    ostack = nxo_thread_ostack_get(a_thread);
    estack = nxo_thread_estack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
	
    NXO_STACK_GET(exec, ostack, a_thread);

    /* Record stack depths so that we can clean up if necessary. */
    edepth = nxo_stack_count(estack);
    tdepth = nxo_stack_count(tstack);

    nxo = nxo_stack_push(estack);
    nxo_dup(nxo, exec);
    nxo_stack_pop(ostack);

    /* Catch a stop exception, if thrown. */
    xep_begin();
    xep_try
    {
	nxo_thread_loop(a_thread);
    }
    xep_catch(CW_ONYXX_STOP)
    {
	cw_nxo_t *istack;

	xep_handled();
	result = TRUE;

	/* Clean up stacks. */
	nxo_stack_npop(estack, nxo_stack_count(estack) - edepth);
	istack = nxo_thread_istack_get(a_thread);
	nxo_stack_npop(istack, nxo_stack_count(istack)
		       - nxo_stack_count(estack));
	nxo_stack_npop(tstack, nxo_stack_count(tstack) - tdepth);
    }
    xep_catch(CW_ONYXX_EXIT)
    {
	/* This is a serious program error, and we've already unwound the C
	 * stack, so there's no going back.  After throwing an error, do the
	 * equivalent of what the quit operator does, so that we'll unwind to
	 * the innermost start context. */
	xep_handled();

	nxo_thread_nerror(a_thread, NXN_invalidexit);

	xep_throw(CW_ONYXX_QUIT);
    }
    xep_end();

    nxo = nxo_stack_push(ostack);
    nxo_boolean_new(nxo, result);
}

void
systemdict_string(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;
    cw_nxoi_t len;

    ostack = nxo_thread_ostack_get(a_thread);
	
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    len = nxo_integer_get(nxo);
    if (len < 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    nxo_string_new(nxo, nxo_thread_currentlocking(a_thread), len);
}

void
systemdict_stuck(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *stack, *stop, *snxo;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(stack, ostack, a_thread);
    if (nxo_type_get(stack) != NXOT_STACK)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    NXO_STACK_GET(stop, stack, a_thread);
    NXO_STACK_DOWN_GET(snxo, stack, a_thread, stop);
    snxo = nxo_stack_under_push(stack, snxo);
    nxo_dup(snxo, stop);

    nxo_stack_pop(ostack);
}

void
systemdict_sub(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo_a, *nxo_b;
    cw_nxoi_t integer_a, integer_b;
#ifdef CW_REAL
    cw_bool_t do_real;
    cw_nxor_t real_a, real_b;
#endif

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo_b, ostack, a_thread);
    NXO_STACK_DOWN_GET(nxo_a, ostack, a_thread, nxo_b);
    switch (nxo_type_get(nxo_a))
    {
	case NXOT_INTEGER:
	{
#ifdef CW_REAL
	    do_real = FALSE;
#endif
	    integer_a = nxo_integer_get(nxo_a);
	    break;
	}
#ifdef CW_REAL
	case NXOT_REAL:
	{
	    do_real = TRUE;
	    real_a = nxo_real_get(nxo_a);
	    break;
	}
#endif
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }
    switch (nxo_type_get(nxo_b))
    {
	case NXOT_INTEGER:
	{
#ifdef CW_REAL
	    if (do_real)
	    {
		real_b = (cw_nxor_t) nxo_integer_get(nxo_b);
	    }
	    else
#endif
	    {
		integer_b = nxo_integer_get(nxo_b);
	    }
	    break;
	}
#ifdef CW_REAL
	case NXOT_REAL:
	{
	    real_b = nxo_real_get(nxo_b);
	    if (do_real == FALSE)
	    {
		do_real = TRUE;
		real_a = (cw_nxor_t) integer_a;
	    }
	    break;
	}
#endif
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }

#ifdef CW_REAL
    if (do_real)
    {
	/* nxo_a may be an integer, so use nxo_real_new() rather than
	 * nxo_real_set(). */
	nxo_real_new(nxo_a, real_a - real_b);
    }
    else
#endif
    {
	nxo_integer_set(nxo_a, integer_a - integer_b);
    }

    nxo_stack_pop(ostack);
}

#ifdef CW_REGEX
void
systemdict_submatch(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    nxo_regex_submatch(a_thread, nxo_integer_get(nxo), nxo);
}
#endif

#ifdef CW_REGEX
void
systemdict_subst(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo, *flags, *template, *pattern, *input, *output;
    cw_uint32_t npop, count;
    cw_nxn_t error;

    ostack = nxo_thread_ostack_get(a_thread);

    flags = NULL;
    npop = 0;
    NXO_STACK_GET(nxo, ostack, a_thread);
    switch (nxo_type_get(nxo))
    {
	case NXOT_DICT:
	{
	    flags = nxo;

	    /* Get template. */
	    npop++;
	    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, nxo);
	    if (nxo_type_get(nxo) != NXOT_STRING)
	    {
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	    }

	    /* Fall through. */
	}
	case NXOT_STRING:
	{
	    cw_bool_t global, insensitive, multiline, singleline;

	    /* Get regex flags. */
	    if (flags != NULL)
	    {
		error = systemdict_p_regex_flags_get(flags, a_thread, NULL,
						     &global, &insensitive,
						     &multiline, &singleline);
		if (error)
		{
		    nxo_thread_nerror(a_thread, error);
		    return;
		}
	    }
	    else
	    {
		global = FALSE;
		insensitive = FALSE;
		multiline = FALSE;
		singleline = FALSE;
	    }

	    template = nxo;

	    /* Get pattern. */
	    npop++;
	    NXO_STACK_DOWN_GET(pattern, ostack, a_thread, nxo);
	    if (nxo_type_get(pattern) != NXOT_STRING)
	    {
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	    }

	    /* Get input string. */
	    npop++;
	    NXO_STACK_DOWN_GET(input, ostack, a_thread, pattern);
	    if (nxo_type_get(input) != NXOT_STRING)
	    {
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	    }

	    output = nxo_stack_under_push(ostack, input);
	    nxo_string_lock(pattern);
	    nxo_string_lock(template);
	    error = nxo_regsub_nonew_subst(a_thread, nxo_string_get(pattern),
					   nxo_string_len_get(pattern), global,
					   insensitive, multiline, singleline,
					   nxo_string_get(template),
					   nxo_string_len_get(template), input,
					   output, &count);
	    nxo_string_unlock(template);
	    nxo_string_unlock(pattern);
	    if (error)
	    {
		nxo_stack_remove(ostack, output);
		nxo_thread_nerror(a_thread, error);
		return;
	    }

	    break;
	}
	case NXOT_REGSUB:
	{
	    cw_nxo_t *regsub;

	    regsub = nxo;

	    /* Get input string. */
	    npop++;
	    NXO_STACK_DOWN_GET(input, ostack, a_thread, nxo);
	    if (nxo_type_get(input) != NXOT_STRING)
	    {
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	    }

	    output = nxo_stack_under_push(ostack, input);
	    nxo_regsub_subst(regsub, a_thread, input, output, &count);

	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }

    nxo_integer_new(input, (cw_nxoi_t) count);
    nxo_stack_npop(ostack, npop);
}
#endif

void
systemdict_sunder(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *stack, *sunder, *snxo;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(stack, ostack, a_thread);
    if (nxo_type_get(stack) != NXOT_STACK)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    NXO_STACK_NGET(sunder, stack, a_thread, 1);
    snxo = nxo_stack_under_push(stack, sunder);
    nxo_dup(snxo, sunder);

    nxo_stack_pop(ostack);
}

void
systemdict_sup(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *stack;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(stack, ostack, a_thread);
    if (nxo_type_get(stack) != NXOT_STACK)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    if (nxo_stack_count(stack) < 3)
    {
	nxo_thread_nerror(a_thread, NXN_stackunderflow);
	return;
    }

    nxo_stack_roll(stack, 3, 1);

    nxo_stack_pop(ostack);
}

/* ( */
void
systemdict_sym_lp(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    nxo = nxo_stack_push(ostack);
    nxo_fino_new(nxo);
}

/* ) */
void
systemdict_sym_rp(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack;
    cw_nxo_t *nstack, *tnxo, *nxo;
    cw_sint32_t nelements, i, depth;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    /* Find the fino. */
    for (i = 0, depth = nxo_stack_count(ostack), nxo = NULL; i < depth; i++)
    {
	nxo = nxo_stack_down_get(ostack, nxo);
	if (nxo_type_get(nxo) == NXOT_FINO)
	{
	    break;
	}
    }
    if (i == depth)
    {
	nxo_thread_nerror(a_thread, NXN_unmatchedfino);
	return;
    }

    /* i is the index of the fino, and nxo points to the fino.  Set nelements
     * accordingly. */
    nelements = i;

    nstack = nxo_stack_push(tstack);
    nxo_stack_new(nstack, nxo_thread_currentlocking(a_thread));

    /* Push objects onto tstack and pop them off ostack. */
    for (i = 0; i < nelements; i++)
    {
	nxo = nxo_stack_get(ostack);
	tnxo = nxo_stack_push(tstack);
	nxo_dup(tnxo, nxo);
	nxo_stack_pop(ostack);
    }

    /* Pop the fino off ostack. */
    nxo_stack_pop(ostack);

    /* Push objects onto nstack and pop them off tstack. */
    for (i = 0; i < nelements; i++)
    {
	nxo = nxo_stack_get(tstack);
	tnxo = nxo_stack_push(nstack);
	nxo_dup(tnxo, nxo);
	nxo_stack_pop(tstack);
    }

    /* Push nstack onto ostack and pop it off of tstack. */
    nxo = nxo_stack_push(ostack);
    nxo_dup(nxo, nstack);
    nxo_stack_pop(tstack);
}

/* > */
void
systemdict_sym_gt(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack;
    cw_nxo_t *nxo, *dict, *key, *val;
    cw_uint32_t npairs, i, depth;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);

    /* Find the mark. */
    for (i = 0, depth = nxo_stack_count(ostack), nxo = NULL; i < depth; i++)
    {
	nxo = nxo_stack_down_get(ostack, nxo);
	if (nxo_type_get(nxo) == NXOT_MARK)
	{
	    break;
	}
    }
    if (i == depth)
    {
	nxo_thread_nerror(a_thread, NXN_unmatchedmark);
	return;
    }
    if ((i & 1) == 1)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    /* i is the index of the mark, and nxo points to the mark.  Set npairs
     * accordingly.  When we pop the nxo's off the stack, we'll have to pop
     * (npairs << 1 + 1) nxo's. */
    npairs = i >> 1;

    dict = nxo_stack_push(tstack);
    nxo_dict_new(dict, nxo_thread_currentlocking(a_thread), npairs);

    /* Traverse down the stack, moving nxo's to the dict. */
    for (i = 0, key = NULL; i < npairs; i++)
    {
	val = nxo_stack_down_get(ostack, key);
	key = nxo_stack_down_get(ostack, val);
	nxo_dict_def(dict, key, val);
    }

    /* Pop the nxo's off the stack now. */
    nxo_stack_npop(ostack, (npairs << 1) + 1);

    /* Push the dict onto the stack. */
    nxo = nxo_stack_push(ostack);
    nxo_dup(nxo, dict);

    nxo_stack_pop(tstack);
}

/* ] */
void
systemdict_sym_rb(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack;
    cw_nxo_t *tnxo, *nxo;
    cw_sint32_t nelements, i, depth;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    /* Find the mark. */
    for (i = 0, depth = nxo_stack_count(ostack), nxo = NULL; i < depth; i++)
    {
	nxo = nxo_stack_down_get(ostack, nxo);
	if (nxo_type_get(nxo) == NXOT_MARK)
	{
	    break;
	}
    }
    if (i == depth)
    {
	nxo_thread_nerror(a_thread, NXN_unmatchedmark);
	return;
    }

    /* i is the index of the mark, and nxo points to the mark.  Set nelements
     * accordingly.  When we pop the nxo's off the stack, we'll have to pop
     * (nelements + 1) nxo's. */
    nelements = i;

    tnxo = nxo_stack_push(tstack);
    nxo_array_new(tnxo, nxo_thread_currentlocking(a_thread), nelements);

    /* Traverse down the stack, moving nxo's to the array. */
    for (i = nelements - 1, nxo = NULL; i >= 0; i--)
    {
	nxo = nxo_stack_down_get(ostack, nxo);
	nxo_array_el_set(tnxo, nxo, i);
    }

    /* Pop the nxo's off the stack now. */
    nxo_stack_npop(ostack, nelements + 1);

    /* Push the array onto the stack. */
    nxo = nxo_stack_push(ostack);
    nxo_dup(nxo, tnxo);

    nxo_stack_pop(tstack);
}

#ifdef CW_POSIX
void
systemdict_symlink(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack;
    cw_nxo_t *filename, *linkname, *tfilename, *tlinkname;
    int error;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    NXO_STACK_GET(linkname, ostack, a_thread);
    NXO_STACK_DOWN_GET(filename, ostack, a_thread, linkname);
    if (nxo_type_get(filename) != NXOT_STRING
	|| nxo_type_get(linkname) != NXOT_STRING)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    /* Create a copy of filename with an extra byte to store a '\0'
     * terminator. */
    tfilename = nxo_stack_push(tstack);
    nxo_string_cstring(tfilename, filename, a_thread);

    /* Create a copy of linkname with an extra byte to store a '\0'
     * terminator. */
    tlinkname = nxo_stack_push(tstack);
    nxo_string_cstring(tlinkname, linkname, a_thread);

    error = symlink(nxo_string_get(tfilename), nxo_string_get(tlinkname));
    nxo_stack_npop(tstack, 2);
    if (error == -1)
    {
	switch (errno)
	{
	    case EDQUOT:
	    case EIO:
	    case EMLINK:
	    case ENOSPC:
	    case EROFS:
	    {
		nxo_thread_nerror(a_thread, NXN_ioerror);
		break;
	    }
	    case EEXIST:
	    case ENOENT:
	    case ENOTDIR:
	    {
		nxo_thread_nerror(a_thread, NXN_undefinedfilename);
		break;
	    }
	    case EACCES:
	    case ELOOP:
	    case ENAMETOOLONG:
	    {
		nxo_thread_nerror(a_thread, NXN_invalidfileaccess);
		break;
	    }
	    case EFAULT:
	    default:
	    {
		nxo_thread_nerror(a_thread, NXN_unregistered);
	    }
	}
	return;
    }

    nxo_stack_npop(ostack, 2);
}
#endif

#ifdef CW_REAL
void
systemdict_tan(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;
    cw_nxor_t real;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    switch (nxo_type_get(nxo))
    {
	case NXOT_INTEGER:
	{
	    real = (cw_nxor_t) nxo_integer_get(nxo);
	    break;
	}
	case NXOT_REAL:
	{
	    real = nxo_real_get(nxo);
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }

    if (fabs(fmod(real, M_PI/2) - M_PI/2) < 1.0E-6)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    nxo_real_new(nxo, tan(real));
}
#endif

#ifdef CW_REAL
void
systemdict_tanh(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;
    cw_nxor_t real;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    switch (nxo_type_get(nxo))
    {
	case NXOT_INTEGER:
	{
	    real = (cw_nxor_t) nxo_integer_get(nxo);
	    break;
	}
	case NXOT_REAL:
	{
	    real = nxo_real_get(nxo);
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }
    nxo_real_new(nxo, tanh(real));
}
#endif

void
systemdict_tell(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *file;
    cw_nxoi_t position;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(file, ostack, a_thread);
	
    if (nxo_type_get(file) != NXOT_FILE)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    position = nxo_file_position_get(file);
    if (position == -1)
    {
	nxo_thread_nerror(a_thread, NXN_ioerror);
	return;
    }
    nxo_integer_new(file, position);
}

#ifdef CW_POSIX
void
systemdict_test(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *file, *test;
    cw_uint8_t c;
    cw_bool_t result;
    cw_sint32_t fd = -1;
    int error;
    struct stat sb;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(test, ostack, a_thread);
    NXO_STACK_DOWN_GET(file, ostack, a_thread, test);
    if ((nxo_type_get(file) != NXOT_FILE && nxo_type_get(file) != NXOT_STRING)
	|| nxo_type_get(test) != NXOT_STRING)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    if (nxo_string_len_get(test) != 1)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }
    nxo_string_el_get(test, 0, &c);
    switch (c)
    {
	case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'k':
	case 'p': case 'r': case 's': case 't': case 'u': case 'w': case 'x':
	case 'L': case 'O': case 'G': case 'S':
	{
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_rangecheck);
	    return;
	}
    }

    /* We haven't determined which test to perform yet, but in any case, we will
     * need the results of a stat()/fstat() call.  Do this first, then use the
     * data as appropriate later. */
    if (nxo_type_get(file) == NXOT_FILE)
    {
	fd = nxo_file_fd_get(file);
	if (fd < 0)
	{
	    nxo_thread_nerror(a_thread, NXN_invalidfileaccess);
	    return;
	}

	error = fstat(fd, &sb);
    }
    else
    {
	cw_nxo_t *tstack, *tfile;

	tstack = nxo_thread_tstack_get(a_thread);

	/* Create a copy of file with an extra byte to store a '\0'
	 * terminator. */
	tfile = nxo_stack_push(tstack);
	nxo_string_cstring(tfile, file, a_thread);

	error = lstat(nxo_string_get(tfile), &sb);

	nxo_stack_pop(tstack);
    }

    if (error == -1)
    {
	/* There was a stat error.  If this is because the file doesn't exist,
	 * set result to FALSE.  Otherwise, throw an error. */
	switch (errno)
	{
	    case EACCES:
	    case ENOENT:
	    case ENOTDIR:
	    {
		result = FALSE;
		break;
	    }
	    case EIO:
	    {
		nxo_thread_nerror(a_thread, NXN_ioerror);
		return;
	    }
	    case ELOOP:
	    case ENAMETOOLONG:
	    {
		nxo_thread_nerror(a_thread, NXN_invalidfileaccess);
		return;
	    }
	    case EBADF:
	    case EFAULT:
	    default:
	    {
		nxo_thread_nerror(a_thread, NXN_unregistered);
		return;
	    }
	}
    }
    else
    {
	switch (c)
	{
	    case 'b':
	    {
		/* Block special device? */
		result = (sb.st_mode & S_IFBLK) ? TRUE : FALSE;
		break;
	    }
	    case 'c':
	    {
		/* Character special device? */
		result = S_ISCHR(sb.st_mode) ? TRUE : FALSE;
		break;
	    }
	    case 'd':
	    {
		/* Directory? */
		result = S_ISDIR(sb.st_mode) ? TRUE : FALSE;
		break;
	    }
	    case 'e':
	    {
		/* Exists? */
		/* There was no stat error, so the file must exist. */
		result = TRUE;
		break;
	    }
	    case 'f':
	    {
		/* Regular file? */
		result = S_ISREG(sb.st_mode) ? TRUE : FALSE;
		break;
	    }
	    case 'g':
	    {
		/* Setgid? */
		result = (sb.st_mode & S_ISGID) ? TRUE : FALSE;
		break;
	    }
	    case 'k':
	    {
		/* Sticky? */
		result = (sb.st_mode & S_ISVTX) ? TRUE : FALSE;
		break;
	    }
	    case 'p':
	    {
		/* Named pipe? */
		result = S_ISFIFO(sb.st_mode) ? TRUE : FALSE;
		break;
	    }
	    case 'r':
	    {
		/* Readable? */
		result = (sb.st_mode & S_IRUSR) ? TRUE : FALSE;
		break;
	    }
	    case 's':
	    {
		/* Size greater than 0? */
		result = (sb.st_size > 0) ? TRUE : FALSE;
		break;
	    }
	    case 't':
	    {
		/* tty? */
		/* fd only.  If a string was passed in, return false. */
		if (fd == -1)
		{
		    result = FALSE;
		}
		else
		{
		    result = (isatty(fd)) ? TRUE : FALSE;
		}
		break;
	    }
	    case 'u':
	    {
		/* Setuid? */
		result = (sb.st_mode & S_ISUID) ? TRUE : FALSE;
		break;
	    }
	    case 'w':
	    {
		/* Write bit set? */
		result = (sb.st_mode & S_IWUSR) ? TRUE : FALSE;
		break;
	    }
	    case 'x':
	    {
		/* Executable bit set? */
		result = (sb.st_mode & S_IXUSR) ? TRUE : FALSE;
		break;
	    }
	    case 'L':
	    {
		/* Symbolic link? */
		result = S_ISLNK(sb.st_mode) ? TRUE : FALSE;
		break;
	    }
	    case 'O':
	    {
		/* Owner matches effective uid? */
		result = (geteuid() == sb.st_uid) ? TRUE : FALSE;
		break;
	    }
	    case 'G':
	    {
		/* Group matches effective gid? */
		result = (getegid() == sb.st_gid) ? TRUE : FALSE;
		break;
	    }
	    case 'S':
	    {
		/* Socket? */
		result = S_ISSOCK(sb.st_mode) ? TRUE : FALSE;
		break;
	    }
	    default:
	    {
		cw_not_reached();
	    }
	}
    }

    nxo_stack_pop(ostack);
    nxo_boolean_new(file, result);
}
#endif

#ifdef CW_THREADS
void
systemdict_thread(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack, *stack;
    cw_nxo_t *entry, *thread, *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);

    NXO_STACK_GET(entry, ostack, a_thread);
    NXO_STACK_DOWN_GET(stack, ostack, a_thread, entry);
    if (nxo_type_get(stack) != NXOT_STACK)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    /* Create the new thread. */
    thread = nxo_stack_under_push(ostack, stack);
    nxo_thread_new(thread, nxo_thread_nx_get(a_thread));

    /* Set up the new thread's ostack. */
    nxo_stack_copy(nxo_thread_ostack_get(thread), stack);
    nxo = nxo_stack_push(nxo_thread_ostack_get(thread));
    nxo_dup(nxo, entry);

    /* Clean up. */
    nxo_stack_npop(ostack, 2);

    /* Start the thread. */
    nxo_thread_thread(thread);
}
#endif

void
systemdict_threaddstack(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *thread, *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(thread, ostack, a_thread);
    if (nxo_type_get(thread) != NXOT_THREAD)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    nxo = nxo_stack_push(ostack);
    nxo_dup(nxo, nxo_thread_dstack_get(thread));

    nxo_stack_remove(ostack, thread);
}

void
systemdict_threadestack(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *thread, *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(thread, ostack, a_thread);
    if (nxo_type_get(thread) != NXOT_THREAD)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    nxo = nxo_stack_push(ostack);
    nxo_dup(nxo, nxo_thread_estack_get(thread));

    nxo_stack_remove(ostack, thread);
}

void
systemdict_threadistack(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *thread, *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(thread, ostack, a_thread);
    if (nxo_type_get(thread) != NXOT_THREAD)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    nxo = nxo_stack_push(ostack);
    nxo_dup(nxo, nxo_thread_istack_get(thread));

    nxo_stack_remove(ostack, thread);
}

void
systemdict_threadostack(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *thread, *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(thread, ostack, a_thread);
    if (nxo_type_get(thread) != NXOT_THREAD)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    nxo = nxo_stack_push(ostack);
    nxo_dup(nxo, nxo_thread_ostack_get(thread));

    nxo_stack_remove(ostack, thread);
}

#ifdef CW_THREADS
void
systemdict_threadsdict(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    nxo = nxo_stack_push(ostack);
    nxo_dup(nxo, nx_threadsdict_get(nxo_thread_nx_get(a_thread)));
}
#endif

#ifdef CW_THREADS
void
systemdict_timedwait(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *condition, *mutex, *nsecs;
    struct timespec timeout;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nsecs, ostack, a_thread);
    NXO_STACK_DOWN_GET(mutex, ostack, a_thread, nsecs);
    NXO_STACK_DOWN_GET(condition, ostack, a_thread, mutex);
    if (nxo_type_get(condition) != NXOT_CONDITION
	|| nxo_type_get(mutex) != NXOT_MUTEX
	|| nxo_type_get(nsecs) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    if (nxo_integer_get(nsecs) < 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    /* Convert integer to timespec. */
    timeout.tv_nsec = nxo_integer_get(nsecs) % 1000000000;
    timeout.tv_sec = nxo_integer_get(nsecs) / 1000000000;

    nxo_boolean_new(condition, nxo_condition_timedwait(condition, mutex,
						       &timeout));

    nxo_stack_npop(ostack, 2);
}
#endif

void
systemdict_token(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack;
    cw_nxo_t *nxo, *tnxo;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    switch (nxo_type_get(nxo))
    {
	case NXOT_STRING:
	{
	    cw_nxo_threadp_t threadp;
	    cw_uint32_t nscanned, scount;
	    cw_bool_t success;

	    scount = nxo_stack_count(ostack);
	    tnxo = nxo_stack_push(tstack);
	    nxo_dup(tnxo, nxo);
	    nxo_threadp_new(&threadp);

	    xep_begin();
	    xep_try
	    {
		nxo_string_lock(tnxo);
		nscanned = nxo_l_thread_token(a_thread, &threadp,
					      nxo_string_get(tnxo),
					      nxo_string_len_get(tnxo));
	    }
	    xep_acatch
	    {
		nxo_string_unlock(tnxo);
		nxo_stack_pop(tstack);
		nxo_threadp_delete(&threadp, a_thread);
	    }
	    xep_end();
	    nxo_string_unlock(tnxo);

	    if (nxo_thread_state(a_thread) == THREADTS_START
		&& nxo_thread_deferred(a_thread) == FALSE
		&& nxo_stack_count(ostack) == scount + 1)
	    {
		success = TRUE;
	    }
	    else
	    {
		xep_begin();
		xep_try
		{
		    /* Flush, but do it such that execution is deferred. */
		    nxo_l_thread_token(a_thread, &threadp, "\n", 1);
		}
		xep_acatch
		{
		    nxo_stack_pop(tstack);
		    nxo_threadp_delete(&threadp, a_thread);
		}
		xep_end();

		if (nxo_thread_state(a_thread) == THREADTS_START
		    && nxo_thread_deferred(a_thread) == FALSE
		    && nxo_stack_count(ostack) == scount + 1)
		{
		    success = TRUE;
		}
		else
		{
		    success = FALSE;
		}
	    }

	    if (success)
	    {
		nxo_string_substring_new(nxo, tnxo, nscanned,
					 nxo_string_len_get(tnxo) - nscanned);
		nxo = nxo_stack_push(ostack);
		nxo_boolean_new(nxo, TRUE);
	    }
	    else
	    {
		cw_uint32_t i;

		/* We failed to scan a token.  Reset a_thread and clean up
		 * ostack. */
		nxo_thread_reset(a_thread);
		for (i = nxo_stack_count(ostack); i > scount; i--)
		{
		    nxo_stack_pop(ostack);
		}

		nxo_boolean_new(nxo, FALSE);
	    }
	    nxo_threadp_delete(&threadp, a_thread);
	    nxo_stack_pop(tstack);
	    break;
	}
	case NXOT_FILE:
	{
	    cw_nxo_threadp_t threadp;
	    cw_sint32_t nread;
	    cw_uint32_t scount;
	    cw_uint8_t c;

	    scount = nxo_stack_count(ostack);
	    tnxo = nxo_stack_push(tstack);
	    nxo_dup(tnxo, nxo);
	    nxo_threadp_new(&threadp);

	    /* Feed the scanner one byte at a time, checking after every
	     * character whether a token was accepted.  If we run out of data,
	     * flush the scanner in the hope of causing token acceptance. */
	    for (nread = nxo_file_read(tnxo, 1, &c);
		 nread > 0;
		 nread = nxo_file_read(tnxo, 1, &c))
	    {
		xep_begin();
		xep_try
		{
		    nxo_l_thread_token(a_thread, &threadp, &c, 1);
		}
		xep_acatch
		{
		    nxo_stack_pop(tstack);
		    nxo_threadp_delete(&threadp, a_thread);
		}
		xep_end();

		if (nxo_thread_state(a_thread) == THREADTS_START
		    && nxo_thread_deferred(a_thread) == FALSE
		    && nxo_stack_count(ostack) == scount + 1)
		{
		    goto SUCCESS;
		}
	    }
	    xep_begin();
	    xep_try
	    {
		/* Flush, but do it such that execution is deferred. */
		nxo_l_thread_token(a_thread, &threadp, "\n", 1);
	    }
	    xep_acatch
	    {
		nxo_stack_pop(tstack);
		nxo_threadp_delete(&threadp, a_thread);
	    }
	    xep_end();

	    if (nxo_thread_state(a_thread) == THREADTS_START
		&& nxo_thread_deferred(a_thread) == FALSE
		&& nxo_stack_count(ostack) == scount + 1)
	    {
		/* Success. */
		SUCCESS:
		nxo_boolean_new(nxo, TRUE);
		nxo_stack_exch(ostack);
	    }
	    else
	    {
		cw_uint32_t i;

		/* We failed to scan a token.  Reset a_thread and clean up
		 * ostack. */
		nxo_thread_reset(a_thread);
		for (i = nxo_stack_count(ostack); i > scount; i--)
		{
		    nxo_stack_pop(ostack);
		}

		nxo_boolean_new(nxo, FALSE);
	    }
	    nxo_threadp_delete(&threadp, a_thread);
	    nxo_stack_pop(tstack);
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }
}

#ifdef CW_REAL
void
systemdict_trunc(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    switch (nxo_type_get(nxo))
    {
	case NXOT_INTEGER:
	{
	    break;
	}
	case NXOT_REAL:
	{
	    nxo_integer_new(nxo, (cw_nxoi_t) nxo_real_get(nxo));
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }
}
#endif

#ifdef CW_POSIX
void
systemdict_truncate(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *file, *length;
    cw_nxoi_t len;
    cw_nxn_t error;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(length, ostack, a_thread);
    NXO_STACK_DOWN_GET(file, ostack, a_thread, length);
    if (nxo_type_get(file) != NXOT_FILE || nxo_type_get(length) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    len = nxo_integer_get(length);
    if (len < 0)
    {
	nxo_thread_nerror(a_thread, NXN_rangecheck);
	return;
    }

    error = nxo_file_truncate(file, len);
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }

    nxo_stack_npop(ostack, 2);
}
#endif

#ifdef CW_THREADS
void
systemdict_trylock(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *mutex;
    cw_bool_t error;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(mutex, ostack, a_thread);
    if (nxo_type_get(mutex) != NXOT_MUTEX)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    error = nxo_mutex_trylock(mutex);

    nxo_boolean_new(mutex, error);
}
#endif

void
systemdict_tuck(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *top, *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(top, ostack, a_thread);
    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, top);
    nxo = nxo_stack_under_push(ostack, nxo);
    nxo_dup(nxo, top);
}

void
systemdict_type(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;
    cw_nxot_t type;
    /* Must be kept in sync with cw_nxot_t. */
    static const cw_nxn_t typenames[] = {
	0,
	NXN_arraytype,
	NXN_booleantype,
#ifdef CW_THREADS
	NXN_conditiontype,
#endif
	NXN_dicttype,
	NXN_filetype,
	NXN_finotype,
#ifdef CW_HANDLE
	NXN_handletype,
#endif
	NXN_integertype,
	NXN_marktype,
#ifdef CW_THREADS
	NXN_mutextype,
#endif
	NXN_nametype,
	NXN_nulltype,
	NXN_operatortype,
	NXN_pmarktype,
#ifdef CW_REAL
	NXN_realtype,
#endif
#ifdef CW_REGEX
	NXN_regextype,
	NXN_regsubtype,
#endif
	NXN_stacktype,
	NXN_stringtype,
	NXN_threadtype
    };
    cw_assert(sizeof(typenames) / sizeof(cw_nxn_t) == NXOT_LAST + 1);

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);

    type = nxo_type_get(nxo);
    cw_assert(type > NXOT_NO && type <= NXOT_LAST);

    nxo_name_new(nxo, nxn_str(typenames[type]), nxn_len(typenames[type]), TRUE);
    nxo_attr_set(nxo, NXOA_EXECUTABLE);
}

#ifdef CW_POSIX
void
systemdict_uid(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    nxo = nxo_stack_push(ostack);
    nxo_integer_new(nxo, getuid());
}
#endif

#ifdef CW_POSIX
void
systemdict_umask(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    mode_t omode, nmode;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    omode = nxo_integer_get(nxo);
    nmode = umask(omode);
    nxo_integer_new(nxo, nmode);
}
#endif

void
systemdict_undef(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *dict, *key;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(key, ostack, a_thread);
    NXO_STACK_DOWN_GET(dict, ostack, a_thread, key);
    if (nxo_type_get(dict) != NXOT_DICT)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    nxo_dict_undef(dict, key);

    nxo_stack_npop(ostack, 2);
}

void
systemdict_under(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *under, *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_NGET(under, ostack, a_thread, 1);
    nxo = nxo_stack_under_push(ostack, under);
    nxo_dup(nxo, under);
}

void
systemdict_unless(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *cond, *exec;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(exec, ostack, a_thread);
    NXO_STACK_DOWN_GET(cond, ostack, a_thread, exec);
    if (nxo_type_get(cond) != NXOT_BOOLEAN)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    if (nxo_boolean_get(cond) == FALSE)
    {
	cw_nxo_t *estack;
	cw_nxo_t *nxo;

	estack = nxo_thread_estack_get(a_thread);
	nxo = nxo_stack_push(estack);
	nxo_dup(nxo, exec);
	nxo_stack_npop(ostack, 2);
	nxo_thread_loop(a_thread);
    }
    else
    {
	nxo_stack_npop(ostack, 2);
    }
}

#ifdef CW_POSIX
void
systemdict_unlink(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack;
    cw_nxo_t *filename, *tfilename;
    int error;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    NXO_STACK_GET(filename, ostack, a_thread);

    if (nxo_type_get(filename) != NXOT_STRING)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    /* Create a copy of filename with an extra byte to store a '\0'
     * terminator. */
    tfilename = nxo_stack_push(tstack);
    nxo_string_cstring(tfilename, filename, a_thread);

    error = unlink(nxo_string_get(tfilename));

    nxo_stack_pop(tstack);

    if (error == -1)
    {
	switch (errno)
	{
	    case EIO:
	    case EBUSY:
	    case ELOOP:
	    case EROFS:
	    {
		nxo_thread_nerror(a_thread, NXN_ioerror);
		break;
	    }
	    case ENOENT:
	    case ENOTDIR:
	    case ENAMETOOLONG:
	    {
		nxo_thread_nerror(a_thread, NXN_undefinedfilename);
		break;
	    }
	    case EACCES:
	    case EPERM:
	    {
		nxo_thread_nerror(a_thread, NXN_invalidfileaccess);
		break;
	    }
	    case EFAULT:
	    default:
	    {
		nxo_thread_nerror(a_thread, NXN_unregistered);
	    }
	}
	return;
    }

    nxo_stack_pop(ostack);
}
#endif

#ifdef CW_THREADS
void
systemdict_unlock(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *mutex;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(mutex, ostack, a_thread);
    if (nxo_type_get(mutex) != NXOT_MUTEX)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    nxo_mutex_unlock(mutex);

    nxo_stack_pop(ostack);
}
#endif

#ifdef CW_POSIX
void
systemdict_unsetenv(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack, *envdict;
    cw_nxo_t *key, *tkey;
    cw_uint32_t len;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    envdict = libonyx_envdict_get();
    NXO_STACK_GET(key, ostack, a_thread);
    if (nxo_type_get(key) != NXOT_NAME)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    tkey = nxo_stack_push(tstack);
    len = nxo_name_len_get(key);
#ifdef HAVE_UNSETENV
    /* Create a copy of the key with an extra byte to store a '\0'
     * terminator. */
    nxo_string_new(tkey, FALSE, len + 1);
    nxo_string_set(tkey, 0, nxo_name_str_get(key), len);
    nxo_string_el_set(tkey, '\0', len);

    /* Do the unsetenv(). */
    unsetenv(nxo_string_get(tkey));
#else
    /* Create a copy of the key with an extra 2 bytes to append "=\0". */
    nxo_string_new(tkey, FALSE, len + 2);
    nxo_string_set(tkey, 0, nxo_name_str_get(key), len);
    nxo_string_set(tkey, len, "=\0", 2);

    /* Do the putenv(). */
    if (putenv(nxo_string_get(tkey)) == -1)
    {
	xep_throw(CW_ONYXX_OOM);
    }
#endif
    nxo_stack_pop(tstack);

    /* Undefine the key/value pair in envdict. */
    nxo_dict_undef(envdict, key);

    nxo_stack_pop(ostack);
}
#endif

void
systemdict_until(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *estack, *tstack;
    cw_nxo_t *nxo, *exec, *cond;
    cw_uint32_t edepth, tdepth;
    cw_bool_t do_exec;
    cw_nxn_t nerror;

    ostack = nxo_thread_ostack_get(a_thread);
    estack = nxo_thread_estack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);

    NXO_STACK_GET(cond, ostack, a_thread);
    NXO_STACK_DOWN_GET(exec, ostack, a_thread, cond);

    /* Move exec and cond to tstack. */
    nxo = nxo_stack_push(tstack);
    nxo_dup(nxo, exec);
    exec = nxo;

    nxo = nxo_stack_push(tstack);
    nxo_dup(nxo, cond);
    cond = nxo;

    nxo_stack_npop(ostack, 2);

    /* Record stack depths so that we can clean up if necessary. */
    edepth = nxo_stack_count(estack);
    tdepth = nxo_stack_count(tstack);

    /* Catch an exit exception, if thrown, but do not continue executing the
     * loop. */
    xep_begin();
    xep_try
    {
	while (1)
	{
	    /* Execute the body of the until statement. */
	    nxo = nxo_stack_push(estack);
	    nxo_dup(nxo, exec);
	    nxo_thread_loop(a_thread);

	    /* Execute the conditional. */
	    nxo = nxo_stack_push(estack);
	    nxo_dup(nxo, cond);
	    nxo_thread_loop(a_thread);

	    /* Get the conditional result. */
	    nxo = nxo_stack_get(ostack);
	    if (nxo == NULL)
	    {
		nerror = NXN_stackunderflow;
	    }
	    else if (nxo_type_get(nxo) != NXOT_BOOLEAN)
	    {
		nerror = NXN_typecheck;
	    }
	    else
	    {
		nerror = NXN_ZERO;
	    }

	    if (nerror != NXN_ZERO)
	    {
		/* Push the inputs back onto ostack before throwing an error. */
		nxo = nxo_stack_push(ostack);
		nxo_dup(nxo, exec);
		nxo = nxo_stack_push(ostack);
		nxo_dup(nxo, cond);

		nxo_stack_npop(tstack, 2);

		nxo_thread_nerror((a_thread), nerror);
		return;
	    }

	    /* Get the result of the conditional, and break out of the loop if
	     * the result was FALSE. */
	    do_exec = nxo_boolean_get(nxo);
	    nxo_stack_pop(ostack);
	    if (do_exec == FALSE)
	    {
		break;
	    }
	}
    }
    xep_catch(CW_ONYXX_EXIT)
    {
	cw_nxo_t *istack;

	xep_handled();

	/* Clean up stacks. */
	nxo_stack_npop(estack, nxo_stack_count(estack) - edepth);
	istack = nxo_thread_istack_get(a_thread);
	nxo_stack_npop(istack, nxo_stack_count(istack)
		       - nxo_stack_count(estack));
	nxo_stack_npop(tstack, nxo_stack_count(tstack) - tdepth);
    }
    xep_end();
    
    /* Remove exec and cond from tstack. */
    nxo_stack_npop(tstack, 2);
}

void
systemdict_up(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;

    ostack = nxo_thread_ostack_get(a_thread);
    if (nxo_stack_count(ostack) < 3)
    {
	nxo_thread_nerror(a_thread, NXN_stackunderflow);
	return;
    }

    nxo_stack_roll(ostack, 3, 1);
}

#ifdef CW_THREADS
void
systemdict_wait(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *condition, *mutex;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(mutex, ostack, a_thread);
    NXO_STACK_DOWN_GET(condition, ostack, a_thread, mutex);
    if (nxo_type_get(condition) != NXOT_CONDITION
	|| nxo_type_get(mutex) != NXOT_MUTEX)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    nxo_condition_wait(condition, mutex);

    nxo_stack_npop(ostack, 2);
}
#endif

#ifdef CW_POSIX
void
systemdict_waitpid(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;
    pid_t pid;
    int status;
    cw_nxoi_t result;
	
    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    if (nxo_type_get(nxo) != NXOT_INTEGER)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }
    pid = nxo_integer_get(nxo);

    while (waitpid(pid, &status, 0) == -1)
    {
	if (errno != EINTR)
	{
	    nxo_thread_nerror(a_thread, NXN_unregistered);
	    return;
	}
    }
    if (WIFEXITED(status))
    {
	/* Normal program exit. */
	result = WEXITSTATUS(status);
    }
    else
    {
	/* Program termination due to a signal.  Set a negative return value. */
	result = -WTERMSIG(status);
    }

    nxo_integer_new(nxo, result);
}
#endif

void
systemdict_where(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *dstack;
    cw_nxo_t *dict, *key, *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    dstack = nxo_thread_dstack_get(a_thread);

    NXO_STACK_GET(key, ostack, a_thread);

    /* Iteratively search the dictionaries on the dictionary stack for key. */
    for (dict = nxo_stack_get(dstack);
	 dict != NULL;
	 dict = nxo_stack_down_get(dstack, dict))
    {
	if (nxo_dict_lookup(dict, key, NULL) == FALSE)
	{
	    /* Found. */
	    nxo = nxo_stack_push(ostack);
	    nxo_dup(key, dict);
	    nxo_boolean_new(nxo, TRUE);
	    return;
	}
    }
    /* Not found. */
    nxo_boolean_new(key, FALSE);
}

void
systemdict_while(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *estack, *tstack;
    cw_nxo_t *nxo, *exec, *cond;
    cw_uint32_t edepth, tdepth;
    cw_bool_t do_exec;
    cw_nxn_t nerror;

    ostack = nxo_thread_ostack_get(a_thread);
    estack = nxo_thread_estack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);

    NXO_STACK_GET(exec, ostack, a_thread);
    NXO_STACK_DOWN_GET(cond, ostack, a_thread, exec);

    /* Move exec and cond to tstack. */
    nxo = nxo_stack_push(tstack);
    nxo_dup(nxo, cond);
    cond = nxo;

    nxo = nxo_stack_push(tstack);
    nxo_dup(nxo, exec);
    exec = nxo;

    nxo_stack_npop(ostack, 2);

    /* Record stack depths so that we can clean up if necessary. */
    edepth = nxo_stack_count(estack);
    tdepth = nxo_stack_count(tstack);

    /* Catch an exit exception, if thrown, but do not continue executing the
     * loop. */
    xep_begin();
    xep_try
    {
	while (1)
	{
	    /* Execute the conditional. */
	    nxo = nxo_stack_push(estack);
	    nxo_dup(nxo, cond);
	    nxo_thread_loop(a_thread);

	    /* Get the conditional result. */
	    nxo = nxo_stack_get(ostack);
	    if (nxo == NULL)
	    {
		nerror = NXN_stackunderflow;
	    }
	    else if (nxo_type_get(nxo) != NXOT_BOOLEAN)
	    {
		nerror = NXN_typecheck;
	    }
	    else
	    {
		nerror = NXN_ZERO;
	    }

	    if (nerror != NXN_ZERO)
	    {
		/* Push the inputs back onto ostack before throwing an error. */
		nxo = nxo_stack_push(ostack);
		nxo_dup(nxo, cond);
		nxo = nxo_stack_push(ostack);
		nxo_dup(nxo, exec);

		nxo_stack_npop(tstack, 2);

		nxo_thread_nerror((a_thread), nerror);
		return;
	    }

	    /* Get the result of the conditional, and break out of the loop if
	     * the result was FALSE. */
	    do_exec = nxo_boolean_get(nxo);
	    nxo_stack_pop(ostack);
	    if (do_exec == FALSE)
	    {
		break;
	    }

	    /* Execute the body of the while statement. */
	    nxo = nxo_stack_push(estack);
	    nxo_dup(nxo, exec);
	    nxo_thread_loop(a_thread);
	}
    }
    xep_catch(CW_ONYXX_EXIT)
    {
	cw_nxo_t *istack;

	xep_handled();

	/* Clean up stacks. */
	nxo_stack_npop(estack, nxo_stack_count(estack) - edepth);
	istack = nxo_thread_istack_get(a_thread);
	nxo_stack_npop(istack, nxo_stack_count(istack)
		       - nxo_stack_count(estack));
	nxo_stack_npop(tstack, nxo_stack_count(tstack) - tdepth);
    }
    xep_end();
    
    /* Remove exec and cond from tstack. */
    nxo_stack_npop(tstack, 2);
}

void
systemdict_write(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *file, *value;
    cw_nxn_t error;
    cw_uint32_t count;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(value, ostack, a_thread);
    NXO_STACK_DOWN_GET(file, ostack, a_thread, value);
	
    if (nxo_type_get(file) != NXOT_FILE)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    switch (nxo_type_get(value))
    {
	case NXOT_INTEGER:
	{
	    cw_uint8_t val;

	    val = (cw_uint8_t) nxo_integer_get(value);
	    error = nxo_file_write(file, &val, 1, &count);
	    if (error)
	    {
		nxo_thread_nerror(a_thread, error);
		return;
	    }
	    if (count == 1)
	    {
		/* Successful write. */
		nxo_boolean_new(file, FALSE);
		nxo_stack_pop(ostack);
	    }
	    else
	    {
		/* Short write. */
		nxo_dup(file, value);
		nxo_boolean_new(value, TRUE);
	    }
	    break;
	}
	case NXOT_STRING:
	{
	    cw_uint32_t len;

	    nxo_string_lock(value);
	    len = nxo_string_len_get(value);
	    error = nxo_file_write(file, nxo_string_get(value), len, &count);
	    nxo_string_unlock(value);
	    if (error)
	    {
		nxo_thread_nerror(a_thread, error);
		return;
	    }
	    if (count == len)
	    {
		/* Successful write. */
		nxo_boolean_new(file, FALSE);
		nxo_stack_pop(ostack);
	    }
	    else
	    {
		/* Short write. */
		nxo_string_substring_new(file, value, count, len - count);
		nxo_boolean_new(value, TRUE);
	    }
	    break;
	}
	default:
	{
	    nxo_thread_nerror(a_thread, NXN_typecheck);
	    return;
	}
    }
}

void
systemdict_xcheck(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
	
    if (nxo_attr_get(nxo) == NXOA_LITERAL)
    {
	nxo_boolean_new(nxo, FALSE);
    }
    else
    {
	nxo_boolean_new(nxo, TRUE);
    }
}

void
systemdict_xor(cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack;
    cw_nxo_t *nxo_a, *nxo_b;

    ostack = nxo_thread_ostack_get(a_thread);

    NXO_STACK_GET(nxo_b, ostack, a_thread);
    NXO_STACK_DOWN_GET(nxo_a, ostack, a_thread, nxo_b);

    if (nxo_type_get(nxo_a) == NXOT_BOOLEAN
	&& nxo_type_get(nxo_b) == NXOT_BOOLEAN)
    {
	cw_bool_t xor;

	if (nxo_boolean_get(nxo_a) || nxo_boolean_get(nxo_b))
	{
	    if (nxo_boolean_get(nxo_a) == nxo_boolean_get(nxo_b))
	    {
		xor = FALSE;
	    }
	    else
	    {
		xor = TRUE;
	    }
	}
	else
	{
	    xor = FALSE;
	}
	nxo_boolean_new(nxo_a, xor);
    }
    else if (nxo_type_get(nxo_a) == NXOT_INTEGER
	     && nxo_type_get(nxo_b) == NXOT_INTEGER)
    {
	nxo_integer_set(nxo_a, nxo_integer_get(nxo_a) ^ nxo_integer_get(nxo_b));
    }
    else
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    nxo_stack_pop(ostack);
}

#ifdef CW_THREADS
void
systemdict_yield(cw_nxo_t *a_thread)
{
    thd_yield();
}
#endif
