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

#include "../include/libkasi/libkasi.h"

#ifdef _LIBKASI_DBG
#  define _CW_KASI_MAGIC 0xae9678fd
#  define _CW_KASIN_MAGIC 0xaeb78944
#endif

/* Number of kasi_bufc structures to allocate at a time via the pezz code. */
#define _CW_KASI_BUFC_CHUNK_COUNT 16

/* Size and fullness control of initial name cache hash table.  We know for sure
 * that there will be about 175 names referenced by systemdict and threaddict to
 * begin with. */
#define _CW_KASI_KASIN_BASE_TABLE  512
#define _CW_KASI_KASIN_BASE_GROW   400
#define _CW_KASI_KASIN_BASE_SHRINK 128

cw_kasin_t *
kasi_p_kasin_new(cw_kasi_t * a_kasi);
void
kasi_p_kasin_delete(cw_kasi_t * a_kasi, cw_kasin_t * a_kasin);
cw_bool_t
kasi_p_kasin_kref(cw_kasi_t * a_kasi, cw_kasin_t * a_kasin, const void * a_key,
		  const void * a_data);
static cw_uint32_t
kasink_p_hash(const void * a_key);
static cw_bool_t
kasink_p_key_comp(const void * a_k1, const void * a_k2);

cw_kasi_t *
kasi_new(cw_kasi_t * a_kasi)
{
  cw_kasi_t * retval;

  if (NULL != a_kasi)
  {
    retval = a_kasi;
    bzero(retval, sizeof(cw_kasi_t));
    retval->is_malloced = FALSE;
  }
  else
  {
    retval = (cw_kasi_t *) _cw_malloc(sizeof(cw_kasi_t));
    if (NULL == retval)
    {
      goto RETURN;
    }
    bzero(retval, sizeof(cw_kasi_t));
    retval->is_malloced = TRUE;
  }

  if (NULL == pezz_new(&retval->kasi_bufc_pezz, sizeof(cw_kasi_bufc_t),
		       _CW_KASI_BUFC_CHUNK_COUNT))
  {
    if (TRUE == retval->is_malloced)
    {
      _cw_free(retval);
    }
    retval = NULL;
    goto RETURN;
  }

  if (NULL == pezz_new(&retval->chi_pezz, sizeof(cw_chi_t),
		       _CW_KASI_KASIN_BASE_GROW / 4))
  {
    pezz_delete(&retval->kasi_bufc_pezz);
    if (TRUE == retval->is_malloced)
    {
      _cw_free(retval);
    }
    retval = NULL;
    goto RETURN;
  }

  if (NULL == pezz_new(&retval->kasin_pezz, sizeof(cw_kasin_t),
		       _CW_KASI_KASIN_BASE_GROW / 4))
  {
    pezz_delete(&retval->chi_pezz);
    pezz_delete(&retval->kasi_bufc_pezz);
    if (TRUE == retval->is_malloced)
    {
      _cw_free(retval);
    }
    retval = NULL;
    goto RETURN;
  }

  if (NULL == dch_new(&retval->kasin_dch, _CW_KASI_KASIN_BASE_TABLE,
		      _CW_KASI_KASIN_BASE_GROW, _CW_KASI_KASIN_BASE_SHRINK,
		      &retval->chi_pezz, kasink_p_hash, kasink_p_key_comp))
  {
    pezz_delete(&retval->chi_pezz);
    pezz_delete(&retval->kasin_pezz);
    pezz_delete(&retval->kasi_bufc_pezz);
    if (TRUE == retval->is_malloced)
    {
      _cw_free(retval);
    }
    retval = NULL;
    goto RETURN;
  }

  mtx_new(&retval->lock);

#ifdef _LIBKASI_DBG
  retval->magic = _CW_KASI_MAGIC;
#endif

  RETURN:
  return retval;
}

void
kasi_delete(cw_kasi_t * a_kasi)
{
  cw_kasink_t * key;
  cw_kasin_t * data;

  _cw_check_ptr(a_kasi);
  _cw_assert(_CW_KASI_MAGIC == a_kasi->magic);

  while (FALSE == dch_remove_iterate(&a_kasi->kasin_dch, (void **) &key,
				     (void **) &data))
  {
    kasi_p_kasin_delete(a_kasi, data);
  }
  dch_delete(&a_kasi->kasin_dch);
  pezz_delete(&a_kasi->kasin_pezz);
    
  pezz_delete(&a_kasi->kasi_bufc_pezz);

  mtx_delete(&a_kasi->lock);

  if (TRUE == a_kasi->is_malloced)
  {
    _cw_free(a_kasi);
  }
#ifdef _LIBKASI_DBG
  else
  {
    memset(a_kasi, 0x5a, sizeof(cw_kasi_t));
  }
#endif
}

cw_kasi_bufc_t *
kasi_get_kasi_bufc(cw_kasi_t * a_kasi)
{
  cw_kasi_bufc_t * retval;

  _cw_check_ptr(a_kasi);
  _cw_assert(_CW_KASI_MAGIC == a_kasi->magic);
  
  retval = (cw_kasi_bufc_t *) _cw_pezz_get(&a_kasi->kasi_bufc_pezz);
  if (NULL == retval)
  {
    goto RETURN;
  }
  bufc_new(&retval->bufc, pezz_put, &a_kasi->kasi_bufc_pezz);
  bzero(retval->buffer, sizeof(retval->buffer));
  bufc_set_buffer(&retval->bufc, retval->buffer, _CW_KASI_BUFC_SIZE, TRUE,
		  NULL, NULL);

  RETURN:
  return retval;
}

const cw_kasin_t *
kasi_kasin_ref(cw_kasi_t * a_kasi, const cw_uint8_t * a_name, cw_uint32_t a_len,
	       cw_bool_t a_force, cw_bool_t a_is_static, const void * a_key,
	       const void * a_data)
{
  cw_kasin_t * retval;
  cw_kasin_t search_key, * data;
  
  _cw_check_ptr(a_kasi);
  _cw_assert(_CW_KASI_MAGIC == a_kasi->magic);
  _cw_check_ptr(a_name);
  _cw_assert(0 < a_len);

  /* Fake up a key. */
  kasink_init(&search_key.key, a_name, a_len);

  mtx_lock(&a_kasi->lock);

  /* Find the name. */
  if (FALSE == dch_search(&a_kasi->kasin_dch, &search_key, (void **) &data))
  {
    /* Add a keyed reference if necessary. */
    if (NULL != a_key)
    {
      mtx_lock(&data->lock);
      if (TRUE == kasi_p_kasin_kref(a_kasi, data, a_key, a_data))
      {
	mtx_unlock(&data->lock);
	retval = NULL;
	goto RETURN;
      }
      mtx_unlock(&data->lock);
    }

    data->ref_count++;
    retval = data;
  }
  else if (TRUE == a_force)
  {
    cw_uint8_t * name;
    
    /* The name doesn't exist, and the caller wants it created. */
    data = kasi_p_kasin_new(a_kasi);
    if (NULL == data)
    {
      retval = NULL;
      goto RETURN;
    }

    if (FALSE == a_is_static)
    {
      /* Copy the name string. */
      name = _cw_malloc(sizeof(cw_uint8_t) * a_len);
      if (NULL == name)
      {
	kasi_p_kasin_delete(a_kasi, data);
	retval = NULL;
	goto RETURN;
      }
      memcpy(name, a_name, a_len);
    }
    else
    {
      /* Cast away the const, though it gets picked right back up in the call to
       * kasink_init() below. */
      name = (cw_uint8_t *) a_name;
      data->is_static_name = TRUE;
    }

    /* Initialize the key. */
    kasink_init(&data->key, a_name, a_len);

    /* Increment the reference count for the caller. */
    data->ref_count++;

    /* Add a keyed reference if necessary.  We don't need to lock the kasin,
     * because it hasn't been inserted into the hash table yet, so no othre
     * threads can get to it. */
    if (NULL != a_key)
    {
      if (TRUE == kasi_p_kasin_kref(a_kasi, data, a_key, a_data))
      {
	if (FALSE == a_is_static)
	{
	  _cw_free(name);
	}
	kasi_p_kasin_delete(a_kasi, data);
	retval = NULL;
	goto RETURN;
      }
    }

    /* Finally, insert the kasin into the hash table. */
    if (TRUE == dch_insert(&a_kasi->kasin_dch, (void *) &data->key,
			   (void *) data))
    {
      if (NULL != data->keyed_refs)
      {
	dch_delete(data->keyed_refs);
      }
      if (FALSE == a_is_static)
      {
	_cw_free(name);
      }
      kasi_p_kasin_delete(a_kasi, data);
      retval = NULL;
      goto RETURN;
    }

    retval = data;
  }
  else
  {
    retval = NULL;
  }
  
  RETURN:
  mtx_unlock(&a_kasi->lock);
  return retval;
}

void
kasi_kasin_unref(cw_kasi_t * a_kasi, const cw_kasin_t * a_kasin,
		 const void * a_key)
{
  cw_kasin_t * kasin = (cw_kasin_t *) a_kasin;
  
  _cw_check_ptr(a_kasi);
  _cw_assert(_CW_KASI_MAGIC == a_kasi->magic);
  _cw_check_ptr(a_kasin);
  _cw_assert(_CW_KASIN_MAGIC == a_kasin->magic);

  mtx_lock(&a_kasi->lock);
  mtx_lock(&kasin->lock);
  
  /* Remove a keyed reference if necessary. */
  if (NULL != a_key)
  {
    _cw_check_ptr(kasin->keyed_refs);
    
    if (TRUE == dch_remove(kasin->keyed_refs, a_key, NULL, NULL))
    {
#ifdef _LIBKASI_DBG
      _cw_error("Trying to remove a non-existent keyed ref");
#endif
    }
    if (0 == dch_count(kasin->keyed_refs))
    {
      dch_delete(kasin->keyed_refs);
      kasin->keyed_refs = NULL;
    }
  }

  kasin->ref_count--;

  if (0 != kasin->ref_count)
  {
    mtx_unlock(&kasin->lock);
  }
  else
  {
    mtx_unlock(&kasin->lock);
    dch_remove(&a_kasi->kasin_dch, &kasin->key, NULL, NULL);
    kasi_p_kasin_delete(a_kasi, kasin);
  }

  mtx_unlock(&a_kasi->lock);
}

const cw_kasink_t *
kasin_get_kasink(const cw_kasin_t * a_kasin)
{
  _cw_check_ptr(a_kasin);
  _cw_assert(_CW_KASIN_MAGIC == a_kasin->magic);

  return &a_kasin->key;
}

void
kasink_init(cw_kasink_t * a_kasink, const cw_uint8_t * a_name,
	    cw_uint32_t a_len)
{
  _cw_check_ptr(a_kasink);

  a_kasink->name = a_name;
  a_kasink->len = a_len;
}

void
kasink_copy(cw_kasink_t * a_to, const cw_kasink_t * a_from)
{
  _cw_check_ptr(a_to);
  _cw_check_ptr(a_from);

  a_to->name = a_from->name;
  a_to->len = a_from->len;
}

const cw_uint8_t *
kasink_get_val(cw_kasink_t * a_kasink)
{
  _cw_check_ptr(a_kasink);

  return a_kasink->name;
}

cw_uint32_t
kasink_get_len(cw_kasink_t * a_kasink)
{
  _cw_check_ptr(a_kasink);

  return a_kasink->len;
}

cw_kasin_t *
kasi_p_kasin_new(cw_kasi_t * a_kasi)
{
  cw_kasin_t * retval;

  retval = _cw_pezz_get(&a_kasi->kasin_pezz);
  if (NULL == retval)
  {
    goto RETURN;
  }
  bzero(retval, sizeof(cw_kasin_t));

  mtx_new(&retval->lock);
#ifdef _LIBKASI_DBG
  retval->magic = _CW_KASIN_MAGIC;
#endif

  RETURN:
  return retval;
}

void
kasi_p_kasin_delete(cw_kasi_t * a_kasi, cw_kasin_t * a_kasin)
{
  _cw_check_ptr(a_kasin);
  _cw_assert(_CW_KASIN_MAGIC == a_kasin->magic);
#ifdef _LIBKASI_DBG
  if (FALSE == a_kasin->is_static_name)
  {
    _cw_out_put_e("Non-static name \"");
    _cw_out_put_n(a_kasin->key.len, "[s]", a_kasin->key.name);
    _cw_out_put("\" still exists with [i] reference[s]\n",
		a_kasin->ref_count, (1 == a_kasin->ref_count) ? "" : "s");
  }
  if (NULL != a_kasin->keyed_refs)
  {
    cw_uint32_t i;
    void * key;
    
    _cw_out_put_e("Name \"");
    _cw_out_put_n(a_kasin->key.len, "[s]", a_kasin->key.name);
    _cw_out_put("\" still exists with [i] keyed reference[s]:",
		dch_count(&a_kasi->kasin_dch),
		(1 == dch_count(&a_kasi->kasin_dch)) ? "" : "s");
    for (i = 0; i < dch_count(&a_kasi->kasin_dch); i++)
    {
      dch_get_iterate(&a_kasi->kasin_dch, &key, NULL);
      _cw_out_put(" 0x[p]", key);
    }
    _cw_out_put("\n");
  }
#endif

  mtx_delete(&a_kasin->lock);
  _cw_pezz_put(&a_kasi->kasin_pezz, a_kasin);
}

cw_bool_t
kasi_p_kasin_kref(cw_kasi_t * a_kasi, cw_kasin_t * a_kasin, const void * a_key,
		  const void * a_data)
{
  cw_bool_t retval, is_new_dch = FALSE;

  /* Assume that a_kasin is locked, or that no other threads can get to it. */

  if (NULL == a_kasin->keyed_refs)
  {
    is_new_dch = TRUE;

    /* XXX Magic numbers here. */
    a_kasin->keyed_refs = dch_new(NULL, 4, 3, 1, &a_kasi->chi_pezz,
				  ch_hash_direct, ch_key_comp_direct);
    if (NULL == a_kasin->keyed_refs)
    {
      retval = TRUE;
      goto RETURN;
    }
  }

  if (TRUE == dch_insert(a_kasin->keyed_refs, a_key, a_data))
  {
    if (TRUE == is_new_dch)
    {
      dch_delete(a_kasin->keyed_refs);
      a_kasin->keyed_refs = NULL;
    }
    retval = TRUE;
    goto RETURN;
  }

  retval = FALSE;

  RETURN:
  return retval;
}

static cw_uint32_t
kasink_p_hash(const void * a_key)
{
  cw_uint32_t retval, i;
  cw_kasink_t * key = (cw_kasink_t *) a_key;
  const char * str;

  _cw_check_ptr(a_key);

  for (i = 0, str = key->name, retval = 0; i < key->len; i++, str++)
  {
    retval = retval * 33 + *str;
  }

  return retval;
}

static cw_bool_t
kasink_p_key_comp(const void * a_k1, const void * a_k2)
{
  cw_kasink_t * k1 = (cw_kasink_t *) a_k1;
  cw_kasink_t * k2 = (cw_kasink_t *) a_k2;
  size_t len;

  _cw_check_ptr(a_k1);
  _cw_check_ptr(a_k2);
  
  if (k1->len > k2->len)
  {
    len = k1->len;
  }
  else
  {
    len = k2->len;
  }

  return strncmp((char *) k1->name, (char *) k2->name, len) ? FALSE : TRUE;
}
