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

#include "../include/libstil/libstil.h"

#ifdef _LIBSTIL_DBG
#define _CW_STILS_MAGIC 0x0ea67890
#endif

cw_stils_t *
stils_new(cw_stils_t *a_stils, cw_pezz_t *a_chunk_pezz)
{
	cw_stils_t *retval;

	_cw_check_ptr(a_stils);
	_cw_check_ptr(a_chunk_pezz);



	return retval;
}

void
stils_delete(cw_stils_t *a_stils)
{
	_cw_check_ptr(a_stils);
	_cw_assert(_CW_STILS_MAGIC == a_stils->magic);

}

void
stils_collect(cw_stils_t *a_stils,
    void (*a_add_root_func) (void *add_root_arg, cw_stiloe_t *root),
    void *a_add_root_arg)
{
}

cw_stilo_t *
stils_push(cw_stils_t *a_stils)
{
	_cw_check_ptr(a_stils);
	_cw_assert(_CW_STILS_MAGIC == a_stils->magic);
}

cw_bool_t
stils_pop(cw_stils_t *a_stils, cw_uint32_t a_count)
{
	_cw_check_ptr(a_stils);
	_cw_assert(_CW_STILS_MAGIC == a_stils->magic);
}

cw_bool_t
stils_roll(cw_stils_t *a_stils, cw_sint32_t a_count)
{
	_cw_check_ptr(a_stils);
	_cw_assert(_CW_STILS_MAGIC == a_stils->magic);
}

cw_bool_t
stils_dup(cw_stils_t *a_stils, cw_uint32_t a_count, cw_uint32_t a_index)
{
	_cw_check_ptr(a_stils);
	_cw_assert(_CW_STILS_MAGIC == a_stils->magic);
}

cw_uint32_t
stils_count(cw_stils_t *a_stils)
{
	_cw_check_ptr(a_stils);
	_cw_assert(_CW_STILS_MAGIC == a_stils->magic);
}

cw_stilo_t *
stils_get(cw_stils_t *a_stils, cw_uint32_t a_index)
{
	_cw_check_ptr(a_stils);
	_cw_assert(_CW_STILS_MAGIC == a_stils->magic);
}

cw_stilo_t *
stils_get_down(cw_stils_t *a_stils, cw_stilo_t *a_stilo)
{
	_cw_check_ptr(a_stils);
	_cw_assert(_CW_STILS_MAGIC == a_stils->magic);
}

cw_stilo_t *
stils_get_up(cw_stils_t *a_stils, cw_stilo_t *a_stilo)
{
	_cw_check_ptr(a_stils);
	_cw_assert(_CW_STILS_MAGIC == a_stils->magic);
}

cw_bool_t
stils_get_index(cw_stils_t *a_stils, cw_stilo_t *a_stilo)
{
	_cw_check_ptr(a_stils);
	_cw_assert(_CW_STILS_MAGIC == a_stils->magic);
}
