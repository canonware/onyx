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

cw_stilo_threade_t stilo_l_operator_print(cw_stilo_t *a_stilo, cw_stilo_t
    *a_file, cw_uint32_t a_depth);
#define	stilo_l_operator_fast_op_get(a_stilo) (a_stilo)->fast_op
#define	stilo_l_operator_fast_op_set(a_stilo, a_op_code) do {		\
	(a_stilo)->fast_op = TRUE;					\
	(a_stilo)->op_code = (a_op_code);				\
} while (0)
#define	stilo_l_operator_fast_op_stiln(a_stilo) (a_stilo)->op_code
