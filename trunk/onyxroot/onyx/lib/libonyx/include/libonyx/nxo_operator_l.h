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

void	nxo_l_operator_print(cw_nxo_t *a_thread);
#define	nxo_l_operator_fast_op_get(a_nxo) (a_nxo)->fast_op
#define	nxo_l_operator_fast_op_set(a_nxo, a_op_code) do {		\
	(a_nxo)->fast_op = TRUE;					\
	(a_nxo)->op_code = (a_op_code);					\
} while (0)
#define	nxo_l_operator_fast_op_nxn(a_nxo) (a_nxo)->op_code
