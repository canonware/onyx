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
 ******************************************************************************/

#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <errno.h>

#include "../include/onyx.h"

/* Prototypes for generated code. */
void
batch_nxcode(cw_nxo_t *a_thread);
void
interactive_nxcode(cw_nxo_t *a_thread);

#define CW_PROMPT_STRLEN 80
#define CW_BUFFER_SIZE 512

typedef struct
{
    cw_bool_t is_expr;
    cw_uint8_t *str; /* If is_expr is FALSE: filename; expression otherwise. */
} cw_nxinit_t;

#if (!defined(CW_POSIX_FILE) || !defined(CW_USE_MODPROMPT))
struct nx_read_arg_s
{
    int fd;
    cw_nxo_t *thread;
    cw_bool_t interactive;
    cw_uint8_t *buffer;
    cw_uint32_t buffer_size;
    cw_uint32_t buffer_count;
};

struct nx_write_arg_s
{
    int fd;
};
#endif

/* Globals.  These are global due to the libedit API not providing a way to pass
 * them to the prompt function. */
cw_nxo_t thread;
cw_nxo_threadp_t threadp;
#ifdef CW_USE_LIBEDIT
EditLine *el;
History *hist;
cw_uint8_t prompt_str[CW_PROMPT_STRLEN];
#endif

/* Function prototypes. */
void
usage(void);

int
interactive_run(int argc, char **argv, char **envp, cw_nxinit_t *a_init,
		cw_uint32_t a_ninit);

int
batch_run(int argc, char **argv, char **envp, cw_bool_t a_version,
	  cw_uint8_t *a_expression);

cw_bool_t
file_setup(cw_nx_t *a_nx, cw_nxo_t *a_thread, const char *a_filename);

#if (!defined(CW_POSIX_FILE) || !defined(CW_USE_MODPROMPT))
void
stdin_init(cw_nx_t *a_nx, cw_bool_t a_interactive);

cw_sint32_t
nx_read(void *a_arg, cw_nxo_t *a_file, cw_uint32_t a_len, cw_uint8_t *r_str);

void
nx_read_shutdown(void *a_arg, cw_nx_t *a_nx);
#endif

#ifndef CW_POSIX_FILE
void
stdout_init(cw_nx_t *a_nx);

void
stderr_init(cw_nx_t *a_nx);

cw_bool_t
nx_write(void *a_arg, cw_nxo_t *a_file, const cw_uint8_t *a_str,
	 cw_uint32_t a_len);
#endif

int
main(int argc, char **argv, char **envp)
{
    int retval, c;
    cw_bool_t opt_version = FALSE;
    cw_uint8_t *opt_expression = NULL;
    cw_uint32_t opt_ninit = 0;
    cw_nxinit_t *opt_init = NULL;
//    cw_uint8_t **opt_init = NULL;

    libonyx_init();

    /* Parse command line arguments, but postpone taking any actions that
     * require the onyx interpreter to be up and running.  This is necessary
     * because we need to know how much of argv to pass to nx_new(). */
    while ((c = getopt(argc, argv,
#ifdef _GNU_SOURCE
		       /* Without this, glibc will permute unknown options to
			* the end of the argument list. */
		       "+"
#endif
		       "hVe:i:s:")) != -1)
    {
	switch (c)
	{
	    case 'h':
	    {
		usage();
		retval = 0;
		goto CLERROR;
	    }
	    case 'V':
	    {
		opt_version = TRUE;
		break;
	    }
	    case 'e':
	    {
		if (argc != 3)
		{
		    fprintf(stderr, "onyx: Incorrect number of arguments\n");
		    usage();
		    retval = 1;
		    goto CLERROR;
		}

		opt_expression = optarg;
		break;
	    }
#ifdef CW_POSIX_FILE
	    case 'i':
	    {
		opt_ninit++;
		if (opt_ninit == 1)
		{
		    opt_init = (cw_nxinit_t *)cw_malloc(sizeof(cw_nxinit_t));
		}
		else
		{
		    opt_init = (cw_nxinit_t *)cw_realloc(opt_init,
							 sizeof(cw_nxinit_t)
							 * opt_ninit);
		}
		opt_init[opt_ninit - 1].is_expr = TRUE;
		opt_init[opt_ninit - 1].str = optarg;
		break;
	    }
#endif
	    case 's':
	    {
		opt_ninit++;
		if (opt_ninit == 1)
		{
		    opt_init = (cw_nxinit_t *)cw_malloc(sizeof(cw_nxinit_t));
		}
		else
		{
		    opt_init = (cw_nxinit_t *)cw_realloc(opt_init,
							 sizeof(cw_nxinit_t)
							 * opt_ninit);
		}
		opt_init[opt_ninit - 1].is_expr = FALSE;
		opt_init[opt_ninit - 1].str = optarg;
		break;
	    }
	    default:
	    {
		fprintf(stderr,  "onyx: Unrecognized option\n");
		usage();
		retval = 1;
		goto CLERROR;
	    }
	}
    }

    /* Do additional command line argument error checking. */
    if ((optind < argc && (opt_expression != NULL || opt_version
			   || opt_ninit != 0))
	|| (opt_version && opt_expression != NULL)
	|| (opt_ninit != 0 && (opt_version || opt_expression != NULL))
	)
    {
	fprintf(stderr, "onyx: Incorrect number of arguments\n");
	usage();
	retval = 1;
	goto CLERROR;
    }

    /* Run differently, depending on whether this is an interactive session, and
     * what flags were specified. */
    if (isatty(0) && optind == argc
	&& opt_version == FALSE && opt_expression == NULL)
    {
	retval = interactive_run(argc, argv, envp, opt_init, opt_ninit);
    }
    else
    {
	retval = batch_run(argc, argv, envp, opt_version, opt_expression);
    }

    CLERROR:
    if (opt_init != NULL)
    {
	cw_free(opt_init);
    }
    libonyx_shutdown();
    return retval;
}

void
usage(void)
{
    printf("onyx usage:\n"
	   "    onyx -h\n"
	   "    onyx -V\n"
	   "    onyx [-e <expr>]\n"
	   "    onyx [-i <expr>]* [-s <file>]*\n"
	   "    onyx <file> [<args>]\n"
	   "\n"
	   "    Option    | Description\n"
	   "    ----------+--------------------------------------------\n"
	   "    -h        | Print usage and exit.\n"
	   "    -V        | Print version information and exit.\n"
	   "    -e <expr> | Evaluate <expr>.\n"
	   "    -i <expr> | Evaluate initialization <expr>.\n"
#ifdef CW_POSIX_FILE
	   "    -s <file> | Evaluate contents of initialization <file>.\n"
#endif
	   );
}

int
interactive_run(int argc, char **argv, char **envp, cw_nxinit_t *a_init,
		cw_uint32_t a_ninit)
{
    int retval;
    cw_nx_t nx;
    cw_nxo_t *nxo;
#ifdef CW_POSIX_FILE
    cw_uint32_t i;
    char *init;
#endif

    /* Initialize the interpreter. */
    nx_new(&nx, NULL, argc, argv, envp);

    /* Make sure that stdin is always synthetic, so that the prompt will be
     * printed. */
#ifndef CW_USE_MODPROMPT
    stdin_init(&nx, TRUE);
#endif
#ifndef CW_POSIX_FILE
    stdout_init(&nx);
    stderr_init(&nx);
#endif

    /* Now that the files have been wrapped, create the initial thread. */
    nxo_thread_new(&thread, &nx);

    /* Run embedded initialization code specific to interactive execution. */
    interactive_nxcode(&thread);

#ifdef CW_POSIX_FILE
    /* Run RC ("run commands") file specified by ONYXRC environment variable,
     * if any. */
    init = getenv("ONYXRC");
    if (init != NULL)
    {
	if (file_setup(&nx, &thread, init))
	{
	    retval = 1;
	    goto RETURN;
	}
	nxo_thread_start(&thread);
    }

    /* Run initialization scripts, if any. */
    for (i = 0; i < a_ninit; i++)
    {
	if (a_init[i].is_expr)
	{
	    /* Set up string for evaluation. */
	    nxo = nxo_stack_push(nxo_thread_ostack_get(&thread));
	    nxo_string_new(nxo, &nx, FALSE, strlen((char *) a_init[i].str));
	    nxo_attr_set(nxo, NXOA_EXECUTABLE);
	    nxo_string_set(nxo, 0, (cw_uint8_t *) a_init[i].str,
			   nxo_string_len_get(nxo));
	}
	else
	{
	    /* Set up file for evaluation. */
	    if (file_setup(&nx, &thread, (char *) a_init[i].str))
	    {
		retval = 1;
		goto RETURN;
	    }
	}
	nxo_thread_start(&thread);
    }
#endif

    /* Push an executable stdin on ostack.  This must be done after loading
     * modprompt, as well as after any initialization scripts that are specified
     * on the command line. */
    nxo = nxo_stack_push(nxo_thread_ostack_get(&thread));
    nxo_dup(nxo, nxo_thread_stdin_get(&thread));
    nxo_attr_set(nxo, NXOA_EXECUTABLE);

    /* Run the interpreter such that it will not exit on errors. */
    nxo_thread_start(&thread);

    retval = 0;
    RETURN:
    nx_delete(&nx);
    return retval;
}

int
batch_run(int argc, char **argv, char **envp, cw_bool_t a_version,
	  cw_uint8_t *a_expression)
{
    int retval;
    cw_nxo_t *file;
    cw_nx_t nx;

    /* Since this is a non-interactive invocation, don't include all elements of
     * argv. */
    nx_new(&nx, NULL, argc - optind, &argv[optind], envp);
#ifndef CW_POSIX_FILE
    stdin_init(&nx, FALSE);
    stdout_init(&nx);
    stderr_init(&nx);
#endif
    nxo_thread_new(&thread, &nx);
    nxo_threadp_new(&threadp);

    /* Run embedded initialization code specific to non-interactive
     * execution. */
    batch_nxcode(&thread);

    /* Act on the command line arguments, now that the interpreter is
     * initialized. */
    if (a_version)
    {
	static const cw_uint8_t version[]
	    = "product print `, version ' print version print"
	    " `.\n' print flush";

	/* Print the version and exit. */
	nxo_thread_interpret(&thread, &threadp, version, sizeof(version) - 1);
	nxo_thread_flush(&thread, &threadp);
	retval = 0;
	goto RETURN;
    }
    else if (a_expression != NULL)
    {
	cw_nxo_t *string;

	/* Push the string onto the execution stack. */
	string = nxo_stack_push(nxo_thread_ostack_get(&thread));
	nxo_string_new(string, &nx, FALSE, strlen((char *) a_expression));
	nxo_attr_set(string, NXOA_EXECUTABLE);
	nxo_string_set(string, 0, a_expression, nxo_string_len_get(string));
    }
    else if (optind < argc)
    {
	if (file_setup(&nx, &thread, argv[optind]))
	{
	    retval = 1;
	    goto RETURN;
	}
    }
    else
    {
	/* No source file specified, and there there was no -e expression
	 * specified either, so treat stdin as the source. */
	file = nxo_stack_push(nxo_thread_ostack_get(&thread));
	nxo_dup(file, nx_stdin_get(&nx));
	nxo_attr_set(file, NXOA_EXECUTABLE);
    }

    /* Run the interpreter non-interactively. */
    nxo_thread_start(&thread);

    retval = 0;
    RETURN:
    nxo_threadp_delete(&threadp, &thread);
    nx_delete(&nx);
    return retval;
}

#ifdef CW_POSIX_FILE
cw_bool_t
file_setup(cw_nx_t *a_nx, cw_nxo_t *a_thread, const char *a_filename)
{
    cw_bool_t retval;
    int src_fd;
    cw_nxo_t *file;

    /* Remaining command line arguments should be the name of a source file,
     * followed by optional arguments.  Open the source file, wrap it in an
     * onyx file object, and push it onto the execution stack. */
    src_fd = open(a_filename, O_RDONLY);
    if (src_fd == -1)
    {
	fprintf(stderr,
		"onyx: Error in open(\"%s\", O_RDONLY): %s\n", a_filename,
		strerror(errno));
	retval = TRUE;
	goto RETURN;
    }
    file = nxo_stack_push(nxo_thread_ostack_get(a_thread));
    nxo_file_new(file, a_nx, FALSE);
    nxo_attr_set(file, NXOA_EXECUTABLE);

    nxo_file_fd_wrap(file, src_fd);

    retval = FALSE;
    RETURN:
    return retval;
}
#else
cw_bool_t
file_setup(cw_nx_t *a_nx, cw_nxo_t *a_thread, const char *a_filename)
{
    cw_bool_t retval;
    int src_fd;
    cw_nxo_t *file;
    static struct nx_read_arg_s src_arg;

    /* Remaining command line arguments should be the name of a source file,
     * followed by optional arguments.  Open the source file, wrap it in an
     * onyx file object, and push it onto the execution stack. */
    src_fd = open(a_filename, O_RDONLY);
    if (src_fd == -1)
    {
	fprintf(stderr,
		"onyx: Error in open(\"%s\", O_RDONLY): %s\n", a_filename,
		strerror(errno));
	retval = TRUE;
	goto RETURN;
    }
    file = nxo_stack_push(nxo_thread_ostack_get(a_thread));
    nxo_file_new(file, a_nx, FALSE);
    nxo_attr_set(file, NXOA_EXECUTABLE);

    /* Initialize the src argument structure. */
    src_arg.fd = src_fd;
    src_arg.thread = a_thread;
    src_arg.buffer = NULL;
    src_arg.buffer_size = 0;
    src_arg.buffer_count = 0;
	
    nxo_file_synthetic(file, nx_read, NULL, NULL, nx_read_shutdown,
		       (void *) &src_arg);

    retval = FALSE;
    RETURN:
    return retval;
}
#endif

#if (!defined(CW_POSIX_FILE) || !defined(CW_USE_MODPROMPT))
void
stdin_init(cw_nx_t *a_nx, cw_bool_t a_interactive)
{
    cw_nxo_t *nxo;
    static struct nx_read_arg_s stdin_arg;

    /* Add a synthetic wrapper for stdin, since normal files can't be
     * used. */
    stdin_arg.fd = 0;
    stdin_arg.thread = &thread;
    stdin_arg.interactive = a_interactive;
    stdin_arg.buffer = NULL;
    stdin_arg.buffer_size = 0;
    stdin_arg.buffer_count = 0;

    nxo = nx_stdin_get(a_nx);
    nxo_file_new(nxo, a_nx, TRUE);
    nxo_file_synthetic(nxo, nx_read, NULL, NULL, nx_read_shutdown,
		       (void *) &stdin_arg);
}

cw_sint32_t
nx_read(void *a_arg, cw_nxo_t *a_file, cw_uint32_t a_len, cw_uint8_t *r_str)
{
    cw_sint32_t retval;
    struct nx_read_arg_s *arg = (struct nx_read_arg_s *) a_arg;

    cw_assert(a_len > 0);

    if (arg->buffer_count == 0)
    {
	/* Print the prompt if interactive and not in deferred execution mode.
	 *
	 * This code assumes that only the initial thread reads from stdin.  Bad
	 * things (likely crashes) will happen if that assumption is broken.
	 * There isn' really a simple way to solve this, and it probably
	 * isn't worthwhile, considering that it's a degenerate case of a
	 * non-standard configuration. */
	if ((arg->interactive)
	    && (nxo_thread_deferred(arg->thread) == FALSE)
	    && (nxo_thread_state(arg->thread) == THREADTS_START))
	{
	    cw_onyx_code(arg->thread, "promptstring print flush");
	}

	/* Read data until there are no more. */
	while ((retval = read(arg->fd, r_str, a_len)) == -1 && errno == EINTR)
	{
	    /* Interrupted system call, probably due to garbage collection. */
	}
	if (retval == -1)
	{
	    /* EOF. */
	    retval = 0;
	}
	else
	{
	    if (retval == a_len)
	    {
		cw_sint32_t count;

		/* There may be more data available.  Store any available data
		 * in a buffer. */
		if (arg->buffer == NULL)
		{
		    /* Initialize the buffer. */
		    arg->buffer = (cw_uint8_t *) cw_malloc(CW_BUFFER_SIZE);
		    arg->buffer_size = CW_BUFFER_SIZE;
		    arg->buffer_count = 0;
		}

		for (;;)
		{
		    while ((count
			    = read(arg->fd, &arg->buffer[arg->buffer_count],
				   arg->buffer_size - arg->buffer_count))
			   == -1
			   && errno == EINTR)
		    {
			/* Interrupted system call, probably due to garbage
			 * collection. */
		    }
		    if (count <= 0)
		    {
			/* EOF or no more data buffered by stdin. */
			break;
		    }

		    arg->buffer_count += count;
		    if (arg->buffer_count == arg->buffer_size)
		    {
			/* Expand the buffer. */
			arg->buffer
			    = (cw_uint8_t *) cw_realloc(arg->buffer,
							arg->buffer_size * 2);
			arg->buffer_size *= 2;
		    }
		}
	    }
	}
    }
    else
    {
	/* Return as much of the buffered data as possible. */
	if (arg->buffer_count > a_len)
	{
	    /* There are more data than we can return. */
	    retval = a_len;
	    memcpy(r_str, arg->buffer, a_len);
	    arg->buffer_count -= a_len;
	    memmove(arg->buffer, &arg->buffer[a_len], arg->buffer_count);
	}
	else
	{
	    /* Return all the data. */
	    retval = arg->buffer_count;
	    memcpy(r_str, arg->buffer, arg->buffer_count);
	    arg->buffer_count = 0;
	}
    }

    return retval;
}

void
nx_read_shutdown(void *a_arg, cw_nx_t *a_nx)
{
    struct nx_read_arg_s *arg = (struct nx_read_arg_s *) a_arg;

    if (arg->buffer != NULL)
    {
	cw_free(arg->buffer);
    }
}
#endif

#ifndef CW_POSIX_FILE
void
stdout_init(cw_nx_t *a_nx)
{
    cw_nxo_t *nxo;
    static struct nx_write_arg_s stdout_arg;

    /* Initialize the stdout argument structure. */
    stdout_arg.fd = 1;

    nxo = nx_stdout_get(a_nx);
    nxo_file_new(nxo, a_nx, TRUE);
    nxo_file_synthetic(nxo, NULL, nx_write, NULL, NULL, (void *) &stdout_arg);
}

void
stderr_init(cw_nx_t *a_nx)
{
    cw_nxo_t *nxo;
    static struct nx_write_arg_s stderr_arg;

    /* Initialize the stderr argument structure. */
    stderr_arg.fd = 2;

    nxo = nx_stderr_get(a_nx);
    nxo_file_new(nxo, a_nx, TRUE);
    nxo_file_synthetic(nxo, NULL, nx_write, NULL, NULL, (void *) &stderr_arg);
}

cw_bool_t
nx_write(void *a_arg, cw_nxo_t *a_file, const cw_uint8_t *a_str,
	 cw_uint32_t a_len)
{
    cw_bool_t retval;
    struct nx_write_arg_s *arg = (struct nx_write_arg_s *) a_arg;
    ssize_t nwritten, cwritten;

    for (nwritten = 0;
	 nwritten < a_len;
	 )
    {
	cwritten = write(arg->fd, &a_str[nwritten], a_len - nwritten);
	if (cwritten == -1)
	{
	    if (errno != EINTR)
	    {
		retval = TRUE;
		goto RETURN;
	    }
	}
	else
	{
	    nwritten += cwritten;
	}
    }

    retval = FALSE;
    RETURN:
    return retval;
}
#endif
