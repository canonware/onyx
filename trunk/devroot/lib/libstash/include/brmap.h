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
 * $Revision: 196 $
 * $Date: 1998-09-05 18:26:34 -0700 (Sat, 05 Sep 1998) $
 *
 * <<< Description >>>
 *
 * brmap is a helper class for br.  brmap's entire job is to keep track of
 * brbs instances and what memory ranges they are mapped to.  Since mapping
 * lookups are very common, and modifying the mappings are relatively
 * uncommon, brmap is optimized to speed up lookups, at the expense of
 * making mapping modifications more expensive.
 *
 ****************************************************************************/

#ifndef _BRMAP_H_
#define _BRMAP_H_

/* Controls the size of the array in each cw_brmap_list_s element.
 * Depending on how much mapping redundancy there is, this number may need
 * increased for performance reasons, though the code will never fail to
 * work correctly, regardless of the redundancy. */
#define _STASH_BRMAP_INTERNAL_LIST_SIZE 3

/* Pseudo-opaque type. */
typedef struct cw_brmap_s cw_brmap_t;

struct cw_brmap_el_s
{
  cw_brbs_t brbs_o;
  cw_oh_t brblk_hash;
};

struct cw_brmap_list_s
{
  cw_uint64_t start_addr;
  cw_uint64_t end_addr;
  cw_uint32_t num_backings; /* Number of backings for this address range. */
  cw_brbs_t brbs_list[_STASH_BRMAP_INTERNAL_LIST_SIZE]; /* Array of first
							 * backings. */
  cw_brbs_t * external_brbs_list; /* Overflow array of backings. */
}

struct cw_brmap_s
{
  cw_oh_t vaddr_hash; /* Hash table of cached blocks. */
  cw_uint32_t num_backings; /* Number of brbs instances currently mapped. */
  cw_brmap_el_s * backings; /* Array of mapped brbs instances. */
  cw_brmap_list_s * map; /* List used for finding the brbs(es) that
			  * correspond(s) to any given address. */
  cw_uint64_t num_cache_brblks; /* Number of brblk's to keep in memory
				 * cache. */
  cw_list_t spare_brblk_list; /* Spare brblk instances. */
};

#define brmap_new _CW_NS_ANY(brmap_new)
#define brmap_delete _CW_NS_ANY(brmap_delete)
#define brmap_add_mapping _CW_NS_ANY(brmap_add_mapping)
#define brmap_rm_mapping _CW_NS_ANY(brmap_rm_mapping)
#define brmap_get_cache_size _CW_NS_ANY(brmap_get_cache_size)
#define brmap_set_cache_size _CW_NS_ANY(brmap_set_cache_size)
#define brmap_get_brblk _CW_NS_ANY(brmap_get_brblk)

cw_brmap_t * brmap_new(cw_brmap_t * a_brmap_o);
void brmap_delete(cw_brmap_t a_brmap_o);

cw_bool_t brmap_add_mapping(cw_brmap_t * a_brmap_o, cw_brbs_t * a_brbs_o,
			    cw_uint64_t a_base_addr);
cw_bool_t brmap_rm_mapping(cw_brmap_t * a_brmap_o, char * a_filename);

cw_uint64_t brmap_get_cache_size(cw_brmap_t * a_brmap_o);
cw_bool_t brmap_set_cache_size(cw_brmap_t * a_brmap_o, cw_uint64_t a_new_size);

cw_bool_t brmap_get_brblk(cw_brmap_t * a_brmap_o, cw_uint64_t a_addr,
			  cw_brblk_t ** a_brblk_o);

#endif /* _BRMAP_H_ */
