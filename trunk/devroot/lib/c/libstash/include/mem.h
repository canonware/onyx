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
 *
 *
 ****************************************************************************/

#ifndef _MEM_H_
#define _MEM_H_

typedef struct cw_mem_s cw_mem_t;

#define mem_new _CW_NS_ANY(mem_new)
#define mem_delete _CW_NS_ANY(mem_delete)
#define mem_malloc _CW_NS_ANY(mem_malloc)
#define mem_calloc _CW_NS_ANY(mem_calloc)
#define mem_realloc _CW_NS_ANY(mem_realloc)
#define mem_free _CW_NS_ANY(mem_free)

cw_mem_t * mem_new();
void mem_delete(cw_mem_t * a_mem_o);
void * mem_malloc(cw_mem_t * a_mem_o, size_t a_size);
void * mem_calloc(cw_mem_t * a_mem_o, size_t a_number, size_t a_size);
void * mem_realloc(cw_mem_t * a_mem_o, void * a_ptr, size_t a_size);
void mem_free(cw_mem_t * a_mem_o, void * a_ptr);

#endif /* _MEM_H_ */
