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

#define _CW_STILOE_DICT_ENTS2SIZEOF(e)        \
  (sizeof(cw_stiloe_dict_t) - sizeof(cw_ch_t) \
   + _CW_CH_TABLE2SIZEOF(5 / 4 * (e)))

/* Defined in stilo.h to resolve a circular dependency. */
#if (0)
typedef struct cw_stiloe_dict_s cw_stiloe_dict_t;
#endif
typedef struct cw_stiloe_dicto_s cw_stiloe_dicto_t;

struct cw_stiloe_dict_s
{
  cw_stiloe_t stiloe;

  /* stiloe_dicto's are allocated from here. */
  cw_pezz_t * stiloe_dicto_pezz;

  /* Name/value pairs.  The keys are (cw_stiloe_name_t *), and the values are
   * (cw_stiloe_dicto_t *).  The stilo from which the key is filched resides in
   * the stiloe_dicto structure.
   *
   * Must be the last field since the hash table is embedded and can vary in
   * size. */
  cw_ch_t hash;
};

struct cw_stiloe_dicto_s
{
#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
  cw_uint32_t magic;
#endif

  /* stiloe_dict this stiloe_dicto is contained in. */
  cw_stiloe_dict_t * stiloe_dict;
  
  cw_stilo_t name;
  cw_stilo_t value;
};

cw_stiloe_dict_t *
stiloe_dict_new(cw_stiloe_dict_t * a_stiloe_dict,
		cw_pezz_t * stiloe_dicto_pezz);

void
stiloe_dict_delete(cw_stiloe_dict_t * a_stiloe_dict);

cw_bool_t
stiloe_dict_def(cw_stiloe_dict_t * a_stiloe_dict, cw_stilo_t * a_name,
		cw_stilo_t * a_val);

cw_bool_t
stiloe_dict_undef(cw_stiloe_dict_t * a_stiloe_dict, cw_stilo_t * a_name);

cw_stilo_t *
stiloe_dict_lookup(cw_stiloe_dict_t * a_stiloe_dict, cw_stilo_t * a_name);

cw_bool_t
stiloe_dict_copy(cw_stiloe_dict_t * a_from, cw_stiloe_dict_t * a_to);

cw_uint32_t
stiloe_dict_count(cw_stiloe_dict_t * a_stiloe_dict);

cw_stilo_t *
stiloe_dict_iterate(cw_stiloe_dict_t * a_stiloe_dict);
