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

/* Size of buffer used for large extended objects. */
#define _CW_KASI_BUFC_SIZE 256

typedef struct cw_kasi_s cw_kasi_t;
typedef struct cw_kasink_s cw_kasink_t;
/* Defined in kasio.h to avoid a circular dependency. */
#if (0)
typedef struct cw_kasin_s cw_kasin_t;
#endif

struct cw_kasi_s
{
#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic;
#endif
  
  cw_bool_t is_malloced;
  cw_pezz_t kasi_bufc_pezz;

  /* Hash of names (cw_kasink_t *) to (cw_kasin_t *).  This hash table keeps
   * track of *all* name "values" in the virtual machine.  When a name object is
   * created, it actually adds a reference to a kasin and uses a pointer to that
   * kasin as a unique key.  Note that each kasit maintains a cache of kasin's,
   * so that under normal circumstances, all objects in a kasit share a single
   * reference. */
  cw_dch_t names;
};

/* Not opaque. */
typedef struct
{
  cw_bufc_t bufc;
  cw_uint8_t buffer[_CW_KASI_BUFC_SIZE];
} cw_kasi_bufc_t;

struct cw_kasink_s
{
  /* If a name that is long enough to be stored as a buf is referenced, the
   * value of the buf is copied to a contiguous string, so that the
   * representation of the name is always a contiguous string.  In practice, it
   * would be ridiculous to have such a long name, but we do the right thing in
   * any case. */
  cw_uint8_t * name;
  cw_uint32_t len;
};

struct cw_kasin_s
{
#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic;
#endif

  cw_mtx_t lock;

  /* Number of simple references to this object. */
  cw_uint32_t ref_count;
  /* If non-NULL, a hash of keyed references to this object.  Keyed references
   * are used by global dictionary entries.  This allows a thread to determine
   * whether an entry exists in a particular global dictionary without having to
   * lock the entire dictionary. */
  cw_dch_t * keyed_refs;

  /* Key.  The value is merely a pointer to this kasin. */
  cw_kasink_t key;
};

cw_kasi_t *
kasi_new(cw_kasi_t * a_kasi);

void
kasi_delete(cw_kasi_t * a_kasi);

cw_kasi_bufc_t *
kasi_get_kasi_bufc(cw_kasi_t * a_kasi);

const cw_kasin_t *
kasin_ref(cw_kasi_t * a_kasi, const cw_uint8_t * a_name, cw_uint32_t a_len,
	  cw_bool_t a_force);

const cw_kasin_t *
kasin_kref(cw_kasi_t * a_kasi, const void * a_key,
	   const cw_uint8_t * a_name, cw_uint32_t a_len, cw_bool_t a_force);

void
kasin_unref(cw_kasi_t * a_kasi, const cw_kasin_t * a_kasin);

void
kasin_kunref(void * a_kasi, const cw_kasin_t * a_kasin, const void * a_key);

const cw_kasink_t *
kasin_get_kasink(const cw_kasin_t * a_kasin);

void
kasink_init(cw_kasink_t * a_kasink, const cw_uint8_t * a_name,
	    cw_uint32_t a_len);

void
kasink_copy(cw_kasink_t * a_to, const cw_kasink_t * a_from);
