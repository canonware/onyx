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

#include "../include/modpane.h"

#include <errno.h>

cw_ds_t *
ds_new(cw_ds_t *a_ds, int a_in, int a_out, const cw_uint8_t *a_term)
{
    cw_ds_t *retval;
    int error;
    struct termios term;
    cw_uint32_t term_strlen;

    cw_check_ptr(a_ds);

    retval = a_ds;
    retval->in = a_in;
    retval->out = a_out;
    term_strlen = strlen(a_term);
    retval->term = (cw_uint8_t *)cw_malloc(term_strlen + 1);
    memcpy(retval->term, a_term, term_strlen + 1);

    fprintf(stderr, "%d %d %s\n", a_in, a_out, a_term);

    do
    {
	error = tcgetattr(retval->in, &retval->term_saved);
	if (error)
	{
	    switch (errno)
	    {
		case EINTR:
		{
		    break;
		}
		case EBADF:
		case ENOTTY:
		case EINVAL:
		default:
		{
		    fprintf(stderr, "%s:%d:%s(): Whoops\n", __FILE__, __LINE__,
			    __FUNCTION__);
		    cw_free(retval->term);
		    retval = NULL;
		    goto RETURN;
		}
	    }
	}
    } while (error != 0);

    /* Switch to raw mode. */
    memcpy(&term, &retval->term_saved, sizeof(struct termios));
    cfmakeraw(&term);
    do
    {
	error = tcsetattr(retval->in, TCSANOW, &term);
	if (error)
	{
	    switch (errno)
	    {
		case EINTR:
		{
		    break;
		}
		case EBADF:
		case ENOTTY:
		case EINVAL:
		default:
		{
		    fprintf(stderr, "%s:%d:%s(): Whoops, %d (%s)\n", __FILE__, __LINE__, __FUNCTION__, errno, strerror(errno));
		    cw_free(retval->term);
		    retval = NULL;
		    goto RETURN;
		}
	    }
	}
    } while (error != 0);

    RETURN:
    return retval;
}

void
ds_delete(cw_ds_t *a_ds)
{
    int error;

    /* Restore terminal settings. */
    do
    {
	error = tcsetattr(a_ds->in, TCSADRAIN | TCSAFLUSH, &a_ds->term_saved);
	if (error)
	{
	    switch (errno)
	    {
		case EINTR:
		{
		    break;
		}
		case EBADF:
		case ENOTTY:
		case EINVAL:
		default:
		{
		    goto RETURN;
		}
	    }
	}
    } while (error != 0);

    RETURN:
    cw_free(a_ds->term);
}
