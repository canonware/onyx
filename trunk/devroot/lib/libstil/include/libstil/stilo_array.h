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

void		stilo_array_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil,
    cw_bool_t a_locking, cw_uint32_t a_len);
void		stilo_array_subarray_new(cw_stilo_t *a_stilo, cw_stilo_t
    *a_array, cw_stil_t *a_stil, cw_uint32_t a_offset, cw_uint32_t a_len);
void		stilo_array_copy(cw_stilo_t *a_to, cw_stilo_t *a_from);
cw_uint32_t	stilo_array_len_get(cw_stilo_t *a_stilo);
void		stilo_array_el_get(cw_stilo_t *a_stilo, cw_stiloi_t a_offset,
    cw_stilo_t *r_el);
void		stilo_array_el_set(cw_stilo_t *a_stilo, cw_stilo_t *a_el,
    cw_stiloi_t a_offset);
