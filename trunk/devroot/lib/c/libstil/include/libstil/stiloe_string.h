/****************************************************************************
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
typedef struct cw_stiloe_string_s cw_stiloe_string_t;
#endif

struct cw_stiloe_string_s {
	cw_stiloe_t	stiloe;
	union {
		cw_stiloec_t	stiloec;
		struct {
			cw_uint8_t	*str;
			cw_sint32_t	len;	/* -1 if unset. */
		}	s;
	}	e;
};

cw_stiloe_string_t *stiloe_string_new(cw_stilt_t *a_stilt);

void		stiloe_string_delete(cw_stiloe_string_t *a_stiloe_string);

cw_sint32_t	stiloe_string_len_get(cw_stiloe_string_t *a_stiloe_string);

void		stiloe_string_len_set(cw_stiloe_string_t *a_stiloe_string,
    cw_stilt_t *a_stilt, cw_uint32_t a_len);

void		stiloe_string_set(cw_stiloe_string_t *a_stiloe_string,
    cw_uint32_t a_offset, const char *a_str, cw_uint32_t a_len);
