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
 * $Revision: 173 $
 * $Date: 1998-08-26 12:34:42 -0700 (Wed, 26 Aug 1998) $
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
  cw_jtl_t jt_lock;
  cw_bool_t is_dirty;
  cw_uint64_t logical_addr;
  cw_uint8_t * buf;
  cw_uint64_t buf_size;
};

/* Namespace definition. */
#define brblk_new _CW_NS_ANY(brblk_new)
#define brblk_delete _CW_NS_ANY(brblk_delete)
#ifdef _STASH_DBG
#  define brblk_slock _CW_NS_ANY(brblk_slock)
#  define brblk_tlock _CW_NS_ANY(brblk_tlock)
#  define brblk_sunlock _CW_NS_ANY(brblk_sunlock)
#  define brblk_tunlock _CW_NS_ANY(brblk_tunlock)
#  define brblk_get_is_dirty _CW_NS_ANY(brblk_get_is_dirty)
#  define brblk_set_is_dirty _CW_NS_ANY(brblk_set_is_dirty)
#  define brblk_s2dlock _CW_NS_ANY(brblk_s2dlock)
#  define brblk_2rlock _CW_NS_ANY(brblk_2rlock)
#  define brblk_2wlock _CW_NS_ANY(brblk_2wlock)
#  define brblk_2xlock _CW_NS_ANY(brblk_2xlock)
#  define brblk_dunlock _CW_NS_ANY(brblk_dunlock)
#  define brblk_runlock _CW_NS_ANY(brblk_runlock)
#  define brblk_wunlock _CW_NS_ANY(brblk_wunlock)
#  define brblk_xunlock _CW_NS_ANY(brblk_xunlock)
#  define brblk_get_byte _CW_NS_ANY(brblk_get_byte)
#  define brblk_set_byte _CW_NS_ANY(brblk_set_byte)
#  define brblk_get_buf_p _CW_NS_ANY(brblk_get_buf_p)
#  define brblk_get_buf_size _CW_NS_ANY(brblk_get_buf_size)
#endif
/* #define brblk_ _CW_NS_ANY(brblk_) */

/* The following several functions should only be used by the br code. */
cw_brblk_t * brblk_new(cw_brblk_t * a_brblk_o, cw_uint32_t a_block_size);
void brblk_delete(cw_brblk_t * a_brblk_o);

#ifdef _STASH_DBG
void brblk_slock(cw_brblk_t * a_brblk_o);
void brblk_tlock(cw_brblk_t * a_brblk_o);
void brblk_sunlock(cw_brblk_t * a_brblk_o);
void brblk_tunlock(cw_brblk_t * a_brblk_o);

cw_bool_t brblk_get_is_dirty(cw_brblk_t * a_brblk_o);
void brblk_set_is_dirty(cw_brblk_t * a_brblk_o, cw_bool_t a_is_dirty);
#else
#  define brblk_slock(a) jtl_slock(&(a)->jt_lock)
#  define brblk_tlock(a) jtl_tlock(&(a)->jt_lock)
#  define brblk_sunlock(a) jtl_sunlock(&(a)->jt_lock)
#  define brblk_tunlock(a) jtl_tunlock(&(a)->jt_lock)

#  define brblk_get_is_dirty(a) (a)->is_dirty
#  define brblk_set_is_dirty(a, b) (a)->is_dirty = (b)
#endif

/* The following functions are safe for use outside of the br code.  The
 * above ones should only be used by br though. */
#ifdef _STASH_DBG
void brblk_s2dlock(cw_brblk_t * a_brblk_o);
void brblk_2rlock(cw_brblk_t * a_brblk_o);
void brblk_2wlock(cw_brblk_t * a_brblk_o);
void brblk_2xlock(cw_brblk_t * a_brblk_o);

void brblk_dunlock(cw_brblk_t * a_brblk_o);
void brblk_runlock(cw_brblk_t * a_brblk_o);
void brblk_wunlock(cw_brblk_t * a_brblk_o);
void brblk_xunlock(cw_brblk_t * a_brblk_o);
#else
#  define brblk_s2dlock(a) jtl_s2dlock(&(a)->jt_lock)
#  define brblk_2rlock(a) jtl_2rlock(&(a)->jt_lock)
#  define brblk_2wlock(a) jtl_2wlock(&(a)->jt_lock), (a)->is_dirty = TRUE
#  define brblk_2xlock(a) jtl_2xlock(&(a)->jt_lock), (a)->is_dirty = TRUE

#  define brblk_dunlock(a) jtl_lock(&(a)->jt_lock)
#  define brblk_runlock(a) jtl_lock(&(a)->jt_lock)
#  define brblk_wunlock(a) jtl_lock(&(a)->jt_lock)
#  define brblk_xunlock(a) jtl_lock(&(a)->jt_lock)
#endif

/* You're on scouts' honor if you use the below functions.  That is, you'd
 * better have already locked the block properly, or Bad Things (TM) will
 * happen. */
#ifdef _STASH_DBG
cw_uint8_t brblk_get_byte(cw_brblk_t * a_brblk_o, cw_uint32_t a_offset);
void brblk_set_byte(cw_brblk_t * a_brblk_o, cw_uint32_t a_offset,
		    cw_uint8_t a_byte);
cw_uint8_t * brblk_get_buf_p(cw_brblk_t * a_brblk_o);
cw_uint32_t brblk_get_buf_size(cw_brblk_t * a_brblk_o);
#else
#  define brblk_get_byte(a, b) (a)->buf[(b)]
#  define brblk_set_byte(a, b, c) (a)->buf[(b)] = (c)
#  define brblk_get_buf_p(a) (a)->buf
#  define brblk_get_buf_size(a) (a)->buf_size
#endif

#endif /* _BRBLK_H_ */
