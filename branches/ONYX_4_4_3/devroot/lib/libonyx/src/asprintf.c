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
 * Emulate asprintf().
 *
 ******************************************************************************/

#include <stdio.h>

static int
asprintf(char **r_result, const char *a_format, ...)
{
    int retval;
    char *result;
    va_list ap;
#define CW_ASPRINTF_STARTLEN 16

    result = (char *) malloc(CW_ASPRINTF_STARTLEN);
    if (result == NULL)
    {
	retval = -1;
	goto RETURN;
    }

    va_start(ap, a_format);
    retval = vsnprintf(result, CW_ASPRINTF_STARTLEN, a_format, ap);
    va_end(ap);

    if (retval == -1)
    {
	goto RETURN;
    }
    else if (retval >= CW_ASPRINTF_STARTLEN)
    {
	free(result);
	result = (char *) malloc(retval + 1);
	if (result == NULL)
	{
	    retval = -1;
	    goto RETURN;
	}

	va_start(ap, a_format);
	retval = vsnprintf(result, retval + 1, a_format, ap);
	va_end(ap);
    }

    *r_result = result;
    RETURN:
    return retval;
}
