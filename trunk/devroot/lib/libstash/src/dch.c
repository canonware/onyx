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
#  define _CW_DCH_MAGIC 0x4327589e
#endif

cw_bool_t
dch_p_grow(cw_dch_t * a_dch);
cw_bool_t
dch_p_shrink(cw_dch_t * a_dch);

cw_dch_t *
dch_new(cw_dch_t * a_dch, cw_uint32_t a_base_table,
	cw_uint32_t a_base_grow, cw_uint32_t a_base_shrink,
	cw_pezz_t * a_chi_pezz,
	cw_ch_hash_t * a_hash, cw_ch_key_comp_t * a_key_comp)
{
  cw_dch_t * retval;

  _cw_assert(0 < a_base_table);
  _cw_assert(0 < a_base_grow);
  _cw_assert(a_base_shrink < a_base_grow);

  if (NULL != a_dch)
  {
    retval = a_dch;
    bzero(retval, sizeof(cw_dch_t));
    retval->is_malloced = FALSE;
  }
  else
  {
    retval = (cw_dch_t *) _cw_malloc(sizeof(cw_dch_t));
    if (NULL == retval)
    {
      goto RETURN;
    }
    bzero(retval, sizeof(cw_dch_t));
    retval->is_malloced = TRUE;
  }

  retval->base_table = a_base_table;
  retval->base_grow = a_base_grow;
  retval->base_shrink = a_base_shrink;
  retval->grow_factor = 1;
  retval->chi_pezz = a_chi_pezz;
  retval->hash = a_hash;
  retval->key_comp = a_key_comp;

  retval->ch = ch_new(NULL, retval->base_table, retval->chi_pezz,
		      retval->hash, retval->key_comp);
  if (NULL == retval)
  {
    if (TRUE == a_dch->is_malloced)
    {
      _cw_free(a_dch);
    }
    goto RETURN;
  }

#ifdef _LIBSTASH_DBG
  retval->magic = _CW_DCH_MAGIC;
#endif
  
  RETURN:
  return retval;
}

void
dch_delete(cw_dch_t * a_dch)
{
  _cw_check_ptr(a_dch);
  _cw_assert(_CW_DCH_MAGIC == a_dch->magic);

  ch_delete(a_dch->ch);
  
  if (TRUE == a_dch->is_malloced)
  {
    _cw_free(a_dch);
  }
#ifdef _LIBSTASH_DBG
  else
  {
    memset(a_dch, 0x5a, sizeof(cw_dch_t));
  }
#endif
}

cw_uint32_t
dch_count(cw_dch_t * a_dch)
{
  _cw_check_ptr(a_dch);
  _cw_assert(_CW_DCH_MAGIC == a_dch->magic);
  
  return ch_count(a_dch->ch);
}

cw_bool_t
dch_insert(cw_dch_t * a_dch, const void * a_key, const void * a_data)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_dch);
  _cw_assert(_CW_DCH_MAGIC == a_dch->magic);

  if (TRUE == dch_p_grow(a_dch))
  {
    retval = TRUE;
    goto RETURN;
  }
  if (TRUE == ch_insert(a_dch->ch, a_key, a_data))
  {
    retval = TRUE;
    goto RETURN;
  }
  
  retval = FALSE;
  RETURN:
  return retval;
}

cw_bool_t
dch_remove(cw_dch_t * a_dch, const void * a_search_key, void ** r_key,
	   void ** r_data)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_dch);
  _cw_assert(_CW_DCH_MAGIC == a_dch->magic);
  
  if (TRUE == dch_p_shrink(a_dch))
  {
    retval = TRUE;
    goto RETURN;
  }
  if (TRUE == ch_remove(a_dch->ch, a_search_key, r_key, r_data))
  {
    retval = TRUE;
    goto RETURN;
  }
  
  retval = FALSE;
  RETURN:
  return retval;
}

cw_bool_t
dch_search(cw_dch_t * a_dch, const void * a_key, void ** r_data)
{
  _cw_check_ptr(a_dch);
  _cw_assert(_CW_DCH_MAGIC == a_dch->magic);

  return ch_search(a_dch->ch, a_key, r_data);
}

cw_bool_t
dch_get_iterate(cw_dch_t * a_dch, void ** r_key, void ** r_data)
{
  _cw_check_ptr(a_dch);
  _cw_assert(_CW_DCH_MAGIC == a_dch->magic);

  return ch_get_iterate(a_dch->ch, r_key, r_data);
}

cw_bool_t
dch_remove_iterate(cw_dch_t * a_dch, void ** r_key, void ** r_data)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_dch);
  _cw_assert(_CW_DCH_MAGIC == a_dch->magic);

  if (TRUE == dch_p_shrink(a_dch))
  {
    retval = TRUE;
    goto RETURN;
  }
  if (TRUE == ch_remove_iterate(a_dch->ch, r_key, r_data))
  {
    retval = TRUE;
    goto RETURN;
  }

  retval = FALSE;
  RETURN:
  return retval;
}

void
dch_dump(cw_dch_t * a_dch, const char * a_prefix)
{
  _cw_check_ptr(a_dch);
  _cw_assert(_CW_DCH_MAGIC == a_dch->magic);
  _cw_check_ptr(a_prefix);

#ifdef _LIBSTASH_DBG
  _cw_out_put("[s]: num_grows: [i], num_shrinks: [i]\n",
	      a_prefix, a_dch->num_grows, a_dch->num_shrinks);
#endif
  _cw_out_put("[s]: is_malloced: [s]\n",
	  a_prefix, (a_dch->is_malloced) ? "TRUE" : "FALSE");
  _cw_out_put("[s]: base_table: [i], base_grow: [i], base_shrink: [i]\n",
	      a_prefix, a_dch->base_table, a_dch->base_grow,
	      a_dch->base_shrink);
  
  ch_dump(a_dch->ch, a_prefix);
}

cw_bool_t
dch_p_grow(cw_dch_t * a_dch)
{
  cw_bool_t retval = FALSE;
  cw_ch_t * t_ch;
  cw_uint32_t count, i;
  void * key, * data;

  count = ch_count(a_dch->ch);
  
  if ((a_dch->grow_factor * a_dch->base_grow) < (count + 1))
  {
    /* Too big.  Create a new ch twice as large and populate it.  Unfortunately,
     * we have to completely build the new ch before deleting anything from the
     * old, in case there is a memory allocation error.  Even if a memory
     * allocation error occurs, allow the loop to run to completion in order to
     * restore a_dch->ch's internal ring to its original state. */

    t_ch = ch_new(NULL, a_dch->base_table * a_dch->grow_factor * 2,
		  a_dch->chi_pezz, a_dch->hash, a_dch->key_comp);
    if (NULL == t_ch)
    {
      retval = TRUE;
      goto RETURN;
    }
    
    for (i = 0; i < count; i++)
    {
      ch_get_iterate(a_dch->ch, &key, &data);

      if (FALSE == retval)
      {
	retval = ch_insert(t_ch, key, data);
      }
    }

    if (FALSE == retval)
    {
      a_dch->grow_factor *= 2;
#ifdef _LIBSTASH_DBG
      a_dch->num_grows++;
      /* Gross violation of data hiding, but I don't want to expose the counters
       * via an API. */
      t_ch->num_collisions += a_dch->ch->num_collisions;
      t_ch->num_inserts += a_dch->ch->num_inserts;
      t_ch->num_deletes += a_dch->ch->num_deletes;
#endif
      ch_delete(a_dch->ch);
      a_dch->ch = t_ch;
    }
    else
    {
      ch_delete(t_ch);
    }
  }

  RETURN:
  return retval;
}

cw_bool_t
dch_p_shrink(cw_dch_t * a_dch)
{
  cw_bool_t retval = FALSE;
  cw_ch_t * t_ch;
  cw_uint32_t count, i;
  void * key, * data;

  count = ch_count(a_dch->ch);
  
  if ((a_dch->grow_factor > 1)
      && ((a_dch->grow_factor * a_dch->base_shrink) > (count - 1)))
  {
    /* Too big.  Create a new ch twice as large and populate it.  Unfortunately,
     * we have to completely build the new ch before deleting anything from the
     * old, in case there is a memory allocation error.  Even if a memory
     * allocation error occurs, allow the loop to run to completion in order to
     * restore a_dch->ch's internal ring to its original state. */
    t_ch = ch_new(NULL, a_dch->base_table * a_dch->grow_factor / 2,
		  a_dch->chi_pezz, a_dch->hash, a_dch->key_comp);
    if (NULL == t_ch)
    {
      retval = TRUE;
      goto RETURN;
    }
    
    for (i = 0; i < count; i++)
    {
      ch_get_iterate(a_dch->ch, &key, &data);
      if (FALSE == retval)
      {
	retval = ch_insert(t_ch, key, data);
      }
    }

    if (FALSE == retval)
    {
      a_dch->grow_factor /= 2;
#ifdef _LIBSTASH_DBG
      a_dch->num_shrinks++;
      /* Gross violation of data hiding, but I don't want to expose the counters
       * via an API. */
      t_ch->num_collisions += a_dch->ch->num_collisions;
      t_ch->num_inserts += a_dch->ch->num_inserts;
      t_ch->num_deletes += a_dch->ch->num_deletes;
#endif
      ch_delete(a_dch->ch);
      a_dch->ch = t_ch;
    }
    else
    {
      ch_delete(t_ch);
    }
  }

  RETURN:
  return retval;
}
