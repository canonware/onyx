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

#include "../include/slate.h"

void	usage(void);

int
main(int argc, char **argv, char **envp)
{
	int	retval;

	libonyx_init();

	usage();

	libonyx_shutdown();
	return retval;
}

void
usage(void)
{
	printf("slate usage:\n"
	    "    slate\n"
	    "    slate -h\n"
	    "    slate -V\n"
	    "    slate <file>+\n"
	    "    slate [-p <line>[:<column>]] <file>\n"
	    "\n"
	    "    Option               | Description\n"
	    "    ---------------------+-------------------------------------------------\n"
	    "    -h                   | Print usage and exit.\n"
	    "    -V                   | Print version information and exit.\n"
	    "    -p <line>[:<column>] | Open file <file> and move to line <line>, column\n"
	    "       <file>            | <column> (default 0).\n");
}
