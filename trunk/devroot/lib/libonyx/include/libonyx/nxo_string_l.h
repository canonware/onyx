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

typedef struct cw_nxoe_string_s cw_nxoe_string_t;

struct cw_nxoe_string_s {
	cw_nxoe_t	nxoe;
#ifdef _CW_THREADS
	/*
	 * Access is locked if this object has the locking bit set.  Indirect
	 * strings aren't locked, but their parents are.
	 */
	cw_mtx_t	lock;
#endif
	/*
	 * Used for remembering the current state of reference iteration.
	 */
	cw_uint32_t	ref_iter;
	union {
		struct {
			cw_nxo_t	nxo;
			cw_uint32_t	beg_offset;
			cw_uint32_t	len;
		}	i;
		struct {
			cw_uint8_t	*str;
			cw_uint32_t	len;
			cw_uint32_t	alloc_len;
		}	s;
	}	e;
};

cw_bool_t nxoe_l_string_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t
    a_iter);
cw_nxoe_t *nxoe_l_string_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);
