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

typedef struct cw_stiloe_string_s cw_stiloe_string_t;

struct cw_stiloe_string_s {
	cw_stiloe_t	stiloe;
	/*
	 * Access is locked if this object has the locking bit set.  Indirect
	 * strings aren't locked, but their parents are.
	 */
	cw_mtx_t	lock;
	/*
	 * Used for remembering the current state of reference iteration.
	 */
	cw_uint32_t	ref_iter;
	union {
		struct {
			cw_stilo_t	stilo;
			cw_uint32_t	beg_offset;
			cw_uint32_t	len;
		}	i;
		struct {
			cw_uint8_t	*str;
			cw_sint32_t	len;
		}	s;
	}	e;
};

void	stiloe_l_string_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil);
cw_stiloe_t *stiloe_l_string_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t a_reset);
cw_stilte_t stilo_l_string_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file,
    cw_uint32_t a_depth);
