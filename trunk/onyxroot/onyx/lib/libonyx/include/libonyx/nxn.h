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

/* Define some cpp magic to allow this file to be treated differently when
 * included by nxn.c, so that cw_g_nx_names can be defined there. */
#ifdef CW_NXN_C_
#define NXN_XENTRY(a_nxn, a_str)	a_str
#else
#define NXN_XENTRY(a_nxn, a_str)	a_nxn
#endif
#define NXN_SENTRY(a_nxn, a_str)	NXN_XENTRY(NXN_##a_nxn, a_str)
#define NXN_ENTRY(a_name)		NXN_XENTRY(NXN_##a_name, #a_name)

/* If nxn.c is including this file the second time, then the following is read
 * as an array of string constants.  Otherwise, it is read as an enumeration,
 * which directly corresponds to the array of string constants.  This trickery
 * is lamentable, but is better than the alternative of maintaining the two
 * representations separately. */
#ifdef CW_NXN_C_
const cw_uint8_t *cw_g_nx_names[] =
#else
extern const cw_uint8_t *cw_g_nx_names[];
typedef enum
#endif
{
    NXN_SENTRY(ZERO, ""),
    NXN_SENTRY(sym_lp, "("),
    NXN_SENTRY(sym_rp, ")"),
    NXN_SENTRY(sym_lt, "<"),
    NXN_SENTRY(sym_gt, ">"),
    NXN_SENTRY(sym_lb, "["),
    NXN_SENTRY(sym_rb, "]"),
#ifdef CW_SOCKET
    NXN_ENTRY(AF_INET),
    NXN_ENTRY(AF_LOCAL),
#endif
    NXN_ENTRY(abs),
#ifdef CW_SOCKET
    NXN_ENTRY(accept),
#endif
    NXN_ENTRY(active),
    NXN_ENTRY(add),
#ifdef CW_SOCKET
    NXN_ENTRY(address),
#endif
    NXN_ENTRY(adn),
    NXN_ENTRY(and),
    NXN_ENTRY(argcheck),
    NXN_ENTRY(argv),
    NXN_ENTRY(array),
    NXN_ENTRY(arraytype),
#ifdef CW_REAL
    NXN_ENTRY(atan),
#endif
#ifdef CW_POSIX
    NXN_ENTRY(atime),
#endif
    NXN_ENTRY(aup),
    NXN_ENTRY(bdup),
    NXN_ENTRY(begin),
    NXN_ENTRY(bind),
#ifdef CW_SOCKET
    NXN_ENTRY(bindsocket),
#endif
#ifdef CW_POSIX
    NXN_ENTRY(blksize),
    NXN_ENTRY(blocks),
#endif
    NXN_ENTRY(booleantype),
    NXN_ENTRY(bpop),
#ifdef CW_THREADS
    NXN_ENTRY(broadcast),
#endif
    NXN_ENTRY(bytesavailable),
#ifdef CW_REGEX
    NXN_ENTRY(c),
#endif
    NXN_ENTRY(cat),
#ifdef CW_POSIX
    NXN_ENTRY(cd),
#endif
#ifdef CW_REAL
    NXN_ENTRY(ceiling),
#endif
#ifdef CW_POSIX
    NXN_ENTRY(chmod),
    NXN_ENTRY(chown),
    NXN_ENTRY(chroot),
#endif
    NXN_ENTRY(clear),
    NXN_ENTRY(cleartomark),
#ifdef CW_POSIX
    NXN_ENTRY(close),
#endif
    NXN_ENTRY(collect),
#ifdef CW_THREADS
    NXN_ENTRY(condition),
    NXN_ENTRY(conditiontype),
#endif
#ifdef CW_SOCKET
    NXN_ENTRY(connect),
#endif
    NXN_ENTRY(copy),
#ifdef CW_REAL
    NXN_ENTRY(cos),
#endif
    NXN_ENTRY(count),
    NXN_ENTRY(countdstack),
    NXN_ENTRY(countestack),
    NXN_ENTRY(counttomark),
#ifdef CW_POSIX
    NXN_ENTRY(ctime),
#endif
    NXN_ENTRY(currentdict),
#ifdef CW_THREADS
    NXN_ENTRY(currentlocking),
#endif
#ifdef CW_REAL
    NXN_ENTRY(cvds),
#endif
    NXN_ENTRY(cve),
#ifdef CW_REAL
    NXN_ENTRY(cves),
#endif
    NXN_ENTRY(cvlit),
    NXN_ENTRY(cvn),
    NXN_ENTRY(cvrs),
    NXN_ENTRY(cvs),
    NXN_ENTRY(cvx),
    NXN_ENTRY(dec),
    NXN_ENTRY(def),
#ifdef CW_THREADS
    NXN_ENTRY(detach),
#endif
#ifdef CW_POSIX
    NXN_ENTRY(dev),
#endif
    NXN_ENTRY(dict),
    NXN_ENTRY(dicttype),
    NXN_ENTRY(die),
#ifdef CW_POSIX
    NXN_ENTRY(dirforeach),
#endif
#ifdef CW_REAL
    NXN_ENTRY(div),
#endif
    NXN_ENTRY(dn),
    NXN_ENTRY(dstack),
    NXN_ENTRY(dup),
    NXN_ENTRY(echeck),
#ifdef CW_POSIX
    NXN_ENTRY(egid),
#endif
    NXN_ENTRY(end),
#ifdef CW_POSIX
    NXN_ENTRY(envdict),
#endif
    NXN_ENTRY(eq),
    NXN_ENTRY(errorname),
    NXN_ENTRY(estack),
    NXN_ENTRY(estackoverflow),
#ifdef CW_POSIX
    NXN_ENTRY(euid),
#endif
    NXN_ENTRY(eval),
    NXN_ENTRY(exch),
#ifdef CW_POSIX
    NXN_ENTRY(exec),
#endif
    NXN_ENTRY(exit),
    NXN_ENTRY(exp),
    NXN_ENTRY(false),
#ifdef CW_POSIX
    NXN_ENTRY(family),
#endif
    NXN_ENTRY(filetype),
    NXN_ENTRY(finotype),
#ifdef CW_REAL
    NXN_ENTRY(floor),
#endif
    NXN_ENTRY(flush),
    NXN_ENTRY(flushfile),
    NXN_ENTRY(for),
    NXN_ENTRY(foreach),
#ifdef CW_POSIX
    NXN_ENTRY(forkexec),
#endif
#ifdef CW_REGEX
    NXN_ENTRY(g),
#endif
    NXN_ENTRY(gcdict),
    NXN_ENTRY(ge),
    NXN_ENTRY(get),
    NXN_ENTRY(getinterval),
#ifdef CW_POSIX
    NXN_ENTRY(gid),
#endif
    NXN_ENTRY(globaldict),
#ifdef CW_THREADS
    NXN_ENTRY(gstderr),
    NXN_ENTRY(gstdin),
    NXN_ENTRY(gstdout),
#endif
    NXN_ENTRY(gt),
    NXN_ENTRY(hooktag),
    NXN_ENTRY(hooktype),
#ifdef CW_POSIX
    NXN_ENTRY(IPPROTO_IP),
    NXN_ENTRY(IPPROTO_HOPOPTS),
    NXN_ENTRY(IPPROTO_ICMP),
    NXN_ENTRY(IPPROTO_IGMP),
    NXN_ENTRY(IPPROTO_IPIP),
    NXN_ENTRY(IPPROTO_TCP),
    NXN_ENTRY(IPPROTO_EGP),
    NXN_ENTRY(IPPROTO_PUP),
    NXN_ENTRY(IPPROTO_UDP),
    NXN_ENTRY(IPPROTO_IDP),
    NXN_ENTRY(IPPROTO_TP),
    NXN_ENTRY(IPPROTO_IPV6),
    NXN_ENTRY(IPPROTO_ROUTING),
    NXN_ENTRY(IPPROTO_FRAGMENT),
    NXN_ENTRY(IPPROTO_RSVP),
    NXN_ENTRY(IPPROTO_GRE),
    NXN_ENTRY(IPPROTO_ESP),
    NXN_ENTRY(IPPROTO_AH),
    NXN_ENTRY(IPPROTO_ICMPV6),
    NXN_ENTRY(IPPROTO_NONE),
    NXN_ENTRY(IPPROTO_DSTOPTS),
    NXN_ENTRY(IPPROTO_MTP),
    NXN_ENTRY(IPPROTO_ENCAP),
    NXN_ENTRY(IPPROTO_PIM),
    NXN_ENTRY(IPPROTO_COMP),
    NXN_ENTRY(IPPROTO_RAW),
#endif
#ifdef CW_REGEX
    NXN_ENTRY(i),
#endif
    NXN_ENTRY(ibdup),
    NXN_ENTRY(ibpop),
    NXN_ENTRY(idiv),
    NXN_ENTRY(idup),
    NXN_ENTRY(if),
    NXN_ENTRY(ifelse),
    NXN_ENTRY(inc),
#ifdef CW_POSIX
    NXN_ENTRY(ino),
#endif
    NXN_ENTRY(integertype),
    NXN_ENTRY(invalidaccess),
    NXN_ENTRY(invalidexit),
    NXN_ENTRY(invalidfileaccess),
    NXN_ENTRY(iobuf),
    NXN_ENTRY(ioerror),
    NXN_ENTRY(ipop),
    NXN_ENTRY(istack),
#ifdef CW_THREADS
    NXN_ENTRY(join),
#endif
    NXN_ENTRY(known),
#ifdef CW_THREADS
    NXN_ENTRY(lcheck),
#endif
    NXN_ENTRY(le),
    NXN_ENTRY(length),
    NXN_ENTRY(limitcheck),
#ifdef CW_POSIX
    NXN_ENTRY(link),
#endif
#ifdef CW_SOCKET
    NXN_ENTRY(listen),
#endif
#ifdef CW_REAL
    NXN_ENTRY(ln),
#endif
    NXN_ENTRY(load),
#ifdef CW_THREADS
    NXN_ENTRY(lock),
#endif
#ifdef CW_REAL
    NXN_ENTRY(log),
#endif
    NXN_ENTRY(loop),
    NXN_ENTRY(lt),
#ifdef CW_SOCKET
    NXN_ENTRY(MSG_OOB),
    NXN_ENTRY(MSG_PEEK),
    NXN_ENTRY(MSG_WAITALL),
#endif
#ifdef CW_REGEX
    NXN_ENTRY(m),
#endif
    NXN_ENTRY(mark),
    NXN_ENTRY(marktype),
#ifdef CW_REGEX
    NXN_ENTRY(match),
    NXN_ENTRY(matches),
#endif
#ifdef CW_POSIX
    NXN_ENTRY(mkdir),
    NXN_ENTRY(mkfifo),
#endif
    NXN_ENTRY(mod),
#ifdef CW_POSIX
    NXN_ENTRY(mode),
#endif
#ifdef CW_MODULES
    NXN_ENTRY(modload),
#endif
#ifdef CW_THREADS
    NXN_ENTRY(monitor),
#endif
#ifdef CW_POSIX
    NXN_ENTRY(mtime),
#endif
    NXN_ENTRY(mul),
#ifdef CW_THREADS
    NXN_ENTRY(mutex),
    NXN_ENTRY(mutextype),
#endif
    NXN_ENTRY(nametype),
    NXN_ENTRY(nbpop),
    NXN_ENTRY(ncat),
    NXN_ENTRY(ndn),
    NXN_ENTRY(ndup),
    NXN_ENTRY(ne),
#ifdef CW_POSIX
    NXN_ENTRY(neterror),
#endif
    NXN_ENTRY(neg),
    NXN_ENTRY(newerror),
    NXN_ENTRY(nip),
#ifdef CW_POSIX
    NXN_ENTRY(nlink),
#endif
    NXN_ENTRY(nonblocking),
    NXN_ENTRY(not),
    NXN_ENTRY(npop),
#ifdef CW_POSIX
    NXN_ENTRY(nsleep),
#endif
    NXN_ENTRY(null),
    NXN_ENTRY(nulltype),
    NXN_ENTRY(nup),
#ifdef CW_REGEX
    NXN_ENTRY(offsets),
#endif
#ifdef CW_POSIX
    NXN_ENTRY(on),
#endif
    NXN_ENTRY(onyxdict),
#ifdef CW_POSIX
    NXN_ENTRY(open),
#endif
    NXN_ENTRY(operatortype),
    NXN_ENTRY(or),
    NXN_ENTRY(ostack),
    NXN_ENTRY(over),
#ifdef CW_SOCKET
    NXN_ENTRY(path),
#endif
#ifdef CW_POSIX
    NXN_ENTRY(POLLERR),
    NXN_ENTRY(POLLHUP),
    NXN_ENTRY(POLLIN),
    NXN_ENTRY(POLLNVAL),
    NXN_ENTRY(POLLOUT),
    NXN_ENTRY(POLLPRI),
    NXN_ENTRY(POLLRDBAND),
    NXN_ENTRY(POLLRDNORM),
    NXN_ENTRY(POLLWRNORM),
    NXN_ENTRY(POLLWRBAND),
#endif
#ifdef CW_SOCKET
    NXN_ENTRY(peername),
#endif
#ifdef CW_PTHREADS
    NXN_ENTRY(period),
#endif
#ifdef CW_POSIX
    NXN_ENTRY(pid),
#endif
    NXN_ENTRY(pmark),
    NXN_ENTRY(pmarktype),
#ifdef CW_POSIX
    NXN_ENTRY(pipe),
    NXN_ENTRY(poll),
#endif
    NXN_ENTRY(pop),
#ifdef CW_POSIX
    NXN_ENTRY(port),
    NXN_ENTRY(ppid),
#endif
    NXN_ENTRY(print),
    NXN_ENTRY(product),
    NXN_ENTRY(promptstring),
    NXN_ENTRY(put),
    NXN_ENTRY(putinterval),
#ifdef CW_POSIX
    NXN_ENTRY(pwd),
#endif
    NXN_ENTRY(quit),
    NXN_ENTRY(rand),
    NXN_ENTRY(rangecheck),
#ifdef CW_POSIX
    NXN_ENTRY(rdev),
#endif
    NXN_ENTRY(read),
    NXN_ENTRY(readline),
#ifdef CW_POSIX
    NXN_ENTRY(readlink),
    NXN_ENTRY(realtime),
#endif
#ifdef CW_REAL
    NXN_ENTRY(realtype),
#endif
#ifdef CW_SOCKET
    NXN_ENTRY(recv),
#endif
#ifdef CW_REGEX
    NXN_ENTRY(regex),
    NXN_ENTRY(regexerror),
    NXN_ENTRY(regextype),
    NXN_ENTRY(regsub),
    NXN_ENTRY(regsubtype),
#endif
#ifdef CW_POSIX
    NXN_ENTRY(rename),
#endif
    NXN_ENTRY(repeat),
#ifdef CW_POSIX
    NXN_ENTRY(rmdir),
#endif
    NXN_ENTRY(roll),
    NXN_ENTRY(rot),
#ifdef CW_REAL
    NXN_ENTRY(round),
#endif
#ifdef CW_SOCKET
    NXN_ENTRY(SO_BROADCAST),
    NXN_ENTRY(SO_DEBUG),
    NXN_ENTRY(SO_DONTROUTE),
    NXN_ENTRY(SO_ERROR),
    NXN_ENTRY(SO_KEEPALIVE),
    NXN_ENTRY(SO_LINGER),
    NXN_ENTRY(SO_OOBINLINE),
    NXN_ENTRY(SO_RCVBUF),
    NXN_ENTRY(SO_RCVLOWAT),
    NXN_ENTRY(SO_RCVTIMEO),
    NXN_ENTRY(SO_REUSEADDR),
#ifdef SO_REUSEPORT
    NXN_ENTRY(SO_REUSEPORT),
#endif
    NXN_ENTRY(SO_SNDBUF),
    NXN_ENTRY(SO_SNDLOWAT),
    NXN_ENTRY(SO_SNDTIMEO),
    NXN_ENTRY(SO_TYPE),
    NXN_ENTRY(SOCK_DGRAM),
    NXN_ENTRY(SOCK_RAW),
    NXN_ENTRY(SOCK_STREAM),
    NXN_ENTRY(SOL_SOCKET),
#endif
#ifdef CW_REGEX
    NXN_ENTRY(s),
#endif
    NXN_ENTRY(sadn),
    NXN_ENTRY(saup),
    NXN_ENTRY(sbdup),
    NXN_ENTRY(sbpop),
    NXN_ENTRY(sbpush),
    NXN_ENTRY(sclear),
    NXN_ENTRY(scleartomark),
    NXN_ENTRY(scount),
    NXN_ENTRY(scounttomark),
    NXN_ENTRY(sdn),
    NXN_ENTRY(sdup),
#ifdef CW_POSIX
    NXN_ENTRY(seek),
#endif
    NXN_ENTRY(self),
#ifdef CW_SOCKET
    NXN_ENTRY(send),
    NXN_ENTRY(serviceport),
#endif
    NXN_ENTRY(setactive),
#ifdef CW_POSIX
    NXN_ENTRY(setegid),
    NXN_ENTRY(setenv),
    NXN_ENTRY(seteuid),
    NXN_ENTRY(setgid),
#endif
#ifdef CW_THREADS
    NXN_ENTRY(setgstderr),
    NXN_ENTRY(setgstdin),
    NXN_ENTRY(setgstdout),
#endif
    NXN_ENTRY(setiobuf),
#ifdef CW_THREADS
    NXN_ENTRY(setlocking),
#endif
    NXN_ENTRY(setnonblocking),
#ifdef CW_PTHREADS
    NXN_ENTRY(setperiod),
#endif
#ifdef CW_SOCKET
    NXN_ENTRY(setsockopt),
#endif
    NXN_ENTRY(setstderr),
    NXN_ENTRY(setstdin),
    NXN_ENTRY(setstdout),
    NXN_ENTRY(setthreshold),
#ifdef CW_POSIX
    NXN_ENTRY(setuid),
#endif
    NXN_ENTRY(sexch),
    NXN_ENTRY(shift),
    NXN_ENTRY(sibdup),
    NXN_ENTRY(sibpop),
    NXN_ENTRY(sidup),
#ifdef CW_THREADS
    NXN_ENTRY(signal),
#endif
#ifdef CW_REAL
    NXN_ENTRY(sin),
#endif
    NXN_ENTRY(sipop),
#ifdef CW_POSIX
    NXN_ENTRY(size),
#endif
    NXN_ENTRY(snbpop),
    NXN_ENTRY(sndn),
    NXN_ENTRY(sndup),
    NXN_ENTRY(snip),
    NXN_ENTRY(snpop),
    NXN_ENTRY(snup),
#ifdef CW_SOCKET
    NXN_ENTRY(socket),
    NXN_ENTRY(socketpair),
    NXN_ENTRY(sockname),
    NXN_ENTRY(sockopt),
#endif
    NXN_ENTRY(sover),
    NXN_ENTRY(spop),
    NXN_ENTRY(spush),
#ifdef CW_REAL
    NXN_ENTRY(sqrt),
#endif
#ifdef CW_POSIX
    NXN_ENTRY(srand),
#endif
    NXN_ENTRY(sroll),
    NXN_ENTRY(srot),
    NXN_ENTRY(stack),
    NXN_ENTRY(stacktype),
    NXN_ENTRY(stackunderflow),
    NXN_ENTRY(start),
    NXN_ENTRY(stats),
#ifdef CW_POSIX
    NXN_ENTRY(status),
#endif
    NXN_ENTRY(stdin),
    NXN_ENTRY(stderr),
    NXN_ENTRY(stdout),
    NXN_ENTRY(stop),
    NXN_ENTRY(stopped),
    NXN_ENTRY(string),
    NXN_ENTRY(stringtype),
    NXN_ENTRY(stuck),
    NXN_ENTRY(sub),
#ifdef CW_REGEX
    NXN_ENTRY(subst),
#endif
    NXN_ENTRY(sunder),
    NXN_ENTRY(sup),
#ifdef CW_POSIX
    NXN_ENTRY(symlink),
#endif
    NXN_ENTRY(syntaxerror),
    NXN_ENTRY(system),
    NXN_ENTRY(systemdict),
    NXN_ENTRY(tell),
#ifdef CW_POSIX
    NXN_ENTRY(test),
#endif
#ifdef CW_THREADS
    NXN_ENTRY(thread),
#endif
    NXN_ENTRY(threaddstack),
    NXN_ENTRY(threadestack),
    NXN_ENTRY(threadistack),
    NXN_ENTRY(threadostack),
    NXN_ENTRY(threadsdict),
    NXN_ENTRY(threadtype),
    NXN_ENTRY(threshold),
    NXN_ENTRY(throw),
#ifdef CW_POSIX
    NXN_ENTRY(time),
#endif
#ifdef CW_THREADS
    NXN_ENTRY(timedwait),
#endif
    NXN_ENTRY(token),
    NXN_ENTRY(true),
#ifdef CW_REAL
    NXN_ENTRY(trunc),
#endif
#ifdef CW_POSIX
    NXN_ENTRY(truncate),
#endif
#ifdef CW_THREADS
    NXN_ENTRY(trylock),
#endif
    NXN_ENTRY(tuck),
    NXN_ENTRY(type),
    NXN_ENTRY(typecheck),
#ifdef CW_POSIX
    NXN_ENTRY(uid),
    NXN_ENTRY(umask),
#endif
    NXN_ENTRY(undef),
    NXN_ENTRY(undefined),
    NXN_ENTRY(undefinedfilename),
    NXN_ENTRY(undefinedresult),
    NXN_ENTRY(under),
#ifdef CW_POSIX
    NXN_ENTRY(unlink),
#endif
#ifdef CW_THREADS
    NXN_ENTRY(unlock),
#endif
    NXN_ENTRY(unmatchedfino),
    NXN_ENTRY(unmatchedmark),
    NXN_ENTRY(unregistered),
#ifdef CW_POSIX
    NXN_ENTRY(unsetenv),
#endif
    NXN_ENTRY(until),
    NXN_ENTRY(up),
    NXN_ENTRY(version),
#ifdef CW_THREADS
    NXN_ENTRY(wait),
#endif
#ifdef CW_POSIX
    NXN_ENTRY(waitpid),
#endif
    NXN_ENTRY(where),
    NXN_ENTRY(while),
    NXN_ENTRY(write),
    NXN_ENTRY(xcheck),
    NXN_ENTRY(xor)
#ifdef CW_THREADS
    ,
    NXN_ENTRY(yield)
#define NXN_LAST NXN_yield
#else
#define NXN_LAST NXN_xor
#endif
}
#ifndef CW_NXN_C_
cw_nxn_t
#endif
;

#ifndef CW_USE_INLINES
const cw_uint8_t *nxn_str(cw_nxn_t a_nxn);
cw_uint32_t nxn_len(cw_nxn_t a_nxn);
#endif

#if (defined(CW_USE_INLINES) && !defined(CW_NXN_C_) \
     || (!defined(CW_USE_INLINES) && defined(CW_NXN_C_)))
CW_INLINE const cw_uint8_t *
nxn_str(cw_nxn_t a_nxn)
{
    cw_assert(a_nxn > NXN_ZERO && a_nxn <= NXN_LAST);

    return cw_g_nx_names[a_nxn];
}

CW_INLINE cw_uint32_t
nxn_len(cw_nxn_t a_nxn)
{
    cw_assert(a_nxn > NXN_ZERO && a_nxn <= NXN_LAST);

    return strlen(cw_g_nx_names[a_nxn]);
}
#endif /* (defined(CW_USE_INLINES) && !defined(CW_NXN_C_) \
	*  || (!defined(CW_USE_INLINES) && defined(CW_NXN_C_))) */

#undef NXN_ENTRY
#undef NXN_SENTRY
#undef NXN_XENTRY
