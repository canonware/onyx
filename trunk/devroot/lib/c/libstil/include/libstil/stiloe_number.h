/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 ****************************************************************************/

/* Defined in stilo.h to resolve a circular dependency. */
#if (0)
typedef struct cw_stiloe_number_s cw_stiloe_number_t;

#endif

struct cw_stiloe_number_s {
	cw_stiloe_t stiloe;
	/* Offset in val that the "decimal point" precedes. */
	cw_uint32_t point;
	/* Base.  Can be from 2 to 36, inclusive. */
	cw_uint32_t base;
	/* Number of bytes that val points to. */
	cw_uint32_t val_len;
	/* Offset of most significant non-zero digit. */
	cw_uint32_t val_msd;
	/*
	 * The least significant digit is at val[0].  Each byte can range in
	 * value from 0 to 35, depending on the base.  This representation
	 * is not compact, but it is easy to work with.
	 */
	cw_uint8_t *val;
};

cw_stiloe_number_t *stiloe_number_new(cw_stiloe_number_t *a_stiloe_number);

void    stiloe_number_delete(cw_stiloe_number_t *a_stiloe_number);
