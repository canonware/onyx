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
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

typedef struct cw_mem_s cw_mem_t;

#define mem_new _CW_NS_LIBSTASH(mem_new)
cw_mem_t *
mem_new();

#define mem_delete _CW_NS_LIBSTASH(mem_delete)
void
mem_delete(cw_mem_t * a_mem);

#define mem_malloc _CW_NS_LIBSTASH(mem_malloc)
void *
mem_malloc(cw_mem_t * a_mem, size_t a_size);

#define mem_calloc _CW_NS_LIBSTASH(mem_calloc)
void *
mem_calloc(cw_mem_t * a_mem, size_t a_number, size_t a_size);

#define mem_realloc _CW_NS_LIBSTASH(mem_realloc)
void *
mem_realloc(cw_mem_t * a_mem, void * a_ptr, size_t a_size);

#define mem_free _CW_NS_LIBSTASH(mem_free)
void
mem_free(cw_mem_t * a_mem, void * a_ptr);

#define mem_dealloc _CW_NS_LIBSTASH(mem_dealloc)
void
mem_dealloc(void * a_mem, void * a_ptr);

