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

void		stilo_string_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil,
    cw_bool_t a_locking, cw_uint32_t a_len);
void		stilo_string_substring_new(cw_stilo_t *a_stilo, cw_stilo_t
    *a_string, cw_stil_t *a_stil, cw_uint32_t a_offset, cw_uint32_t a_len);
void		stilo_string_copy(cw_stilo_t *a_to, cw_stilo_t *a_from);
cw_uint32_t	stilo_string_len_get(cw_stilo_t *a_stilo);
void		stilo_string_el_get(cw_stilo_t *a_stilo, cw_stiloi_t a_offset,
    cw_uint8_t *r_el);
void		stilo_string_el_set(cw_stilo_t *a_stilo, cw_uint8_t a_el,
    cw_stiloi_t a_offset);
void		stilo_string_lock(cw_stilo_t *a_stilo);
void		stilo_string_unlock(cw_stilo_t *a_stilo);
cw_uint8_t	*stilo_string_get(cw_stilo_t *a_stilo);
void		stilo_string_set(cw_stilo_t *a_stilo, cw_uint32_t a_offset,
    const cw_uint8_t *a_str, cw_uint32_t a_len);
