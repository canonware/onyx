/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 ****************************************************************************/

#include "../include/libstash/libstash.h"

cw_ch_t *
ch_new(cw_ch_t * a_ch, cw_uint32_t a_table_size, cw_pezz_t * a_chi_pezz,
       ch_hash_t * a_hash, ch_key_comp_t * a_key_comp)
{
  cw_ch_t * retval;

  return retval;
}

void
ch_delete(cw_ch_t * a_ch)
{
}

cw_uint32_t
ch_count(cw_ch_t * a_ch)
{
}

cw_sint32_t
ch_insert(cw_ch_t * a_ch, const void * a_key, const void * a_data)
{
}

cw_bool_t
ch_remove(cw_ch_t * a_ch, const void * a_search_key, void ** r_key,
	  void ** r_data)
{
}

cw_bool_t
ch_search(cw_ch_t * a_ch, const void * a_key, void ** r_data)
{
}

cw_bool_t
ch_get_iterate(cw_ch_t * a_ch, void ** r_key, void ** r_data)
{
}

cw_bool_t
ch_remove_iterate(cw_ch_t * a_ch, void ** r_key, void ** r_data)
{
}

void
ch_dump(cw_ch_t * a_ch, cw_bool_t a_all)
{
}

cw_uint32_t
ch_hash_string(const void * a_key)
{
}

cw_uint32_t
ch_hash_direct(const void * a_key)
{
}

cw_bool_t
ch_key_comp_string(const void * a_k1, const void * a_k2)
{
}

cw_bool_t
ch_key_comp_direct(const void * a_k1, const void * a_k2)
{
}
