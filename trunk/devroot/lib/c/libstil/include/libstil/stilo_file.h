/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

typedef cw_sint32_t cw_stilo_file_read_t (void *a_arg, cw_stilo_t *a_file,
    cw_uint32_t a_len, cw_uint8_t *r_str);
typedef cw_bool_t cw_stilo_file_write_t (void *a_arg, cw_stilo_t *a_file, const
    cw_uint8_t *a_str, cw_uint32_t a_len);

void		stilo_file_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil, cw_bool_t
    a_locking);
void		stilo_file_fd_wrap(cw_stilo_t *a_stilo, cw_uint32_t a_fd);
void		stilo_file_interactive(cw_stilo_t *a_stilo, cw_stilo_file_read_t
    *a_read, cw_stilo_file_write_t *a_write, void *a_arg);
cw_stilo_threade_t stilo_file_open(cw_stilo_t *a_stilo, const cw_uint8_t
    *a_filename, cw_uint32_t a_nlen, const cw_uint8_t *a_flags, cw_uint32_t
    a_flen);
cw_stilo_threade_t stilo_file_close(cw_stilo_t *a_stilo);
cw_sint32_t	stilo_file_read(cw_stilo_t *a_stilo, cw_uint32_t a_len,
    cw_uint8_t *r_str);
cw_stilo_threade_t stilo_file_readline(cw_stilo_t *a_stilo, cw_stil_t *a_stil,
    cw_bool_t a_locking, cw_stilo_t *r_string, cw_bool_t *r_eof);
cw_stilo_threade_t stilo_file_write(cw_stilo_t *a_stilo, const cw_uint8_t
    *a_str, cw_uint32_t a_len);
cw_stilo_threade_t stilo_file_output(cw_stilo_t *a_stilo, const char *a_format,
    ...);
cw_stilo_threade_t stilo_file_output_n(cw_stilo_t *a_stilo, cw_uint32_t a_size,
    const char *a_format, ...);
cw_stilo_threade_t stilo_file_truncate(cw_stilo_t *a_stilo, off_t a_length);
cw_stiloi_t	stilo_file_position_get(cw_stilo_t *a_stilo);
cw_stilo_threade_t stilo_file_position_set(cw_stilo_t *a_stilo, cw_stiloi_t
    a_position);
cw_uint32_t	stilo_file_buffer_size_get(cw_stilo_t *a_stilo);
void		stilo_file_buffer_size_set(cw_stilo_t *a_stilo, cw_uint32_t
    a_size);
cw_stiloi_t	stilo_file_buffer_count(cw_stilo_t *a_stilo);
cw_stilo_threade_t stilo_file_buffer_flush(cw_stilo_t *a_stilo);
void		stilo_file_buffer_reset(cw_stilo_t *a_stilo);
cw_bool_t	stilo_file_status(cw_stilo_t *a_stilo);
cw_stiloi_t	stilo_file_mtime(cw_stilo_t *a_stilo);
