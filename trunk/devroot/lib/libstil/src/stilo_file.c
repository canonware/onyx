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

#include "../include/libstil/libstil.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <poll.h>

#include "../include/libstil/stila_l.h"
#include "../include/libstil/stilo_l.h"
#include "../include/libstil/stilo_file_l.h"

static cw_stilo_threade_t stilo_p_file_buffer_flush(cw_stilo_t *a_stilo);

#define		stiloe_p_file_lock(a_stiloe) do {			\
	if ((a_stiloe)->stiloe.locking)					\
		mtx_lock(&(a_stiloe)->lock);				\
} while (0)
#define		stiloe_p_file_unlock(a_stiloe) do {			\
	if ((a_stiloe)->stiloe.locking)					\
		mtx_unlock(&(a_stiloe)->lock);				\
} while (0)

void
stilo_file_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil, cw_bool_t a_locking)
{
	cw_stiloe_file_t	*file;

	file = (cw_stiloe_file_t *)_cw_malloc(sizeof(cw_stiloe_file_t));

	stiloe_l_new(&file->stiloe, STILOT_FILE, a_locking);
	if (a_locking)
		mtx_new(&file->lock);
	file->fd = -1;

	file->buffer = NULL;
	file->buffer_mode = BUFFER_EMPTY;
	file->buffer_size = 0;
	file->buffer_offset = 0;

	file->read_f = NULL;
	file->write_f = NULL;
	file->arg = NULL;
	file->position = 0;

	memset(a_stilo, 0, sizeof(cw_stilo_t));
	a_stilo->o.stiloe = (cw_stiloe_t *)file;
#ifdef _LIBSTIL_DBG
	a_stilo->magic = _CW_STILO_MAGIC;
#endif
	a_stilo->type = STILOT_FILE;

	stila_l_gc_register(stil_stila_get(a_stil), (cw_stiloe_t *)file);
}

void
stiloe_l_file_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil)
{
	cw_stiloe_file_t	*file;
	cw_bool_t		ioerror = FALSE;

	file = (cw_stiloe_file_t *)a_stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(file->stiloe.type == STILOT_FILE);

	if (file->buffer != NULL) {
		if (file->buffer_mode == BUFFER_WRITE) {
			switch (file->fd) {
			case -2: {
				cw_stilo_t	tstilo;

				stilo_l_new(&tstilo, STILOT_FILE);
				tstilo.o.stiloe = (cw_stiloe_t *)file;

				if (file->write_f(file->arg, &tstilo,
				    file->buffer, file->buffer_offset))
					ioerror = TRUE;
				break;
			}
			case -1:
				break;
			default:
				while (write(file->fd, file->buffer,
				    file->buffer_offset) == -1) {
					if (errno != EINTR) {
						ioerror = TRUE;
						break;
					}
				}
				break;
			}
		}
		_cw_free(file->buffer);
	}
	if (file->stiloe.locking)
		mtx_delete(&file->lock);
	/*
	 * Don't automatically close() predefined or wrapped descriptors.
	 */
	if (file->fd >= 3) {
		if (close(file->fd) == -1)
			ioerror = TRUE;
	}

	if (ioerror) {
		/*
		 * GC-induced deletion can get a write error, but there's no
		 * thread to report it to.  Oh well.
		 */
	}

	_CW_STILOE_FREE(file);
}

cw_stiloe_t *
stiloe_l_file_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t a_reset)
{
	return NULL;
}

cw_stilo_threade_t
stilo_l_file_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file, cw_uint32_t
    a_depth)
{
	cw_stilo_threade_t	retval;

	retval = stilo_file_output(a_file, "-file-");

	return retval;
}

void
stilo_file_fd_wrap(cw_stilo_t *a_stilo, cw_uint32_t a_fd)
{
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(file->stiloe.type == STILOT_FILE);
	_cw_assert(file->fd == -1);

	file->fd = a_fd;
}

void
stilo_file_interactive(cw_stilo_t *a_stilo, cw_stilo_file_read_t *a_read,
    cw_stilo_file_write_t *a_write, void *a_arg)
{
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(file->stiloe.type == STILOT_FILE);
	_cw_assert(file->fd == -1);

	/*
	 * -2 is a special value that signifies this is an interactive editor
	 * "file".
	 */
	file->fd = -2;
	file->read_f = a_read;
	file->write_f = a_write;
	file->arg = a_arg;
	file->position = 0;
}

/*
 * STILO_THREADE_IOERROR, STILO_THREADE_LIMITCHECK,
 * STILO_THREADE_INVALIDFILEACCESS
 */
cw_stilo_threade_t
stilo_file_open(cw_stilo_t *a_stilo, const cw_uint8_t *a_filename, cw_uint32_t
    a_nlen, const cw_uint8_t *a_flags, cw_uint32_t a_flen)
{
	cw_stilo_threade_t	retval;
	cw_stiloe_file_t	*file;
	cw_uint8_t		filename[PATH_MAX], flags[3];
	int			access;

	/*
	 * Copy the arguments to local buffers in order to assure '\0'
	 * termination.
	 */
	if (a_nlen >= sizeof(filename)) {
		retval = STILO_THREADE_LIMITCHECK;
		goto RETURN;
	}
	memcpy(filename, a_filename, a_nlen);
	filename[a_nlen] = '\0';

	if (a_flen >= sizeof(flags)) {
		retval = STILO_THREADE_LIMITCHECK;
		goto RETURN;
	}
	memcpy(flags, a_flags, a_flen);
	flags[a_flen] = '\0';

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(file->stiloe.type == STILOT_FILE);

	stiloe_p_file_lock(file);
	if (file->fd != -1) {
		retval = STILO_THREADE_INVALIDFILEACCESS;
		goto URETURN;
	}

	/* Convert a_flags to the integer representation. */
	switch (flags[0]) {
	case 'r':
		switch (flags[1]) {
		case '\0':
			access = O_RDONLY;
			break;
		case '+':
			access = O_RDWR;
			break;
		default:
			retval = STILO_THREADE_INVALIDFILEACCESS;
			goto URETURN;
		}
		break;
	case 'w':
		switch (flags[1]) {
		case '\0':
			access = O_WRONLY | O_CREAT | O_TRUNC;
			break;
		case '+':
			access = O_RDWR | O_CREAT | O_TRUNC;
			break;
		default:
			retval = STILO_THREADE_INVALIDFILEACCESS;
			goto URETURN;
		}
		break;
	case 'a':
		switch (flags[1]) {
		case '\0':
			access = O_WRONLY | O_APPEND | O_CREAT;
			break;
		case '+':
			access = O_RDWR | O_APPEND | O_CREAT;
			break;
		default:
			retval = STILO_THREADE_INVALIDFILEACCESS;
			goto URETURN;
		}
		break;
	default:
		retval = STILO_THREADE_INVALIDFILEACCESS;
		goto URETURN;
	}

	file->fd = open(filename, access, 0x1ff);
	if (file->fd == -1) {
		switch (errno) {
		case ENOSPC:
		case EMFILE:
		case ENFILE:
			retval = STILO_THREADE_IOERROR;
			goto URETURN;
		default:
			retval = STILO_THREADE_INVALIDFILEACCESS;
			goto URETURN;
		}
	}

	retval = STILO_THREADE_NONE;
	URETURN:
	stiloe_p_file_unlock(file);
	RETURN:
	return retval;
}

/* STILO_THREADE_IOERROR */
cw_stilo_threade_t
stilo_file_close(cw_stilo_t *a_stilo)
{
	cw_stilo_threade_t	retval;
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(file->stiloe.type == STILOT_FILE);

	stiloe_p_file_lock(file);
	if (file->fd == -1) {
		retval = STILO_THREADE_IOERROR;
		goto RETURN;
	}

	/* Flush and get rid of the buffer if necessary. */
	retval = stilo_p_file_buffer_flush(a_stilo);
	if (retval)
		goto RETURN;
	if (file->buffer != NULL) {
		_cw_free(file->buffer);
		file->buffer = NULL;
		file->buffer_size = 0;
		file->buffer_mode = BUFFER_EMPTY;
	}

	if ((file->fd >= 0) && (close(file->fd) == -1)) {
		file->fd = -1;
		retval = STILO_THREADE_IOERROR;
		goto RETURN;
	}
	file->fd = -1;

	retval = STILO_THREADE_NONE;
	RETURN:
	stiloe_p_file_unlock(file);
	return retval;
}

/* -1: STILO_THREADE_IOERROR */
cw_sint32_t
stilo_file_read(cw_stilo_t *a_stilo, cw_uint32_t a_len, cw_uint8_t *r_str)
{
	cw_sint32_t		retval;
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(file->stiloe.type == STILOT_FILE);

	stiloe_p_file_lock(file);
	if (file->fd == -1) {
		retval = -1;
		goto RETURN;
	}

	/*
	 * Use the internal buffer if it exists.  If there aren't enough data in
	 * the buffer to fill r_str, copy what we have to r_str, then do a
	 * readv() to fill r_str and the buffer.  If the buffer is empty, also
	 * do a readv() to replenish it.
	 */
	if (file->buffer != NULL) {
		if ((file->buffer_mode == BUFFER_READ) && (a_len <=
		    file->buffer_offset)) {
			/* We had enough buffered data. */
			memcpy(r_str, file->buffer, a_len);
			memmove(file->buffer, &file->buffer[a_len],
			    file->buffer_offset - a_len);
			retval = a_len;
			file->buffer_offset -= a_len;
			if (file->buffer_offset == 0)
				file->buffer_mode = BUFFER_EMPTY;
		} else {
			ssize_t		nread;

			/*
			 * Copy any buffered before reading more data.
			 */
			if (file->buffer_mode == BUFFER_READ) {
				memcpy(r_str, file->buffer,
				    file->buffer_offset);
				retval = file->buffer_offset;
				r_str += file->buffer_offset;
				a_len -= file->buffer_offset;
			} else
				retval = 0;
			/* Clear the buffer. */
			file->buffer_offset = 0;
			file->buffer_mode = BUFFER_EMPTY;

			if (file->fd >= 0) {
				struct pollfd	events;
				struct iovec	iov[2];

				if (retval == 0) {
					/*
					 * No data read yet.  Sleep until some
					 * data are available.
					 */
					events.fd = file->fd;
					events.events = POLLIN | POLLRDNORM;
					while ((poll(&events, 1, -1)) == -1)
						; /* EINTR; try again. */

					/*
					 * Finish filling r_str and replenish
					 * the internal buffer.
					 */
					iov[0].iov_base = r_str;
					iov[0].iov_len = a_len;
					iov[1].iov_base = file->buffer;
					iov[1].iov_len = file->buffer_size;

					while ((nread = readv(file->fd, iov, 2))
					    == -1) {
						if (errno != EINTR)
							break;
					}
				} else {
					int	nready;

					/*
					 * Only read if data are available.
					 */
					events.fd = file->fd;
					events.events = POLLIN | POLLRDNORM;
					while ((nready = poll(&events, 1, 0)) ==
					    -1)
						; /* EINTR; try again. */

					if (nready == 1) {
						/*
						 * Finish filling r_str and
						 * replenish the internal
						 * buffer.
						 */
						iov[0].iov_base = r_str;
						iov[0].iov_len = a_len;
						iov[1].iov_base = file->buffer;
						iov[1].iov_len =
						    file->buffer_size;

						while ((nread = readv(file->fd,
						    iov, 2)) == -1) {
							if (errno != EINTR)
								break;
						}
					} else
						nread = 0;
				}
			} else {
				/* Use the read wrapper function. */
				nread = file->read_f(file->arg, a_stilo, a_len,
				    r_str);
			}

			/* Handle various read return values. */
			if (nread == -1) {
				if (retval == 0) {
					retval = -1;
					goto RETURN;
				}
				/*
				 * There was an error, but we already managed to
				 * provide some data, so don't report an error
				 * this time around.
				 */
			} else if (nread <= a_len) {
				/*
				 * We didn't get enough data to start filling
				 * the internal buffer.
				 */
				retval += nread;
			} else {
				retval += a_len;
				file->buffer_offset = nread - a_len;
				file->buffer_mode = BUFFER_READ;
			}
		}
	} else {
		if (file->fd >= 0) {
			while ((retval = read(file->fd, r_str, a_len)) == -1) {
				if (errno != EINTR)
					break;
			}
		}else
			retval = file->read_f(file->arg, a_stilo, a_len, r_str);
	}

	if (retval == 0 && file->buffer != NULL) {
		file->buffer_offset = 0;
		file->buffer_mode = BUFFER_EMPTY;
	}
	RETURN:
	if (retval != -1)
		file->position += retval;
	stiloe_p_file_unlock(file);
	return retval;
}

/* STILO_THREADE_IOERROR */
cw_stilo_threade_t
stilo_file_readline(cw_stilo_t *a_stilo, cw_stil_t *a_stil, cw_bool_t a_locking,
    cw_stilo_t *r_string, cw_bool_t *r_eof)
{
	cw_stilo_threade_t	retval;
	cw_stiloe_file_t	*file;
	cw_uint8_t		*line, s_line[_CW_STILO_FILE_READLINE_BUFSIZE];
	cw_uint32_t		i, maxlen;
	cw_sint32_t		nread;
	enum {NORMAL, CR}	state;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(file->stiloe.type == STILOT_FILE);

	line = s_line;

	stiloe_p_file_lock(file);
	if (file->fd == -1) {
		retval = STILO_THREADE_IOERROR;
		goto RETURN;
	}

	/*
	 * Copy bytes until we see a \n or EOF.  Note that \r\n is converted to
	 * \n.
	 */
	if (file->buffer != NULL) {
		cw_uint32_t	offset;

		/* Flush the internal buffer if it has write data. */
		if (file->buffer_mode == BUFFER_WRITE) {
			retval = stilo_p_file_buffer_flush(a_stilo);
			if (retval)
				goto RETURN;
		}

		/*
		 * Copy bytes from the internal buffer, one at a time,
		 * replenishing the internal buffer as necessary.
		 */
		for (i = 0, offset = 0, maxlen =
		    _CW_STILO_FILE_READLINE_BUFSIZE, state = NORMAL;; i++) {
			/*
			 * If we're about to overflow the line buffer, expand
			 * it.
			 */
			if (i == maxlen) {
				if (line == s_line) {
					/* First expansion. */
					line = (cw_uint8_t *)_cw_malloc(maxlen
					    << 1);
					memcpy(line, s_line, maxlen);
				} else {
					cw_uint8_t	*oldline;

					/*
					 * We've already expanded at least
					 * once.
					 */
					oldline = line;
					line = (cw_uint8_t *)_cw_malloc(maxlen
					    << 1);
					memcpy(line, oldline, maxlen);
					_cw_free(oldline);
				}
				maxlen <<= 1;
			}

			if ((offset >= file->buffer_offset) ||
			    (file->buffer_mode == BUFFER_EMPTY)) {
				/* Replenish the internal buffer. */
				if (file->fd >= 0) {
					while ((nread = read(file->fd,
					    file->buffer, file->buffer_size)) ==
					    -1) {
						if (errno != EINTR)
							break;
					}
				} else {
					/* Use the read wrapper function. */
					nread = file->read_f(file->arg, a_stilo,
					    file->buffer_size,
					    file->buffer);
				}
				if (nread <= 0) {
					/* EOF. */
					file->fd = -1;
					file->buffer_offset = 0;
					file->buffer_mode = BUFFER_EMPTY;

					stilo_string_new(r_string, a_stil,
					    a_locking, i);
					stilo_string_lock(r_string);
					stilo_string_set(r_string, 0, line, i);
					stilo_string_unlock(r_string);
					file->position += i;

					retval = STILO_THREADE_NONE;
					*r_eof = TRUE;
					goto RETURN;
				}

				file->buffer_mode = BUFFER_READ;
				file->buffer_offset = nread;
				offset = 0;
			}

			line[i] = file->buffer[offset];
			offset++;

			switch (line[i]) {
			case '\r':
				state = CR;
				break;
			case '\n':
				if (state == CR) {
					/* Throw away the preceding \r. */
					i--;
				}
				stilo_string_new(r_string, a_stil, a_locking,
				    i);
				stilo_string_lock(r_string);
				stilo_string_set(r_string, 0, line, i);
				stilo_string_unlock(r_string);
				file->position += i;

				/*
				 * Shift the remaining buffered data to the
				 * beginning of the buffer.
				 */
				if (file->buffer_offset - offset > 0) {
					memmove(file->buffer,
					    &file->buffer[offset],
					    file->buffer_offset - offset);
					file->buffer_offset -= offset;
				} else {
					file->buffer_offset = 0;
					file->buffer_mode = BUFFER_EMPTY;
				}

				retval = STILO_THREADE_NONE;
				*r_eof = FALSE;
				goto RETURN;
			default:
				state = NORMAL;
				break;
			}
		}
	} else {
		/*
		 * There is no internal buffer, so we must read one byte at a
		 * time from the file.  This is horribly inneficient, but we
		 * don't have anywhere to unput characters past a newline.
		 */
		for (i = 0, maxlen = _CW_STILO_FILE_READLINE_BUFSIZE, state =
			 NORMAL;; i++) {
			/*
			 * If we're about to overflow the line buffer, expand
			 * it.
			 */
			if (i == maxlen) {
				if (line == s_line) {
					/* First expansion. */
					line = (cw_uint8_t *)_cw_malloc(maxlen
					    << 1);
					memcpy(line, s_line, maxlen);
				} else {
					cw_uint8_t	*oldline;

					/*
					 * We've already expanded at least
					 * once.
					 */
					oldline = line;
					line = (cw_uint8_t *)_cw_malloc(maxlen
					    << 1);
					memcpy(line, oldline, maxlen);
					_cw_free(oldline);
				}
				maxlen <<= 1;
			}

			if (file->fd >= 0)
				while ((nread = read(file->fd, &line[i], 1)) ==
				    -1) {
					if (errno != EINTR)
						break;
				}
			else {
				/* Use the read wrapper function. */
				nread = file->read_f(file->arg, a_stilo, 1,
				    &line[i]);
			}
			if (nread <= 0) {
				/* EOF. */
				stilo_string_new(r_string, a_stil, a_locking,
				    i);
				stilo_string_lock(r_string);
				stilo_string_set(r_string, 0, line, i);
				stilo_string_unlock(r_string);
				file->position += i;

				retval = STILO_THREADE_NONE;
				*r_eof = TRUE;
				goto RETURN;
			}

			switch (line[i]) {
			case '\r':
				state = CR;
				break;
			case '\n':
				if (state == CR) {
					/* Throw away the preceding \r. */
					i--;
				}
				stilo_string_new(r_string, a_stil, a_locking,
				    i);
				stilo_string_lock(r_string);
				stilo_string_set(r_string, 0, line, i);
				stilo_string_unlock(r_string);
				file->position += i;

				retval = STILO_THREADE_NONE;
				*r_eof = FALSE;
				goto RETURN;
			default:
				state = NORMAL;
				break;
			}
		}
	}

	RETURN:
	stiloe_p_file_unlock(file);
	if (line != s_line)
		_cw_free(line);
	return retval;
}

/* STILO_THREADE_IOERROR */
cw_stilo_threade_t
stilo_file_write(cw_stilo_t *a_stilo, const cw_uint8_t *a_str, cw_uint32_t
    a_len)
{
	cw_stilo_threade_t	retval;
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(file->stiloe.type == STILOT_FILE);

	stiloe_p_file_lock(file);
	if (file->fd == -1) {
		retval = STILO_THREADE_IOERROR;
		goto RETURN;
	}

	if (file->buffer != NULL) {
		/* Discard cached read data if necessary. */
		if (file->buffer_mode == BUFFER_READ) {
			file->buffer_mode = BUFFER_EMPTY;
			file->buffer_offset = 0;
		}

		/*
		 * Use the internal buffer.  If a_str won't entirely fit, do a
		 * writev() for a normal file descriptor, or a straight write
		 * for a wrapped file.
		 */
		if (a_len <= file->buffer_size - file->buffer_offset) {
			/* a_str will fit. */
			memcpy(&file->buffer[file->buffer_offset],
			    a_str, a_len);
			file->buffer_mode = BUFFER_WRITE;
			file->buffer_offset += a_len;
		} else if (file->fd >= 0) {
			struct iovec	iov[2];

			/* a_str won't fit.  Do a writev(). */

			iov[0].iov_base = file->buffer;
			iov[0].iov_len = file->buffer_offset;
			iov[1].iov_base = (char *)a_str;
			iov[1].iov_len = a_len;

			while (writev(file->fd, iov, 2) == -1) {
				if (errno != EINTR) {
					retval = STILO_THREADE_IOERROR;
					goto RETURN;
				}
			}

			file->buffer_mode = BUFFER_EMPTY;
			file->buffer_offset = 0;
		} else {
			/*
			 * a_str won't fit.  Flush the buffer and call the
			 * write wrapper function.
			 */
			retval = stilo_p_file_buffer_flush(a_stilo);
			if (retval)
				goto RETURN;

			if (file->write_f(file->arg, a_stilo, a_str, a_len)) {
				retval = STILO_THREADE_IOERROR;
				goto RETURN;
			}
			file->position += a_len;
		}
	} else {
		if (file->fd >= 0) {
			while (write(file->fd, a_str, a_len) == -1) {
				if (errno != EINTR) {
					retval = STILO_THREADE_IOERROR;
					goto RETURN;
				}
			}
		} else {
			if (file->write_f(file->arg, a_stilo, a_str, a_len)) {
				retval = STILO_THREADE_IOERROR;
				goto RETURN;
			}
			file->position += a_len;
		}
	}

	retval = STILO_THREADE_NONE;
	RETURN:
	stiloe_p_file_unlock(file);
	return retval;
}

/* STILO_THREADE_IOERROR */
cw_stilo_threade_t
stilo_file_output(cw_stilo_t *a_stilo, const char *a_format, ...)
{
	cw_stilo_threade_t	retval;
	cw_stiloe_file_t	*file;
	va_list			ap;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(file->stiloe.type == STILOT_FILE);

	stiloe_p_file_lock(file);
	if (file->fd == -1) {
		retval = STILO_THREADE_IOERROR;
		goto RETURN;
	}

	if (file->buffer != NULL) {
		cw_uint32_t	maxlen;
		cw_sint32_t	nwrite;

		/*
		 * There is an internal buffer; attempt to render to it.  If the
		 * resulting string doesn't fit in the buffer, flush the buffer
		 * and write the string directly to the file.  This results in
		 * rendering part of the string twice, but the alternative is to
		 * always malloc().  Chances are good that rendering to the
		 * buffer will succeed, so this seems like a reasonable trade
		 * off.
		 */

		/* Discard cached read data if necessary. */
		if (file->buffer_mode == BUFFER_READ) {
			file->buffer_mode = BUFFER_EMPTY;
			file->buffer_offset = 0;
		}

		maxlen = file->buffer_size - file->buffer_offset;
		va_start(ap, a_format);
		if ((nwrite = out_put_svn(NULL,
		    &file->buffer[file->buffer_offset], maxlen, a_format, ap))
		    == maxlen) {
			/*
			 * It probably didn't fit (there's definitely no space
			 * left over).
			 */
			va_end(ap);

			/* Flush the internal buffer. */
			retval = stilo_p_file_buffer_flush(a_stilo);
			if (retval)
				goto RETURN;

			if (file->fd >= 0) {
				/* Write directly to the file. */
				va_start(ap, a_format);
				if (out_put_fv(NULL, file->fd, a_format, ap)
				    == -1) {
					retval = STILO_THREADE_IOERROR;
					goto RETURN;
				}
			} else {
				/*
				 * Try to render to the buffer again.  If it
				 * still doesn't fit, use out_put_sva to
				 * allocate a string, write the string, then
				 * deallocate it.
				 */
				va_end(ap);
				va_start(ap, a_format);

				if ((nwrite = out_put_svn(NULL,
				    &file->buffer[file->buffer_offset], maxlen,
				    a_format, ap)) == maxlen) {
					char	*str;

					va_end(ap);
					va_start(ap, a_format);

					nwrite = out_put_sva(NULL, &str,
					    a_format, ap);
					if (file->write_f(file->arg, a_stilo,
					    str, nwrite)) {
						_cw_free(str);
						retval = STILO_THREADE_IOERROR;
						goto RETURN;
					}
					_cw_free(str);
				} else {
					/* It fit on the second try. */
					file->buffer_mode = BUFFER_WRITE;
					file->buffer_offset += nwrite;
				}
			}
		} else {
			/* It fit. */
			file->buffer_mode = BUFFER_WRITE;
			file->buffer_offset += nwrite;
		}
		va_end(ap);
	} else {
		va_start(ap, a_format);
		if (out_put_fv(NULL, file->fd, a_format, ap) == -1) {
			retval = STILO_THREADE_IOERROR;
			goto RETURN;
		}
		va_end(ap);
	}

	retval = STILO_THREADE_NONE;
	RETURN:
	stiloe_p_file_unlock(file);
	return retval;
}

/* STILO_THREADE_IOERROR */
cw_stilo_threade_t
stilo_file_output_n(cw_stilo_t *a_stilo, cw_uint32_t a_size, const char
    *a_format, ...)
{
	cw_stilo_threade_t	retval;
	cw_stiloe_file_t	*file;
	va_list			ap;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(file->stiloe.type == STILOT_FILE);

	stiloe_p_file_lock(file);
	if (file->fd == -1) {
		retval = STILO_THREADE_IOERROR;
		goto RETURN;
	}

	va_start(ap, a_format);
	if (file->buffer != NULL) {
		cw_uint32_t	maxlen;

		/*
		 * There is an internal buffer; if a_size is less than the
		 * amount of free buffer space, render to it.  Otherwise, flush
		 * the buffer and write the string directly to the file.
		 */

		/* Discard cached read data if necessary. */
		if (file->buffer_mode == BUFFER_READ) {
			file->buffer_mode = BUFFER_EMPTY;
			file->buffer_offset = 0;
		}

		maxlen = file->buffer_size - file->buffer_offset;
		if (a_size < maxlen) {
			cw_sint32_t	nwrite;

			/* It will fit. */

			nwrite = out_put_svn(NULL,
			    &file->buffer[file->buffer_offset],
			    a_size, a_format, ap);
			_cw_assert(nwrite == a_size);
			file->buffer_mode = BUFFER_WRITE;
			file->buffer_offset += nwrite;
		} else {
			/* It won't fit. */

			/* Flush the internal buffer, if necessary. */
			retval = stilo_p_file_buffer_flush(a_stilo);
			if (retval)
				goto RETURN;

			if (file->fd >= 0) {
				/* Write directly to the file. */
				if (out_put_fvn(NULL, file->fd, a_size,
				    a_format, ap) == -1) {
					retval = STILO_THREADE_IOERROR;
					goto RETURN;
				}
			} else {
				cw_sint32_t	nwrite;
				char		*str;

				nwrite = out_put_sva(NULL, &str, a_format,
				    ap);
				if (file->write_f(file->arg, a_stilo, str,
				    (nwrite < a_size) ? nwrite : a_size)) {
					_cw_free(str);
					retval = STILO_THREADE_IOERROR;
					goto RETURN;
				}
				_cw_free(str);
			}
		}
	} else {
		if (file->fd >= 0) {
			if (out_put_fvn(NULL, file->fd, a_size, a_format,
			    ap) == -1) {
				retval = STILO_THREADE_IOERROR;
				goto RETURN;
			}
		} else {
			cw_sint32_t	nwrite;
			char		*str;

			nwrite = out_put_sva(NULL, &str, a_format, ap);
			if (file->write_f(file->arg, a_stilo, str, (nwrite <
			    a_size) ? nwrite : a_size)) {
				_cw_free(str);
				retval = STILO_THREADE_IOERROR;
				goto RETURN;
			}
			_cw_free(str);
		}
	}
	va_end(ap);

	retval = STILO_THREADE_NONE;
	RETURN:
	stiloe_p_file_unlock(file);
	return retval;
}

/* STILO_THREADE_IOERROR */
cw_stilo_threade_t
stilo_file_truncate(cw_stilo_t *a_stilo, off_t a_length)
{
	cw_stilo_threade_t	retval;
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(file->stiloe.type == STILOT_FILE);

	stiloe_p_file_lock(file);
	if (file->fd == -2) {
		retval = STILO_THREADE_IOERROR;
		goto RETURN;
	}

	stilo_p_file_buffer_flush(a_stilo);

	if (ftruncate(file->fd, a_length)) {
		retval = STILO_THREADE_IOERROR;
		goto RETURN;
	}

	retval = STILO_THREADE_NONE;
	RETURN:
	stiloe_p_file_unlock(file);
	return retval;
}

/* -1: STILO_THREADE_IOERROR */
cw_stiloi_t
stilo_file_position_get(cw_stilo_t *a_stilo)
{
	cw_stiloi_t		retval;
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(file->stiloe.type == STILOT_FILE);

	stiloe_p_file_lock(file);
	if (file->fd == -1) {
		retval = -1;
		goto RETURN;
	}

	if (file->fd == -2)
		retval = file->position;
	else
		retval = lseek(file->fd, 0, SEEK_CUR);

	RETURN:
	stiloe_p_file_unlock(file);
	return retval;
}

/* STILO_THREADE_IOERROR */
cw_stilo_threade_t
stilo_file_position_set(cw_stilo_t *a_stilo, cw_stiloi_t a_position)
{
	cw_stilo_threade_t	retval;
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(file->stiloe.type == STILOT_FILE);

	stiloe_p_file_lock(file);
	if (file->fd < 0) {
		retval = STILO_THREADE_IOERROR;
		goto RETURN;
	}

	retval = stilo_p_file_buffer_flush(a_stilo);
	if (retval)
		goto RETURN;

	if (lseek(file->fd, a_position, SEEK_SET) == -1) {
		retval = STILO_THREADE_IOERROR;
		goto RETURN;
	}

	retval = STILO_THREADE_NONE;
	RETURN:
	stiloe_p_file_unlock(file);
	return retval;
}

cw_uint32_t
stilo_file_buffer_size_get(cw_stilo_t *a_stilo)
{
	cw_uint32_t		retval;
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(file->stiloe.type == STILOT_FILE);

	stiloe_p_file_lock(file);
	retval = file->buffer_size;
	stiloe_p_file_unlock(file);

	return retval;
}

void
stilo_file_buffer_size_set(cw_stilo_t *a_stilo, cw_uint32_t a_size)
{
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(file->stiloe.type == STILOT_FILE);

	stiloe_p_file_lock(file);
	if (a_size == 0) {
		if (file->buffer != NULL) {
			_cw_free(file->buffer);
			file->buffer = NULL;
			file->buffer_size = 0;
		}
	} else {
		if (file->buffer != NULL)
			_cw_free(file->buffer);
		file->buffer = (cw_uint8_t *)_cw_malloc(a_size);
		file->buffer_size = a_size;
	}
	file->buffer_mode = BUFFER_EMPTY;
	file->buffer_offset = 0;
	stiloe_p_file_unlock(file);
}

cw_stiloi_t
stilo_file_buffer_count(cw_stilo_t *a_stilo)
{
	cw_stiloi_t		retval;
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(file->stiloe.type == STILOT_FILE);

	stiloe_p_file_lock(file);
	if ((file->fd != -1 && file->buffer != NULL && file->buffer_mode
	    != BUFFER_WRITE))
		retval = file->buffer_offset;
	else
		retval = 0;
	stiloe_p_file_unlock(file);

	return retval;
}

/* STILO_THREADE_IOERROR */
cw_stilo_threade_t
stilo_file_buffer_flush(cw_stilo_t *a_stilo)
{
	cw_stilo_threade_t	retval;
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(file->stiloe.type == STILOT_FILE);

	stiloe_p_file_lock(file);
	retval = stilo_p_file_buffer_flush(a_stilo);
	stiloe_p_file_unlock(file);

	return retval;
}

static cw_stilo_threade_t
stilo_p_file_buffer_flush(cw_stilo_t *a_stilo)
{
	cw_stilo_threade_t	retval;
	cw_stiloe_file_t	*file;

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	if (file->fd == -1) {
		retval = STILO_THREADE_IOERROR;
		goto RETURN;
	}
	
	if (file->buffer != NULL) {
		/* Only write if the buffered data is for writing. */
		if (file->buffer_mode == BUFFER_WRITE) {
			if (file->fd >= 0) {
				/* Normal file descriptor. */
				while (write(file->fd, file->buffer,
				    file->buffer_offset) == -1) {
					if (errno != EINTR) {
						retval = STILO_THREADE_IOERROR;
						goto RETURN;
					}
				}
			} else {
				/* Use the write wrapper function. */
				if (file->write_f(file->arg, a_stilo,
				    file->buffer, file->buffer_offset)) {
					retval = STILO_THREADE_IOERROR;
					goto RETURN;
				}
			}
		}
		/*
		 * Reset the buffer to being empty, regardless of the type of
		 * buffered data.
		 */
		file->buffer_mode = BUFFER_EMPTY;
		file->buffer_offset = 0;
	}

	retval = STILO_THREADE_NONE;
	RETURN:
	return retval;
}

void
stilo_file_buffer_reset(cw_stilo_t *a_stilo)
{
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(file->stiloe.type == STILOT_FILE);

	stiloe_p_file_lock(file);
	file->buffer_mode = BUFFER_EMPTY;
	file->buffer_offset = 0;
	stiloe_p_file_unlock(file);
}

cw_bool_t
stilo_file_status(cw_stilo_t *a_stilo)
{
	cw_bool_t		retval;
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(file->stiloe.type == STILOT_FILE);

	stiloe_p_file_lock(file);
	if (file->fd != -1)
		retval = FALSE;
	else
		retval = TRUE;
	stiloe_p_file_unlock(file);

	return retval;
}

/* -1: STILO_THREADE_IOERROR */
cw_stiloi_t
stilo_file_mtime(cw_stilo_t *a_stilo)
{
	cw_stiloi_t		retval;
	struct stat		sb;
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(file->stiloe.type == STILOT_FILE);

	stiloe_p_file_lock(file);
	if (file->fd < 0) {
		retval = -1;
		goto RETURN;
	}

	if (fstat(file->fd, &sb) == -1) {
		retval = -1;
		goto RETURN;
	}
	/* XXX Replace this function with generic status. */
	/* Keep 63 bits of accuracy. */
/*  	retval = sb.st_mtimespec.tv_sec; */
/*  	retval <<= 31; */
/*  	retval |= (sb.st_mtimespec.tv_nsec >> 1); */
	retval = 0;

	RETURN:
	stiloe_p_file_unlock(file);
	return retval;
}
