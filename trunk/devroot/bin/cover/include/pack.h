/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Packed matrices.
 *
 ******************************************************************************/

typedef struct
{
	cw_uint32_t	x_size, y_size;
	cw_uint32_t	*pack;
} pack_t;

/*
 * a : (pack_t *).
 * x : (cw_uint32_t) x size.
 * y : (cw_uint32_t) y size.
 */
#define	pack_new(a, x, y) do {						\
		(a)->x_size = (x);					\
		(a)->y_size = ((y) >> 5) + (((y) & 0x1f) ? 1 : 0);	\
		(a)->pack = (cw_uint32_t *) _cw_calloc((a)->x_size *	\
		    (a)->y_size, sizeof(cw_uint32_t));			\
		bzero((a)->pack, (a)->x_size * (a)->y_size		\
		    * sizeof(cw_uint32_t));				\
	} while (0)

/* a : (pack_t *). */
#define	pack_delete(a) _cw_free((a)->pack)

/*
 * a : (pack_t).
 * x : (cw_uint32_t) x position.
 * y : (cw_uint32_t) y position.
 */
#define	pack_get_el(a, x, y)						\
	(((a).pack[((x) * (a).y_size) + ((y) >> 5)] >> ((y) & 0x1f)) & 0x1)

/*
 * a : (pack_t).
 * x : (cw_uint32_t) x position.
 * y : (cw_uint32_t) y position.
 */
#define	pack_set_el(a, x, y)						\
	(a).pack[((x) * (a).y_size) + ((y) >> 5)] |= (0x1 << ((y) & 0x1f))

/*
 * a : (pack_t).
 * x : (cw_uint32_t) x position.
 * y : (cw_uint32_t) y position.
 */
#define	pack_unset_el(a, x, y)						\
	(a).pack[((x) * (a).y_size) + ((y) >> 5)] &= (~(0x1 << ((y) & 0x1f)))
