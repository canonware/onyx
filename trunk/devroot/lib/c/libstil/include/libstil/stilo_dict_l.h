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

/* This is private, but stila needs to know its size. */
struct cw_stiloe_dicto_s {
	cw_stilo_t	key;
	cw_stilo_t	val;
};

struct cw_stiloe_dict_s {
	cw_stiloe_t	stiloe;
	/*
	 * Access is locked if this object has the locking bit set.
	 */
	cw_mtx_t	lock;
	/*
	 * Used for remembering the current state of reference iteration.
	 */
	cw_uint32_t	ref_iter;
	/*
	 * If non-NULL, the previous reference iteration returned the key of
	 * this dicto, so the value of this dicto is the next reference to
	 * check.
	 */
	cw_stiloe_dicto_t *dicto;
	/*
	 * Name/value pairs.  The keys are (cw_stilo_t *), and the values are
	 * (cw_stiloe_dicto_t *).  The stilo that the key points to resides in
	 * the stiloe_dicto (value) structure.
	 */
	cw_dch_t	hash;
};

void	stiloe_l_dict_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil);
cw_stiloe_t *stiloe_l_dict_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t a_reset);
void	stilo_l_dict_print(cw_stilo_t *a_thread);
cw_stilo_t *stilo_l_dict_lookup(cw_stilo_t *a_stilo, const cw_stilo_t *a_key);
