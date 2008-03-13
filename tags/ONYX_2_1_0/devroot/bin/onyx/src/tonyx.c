/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Wrapper program that sets ONYX_MPATH and ONYX_RPATH approriately for running
 * onyx in the source tree.  This must be a binary executable rather than a
 * shell script due to how nested #! magic is (not) handled by the OS.
 *
 ******************************************************************************/

#include <stdio.h> /* XXX */
#include <stdlib.h>
#include <unistd.h>

#include "../include/onyx_defs.h"

int
main(int argc, char **argv)
{
	char	**arg;
	int	i;

	if (putenv(_CW_TONYX_RPATH) == -1)
		goto RETURN;
	if (putenv(_CW_TONYX_MPATH) == -1)
		goto RETURN;

	arg = (char **)malloc((argc + 1) * sizeof(char *));
	if (arg == NULL)
		goto RETURN;

	arg[0] = _CW_TONYX_ONYX;
	for (i = 1; i < argc; i++)
		arg[i] = argv[i];
	arg[i] = NULL;

	execv(_CW_TONYX_ONYX, arg);
	/* Not reached. */

	RETURN:
	return 1;
}
