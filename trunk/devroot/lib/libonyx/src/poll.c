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
 * Compatibility shim for poll() that uses select().
 *
 ******************************************************************************/

#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>

struct pollfd
{
    int fd;
    short events;
    short revents;
};

#define POLLIN		0x0001
#define POLLRDNORM	0x0002
#define POLLRDBAND	0x0004
#define POLLPRI		0x0008
#define POLLOUT		0x0010
#define POLLWRNORM	0x0020
#define POLLWRBAND	0x0040
#define POLLERR		0x0080
#define POLLHUP		0x0100
#define POLLNVAL	0x0200

static int
poll(struct pollfd *fds, unsigned int nfds, int timeout)
{
    int retval, maxfd = 0;
    fd_set rfds, wfds, efds, ifds;
    struct timeval *tout, toutbuf;
    unsigned i;

    FD_ZERO(&ifds);

    /* Loop in the case of a select() error due to an exceptional event. */
    while (1)
    {
	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_ZERO(&efds);

	/* Iterate through fds and set appropriate bits in rfds, wfds, and efds
	 * in preparation for the select() call. */
	for (i = 0; i < nfds; i++)
	{
	    cw_assert(fds[i].fd >= 0);

	    /* Only process this element if it was not marked to be ignored. */
	    if (FD_ISSET(fds[i].fd, &ifds) == 0)
	    {
		/* Check for read events? */
		if (fds[i].events & (POLLIN | POLLRDNORM))
		{
		    FD_SET(fds[i].fd, &rfds);
		    if (fds[i].fd > maxfd)
		    {
			maxfd = fds[i].fd;
		    }
		}

		/* Check for write events? */
		if (fds[i].events & (POLLOUT | POLLWRNORM | POLLWRBAND))
		{
		    FD_SET(fds[i].fd, &wfds);
		    if (fds[i].fd > maxfd)
		    {
			maxfd = fds[i].fd;
		    }
		}

		/* Check for exceptional events? */
		if (fds[i].events & (POLLRDBAND | POLLPRI))
		{
		    FD_SET(fds[i].fd, &efds);
		    if (fds[i].fd > maxfd)
		    {
			maxfd = fds[i].fd;
		    }
		}

		/* Clear revents. */
		fds[i].revents = 0;
	    }
	}

	/* Convert timeout. */
	if (timeout >= 0)
	{
	    toutbuf.tv_sec = timeout / 1000;
	    toutbuf.tv_usec = (timeout % 1000) * 1000;
	    tout = &toutbuf;
	}
	else
	{
	    tout = NULL;
	}

	/* Make the call. */
	retval = select(maxfd + 1, &rfds, &wfds, &efds, tout);

	/* Check for an error, and retry if the error is due to an invalid file
	 * descriptor. */
	if (retval != -1)
	{
	    break;
	}
	else
	{
	    if (errno == EBADF)
	    {
		struct stat buf;

		/* Determine which file descriptor caused the error.  Multiple
		 * errors cause multiple iterations through the enclosing while
		 * loop. */
		for (i = 0; i < nfds; i++)
		{
		    retval = fstat(fds[i].fd, &buf);
		    if (retval == -1 && errno == EBADF)
		    {
			FD_SET(fds[i].fd, &ifds);
			continue;
		    }
		}
		continue;
	    }
	    else
	    {
		goto RETURN;
	    }
	}
    }

    /* Iterate through fds and check if interesting bits were set by
     * select(). */
    for (i = retval = 0; i < nfds; i++)
    {
	if (FD_ISSET(fds[i].fd, &ifds))
	{
	    fds[i].revents = POLLNVAL;
	}
	else
	{
	    if (FD_ISSET(fds[i].fd, &rfds))
	    {
		if (fds[i].events & POLLIN)
		{
		    fds[i].revents |= POLLIN;
		}
		if (fds[i].events & POLLRDNORM)
		{
		    fds[i].revents |= POLLRDNORM;
		}
	    }
	    if (FD_ISSET(fds[i].fd, &wfds))
	    {
		if (fds[i].events & POLLOUT)
		{
		    fds[i].revents |= POLLOUT;
		}
		if (fds[i].events & POLLWRNORM)
		{
		    fds[i].revents |= POLLWRNORM;
		}
		if (fds[i].events & POLLWRBAND)
		{
		    fds[i].revents |= POLLWRBAND;
		}
	    }
	    if (FD_ISSET(fds[i].fd, &efds))
	    {
		if (fds[i].events & POLLRDBAND)
		{
		    fds[i].revents |= POLLRDBAND;
		}
		if (fds[i].events & POLLPRI)
		{
		    fds[i].revents |= POLLPRI;
		}
	    }
	}

	if (fds[i].revents != 0)
	{
	    retval++;
	}
    }

    RETURN:
    return retval;
}
