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
 ******************************************************************************
 *
 * Maintain a hash of address-->ostr/line_num mappings, which can be used
 * to report the origin of procedures in stack traces.  A separate hash of
 * ostrs is maintained, in order to reduce the total amount of memory used.
 *
 * All memory allocated in this file uses cw_g_mem/cw_g_mema, rather than the
 * nxa APIs, in order to avoid bootstrapping issues.
 *
 ******************************************************************************/

#include "../include/libonyx/libonyx.h"

typedef struct cw_origin_ostr_s cw_origin_ostr_t;
struct cw_origin_ostr_s
{
    cw_uint8_t *ostr;
    cw_uint32_t olen;

    cw_uint32_t ref_count; /* Reference count, used to remove hash items. */
    cw_chi_t chi; /* For internal dch linkage. */
};

typedef struct cw_origin_obj_s cw_origin_obj_t;
struct cw_origin_obj_s
{
    cw_origin_ostr_t *ostr_hash_key;
    cw_uint32_t line_num;
    cw_chi_t chi; /* For internal dch linkage. */
};

/* File-global variables. */
#ifdef CW_DBG
static cw_bool_t s_origin_initialized = FALSE;
#endif

#ifdef CW_THREADS
static cw_mtx_t s_origin_lock;
#endif

/* Slots in base hash table. */
#define CW_ORIGIN_OSTR_BASE_TABLE 32
/* Maximum fullness of base table. */
#define CW_ORIGIN_OSTR_BASE_GROW 8
/* Proportional minimal fullness. */
#define CW_ORIGIN_OSTR_BASE_SHRINK 1
static cw_dch_t s_origin_ostr_hash;

/* Slots in base hash table. */
#define CW_ORIGIN_OBJ_BASE_TABLE 1024
/* Maximum fullness of base table. */
#define CW_ORIGIN_OBJ_BASE_GROW 256
/* Proportional minimal fullness. */
#define CW_ORIGIN_OBJ_BASE_SHRINK 1
static cw_dch_t s_origin_obj_hash;

/* Prototypes. */
static cw_uint32_t
origin_p_ostr_hash(const void *a_key);
static cw_bool_t
origin_p_ostr_key_comp(const void *a_k1, const void *a_k2);

void
origin_l_init(void)
{
    cw_assert(s_origin_initialized == FALSE);

#ifdef CW_THREADS
    mtx_new(&s_origin_lock);
#endif

    /* Initialize ostr hash. */
    dch_new(&s_origin_ostr_hash, cw_g_mema, CW_ORIGIN_OSTR_BASE_TABLE,
	    CW_ORIGIN_OSTR_BASE_GROW, CW_ORIGIN_OSTR_BASE_SHRINK,
	    origin_p_ostr_hash, origin_p_ostr_key_comp);

    /* Initialize obj hash. */
    dch_new(&s_origin_obj_hash, cw_g_mema, CW_ORIGIN_OBJ_BASE_TABLE,
	    CW_ORIGIN_OBJ_BASE_GROW, CW_ORIGIN_OBJ_BASE_SHRINK,
	    ch_direct_hash, ch_direct_key_comp);

#ifdef CW_DBG
    s_origin_initialized = TRUE;
#endif
}

void
origin_l_shutdown(void)
{
    cw_assert(s_origin_initialized);

#ifdef CW_DBG
    s_origin_initialized = FALSE;
#endif

    /* Destroy obj hash. */
    cw_assert(dch_count(&s_origin_obj_hash) == 0);
    dch_delete(&s_origin_obj_hash);

    /* Destroy ostr hash. */
    cw_assert(dch_count(&s_origin_ostr_hash) == 0);
    dch_delete(&s_origin_ostr_hash);
    
#ifdef CW_THREADS
    mtx_delete(&s_origin_lock);
#endif
}

CW_P_INLINE cw_origin_ostr_t *
origin_p_ostr_insert(const cw_uint8_t *a_ostr, cw_uint32_t a_olen)
{
    cw_origin_ostr_t *retval;
    cw_origin_ostr_t tkey;

    tkey.ostr = (cw_uint8_t *) a_ostr;
    tkey.olen = a_olen;

    if (dch_search(&s_origin_ostr_hash, (void *) &tkey, (void **) &retval)
	== FALSE)
    {
	/* Increment reference count. */
	retval->ref_count++;
    }
    else
    {
	/* Allocate ostr string. */

	/* Insert a new record. */
	retval = (cw_origin_ostr_t *) cw_malloc(sizeof(cw_origin_ostr_t));
	retval->ostr = cw_malloc(a_olen);
	memcpy(retval->ostr, a_ostr, a_olen);
	retval->olen = a_olen;
	retval->ref_count = 1;

	dch_insert(&s_origin_ostr_hash, (void *) retval, (void *) retval,
		   &retval->chi);
    }

    return retval;
}

CW_P_INLINE void
origin_p_ostr_remove(cw_origin_ostr_t *a_ostr_hash_key)
{
    if (dch_search(&s_origin_ostr_hash, (void *) a_ostr_hash_key, NULL))
    {
	cw_not_reached();
    }

    /* Decrement reference count. */
    a_ostr_hash_key->ref_count--;

    if (a_ostr_hash_key->ref_count == 0)
    {
	/* Remove record. */
	dch_remove(&s_origin_ostr_hash, (void *) a_ostr_hash_key, NULL,
		   NULL, NULL);

	/* Deallocate. */
	cw_free(a_ostr_hash_key->ostr);
	cw_free(a_ostr_hash_key);
    }
}

void
origin_l_insert(void *a_obj, const cw_uint8_t *a_ostr, cw_uint32_t a_olen,
		cw_uint32_t a_line_num)
{
    cw_origin_obj_t *obj_hash_item;
    cw_origin_ostr_t *ostr_hash_key;

    cw_assert(s_origin_initialized);
    cw_check_ptr(a_obj);

#ifdef CW_THREADS
    mtx_lock(&s_origin_lock);
#endif
    
    cw_assert(dch_search(&s_origin_obj_hash, a_obj, NULL));

    /* Get a ostr hash key. */
    ostr_hash_key = origin_p_ostr_insert(a_ostr, a_olen);

    obj_hash_item = (cw_origin_obj_t *) cw_malloc(sizeof(cw_origin_obj_t));
    obj_hash_item->ostr_hash_key = ostr_hash_key;
    obj_hash_item->line_num = a_line_num;

    dch_insert(&s_origin_obj_hash, a_obj, (void *) obj_hash_item,
	       &obj_hash_item->chi);

#ifdef CW_THREADS
    mtx_unlock(&s_origin_lock);
#endif
}

void
origin_l_remove(void *a_obj)
{
    cw_origin_obj_t *obj_hash_item;

    cw_assert(s_origin_initialized);

#ifdef CW_THREADS
    mtx_lock(&s_origin_lock);
#endif

    if (dch_remove(&s_origin_obj_hash, a_obj, NULL, (void **) &obj_hash_item,
		   NULL))
    {
	/* No record for this obj. */
#ifdef CW_THREADS
	mtx_unlock(&s_origin_lock);
#endif
	return;
    }

    /* Remove the reference to the ostr in the ostr hash. */
    origin_p_ostr_remove(obj_hash_item->ostr_hash_key);

    /* Deallocate. */
    cw_free(obj_hash_item);

#ifdef CW_THREADS
    mtx_unlock(&s_origin_lock);
#endif
}

cw_bool_t
origin_l_lookup(void *a_obj, const cw_uint8_t **r_ostr,
		cw_uint32_t *r_olen, cw_uint32_t *r_line_num)
{
    cw_bool_t retval;
    cw_origin_obj_t *obj_hash_item;

    cw_assert(s_origin_initialized);

#ifdef CW_THREADS
    mtx_lock(&s_origin_lock);
#endif

    retval = dch_search(&s_origin_obj_hash, a_obj, (void **) &obj_hash_item);
    if (retval == FALSE)
    {
	if (r_ostr != NULL)
	{
	    *r_ostr = obj_hash_item->ostr_hash_key->ostr;
	}
	if (r_olen != NULL)
	{
	    *r_olen = obj_hash_item->ostr_hash_key->olen;
	}
	if (r_line_num != NULL)
	{
	    *r_line_num = obj_hash_item->line_num;
	}
    }

#ifdef CW_THREADS
    mtx_unlock(&s_origin_lock);
#endif
    return retval;
}

static cw_uint32_t
origin_p_ostr_hash(const void *a_key)
{
    cw_uint32_t retval;
    cw_origin_ostr_t *key = (cw_origin_ostr_t *) a_key;
    cw_uint8_t *str;
    cw_uint32_t i, len;

    str = key->ostr;
    len = key->olen;
    for (i = retval = 0; i < len; i++)
    {
	retval = retval * 33 + str[i];
    }

    return retval;
}

static cw_bool_t
origin_p_ostr_key_comp(const void *a_k1, const void *a_k2)
{
    cw_bool_t retval;
    cw_origin_ostr_t *k1 = (cw_origin_ostr_t *) a_k1;
    cw_origin_ostr_t *k2 = (cw_origin_ostr_t *) a_k2;

    cw_check_ptr(k1);
    cw_check_ptr(k2);

    if (k1->olen == k2->olen
	&& (memcmp(k1->ostr, k2->ostr, k1->olen) == 0))
    {
	retval = TRUE;
    }
    else
    {
	retval = FALSE;
    }

    return retval;
}
