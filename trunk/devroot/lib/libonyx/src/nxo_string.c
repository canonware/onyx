/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#include "../include/libonyx/libonyx.h"

#include <stdarg.h>
#include <ctype.h>

#include "../include/libonyx/nxa_l.h"
#include "../include/libonyx/nxo_l.h"
#include "../include/libonyx/nxo_string_l.h"

#ifdef CW_THREADS
#define		nxoe_p_string_lock(a_nxoe) do {			\
	if ((a_nxoe)->nxoe.locking && !(a_nxoe)->nxoe.indirect)	\
		mtx_lock(&(a_nxoe)->lock);				\
} while (0)
#define		nxoe_p_string_unlock(a_nxoe) do {			\
	if ((a_nxoe)->nxoe.locking && !(a_nxoe)->nxoe.indirect)	\
		mtx_unlock(&(a_nxoe)->lock);				\
} while (0)
#endif

void
nxo_string_new(cw_nxo_t *a_nxo, cw_nx_t *a_nx, cw_bool_t a_locking, cw_uint32_t
    a_len)
{
	cw_nxoe_string_t	*string;
	cw_nxa_t		*nxa;

	nxa = nx_nxa_get(a_nx);
	string = (cw_nxoe_string_t *)nxa_malloc(nxa, sizeof(cw_nxoe_string_t));

	nxoe_l_new(&string->nxoe, NXOT_STRING, a_locking);
#ifdef CW_THREADS
	if (a_locking)
		mtx_new(&string->lock);
#endif
	string->e.s.len = a_len;
	string->e.s.alloc_len = a_len;
	if (string->e.s.len > 0) {
		string->e.s.str = (cw_uint8_t *)nxa_malloc(nxa,
		    string->e.s.alloc_len);
		memset(string->e.s.str, 0, string->e.s.len);
	} else
		string->e.s.str = NULL;

	nxo_no_new(a_nxo);
	a_nxo->o.nxoe = (cw_nxoe_t *)string;
	nxo_p_type_set(a_nxo, NXOT_STRING);

	nxa_l_gc_register(nx_nxa_get(a_nx), (cw_nxoe_t *)string);
}

void
nxo_string_substring_new(cw_nxo_t *a_nxo, cw_nxo_t *a_string, cw_nx_t *a_nx,
    cw_uint32_t a_offset, cw_uint32_t a_len)
{
	cw_nxoe_string_t	*string, *orig;

	cw_check_ptr(a_nxo);

	orig = (cw_nxoe_string_t *)a_string->o.nxoe;
	cw_check_ptr(orig);
	cw_dassert(orig->nxoe.magic == CW_NXOE_MAGIC);

	if (orig->nxoe.indirect) {
		nxo_string_substring_new(a_nxo, &orig->e.i.nxo, a_nx, a_offset +
		    orig->e.i.beg_offset, a_len);
	} else {
		cw_assert(a_offset + a_len <= orig->e.s.len);

		string = (cw_nxoe_string_t
		    *)nxa_malloc(nx_nxa_get(a_nx), sizeof(cw_nxoe_string_t));

		nxoe_l_new(&string->nxoe, NXOT_STRING, FALSE);
		string->nxoe.indirect = TRUE;
		memcpy(&string->e.i.nxo, a_string, sizeof(cw_nxo_t));
		string->e.i.beg_offset = a_offset;
		string->e.i.len = a_len;

		nxo_no_new(a_nxo);
		a_nxo->o.nxoe = (cw_nxoe_t *)string;
		nxo_p_type_set(a_nxo, NXOT_STRING);

		nxa_l_gc_register(nx_nxa_get(a_nx), (cw_nxoe_t *)string);
	}
}

cw_bool_t
nxoe_l_string_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t a_iter)
{
	cw_nxoe_string_t	*string;

	string = (cw_nxoe_string_t *)a_nxoe;

	cw_check_ptr(string);
	cw_dassert(string->nxoe.magic == CW_NXOE_MAGIC);
	cw_assert(string->nxoe.type == NXOT_STRING);

	if (string->nxoe.indirect == FALSE && string->e.s.alloc_len > 0)
		nxa_free(a_nxa, string->e.s.str, string->e.s.alloc_len);

#ifdef CW_THREADS
	if (string->nxoe.locking && string->nxoe.indirect == FALSE)
		mtx_delete(&string->lock);
#endif

	nxa_free(a_nxa, string, sizeof(cw_nxoe_string_t));

	return FALSE;
}

cw_nxoe_t *
nxoe_l_string_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset)
{
	cw_nxoe_t		*retval;
	cw_nxoe_string_t	*string;

	string = (cw_nxoe_string_t *)a_nxoe;

	if (a_reset)
		string->ref_iter = 0;

	if (a_nxoe->indirect == FALSE)
		retval = NULL;
	else if (string->ref_iter == 0) {
		retval = string->e.i.nxo.o.nxoe;
		string->ref_iter++;
	} else
		retval = NULL;

	return retval;
}

void
nxo_string_copy(cw_nxo_t *a_to, cw_nxo_t *a_from)
{
	cw_nxoe_string_t	*string_fr, *string_fr_i = NULL, *string_fr_l;
	cw_nxoe_string_t	*string_to, *string_to_i = NULL, *string_to_l;
	cw_uint8_t		*str_fr, *str_to;
	cw_uint32_t		len_fr, len_to;

	/*
	 * Set string pointers.
	 */
	string_fr = (cw_nxoe_string_t *)a_from->o.nxoe;
	if (string_fr->nxoe.indirect)
		string_fr_i = (cw_nxoe_string_t *)string_fr->e.i.nxo.o.nxoe;
	string_to = (cw_nxoe_string_t *)a_to->o.nxoe;
	if (string_to->nxoe.indirect)
		string_to_i = (cw_nxoe_string_t *)string_to->e.i.nxo.o.nxoe;

	/*
	 * Set str_fr and len_fr according to whether string_fr is an indirect
	 * object.
	 */
	if (string_fr_i != NULL) {
		string_fr_l = string_fr_i;
		str_fr = &string_fr_i->e.s.str[string_fr->e.i.beg_offset];
		len_fr = string_fr->e.i.len;
		cw_assert(len_fr + string_fr->e.i.beg_offset <=
		    string_fr_i->e.s.len);
	} else {
		string_fr_l = string_fr;
		str_fr = string_fr->e.s.str;
		len_fr = string_fr->e.s.len;
	}

	/*
	 * Set str_to and len_to according to whether string_to is an indirect
	 * object.
	 */
	if (string_to_i != NULL) {
		string_to_l = string_to_i;
		str_to = &string_to_i->e.s.str[string_to->e.i.beg_offset];
		len_to = string_to->e.i.len;
	} else {
		string_to_l = string_to;
		str_to = string_to->e.s.str;
		len_to = string_to->e.s.len;
	}

	/* Make sure destination is large enough. */
	cw_assert(len_fr <= len_to);

	/* Copy bytes. */
#ifdef CW_THREADS
	nxoe_p_string_lock(string_fr_l);
	nxoe_p_string_lock(string_to_l);
#endif
	memcpy(str_to, str_fr, len_fr);
#ifdef CW_THREADS
	nxoe_p_string_unlock(string_fr_l);
#endif

	/*
	 * Truncate the destination string if it is shorter than the source
	 * string.
	 */
	if (len_to > len_fr) {
		if (string_to_i != NULL)
			string_to->e.i.len = len_fr;
		else
			string_to->e.s.len = len_fr;
	}
#ifdef CW_THREADS
	nxoe_p_string_unlock(string_to_l);
#endif
}

void
nxo_string_cstring(cw_nxo_t *a_to, cw_nxo_t *a_from, cw_nxo_t *a_thread)
{
	cw_uint32_t	from_len;

	/*
	 * Create a copy of a_from, but with a trailing '\0' so that it can be
	 * used in calls to standard C functions.
	 */
	from_len = nxo_string_len_get(a_from);
	nxo_string_new(a_to, nxo_thread_nx_get(a_thread), FALSE, from_len + 1);
	nxo_string_lock(a_from);
	nxo_string_set(a_to, 0, nxo_string_get(a_from), from_len);
	nxo_string_el_set(a_to, '\0', from_len);
	nxo_string_unlock(a_from);
}

cw_uint32_t
nxo_string_len_get(cw_nxo_t *a_nxo)
{
	cw_uint32_t		retval;
	cw_nxoe_string_t	*string;

	cw_check_ptr(a_nxo);
	cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
	cw_assert(nxo_type_get(a_nxo) == NXOT_STRING);

	string = (cw_nxoe_string_t *)a_nxo->o.nxoe;

	cw_check_ptr(string);
	cw_dassert(string->nxoe.magic == CW_NXOE_MAGIC);
	cw_assert(string->nxoe.type == NXOT_STRING);

	if (string->nxoe.indirect == FALSE)
		retval = string->e.s.len;
	else
		retval = string->e.i.len;

	return retval;
}

void
nxo_string_el_get(cw_nxo_t *a_nxo, cw_nxoi_t a_offset, cw_uint8_t *r_el)
{
	cw_nxoe_string_t	*string;

	cw_check_ptr(a_nxo);
	cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
	cw_assert(nxo_type_get(a_nxo) == NXOT_STRING);

	string = (cw_nxoe_string_t *)a_nxo->o.nxoe;

	cw_check_ptr(string);
	cw_dassert(string->nxoe.magic == CW_NXOE_MAGIC);
	cw_assert(string->nxoe.type == NXOT_STRING);

	if (string->nxoe.indirect == FALSE) {
		cw_assert(a_offset >= 0 && a_offset < string->e.s.len);
		*r_el = string->e.s.str[a_offset];
	} else {
		nxo_string_el_get(&string->e.i.nxo, a_offset +
		    string->e.i.beg_offset, r_el);
	}
}

void
nxo_string_el_set(cw_nxo_t *a_nxo, cw_uint8_t a_el, cw_nxoi_t a_offset)
{
	cw_nxoe_string_t	*string;

	cw_check_ptr(a_nxo);
	cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
	cw_assert(nxo_type_get(a_nxo) == NXOT_STRING);

	string = (cw_nxoe_string_t *)a_nxo->o.nxoe;

	cw_check_ptr(string);
	cw_dassert(string->nxoe.magic == CW_NXOE_MAGIC);
	cw_assert(string->nxoe.type == NXOT_STRING);

	if (string->nxoe.indirect == FALSE) {
		cw_assert(a_offset >= 0 && a_offset < string->e.s.len);
		string->e.s.str[a_offset] = a_el;
	} else {
		nxo_string_el_set(&string->e.i.nxo, a_el, a_offset +
		    string->e.i.beg_offset);
	}
}

#ifdef CW_THREADS
void
nxo_string_lock(cw_nxo_t *a_nxo)
{
	cw_nxoe_string_t	*string;

	cw_check_ptr(a_nxo);
	cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
	cw_assert(nxo_type_get(a_nxo) == NXOT_STRING);

	string = (cw_nxoe_string_t *)a_nxo->o.nxoe;

	cw_check_ptr(string);
	cw_dassert(string->nxoe.magic == CW_NXOE_MAGIC);
	cw_assert(string->nxoe.type == NXOT_STRING);

	if (string->nxoe.indirect == FALSE)
		nxoe_p_string_lock(string);
	else
		nxo_string_lock(&string->e.i.nxo);
}

void
nxo_string_unlock(cw_nxo_t *a_nxo)
{
	cw_nxoe_string_t	*string;

	cw_check_ptr(a_nxo);
	cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
	cw_assert(nxo_type_get(a_nxo) == NXOT_STRING);

	string = (cw_nxoe_string_t *)a_nxo->o.nxoe;

	cw_check_ptr(string);
	cw_dassert(string->nxoe.magic == CW_NXOE_MAGIC);
	cw_assert(string->nxoe.type == NXOT_STRING);

	if (string->nxoe.indirect == FALSE)
		nxoe_p_string_unlock(string);
	else
		nxo_string_unlock(&string->e.i.nxo);
}
#endif

cw_uint8_t *
nxo_string_get(cw_nxo_t *a_nxo)
{
	cw_uint8_t		*retval;
	cw_nxoe_string_t	*string;

	cw_check_ptr(a_nxo);
	cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
	cw_assert(nxo_type_get(a_nxo) == NXOT_STRING);

	string = (cw_nxoe_string_t *)a_nxo->o.nxoe;

	cw_check_ptr(string);
	cw_dassert(string->nxoe.magic == CW_NXOE_MAGIC);
	cw_assert(string->nxoe.type == NXOT_STRING);

	if (string->nxoe.indirect == FALSE)
		retval = string->e.s.str;
	else {
		retval = &nxo_string_get(&string->e.i.nxo)
		    [string->e.i.beg_offset];
	}

	return retval;
}

void
nxo_string_set(cw_nxo_t *a_nxo, cw_uint32_t a_offset, const cw_uint8_t *a_str,
    cw_uint32_t a_len)
{
	cw_nxoe_string_t	*string;
	cw_uint8_t		*str;

	cw_check_ptr(a_nxo);
	cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
	cw_assert(nxo_type_get(a_nxo) == NXOT_STRING);

	string = (cw_nxoe_string_t *)a_nxo->o.nxoe;

	cw_check_ptr(string);
	cw_dassert(string->nxoe.magic == CW_NXOE_MAGIC);
	cw_assert(string->nxoe.type == NXOT_STRING);

	/* Get the string pointer. */
	if (string->nxoe.indirect == FALSE) {
		cw_assert(a_offset + a_len <= string->e.s.len);
		str = string->e.s.str;

		memcpy(&str[a_offset], a_str, a_len);
	} else
		nxo_string_set(&string->e.i.nxo, a_offset, a_str, a_len);
}
