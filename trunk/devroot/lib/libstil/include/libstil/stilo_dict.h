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

typedef struct cw_stiloe_dicto_s cw_stiloe_dicto_t;
typedef struct cw_stiloe_dict_s cw_stiloe_dict_t;

void		stilo_dict_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil, cw_bool_t
    a_locking, cw_uint32_t a_dict_size);
void		stilo_dict_copy(cw_stilo_t *a_to, cw_stilo_t *a_from,
    cw_stil_t *a_stil, cw_bool_t a_locking);
void		stilo_dict_def(cw_stilo_t *a_stilo, cw_stil_t *a_stil,
    cw_stilo_t *a_key, cw_stilo_t *a_val);
void		stilo_dict_undef(cw_stilo_t *a_stilo, cw_stil_t *a_stil, const
    cw_stilo_t *a_key);
cw_bool_t	stilo_dict_lookup(cw_stilo_t *a_stilo, const cw_stilo_t *a_key,
    cw_stilo_t *r_stilo);
cw_uint32_t	stilo_dict_count(cw_stilo_t *a_stilo);
void		stilo_dict_iterate(cw_stilo_t *a_stilo, cw_stilo_t *r_stilo);
