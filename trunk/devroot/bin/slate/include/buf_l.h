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

void bufm_l_insert(cw_bufm_t *a_bufm, cw_bool_t a_record, cw_bool_t a_after,
    const cw_bufv_t a_bufv, cw_uint32_t a_bufvcnt, cw_uint32_t a_elmsize);
void bufm_l_remove(cw_bufm_t *a_start, cw_bufm_t *a_end, cw_bool_t a_record);
