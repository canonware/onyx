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

#define _CW_KASIOE_DICT_ENTS2SIZEOF(e)        \
  (sizeof(cw_kasioe_dict_t) - sizeof(cw_ch_t) \
   + _CW_CH_TABLE2SIZEOF(5 / 4 * (e)))

/* Defined in kasio.h to resolve a circular dependency. */
#if (0)
typedef struct cw_kasioe_dict_s cw_kasioe_dict_t;
#endif
typedef struct cw_kasioe_dicto_s cw_kasioe_dicto_t;

struct cw_kasioe_dict_s
{
  cw_kasioe_t kasioe;

  /* kasioe_dicto's are allocated from here. */
  cw_pezz_t * kasioe_dicto_pezz;

  /* Name/value pairs.  The keys are (cw_kasioe_name_t *), and the values are
   * (cw_kasioe_dicto_t *).  The kasio from which the key is filched resides in
   * the kasioe_dicto structure.
   *
   * Must be the last field since the hash table is embedded and can vary in
   * size. */
  cw_ch_t hash;
};

struct cw_kasioe_dicto_s
{
#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic;
#endif

  /* kasioe_dict this kasioe_dicto is contained in. */
  cw_kasioe_dict_t * kasioe_dict;
  
  cw_kasio_t name;
  cw_kasio_t value;
};

cw_kasioe_dict_t *
kasioe_dict_new(cw_kasioe_dict_t * a_kasioe_dict,
		cw_pezz_t * kasioe_dicto_pezz);

void
kasioe_dict_delete(cw_kasioe_dict_t * a_kasioe_dict);

cw_bool_t
kasioe_dict_def(cw_kasioe_dict_t * a_kasioe_dict, cw_kasio_t * a_name,
		cw_kasio_t * a_val);

cw_bool_t
kasioe_dict_undef(cw_kasioe_dict_t * a_kasioe_dict, cw_kasio_t * a_name);

cw_kasio_t *
kasioe_dict_lookup(cw_kasioe_dict_t * a_kasioe_dict, cw_kasio_t * a_name);

cw_bool_t
kasioe_dict_copy(cw_kasioe_dict_t * a_from, cw_kasioe_dict_t * a_to);

cw_uint32_t
kasioe_dict_count(cw_kasioe_dict_t * a_kasioe_dict);

cw_kasio_t *
kasioe_dict_iterate(cw_kasioe_dict_t * a_kasioe_dict);
