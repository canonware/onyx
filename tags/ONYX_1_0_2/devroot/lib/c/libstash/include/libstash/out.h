/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

/*
 * Typedef's to allow easy function pointer passing.
 */
typedef cw_uint32_t cw_out_render_t(const char *a_format, cw_uint32_t
    a_format_len, const void *a_arg, cw_uint32_t a_max_len, cw_uint8_t *r_str);

/* Maximum type string length, including NULL termination. */
#define _CW_OUT_MAX_TYPE 16

/* Pseudo-opaque types. */
typedef struct cw_out_s cw_out_t;
typedef struct cw_out_ent_s cw_out_ent_t;

struct cw_out_s {
#ifdef _CW_DBG
	cw_uint32_t	magic;
#endif

	/* Allocator. */
	cw_mem_t	*mem;

	cw_bool_t	is_malloced;
	cw_sint32_t	fd;

	cw_mtx_t	lock;

	cw_uint32_t	nextensions;
	cw_out_ent_t	*extensions;
};

struct cw_out_ent_s {
	cw_uint8_t	type[_CW_OUT_MAX_TYPE];
	cw_uint32_t	len;
	cw_uint32_t	size;
	cw_out_render_t	*render_func;
};

cw_out_t	*out_new(cw_out_t *a_out, cw_mem_t *a_mem);
void		out_delete(cw_out_t *a_out);
void		out_register(cw_out_t *a_out, const char *a_type, cw_uint32_t
    a_size, cw_out_render_t *a_render_func);
void		out_merge(cw_out_t *a_a, cw_out_t *a_b);
cw_sint32_t	out_default_fd_get(cw_out_t *a_out);
void		out_default_fd_set(cw_out_t *a_out, cw_sint32_t a_fd);

#define		_cw_out_put(a_format, a_args...)			\
	out_put(out_std, a_format , ## a_args)
cw_sint32_t	out_put(cw_out_t *a_out, const char *a_format,...);

#define		_cw_out_put_e(a_format, a_args...)			\
	out_put_e(out_err, __FILE__, __LINE__, __FUNCTION__, a_format	\
	    , ## a_args)
cw_sint32_t	out_put_e(cw_out_t *a_out, const char *a_file_name, cw_uint32_t
    a_line_num, const char *a_func_name, const char *a_format,...);

#define		_cw_out_put_n(a_size, a_format, a_args...)		\
	out_put_n(out_std, a_size, a_format , ## a_args)
cw_sint32_t out_put_n(cw_out_t *a_out, cw_uint32_t a_size, const char
    *a_format,...);

#define		_cw_out_put_f(a_fd, a_format, a_args...)		\
	out_put_f(NULL, a_fd, a_format, ## a_args)
cw_sint32_t	out_put_f(cw_out_t *a_out, cw_sint32_t a_fd, const char
    *a_format,...);

#define		_cw_out_put_fn(a_fd, a_size, a_format, a_args...)	\
	out_put_fn(NULL, a_fd, a_size, a_format, ## a_args)
cw_sint32_t	out_put_fn(cw_out_t *a_out, cw_sint32_t a_fd, cw_uint32_t
    a_size, const char *a_format,...);

cw_sint32_t	out_put_fv(cw_out_t *a_out, cw_sint32_t a_fd, const char
    *a_format, va_list a_p);

cw_sint32_t	out_put_fvn(cw_out_t *a_out, cw_sint32_t a_fd, cw_uint32_t
    a_size, const char *a_format, va_list a_p);

#define		_cw_out_put_s(a_str, a_format, a_args...)		\
	out_put_s(NULL, a_str, a_format, ## a_args)
cw_sint32_t	out_put_s(cw_out_t *a_out, char *a_str, const char
    *a_format,...);

cw_sint32_t	out_put_sa(cw_out_t *a_out, char **r_str, const char
    *a_format,...);

#define		_cw_out_put_sn(a_str, a_size, a_format, a_args...)	\
	out_put_sn(NULL, a_str, a_size, a_format, ## a_args)
cw_sint32_t	out_put_sn(cw_out_t *a_out, char *a_str, cw_uint32_t a_size,
    const char *a_format,...);

cw_sint32_t	out_put_sv(cw_out_t *a_out, char *a_str, const char *a_format,
    va_list a_p);
cw_sint32_t	out_put_sva(cw_out_t *a_out, char **r_str, const char *a_format,
    va_list a_p);
cw_sint32_t	out_put_svn(cw_out_t *a_out, char *a_str, cw_uint32_t a_size,
    const char *a_format, va_list a_p);

cw_sint32_t	spec_type_get(const char *a_spec, cw_uint32_t a_spec_len, const
    char **r_val);
cw_sint32_t	spec_val_get(const char *a_spec, cw_uint32_t a_spec_len, const
    char *a_name, cw_uint32_t a_name_len, const cw_uint8_t **r_val);
