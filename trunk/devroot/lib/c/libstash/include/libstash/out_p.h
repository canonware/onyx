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
 *
 *
 ****************************************************************************/

#ifdef _LIBSTASH_DBG
#  define _LIBSTASH_OUT_MAGIC 0x8293cade
#endif

/* Designator values.  These codes are used to designate the type of each
 * byte in a_format by building a corresponding string during parsing. */
#define _LIBSTASH_OUT_DES_NORMAL    'n'
#define _LIBSTASH_OUT_DES_SPECIFIER 's'
#define _LIBSTASH_OUT_DES_WHITEOUT  'w'

/* Maximum size of specifier to use a stack buffer for parsing. */
#ifdef _LIBSTASH_DBG
#  define _LIBSTASH_OUT_SPEC_BUF 8
#else
#  define _LIBSTASH_OUT_SPEC_BUF 128
#endif

/* Maximum size of stack buffer to use for printing. */
#ifdef _LIBSTASH_DBG
#  define _LIBSTASH_OUT_PRINT_BUF 8
#else
#  define _LIBSTASH_OUT_PRINT_BUF 1024
#endif

#ifdef _LIBSTASH_DBG
#  define _LIBSTASH_OUT_ENT_CACHE 1
#else
#  define _LIBSTASH_OUT_ENT_CACHE 8
#endif

/* The following two structures are used for caching the results from
 * out_p_metric() in order to avoid recalculating it later on. */
typedef struct
{
  cw_sint32_t metric;
  cw_sint32_t spec_len;
  cw_out_ent_t * ent;
} cw_out_ent_el_t;

typedef struct
{
  cw_uint32_t metric;
  cw_uint32_t format_len;
  cw_bool_t raw;
  char format_key_buf[_LIBSTASH_OUT_SPEC_BUF];
  char * format_key;
  cw_out_ent_el_t ents[_LIBSTASH_OUT_ENT_CACHE];
} cw_out_key_t;

static cw_sint32_t
out_p_put_fvle(cw_out_t * a_out, cw_sint32_t a_fd,
	       cw_bool_t a_time_stamp,
	       const char * a_file_name,
	       cw_uint32_t a_line_num,
	       const char * a_func_name,
	       const char * a_format,
	       va_list a_p);

static cw_sint32_t
out_p_put_fvn(cw_out_t * a_out, cw_sint32_t a_fd, cw_uint32_t a_size,
	      cw_out_key_t * a_key,
	      const char * a_format, va_list a_p);

cw_sint32_t
out_p_put_sva(cw_out_t * a_out, char ** r_str,
	      cw_out_key_t * a_key,
	      const char * a_format, va_list a_p);

cw_sint32_t
out_p_put_svn(cw_out_t * a_out, char * a_str, cw_uint32_t a_size,
	      cw_out_key_t * a_key,
	      const char * a_format, va_list a_p);

static cw_sint32_t
out_p_metric(cw_out_t * a_out, const char * a_format,
	     cw_out_key_t * a_key,
	     va_list a_p);

static cw_out_ent_t *
out_p_get_ent(cw_out_t * a_out, const char * a_format, cw_uint32_t a_len);

static cw_sint32_t
out_p_metric_int(const char * a_format, cw_uint32_t a_len,
		 cw_uint64_t a_arg,
		 cw_uint32_t a_nbits, cw_uint32_t a_default_base);

static char *
out_p_render_int(const char * a_format, cw_uint32_t a_len,
		 cw_uint64_t a_arg, char * r_buf,
		 cw_uint32_t a_nbits, cw_uint32_t a_default_base);

static cw_sint32_t
out_p_metric_int32(const char * a_format, cw_uint32_t a_len,
		   const void * a_arg);

static char *
out_p_render_int32(const char * a_format, cw_uint32_t a_len,
		   const void * a_arg, char * r_buf);

static cw_sint32_t
out_p_metric_int64(const char * a_format, cw_uint32_t a_len,
		   const void * a_arg);

static char *
out_p_render_int64(const char * a_format, cw_uint32_t a_len,
		   const void * a_arg, char * r_buf);

static cw_sint32_t
out_p_metric_char(const char * a_format, cw_uint32_t a_len,
		  const void * a_arg);

static char *
out_p_render_char(const char * a_format, cw_uint32_t a_len,
		  const void * a_arg, char * r_buf);

static cw_sint32_t
out_p_metric_string(const char * a_format, cw_uint32_t a_len,
		    const void * a_arg);

static char *
out_p_render_string(const char * a_format, cw_uint32_t a_len,
		    const void * a_arg, char * r_buf);

static cw_sint32_t
out_p_metric_pointer(const char * a_format, cw_uint32_t a_len,
		     const void * a_arg);

static char *
out_p_render_pointer(const char * a_format, cw_uint32_t a_len,
		     const void * a_arg, char * r_buf);

static cw_out_ent_t cw_g_out_builtins[] = 
{
  {"s",    1, sizeof(cw_uint8_t *), out_p_metric_string,  out_p_render_string},
  {"i",    1, sizeof(cw_uint32_t),  out_p_metric_int32,   out_p_render_int32},
  {"p",    1, sizeof(void *),       out_p_metric_pointer, out_p_render_pointer},
  {"c",    1, sizeof(cw_uint8_t),   out_p_metric_char,    out_p_render_char},
  {"q",    1, sizeof(cw_uint64_t),  out_p_metric_int64,   out_p_render_int64},

  {"b",    1, sizeof(cw_buf_t *),   buf_out_metric,       buf_out_render},

#ifdef _TYPE_FP32_DEFINED
  {"f32",  3, sizeof(cw_fp32_t),    NULL,                 NULL},
#endif
#ifdef _TYPE_FP64_DEFINED
  {"f64",  3, sizeof(cw_fp64_t),    NULL,                 NULL},
#endif
#ifdef _TYPE_FP96_DEFINED
  {"f96",  3, sizeof(cw_fp96_t),    NULL,                 NULL},
#endif
#ifdef _TYPE_FP128_DEFINED
  {"f128", 4, sizeof(cw_fp128_t),   NULL,                 NULL}
#endif
};
