/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Master header file for libstil.
 *
 ******************************************************************************/

#ifdef __cplusplus
extern  "C" {
#endif

#ifndef _LIBSTIL_H_
#define _LIBSTIL_H_

#define _LIBSTIL_VERSION "<Version>"

/* Causes verbose GC-related output. */
/*  #define	_LIBSTIL_CONFESS */

/*
 * Minimum initial size of dictionaries.
 */
#define	_LIBSTIL_DICT_SIZE		  16

/*
 * Maximum depth of estack.
 */
#define	_LIBSTIL_ESTACK_MAX		 256

/*
 * Default file buffer size.
 */
#define	_LIBSTIL_FILE_BUFFER_SIZE	 512

/*
 * Size of stack-allocated buffer to use when executing file objects.  This
 * generally doesn't need to be huge, because there is usually additional
 * buffering going on upstream.
 */
#define	_LIBSTIL_FILE_EVAL_READ_SIZE	128

/*
 * Default minimum period of registration inactivity before a periodic
 * collection is done (if any registrations have occured since the last
 * collection).  On average, the actual inactivity period will be 1.5 times
 * this, but can range from 1 to 2 times this.
 */
#define	_LIBSTIL_GCDICT_PERIOD		  20

/*
 * Default number of sequence set additions since last collection that will
 * cause an immediate collection.
 */
#define	_LIBSTIL_GCDICT_THRESHOLD      50000

/*
 * Initial size of globaldict.  This is a bit arbitrary, and some applications
 * could benefit from making it larger or smaller.
 */
#define	_LIBSTIL_GLOBALDICT_HASH	  64

/*
 * Initial size initial name cache hash table.  We know for sure that there will
 * be about 175 names referenced by systemdict, threaddict, errordict, and
 * currenterror to begin with.
 */
#define _LIBSTIL_NAME_HASH		1024

/*
 * Number of objects to allocate in a chunk for internal stils use.
 */
#define	_LIBSTIL_STILSC_COUNT		  32

/*
 * Initial size of userdict.
 */
#define	_LIBSTASH_USERDICT_HASH	 64

/*
 * Exception numbers.  libstil reserves -128 to -255.
 */
#define	_CW_STILX_MAX			-128
#define	_CW_STILX_MIN			-255

/* errordict not found, or handler not found in errordict. */
#define	_CW_STILX_ERRORDICT		-128
/* Error accessing currenterror. */
#define	_CW_STILX_CURRENTERROR		-129

/* Internal use, for the exit operator. */
#define	_CW_STILX_EXIT			-129
/* Internal use, for the stop operator. */
#define	_CW_STILX_STOP			-130
/* Internal use, for the quit operator, caught by the start operator. */
#define	_CW_STILX_QUIT			-131

#include "libstil_defs.h"
#include "libstil_incs.h"

/* Convenience macro for static embedded stil code. */
#define	_cw_stil_code(a_stilt, a_code) do {				\
	cw_stilts_t		stilts;					\
	static const cw_uint8_t	code[] = (a_code);			\
									\
	stilts_new(&stilts);						\
	stilt_interpret((a_stilt), &stilts, code, sizeof(code) - 1);	\
	stilt_flush((a_stilt), &stilts);				\
	stilts_delete(&stilts, (a_stilt));				\
} while (0)

#endif	/* _LIBSTIL_H_ */

#ifdef __cplusplus
};
#endif
