/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Wrapper program that sets SLATE_MPATH and SLATE_RPATH appropriately for
 * running slate in the source tree.  This must be a binary executable rather
 * than a shell script due to how nested #! magic is (not) handled by the OS.
 *
 ******************************************************************************/

#include <stdlib.h>
#include <unistd.h>

#include "../include/slate_defs.h"

int
main(int argc, char **argv)
{
	char	**arg;
	int	i;

	if (putenv(CW_TSLATE_RPATH) == -1)
		goto RETURN;
	if (putenv(CW_TSLATE_MPATH) == -1)
		goto RETURN;

	arg = (char **)malloc((argc + 1) * sizeof(char *));
	if (arg == NULL)
		goto RETURN;

	arg[0] = CW_TSLATE_SLATE;
	for (i = 1; i < argc; i++)
		arg[i] = argv[i];
	arg[i] = NULL;

	execv(CW_TSLATE_SLATE, arg);
	/* Not reached. */

	RETURN:
	return 1;
}
