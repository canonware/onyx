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

typedef int32_t cw_nxo_file_read_t (void *a_arg, cw_nxo_t *a_file,
				    uint32_t a_len, uint8_t *r_str);

typedef bool cw_nxo_file_write_t (void *a_arg, cw_nxo_t *a_file,
				  const uint8_t *a_str,
				  uint32_t a_len);

typedef cw_nxoe_t *cw_nxo_file_ref_iter_t (void *a_arg, bool a_reset);

typedef void cw_nxo_file_delete_t (void *a_arg);

void
nxo_file_new(cw_nxo_t *a_nxo, bool a_locking);

#ifdef CW_POSIX_FILE
void
nxo_file_fd_wrap(cw_nxo_t *a_nxo, uint32_t a_fd, bool a_close);
#endif

void
nxo_file_synthetic(cw_nxo_t *a_nxo, cw_nxo_file_read_t *a_read,
		   cw_nxo_file_write_t *a_write,
		   cw_nxo_file_ref_iter_t *a_ref_iter,
		   cw_nxo_file_delete_t *a_delete, void *a_arg);

#ifdef CW_POSIX_FILE
cw_nxn_t
nxo_file_open(cw_nxo_t *a_nxo, const uint8_t *a_filename, uint32_t a_nlen,
	      const uint8_t *a_flags, uint32_t a_flen, mode_t a_mode);
#endif

cw_nxn_t
nxo_file_close(cw_nxo_t *a_nxo);

void
nxo_file_origin_get(const cw_nxo_t *a_nxo, const uint8_t **r_origin,
		    uint32_t *r_olen);

void
nxo_file_origin_set(cw_nxo_t *a_nxo, const uint8_t *a_origin,
		    uint32_t a_olen);

#ifdef CW_POSIX_FILE
int32_t
nxo_file_fd_get(const cw_nxo_t *a_nxo);
#endif

bool
nxo_file_nonblocking_get(const cw_nxo_t *a_nxo);

bool
nxo_file_nonblocking_set(cw_nxo_t *a_nxo, bool a_nonblocking);

int32_t
nxo_file_read(cw_nxo_t *a_nxo, uint32_t a_len, uint8_t *r_str);

cw_nxn_t
nxo_file_readline(cw_nxo_t *a_nxo, bool a_locking, cw_nxo_t *r_string,
		  bool *r_eof);

cw_nxn_t
nxo_file_write(cw_nxo_t *a_nxo, const uint8_t *a_str, uint32_t a_len,
	       uint32_t *r_count);

#ifdef CW_POSIX_FILE
cw_nxn_t
nxo_file_truncate(cw_nxo_t *a_nxo, off_t a_length);
#endif

cw_nxoi_t
nxo_file_position_get(cw_nxo_t *a_nxo);

#ifdef CW_POSIX_FILE
cw_nxn_t
nxo_file_position_set(cw_nxo_t *a_nxo, cw_nxoi_t a_position);
#endif

uint32_t
nxo_file_buffer_size_get(const cw_nxo_t *a_nxo);

void
nxo_file_buffer_size_set(cw_nxo_t *a_nxo, uint32_t a_size);

cw_nxoi_t
nxo_file_buffer_count(const cw_nxo_t *a_nxo);

cw_nxn_t
nxo_file_buffer_flush(cw_nxo_t *a_nxo);
