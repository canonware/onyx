/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 86 $
 * $Date: 1998-06-23 17:40:29 -0700 (Tue, 23 Jun 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#ifndef _BRBLK_H_
#define _BRBLK_H_

/* Pseudo-opaque type. */
typedef struct cw_brblk_s cw_brblk_t;

struct cw_brblk_s
{
  cw_rwl_t rw_lock;
  cw_btl_t bt_lock;
  cw_bool_t is_valid;
  cw_bool_t is_dirty;
  cw_uint64_t logical_addr;
  cw_uint8_t * block;
};

/* Namespace definition. */
#define brblk_new _CW_NS_CMN(brblk_new)
#define brblk_delete _CW_NS_CMN(brblk_delete)
#define brblk_slock _CW_NS_CMN(brblk_slock)
#define brblk_tlock _CW_NS_CMN(brblk_tlock)
#define brblk_s2dlock _CW_NS_CMN(brblk_s2dlock)
#define brblk_s2rlock _CW_NS_CMN(brblk_s2rlock)
#define brblk_s2wlock _CW_NS_CMN(brblk_s2wlock)
#define brblk_s2xlock _CW_NS_CMN(brblk_s2xlock)
#define brblk_t2rlock _CW_NS_CMN(brblk_t2rlock)
#define brblk_t2wlock _CW_NS_CMN(brblk_t2wlock)
#define brblk_t2xlock _CW_NS_CMN(brblk_t2xlock)
#define brblk_sunlock _CW_NS_CMN(brblk_sunlock)
#define brblk_tunlock _CW_NS_CMN(brblk_tunlock)
#define brblk_dunlock _CW_NS_CMN(brblk_dunlock)
#define brblk_runlock _CW_NS_CMN(brblk_runlock)
#define brblk_wunlock _CW_NS_CMN(brblk_wunlock)
#define brblk_xunlock _CW_NS_CMN(brblk_xunlock)
#define brblk_get_byte _CW_NS_CMN(brblk_get_byte)
#define brblk_set_byte _CW_NS_CMN(brblk_set_byte)
#define brblk_is_dirty _CW_NS_CMN(brblk_is_dirty)
#define brblk_flush _CW_NS_CMN(brblk_flush)
/* #define brblk_ _CW_NS_CMN(brblk_) */
/* #define brblk_ _CW_NS_CMN(brblk_) */

/* Function prototypes. */
cw_brblk_t * brblk_new(cw_brblk_t * a_brblk_o);
void brblk_delete(cw_brblk_t * a_brblk_o);

void brblk_slock(cw_brblk_t * a_brblk_o);
void brblk_tlock(cw_brblk_t * a_brblk_o);

void brblk_s2dlock(cw_brblk_t * a_brblk_o);
void brblk_s2rlock(cw_brblk_t * a_brblk_o);
void brblk_s2wlock(cw_brblk_t * a_brblk_o);
void brblk_s2xlock(cw_brblk_t * a_brblk_o);
void brblk_t2rlock(cw_brblk_t * a_brblk_o);
void brblk_t2wlock(cw_brblk_t * a_brblk_o);
void brblk_t2xlock(cw_brblk_t * a_brblk_o);

void brblk_sunlock(cw_brblk_t * a_brblk_o);
void brblk_tunlock(cw_brblk_t * a_brblk_o);
void brblk_dunlock(cw_brblk_t * a_brblk_o);
void brblk_runlock(cw_brblk_t * a_brblk_o);
void brblk_wunlock(cw_brblk_t * a_brblk_o);
void brblk_xunlock(cw_brblk_t * a_brblk_o);

cw_bool_t brblk_get_byte(cw_brblk_t * a_brblk_o, cw_uint32_t a_offset,
			 cw_uint8_t * a_byte);
cw_bool_t brblk_set_byte(cw_brblk_t * a_brblk_o, cw_uint32_t a_offset,
			 cw_uint8_t a_byte);

cw_bool_t brblk_is_dirty(cw_brblk_t * a_brblk_o);
cw_bool_t brblk_flush(cw_brblk_t * a_brblk_o);

#endif /* _BRBLK_H_ */
