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
 * $Revision: 190 $
 * $Date: 1998-09-01 18:14:15 -0700 (Tue, 01 Sep 1998) $
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
  cw_rwl_t rw_lock;
  cw_bool_t is_open;
  cw_oh_t vaddr_hash;
  cw_res_t res;
  char * res_file_a;
  char * res_file_b;
  char * res_file_c;
  
};

/* Namespace definitions for br. */
#define br_new _CW_NS_ANY(br_new)
#define br_delete _CW_NS_ANY(br_delete)
#define br_create _CW_NS_ANY(br_create)
#define br_get_res_files _CW_NS_ANY(br_get_res_files)
#define br_set_res_files _CW_NS_ANY(br_set_res_files)
#define br_fsck _CW_NS_ANY(br_fsck)
#define br_dump _CW_NS_ANY(br_dump)
#define br_is_open _CW_NS_ANY(br_is_open)
#define br_open _CW_NS_ANY(br_open)
#define br_close _CW_NS_ANY(br_close)
#define br_add_brbs _CW_NS_ANY(br_add_brbs)
#define br_get_brbs_p _CW_NS_ANY(br_get_brbs_p)
#define br_rm_brbs _CW_NS_ANY(br_rm_brbs)
#define br_block_create _CW_NS_ANY(br_block_create)
#define br_block_destroy _CW_NS_ANY(br_block_destroy)
#define br_block_slock _CW_NS_ANY(br_block_slock)
#define br_block_tlock _CW_NS_ANY(br_block_tlock)
#define br_block_flush _CW_NS_ANY(br_block_flush)

/* Function prototypes. */
cw_br_t * br_new(cw_br_t * a_br_o);
void br_delete(cw_br_t * a_br_o);

cw_bool_t br_create(cw_br_t * a_br_o, cw_uint32_t a_block_size);

void br_get_res_files(cw_br_t * a_br_o, char ** a_res_file_a,
		      char ** a_res_file_b, char ** a_res_file_c);
cw_bool_t br_set_res_files(cw_br_t * a_br_o, char * a_res_file_a,
			   char * a_res_file_b, char * a_res_file_c);

cw_bool_t br_fsck(cw_br_t * a_br_o);

void br_dump(cw_br_t * a_br_o);
cw_bool_t br_is_open(cw_br_t * a_br_o);

cw_bool_t br_open(cw_br_t * a_br_o);
cw_bool_t br_close(cw_br_t * a_br_o);

cw_bool_t br_add_brbs(cw_br_t * a_br_o, cw_brbs_t * a_brbs_t,
		      cw_uint64_t a_base_addr);
cw_bool_t br_get_brbs_p(cw_br_t * a_br_o, char * a_filename,
			cw_brbs_t ** a_brbs_o);
cw_bool_t br_rm_brbs(cw_br_t * a_br_o, char * a_filename);

cw_bool_t br_block_create(cw_br_t * a_br_o, cw_brblk_t ** a_brblk_o);
cw_bool_t br_block_destroy(cw_br_t * a_br_o, cw_brblk_t * a_brblk_o);

cw_bool_t br_block_slock(cw_br_t * a_br_o,
			 cw_uint64_t a_logical_addr,
			 cw_brblk_t ** a_brblk_o);
cw_bool_t br_block_tlock(cw_br_t * a_br_o,
			 cw_uint64_t a_logical_addr,
			 cw_brblk_t ** a_brblk_o);

/* The caller must already hold an s or t lock before calling this. */
cw_bool_t br_block_flush(cw_br_t * a_br_o, cw_brblk_t * a_brblk_o);

#endif /* _BR_H_ */
