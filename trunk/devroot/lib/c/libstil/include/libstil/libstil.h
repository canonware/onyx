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

/* Exception numbers.  libstil reserves -128 to -255. */
#define	_CW_STILX_MAX			-128
#define	_CW_STILX_MIN			-255

/* errordict not found, or handler not found in errordict. */
#define	_CW_STILX_ERRORDICT		-128
/* Error accessing $error. */
#define	_CW_STILX_DERROR		-129

/* Internal use, for the exit operator. */
#define	_CW_STILX_EXIT			-129
/* Internal use, for the stop operator. */
#define	_CW_STILX_STOP			-130
/* Internal use, for the quit operator, caught by the start operator. */
#define	_CW_STILX_QUIT			-131

#include "libstil_defs.h"
#include "libstil_incs.h"

#endif	/* _LIBSTIL_H_ */

#ifdef __cplusplus
};
#endif
