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

typedef struct cw_tm_s cw_tm_t;

struct cw_tm_s
{
#ifdef CW_DBG
    cw_uint32_t magic;
#define CW_TM_MAGIC 0x928c888a
#endif
    cw_uint8_t *term;
};

cw_bool_t
tm_new(cw_tm_t *a_tm, const cw_uint8_t *a_term);

void
tm_delete(cw_tm_t *a_tm);

const char *
tm_term_get(cw_tm_t *a_tm);

//tm_pos_get(cw_tm_t *a_tm);

//tm_pos_set(cw_tm_t *a_tm);

cw_bool_t
tm_setup(cw_tm_t *a_tm, int a_in, int a_out);

cw_bool_t
tm_shutdown(cw_tm_t *a_tm, int a_in, int a_out);

cw_bool_t
tm_suspend(cw_tm_t *a_tm, int a_in, int a_out);

cw_bool_t
tm_restore(cw_tm_t *a_tm, int a_in, int a_out);

cw_sint32_t
tm_put_f(cw_tm_t *a_tm, int a_fd, cw_uint32_t a_attrs, const cw_uint8_t *a_str,
	 ...);

cw_sint32_t
tm_put_fv(cw_tm_t *a_tm, int a_fd, cw_uint32_t a_attrs, const cw_uint8_t *a_str,
	 va_list a_p);

cw_sint32_t
tm_put_s(cw_tm_t *a_tm, cw_uint8_t *a_str, cw_uint32_t a_attrs,
	 const cw_uint8_t *a_format, ...);

cw_sint32_t
tm_put_sv(cw_tm_t *a_tm, cw_uint8_t *a_str, cw_uint32_t a_attrs,
	  const cw_uint8_t *a_format, va_list a_p);

cw_sint32_t
tm_put_sn(cw_tm_t *a_tm, cw_uint8_t *a_str, cw_uint32_t a_size,
	  cw_uint32_t a_attrs, const cw_uint8_t *a_format, ...);

cw_sint32_t
tm_put_svn(cw_tm_t *a_tm, cw_uint8_t *a_str, cw_uint32_t a_size,
	   cw_uint32_t a_attrs, const cw_uint8_t *a_format, va_list a_p);

cw_sint32_t
tm_put_sa(cw_tm_t *a_tm, cw_uint8_t **r_str, cw_uint32_t a_attrs,
	  const cw_uint8_t *a_format, ...);

cw_sint32_t
tm_put_sva(cw_tm_t *a_tm, cw_uint8_t **r_str, cw_uint32_t a_attrs,
	   const cw_uint8_t *a_format, va_list a_p);

cw_bool_t
tm_flag_get(cw_tm_t *a_tm, const cw_uint8_t *a_capability);

cw_uint32_t
tm_num_get(cw_tm_t *a_tm, const cw_uint8_t *a_capability);

const cw_uint8_t *
tm_str_get(cw_tm_t *a_tm, const cw_uint8_t *a_capability);
