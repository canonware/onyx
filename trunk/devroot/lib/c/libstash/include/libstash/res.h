/****************************************************************************
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
typedef struct cw_res_s cw_res_t;

struct cw_res_s {
	cw_bool_t	is_malloced;
	cw_rwl_t	rw_lock;
	cw_dch_t	hash;
	FILE		*fd;
	char		*str;
};

cw_res_t	*res_new(cw_res_t *a_res);

void		res_delete(cw_res_t *a_res);

void		res_clear(cw_res_t *a_res);

cw_bool_t	res_is_equal(cw_res_t *a_a, cw_res_t *a_b);

cw_bool_t	res_merge_file(cw_res_t *a_res, const char *a_filename);

cw_bool_t	res_merge_list(cw_res_t *a_res,...);

const char	*res_get_res_val(cw_res_t *a_res, const char *a_res_name);

cw_bool_t	res_extract_res(cw_res_t *a_res, const char *a_res_key, char
    **r_res_name, char **r_res_val);

cw_bool_t	res_dump(cw_res_t *a_res, const char *a_filename);
