/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

struct cw_mem_s
{
#ifdef _LIBSTASH_DBG
  cw_oh_t addr_hash;
#else
  int garbage;
#endif
};

#ifdef _LIBSTASH_DBG
struct cw_mem_item_s
{
  cw_uint32_t size;
  const char * filename;
  cw_uint32_t line_num;
};
#endif

#ifdef _LIBSTASH_DBG
static cw_uint64_t
mem_p_oh_h1(cw_oh_t * a_oh, const void * a_key);

static cw_bool_t
mem_p_oh_key_compare(const void * a_k1, const void * a_k2);
#endif
