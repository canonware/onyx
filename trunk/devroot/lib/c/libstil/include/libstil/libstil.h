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
#define	_CW_STILX_DICTFULL		-128
#define	_CW_STILX_DICTSTACKOVERFLOW	-129
#define	_CW_STILX_DICTSTACKUNDERFLOW	-130
#define	_CW_STILX_EXECSTACKOVERFLOW	-131
#define	_CW_STILX_HANDLEERROR		-132
#define	_CW_STILX_INTERRUPT		-133
#define	_CW_STILX_INVALIDACCESS		-134
#define	_CW_STILX_INVALIDCONTEXT	-135
#define	_CW_STILX_INVALIDEXIT		-136
#define	_CW_STILX_INVALIDFILEACCESS	-137
#define	_CW_STILX_INVALID		-138
#define	_CW_STILX_IOERROR		-139
#define	_CW_STILX_LIMITCHECK		-140
#define	_CW_STILX_RANGECHECK		-141
#define	_CW_STILX_STACKOVERFLOW		-142
#define	_CW_STILX_STACKUNDERFLOW	-143
#define	_CW_STILX_SYNTAXERROR		-144
#define	_CW_STILX_TIMEOUT		-145
#define	_CW_STILX_TYPECHECK		-146
#define	_CW_STILX_UNDEFINED		-147
#define	_CW_STILX_UNDEFINEDFILENAME	-148
#define	_CW_STILX_UNDEFINEDRESOURCE	-149
#define	_CW_STILX_UNDEFINEDRESULT	-150
#define	_CW_STILX_UNMATCHEDMARK		-151
#define	_CW_STILX_UNREGISTERED		-152
#define	_CW_STILX_VMERROR		-153

/* For the exit operator. */
#define	_CW_STILX_EXIT			-154
/* For the stop operator. */
#define	_CW_STILX_STOP			-155
/* For the quit operator, caught by the start operator. */
#define	_CW_STILX_QUIT			-156

#include "libstil_defs.h"
#include "libstil_incs.h"

#endif	/* _LIBSTIL_H_ */

#ifdef __cplusplus
};

#endif
