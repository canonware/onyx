/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 233 $
 * $Date: 1998-09-23 16:22:28 -0700 (Wed, 23 Sep 1998) $
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
  cw_bool_t is_malloced;
  cw_mtx_t lock;
  cw_jtl_t jt_lock;
  cw_bool_t is_dirty;
  cw_bool_t atomic_writes;
  cw_uint64_t vaddr;
  cw_uint64_t paddr;
  cw_uint8_t * buf;
  cw_uint64_t buf_size;
};

/* Namespace definition. */
#define brblk_new _CW_NS_ANY(brblk_new)
#define brblk_delete _CW_NS_ANY(brblk_delete)
#define brblk_slock _CW_NS_ANY(brblk_slock)
#define brblk_tlock _CW_NS_ANY(brblk_tlock)
#define brblk_get_is_dirty _CW_NS_ANY(brblk_get_is_dirty)
#define brblk_set_is_dirty _CW_NS_ANY(brblk_set_is_dirty)
#define brblk_get_atomic_writes _CW_NS_ANY(brblk_get_atomic_writes)
#define brblk_set_atomic_writes _CW_NS_ANY(brblk_set_atomic_writes)
#define brblk_s2dlock _CW_NS_ANY(brblk_s2dlock)
#define brblk_2rlock _CW_NS_ANY(brblk_2rlock)
#define brblk_2wlock _CW_NS_ANY(brblk_2wlock)
#define brblk_2xlock _CW_NS_ANY(brblk_2xlock)
#define brblk_sunlock _CW_NS_ANY(brblk_sunlock)
#define brblk_tunlock _CW_NS_ANY(brblk_tunlock)
#define brblk_dunlock _CW_NS_ANY(brblk_dunlock)
#define brblk_runlock _CW_NS_ANY(brblk_runlock)
#define brblk_wunlock _CW_NS_ANY(brblk_wunlock)
#define brblk_xunlock _CW_NS_ANY(brblk_xunlock)
#define brblk_get_byte _CW_NS_ANY(brblk_get_byte)
#define brblk_set_byte _CW_NS_ANY(brblk_set_byte)
#define brblk_get_buf_p _CW_NS_ANY(brblk_get_buf_p)
#define brblk_get_buf_size _CW_NS_ANY(brblk_get_buf_size)
#define brblk_get_dlocks _CW_NS_ANY(brblk_get_dlocks)
#define brblk_set_dlocks _CW_NS_ANY(brblk_set_dlocks)

/* The following several functions should only be used by the br code. */
cw_brblk_t * brblk_new(cw_brblk_t * a_brblk_o, cw_uint32_t a_block_size);
void brblk_delete(cw_brblk_t * a_brblk_o);

void brblk_slock(cw_brblk_t * a_brblk_o);
void brblk_tlock(cw_brblk_t * a_brblk_o);

/* The following functions are safe for use outside of the br code. */
cw_bool_t brblk_get_is_dirty(cw_brblk_t * a_brblk_o);
void brblk_set_is_dirty(cw_brblk_t * a_brblk_o, cw_bool_t a_is_dirty);

cw_bool_t brblk_get_atomic_writes(cw_brblk_t * a_brblk_o);
void brblk_set_atomic_writes(cw_brblk_t * a_brblk_o, cw_bool_t a_atomic_writes);

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

/* You're on scouts' honor if you use the below functions.  That is, you'd
 * better have already locked the block properly, or Bad Things (TM) will
 * happen. */
cw_uint32_t brblk_get_dlocks(cw_brblk_t * a_brblk_o);
cw_uint32_t brblk_set_dlocks(cw_brblk_t * a_brblk_o, cw_uint32_t a_dlocks);

cw_uint8_t brblk_get_byte(cw_brblk_t * a_brblk_o, cw_uint32_t a_offset);
void brblk_set_byte(cw_brblk_t * a_brblk_o, cw_uint32_t a_offset,
		    cw_uint8_t a_byte);
cw_uint8_t * brblk_get_buf_p(cw_brblk_t * a_brblk_o);
cw_uint32_t brblk_get_buf_size(cw_brblk_t * a_brblk_o);

#endif /* _BRBLK_H_ */
