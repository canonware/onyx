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

/*
 * Private, but needed below.
 */
#define	nxo_p_opcode_get(a_nxo) (((a_nxo)->flags >> 8) & 0x3ff)
#define	nxo_p_opcode_set(a_nxo, a_opcode) do {				\
	(a_nxo)->flags = ((a_nxo)->flags & 0xfffc00ff) |		\
	    ((a_opcode) << 8);						\
} while (0)

#define	nxo_p_fastop_get(a_nxo) (((a_nxo)->flags >> 5) & 1)
#define	nxo_p_fastop_set(a_nxo, a_fastop) do {				\
	(a_nxo)->flags = ((a_nxo)->flags & 0xffffffdf) |		\
	    ((a_fastop) << 5);						\
} while (0)

/* Library-private. */
#define	nxo_l_operator_fast_op_get(a_nxo) nxo_p_fastop_get(a_nxo)
#define	nxo_l_operator_fast_op_set(a_nxo, a_op_code) do {		\
	nxo_p_fastop_set((a_nxo), TRUE);				\
	nxo_p_opcode_set((a_nxo), (a_op_code));				\
} while (0)
#define	nxo_l_operator_fast_op_nxn(a_nxo) nxo_p_opcode_get(a_nxo)
