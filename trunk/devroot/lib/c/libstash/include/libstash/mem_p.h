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
#  ifdef _CW_REENTRANT
  cw_mtx_t lock;
#  endif
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
