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

#include <errno.h>

#include "../include/nx.h"

/* Include generated code. */
#include "nx_nxcode.c"

#define	_BUFFER_SIZE	512

struct nx_read_arg_s {
	cw_bool_t	interactive;
	cw_nxo_t	*thread;
	cw_uint8_t	*buffer;
	cw_uint32_t	buffer_size;
	cw_uint32_t	buffer_count;
};

struct nx_write_arg_s {
	int	fd;
};

cw_sint32_t	nx_read(void *a_arg, cw_nxo_t *a_file, cw_uint32_t a_len,
    cw_uint8_t *r_str);
cw_bool_t	nx_write(void *a_arg, cw_nxo_t *a_file, const cw_uint8_t *a_str,
    cw_uint32_t a_len);

int
main(int argc, char **argv)
{
	cw_nx_t			nx;
	cw_nxo_t		thread, *nxo;
	struct nx_read_arg_s	stdin_arg;
	struct nx_write_arg_s	stdout_arg, stderr_arg;
	cw_bool_t		interactive;

	/* Start up. */
	libstash_init();
	nx_new(&nx, NULL, argc, argv, NULL);
	interactive = (isatty(0) && argc == 1) ? TRUE : FALSE;

	/*
	 * Wrap stdin, stdout, and stderr, so that file operations will work
	 * even if _CW_POSIX isn't defined.
	 */
	/* stdin. */
	nxo = nx_stdin_get(&nx);
	nxo_file_new(nxo, &nx, TRUE);
	nxo_file_interactive(nxo, nx_read, NULL, (void *)&stdin_arg);
	/* Initialize the stdin argument structure. */
	stdin_arg.interactive = interactive;
	stdin_arg.thread = &thread;
	stdin_arg.buffer = NULL;
	stdin_arg.buffer_size = 0;
	stdin_arg.buffer_count = 0;

	/* stdout. */
	nxo = nx_stdout_get(&nx);
	nxo_file_new(nxo, &nx, TRUE);
	nxo_file_interactive(nxo, NULL, nx_write, (void *)&stdout_arg);
	/* Initialize the stdout argument structure. */
	stdout_arg.fd = 1;

	/* stderr. */
	nxo = nx_stderr_get(&nx);
	nxo_file_new(nxo, &nx, TRUE);
	nxo_file_interactive(nxo, NULL, nx_write, (void *)&stderr_arg);
	/* Initialize the stderr argument structure. */
	stderr_arg.fd = 2;

	/* Now that the files have been wrapped, create the initial thread. */
	nxo_thread_new(&thread, &nx);

	if (interactive) {
		/* Run embedded initialization code. */
		nx_nxcode(&thread);
	}

	/* Run the interpreter. */
	_cw_onyx_code(&thread, "stdin cvx start");

	/* Shut down. */
	if (stdin_arg.buffer != NULL)
		_cw_free(stdin_arg.buffer);
	nx_delete(&nx);
	libstash_shutdown();
	return 0;
}

cw_sint32_t
nx_read(void *a_arg, cw_nxo_t *a_file, cw_uint32_t a_len, cw_uint8_t *r_str)
{
	cw_sint32_t		retval;
	struct nx_read_arg_s	*arg = (struct nx_read_arg_s *)a_arg;

	_cw_assert(a_len > 0);

	if (arg->buffer_count == 0) {
		/*
		 * Print the prompt if interactive and not in deferred execution
		 * mode.
		 */
		if ((arg->interactive) && (nxo_thread_deferred(arg->thread) ==
		    FALSE) && (nxo_thread_state(arg->thread) ==
		    THREADTS_START))
			_cw_onyx_code(arg->thread, "promptstring print flush");

		/*
		 * Read data until there are no more.
		 */
		while ((retval = read(0, r_str, a_len)) == -1 && errno ==
		    EINTR) {
			/*
			 * Interrupted system call, probably due to garbage
			 * collection.
			 */
		}
		if (retval == -1) {
			/* EOF. */
			retval = 0;
		} else {
			if (retval == a_len) {
				cw_sint32_t	count;

				/*
				 * There may be more data available.  Store any
				 * available data in a buffer.
				 */
				if (arg->buffer == NULL) {
					/* Initialize the buffer. */
					arg->buffer = (cw_uint8_t
					    *)_cw_malloc(_BUFFER_SIZE);
					arg->buffer_size = _BUFFER_SIZE;
					arg->buffer_count = 0;
				}

				for (;;) {
					while ((count = read(0,
					    &arg->buffer[arg->buffer_count],
					    arg->buffer_size -
					    arg->buffer_count)) == -1 && errno
					    == EINTR) {
						/*
						 * Interrupted system call,
						 * probably due to garbage
						 * collection.
						 */
					}
					if (count <= 0) {
						/*
						 * EOF or no more data buffered
						 * by stdin.
						 */
						break;
					}

					arg->buffer_count += count;
					if (arg->buffer_count ==
					    arg->buffer_size) {
						/* Expand the buffer. */
						arg->buffer = (cw_uint8_t
						    *)_cw_realloc(arg->buffer,
						    arg->buffer_size * 2);
						arg->buffer_size *= 2;
					}
				}
			}
		}
	} else {
		/*
		 * Return as much of the buffered data as possible.
		 */
		if (arg->buffer_count > a_len) {
			/* There are more data than we can return. */
			retval = a_len;
			memcpy(r_str, arg->buffer, a_len);
			arg->buffer_count -= a_len;
			memmove(arg->buffer, &arg->buffer[a_len],
			    arg->buffer_count);
		} else {
			/* Return all the data. */
			retval = arg->buffer_count;
			memcpy(r_str, arg->buffer, arg->buffer_count);
			arg->buffer_count = 0;
		}
	}

	return retval;
}

cw_bool_t
nx_write(void *a_arg, cw_nxo_t *a_file, const cw_uint8_t *a_str, cw_uint32_t
    a_len)
{
	cw_bool_t		retval;
	struct nx_write_arg_s	*arg = (struct nx_write_arg_s *)a_arg;
	ssize_t			nwritten;

	while (((nwritten = write(arg->fd, a_str, a_len)) == -1) && errno ==
	    EINTR) {
		/*
		 * Interrupted system call, likely due to garbage collection.
		 */
	}
	if (nwritten != a_len) {
		_cw_assert(nwritten == -1);
		retval = TRUE;
		goto RETURN;
	}

	retval = FALSE;
	RETURN:
	return retval;
}
