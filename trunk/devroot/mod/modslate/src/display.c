/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version = slate>
 *
 ******************************************************************************/

#include "../include/modslate.h"

struct cw_display
{
    /* For GC iteration. */
    cw_uint32_t iter;

    /* Reference to =display=, prevents premature module unload. */
    cw_nxo_t hook;

    /* Auxiliary data for display_aux_[gs]et. */
    cw_nxo_t aux;

    /* Reference to terminal string ('\0'-terminated). */
    cw_nxo_t term;

    /* Reference to input file. */
    cw_nxo_t infile;

    /* Reference to output file. */
    cw_nxo_t outfile;

    /* Associated with infile. */
    FILE *in;

    /* Associated with outfile. */
    FILE *out;

    /* curses screen. */
    SCREEN *screen;
};

static const struct cw_modslate_entry modslate_display_hooks[] = {
    MODSLATE_ENTRY(display),
    {"display?", modslate_display_p},
    MODSLATE_ENTRY(display_aux_get),
    MODSLATE_ENTRY(display_aux_set),
    MODSLATE_ENTRY(display_start),
    MODSLATE_ENTRY(display_stop),
    MODSLATE_ENTRY(display_redisplay),
};

static cw_nxoe_t *
display_p_ref_iter(void *a_data, cw_bool_t a_reset);
static cw_bool_t
display_p_delete(void *a_data, cw_nx_t *a_nx, cw_uint32_t a_iter);

void
modslate_display_init(cw_nxo_t *a_thread)
{
    modslate_hooks_init(a_thread, modslate_display_hooks,
			(sizeof(modslate_display_hooks)
			 / sizeof(struct cw_modslate_entry)));
}

static cw_nxoe_t *
display_p_ref_iter(void *a_data, cw_bool_t a_reset)
{
    cw_nxoe_t *retval;
    struct cw_display *display = (struct cw_display *) a_data;

    if (a_reset)
    {
	display->iter = 0;
    }

    for (retval = NULL; retval == NULL; display->iter++)
    {
	switch (display->iter)
	{
	    case 0:
	    {
		retval = nxo_nxoe_get(&display->hook);
		break;
	    }
	    case 1:
	    {
		retval = nxo_nxoe_get(&display->aux);
		break;
	    }
	    case 2:
	    {
		retval = nxo_nxoe_get(&display->term);
		break;
	    }
	    case 3:
	    {
		retval = nxo_nxoe_get(&display->infile);
		break;
	    }
	    case 4:
	    {
		retval = nxo_nxoe_get(&display->outfile);
		break;
	    }
	    default:
	    {
		retval = NULL;
		goto RETURN;
	    }
	}
    }

    RETURN:
    return retval;
}

static cw_bool_t
display_p_delete(void *a_data, cw_nx_t *a_nx, cw_uint32_t a_iter)
{
    cw_bool_t retval;
    struct cw_display *display = (struct cw_display *) a_data;

    /* Don't delete until the third GC sweep iteration, so that associated
     * frames and windows can be deleted first. */
    if (a_iter != 2)
    {
	retval = TRUE;
	goto RETURN;
    }

    /* XXX Need locking. */
    delscreen(display->screen);

    if (display->in != NULL)
    {
#ifdef CW_DBG
	int error;

	error = fclose(display->in);
	if (error)
	{
	    fprintf(stderr, "%s:%d:%s(): Error in fclose(): %s\n",
		    __FILE__, __LINE__, __FUNCTION__, strerror(error));
	}
#else
	fclose(display->in);
#endif
    }
		
    if (display->out != NULL)
    {
#ifdef CW_DBG
	int error;

	error = fclose(display->out);
	if (error)
	{
	    fprintf(stderr, "%s:%d:%s(): Error in fclose(): %s\n",
		    __FILE__, __LINE__, __FUNCTION__, strerror(error));
	}
#else
	fclose(display->out);
#endif
    }

    nxa_free(nx_nxa_get(a_nx), display, sizeof(struct cw_display));

    retval = FALSE;
    RETURN:
    return retval;
}

/*
 * Verify that a_nxo is a =display=.
 */
cw_nxn_t
display_type(cw_nxo_t *a_nxo)
{
    cw_nxn_t retval;
    cw_nxo_t *tag;
    cw_uint32_t name_len;
    const cw_uint8_t *name;

    if (nxo_type_get(a_nxo) != NXOT_HOOK)
    {
	retval = NXN_typecheck;
	goto RETURN;
    }

    tag = nxo_hook_tag_get(a_nxo);
    if (nxo_type_get(tag) != NXOT_NAME)
    {
	retval = NXN_typecheck;
	goto RETURN;
    }

    name_len = nxo_name_len_get(tag);
    name = nxo_name_str_get(tag);
    if ((name_len != strlen("display")) || strncmp("display", name, name_len))
    {
	retval = NXN_typecheck;
	goto RETURN;
    }

    retval = NXN_ZERO;
    RETURN:
    return retval;
}

/* #term #infile #outfile display #=display= */
void
modslate_display(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *estack, *ostack, *tstack, *tnxo, *tag;
    cw_nxo_t *term, *infile, *outfile;
    cw_nx_t *nx;
    struct cw_display *display;

    estack = nxo_thread_estack_get(a_thread);
    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    nx = nxo_thread_nx_get(a_thread);
    NXO_STACK_GET(outfile, ostack, a_thread);
    NXO_STACK_DOWN_GET(infile, ostack, a_thread, outfile);
    NXO_STACK_DOWN_GET(term, ostack, a_thread, infile);
    if (nxo_type_get(outfile) != NXOT_FILE
	|| nxo_type_get(infile) != NXOT_FILE
	|| nxo_type_get(term) != NXOT_STRING)
    {
	nxo_thread_nerror(a_thread, NXN_typecheck);
	return;
    }

    display = (struct cw_display *)nxa_malloc(nx_nxa_get(nx),
					      sizeof(struct cw_display));

    /* Create a reference to this hook in order to prevent the module from being
     * prematurely unloaded. */
    nxo_no_new(&display->hook);
    nxo_dup(&display->hook, nxo_stack_get(estack));

    /* Create a reference to the display. */
    tnxo = nxo_stack_push(tstack);
    nxo_hook_new(tnxo, nx, display, NULL, display_p_ref_iter, display_p_delete);

    /* Set the hook tag. */
    tag = nxo_hook_tag_get(tnxo);
    nxo_name_new(tag, nx, "display", sizeof("display") - 1, FALSE);
    nxo_attr_set(tag, NXOA_EXECUTABLE);

    /* Initialize aux. */
    nxo_null_new(&display->aux);

    /* Create references to the arguments. */
    nxo_no_new(&display->term);
    nxo_string_cstring(&display->term, term, a_thread);

    nxo_no_new(&display->infile);
    nxo_dup(&display->infile, infile);

    nxo_no_new(&display->outfile);
    nxo_dup(&display->outfile, outfile);

    display->in = fdopen(nxo_file_fd_get(&display->infile), "r");
    if (display->in == NULL)
    {
	nxo_thread_nerror(a_thread, NXN_invalidfileaccess);
	nxo_stack_pop(tstack);
	return;
    }

    display->out = fdopen(nxo_file_fd_get(&display->outfile), "a");
    if (display->out == NULL)
    {
	nxo_thread_nerror(a_thread, NXN_invalidfileaccess);
	nxo_stack_pop(tstack);
	return;
    }

    display->screen = newterm(nxo_string_get(&display->term), display->out,
			      display->in);
    if (display->screen == NULL)
    {
	nxo_thread_nerror(a_thread, NXN_unregistered);
	nxo_stack_pop(tstack);
	return;
    }

    /* Clean up the stacks. */
    nxo_stack_npop(ostack, 2);
    nxo_stack_pop(tstack);
}

/* #object display? #boolean */
void
modslate_display_p(void *a_data, cw_nxo_t *a_thread)
{
    modslate_hook_p(a_data, a_thread, "frame");
}

void
modslate_display_aux_get(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *tstack, *nxo, *tnxo;
    cw_nxn_t error;
    struct cw_display *display;

    ostack = nxo_thread_ostack_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = modslate_hook_type(nxo, "display");
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    display = (struct cw_display *) nxo_hook_data_get(nxo);

    /* Avoid a GC race by using tnxo to store a reachable ref to the display. */
    tnxo = nxo_stack_push(tstack);
    nxo_dup(tnxo, nxo);
    nxo_dup(nxo, &display->aux);
    nxo_stack_pop(tstack);
}

void
modslate_display_aux_set(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo, *aux;
    cw_nxn_t error;
    struct cw_display *display;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(aux, ostack, a_thread);
    NXO_STACK_DOWN_GET(nxo, ostack, a_thread, aux);
    error = modslate_hook_type(nxo, "display");
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    display = (struct cw_display *) nxo_hook_data_get(nxo);

    nxo_dup(&display->aux, aux);
    nxo_stack_npop(ostack, 2);
}

/* #=display= display_start - */
void
modslate_display_start(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_display *display;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = modslate_hook_type(nxo, "display");
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    display = (struct cw_display *)nxo_hook_data_get(nxo);

    set_term(display->screen);
    cbreak();
    noecho();
    nonl();
    intrflush(stdscr, FALSE);
    keypad(stdscr, TRUE);
}

/* #=display= display_stop - */
void
modslate_display_stop(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_display *display;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = modslate_hook_type(nxo, "display");
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    display = (struct cw_display *)nxo_hook_data_get(nxo);

    set_term(display->screen);
    endwin();
}

/* #=display= display_redisplay - */
void
modslate_display_redisplay(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_display *display;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = modslate_hook_type(nxo, "display");
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    display = (struct cw_display *)nxo_hook_data_get(nxo);

    set_term(display->screen);
    refresh();
}
