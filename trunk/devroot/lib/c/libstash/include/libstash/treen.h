/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 ****************************************************************************/

/* Pseudo-opaque type. */
typedef struct cw_treen_s cw_treen_t;

struct cw_treen_s
{
#ifdef _CW_REENTRANT
  cw_bool_t is_thread_safe;
  cw_mtx_t lock;
#endif
  void * data;
  cw_uint32_t num_children;
  cw_treen_t ** children;
};

cw_treen_t *
treen_new(void);
#ifdef _CW_REENTRANT
cw_treen_t *
treen_new_r(void);
#endif

void
treen_delete(cw_treen_t * a_treen);

cw_uint32_t
treen_get_num_children(cw_treen_t * a_treen);

cw_bool_t
treen_link_child(cw_treen_t * a_treen, cw_treen_t * a_child,
		 cw_uint32_t a_position);

cw_bool_t
treen_unlink_child(cw_treen_t * a_treen, cw_uint32_t a_position,
		   cw_treen_t ** r_child);

cw_bool_t
treen_get_child_ptr(cw_treen_t * a_treen, cw_uint32_t a_position,
		    cw_treen_t ** r_child);

void *
treen_get_data_ptr(cw_treen_t * a_treen);

void *
treen_set_data_ptr(cw_treen_t * a_treen, void * a_data);
