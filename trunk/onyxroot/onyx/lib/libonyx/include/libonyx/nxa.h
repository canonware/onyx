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

/* Global variables. */
extern cw_mema_t *cw_g_nxaa;

void *
nxa_malloc_e(void *a_arg, size_t a_size, const char *a_filename,
	     cw_uint32_t a_line_num);

void *
nxa_calloc_e(void *a_arg, size_t a_number, size_t a_size,
	     const char *a_filename, cw_uint32_t a_line_num);

void *
nxa_realloc_e(void *a_arg, void *a_ptr, size_t a_size, size_t
	      a_old_size, const char *a_filename, cw_uint32_t a_line_num);

void
nxa_free_e(void *a_arg, void *a_ptr, size_t a_size, const char *a_filename,
	   cw_uint32_t a_line_num);

#ifdef CW_DBG
#define nxa_malloc(a_size)						\
    nxa_malloc_e(NULL, (a_size), __FILE__, __LINE__)
#define nxa_calloc(a_num, a_size)					\
    nxa_calloc_e(NULL, (a_num), (a_size), __FILE__, __LINE__)
#define nxa_realloc(a_ptr, a_size, a_old_size)				\
    nxa_realloc_e(NULL, (a_ptr), (a_size), (a_old_size), __FILE__, __LINE__)
#define nxa_free(a_ptr, a_size)						\
    nxa_free_e(NULL, (a_ptr), (a_size), __FILE__, __LINE__)
#else
#define nxa_malloc(a_size)						\
    nxa_malloc_e(NULL, (a_size), NULL, 0)
#define nxa_calloc(a_num, a_size)					\
    nxa_calloc_e(NULL, (a_num), (a_size), NULL, 0)
#define nxa_realloc(a_ptr, a_size, a_old_size)				\
    nxa_realloc_e(NULL, (a_ptr), (a_size), (a_old_size), NULL, 0)
#define nxa_free(a_ptr, a_size)						\
    nxa_free_e(NULL, (a_ptr), (a_size), NULL, 0)
#endif

void
nxa_collect(void);

cw_bool_t
nxa_active_get(void);

void
nxa_active_set(cw_bool_t a_active);

#ifdef CW_PTHREADS
cw_nxoi_t
nxa_period_get(void);

void
nxa_period_set(cw_nxoi_t a_period);
#endif
cw_nxoi_t
nxa_threshold_get(void);

void
nxa_threshold_set(cw_nxoi_t a_threshold);

void
nxa_stats_get(cw_nxoi_t *r_collections, cw_nxoi_t *r_count,
	      cw_nxoi_t *r_ccount, cw_nxoi_t *r_cmark, cw_nxoi_t *r_csweep,
	      cw_nxoi_t *r_mcount, cw_nxoi_t *r_mmark, cw_nxoi_t *r_msweep,
	      cw_nxoi_t *r_scount, cw_nxoi_t *r_smark, cw_nxoi_t *r_ssweep);
