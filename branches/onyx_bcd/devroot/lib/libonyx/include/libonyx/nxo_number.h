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

typedef struct cw_nxoe_number_s cw_nxoe_number_t;

/* Arbitrary precision base 10 floating point numbers, with some implementation
 * limits. */
struct cw_nxoe_thread_s
{
    cw_nxoe_t nxoe;

    /* Sign of number. */
    cw_bool_t neg;

    /* Coefficient, with up to 2^32 - 1 digits of significance.  There can be
     * leading and trailing 0's, which preserve significant digits. */
    cw_uint8_t *coef;
    cw_uint32_t coef_len;

    /* Base 10 exponent, where {exp: -(2^63) <= exp <= 2^63 - 1}. */
    cw_sint64_t exp;
};

void
nxo_number_new(cw_nxo_t *a_nxo, cw_bool_t a_neg,
	       const cw_uint8_t *a_whole, cw_uint32_t a_whole_len,
	       const cw_uint8_t *a_frac, cw_uint32_t a_frac_len,
	       cw_sint32_t a_exp);

void
nxo_number_int_new(cw_nxo_t *a_nxo, cw_sint32_t a_val);

cw_bool_t
nxo_number_negative(cw_nxo_t *a_nxo);
cw_bool_t
nxo_number_integer(cw_nxo_t *a_nxo);

cw_bool_t
nxo_number_sint32_cast(cw_nxo_t *a_nxo, cw_sint32_t *r_int);
cw_bool_t
nxo_number_uint32_cast(cw_nxo_t *a_nxo, cw_uint32_t *r_int);
cw_bool_t
nxo_number_sint64_cast(cw_nxo_t *a_nxo, cw_sint64_t *r_int);
cw_bool_t
nxo_number_uint64_cast(cw_nxo_t *a_nxo, cw_uint64_t *r_int);

cw_sint32_t
nxo_number_compare(cw_nxo_t *a_a, cw_nxo_t *a_b);

void
nxo_number_abs(cw_nxo_t *a_nxo, cw_nxo_t *a_thread);
void
nxo_number_neg(cw_nxo_t *a_nxo, cw_nxo_t *a_thread);
void
nxo_number_ceiling(cw_nxo_t *a_nxo, cw_nxo_t *a_dec, cw_nxo_t *a_thread);
void
nxo_number_floor(cw_nxo_t *a_nxo, cw_nxo_t *a_dec, cw_nxo_t *a_thread);
void
nxo_number_truncate(cw_nxo_t *a_nxo, cw_nxo_t *a_dec, cw_nxo_t *a_thread);
void
nxo_number_round(cw_nxo_t *a_nxo, cw_nxo_t *a_dec, cw_nxo_t *a_thread);

cw_nxn_t
nxo_number_add(cw_nxo_t *a_a, cw_nxo_t *a_b, cw_nxo_t *a_thread, cw_nxo_t *r_r);
cw_nxn_t
nxo_number_sub(cw_nxo_t *a_a, cw_nxo_t *a_b, cw_nxo_t *a_thread, cw_nxo_t *r_r);
cw_nxn_t
nxo_number_mul(cw_nxo_t *a_a, cw_nxo_t *a_b, cw_nxo_t *a_thread, cw_nxo_t *r_r);
cw_nxn_t
nxo_number_div(cw_nxo_t *a_a, cw_nxo_t *a_b, cw_nxo_t *a_thread, cw_nxo_t *r_r);
cw_nxn_t
nxo_number_mod(cw_nxo_t *a_a, cw_nxo_t *a_b, cw_nxo_t *a_thread, cw_nxo_t *r_r);
cw_nxn_t
nxo_number_exp(cw_nxo_t *a_a, cw_nxo_t *a_b, cw_nxo_t *a_thread, cw_nxo_t *r_r);
cw_nxn_t
nxo_number_sqrt(cw_nxo_t *a_nxo, cw_nxo_t *a_thread, cw_nxo_t *r_r);

cw_nxn_t
nxo_number_sin(cw_nxo_t *a_nxo, cw_nxo_t *a_thread, cw_nxo_t *r_r);
cw_nxn_t
nxo_number_cos(cw_nxo_t *a_nxo, cw_nxo_t *a_thread, cw_nxo_t *r_r);
cw_nxn_t
nxo_number_atan(cw_nxo_t *a_num, cw_nxo_t *a_den, cw_nxo_t *a_thread,
		cw_nxo_t *r_r);
cw_nxn_t
nxo_number_ln(cw_nxo_t *a_nxo, cw_nxo_t *a_thread, cw_nxo_t *r_r);
cw_nxn_t
nxo_number_log(cw_nxo_t *a_nxo, cw_nxo_t *a_thread, cw_nxo_t *r_r);
