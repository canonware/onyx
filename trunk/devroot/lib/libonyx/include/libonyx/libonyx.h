/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Master header file for libonyx.
 *
 ******************************************************************************/

#ifdef __cplusplus
extern  "C" {
#endif

#ifndef _LIBONYX_H_
#define _LIBONYX_H_

#define _CW_LIBONYX_VERSION "<Version>"

/*
 * Minimum initial size of dictionaries.
 */
#define	_CW_LIBONYX_DICT_SIZE		   16

/*
 * Maximum depth of estack.
 */
#define	_CW_LIBONYX_ESTACK_MAX		  256

/*
 * Default file buffer size.
 */
#define	_CW_LIBONYX_FILE_BUFFER_SIZE	  512

/*
 * Size of stack-allocated buffer to use when executing file objects.  This
 * generally doesn't need to be huge, because there is usually additional
 * buffering going on upstream.
 */
#define	_CW_LIBONYX_FILE_EVAL_READ_SIZE	  128

/*
 * Default minimum period of registration inactivity before a periodic
 * collection is done (if any registrations have occured since the last
 * collection).  On average, the actual inactivity period will be 1.5 times
 * this, but can range from 1 to 2 times this.
 */
#define	_CW_LIBONYX_GCDICT_PERIOD	   20

/*
 * Default number of sequence set additions since last collection that will
 * cause an immediate collection.
 */
#define	_CW_LIBONYX_GCDICT_THRESHOLD	20000

/*
 * Initial size of globaldict.  This is a bit arbitrary, and some applications
 * could benefit from making it larger or smaller.
 */
#define	_CW_LIBONYX_GLOBALDICT_HASH	   64

/*
 * Initial size of threadsdict.  Most applications don't use many threads, so
 * the initial size is set pretty low.
 */
#define	_CW_LIBONYX_THREADSDICT_HASH	   16

/*
 * Initial size initial name cache hash table.  We know for sure that there will
 * be about 250 names referenced by systemdict, errordict, and currenterror to
 * begin with.
 */
#define _CW_LIBONYX_NAME_HASH		 1024

/*
 * Number of objects to allocate in a chunk for internal stack use.
 */
#define	_CW_LIBONYX_STACKC_COUNT	   32

/*
 * Initial size of userdict.
 */
#define	_CW_LIBONYX_USERDICT_HASH	   64

/*
 * Exception numbers.  libonyx reserves 128 through 255.
 */
#define	_CW_ONYXX_MIN			  128
#define	_CW_ONYXX_MAX			  255

/* Internal use, for the exit operator. */
#define	_CW_ONYXX_EXIT			  128
/* Internal use, for the stop operator, caught by the stopped operator. */
#define	_CW_ONYXX_STOP			  129
/* Internal use, for the quit operator, caught by the start operator. */
#define	_CW_ONYXX_QUIT			  130

#include "libonyx_defs.h"
#include "libonyx_incs.h"

/* Convenience macro for static embedded onyx code. */
#define	_cw_onyx_code(a_thread, a_code) do {				\
	cw_nxo_threadp_t	threadp;				\
	static const cw_uint8_t	code[] = (a_code);			\
									\
	nxo_threadp_new(&threadp);					\
	nxo_thread_interpret((a_thread), &threadp, code,		\
	    sizeof(code) - 1);						\
	nxo_thread_flush((a_thread), &threadp);				\
	nxo_threadp_delete(&threadp, (a_thread));			\
} while (0)

#endif	/* _LIBONYX_H_ */

#ifdef __cplusplus
};
#endif
