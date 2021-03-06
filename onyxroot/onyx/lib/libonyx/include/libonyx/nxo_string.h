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

void
nxo_string_new(cw_nxo_t *a_nxo, bool a_locking, uint32_t a_len);

void
nxo_string_substring_new(cw_nxo_t *a_nxo, cw_nxo_t *a_string,
			 uint32_t a_offset, uint32_t a_len);

void
nxo_string_copy(cw_nxo_t *a_to, cw_nxo_t *a_from);

void
nxo_string_cstring(cw_nxo_t *a_to, cw_nxo_t *a_from, cw_nxo_t *a_thread);

uint32_t
nxo_string_len_get(const cw_nxo_t *a_nxo);

void
nxo_string_el_get(const cw_nxo_t *a_nxo, cw_nxoi_t a_offset, char *r_el);

void
nxo_string_el_set(cw_nxo_t *a_nxo, char a_el, cw_nxoi_t a_offset);

#ifdef CW_THREADS
void
nxo_string_lock(cw_nxo_t *a_nxo);

void
nxo_string_unlock(cw_nxo_t *a_nxo);
#else
#define nxo_string_lock(a_nxo)
#define nxo_string_unlock(a_nxo)
#endif

char *
nxo_string_get(const cw_nxo_t *a_nxo);

void
nxo_string_set(cw_nxo_t *a_nxo, uint32_t a_offset, const char *a_str,
	       uint32_t a_len);
