/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 86 $
 * $Date: 1998-06-23 17:40:29 -0700 (Tue, 23 Jun 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#ifndef _MEM_H_
#define _MEM_H_

typedef struct cw_mem_s cw_mem_t;

#define mem_new _CW_NS_CMN(mem_new)
#define mem_delete _CW_NS_CMN(mem_delete)
#define mem_malloc _CW_NS_CMN(mem_malloc)
#define mem_calloc _CW_NS_CMN(mem_calloc)
#define mem_realloc _CW_NS_CMN(mem_realloc)
#define mem_free _CW_NS_CMN(mem_free)

cw_mem_t * mem_new();
void mem_delete(cw_mem_t * a_mem_o);
void * mem_malloc(cw_mem_t * a_mem_o, size_t a_size);
void * mem_calloc(cw_mem_t * a_mem_o, size_t a_number, size_t a_size);
void * mem_realloc(cw_mem_t * a_mem_o, void * a_ptr, size_t a_size);
void mem_free(cw_mem_t * a_mem_o, void * a_ptr);

#endif /* _MEM_H_ */
