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

/* Defined in kasio.h to resolve a circular dependency. */
#if (0)
typedef struct cw_kasioe_number_s cw_kasioe_number_t;
#endif

struct cw_kasioe_number_s
{
  cw_kasioe_t kasioe;
  /* Offset in val that the "decimal point" precedes. */
  cw_uint32_t point;
  /* Base.  Can be from 2 to 36, inclusive. */
  cw_uint32_t base;
  /* Number of bytes that val points to. */
  cw_uint32_t val_len;
  /* Offset of most significant non-zero digit. */
  cw_uint32_t val_msd;
  /* The least significant digit is at val[0].  Each byte can range in value
   * from 0 to 35, depending on the base.  This representation is not compact,
   * but it is easy to work with. */
  cw_uint8_t * val;
};

cw_kasioe_number_t *
kasioe_number_new(cw_kasioe_number_t * a_kasioe_number);

void
kasioe_number_delete(cw_kasioe_number_t * a_kasioe_number);
