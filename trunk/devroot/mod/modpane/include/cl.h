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

/* Cell attributes.  A cell is a 32 bit unsigned integer, with the following
 * layout:
 *
 *   CCCCCCCC CCCCCCCC rfisdbua SSSSSSSS
 *
 *   C : Color pair.
 *   r : Reverse video?
 *   f : Flash?
 *   i : Invisible?
 *   s : Standout?
 *   d : Dim?
 *   b : Bold?
 *   u : Underline?
 *   a : Alternate character set?
 *   S : Symbol (character).
 */
#define CLA_REVERSE	((cw_uint32_t)0x00008000)
#define CLA_FLASH	((cw_uint32_t)0x00004000)
#define CLA_INVISIBLE	((cw_uint32_t)0x00002000)
#define CLA_STANDOUT	((cw_uint32_t)0x00001000)
#define CLA_DIM		((cw_uint32_t)0x00000800)
#define CLA_BOLD	((cw_uint32_t)0x00000400)
#define CLA_UNDERLINE	((cw_uint32_t)0x00000200)
#define CLA_ALTCHARSET	((cw_uint32_t)0x00000100)


typedef cw_uint32_t cw_cl_t;

#ifndef CW_USE_INLINES
void
cl_init(cw_cl_t *a_cl);

cw_uint8_t
cl_symbol_get(cw_cl_t *a_cl);

void
cl_symbol_set(cw_cl_t *a_cl, cw_uint8_t a_symbol);

cw_uint16_t
cl_colorpair_get(cw_cl_t *a_cl);

void
cl_colorpair_set(cw_cl_t *a_cl, cw_uint16_t a_colorpair);

cw_bool_t
cl_attr_get(cw_cl_t *a_cl, cw_uint32_t a_attr);

void
cl_attrs_set(cw_cl_t *a_cl, cw_uint32_t a_attrs);

void
cl_attrs_unset(cw_cl_t *a_cl, cw_uint32_t a_attrs);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_CL_C_))
CW_INLINE void
cl_init(cw_cl_t *a_cl)
{
    *a_cl = 0;
}

CW_INLINE cw_uint8_t
cl_symbol_get(cw_cl_t *a_cl)
{
    return ((*a_cl) & 0xff);
}

CW_INLINE void
cl_symbol_set(cw_cl_t *a_cl, cw_uint8_t a_symbol)
{
    *a_cl = (*a_cl & 0xffffff00) | ((cw_uint32_t)a_symbol);
}

CW_INLINE cw_uint16_t
cl_colorpair_get(cw_cl_t *a_cl)
{
    return ((*a_cl) >> 16);
}

CW_INLINE void
cl_colorpair_set(cw_cl_t *a_cl, cw_uint16_t a_colorpair)
{
    *a_cl = (*a_cl & 0xffff) | ((cw_uint32_t)a_colorpair);
}

CW_INLINE cw_bool_t
cl_attr_get(cw_cl_t *a_cl, cw_uint32_t a_attr)
{
#ifdef CW_DBG
    switch (a_attr)
    {
	case CLA_REVERSE:
	case CLA_FLASH:
	case CLA_INVISIBLE:
	case CLA_STANDOUT:
	case CLA_DIM:
	case CLA_BOLD:
	case CLA_UNDERLINE:
	case CLA_ALTCHARSET:
	{
	    break;
	}
	default:
	{
	    cw_error("Illegal attribute");
	}
    }
#endif
    return (*a_cl & a_attr);
}

CW_INLINE void
cl_attrs_set(cw_cl_t *a_cl, cw_uint32_t a_attrs)
{
    cw_assert((a_attrs & 0xffff00ff) == 0);
    *a_cl |= a_attrs;
}

CW_INLINE void
cl_attrs_unset(cw_cl_t *a_cl, cw_uint32_t a_attrs)
{
    cw_assert((a_attrs & 0xffff00ff) == 0);
    *a_cl &= ~a_attrs;
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_CL_C_)) */
