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
#define	nxo_l_operator_fast_op_get(a_nxo) nxo_p_fastop_get(a_nxo)
#define	nxo_l_operator_fast_op_set(a_nxo, a_op_code) do {		\
	nxo_p_fastop_set((a_nxo), TRUE);				\
	nxo_p_opcode_set((a_nxo), (a_op_code));				\
} while (0)
#define	nxo_l_operator_fast_op_nxn(a_nxo) nxo_p_opcode_get(a_nxo)
