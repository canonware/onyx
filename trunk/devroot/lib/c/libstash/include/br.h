/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * <<< Description >>>
 *
 * Prototypes for the br class.
 *
 ****************************************************************************/

#ifndef _BR_H_
#define _BR_H_

#define _STASH_BR_INTERNAL_LIST_SIZE 3

/* Pseudo-opaque type. */
typedef struct cw_br_s cw_br_t;

/* Each cw_br_list_s element contains all data that is necessary to manage
 * a chunk of physically backed paddr space. */
struct cw_br_list_s
{
  cw_uint64_t start_addr;
  cw_uint64_t end_addr;
  cw_oh_t paddr_hash; /* Hash table of brblk instances that map to this
		       * physical address range. */
  cw_uint32_t num_backings; /* Number of backings for this address range. */
  cw_brbs_t * brbs_list[_STASH_BR_INTERNAL_LIST_SIZE]; /* Array of first
							* backings. */
  cw_brbs_t ** brbs_more; /* Pointer to an additional list of brbs
			   * pointers.  This is only used if brbs_list is
			   * full. */
};

struct cw_br_s
{
  cw_bool_t is_malloced;
  cw_rwl_t rw_lock;
  cw_bool_t is_open;

  cw_res_t res;
  char * res_files[3];

  cw_oh_t vaddr_hash; /* Hash table of all cached brblks. */
  cw_uint32_t num_backings; /* Total number of brbs instances currently
			     * mapped. */
  struct cw_brmap_list_s * map; /* List used for finding the brbs(es) that
				 * correspond(s) to any given address. */
  cw_uint64_t num_cache_brblks; /* Number of brblk's to keep in memory
				 * cache. */
  cw_list_t spare_brblk_list; /* Spare brblk instances. */
};

/* Namespace definitions for br. */
#define br_new _CW_NS_ANY(br_new)
#define br_delete _CW_NS_ANY(br_delete)
#define br_create _CW_NS_ANY(br_create)
#define br_get_res_files _CW_NS_ANY(br_get_res_files)
#define br_set_res_files _CW_NS_ANY(br_set_res_files)
#define br_fsck _CW_NS_ANY(br_fsck)
#define br_dump _CW_NS_ANY(br_dump)
#define br_get_num_cache_brblks _CW_NS_ANY(br_get_num_cache_brblks)
#define br_set_num_cache_brblks _CW_NS_ANY(br_set_num_cache_brblks)
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

void br_get_res_files(cw_br_t * a_br_o, char * a_res_files[]);
cw_bool_t br_set_res_files(cw_br_t * a_br_o, char * a_res_files[]);

cw_bool_t br_fsck(cw_br_t * a_br_o);
void br_dump(cw_br_t * a_br_o);

cw_uint64_t br_get_num_cache_brblks(cw_br_t * a_br_o);
cw_bool_t br_set_num_cache_brblks(cw_br_t * a_br_o,
				  cw_uint64_t a_num_brblks);

cw_bool_t br_is_open(cw_br_t * a_br_o);
cw_bool_t br_open(cw_br_t * a_br_o);
cw_bool_t br_close(cw_br_t * a_br_o);

cw_bool_t br_add_brbs(cw_br_t * a_br_o, cw_brbs_t * a_brbs_t,
		      cw_uint64_t a_base_addr);
cw_bool_t br_get_brbs_p(cw_br_t * a_br_o, char * a_filename,
			cw_brbs_t ** a_brbs_o);
cw_bool_t br_rm_brbs(cw_br_t * a_br_o, cw_brbs_t * a_brbs_o);

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
