/******************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

typedef struct cw_stil_s cw_stil_t;
typedef struct cw_stilnk_s cw_stilnk_t;

struct cw_stil_s {
#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
	cw_uint32_t	magic;
#endif

	cw_bool_t	is_malloced;
	cw_mtx_t	lock;

	/* Global allocator. */
	cw_stilag_t	stilag;

	/* Global name cache. */
	cw_stilng_t	stilng;

	cw_stilo_t	systemdict;
};

/* Not opaque. */
struct cw_stil_bufc_s {
	cw_bufc_t	bufc;
/* Size of buffer used for large extended objects. */
#define _CW_STIL_BUFC_SIZE 256
	cw_uint8_t	buffer[_CW_STIL_BUFC_SIZE];
};

/* stil. */
cw_stil_t	*stil_new(cw_stil_t *a_stil);
void		stil_delete(cw_stil_t *a_stil);

cw_stil_bufc_t	*stil_stil_bufc_get(cw_stil_t *a_stil);

#define stil_stilag_get(a_stil) (&(a_stil)->stilag)
#define stil_stilng_get(a_stil) (&(a_stil)->stilng)
#define stil_systemdict_get(a_stil) (&(a_stil)->systemdict)
