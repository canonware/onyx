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
 * <<< Description >>>
 *
 * Public interface for the out (formatted output) class.
 *
 ****************************************************************************/

/* Typedef's to allow easy function pointer passing. */
typedef cw_uint32_t cw_out_metric_t(const char *, cw_uint32_t, const void *);
typedef char * cw_out_render_t(const char *, cw_uint32_t, const void *, char *);

typedef struct cw_out_s cw_out_t;
typedef struct cw_out_ent_s cw_out_ent_t;

struct cw_out_s
{
#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
  cw_uint32_t magic;
#endif

  cw_bool_t is_malloced;
  cw_sint32_t fd;

#ifdef _CW_REENTRANT
  cw_mtx_t lock;
#endif

  cw_uint32_t nextensions;
  cw_out_ent_t * extensions;
};

struct cw_out_ent_s
{
  char * type;
  cw_uint32_t size;
  cw_out_metric_t * metric_func;
  cw_out_render_t * render_func;
};

cw_out_t *
out_new(cw_out_t * a_out);

void
out_delete(cw_out_t * a_out);

cw_bool_t
out_register(cw_out_t * a_out,
	     const char * a_type,
	     cw_uint32_t a_size,
	     cw_out_metric_t * a_metric_func,
	     cw_out_render_t * a_render_func);

cw_sint32_t 
out_get_default_fd(cw_out_t * a_out);

void
out_set_default_fd(cw_out_t * a_out, cw_sint32_t a_fd);

cw_sint32_t
out_put(cw_out_t * a_out, const char * a_format, ...);

cw_sint32_t
out_put_e(cw_out_t * a_out,
	  const char * a_file_name,
	  cw_uint32_t a_line_num,
	  const char * a_func_name,
	  const char * a_format,
	  ...);

cw_sint32_t
out_put_f(cw_out_t * a_out, cw_sint32_t a_fd, const char * a_format, ...);

cw_sint32_t
out_put_fe(cw_out_t * a_out, cw_sint32_t a_fd,
	   const char * a_file_name,
	   cw_uint32_t a_line_num,
	   const char * a_func_name,
	   const char * a_format,
	   ...);

cw_sint32_t
out_put_fle(cw_out_t * a_out, cw_sint32_t a_fd,
	    const char * a_file_name,
	    cw_uint32_t a_line_num,
	    const char * a_func_name,
	    const char * a_format,
	    ...);

cw_sint32_t
out_put_fn(cw_out_t * a_out, cw_sint32_t a_fd, cw_uint32_t a_size,
	   const char * a_format, ...);

cw_sint32_t
out_put_fv(cw_out_t * a_out, cw_sint32_t a_fd,
	   const char * a_format, va_list a_p);

cw_sint32_t
out_put_s(cw_out_t * a_out, char * a_str, const char * a_format, ...);

cw_sint32_t
out_put_sa(cw_out_t * a_out, char ** r_str, const char * a_format, ...);

cw_sint32_t
out_put_sn(cw_out_t * a_out, char * a_str, cw_uint32_t a_size,
	   const char * a_format, ...);

cw_sint32_t
out_put_sv(cw_out_t * a_out, char * a_str,
	   const char * a_format, va_list a_p);

cw_sint32_t
out_put_sva(cw_out_t * a_out, char ** r_str,
	    const char * a_format, va_list a_p);

cw_sint32_t
out_put_svn(cw_out_t * a_out, char * a_str, cw_uint32_t a_size,
	    const char * a_format, va_list a_p);

/* spec */
cw_sint32_t
spec_get_type(const char * a_spec, cw_uint32_t a_spec_len, const char ** r_val);

cw_sint32_t
spec_get_val(const char * a_spec, cw_uint32_t a_spec_len,
	     const char * a_name, const char ** r_val);

