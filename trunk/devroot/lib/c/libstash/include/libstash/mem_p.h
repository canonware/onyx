/****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 ****************************************************************************/

struct cw_mem_s {
	cw_mtx_t lock;

#ifdef _LIBSTASH_DBG
	cw_oh_t addr_hash;
#endif

	cw_mem_oom_handler_t *oom_handler;
        const void *handler_data;
};

#ifdef _LIBSTASH_DBG
struct cw_mem_item_s {
	cw_uint32_t size;
        const char *filename;
	cw_uint32_t line_num;
};
#endif
