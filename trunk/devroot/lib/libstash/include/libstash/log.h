/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * <<< Description >>>
 *
 * Public interface for the log (logging) class.  Output can be sent to a file
 * (stderr by default).  Functions similar to printf() are provided for
 * formatted output.
 *
 ****************************************************************************/

typedef struct cw_log_s cw_log_t;

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * None.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a log, or NULL.
 *          NULL : Memory allocation error.
 *
 * <<< Description >>>
 *
 * Constructor.
 *
 ****************************************************************************/
cw_log_t *
log_new(void);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_log : Pointer to a log.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Destructor.
 *
 ****************************************************************************/
void
log_delete(cw_log_t * a_log);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_log : Pointer to a log.
 *
 * a_logfile : Pointer to a string that represents a file to log to.
 *
 * a_overwrite : TRUE == truncate a_logfile, FALSE == append to a_logfile.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : Error opening a_logfile.
 *               : fflush() error.
 *               : fclose() error.
 *               : Memory allocation error.
 *
 * <<< Description >>>
 *
 * Open file for logging.  If another file is already being used, close it
 * first.
 *
 ****************************************************************************/
cw_bool_t
log_set_logfile(cw_log_t * a_log, const char * a_logfile,
		cw_bool_t a_overwrite);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_log : Pointer to a log.
 *
 * a_format : Formatting string.
 *
 * ... : Arguments.
 *
 * <<< Output(s) >>>
 *
 * retval : Number of characters printed.
 *
 * <<< Description >>>
 *
 * printf() wrapper.
 *
 ****************************************************************************/
int
log_printf(cw_log_t * a_log, const char * a_format, ...);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_log : Pointer to a log.
 *
 * a_filename : __FILE__, or NULL.
 *
 * a_line_num : __LINE__.  Ignored if a_filename == NULL.
 *
 * a_func_name : __FUNCTION__, or NULL.
 *
 * a_format : Formatting string.
 *
 * ... : Arguments.
 *
 * <<< Output(s) >>>
 *
 * retval : Number of characters printed.
 *
 * <<< Description >>>
 *
 * printf() wrapper.  Optional arguments prepend filename, line number, and
 * function name.
 *
 ****************************************************************************/
int
log_eprintf(cw_log_t * a_log, 
	    const char * a_filename,
	    int a_line_num,
	    const char * a_func_name,
	    const char * a_format,
	    ...);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_log : Pointer to a log.
 *
 * a_size : Maximum number of bytes to print.
 *
 * a_format : Formatting string.
 *
 * ... : Arguments.
 *
 * <<< Output(s) >>>
 *
 * retval : Number of bytes printed, or -1.
 *          -1 : Memory allocation error.
 *
 * <<< Description >>>
 *
 * printf() wrapper, but print no more than a_size characters.
 *
 ****************************************************************************/
int
log_nprintf(cw_log_t * a_log,
	    cw_uint32_t a_size,
	    const char * a_format,
	    ...);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_log : Pointer to a log.
 *
 * a_format : Formatting string.
 *
 * ... : Arguments.
 *
 * <<< Output(s) >>>
 *
 * retval : Number of bytes printed.
 *
 * <<< Description >>>
 *
 * printf() wrapper, with a time stamp prepended.
 *
 ****************************************************************************/
int
log_lprintf(cw_log_t * a_log, const char * a_format, ...);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_log : Pointer to a log.
 *
 * a_filename : __FILE__, or NULL.
 *
 * a_line_num : __LINE__.  Ignored if a_filename == NULL.
 *
 * a_func_name : __FUNCTION__, or NULL.
 *
 * a_format : Formatting string.
 *
 * ... : Arguments.
 *
 * <<< Output(s) >>>
 *
 * retval : Number of characters printed.
 *
 * <<< Description >>>
 *
 * printf() wrapper, with a time stamp prepended.  Optional arguments prepend
 * filename, line number, and function name.
 *
 ****************************************************************************/
int
log_leprintf(cw_log_t * a_log,
	     const char * a_filename,
	     int a_line_num,
	     const char * a_func_name,
	     const char * a_format,
	     ...);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_val : Value to convert to a string.
 *
 * a_base : Number base to convert to.
 *
 * a_buf : A buffer of at least 65 bytes for base 2 conversion, 21 bytes
 *         for base 10 conversion, and 17 bytes for base 16 conversion.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a string that represents a_val, in base a_base (a_buf).
 *
 * <<< Description >>>
 *
 * Convert a_val to a number string in base a_base and put the result into
 * *a_buf.  Supported bases are 2, 10, and 16.
 *
 ****************************************************************************/
char *
log_print_uint64(cw_uint64_t a_val, cw_uint32_t a_base, char * a_buf);
