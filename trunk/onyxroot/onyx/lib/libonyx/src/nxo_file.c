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

#include "../include/libonyx/libonyx.h"

#ifdef _CW_POSIX_FILE
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef HAVE_POLL
#include <poll.h>
#endif
#endif

#include "../include/libonyx/nxa_l.h"
#include "../include/libonyx/nxo_l.h"
#include "../include/libonyx/nxo_file_l.h"

static cw_nxn_t nxo_p_file_buffer_flush(cw_nxoe_file_t *a_file);

#ifdef _CW_THREADS
#define		nxoe_p_file_lock(a_nxoe) do {			\
	if ((a_nxoe)->nxoe.locking)					\
		mtx_lock(&(a_nxoe)->lock);				\
} while (0)
#define		nxoe_p_file_unlock(a_nxoe) do {			\
	if ((a_nxoe)->nxoe.locking)					\
		mtx_unlock(&(a_nxoe)->lock);				\
} while (0)
#endif

void
nxo_file_new(cw_nxo_t *a_nxo, cw_nx_t *a_nx, cw_bool_t a_locking)
{
	cw_nxoe_file_t	*file;

	file = (cw_nxoe_file_t *)nxa_malloc(nx_nxa_get(a_nx),
	    sizeof(cw_nxoe_file_t));

	nxoe_l_new(&file->nxoe, NXOT_FILE, a_locking);
#ifdef _CW_THREADS
	if (a_locking)
		mtx_new(&file->lock);
#endif
	file->nx = a_nx;
	file->mode = FILE_NONE;

	file->buffer = NULL;
	file->buffer_mode = BUFFER_EMPTY;
	file->buffer_size = 0;
	file->buffer_offset = 0;

	nxo_no_new(a_nxo);
	a_nxo->o.nxoe = (cw_nxoe_t *)file;
	nxo_p_type_set(a_nxo, NXOT_FILE);

	nxa_l_gc_register(nx_nxa_get(a_nx), (cw_nxoe_t *)file);
}

cw_bool_t
nxoe_l_file_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t a_iter)
{
	cw_nxoe_file_t	*file;

	file = (cw_nxoe_file_t *)a_nxoe;

	_cw_check_ptr(file);
	_cw_dassert(file->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(file->nxoe.type == NXOT_FILE);

	nxo_p_file_buffer_flush(file);
	if (file->buffer != NULL)
		nxa_free(a_nxa, file->buffer, file->buffer_size);
#ifdef _CW_THREADS
	if (file->nxoe.locking)
		mtx_delete(&file->lock);
#endif
	switch (file->mode) {
	case FILE_NONE:
		break;
#ifdef _CW_POSIX_FILE
	case FILE_POSIX:
		if (file->f.p.wrapped == FALSE)
			close(file->f.p.fd);
		break;
#endif
	case FILE_SYNTHETIC:
		if (file->f.s.delete_f != NULL)
			file->f.s.delete_f(file->f.s.arg, nxa_nx_get(a_nxa));
		break;
	}

	nxa_free(a_nxa, file, sizeof(cw_nxoe_file_t));

	return FALSE;
}

cw_nxoe_t *
nxoe_l_file_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset)
{
	cw_nxoe_t	*retval;
	cw_nxoe_file_t	*file;

	file = (cw_nxoe_file_t *)a_nxoe;

	if (file->mode == FILE_SYNTHETIC && file->f.s.ref_iter_f != NULL)
		retval = file->f.s.ref_iter_f(file->f.s.arg, a_reset);
	else
		retval = NULL;

	return retval;
}

#ifdef _CW_POSIX_FILE
void
nxo_file_fd_wrap(cw_nxo_t *a_nxo, cw_uint32_t a_fd)
{
	cw_nxoe_file_t	*file;

	_cw_check_ptr(a_nxo);
	_cw_dassert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(nxo_type_get(a_nxo) == NXOT_FILE);

	file = (cw_nxoe_file_t *)a_nxo->o.nxoe;

	_cw_check_ptr(file);
	_cw_dassert(file->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(file->nxoe.type == NXOT_FILE);
	_cw_assert(file->mode == FILE_NONE);

	file->mode = FILE_POSIX;
	file->f.p.fd = a_fd;
	file->f.p.wrapped = TRUE;
}
#endif

void
nxo_file_synthetic(cw_nxo_t *a_nxo, cw_nxo_file_read_t *a_read,
    cw_nxo_file_write_t *a_write, cw_nxo_file_ref_iter_t *a_ref_iter,
    cw_nxo_file_delete_t *a_delete, void *a_arg)
{
	cw_nxoe_file_t	*file;

	_cw_check_ptr(a_nxo);
	_cw_dassert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(nxo_type_get(a_nxo) == NXOT_FILE);

	file = (cw_nxoe_file_t *)a_nxo->o.nxoe;

	_cw_check_ptr(file);
	_cw_dassert(file->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(file->nxoe.type == NXOT_FILE);
	_cw_assert(file->mode == FILE_NONE);

	file->mode = FILE_SYNTHETIC;
	file->f.s.read_f = a_read;
	file->f.s.write_f = a_write;
	file->f.s.ref_iter_f = a_ref_iter;
	file->f.s.delete_f = a_delete;
	file->f.s.arg = a_arg;
	file->f.s.position = 0;
}

#ifdef _CW_POSIX_FILE
/*
 * NXN_ioerror, NXN_limitcheck,
 * NXN_invalidfileaccess
 */
cw_nxn_t
nxo_file_open(cw_nxo_t *a_nxo, const cw_uint8_t *a_filename, cw_uint32_t a_nlen,
    const cw_uint8_t *a_flags, cw_uint32_t a_flen)
{
	cw_nxn_t		retval;
	cw_nxoe_file_t		*file;
	cw_uint8_t		filename[PATH_MAX], flags[3];
	int			access;

	/*
	 * Copy the arguments to local buffers in order to assure '\0'
	 * termination.
	 */
	if (a_nlen >= sizeof(filename)) {
		retval = NXN_limitcheck;
		goto RETURN;
	}
	memcpy(filename, a_filename, a_nlen);
	filename[a_nlen] = '\0';

	if (a_flen >= sizeof(flags)) {
		retval = NXN_limitcheck;
		goto RETURN;
	}
	memcpy(flags, a_flags, a_flen);
	flags[a_flen] = '\0';

	_cw_check_ptr(a_nxo);
	_cw_dassert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(nxo_type_get(a_nxo) == NXOT_FILE);

	file = (cw_nxoe_file_t *)a_nxo->o.nxoe;

	_cw_check_ptr(file);
	_cw_dassert(file->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(file->nxoe.type == NXOT_FILE);

#ifdef _CW_THREADS
	nxoe_p_file_lock(file);
#endif
	if (file->mode != FILE_NONE) {
		retval = NXN_invalidfileaccess;
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
			retval = NXN_invalidfileaccess;
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
			retval = NXN_invalidfileaccess;
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
			retval = NXN_invalidfileaccess;
			goto URETURN;
		}
		break;
	default:
		retval = NXN_invalidfileaccess;
		goto URETURN;
	}

	file->f.p.fd = open(filename, access, 0x1ff);
	if (file->f.p.fd == -1) {
		switch (errno) {
		case ENOSPC:
		case EMFILE:
		case ENFILE:
			retval = NXN_ioerror;
			goto URETURN;
		default:
			retval = NXN_invalidfileaccess;
			goto URETURN;
		}
	}

	file->mode = FILE_POSIX;
	file->f.p.wrapped = FALSE;

	retval = NXN_ZERO;
	URETURN:
#ifdef _CW_THREADS
	nxoe_p_file_unlock(file);
#endif
	RETURN:
	return retval;
}
#endif

/* NXN_ioerror */
cw_nxn_t
nxo_file_close(cw_nxo_t *a_nxo)
{
	cw_nxn_t		retval;
	cw_nxoe_file_t		*file;

	_cw_check_ptr(a_nxo);
	_cw_dassert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(nxo_type_get(a_nxo) == NXOT_FILE);

	file = (cw_nxoe_file_t *)a_nxo->o.nxoe;

	_cw_check_ptr(file);
	_cw_dassert(file->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(file->nxoe.type == NXOT_FILE);

#ifdef _CW_THREADS
	nxoe_p_file_lock(file);
#endif
	if (file->mode == FILE_NONE) {
		retval = NXN_ioerror;
		goto RETURN;
	}

	/* Flush and get rid of the buffer if necessary. */
	retval = nxo_p_file_buffer_flush(file);
	if (retval)
		goto RETURN;
	if (file->buffer != NULL) {
		nxa_free(nx_nxa_get(file->nx), file->buffer, file->buffer_size);
		file->buffer = NULL;
		file->buffer_size = 0;
		file->buffer_mode = BUFFER_EMPTY;
	}

	switch (file->mode) {
	case FILE_NONE:
		_cw_not_reached();
#ifdef _CW_POSIX_FILE
	case FILE_POSIX:
		file->mode = FILE_NONE;
		if (file->f.p.wrapped == FALSE) {
			if (close(file->f.p.fd) == -1) {
				retval = NXN_ioerror;
				goto RETURN;
			}
		}
		break;
#endif
	case FILE_SYNTHETIC:
		file->mode = FILE_NONE;
		if (file->f.s.delete_f != NULL)
			file->f.s.delete_f(file->f.s.arg, file->nx);
		break;
	}

	retval = NXN_ZERO;
	RETURN:
#ifdef _CW_THREADS
	nxoe_p_file_unlock(file);
#endif
	return retval;
}

#ifdef _CW_POSIX_FILE
cw_sint32_t
nxo_file_fd_get(cw_nxo_t *a_nxo)
{
	cw_sint32_t	retval;
	cw_nxoe_file_t	*file;

	_cw_check_ptr(a_nxo);
	_cw_dassert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(nxo_type_get(a_nxo) == NXOT_FILE);

	file = (cw_nxoe_file_t *)a_nxo->o.nxoe;

	_cw_check_ptr(file);
	_cw_dassert(file->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(file->nxoe.type == NXOT_FILE);

#ifdef _CW_THREADS
	nxoe_p_file_lock(file);
#endif
	switch (file->mode) {
	case FILE_NONE:
	case FILE_SYNTHETIC:
		retval = -1;
		break;
#ifdef _CW_POSIX_FILE
	case FILE_POSIX:
		retval = file->f.p.fd;
		break;
#endif
	}

#ifdef _CW_THREADS
	nxoe_p_file_unlock(file);
#endif
	return retval;
}
#endif

/* -1: NXN_ioerror */
cw_sint32_t
nxo_file_read(cw_nxo_t *a_nxo, cw_uint32_t a_len, cw_uint8_t *r_str)
{
	cw_sint32_t	retval;
	cw_nxoe_file_t	*file;

	_cw_check_ptr(a_nxo);
	_cw_dassert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(nxo_type_get(a_nxo) == NXOT_FILE);

	file = (cw_nxoe_file_t *)a_nxo->o.nxoe;

	_cw_check_ptr(file);
	_cw_dassert(file->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(file->nxoe.type == NXOT_FILE);

#ifdef _CW_THREADS
	nxoe_p_file_lock(file);
#endif
	if (file->mode == FILE_NONE) {
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
			ssize_t	nread;

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

			switch (file->mode) {
			case FILE_NONE:
				_cw_not_reached();
#ifdef _CW_POSIX_FILE
			case FILE_POSIX: {
#ifdef HAVE_POLL
				struct pollfd	events;
#else
				fd_set		fdset;
#endif
				struct iovec	iov[2];

				if (retval == 0) {
					/*
					 * No data read yet.  Sleep until some
					 * data are available.
					 */
#ifdef HAVE_POLL
					events.fd = file->f.p.fd;
					events.events = POLLIN
#ifdef POLLRDNORM
					    | POLLRDNORM
#endif
					    ;
					while ((poll(&events, 1, -1)) == -1)
						; /* EINTR; try again. */
#else
					_cw_assert(file->f.p.fd <= FD_SETSIZE);
					FD_ZERO(&fdset);
					FD_SET(file->f.p.fd, &fdset);
					while (select(file->f.p.fd + 1, &fdset,
					    NULL, NULL, NULL) == -1)
						; /* EINTR; try again. */
#endif

					/*
					 * Finish filling r_str and replenish
					 * the internal buffer.
					 */
					iov[0].iov_base = r_str;
					iov[0].iov_len = a_len;
					iov[1].iov_base = file->buffer;
					iov[1].iov_len = file->buffer_size;

					while ((nread = readv(file->f.p.fd, iov,
					    2)) == -1) {
						if (errno != EINTR)
							break;
					}
				} else {
					int		nready;
#ifndef HAVE_POLL
					struct timeval	timeout;
#endif

					/*
					 * Only read if data are available.
					 */
#ifdef HAVE_POLL
					events.fd = finle->f.p.fd;
					events.events = POLLIN
#ifdef POLLRDNORM
					    | POLLRDNORM
#endif
					    ;
					while ((nready = poll(&events, 1, 0)) ==
					    -1)
						; /* EINTR; try again. */
#else
					_cw_assert(file->f.p.fd <= FD_SETSIZE);
					FD_ZERO(&fdset);
					FD_SET(file->f.p.fd, &fdset);
					timeout.tv_sec = 0;
					timeout.tv_usec = 0;
					while ((nready = select(file->f.p.fd +
					    1, &fdset, NULL, NULL, &timeout)) ==
						   -1)
						; /* EINTR; try again. */
#endif

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

						while ((nread =
						    readv(file->f.p.fd, iov, 2))
						    == -1) {
							if (errno != EINTR)
								break;
						}
					} else
						nread = 0;
				}
				break;
			}
#endif
			case FILE_SYNTHETIC:
				/* Use the read wrapper function. */
				nread = file->f.s.read_f(file->f.s.arg, a_nxo,
				    a_len, r_str);
				break;
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
		switch (file->mode) {
		case FILE_NONE:
			_cw_not_reached();
#ifdef _CW_POSIX_FILE
		case FILE_POSIX:
			while ((retval = read(file->f.p.fd, r_str, a_len)) ==
			    -1) {
				if (errno != EINTR)
					break;
			}
			break;
#endif
		case FILE_SYNTHETIC:
			retval = file->f.s.read_f(file->f.s.arg, a_nxo, a_len,
			    r_str);
			break;
		}
	}

	if (retval == 0 && file->buffer != NULL) {
		file->buffer_offset = 0;
		file->buffer_mode = BUFFER_EMPTY;
	}
	RETURN:
	if (file->mode == FILE_SYNTHETIC && retval != -1)
		file->f.s.position += retval;
#ifdef _CW_THREADS
	nxoe_p_file_unlock(file);
#endif
	return retval;
}

/* NXN_ioerror */
cw_nxn_t
nxo_file_readline(cw_nxo_t *a_nxo, cw_bool_t a_locking, cw_nxo_t *r_string,
    cw_bool_t *r_eof)
{
	cw_nxn_t		retval;
	cw_nxoe_file_t		*file;
	cw_uint8_t		*line, s_line[_CW_NXO_FILE_READLINE_BUFSIZE];
	cw_uint32_t		i, maxlen;
	cw_sint32_t		nread;
	enum {NORMAL, CR}	state;

	_cw_check_ptr(a_nxo);
	_cw_dassert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(nxo_type_get(a_nxo) == NXOT_FILE);

	file = (cw_nxoe_file_t *)a_nxo->o.nxoe;

	_cw_check_ptr(file);
	_cw_dassert(file->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(file->nxoe.type == NXOT_FILE);

	line = s_line;

#ifdef _CW_THREADS
	nxoe_p_file_lock(file);
#endif
	if (file->mode == FILE_NONE) {
		retval = NXN_ioerror;
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
			retval = nxo_p_file_buffer_flush(file);
			if (retval)
				goto RETURN;
		}

		/*
		 * Copy bytes from the internal buffer, one at a time,
		 * replenishing the internal buffer as necessary.
		 */
		for (i = 0, offset = 0, maxlen =
		    _CW_NXO_FILE_READLINE_BUFSIZE, state = NORMAL;; i++) {
			/*
			 * If we're about to overflow the line buffer, expand
			 * it.
			 */
			if (i == maxlen) {
				if (line == s_line) {
					/* First expansion. */
					line = (cw_uint8_t
					    *)nxa_malloc(nx_nxa_get(file->nx),
					    maxlen << 1);
					memcpy(line, s_line, maxlen);
				} else {
					cw_uint8_t	*oldline;

					/*
					 * We've already expanded at least
					 * once.
					 */
					oldline = line;
					line = (cw_uint8_t
					    *)nxa_malloc(nx_nxa_get(file->nx),
					    maxlen << 1);
					memcpy(line, oldline, maxlen);
					nxa_free(nx_nxa_get(file->nx), oldline,
					    maxlen);
				}
				maxlen <<= 1;
			}

			if ((offset >= file->buffer_offset) ||
			    (file->buffer_mode == BUFFER_EMPTY)) {
				/* Replenish the internal buffer. */
				switch (file->mode) {
				case FILE_NONE:
					_cw_not_reached();
#ifdef _CW_POSIX_FILE
				case FILE_POSIX:
					while ((nread = read(file->f.p.fd,
					    file->buffer, file->buffer_size)) ==
					    -1) {
						if (errno != EINTR)
							break;
					}
					break;
#endif
				case FILE_SYNTHETIC:
					/* Use the read wrapper function. */
					nread = file->f.s.read_f(file->f.s.arg,
					    a_nxo, file->buffer_size,
					    file->buffer);
					break;
				}

				if (nread <= 0) {
					/* EOF. */
					file->buffer_offset = 0;
					file->buffer_mode = BUFFER_EMPTY;

					nxo_string_new(r_string, file->nx,
					    a_locking, i);
					nxo_string_lock(r_string);
					nxo_string_set(r_string, 0, line, i);
					nxo_string_unlock(r_string);

					if (file->mode == FILE_SYNTHETIC)
						file->f.s.position += i;

					retval = NXN_ZERO;
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
				nxo_string_new(r_string, file->nx, a_locking,
				    i);
				nxo_string_lock(r_string);
				nxo_string_set(r_string, 0, line, i);
				nxo_string_unlock(r_string);

				if (file->mode == FILE_SYNTHETIC)
					file->f.s.position += i;

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

				retval = NXN_ZERO;
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
		for (i = 0, maxlen = _CW_NXO_FILE_READLINE_BUFSIZE, state =
			 NORMAL;; i++) {
			/*
			 * If we're about to overflow the line buffer, expand
			 * it.
			 */
			if (i == maxlen) {
				if (line == s_line) {
					/* First expansion. */
					line = (cw_uint8_t
					    *)nxa_malloc(nx_nxa_get(file->nx),
					    maxlen << 1);
					memcpy(line, s_line, maxlen);
				} else {
					cw_uint8_t	*oldline;

					/*
					 * We've already expanded at least
					 * once.
					 */
					oldline = line;
					line = (cw_uint8_t
					    *)nxa_malloc(nx_nxa_get(file->nx),
					    maxlen << 1);
					memcpy(line, oldline, maxlen);
					nxa_free(nx_nxa_get(file->nx), oldline,
					    maxlen);
				}
				maxlen <<= 1;
			}

			switch (file->mode) {
			case FILE_NONE:
				_cw_not_reached();
#ifdef _CW_POSIX_FILE
			case FILE_POSIX:
				while ((nread = read(file->f.p.fd, &line[i], 1))
				    == -1) {
					if (errno != EINTR)
						break;
				}
				break;
#endif
			case FILE_SYNTHETIC:
				/* Use the read wrapper function. */
				nread = file->f.s.read_f(file->f.s.arg, a_nxo,
				    1, &line[i]);
				break;
			}

			if (nread <= 0) {
				/* EOF. */
				nxo_string_new(r_string, file->nx, a_locking,
				    i);
				nxo_string_lock(r_string);
				nxo_string_set(r_string, 0, line, i);
				nxo_string_unlock(r_string);

				if (file->mode == FILE_SYNTHETIC)
					file->f.s.position += i;

				retval = NXN_ZERO;
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
				nxo_string_new(r_string, file->nx, a_locking,
				    i);
				nxo_string_lock(r_string);
				nxo_string_set(r_string, 0, line, i);
				nxo_string_unlock(r_string);

				if (file->mode == FILE_SYNTHETIC)
					file->f.s.position += i;

				retval = NXN_ZERO;
				*r_eof = FALSE;
				goto RETURN;
			default:
				state = NORMAL;
				break;
			}
		}
	}

	RETURN:
#ifdef _CW_THREADS
	nxoe_p_file_unlock(file);
#endif
	if (line != s_line)
		nxa_free(nx_nxa_get(file->nx), line, maxlen);
	return retval;
}

/* NXN_ioerror */
cw_nxn_t
nxo_file_write(cw_nxo_t *a_nxo, const cw_uint8_t *a_str, cw_uint32_t a_len)
{
	cw_nxn_t		retval;
	cw_nxoe_file_t		*file;

	_cw_check_ptr(a_nxo);
	_cw_dassert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(nxo_type_get(a_nxo) == NXOT_FILE);

	file = (cw_nxoe_file_t *)a_nxo->o.nxoe;

	_cw_check_ptr(file);
	_cw_dassert(file->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(file->nxoe.type == NXOT_FILE);

#ifdef _CW_THREADS
	nxoe_p_file_lock(file);
#endif
	if (file->mode == FILE_NONE) {
		retval = NXN_ioerror;
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
			memcpy(&file->buffer[file->buffer_offset], a_str,
			    a_len);
			file->buffer_mode = BUFFER_WRITE;
			file->buffer_offset += a_len;
		} else {
			switch (file->mode) {
			case FILE_NONE:
				_cw_not_reached();
#ifdef _CW_POSIX_FILE
			case FILE_POSIX: {
				struct iovec	iov[2];

				/* a_str won't fit.  Do a writev(). */

				iov[0].iov_base = file->buffer;
				iov[0].iov_len = file->buffer_offset;
				iov[1].iov_base = (char *)a_str;
				iov[1].iov_len = a_len;

				while (writev(file->f.p.fd, iov, 2) == -1) {
					if (errno != EINTR) {
						retval = NXN_ioerror;
						goto RETURN;
					}
				}

				file->buffer_mode = BUFFER_EMPTY;
				file->buffer_offset = 0;
				break;
			}
#endif
			case FILE_SYNTHETIC:
				/*
				 * a_str won't fit.  Flush the buffer and call
				 * the write wrapper function.
				 */
				retval = nxo_p_file_buffer_flush(file);
				if (retval)
					goto RETURN;

				if (file->f.s.write_f(file->f.s.arg, a_nxo,
				    a_str, a_len)) {
					retval = NXN_ioerror;
					goto RETURN;
				}
				file->f.s.position += a_len;
			}
		}
	} else {
		switch (file->mode) {
		case FILE_NONE:
			_cw_not_reached();
#ifdef _CW_POSIX_FILE
		case FILE_POSIX:
			while (write(file->f.p.fd, a_str, a_len) == -1) {
				if (errno != EINTR) {
					retval = NXN_ioerror;
					goto RETURN;
				}
			}
			break;
#endif
		case FILE_SYNTHETIC:
			if (file->f.s.write_f(file->f.s.arg, a_nxo, a_str,
			    a_len)) {
				retval = NXN_ioerror;
				goto RETURN;
			}
			file->f.s.position += a_len;
		}
	}

	retval = NXN_ZERO;
	RETURN:
#ifdef _CW_THREADS
	nxoe_p_file_unlock(file);
#endif
	return retval;
}

#ifdef _CW_POSIX_FILE
/* NXN_ioerror */
cw_nxn_t
nxo_file_truncate(cw_nxo_t *a_nxo, off_t a_length)
{
	cw_nxn_t		retval;
	cw_nxoe_file_t		*file;

	_cw_check_ptr(a_nxo);
	_cw_dassert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(nxo_type_get(a_nxo) == NXOT_FILE);

	file = (cw_nxoe_file_t *)a_nxo->o.nxoe;

	_cw_check_ptr(file);
	_cw_dassert(file->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(file->nxoe.type == NXOT_FILE);

#ifdef _CW_THREADS
	nxoe_p_file_lock(file);
#endif
	switch (file->mode) {
	case FILE_NONE:
	case FILE_SYNTHETIC:
		retval = NXN_ioerror;
		goto RETURN;
	case FILE_POSIX:
		nxo_p_file_buffer_flush(file);
		if (ftruncate(file->f.p.fd, a_length)) {
			retval = NXN_ioerror;
			goto RETURN;
		}
	}

	retval = NXN_ZERO;
	RETURN:
#ifdef _CW_THREADS
	nxoe_p_file_unlock(file);
#endif
	return retval;
}
#endif

/* -1: NXN_ioerror */
cw_nxoi_t
nxo_file_position_get(cw_nxo_t *a_nxo)
{
	cw_nxoi_t		retval;
	cw_nxoe_file_t		*file;

	_cw_check_ptr(a_nxo);
	_cw_dassert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(nxo_type_get(a_nxo) == NXOT_FILE);

	file = (cw_nxoe_file_t *)a_nxo->o.nxoe;

	_cw_check_ptr(file);
	_cw_dassert(file->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(file->nxoe.type == NXOT_FILE);

#ifdef _CW_THREADS
	nxoe_p_file_lock(file);
#endif

	switch (file->mode) {
	case FILE_NONE:
		retval = -1;
		break;
#ifdef _CW_POSIX_FILE
	case FILE_POSIX:
		/*
		 * Flush the file in case there are buffered data that have yet
		 * to be written.
		 */
		retval = nxo_p_file_buffer_flush(file);
		if (retval)
			break;
		retval = lseek(file->f.p.fd, 0, SEEK_CUR);
		break;
#endif
	case FILE_SYNTHETIC:
		retval = file->f.s.position;
	}

#ifdef _CW_THREADS
	nxoe_p_file_unlock(file);
#endif
	return retval;
}

#ifdef _CW_POSIX_FILE
/* NXN_ioerror */
cw_nxn_t
nxo_file_position_set(cw_nxo_t *a_nxo, cw_nxoi_t a_position)
{
	cw_nxn_t		retval;
	cw_nxoe_file_t		*file;

	_cw_check_ptr(a_nxo);
	_cw_dassert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(nxo_type_get(a_nxo) == NXOT_FILE);

	file = (cw_nxoe_file_t *)a_nxo->o.nxoe;

	_cw_check_ptr(file);
	_cw_dassert(file->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(file->nxoe.type == NXOT_FILE);

#ifdef _CW_THREADS
	nxoe_p_file_lock(file);
#endif
	switch (file->mode) {
	case FILE_NONE:
	case FILE_SYNTHETIC:
		retval = NXN_ioerror;
		break;
	case FILE_POSIX:
		retval = nxo_p_file_buffer_flush(file);
		if (retval)
			break;

		if (lseek(file->f.p.fd, a_position, SEEK_SET) == -1)
			retval = NXN_ioerror;
	}

#ifdef _CW_THREADS
	nxoe_p_file_unlock(file);
#endif
	return retval;
}
#endif

cw_uint32_t
nxo_file_buffer_size_get(cw_nxo_t *a_nxo)
{
	cw_uint32_t		retval;
	cw_nxoe_file_t		*file;

	_cw_check_ptr(a_nxo);
	_cw_dassert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(nxo_type_get(a_nxo) == NXOT_FILE);

	file = (cw_nxoe_file_t *)a_nxo->o.nxoe;

	_cw_check_ptr(file);
	_cw_dassert(file->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(file->nxoe.type == NXOT_FILE);

#ifdef _CW_THREADS
	nxoe_p_file_lock(file);
#endif
	retval = file->buffer_size;
#ifdef _CW_THREADS
	nxoe_p_file_unlock(file);
#endif

	return retval;
}

void
nxo_file_buffer_size_set(cw_nxo_t *a_nxo, cw_uint32_t a_size)
{
	cw_nxoe_file_t	*file;
	cw_nxa_t	*nxa;

	_cw_check_ptr(a_nxo);
	_cw_dassert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(nxo_type_get(a_nxo) == NXOT_FILE);

	file = (cw_nxoe_file_t *)a_nxo->o.nxoe;

	_cw_check_ptr(file);
	_cw_dassert(file->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(file->nxoe.type == NXOT_FILE);

	nxa = nx_nxa_get(file->nx);
#ifdef _CW_THREADS
	nxoe_p_file_lock(file);
#endif
	if (a_size == 0) {
		if (file->buffer != NULL) {
			nxa_free(nxa, file->buffer, file->buffer_size);
			file->buffer = NULL;
			file->buffer_size = 0;
		}
	} else {
		if (file->buffer != NULL)
			nxa_free(nxa, file->buffer, file->buffer_size);
		file->buffer = (cw_uint8_t *)nxa_malloc(nx_nxa_get(file->nx),
		    a_size);
		file->buffer_size = a_size;
	}
	file->buffer_mode = BUFFER_EMPTY;
	file->buffer_offset = 0;
#ifdef _CW_THREADS
	nxoe_p_file_unlock(file);
#endif
}

cw_nxoi_t
nxo_file_buffer_count(cw_nxo_t *a_nxo)
{
	cw_nxoi_t		retval;
	cw_nxoe_file_t		*file;

	_cw_check_ptr(a_nxo);
	_cw_dassert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(nxo_type_get(a_nxo) == NXOT_FILE);

	file = (cw_nxoe_file_t *)a_nxo->o.nxoe;

	_cw_check_ptr(file);
	_cw_dassert(file->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(file->nxoe.type == NXOT_FILE);

#ifdef _CW_THREADS
	nxoe_p_file_lock(file);
#endif
	if ((file->mode != FILE_NONE && file->buffer != NULL &&
	    file->buffer_mode != BUFFER_WRITE))
		retval = file->buffer_offset;
	else
		retval = 0;
#ifdef _CW_THREADS
	nxoe_p_file_unlock(file);
#endif

	return retval;
}

/* NXN_ioerror */
cw_nxn_t
nxo_file_buffer_flush(cw_nxo_t *a_nxo)
{
	cw_nxn_t		retval;
	cw_nxoe_file_t		*file;

	_cw_check_ptr(a_nxo);
	_cw_dassert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(nxo_type_get(a_nxo) == NXOT_FILE);

	file = (cw_nxoe_file_t *)a_nxo->o.nxoe;

	_cw_check_ptr(file);
	_cw_dassert(file->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(file->nxoe.type == NXOT_FILE);

#ifdef _CW_THREADS
	nxoe_p_file_lock(file);
#endif
	retval = nxo_p_file_buffer_flush(file);
#ifdef _CW_THREADS
	nxoe_p_file_unlock(file);
#endif

	return retval;
}

static cw_nxn_t
nxo_p_file_buffer_flush(cw_nxoe_file_t *a_file)
{
	cw_nxn_t	retval;

	if (a_file->mode == FILE_NONE) {
		retval = NXN_ioerror;
		goto RETURN;
	}
	
	if (a_file->buffer != NULL) {
		/* Only write if the buffered data is for writing. */
		if (a_file->buffer_mode == BUFFER_WRITE) {
			switch (a_file->mode) {
			case FILE_NONE:
				_cw_not_reached();
#ifdef _CW_POSIX_FILE
			case FILE_POSIX:
				while (write(a_file->f.p.fd, a_file->buffer,
				    a_file->buffer_offset) == -1) {
					if (errno != EINTR) {
						retval = NXN_ioerror;
						goto RETURN;
					}
				}
				break;
#endif
			case FILE_SYNTHETIC: {
				cw_nxo_t	tnxo;

				/* Fake up a file object. */
				nxo_p_new(&tnxo, NXOT_FILE);
				tnxo.o.nxoe = (cw_nxoe_t *)a_file;

				/* Use the write wrapper function. */
				if (a_file->f.s.write_f(a_file->f.s.arg, &tnxo,
				    a_file->buffer, a_file->buffer_offset)) {
					retval = NXN_ioerror;
					goto RETURN;
				}
			}
			}
		}
		/*
		 * Reset the buffer to being empty, regardless of the type of
		 * buffered data.
		 */
		a_file->buffer_mode = BUFFER_EMPTY;
		a_file->buffer_offset = 0;
	}

	retval = NXN_ZERO;
	RETURN:
	return retval;
}
