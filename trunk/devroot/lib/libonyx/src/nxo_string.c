/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: Onyx <Version = onyx>
 *
 ******************************************************************************/

#define CW_NXO_STRING_C_

#include "../include/libonyx/libonyx.h"

#include <stdarg.h>
#include <ctype.h>

#include "../include/libonyx/nxa_l.h"
#include "../include/libonyx/nxo_l.h"
#include "../include/libonyx/nxo_string_l.h"

#ifdef CW_THREADS
#define nxoe_p_string_lock(a_nxoe)					\
    do									\
    {									\
	if ((a_nxoe)->nxoe.locking && !(a_nxoe)->nxoe.indirect)		\
	{								\
	    mtx_lock(&(a_nxoe)->lock);					\
	}								\
    } while (0)
#define nxoe_p_string_unlock(a_nxoe)					\
    do									\
    {									\
	if ((a_nxoe)->nxoe.locking && !(a_nxoe)->nxoe.indirect)		\
	{								\
	    mtx_unlock(&(a_nxoe)->lock);				\
	}								\
    } while (0)
#endif

void
nxo_string_new(cw_nxo_t *a_nxo, cw_bool_t a_locking, cw_uint32_t a_len)
{
    cw_nxoe_string_t *string;

    string = (cw_nxoe_string_t *) nxa_malloc(sizeof(cw_nxoe_string_t));

    nxoe_l_new(&string->nxoe, NXOT_STRING, a_locking);
#ifdef CW_THREADS
    if (a_locking)
    {
	mtx_new(&string->lock);
    }
#endif
    string->e.s.len = a_len;
    string->e.s.alloc_len = a_len;
    if (string->e.s.len > 0)
    {
	string->e.s.str = (cw_uint8_t *) nxa_calloc(1, string->e.s.alloc_len);
    }
    else
    {
	string->e.s.str = NULL;
    }

    nxo_no_new(a_nxo);
    a_nxo->o.nxoe = (cw_nxoe_t *) string;
    nxo_p_type_set(a_nxo, NXOT_STRING);

    nxa_l_gc_register((cw_nxoe_t *) string);
}

void
nxo_string_substring_new(cw_nxo_t *a_nxo, cw_nxo_t *a_string,
			 cw_uint32_t a_offset, cw_uint32_t a_len)
{
    cw_nxoe_string_t *string, *orig;

    cw_check_ptr(a_nxo);

    orig = (cw_nxoe_string_t *) a_string->o.nxoe;
    cw_check_ptr(orig);
    cw_dassert(orig->nxoe.magic == CW_NXOE_MAGIC);

    string = (cw_nxoe_string_t *) nxa_malloc(sizeof(cw_nxoe_string_t));

    nxoe_l_new(&string->nxoe, NXOT_STRING, FALSE);
    string->nxoe.indirect = TRUE;

    if (orig->nxoe.indirect)
    {
	cw_assert(a_offset + a_len + orig->e.i.beg_offset
		  <= orig->e.i.string->e.s.len);
	string->e.i.string = orig->e.i.string;
	string->e.i.beg_offset = a_offset + orig->e.i.beg_offset;
    }
    else
    {
	cw_assert(a_offset + a_len <= orig->e.s.len);
	string->e.i.string = orig;
	string->e.i.beg_offset = a_offset;
    }
    string->e.i.len = a_len;

    nxo_no_new(a_nxo);
    a_nxo->o.nxoe = (cw_nxoe_t *) string;
    nxo_p_type_set(a_nxo, NXOT_STRING);

    nxa_l_gc_register((cw_nxoe_t *) string);
}

void
nxo_string_copy(cw_nxo_t *a_to, cw_nxo_t *a_from)
{
    cw_nxoe_string_t *string_fr, *string_fr_i = NULL, *string_fr_l;
    cw_nxoe_string_t *string_to, *string_to_i = NULL, *string_to_l;
    cw_uint8_t *str_fr, *str_to;
    cw_uint32_t len_fr, len_to;

    /* Set string pointers. */
    string_fr = (cw_nxoe_string_t *) a_from->o.nxoe;
    if (string_fr->nxoe.indirect)
    {
	string_fr_i = string_fr->e.i.string;
    }
    string_to = (cw_nxoe_string_t *) a_to->o.nxoe;
    if (string_to->nxoe.indirect)
    {
	string_to_i = string_to->e.i.string;
    }

    /* Set str_fr and len_fr according to whether string_fr is an indirect
     * object. */
    if (string_fr_i != NULL)
    {
	string_fr_l = string_fr_i;
	str_fr = &string_fr_i->e.s.str[string_fr->e.i.beg_offset];
	len_fr = string_fr->e.i.len;
	cw_assert(len_fr + string_fr->e.i.beg_offset <= string_fr_i->e.s.len);
    }
    else
    {
	string_fr_l = string_fr;
	str_fr = string_fr->e.s.str;
	len_fr = string_fr->e.s.len;
    }

    /* Set str_to and len_to according to whether string_to is an indirect
     * object. */
    if (string_to_i != NULL)
    {
	string_to_l = string_to_i;
	str_to = &string_to_i->e.s.str[string_to->e.i.beg_offset];
	len_to = string_to->e.i.len;
    }
    else
    {
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

    /* Truncate the destination string if it is shorter than the source
     * string. */
    if (len_to > len_fr)
    {
	if (string_to_i != NULL)
	{
	    string_to->e.i.len = len_fr;
	}
	else
	{
	    string_to->e.s.len = len_fr;
	}
    }
#ifdef CW_THREADS
    nxoe_p_string_unlock(string_to_l);
#endif
}

void
nxo_string_cstring(cw_nxo_t *a_to, cw_nxo_t *a_from, cw_nxo_t *a_thread)
{
    cw_uint32_t from_len;

    cw_assert(nxo_type_get(a_from) == NXOT_STRING
	      || nxo_type_get(a_from) == NXOT_NAME);

    /* Create a copy of a_from, but with a trailing '\0' so that it can be used
     * in calls to standard C functions. */
    if (nxo_type_get(a_from) == NXOT_STRING)
    {
	from_len = nxo_string_len_get(a_from);
	nxo_string_new(a_to, FALSE, from_len + 1);
	nxo_string_lock(a_from);
	nxo_string_set(a_to, 0, nxo_string_get(a_from), from_len);
	nxo_string_el_set(a_to, '\0', from_len);
	nxo_string_unlock(a_from);
    }
    else
    {
	from_len = nxo_name_len_get(a_from);
	nxo_string_new(a_to, FALSE, from_len + 1);
	nxo_string_set(a_to, 0, nxo_name_str_get(a_from), from_len);
	nxo_string_el_set(a_to, '\0', from_len);
    }
}

cw_uint32_t
nxo_string_len_get(const cw_nxo_t *a_nxo)
{
    cw_uint32_t retval;
    cw_nxoe_string_t *string;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_STRING);

    string = (cw_nxoe_string_t *) a_nxo->o.nxoe;

    cw_check_ptr(string);
    cw_dassert(string->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(string->nxoe.type == NXOT_STRING);

    if (string->nxoe.indirect)
    {
	retval = string->e.i.len;
    }
    else
    {
	retval = string->e.s.len;
    }

    return retval;
}

void
nxo_string_el_get(const cw_nxo_t *a_nxo, cw_nxoi_t a_offset, cw_uint8_t *r_el)
{
    cw_nxoe_string_t *string;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_STRING);

    string = (cw_nxoe_string_t *) a_nxo->o.nxoe;

    cw_check_ptr(string);
    cw_dassert(string->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(string->nxoe.type == NXOT_STRING);

    if (string->nxoe.indirect)
    {
	a_offset += string->e.i.beg_offset;
	string = string->e.i.string;
    }
    cw_assert(string->nxoe.indirect == FALSE);

    cw_assert(a_offset >= 0 && a_offset < string->e.s.len);
    *r_el = string->e.s.str[a_offset];
}

void
nxo_string_el_set(cw_nxo_t *a_nxo, cw_uint8_t a_el, cw_nxoi_t a_offset)
{
    cw_nxoe_string_t *string;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_STRING);

    string = (cw_nxoe_string_t *) a_nxo->o.nxoe;

    cw_check_ptr(string);
    cw_dassert(string->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(string->nxoe.type == NXOT_STRING);

    if (string->nxoe.indirect)
    {
	a_offset += string->e.i.beg_offset;
	string = string->e.i.string;
    }
    cw_assert(string->nxoe.indirect == FALSE);

    cw_assert(a_offset >= 0 && a_offset < string->e.s.len);
    string->e.s.str[a_offset] = a_el;
}

#ifdef CW_THREADS
void
nxo_string_lock(cw_nxo_t *a_nxo)
{
    cw_nxoe_string_t *string;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_STRING);

    string = (cw_nxoe_string_t *) a_nxo->o.nxoe;

    cw_check_ptr(string);
    cw_dassert(string->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(string->nxoe.type == NXOT_STRING);

    if (string->nxoe.indirect)
    {
	string = string->e.i.string;
    }
    cw_assert(string->nxoe.indirect == FALSE);

    nxoe_p_string_lock(string);
}

void
nxo_string_unlock(cw_nxo_t *a_nxo)
{
    cw_nxoe_string_t *string;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_STRING);

    string = (cw_nxoe_string_t *) a_nxo->o.nxoe;

    cw_check_ptr(string);
    cw_dassert(string->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(string->nxoe.type == NXOT_STRING);

    if (string->nxoe.indirect)
    {
	string = string->e.i.string;
    }
    cw_assert(string->nxoe.indirect == FALSE);

    nxoe_p_string_unlock(string);
}
#endif

cw_uint8_t *
nxo_string_get(const cw_nxo_t *a_nxo)
{
    cw_uint8_t *retval;
    cw_nxoe_string_t *string;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_STRING);

    string = (cw_nxoe_string_t *) a_nxo->o.nxoe;

    cw_check_ptr(string);
    cw_dassert(string->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(string->nxoe.type == NXOT_STRING);

    if (string->nxoe.indirect)
    {
	retval = &string->e.i.string->e.s.str[string->e.i.beg_offset];
    }
    else
    {
	retval = string->e.s.str;
    }

    return retval;
}

void
nxo_string_set(cw_nxo_t *a_nxo, cw_uint32_t a_offset, const cw_uint8_t *a_str,
	       cw_uint32_t a_len)
{
    cw_nxoe_string_t *string;
    cw_uint8_t *str;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_STRING);

    string = (cw_nxoe_string_t *) a_nxo->o.nxoe;

    cw_check_ptr(string);
    cw_dassert(string->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(string->nxoe.type == NXOT_STRING);

    /* Get the string pointer. */
    if (string->nxoe.indirect)
    {
	a_offset += string->e.i.beg_offset;
	cw_assert(a_offset + a_len <= string->e.i.string->e.s.len);
	str = string->e.i.string->e.s.str;
    }
    else
    {
	cw_assert(a_offset + a_len <= string->e.s.len);
	str = string->e.s.str;
    }
    memcpy(&str[a_offset], a_str, a_len);
}
