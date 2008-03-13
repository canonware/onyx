/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: Onyx <Version = onyx>
 *
 ******************************************************************************/

/* nxo. */
#define nxo_p_type_set(a_nxo, a_type)					\
    do									\
    {									\
	mb_write();							\
	(a_nxo)->flags = ((a_nxo)->flags & 0xffffffe0) | (a_type);	\
    } while (0)

/* nxoe. */
void
nxoe_l_new(cw_nxoe_t *a_nxoe, cw_nxot_t a_type, bool a_locking);

#define nxoe_l_color_get(a_nxoe) (a_nxoe)->color
#define nxoe_l_color_set(a_nxoe, a_color) (a_nxoe)->color = (a_color)

#define nxoe_l_registered_get(a_nxoe) (a_nxoe)->registered
#define nxoe_l_registered_set(a_nxoe, a_registered)			\
	(a_nxoe)->registered = (a_registered)

#define nxoe_l_type_get(a_nxoe) (a_nxoe->type)
