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
	      const char * a_format, va_list a_p);

static cw_sint32_t
out_p_metric(cw_out_t * a_out, const char * a_format, char ** r_format,
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
out_p_metric_int8(const char * a_format, cw_uint32_t a_len,
		  const void * a_arg);

static char *
out_p_render_int8(const char * a_format, cw_uint32_t a_len,
		  const void * a_arg, char * r_buf);

static cw_sint32_t
out_p_metric_int16(const char * a_format, cw_uint32_t a_len,
		   const void * a_arg);

static char *
out_p_render_int16(const char * a_format, cw_uint32_t a_len,
		   const void * a_arg, char * r_buf);

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
  {"int8",     sizeof(cw_uint8_t),   out_p_metric_int8,    out_p_render_int8},
  {"i8",       sizeof(cw_uint8_t),   out_p_metric_int8,    out_p_render_int8},
  
  {"int16",    sizeof(cw_uint16_t),  out_p_metric_int16,   out_p_render_int16},
  {"i16",      sizeof(cw_uint16_t),  out_p_metric_int16,   out_p_render_int16},
  
  {"int32",    sizeof(cw_uint32_t),  out_p_metric_int32,   out_p_render_int32},
  {"i32",      sizeof(cw_uint32_t),  out_p_metric_int32,   out_p_render_int32},
  
  {"int64",    sizeof(cw_uint64_t),  out_p_metric_int64,   out_p_render_int64},
  {"i64",      sizeof(cw_uint64_t),  out_p_metric_int64,   out_p_render_int64},

  {"float32",  4,                    NULL,                 NULL},
  {"f32",      4,                    NULL,                 NULL},

  {"float64",  8,                    NULL,                 NULL},
  {"f64",      8,                    NULL,                 NULL},

  {"float96",  12,                   NULL,                 NULL},
  {"f96",      12,                   NULL,                 NULL},

  {"float128", 16,                   NULL,                 NULL},
  {"f128",     16,                   NULL,                 NULL},

  {"char",     sizeof(cw_uint8_t),   out_p_metric_char,    out_p_render_char},
  {"c",        sizeof(cw_uint8_t),   out_p_metric_char,    out_p_render_char},
  
  {"string",   sizeof(cw_uint8_t *), out_p_metric_string,  out_p_render_string},
  {"s",        sizeof(cw_uint8_t *), out_p_metric_string,  out_p_render_string},
  
  {"pointer",  sizeof(void *),       out_p_metric_pointer, out_p_render_pointer},
  {"p",        sizeof(void *),       out_p_metric_pointer, out_p_render_pointer}
};

static cw_sint32_t
spec_p_has_specifier(const char * a_format);
