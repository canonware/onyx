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

/* Include generated code. */
#include "slate_nxcode.c"

void	usage(void);

int
main(int argc, char **argv, char **envp)
{
	int		retval;
	cw_nx_t		nx;
	cw_nxo_t	thread;

	libonyx_init();
	nx_new(&nx, NULL, argc, argv, envp);
	nxo_thread_new(&thread, &nx);

	slate_nxcode(&thread);
	usage();

	nx_delete(&nx);
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
