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

cw_sint32_t
out_p_put_vfe(cw_out_t * a_out, cw_sint32_t a_fd,
	      const char * a_file_name,
	      cw_uint32_t a_line_num,
	      const char * a_func_name,
	      const char * a_format,
	      va_list a_p);

static cw_sint32_t
out_p_metric(cw_out_t * a_out, const char * a_format, char ** r_format,
	     va_list a_p);

static void
out_p_add(cw_uint32_t a_base, cw_uint32_t a_ndigits,
	  char * r_result, const char * a_a, const char * a_b);

static cw_uint32_t
out_p_metric_int8(const char * a_format, cw_uint32_t a_len,
		  const void * a_arg);

static char *
out_p_render_int8(const char * a_format, cw_uint32_t a_len,
		  const void * a_arg, char * r_buf);

static cw_uint32_t
out_p_metric_int16(const char * a_format, cw_uint32_t a_len,
		   const void * a_arg);

static char *
out_p_render_int16(const char * a_format, cw_uint32_t a_len,
		   const void * a_arg, char * r_buf);

static cw_uint32_t
out_p_metric_int32(const char * a_format, cw_uint32_t a_len,
		   const void * a_arg);

static char *
out_p_render_int32(const char * a_format, cw_uint32_t a_len,
		   const void * a_arg, char * r_buf);

static cw_uint32_t
out_p_metric_int64(const char * a_format, cw_uint32_t a_len,
		   const void * a_arg);

static char *
out_p_render_int64(const char * a_format, cw_uint32_t a_len,
		   const void * a_arg, char * r_buf);

static cw_uint32_t
out_p_metric_string(const char * a_format, cw_uint32_t a_len,
		    const void * a_arg);

static char *
out_p_render_string(const char * a_format, cw_uint32_t a_len,
		    const void * a_arg, char * r_buf);

static cw_uint32_t
out_p_metric_pointer(const char * a_format, cw_uint32_t a_len,
		     const void * a_arg);

static char *
out_p_render_pointer(const char * a_format, cw_uint32_t a_len,
		     const void * a_arg, char * r_buf);

static cw_uint32_t
out_p_metric_undef(const char * a_format, cw_uint32_t a_len,
		   const void * a_arg);

static char *
out_p_render_undef(const char * a_format, cw_uint32_t a_len,
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
  
  {"float32",  4,                    out_p_metric_undef,   out_p_render_undef},
  {"f32",      4,                    out_p_metric_undef,   out_p_render_undef},

  {"float64",  8,                    out_p_metric_undef,   out_p_render_undef},
  {"f64",      8,                    out_p_metric_undef,   out_p_render_undef},

  {"float96",  12,                   out_p_metric_undef,   out_p_render_undef},
  {"f96",      12,                   out_p_metric_undef,   out_p_render_undef},

  {"float128", 16,                   out_p_metric_undef,   out_p_render_undef},
  {"f128",     16,                   out_p_metric_undef,   out_p_render_undef},
  
  {"string",   sizeof(cw_uint8_t *), out_p_metric_string,  out_p_render_string},
  {"s",        sizeof(cw_uint8_t *), out_p_metric_string,  out_p_render_string},
  
  {"pointer",  sizeof(void *),       out_p_metric_pointer, out_p_render_pointer},
  {"p",        sizeof(void *),       out_p_metric_pointer, out_p_render_pointer}
};
