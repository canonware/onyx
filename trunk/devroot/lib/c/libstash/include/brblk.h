/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 96 $
 * $Date: 1998-06-26 23:48:21 -0700 (Fri, 26 Jun 1998) $
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
  cw_jtl_t jt_lock;
  cw_bool_t is_valid;
  cw_bool_t is_dirty;
  cw_uint64_t logical_addr;
  cw_uint8_t * buf;
  cw_uint64_t buf_size;
};

/* Namespace definition. */
#define brblk_new _CW_NS_CMN(brblk_new)
#define brblk_delete _CW_NS_CMN(brblk_delete)
#define brblk_slock _CW_NS_CMN(brblk_slock)
#define brblk_tlock _CW_NS_CMN(brblk_tlock)
#define brblk_s2dlock _CW_NS_CMN(brblk_s2dlock)
#define brblk_2rlock _CW_NS_CMN(brblk_2rlock)
#define brblk_2wlock _CW_NS_CMN(brblk_2wlock)
#define brblk_2xlock _CW_NS_CMN(brblk_2xlock)
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

/* Function prototypes. */
cw_brblk_t * brblk_new(cw_brblk_t * a_brblk_o, cw_uint32_t a_block_size);
void brblk_delete(cw_brblk_t * a_brblk_o);

void brblk_slock(cw_brblk_t * a_brblk_o);
void brblk_tlock(cw_brblk_t * a_brblk_o);

void brblk_s2dlock(cw_brblk_t * a_brblk_o);
void brblk_2rlock(cw_brblk_t * a_brblk_o);
void brblk_2wlock(cw_brblk_t * a_brblk_o);
void brblk_2xlock(cw_brblk_t * a_brblk_o);

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

cw_uint8_t * brblk_get_buf_p(cw_brblk_t * a_brblk_o);

cw_bool_t brblk_get_is_dirty(cw_brblk_t * a_brblk_o);
void brblk_set_is_dirty(cw_brblk_t * a_brblk_o, cw_bool_t a_is_dirty);

#endif /* _BRBLK_H_ */
