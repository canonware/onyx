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

#include "../include/libstash/libstash.h"

#ifdef _LIBSTASH_DBG
#  define _CW_CH_MAGIC 0x574936af
#endif

cw_ch_t *
ch_new(cw_ch_t * a_ch, cw_uint32_t a_table_size, cw_pezz_t * a_chi_pezz,
       ch_hash_t * a_hash, ch_key_comp_t * a_key_comp)
{
  cw_ch_t * retval;

  _cw_assert(0 < a_table_size);

  if (NULL != a_ch)
  {
    retval = a_ch;
    bzero(retval, _CW_CH_TABLE2SIZEOF(a_table_size));
    retval->is_malloced = FALSE;
  }
  else
  {
    retval = (cw_ch_t *) _cw_malloc(_CW_CH_TABLE2SIZEOF(a_table_size));
    if (NULL == retval)
    {
      goto RETURN;
    }
    bzero(retval, _CW_CH_TABLE2SIZEOF(a_table_size));
    retval->is_malloced = TRUE;
  }

  retval->table_size = a_table_size;
  retval->chi_pezz = a_chi_pezz;
#ifdef _LIBSTASH_DBG
  if (NULL != retval->chi_pezz)
  {
    _cw_assert(sizeof(cw_chi_t) <= pezz_get_buffer_size(retval->chi_pezz));
  }
#endif
  retval->hash = a_hash;
  retval->key_comp = a_key_comp;

#ifdef _LIBSTASH_DBG
  retval->magic = _CW_CH_MAGIC;
#endif
  
  RETURN:
  return retval;
}

void
ch_delete(cw_ch_t * a_ch)
{
  cw_ring_t * t_ring;
  cw_chi_t * chi;
  
  _cw_check_ptr(a_ch);
  _cw_assert(_CW_CH_MAGIC == a_ch->magic);

  if (NULL != a_ch->chi_ring)
  {
    if (NULL != a_ch->chi_pezz)
    {
      do
      {
	t_ring = a_ch->chi_ring;
	a_ch->chi_ring = ring_cut(t_ring);
	chi = ring_get_data(t_ring);
	_cw_pezz_put(a_ch->chi_pezz, chi);
      } while (a_ch->chi_ring != t_ring);
    }
    else
    {
      do
      {
	t_ring = a_ch->chi_ring;
	a_ch->chi_ring = ring_cut(t_ring);
	chi = ring_get_data(t_ring);
	_cw_free(chi);
      } while (a_ch->chi_ring != t_ring);
    }
  }
  
  if (TRUE == a_ch->is_malloced)
  {
    _cw_free(a_ch);
  }
#ifdef _LIBSTASH_DBG
  else
  {
    memset(a_ch, 0x5a, _CW_CH_TABLE2SIZEOF(a_ch->table_size));
  }
#endif
}

cw_uint32_t
ch_count(cw_ch_t * a_ch)
{
  _cw_check_ptr(a_ch);
  _cw_assert(_CW_CH_MAGIC == a_ch->magic);
  
  return a_ch->count;
}

cw_bool_t
ch_insert(cw_ch_t * a_ch, const void * a_key, const void * a_data)
{
  cw_bool_t retval;
  cw_uint32_t slot;
  cw_chi_t * chi;
  
  _cw_check_ptr(a_ch);
  _cw_assert(_CW_CH_MAGIC == a_ch->magic);

  /* Initialize chi. */
  if (NULL != a_ch->chi_pezz)
  {
    chi = (cw_chi_t *) _cw_pezz_get(a_ch->chi_pezz);
  }
  else
  {
    chi = (cw_chi_t *) _cw_malloc(sizeof(cw_chi_t));
  }
  if (NULL == chi)
  {
    retval = TRUE;
    goto RETURN;
  }
  chi->key = a_key;
  chi->data = a_data;
  ring_new(&chi->ch_link);
  ring_set_data(&chi->ch_link, chi);
  ring_new(&chi->slot_link);
  ring_set_data(&chi->slot_link, chi);
  slot = a_ch->hash(a_key) % a_ch->table_size;
  chi->slot = slot;

  /* Hook into ch-wide ring.. */
  if (NULL != a_ch->chi_ring)
  {
    ring_meld(a_ch->chi_ring, &chi->ch_link);
  }
  else
  {
    a_ch->chi_ring = &chi->ch_link;
  }

  if (NULL != a_ch->table[slot])
  {
    /* Other chi's in this slot already.  Put this one at the head, in order to
     * implement LIFO ordering for multiple chi's with the same key. */
    ring_meld(&chi->slot_link, a_ch->table[slot]);

#ifdef _LIBSTASH_DBG
    a_ch->num_collisions++;
#endif
  }
  a_ch->table[slot] = &chi->slot_link;

  a_ch->count++;
#ifdef _LIBSTASH_DBG
  a_ch->num_inserts++;
#endif
  
  retval = FALSE;
  RETURN:
  return retval;
}

cw_bool_t
ch_remove(cw_ch_t * a_ch, const void * a_search_key, void ** r_key,
	  void ** r_data)
{
  cw_bool_t retval;
  cw_uint32_t slot;
  cw_chi_t * chi;
  cw_ring_t * t_ring;
  
  _cw_check_ptr(a_ch);
  _cw_assert(_CW_CH_MAGIC == a_ch->magic);
  
  slot = a_ch->hash(a_search_key) % a_ch->table_size;

  if (NULL == a_ch->table[slot])
  {
    retval = TRUE;
    goto RETURN;
  }

  t_ring = a_ch->table[slot];
  do
  {
    chi = (cw_chi_t *) ring_get_data(t_ring);

    /* Is this the chi we want? */
    if (a_search_key == chi->key)
    {
      /* Detach from ch-wide ring. */
      if (a_ch->chi_ring == &chi->ch_link)
      {
	a_ch->chi_ring = ring_next(a_ch->chi_ring);
	if (a_ch->chi_ring == &chi->ch_link)
	{
	  a_ch->chi_ring = NULL;
	}
      }
      ring_cut(&chi->ch_link);
      
      /* Detach from the slot ring. */
      if (a_ch->table[slot] == t_ring)
      {
	a_ch->table[slot] = ring_next(a_ch->table[slot]);
	if (a_ch->table[slot] == t_ring)
	{
	  a_ch->table[slot] = NULL;
	}
      }
      ring_cut(&chi->slot_link);

      if (NULL != r_key)
      {
	*r_key = (void *) chi->key;
      }
      if (NULL != r_data)
      {
	*r_data = (void *) chi->data;
      }

      /* Deallocate the chi. */
      if (NULL != a_ch->chi_pezz)
      {
	_cw_pezz_put(a_ch->chi_pezz, chi);
      }
      else
      {
	_cw_free(chi);
      }

      a_ch->count--;
#ifdef _LIBSTASH_DBG
      a_ch->num_deletes++;
#endif
      retval = FALSE;
      goto RETURN;
    }
    
    t_ring = ring_next(t_ring);
  } while (t_ring != a_ch->table[slot]);

  retval = TRUE;
  RETURN:
  return retval;
}

cw_bool_t
ch_search(cw_ch_t * a_ch, const void * a_key, void ** r_data)
{
  cw_bool_t retval;
  cw_uint32_t slot;
  cw_chi_t * chi;
  cw_ring_t * t_ring;
  
  _cw_check_ptr(a_ch);
  _cw_assert(_CW_CH_MAGIC == a_ch->magic);
  
  slot = a_ch->hash(a_key) % a_ch->table_size;

  if (NULL == a_ch->table[slot])
  {
    retval = TRUE;
    goto RETURN;
  }

  t_ring = a_ch->table[slot];
  do
  {
    chi = (cw_chi_t *) ring_get_data(t_ring);

    /* Is this the chi we want? */
    if (a_key == chi->key)
    {
      if (NULL != r_data)
      {
	*r_data = (void *) chi->data;
      }

      retval = FALSE;
      goto RETURN;
    }
    
    t_ring = ring_next(t_ring);
  } while (t_ring != a_ch->table[slot]);

  retval = TRUE;
  RETURN:
  return retval;
}

cw_bool_t
ch_get_iterate(cw_ch_t * a_ch, void ** r_key, void ** r_data)
{
  cw_bool_t retval;
  cw_chi_t * chi;
  
  _cw_check_ptr(a_ch);
  _cw_assert(_CW_CH_MAGIC == a_ch->magic);
  
  if (NULL == a_ch->chi_ring)
  {
    retval = TRUE;
    goto RETURN;
  }

  chi = (cw_chi_t *) ring_get_data(a_ch->chi_ring);
  if (NULL != r_key)
  {
    *r_key = (void *) chi->key;
  }
  if (NULL != r_data)
  {
    *r_data = (void *) chi->data;
  }

  a_ch->chi_ring = ring_next(a_ch->chi_ring);

  retval = FALSE;
  RETURN:
  return retval;
}

cw_bool_t
ch_remove_iterate(cw_ch_t * a_ch, void ** r_key, void ** r_data)
{
  cw_bool_t retval;
  cw_ring_t * t_ring;
  cw_chi_t * chi;
  
  _cw_check_ptr(a_ch);
  _cw_assert(_CW_CH_MAGIC == a_ch->magic);
  
  if (NULL == a_ch->chi_ring)
  {
    retval = TRUE;
    goto RETURN;
  }

  chi = (cw_chi_t *) ring_get_data(a_ch->chi_ring);
  if (NULL != r_key)
  {
    *r_key = (void *) chi->key;
  }
  if (NULL != r_data)
  {
    *r_data = (void *) chi->data;
  }
  
  /* Detach from the ch-wide ring. */
  t_ring = a_ch->chi_ring;
  a_ch->chi_ring = ring_cut(t_ring);
  if (t_ring == a_ch->chi_ring)
  {
    a_ch->chi_ring = NULL;
  }

  /* Detach from the slot ring. */
  t_ring = &chi->slot_link;
  if (t_ring == a_ch->table[chi->slot])
  {
    a_ch->table[chi->slot] = ring_next(a_ch->table[chi->slot]);
    if (t_ring == a_ch->table[chi->slot])
    {
      a_ch->table[chi->slot] = NULL;
    }
  }
  ring_cut(t_ring);

  /* Deallocate the chi. */
  if (NULL != a_ch->chi_pezz)
  {
    _cw_pezz_put(a_ch->chi_pezz, chi);
  }
  else
  {
    _cw_free(chi);
  }

  a_ch->count--;
#ifdef _LIBSTASH_DBG
  a_ch->num_deletes++;
#endif

  retval = FALSE;
  RETURN:
  return retval;
}

void
ch_dump(cw_ch_t * a_ch, const char * a_prefix)
{
  cw_uint32_t i;
  cw_ring_t * t_ring;
  cw_chi_t * chi;
  
  _cw_check_ptr(a_ch);
  _cw_assert(_CW_CH_MAGIC == a_ch->magic);
  _cw_check_ptr(a_prefix);

#ifdef _LIBSTASH_DBG
  _cw_out_put("[s]: num_collisions: [i], num_inserts: [i], num_deletes: [i]\n",
	      a_prefix, a_ch->num_collisions, a_ch->num_inserts,
	      a_ch->num_deletes);
#endif

  _cw_out_put("[s]: is_malloced: [s]\n",
	  a_prefix, (a_ch->is_malloced) ? "TRUE" : "FALSE");
  _cw_out_put("[s]: chi_ring: 0x[p]\n",
	  a_prefix, a_ch->chi_ring);
  _cw_out_put("[s]: count: [i], table_size: [i]\n",
	  a_prefix, a_ch->count, a_ch->table_size);

  /* Table. */
  _cw_out_put(
    "[s]: table ------------------------------------------------------------\n",
    a_prefix);

  for (i = 0; i < a_ch->table_size; i++)
  {
    if (NULL != a_ch->table[i])
    {
      t_ring = a_ch->table[i];

      do
      {
	chi = (cw_chi_t *) ring_get_data(t_ring);
	_cw_out_put("[s]: [i]: key: 0x[p], data: 0x[p], slot: [i]\n",
		a_prefix, i, chi->key, chi->data, chi->slot);
	t_ring = ring_next(t_ring);
      } while (a_ch->table[i] != t_ring);
    }
    else
    {
      _cw_out_put("[s]: [i]: NULL\n", a_prefix, i);
    }
  }
  
  /* chi ring. */
  _cw_out_put(
    "[s]: chi_ring ---------------------------------------------------------\n",
	  a_prefix);
  if (NULL != a_ch->chi_ring)
  {
    t_ring = a_ch->chi_ring;
    do
    {
      chi = (cw_chi_t *) ring_get_data(t_ring);
      _cw_out_put("[s]: key: 0x[p], data: 0x[p], slot: [i]\n",
		  a_prefix, chi->key, chi->data, chi->slot);
      t_ring = ring_next(t_ring);
    } while (a_ch->chi_ring != t_ring);
  }
  else
  {
    _cw_out_put("[s]: Empty\n", a_prefix);
  }
}

cw_uint32_t
ch_hash_string(const void * a_key)
{
  cw_uint32_t retval;
  char * str;

  _cw_check_ptr(a_key);

  for (str = (char *) a_key, retval = 0; *str != 0; str++)
  {
    retval = retval * 33 + *str;
  }

  return retval;
}

cw_uint32_t
ch_hash_direct(const void * a_key)
{
  cw_uint32_t retval;
  cw_uint32_t i;

  retval = (cw_uint32_t) a_key;

  /* Shift right until we've shifted one 1 bit off. */
  for (i = 0; i < 8 * sizeof(void *); i++)
  {
    if ((retval & 0x1) == 1)
    {
      retval >>= 1;
      break;
    }
    else
    {
      retval >>= 1;
    }
  }

  return retval;
}

cw_bool_t
ch_key_comp_string(const void * a_k1, const void * a_k2)
{
  _cw_check_ptr(a_k1);
  _cw_check_ptr(a_k2);
  
  return strcmp((char *) a_k1, (char *) a_k2) ? FALSE : TRUE;
}

cw_bool_t
ch_key_comp_direct(const void * a_k1, const void * a_k2)
{
  return (a_k1 == a_k2) ? TRUE : FALSE;
}
