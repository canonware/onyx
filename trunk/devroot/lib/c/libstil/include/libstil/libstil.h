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
#define	_CW_XEPV_DICTFULL		-128
#define	_CW_XEPV_DICTSTACKOVERFLOW	-129
#define	_CW_XEPV_DICTSTACKUNDERFLOW	-130
#define	_CW_XEPV_EXECSTACKOVERFLOW	-131
#define	_CW_XEPV_HANDLEERROR		-132
#define	_CW_XEPV_INTERRUPT		-133
#define	_CW_XEPV_INVALIDACCESS		-134
#define	_CW_XEPV_INVALIDCONTEXT		-135
#define	_CW_XEPV_INVALIDEXIT		-136
#define	_CW_XEPV_INVALIDFILEACCESS	-137
#define	_CW_XEPV_INVALID		-138
#define	_CW_XEPV_IOERROR		-139
#define	_CW_XEPV_LIMITCHECK		-140
#define	_CW_XEPV_RANGECHECK		-141
#define	_CW_XEPV_STACKOVERFLOW		-142
#define	_CW_XEPV_STACKUNDERFLOW		-143
#define	_CW_XEPV_SYNTAXERROR		-144
#define	_CW_XEPV_TIMEOUT		-145
#define	_CW_XEPV_TYPECHECK		-146
#define	_CW_XEPV_UNDEFINED		-147
#define	_CW_XEPV_UNDEFINEDFILENAME	-148
#define	_CW_XEPV_UNDEFINEDRESOURCE	-149
#define	_CW_XEPV_UNDEFINEDRESULT	-150
#define	_CW_XEPV_UNMATCHEDMARK		-151
#define	_CW_XEPV_UNREGISTERED		-152
#define	_CW_XEPV_VMERROR		-153

#include "libstil_defs.h"
#include "libstil_incs.h"

#endif	/* _LIBSTIL_H_ */

#ifdef __cplusplus
};

#endif
