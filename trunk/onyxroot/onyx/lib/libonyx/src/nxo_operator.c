/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#define CW_NXO_OPERATOR_C_

#include "../include/libonyx/libonyx.h"
#include "../include/libonyx/nxo_l.h"
#include "../include/libonyx/nxo_operator_l.h"

void
nxo_operator_new(cw_nxo_t *a_nxo, cw_op_t *a_op, cw_nxn_t a_nxn)
{
    nxo_p_new(a_nxo, NXOT_OPERATOR);
    a_nxo->o.operator.f = a_op;
    nxo_p_opcode_set(a_nxo, a_nxn);
}
