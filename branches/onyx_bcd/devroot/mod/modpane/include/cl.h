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

typedef struct cw_cl_s cw_cl_t;

struct cw_cl_s {
    /* Character value. */
    cw_uint8_t c:8;

    /* Color pair. */
    cw_uint16_t color:16;

    /* Reverse video? */
    cw_bool_t reverse:1;

    /* Bold? */
    cw_bool_t bold:1;

    /* Underline? */
    cw_bool_t underline:1;
};

void
cl_new(cw_cl_t *a_cl);

void
cl_delete(cw_cl_t *a_cl);
