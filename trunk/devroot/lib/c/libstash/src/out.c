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

#include "../include/libstash/libstash.h"

#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>

#ifdef _LIBSTASH_DBG
#define _CW_OUT_MAGIC 0x8293cade

#ifdef _cw_assert
#undef _cw_assert
#endif
#define _cw_assert(a)							\
	do {								\
		if (!(a)) {						\
			fprintf(stderr, "At %s, line %d: %s():"		\
			    " Failed assertion: \"%s\"\n", __FILE__,	\
			    __LINE__, __FUNCTION__, #a);		\
			abort();					\
		}							\
	} while (0)
#endif

/*
 * Maximum size of stack buffer to use for printing.  Must be at least 2
 * bytes.
 */
#ifdef _LIBSTASH_DBG
#define _CW_OUT_PRINT_BUF	   8
#else
#define _CW_OUT_PRINT_BUF 	 128
#endif

/*
 * Number of bytes to add to the size of a buffer when realloc()ing.  Must be
 * at least 1 byte.
 */
#ifdef _LIBSTASH_DBG
#define _CW_OUT_REALLOC_INC	   8
#else
#define _CW_OUT_REALLOC_INC	4096
#endif

/*
 * Maximum number of el's to store in a stack buffer before allocating space on
 * the heap.  Must be at least 1.
 */
#ifdef _LIBSTASH_DBG
#define _CW_OUT_ELS_BUF		   1
#else
#define _CW_OUT_ELS_BUF		  16
#endif

/*
 * The following structures are used for caching the results from
 * out_p_format_scan() in order to avoid recalculating it later on.
 */
typedef enum {
	TEXT,
	SPEC
}	cw_out_el_type_t;

typedef struct {
	cw_out_el_type_t el_type;
	cw_uint32_t	el_offset;
	cw_uint32_t	el_len;
}       cw_out_el_t;

typedef struct {
	cw_uint32_t	key_nels;
	cw_out_el_t	key_els_buf[_CW_OUT_ELS_BUF];
	cw_out_el_t	*key_els;
}       cw_out_key_t;

static cw_sint32_t 	out_p_put_fvle(cw_out_t *a_out, cw_sint32_t a_fd,
    cw_bool_t a_time_stamp, const char *a_file_name, cw_uint32_t a_line_num,
    const char *a_func_name, const char *a_format, va_list a_p);
static cw_sint32_t	out_p_put_fvn(cw_out_t *a_out, cw_sint32_t a_fd,
    cw_uint32_t a_size, const char *a_format, va_list
    a_p);
static cw_sint32_t	out_p_put_sva(cw_out_t *a_out, char **r_str,
    const char *a_format, va_list a_p);
static cw_sint32_t	out_p_put_svn(cw_out_t *a_out, char **a_str, cw_uint32_t
    a_size, cw_uint32_t a_max_size, const char *a_format, va_list a_p);
static cw_uint8_t	*out_p_buffer_expand(cw_out_t *a_out, cw_uint32_t
    *ar_acount, cw_uint8_t *a_buffer, cw_uint32_t a_copy_size, cw_uint32_t
    a_expand_size);
static cw_sint32_t	out_p_format_scan(cw_out_t *a_out, const char *a_format,
     cw_out_key_t *a_key, va_list a_p);
static cw_sint32_t	out_p_el_accept(cw_out_t *a_out, const char *a_format,
    cw_out_key_t *a_key, cw_out_el_type_t a_type, cw_uint32_t a_offset,
    cw_uint32_t a_len);
static cw_out_ent_t	*out_p_ent_get(cw_out_t *a_out, const char *a_format,
    cw_uint32_t a_len);
static void		out_p_common_render(const char *a_format, cw_uint32_t
    a_len, cw_uint32_t a_max_len, cw_uint32_t a_rlen, cw_uint8_t *r_buf,
    cw_uint32_t *r_width, cw_uint32_t *r_owidth, cw_uint32_t *r_offset);
static cw_uint32_t	out_p_int_render(const char *a_format, cw_uint32_t
    a_len, cw_uint64_t a_arg, cw_uint32_t a_max_len, cw_uint8_t *r_buf,
    cw_uint32_t a_nbits, cw_uint32_t a_default_base);
static cw_uint32_t	out_p_int32_render(const char *a_format, cw_uint32_t
    a_len, const void *a_arg, cw_uint32_t a_max_len, cw_uint8_t *r_buf);
static cw_uint32_t	out_p_int64_render(const char *a_format, cw_uint32_t
    a_len, const void *a_arg, cw_uint32_t a_max_len, cw_uint8_t *r_buf);
static cw_uint32_t	out_p_char_render(const char *a_format, cw_uint32_t
    a_len, const void *a_arg, cw_uint32_t a_max_len, cw_uint8_t *r_buf);
static cw_uint32_t	out_p_string_render(const char *a_format, cw_uint32_t
    a_len, const void *a_arg, cw_uint32_t a_max_len, cw_uint8_t *r_buf);
static cw_uint32_t	out_p_pointer_render(const char *a_format, cw_uint32_t
    a_len, const void *a_arg, cw_uint32_t a_max_len, cw_uint8_t *r_buf);
static cw_uint32_t	out_p_buf_render(const char *a_format, cw_uint32_t
    a_len, const void *a_arg, cw_uint32_t a_max_len, cw_uint8_t *r_buf);

static cw_out_ent_t cw_g_out_builtins[] = {
	{"s",	1,	sizeof(cw_uint8_t *),	out_p_string_render},
	{"i",	1,	sizeof(cw_uint32_t),	out_p_int32_render},
	{"p",	1,	sizeof(void *),		out_p_pointer_render},
	{"c",	1,	sizeof(cw_uint8_t),	out_p_char_render},
	{"q",	1,	sizeof(cw_uint64_t),	out_p_int64_render},
	{"b",	1,	sizeof(cw_buf_t *),	out_p_buf_render},

#ifdef _TYPE_FP32_DEFINED
	{"f32",	3,	sizeof(cw_fp32_t),	NULL},
#endif
#ifdef _TYPE_FP64_DEFINED
	{"f64",	3,	sizeof(cw_fp64_t),	NULL},
#endif
#ifdef _TYPE_FP96_DEFINED
	{"f96",	3,	sizeof(cw_fp96_t),	NULL},
#endif
#ifdef _TYPE_FP128_DEFINED
	{"f128",4,	sizeof(cw_fp128_t),	NULL}
#endif
};

cw_out_t *
out_new(cw_out_t *a_out, cw_mem_t *a_mem)
{
	cw_out_t	*retval;

	if (a_out != NULL) {
		retval = a_out;
		retval->is_malloced = FALSE;
	} else {
		retval = (cw_out_t *)_cw_mem_malloc(a_mem, sizeof(cw_out_t));
		if (retval == NULL)
			goto RETURN;
		retval->is_malloced = TRUE;
	}

	retval->mem = a_mem;
	retval->fd = 2;

	mtx_new(&retval->lock);

	retval->nextensions = 0;
	retval->extensions = NULL;

#ifdef _LIBSTASH_DBG
	retval->magic = _CW_OUT_MAGIC;
#endif

	RETURN:
	return retval;
}

void
out_delete(cw_out_t *a_out)
{
	_cw_check_ptr(a_out);
	_cw_assert(a_out->magic == _CW_OUT_MAGIC);

	if (a_out->extensions != NULL)
		_cw_mem_free(a_out->mem, a_out->extensions);
	mtx_delete(&a_out->lock);

	if (a_out->is_malloced)
		_cw_mem_free(a_out->mem, a_out);
#ifdef _LIBSTASH_DBG
	else
		memset(a_out, 0x5a, sizeof(cw_out_t));
#endif
}

cw_bool_t
out_register(cw_out_t *a_out, const char *a_type, cw_uint32_t a_size,
    cw_out_render_t * a_render_func)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_out);
	_cw_assert(a_out->magic == _CW_OUT_MAGIC);
	_cw_check_ptr(a_type);
	_cw_assert(strlen(a_type) <= _CW_OUT_MAX_TYPE);
	_cw_assert((a_size == 1) || (a_size == 2) || (a_size == 4) ||
	    (a_size == 8) || (a_size == 12) || (a_size == 16));
	_cw_check_ptr(a_render_func);

	if (a_out->extensions == NULL) {
		a_out->extensions = (cw_out_ent_t
		    *)_cw_mem_malloc(a_out->mem, sizeof(cw_out_ent_t));
		if (a_out->extensions == NULL) {
			retval = TRUE;
			goto RETURN;
		}
	} else {
		cw_out_ent_t	*t_ptr;

		t_ptr = (cw_out_ent_t *)_cw_mem_realloc(a_out->mem,
		    a_out->extensions, ((a_out->nextensions + 1) *
		    sizeof(cw_out_ent_t)));
		if (t_ptr == NULL) {
			retval = TRUE;
			goto RETURN;
		}
		a_out->extensions = t_ptr;
	}

	memcpy(a_out->extensions[a_out->nextensions].type, a_type,
	    strlen(a_type));
	a_out->extensions[a_out->nextensions].len = strlen(a_type);
	a_out->extensions[a_out->nextensions].size = a_size;
	a_out->extensions[a_out->nextensions].render_func = a_render_func;

	a_out->nextensions++;

	retval = FALSE;

	RETURN:
	return retval;
}

cw_bool_t
out_merge(cw_out_t *a_a, cw_out_t *a_b)
{
	cw_bool_t	retval;
	cw_sint32_t	i;

	_cw_check_ptr(a_a);
	_cw_assert(a_a->magic == _CW_OUT_MAGIC);
	_cw_check_ptr(a_b);
	_cw_assert(a_b->magic == _CW_OUT_MAGIC);

	if (a_b->nextensions > 0) {
		if (a_a->extensions == NULL) {
			a_a->extensions = (cw_out_ent_t
			    *)_cw_mem_calloc(a_a->mem, a_b->nextensions,
			    sizeof(cw_out_ent_t));
			if (a_a->extensions == NULL) {
				retval = TRUE;
				goto RETURN;
			}
		} else {
			cw_out_ent_t	*t_ptr;

			t_ptr = (cw_out_ent_t *)_cw_mem_realloc(a_a->mem,
			    a_a->extensions, ((a_a->nextensions +
			    a_b->nextensions) * sizeof(cw_out_ent_t)));
			if (t_ptr == NULL) {
				retval = TRUE;
				goto RETURN;
			}
			a_a->extensions = t_ptr;
		}

		memcpy(&a_a->extensions[a_a->nextensions], a_b->extensions,
		    a_b->nextensions * sizeof(cw_out_ent_t));

		/* Make copies of the type strings. */
		for (i = 0; i < a_b->nextensions; i++) {
			memcpy(a_b->extensions[i].type, a_a->extensions[i +
			    a_a->nextensions].type, _CW_OUT_MAX_TYPE);
		}

		a_a->nextensions += a_b->nextensions;
	}

	retval = FALSE;
	RETURN:
	return retval;
}

cw_sint32_t
out_get_default_fd(cw_out_t *a_out)
{
	cw_sint32_t retval;

	if (a_out != NULL)
		retval = a_out->fd;
	else
		retval = 2;

	return retval;
}

void
out_set_default_fd(cw_out_t *a_out, cw_sint32_t a_fd)
{
	_cw_check_ptr(a_out);
	_cw_assert(a_out->magic == _CW_OUT_MAGIC);
	_cw_assert(0 <= a_fd);

	a_out->fd = a_fd;
}

cw_sint32_t
out_put(cw_out_t *a_out, const char *a_format,...)
{
	cw_sint32_t	retval, fd;
	va_list		ap;

	_cw_check_ptr(a_format);

	if (a_out != NULL) {
		_cw_assert(_CW_OUT_MAGIC == a_out->magic);
		fd = a_out->fd;
	} else
		fd = 2;

	va_start(ap, a_format);
	retval = out_put_fv(a_out, fd, a_format, ap);
	va_end(ap);

	return retval;
}

cw_sint32_t
out_put_e(cw_out_t *a_out, const char *a_file_name, cw_uint32_t a_line_num,
    const char *a_func_name, const char *a_format, ...)
{
	cw_sint32_t	retval, fd;
	va_list		ap;

	_cw_check_ptr(a_format);

	if (a_out != NULL) {
		_cw_assert(_CW_OUT_MAGIC == a_out->magic);
		fd = a_out->fd;
	} else
		fd = 2;

	va_start(ap, a_format);
	retval = out_p_put_fvle(a_out, fd, FALSE, a_file_name, a_line_num,
	    a_func_name, a_format, ap);
	va_end(ap);

	return retval;
}

cw_sint32_t
out_put_l(cw_out_t *a_out, const char *a_format,...)
{
	cw_sint32_t	retval, fd;
	va_list		ap;

	_cw_check_ptr(a_format);

	if (a_out != NULL) {
		_cw_assert(_CW_OUT_MAGIC == a_out->magic);
		fd = a_out->fd;
	} else
		fd = 2;

	va_start(ap, a_format);
	retval = out_p_put_fvle(a_out, fd, TRUE, NULL, 0, NULL, a_format, ap);
	va_end(ap);

	return retval;
}

cw_sint32_t
out_put_le(cw_out_t *a_out, const char *a_file_name, cw_uint32_t a_line_num,
    const char *a_func_name, const char *a_format, ...)
{
	cw_sint32_t	retval, fd;
	va_list		ap;

	_cw_check_ptr(a_format);

	if (a_out != NULL) {
		_cw_assert(_CW_OUT_MAGIC == a_out->magic);
		fd = a_out->fd;
	} else
		fd = 2;

	va_start(ap, a_format);
	retval = out_p_put_fvle(a_out, fd, TRUE, a_file_name, a_line_num,
	    a_func_name, a_format, ap);
	va_end(ap);

	return retval;
}

cw_sint32_t
out_put_n(cw_out_t *a_out, cw_uint32_t a_size, const char *a_format,...)
{
	cw_sint32_t	retval, fd;
	va_list		ap;

	_cw_check_ptr(a_format);

	if (a_out != NULL) {
		_cw_assert(_CW_OUT_MAGIC == a_out->magic);
		fd = a_out->fd;
	} else
		fd = 2;

	va_start(ap, a_format);
	retval = out_p_put_fvn(a_out, fd, a_size, a_format, ap);
	va_end(ap);

	return retval;
}

cw_sint32_t
out_put_f(cw_out_t *a_out, cw_sint32_t a_fd, const char *a_format,...)
{
	cw_sint32_t	retval;
	va_list		ap;

	_cw_assert(0 <= a_fd);
	_cw_check_ptr(a_format);

	va_start(ap, a_format);
	retval = out_put_fv(a_out, a_fd, a_format, ap);
	va_end(ap);

	return retval;
}

cw_sint32_t
out_put_fe(cw_out_t *a_out, cw_sint32_t a_fd, const char *a_file_name,
    cw_uint32_t a_line_num, const char *a_func_name, const char *a_format, ...)
{
	cw_sint32_t	retval;
	va_list		ap;

	_cw_assert(a_fd >= 0);
	_cw_check_ptr(a_format);

	va_start(ap, a_format);
	retval = out_p_put_fvle(a_out, a_fd, FALSE, a_file_name, a_line_num,
	    a_func_name, a_format, ap);
	va_end(ap);

	return retval;
}

cw_sint32_t
out_put_fl(cw_out_t *a_out, cw_sint32_t a_fd, const char *a_format,...)
{
	cw_sint32_t	retval;
	va_list		ap;

	_cw_assert(a_fd >= 0);
	_cw_check_ptr(a_format);

	va_start(ap, a_format);
	retval = out_p_put_fvle(a_out, a_fd, TRUE, NULL, 0, NULL, a_format, ap);
	va_end(ap);

	return retval;
}

cw_sint32_t
out_put_fle(cw_out_t *a_out, cw_sint32_t a_fd, const char *a_file_name,
    cw_uint32_t a_line_num, const char *a_func_name, const char *a_format, ...)
{
	cw_sint32_t	retval;
	va_list		ap;

	_cw_assert(a_fd >= 0);
	_cw_check_ptr(a_format);

	va_start(ap, a_format);
	retval = out_p_put_fvle(a_out, a_fd, TRUE, a_file_name, a_line_num,
	    a_func_name, a_format, ap);
	va_end(ap);

	return retval;
}

cw_sint32_t
out_put_fn(cw_out_t *a_out, cw_sint32_t a_fd, cw_uint32_t a_size,
    const char *a_format,...)
{
	cw_sint32_t	retval;
	va_list		ap;

	_cw_assert(a_fd >= 0);
	_cw_check_ptr(a_format);

	va_start(ap, a_format);
	retval = out_p_put_fvn(a_out, a_fd, a_size, a_format, ap);
	va_end(ap);

	return retval;
}

cw_sint32_t
out_put_fv(cw_out_t *a_out, cw_sint32_t a_fd, const char *a_format, va_list a_p)
{
	cw_sint32_t	retval;

	_cw_assert(a_fd >= 0);
	_cw_check_ptr(a_format);

	retval = out_p_put_fvn(a_out, a_fd, UINT_MAX, a_format, a_p);

	return retval;
}

cw_sint32_t
out_put_s(cw_out_t *a_out, char *a_str, const char *a_format,...)
{
	cw_sint32_t	retval;
	va_list		ap;

	_cw_check_ptr(a_str);
	_cw_check_ptr(a_format);

	va_start(ap, a_format);
	retval = out_put_sv(a_out, a_str, a_format, ap);
	va_end(ap);

	return retval;
}

cw_sint32_t
out_put_sa(cw_out_t *a_out, char **r_str, const char *a_format,...)
{
	cw_sint32_t	retval;
	va_list		ap;

	_cw_check_ptr(a_format);

	va_start(ap, a_format);
	retval = out_put_sva(a_out, r_str, a_format, ap);
	va_end(ap);

	return retval;
}

cw_sint32_t
out_put_sn(cw_out_t *a_out, char *a_str, cw_uint32_t a_size, const char
    *a_format,...)
{
	cw_sint32_t	retval;
	va_list		ap;

	_cw_check_ptr(a_str);
	_cw_check_ptr(a_format);

	va_start(ap, a_format);
	retval = out_put_svn(a_out, a_str, a_size, a_format, ap);
	va_end(ap);

	return retval;
}

cw_sint32_t
out_put_sv(cw_out_t *a_out, char *a_str, const char *a_format, va_list a_p)
{
	cw_sint32_t	retval;

	_cw_check_ptr(a_str);
	_cw_check_ptr(a_format);

	retval = out_p_put_svn(a_out, &a_str, UINT_MAX, UINT_MAX, a_format,
	    a_p);
	if (retval < 0)
		goto RETURN;
	a_str[retval] = '\0';

	RETURN:
	return retval;
}

cw_sint32_t
out_put_sva(cw_out_t *a_out, char **r_str, const char *a_format, va_list a_p)
{
	cw_sint32_t	retval;

	_cw_check_ptr(a_format);

	retval = out_p_put_sva(a_out, r_str, a_format, a_p);

	return retval;
}

cw_sint32_t
out_put_svn(cw_out_t *a_out, char *a_str, cw_uint32_t a_size,
    const char *a_format, va_list a_p)
{
	cw_sint32_t	retval;

	_cw_check_ptr(a_str);
	_cw_assert(a_size >= 0);
	_cw_check_ptr(a_format);

	retval = out_p_put_svn(a_out, &a_str, a_size, a_size, a_format,
	    a_p);

	return retval;
}

cw_sint32_t
spec_get_type(const char *a_spec, cw_uint32_t a_spec_len, const char **r_val)
{
	cw_sint32_t	retval, i;

	_cw_check_ptr(a_spec);
	_cw_assert(a_spec_len > 0);
	_cw_check_ptr(r_val);

	/*
	 * If we don't hit a '|' before the end of the specifier, then the
	 * entire specifier is the type.
	 */
	for (i = 0; i < a_spec_len; i++) {
		if (a_spec[i] == '|')
			break;
	}

	*r_val = a_spec;
	retval = i;

	return retval;
}

cw_sint32_t
spec_get_val(const char *a_spec, cw_uint32_t a_spec_len, const char *a_name,
    cw_uint32_t a_name_len, const cw_uint8_t **r_val)
{
	cw_sint32_t	retval, i, curr_name_len, val_len;
	cw_bool_t	match;
	enum {
		NAME,
		VALUE
	}	state;

	_cw_check_ptr(a_spec);
	_cw_assert(a_spec_len > 0);
	_cw_check_ptr(a_name);
	_cw_check_ptr(r_val);

	curr_name_len = 0;	/* Shut up the optimizer warnings. */

	for (i = val_len = 0, match = FALSE, state = VALUE; i < a_spec_len;
	    i++) {
		switch (state) {
		case NAME:
			if (a_spec[i] == ':') {
				if (curr_name_len != a_name_len) {
					/* Too short. */
					match = FALSE;
				}
				if (match) {
					/*
					 * Set the return pointer.  We'll figure
					 * out how long the value is later.
					 */
					*r_val = &a_spec[i + 1];
				}
				val_len = 0;
				state = VALUE;
			} else if (a_name[curr_name_len] != a_spec[i]) {
				curr_name_len++;
				match = FALSE;
			} else
				curr_name_len++;

			break;
		case VALUE:
			if (a_spec[i] == '|') {
				/* End of the value. */
				if (match) {
					retval = val_len;
					goto RETURN;
				} else {
					curr_name_len = 0;
					match = TRUE;
					state = NAME;
				}
			} else if (i == a_spec_len - 1) {
				/*
				 * End of the value, and end of specifier.  Add
				 * one to val_len.
				 */
				val_len++;

				if (match) {
					retval = val_len;
					goto RETURN;
				} else {
					curr_name_len = 0;
					match = TRUE;
					state = NAME;
				}
			} else
				val_len++;

			break;
		default:
			_cw_error("Programming error");
		}
	}

	retval = -1;

	RETURN:
	return retval;
}

static cw_sint32_t
out_p_put_fvle(cw_out_t *a_out, cw_sint32_t a_fd, cw_bool_t a_time_stamp, const
    char *a_file_name, cw_uint32_t a_line_num, const char *a_func_name, const
    char *a_format, va_list a_p)
{
	cw_sint32_t	retval;
	char		*format = NULL, timestamp[128];

	_cw_assert(a_fd >= 0);
	_cw_check_ptr(a_format);

	if (a_time_stamp) {
		time_t		curr_time;
		struct tm	*cts;

		curr_time = time(NULL);
		cts = localtime(&curr_time);
		if (strftime(timestamp, sizeof(timestamp),
		    "[[%Y/%m/%d %T %Z]: ", cts) == 0) {
			/*
			 * Wow, this locale must be *really* verbose about
			 * displaying time. Terminate the string, since there's
			 * no telling what's there.
			 */
			timestamp[0] = '\0';
		}
	} else
		timestamp[0] = '\0';

	if (a_file_name != NULL) {
		if (a_func_name != NULL) {
			/* Print filename, line number, and function name. */
			if ((retval = out_put_sa(a_out, &format,
			    "[s]At [s], line [i]: [s](): [s]", timestamp,
			    a_file_name, a_line_num, a_func_name, a_format)) <
			    0)
				goto RETURN;
		} else {
			/* Print filename and line number. */
			if ((retval = out_put_sa(a_out, &format,
			    "[s]At [s], line [i]: [s]", timestamp, a_file_name,
			    a_line_num, a_format)) < 0)
				goto RETURN;
		}
	} else if (a_func_name != NULL) {
		/* Print function name. */
		if ((retval = out_put_sa(a_out, &format, "[s][s](): [s]",
		    timestamp, a_func_name, a_format)) < 0)
			goto RETURN;
	} else {
		/* Make no modifications. */
		if ((retval = out_put_sa(a_out, &format, "[s][s]", timestamp,
		    a_format)) < 0)
			goto RETURN;
	}

	retval = out_put_fv(a_out, a_fd, format, a_p);

	RETURN:
	if (format != NULL)
		_cw_mem_free((a_out != NULL) ? a_out->mem : NULL, format);
	return retval;
}

static cw_sint32_t
out_p_put_fvn(cw_out_t *a_out, cw_sint32_t a_fd, cw_uint32_t a_size, const char
    *a_format, va_list a_p)
{
	cw_sint32_t	retval, i, out_size, nwritten;
	cw_uint32_t	osize;
	char		sbuf[_CW_OUT_PRINT_BUF];
	char		*obuf;

	_cw_assert(a_fd >= 0);
	_cw_check_ptr(a_format);

	if (a_size > _CW_OUT_PRINT_BUF)
		osize = _CW_OUT_PRINT_BUF;
	else
		osize = a_size;
	obuf = sbuf;
	if ((out_size = out_p_put_svn(a_out, &obuf, osize, a_size,
	    a_format, a_p)) < 0) {
		retval = out_size;
		goto RETURN;
	}

	if (a_out != NULL)
		mtx_lock(&a_out->lock);
	i = 0;
	do {
		nwritten = write(a_fd, &obuf[i], out_size - i);
		if (nwritten != -1)
			i += nwritten;
		else {
			if (a_out != NULL)
				mtx_unlock(&a_out->lock);
			retval = -3;
			goto RETURN;
		}
	} while ((i < out_size) && (nwritten == -1) && (errno == EAGAIN));

	retval = i;

	if (a_out != NULL)
		mtx_unlock(&a_out->lock);
	RETURN:
	if (obuf != sbuf)
		_cw_mem_free((a_out != NULL) ? a_out->mem : NULL, obuf);
	return retval;
}

static cw_sint32_t
out_p_put_sva(cw_out_t *a_out, char **r_str, const char *a_format, va_list a_p)
{
	cw_sint32_t	retval;

	*r_str = NULL;
	retval = out_p_put_svn(a_out, r_str, 0, UINT_MAX, a_format, a_p);

	if (retval >= 0) {
		if (*r_str != NULL)
			(*r_str)[retval] = '\0';
	} else
		*r_str = NULL;
	return retval;
}

static cw_sint32_t
out_p_put_svn(cw_out_t *a_out, char **a_str, cw_uint32_t a_size, cw_uint32_t
    a_max_size, const char *a_format, va_list a_p)
{
	cw_sint32_t	retval, i, key_offset, rcount;
	cw_out_key_t	key;
	cw_uint32_t	osize;
	cw_uint32_t	acount = 0;	/* Number of realloc()s done so far. */
	cw_uint8_t	*obuf;

	_cw_check_ptr(a_str);
	_cw_check_ptr(a_format);

	if ((retval = out_p_format_scan(a_out, a_format, &key, a_p)) < 0)
		goto RETURN;

	/*
	 * Make sure that there is a buffer to start working with.
	 */
	if (*a_str != NULL) {
		/* A buffer was passed in. */
		osize = a_size;
		obuf = *a_str;
	} else {
		/*
		 * Allocate an initial buffer.  It's possible that the
		 * formatting string is raw (i.e. we could deterministically
		 * allocate exactly the right amount of memory), but this would
		 * imply that the user has asked for dynamic string allocation
		 * for an output string that is always the same length.  This is
		 * atypical usage, so don't go to the extra trouble of checking
		 * for this condition.
		 */
		obuf = out_p_buffer_expand(a_out, &acount, NULL, 0,
		    _CW_OUT_PRINT_BUF);
		if (obuf == NULL) {
			retval = -1;
			goto RETURN;
		}
		osize = _CW_OUT_PRINT_BUF;
	}

	for (i = key_offset = 0; key_offset < key.key_nels && i < a_max_size;
	     key_offset++) {
		if (key.key_els[key_offset].el_type == TEXT) {
			/* Text el. */

			/*
			 * Make sure there is enough room to memcpy() this el.
			 * If not, reallocate and try to add additional padding.
			 */
			if (i + key.key_els[key_offset].el_len >= osize) {
				cw_uint32_t	esize;

				/* Overflow. */
				if (osize + key.key_els[key_offset].el_len +
				    _CW_OUT_REALLOC_INC <= a_max_size) {
					esize = osize +
					    key.key_els[key_offset].el_len +
					    _CW_OUT_REALLOC_INC;
				} else
					esize = a_max_size;

				obuf = out_p_buffer_expand(a_out, &acount, obuf,
				    osize, esize);
				if (obuf == NULL) {
					retval = -1;
					goto RETURN;
				}
				osize = esize;
			}

			/*
			 * memcpy as much as this el as will fit in obuf.
			 */
			if (i + key.key_els[key_offset].el_len <= osize) {
				/* The whole el will fit into obuf. */
				memcpy(&obuf[i],
				    &a_format[key.key_els[key_offset].el_offset],
				    key.key_els[key_offset].el_len);
				i += key.key_els[key_offset].el_len;
			} else {
				/* Only part of the el will fit into obuf. */
				memcpy(&obuf[i],
				    &a_format[key.key_els[key_offset].el_offset],
				    osize - i);
				i += osize - i;
			}
		} else {
			cw_sint32_t	type_len;
			const char	*type;
			cw_out_ent_t	*ent;
			void		*arg;

			/* Specifier el. */

			/* Find the type string. */
			type_len =
			    spec_get_type(&a_format[key.key_els[key_offset].el_offset],
			    key.key_els[key_offset].el_len, &type);
			_cw_assert(type_len >= 0);
			/* Get the handler for this specifier. */
			ent = out_p_ent_get(a_out, type, type_len);
			if (ent == NULL) {
				/* No handler. */
				retval = -2;
				goto RETURN;
			}

			switch (ent->size) {
			case 1: case 2: case 4:
				arg = (void *)&va_arg(a_p, cw_uint32_t);
				break;
			case 8:
				arg = (void *)&va_arg(a_p, cw_uint64_t);
				break;
#ifdef _TYPE_FP96_DEFINED
			case 12:
				arg = (void *)&va_arg(a_p, cw_fp96_t);
				break;
#endif
#ifdef _TYPE_FP128_DEFINED
			case 16:
				arg = (void *)&va_arg(a_p, cw_fp128_t);
				break;
#endif
			default:
				arg = NULL;	/*
						 * Keep the optimizer
						 * quiet.
						 */
				_cw_error("Programming error");
			}

			/*
			 * Try to render this element.  If it fails due to
			 * overflow, reallocate and try again.  Add a spare
			 * _CW_OUT_REALLOC_INC bytes in order to hopefully avoid
			 * another overflow.
			 */
			rcount =
			    ent->render_func(&a_format[key.key_els[key_offset].el_offset],
			    key.key_els[key_offset].el_len, arg, osize - i,
			    &obuf[i]);
			if (rcount > osize - i) {
				/*
				 * Calculate the new size to expand the buffer
				 * to.
				 */
				if (i + rcount + _CW_OUT_REALLOC_INC <=
				    a_max_size) {
					osize = i + rcount +
					    _CW_OUT_REALLOC_INC;
				} else {
					if (osize == a_max_size) {
						/*
						 * We're not allowed to expand
						 * any farther.
						 */
						i = a_max_size;
						goto DONE;
					}
					osize = a_max_size;
				}

				obuf = out_p_buffer_expand(a_out, &acount, obuf,
				    i, osize);
				if (obuf == NULL) {
					retval = -1;
					goto RETURN;
				}

				rcount = ent->render_func(&a_format[key.key_els[key_offset].el_offset],
				    key.key_els[key_offset].el_len, arg, osize -
				    i, &obuf[i]);
				if (rcount > osize - i)
					rcount = osize - i;
			}

			i += rcount;
		}
	}

	DONE:
	retval = i;

	if (acount > 0) {
		/*
		 * If doing dynamic allocation, release any extra space at the
		 * end of the buffer that didn't end up getting used (or perhaps
		 * make space for a '\0' terminator).  Make sure to leave space
		 * for a '\0' terminator.
		 */
		if ((*a_str == NULL) && (osize != retval + 1)) {
			obuf = out_p_buffer_expand(a_out, &acount, obuf, osize,
			    retval + 1);
			if (obuf == NULL) {
				retval = -1;
				goto RETURN;
			}
		}
		*a_str = obuf;
	}
	RETURN:
	if (key.key_els != key.key_els_buf) {
		/* out_p_format_scan() allocated a new spec key. */
		_cw_mem_free((a_out != NULL) ? a_out->mem : NULL,
		    key.key_els);
	}
	return retval;
}

static cw_uint8_t *
out_p_buffer_expand(cw_out_t *a_out, cw_uint32_t *ar_acount, cw_uint8_t
    *a_buffer, cw_uint32_t a_copy_size, cw_uint32_t a_expand_size)
{
	cw_uint8_t	*retval;

	if (*ar_acount == 0) {
		retval = (cw_uint8_t *)_cw_mem_malloc((a_out != NULL) ?
		    a_out->mem : NULL, a_expand_size);
		if (retval == NULL)
			goto RETURN;
		if (a_copy_size > 0)
			memcpy(retval, a_buffer, a_copy_size);
	} else {
		retval = (cw_uint8_t *)_cw_mem_realloc((a_out != NULL) ?
		    a_out->mem : NULL, a_buffer, a_expand_size);
		if (retval == NULL) {
			_cw_mem_free((a_out != NULL) ? a_out->mem : NULL,
			    a_buffer);
			goto RETURN;
		}
		/* realloc() copies for us. */
	}
	(*ar_acount)++;

	RETURN:
	return retval;
}

static cw_sint32_t
out_p_format_scan(cw_out_t *a_out, const char *a_format, cw_out_key_t *a_key,
    va_list a_p)
{
	cw_sint32_t	retval = 0;
	cw_uint32_t	i, el_offset;
	enum {
		NORMAL,
		BRACKET,
		NAME,
		VALUE
	}	state;

	a_key->key_nels = 0;
	a_key->key_els = a_key->key_els_buf;

	for (i = el_offset = 0, state = NORMAL; a_format[i] != '\0'; i++) {
		switch (state) {
		case NORMAL:
			if (a_format[i] == '[') {
				/*
				 * This character signifies the end of a text
				 * el (unless i == 0, in which case, we've just
				 * begun).  If the next character is also a '[',
				 * it will mark the beginning of a new text el.
				 */
				if (i > 0 && (retval = out_p_el_accept(a_out,
				    a_format, a_key, TEXT, el_offset, i
				    - el_offset)) < 0)
					goto RETURN;
				el_offset = i + 1;
				state = BRACKET;
			}
			break;
		case BRACKET:
			if (a_format[i] == '[')
				state = NORMAL;
			else
				state = VALUE;
			break;
		case NAME:
			if (a_format[i] == ':')
					state = VALUE;
			break;
		case VALUE:
			if (a_format[i] == '|')
				state = NAME;
			else if (a_format[i] == ']') {
				if ((retval = out_p_el_accept(a_out, a_format,
				    a_key, SPEC, el_offset, i - el_offset)) < 0)
					goto RETURN;
				el_offset = i + 1;
				state = NORMAL;
			}

			break;
		default:
			_cw_error("Programming error");
		}
	}
	if (state != NORMAL) {
		retval = -2;
		goto RETURN;
	}
	/*
	 * Accept the last el, if there is one.
	 */
	if (i > el_offset) {
		retval = out_p_el_accept(a_out, a_format, a_key, TEXT,
		    el_offset, i - el_offset);
	}

	RETURN:
	return retval;
}

static cw_sint32_t
out_p_el_accept(cw_out_t *a_out, const char *a_format, cw_out_key_t *a_key,
    cw_out_el_type_t a_type, cw_uint32_t a_offset, cw_uint32_t a_len)
{
	cw_sint32_t	retval;

	/* Check for overflow of the stack-allocated els buffer. */
	if (a_key->key_nels == _CW_OUT_ELS_BUF) {
		cw_uint32_t	i, total_els = _CW_OUT_ELS_BUF + 2;

		_cw_assert(a_key->key_els == a_key->key_els_buf);
			
		/*
		 * Overflow.  Time to face cold hard reality, calculate a tight
		 * upper bound (at most off by one) on the total number of el's,
		 * allocate a buffer, and copy the static buffer's contents
		 * over.  We know that we need to expand by at least 1 el, and
		 * there may be one more el than there are '[' characters (hence
		 * the initialization of total_els), plus anything after the end
		 * of this el.
		 */
		for (i = a_offset + a_len; a_format[i] != '\0'; i++) {
			if (a_format[i] == '[') {
				if (a_format[i + 1] == '[') {
					/*
					 * "[[".  Only add one to
					 * total_els.
					 */
					i++;
					total_els++;
				} else
					total_els += 2;
			}
		}
		a_key->key_els = (cw_out_el_t *)_cw_mem_malloc((a_out != NULL)
		    ? a_out->mem : NULL, total_els * sizeof(cw_out_el_t));
		if (a_key->key_els == NULL) {
			retval = -1;
			goto RETURN;
		}
#ifdef _LIBSTASH_DBG
		memset(a_key->key_els, 0, total_els * sizeof(cw_out_el_t));
#endif
		memcpy(a_key->key_els, a_key->key_els_buf, _CW_OUT_ELS_BUF *
		    sizeof(cw_out_el_t));
#ifdef _LIBSTASH_DBG
		memset(a_key->key_els_buf, 0, _CW_OUT_ELS_BUF *
		    sizeof(cw_out_el_t));
#endif
	}

	a_key->key_els[a_key->key_nels].el_type = a_type;
	a_key->key_els[a_key->key_nels].el_offset = a_offset;
	a_key->key_els[a_key->key_nels].el_len = a_len;
	a_key->key_nels++;

	retval = 0;
	RETURN:
	return retval;
}

static cw_out_ent_t *
out_p_ent_get(cw_out_t *a_out, const char *a_format, cw_uint32_t a_len)
{
	cw_out_ent_t	*retval;
	cw_uint32_t	i;

	_cw_check_ptr(a_format);
	_cw_assert(a_len > 0);

	/*
	 * Find a match for the type.  Use the first match found by searching
	 * the built in types, then the extended types.  If there is no match,
	 * return an error, since we have no way of knowing the size of argument
	 * to use.
	 */
	for (i = 0; i < (sizeof(cw_g_out_builtins) / sizeof(struct
	    cw_out_ent_s)); i++) {
		if ((strncmp(a_format, cw_g_out_builtins[i].type,
		    cw_g_out_builtins[i].len) == 0) && (a_len ==
		    cw_g_out_builtins[i].len)) {
			retval = &cw_g_out_builtins[i];
			goto RETURN;
		}
	}

	if (a_out != NULL) {
		for (i = 0; i < a_out->nextensions; i++) {
			if ((strncmp(a_format, a_out->extensions[i].type,
			    a_out->extensions[i].len) == 0) && (a_len ==
			    a_out->extensions[i].len)) {
				retval = &a_out->extensions[i];
				goto RETURN;
			}
		}
	}
	retval = NULL;

	RETURN:
	return retval;
}

static void
out_p_common_render(const char *a_format, cw_uint32_t a_len, cw_uint32_t
    a_max_len, cw_uint32_t a_rlen, cw_uint8_t *r_buf, cw_uint32_t *r_width,
    cw_uint32_t *r_owidth, cw_uint32_t *r_offset)
{
	cw_uint32_t	width, owidth, offset;
	cw_sint32_t	val_len;
	const cw_uint8_t *val;

	/*
	 * Calculate the width of what we'll print, assuming that it will fit in
	 * r_buf.
	 */
	if ((val_len = spec_get_val(a_format, a_len, "w", 1, &val)) != -1) {
		/* Width specified. */
		/*
		 * The next character after val is either `|' or `]', so we
		 * don't have to worry about terminating the string that val
		 * points to.
		 */
		width = strtoul(val, NULL, 10);
		if (width < a_rlen)
			width = a_rlen;
	} else
		width = a_rlen;

	/* Determine the total number of bytes to actually output. */
	if (width <= a_max_len)
		owidth = width;
	else
		owidth = a_max_len;

	if (width > a_rlen) {
		cw_uint8_t	pad, justify;

		/*
		 * Padding needed.  memset() the output string to the padding
		 * character, then determine where to render the integer based
		 * on justification.
		 */
		if ((val_len = spec_get_val(a_format, a_len, "p", 1, &val)) !=
		    -1)
			pad = val[0];
		else
			pad = ' ';

		memset(r_buf, pad, owidth);

		if ((val_len = spec_get_val(a_format, a_len, "j", 1, &val)) !=
		    -1)
			justify = val[0];
		else
			justify = 'r';

		switch (justify) {
		case 'r':
			offset = width - a_rlen;
			break;
		case 'l':
			offset = 0;
			break;
		case 'c':
			offset = (width - a_rlen) / 2;
			break;
		default:
			_cw_error("Unknown justification");
		}
	} else
		offset = 0;

	*r_width = width;
	*r_owidth = owidth;
	*r_offset = offset;
}

static cw_uint32_t
out_p_int_render(const char *a_format, cw_uint32_t a_len, cw_uint64_t a_arg,
    cw_uint32_t a_max_len, cw_uint8_t *r_buf, cw_uint32_t a_nbits, cw_uint32_t
    a_default_base)
{
	cw_uint32_t	base, olen, rlen, owidth, width, offset, i;
	cw_sint32_t	val_len;
	cw_uint64_t	arg = a_arg;
	cw_bool_t	is_negative, show_sign;
	const cw_uint8_t *val;
	cw_uint8_t	*syms = "0123456789abcdefghijklmnopqrstuvwxyz";
	cw_uint8_t	*result, s_result[66] =
	    "00000000000000000000000000000000000000000000000000000000000000000";

	_cw_assert((a_nbits == 8) || (a_nbits == 16) || (a_nbits == 32) ||
	    (a_nbits == 64));

	/*
	 * Move the pointer forward so that unnecessary digits can be ignored.
	 */
	result = &s_result[65 - a_nbits];

	if ((val_len = spec_get_val(a_format, a_len, "b", 1, &val)) != -1) {
		/* Base specified. */
		/*
		 * The next character after val is either `|' or `]', so we
		 * don't have to worry about terminating the string that val
		 * points to.
		 */
		base = strtoul(val, NULL, 10);
		_cw_assert(2 <= base);
		_cw_assert(36 >= base);
	} else
		base = a_default_base;

	/* Determine sign. */
	if (((val_len = spec_get_val(a_format, a_len, "s", 1, &val)) != -1) &&
	    (val[0] == 's') && ((arg & (((cw_uint64_t)1) << (a_nbits - 1))) !=
	    0)) {
		is_negative = TRUE;
		/* Convert two's complement to positive. */
		arg ^= ((cw_uint64_t)0xffffffff << 32) + 0xffffffff;
		arg++;
	} else
		is_negative = FALSE;

	/* Should we show the sign if the number is positive? */
	if ((((val_len = spec_get_val(a_format, a_len, "+", 1, &val)) != -1) &&
	    (val[0]) == '+') || (is_negative))
		show_sign = TRUE;
	else
		show_sign = FALSE;

	/*
	 * Treat 64 bit numbers separately, since they're much slower on 32 bit
	 * architectures.
	 *
	 * Treat base 16 numbers specially, since they can be rendered more
	 * quickly with special case code, and they tend to be used a lot.
	 */
	if (arg == 0)
		result += a_nbits - 1;
	else if (a_nbits != 64) {
		cw_uint32_t	rval = (cw_uint32_t)arg;

		if (base == 16) {
			for (i = a_nbits - 1; rval != 0; i--) {
				result[i] = syms[rval & 0xf];
				rval >>= 4;
			}
		} else {
			for (i = a_nbits - 1; rval != 0; i--) {
				result[i] = syms[rval % base];
				rval /= base;
			}
		}
		result += i + 1;
	} else {
		cw_uint64_t	rval = arg;

		if (base == 16) {
			for (i = 63; rval != 0; i--) {
				result[i] = syms[rval & 0xf];
				rval >>= 4;
			}
		} else {
			for (i = 63; rval != 0; i--) {
				result[i] = syms[rval % base];
				rval /= base;
			}
		}
		result += i + 1;
	}

	if (show_sign) {
		result--;
		result[0] = (is_negative) ? '-' : '+';
	}
	rlen = &s_result[65] - result;
	_cw_assert(rlen <= 65);

	out_p_common_render(a_format, a_len, a_max_len, rlen, r_buf, &width,
	    &owidth, &offset);

	if (offset < owidth) {
		if (offset + rlen <= owidth)
			olen = rlen;
		else
			olen = owidth - offset;
		memcpy(&r_buf[offset], result, olen);
	}
	return width;
}

static cw_uint32_t
out_p_int32_render(const char *a_format, cw_uint32_t a_len, const void *a_arg,
    cw_uint32_t a_max_len, cw_uint8_t *r_buf)
{
	cw_uint32_t	retval;
	cw_uint64_t	arg;

	_cw_check_ptr(a_format);
	_cw_assert(a_len > 0);
	_cw_check_ptr(a_arg);
	_cw_check_ptr(r_buf);

	arg = (cw_uint64_t)*(const cw_uint32_t *)a_arg;

	retval = out_p_int_render(a_format, a_len, arg, a_max_len, r_buf, 32,
	    10);

	return retval;
}

static cw_uint32_t
out_p_int64_render(const char *a_format, cw_uint32_t a_len, const void *a_arg,
    cw_uint32_t a_max_len, cw_uint8_t *r_buf)
{
	cw_uint32_t	retval;
	cw_uint64_t	arg;

	_cw_check_ptr(a_format);
	_cw_assert(a_len > 0);
	_cw_check_ptr(a_arg);
	_cw_check_ptr(r_buf);

	arg = *(const cw_uint64_t *)a_arg;

	retval = out_p_int_render(a_format, a_len, arg, a_max_len, r_buf, 64,
	    10);

	return retval;
}

static cw_uint32_t
out_p_char_render(const char *a_format, cw_uint32_t a_len, const void *a_arg,
    cw_uint32_t a_max_len, cw_uint8_t *r_buf)
{
	cw_uint32_t	rlen, owidth, width, offset;
	cw_uint8_t	c;

	_cw_check_ptr(a_format);
	_cw_assert(a_len > 0);
	_cw_check_ptr(a_arg);
	_cw_check_ptr(r_buf);

	c = *(const cw_uint32_t *)a_arg;

	rlen = 1;

	out_p_common_render(a_format, a_len, a_max_len, rlen, r_buf, &width,
	    &owidth, &offset);

	if (offset < owidth)
		r_buf[offset] = c;

	return width;
}

static cw_uint32_t
out_p_string_render(const char *a_format, cw_uint32_t a_len, const void *a_arg,
    cw_uint32_t a_max_len, cw_uint8_t *r_buf)
{
	cw_sint32_t	val_len;
	const cw_uint8_t *val;
	cw_uint32_t	olen, rlen, owidth, width, offset;
	const char	*str;

	_cw_check_ptr(a_format);
	_cw_assert(a_len > 0);
	_cw_check_ptr(a_arg);
	_cw_check_ptr(r_buf);

	str = *(const char **)a_arg;

	rlen = strlen(str);

	if ((val_len = spec_get_val(a_format, a_len, "w", 1, &val)) != -1) {
		/* Width specified. */
		/*
		 * The next character after val is either `|' or `]', so we
		 * don't have to worry about terminating the string that val
		 * points to.
		 */
		width = strtoul(val, NULL, 10);
		if (width < rlen)
			width = rlen;
	} else
		width = rlen;

	out_p_common_render(a_format, a_len, a_max_len, rlen, r_buf, &width,
	    &owidth, &offset);

	if (offset < owidth) {
		if (offset + rlen <= owidth)
			olen = rlen;
		else
			olen = owidth - offset;
		memcpy(&r_buf[offset], str, olen);
	}

	return width;
}

static cw_uint32_t
out_p_pointer_render(const char *a_format, cw_uint32_t a_len, const void *a_arg,
    cw_uint32_t a_max_len, cw_uint8_t *r_buf)
{
	cw_uint32_t	retval;

	_cw_check_ptr(a_format);
	_cw_assert(a_len > 0);
	_cw_check_ptr(a_arg);
	_cw_check_ptr(r_buf);

#if (SIZEOF_INT_P == 4)
	{
		cw_uint64_t	arg;
		arg = (cw_uint64_t)(cw_uint32_t)*(const void **)a_arg;

		retval = out_p_int_render(a_format, a_len, arg, a_max_len,
		    r_buf, 32, 16);
	}
#elif (SIZEOF_INT_P == 8)
	{
		cw_uint64_t	arg;
		arg = (cw_uint64_t)*(const void **)a_arg;

		retval = out_p_int_render(a_format, a_len, arg, a_max_len,
		    r_buf, 64, 16);
	}
#else
#error Unsupported pointer size
#endif

	return retval;
}

static cw_uint32_t
out_p_buf_render(const char *a_format, cw_uint32_t a_len, const void *a_arg,
    cw_uint32_t a_max_len, cw_uint8_t *r_buf)
{
	cw_sint32_t	val_len;
	const cw_uint8_t *val;
	cw_uint32_t	olen, rlen, owidth, width, offset;
	cw_buf_t	*buf;
	const struct iovec *iov;
	int		iov_cnt, i;

	_cw_check_ptr(a_format);
	_cw_assert(a_len > 0);
	_cw_check_ptr(a_arg);
	_cw_check_ptr(r_buf);

	buf = *(cw_buf_t **)a_arg;
	_cw_check_ptr(buf);

	rlen = buf_size_get(buf);

	if ((val_len = spec_get_val(a_format, a_len, "w", 1, &val)) != -1) {
		/* Width specified. */
		/*
		 * The next character after val is either `|' or `]', so we
		 * don't have to worry about terminating the string that val
		 * points to.
		 */
		width = strtoul(val, NULL, 10);
		if (width < rlen)
			width = rlen;
	} else
		width = rlen;

	out_p_common_render(a_format, a_len, a_max_len, rlen, r_buf, &width,
	    &owidth, &offset);

	if (offset < owidth) {
		/*
		 * Copy bytes from the buf to the output string.  Use the buf's
		 * iovec and memcpy for efficiency.
		 */
		if (offset + rlen <= owidth)
			olen = rlen;
		else
			olen = owidth - offset;
		iov = buf_iovec_get(buf, olen, FALSE, &iov_cnt);
		r_buf += offset;
		for (i = 0; i < iov_cnt; i++) {
			memcpy(r_buf, iov[i].iov_base, iov[i].iov_len);
			r_buf += iov[i].iov_len;
		}
	}

	return width;
}
