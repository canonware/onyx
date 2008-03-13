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
 * Initialization and shutdown functions for the whole library.
 *
 ******************************************************************************/

#define CW_LIBONYX_C_

#include "../include/libonyx/libonyx.h"
#ifdef CW_POSIX
#include "../include/libonyx/envdict_l.h"
#endif
#include "../include/libonyx/gcdict_l.h"

/* Prototypes for library-private functions that are only used in this file. */
#ifdef CW_THREADS
void
thd_l_init(void);
void
thd_l_shutdown(void);
#endif
void
xep_l_init(void);
void
xep_l_shutdown(void);
void
mem_l_init(void);
void
mem_l_shutdown(void);
void
nxa_l_init(void);
void
nxa_l_shutdown(void);
void
systemdict_l_init(void);
void
systemdict_l_shutdown(void);
void
origin_l_init(void);
void
origin_l_shutdown(void);

/* Global variables. */
cw_nxo_t cw_g_argv;
#ifdef CW_POSIX
cw_nxo_t cw_g_envdict;
#endif
cw_nxo_t cw_g_gcdict;

void
libonyx_init(int a_argc, char **a_argv, char **a_envp)
{
    /* Start up global modules.  Since there is no way for the caller to have
     * set up an exception handler, if an exception occurs during
     * initialization, it will result in program termination, so don't bother
     * cleaning up if an exception occurs. */
#ifdef CW_THREADS
    thd_l_init();
#endif
    xep_l_init();
    mem_l_init();
    origin_l_init();
    nxa_l_init();
    systemdict_l_init();

    /* Initialize argv. */
    {
	int i;
	int32_t len;
	cw_nxo_t str_nxo; /* GC-unsafe, but GC is disabled. */
	char *t_str;

	/* Create the argv array and populate it. */
	nxo_array_new(&cw_g_argv, true, a_argc);
	for (i = 0; i < a_argc; i++)
	{
	    len = strlen(a_argv[i]);
	    nxo_string_new(&str_nxo, true, len);
	    t_str = nxo_string_get(&str_nxo);
	    memcpy(t_str, a_argv[i], len);

	    nxo_array_el_set(&cw_g_argv, &str_nxo, i);
	}
    }

#ifdef CW_POSIX
    /* Initialize envdict. */
    {
	cw_nxo_t temp_a, temp_b; /* GC-unsafe, but GC is disabled. */

	envdict_l_populate(&cw_g_envdict, &temp_a, &temp_b, a_envp);
    }
#endif

    /* Initialize gcdict. */
    {
	cw_nxo_t temp_a, temp_b; /* GC-unsafe, but GC is disabled. */

	gcdict_l_populate(&cw_g_gcdict, &temp_a, &temp_b);
    }

    /* Turn the GC on, now that argv, envdict, and gcdict are initialized. */
    nxa_active_set(true);
}

void
libonyx_shutdown(void)
{
    /* Shut down global modules in reverse order. */
    systemdict_l_shutdown();
    nxa_l_shutdown();
    origin_l_shutdown();
    mem_l_shutdown();
    xep_l_shutdown();
#ifdef CW_THREADS
    thd_l_shutdown();
#endif
}
