/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 91 $
 * $Date: 1998-06-24 23:46:00 -0700 (Wed, 24 Jun 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#ifndef _BR_H_
#define _BR_H_

/* Pseudo-opaque type. */
typedef struct cw_br_s cw_br_t;

struct cw_br_s
{
  cw_bool_t is_malloced;
  cw_bool_t is_thread_safe;
  cw_rwl_t rw_lock;
  cw_bool_t is_open;
};

/* Namespace definitions for br. */
#define br_new _CW_NS_CMN(br_new)
#define br_delete _CW_NS_CMN(br_delete)
#define br_is_open _CW_NS_CMN(br_is_open)
#define br_open _CW_NS_CMN(br_open)
#define br_close _CW_NS_CMN(br_close)
#define br_get_block_size _CW_NS_CMN(br_get_block_size)
#define br_add_file _CW_NS_CMN(br_add_file)
#define br_rm_file _CW_NS_CMN(br_rm_file)
#define br_block_create _CW_NS_CMN(br_block_create)
#define br_block_destroy _CW_NS_CMN(br_block_destroy)
#define br_block_slock _CW_NS_CMN(br_block_slock)
#define br_block_tlock _CW_NS_CMN(br_block_tlock)

/* Function prototypes. */
cw_br_t * br_new(cw_br_t * a_br_o, cw_bool_t a_is_thread_safe);
void br_delete(cw_br_t * a_br_o);

cw_bool_t br_is_open(cw_br_t * a_br_o);

cw_bool_t br_open(cw_br_t * a_br_o, char * a_filename);
cw_bool_t br_close(cw_br_t * a_br_o);

cw_uint64_t br_get_block_size(cw_br_t * a_br_o);

cw_bool_t br_add_file(cw_br_t * a_br_o, char * a_filename,
		      cw_bool_t a_is_raw, cw_bool_t a_can_overlap,
		      cw_bool_t a_is_dynamic,
		      cw_uint64_t a_base_addr, cw_uint64_t a_max_size);
cw_bool_t br_rm_file(cw_br_t * a_br_o, char * a_filename);

/* XXX Should br_block_create() imply some sort of lock is held?  Since the 
 * block didn't exist in a valid state until this function returned, no one 
 * else is in the block, but it is conceivable for someone else to ask (and 
 * get) a lock on the block, even though they don't have an explicit way of 
 * knowing that it exists.  Hmm... */
cw_bool_t br_block_create(cw_br_t * a_br_o, cw_brblk_t ** a_brblk_o);
cw_bool_t br_block_destroy(cw_br_t * a_br_o, cw_brblk_t * a_brblk_o);

cw_bool_t br_block_slock(cw_br_t * a_br_o,
			 cw_uint64_t a_logical_addr,
			 cw_brblk_t ** a_brblk_o);
cw_bool_t br_block_tlock(cw_br_t * a_br_o,
			 cw_uint64_t a_logical_addr,
			 cw_brblk_t ** a_brblk_o);
#endif /* _BR_H_ */
