/******************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Operations on genes.  These are all macros for performance reasons.
 *
 ******************************************************************************/

typedef struct
{
	cw_uint32_t	arr_len;
	char		*gene;
	cw_uint32_t	score;
	cw_uint32_t	total;
} gene_t;

/*
 * a : (gene_t *).
 * b : (cw_uint32_t) Number of loci.
 */
#define	gene_new(a, b) do {						\
		(a)->arr_len = ((b) >> 3) + (((b) & 0x7) ? 1 : 0);	\
		(a)->gene = (char *)_cw_malloc((a)->arr_len);		\
	} while (0)

/* a : (gene_t *). */
#define	gene_delete(a) _cw_free((a)->gene);

/*
 * Copy b to a.
 * a : (gene_t *).
 * b : (gene_t *).
 */
#define	gene_copy(a, b) memcpy((a)->gene, (b)->gene, (a)->arr_len)

/* a : (gene_t *). */
#define	gene_clear(a) bzero((a)->gene, (a)->arr_len)

/*
 * a : (gene_t *).
 * b : (cw_uint32_t) locus.
 */
#define	gene_get_locus(a, b) !!((a)->gene[((b) >> 3)] & (0x1 << ((b) & 0x7)))

/*
 * a : (gene_t *).
 * b : (cw_uint32_t) locus.
 * c : (cw_uint32_t) value (0 or 1).
 */
#define	gene_set_locus(a, b, c)						\
	(a)->gene[((b) >> 3)] =						\
	    ( ((a)->gene[((b) >> 3)] & (~(0x1 << ((b) & 0x7))))		\
	    | ((c) << ((b) & 0x7)) )
