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

#include "../include/modslate.h"

struct cw_display {
	cw_uint32_t	iter;	/* For GC iteration. */
	cw_nxo_t	hook;	/* Ref to =display=, prevents mod unload. */
	cw_nxo_t	term;	/* Ref to term string ('\0'-terminated). */
	cw_nxo_t	infile;	/* Ref to input file. */
	cw_nxo_t	outfile;/* Ref to input file. */
	FILE		*in;	/* Associated with infile. */
	FILE		*out;	/* Associated with outfile. */
	SCREEN		*screen;/* curses screen. */
};

static const struct cw_slate_entry slate_display_ops[] = {
	SLATE_ENTRY(display),
	SLATE_ENTRY(display_start),
	SLATE_ENTRY(display_stop),
	SLATE_ENTRY(display_redisplay)
};

void
slate_display_init(cw_nxo_t *a_thread)
{
	slate_hooks_init(a_thread, slate_display_ops,
	    (sizeof(slate_display_ops) / sizeof(struct cw_slate_entry)));
}

static cw_nxoe_t *
display_p_ref_iter(void *a_data, cw_bool_t a_reset)
{
	cw_nxoe_t		*retval;
	struct cw_display	*display = (struct cw_display *)a_data;

	if (a_reset)
		display->iter = 0;

	switch (display->iter) {
	case 0:
		retval = nxo_nxoe_get(&display->hook);
		display->iter++;
		_cw_check_ptr(retval);
		break;
	case 1:
		retval = nxo_nxoe_get(&display->term);
		display->iter++;
		if (retval != NULL)
			break;
		/* Fall through. */
	case 2:
		retval = nxo_nxoe_get(&display->infile);
		display->iter++;
		if (retval != NULL)
			break;
		/* Fall through. */
	case 3:
		retval = nxo_nxoe_get(&display->outfile);
		display->iter++;
		break;
	default:
		retval = NULL;
	}
	
	return retval;
}

static cw_bool_t
display_p_delete(void *a_data, cw_nx_t *a_nx, cw_uint32_t a_iter)
{
	cw_bool_t		retval;
	struct cw_display	*display = (struct cw_display *)a_data;

	if (a_iter != 1) {
		retval = TRUE;
		goto RETURN;
	}

	slate_slate_lock(NULL, NULL);
	delscreen(display->screen);
	slate_slate_unlock(NULL, NULL);

	if (display->in != NULL) {
#ifdef _CW_DBG
		int	error;

		error = fclose(display->in);
		if (error) {
			fprintf(stderr, "%s:%d:%s(): Error in fclose(): %s\n",
			    __FILE__, __LINE__, __FUNCTION__, strerror(error));
		}
#else
		fclose(display->in);
#endif
	}
		
	if (display->out != NULL) {
#ifdef _CW_DBG
		int	error;

		error = fclose(display->out);
		if (error) {
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

cw_nxn_t
display_type(cw_nxo_t *a_nxo)
{
	cw_nxn_t		retval;
	cw_nxo_t		*tag;
	cw_uint32_t		name_len;
	const cw_uint8_t	*name;

	if (nxo_type_get(a_nxo) != NXOT_HOOK) {
		retval = NXN_typecheck;
		goto RETURN;
	}

	tag = nxo_hook_tag_get(a_nxo);
	if (nxo_type_get(tag) != NXOT_NAME) {
		retval = NXN_typecheck;
		goto RETURN;
	}

	name_len = nxo_name_len_get(tag);
	name = nxo_name_str_get(tag);
	if ((name_len != strlen("display")) || strncmp("display", name,
	    name_len)) {
		retval = NXN_typecheck;
		goto RETURN;
	}

	retval = NXN_ZERO;
	RETURN:
	return retval;
}

/* %term %infile %outfile display %=display= */
void
slate_display(void *a_data, cw_nxo_t *a_thread)
{
	cw_nxo_t		*estack, *ostack, *tstack, *tnxo, *tag;
	cw_nxo_t		*term, *infile, *outfile;
	cw_nx_t			*nx;
	struct cw_display	*display;

	estack = nxo_thread_estack_get(a_thread);
	ostack = nxo_thread_ostack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);
	nx = nxo_thread_nx_get(a_thread);
	NXO_STACK_GET(outfile, ostack, a_thread);
	NXO_STACK_DOWN_GET(infile, ostack, a_thread, outfile);
	NXO_STACK_DOWN_GET(term, ostack, a_thread, infile);
	if (nxo_type_get(outfile) != NXOT_FILE || nxo_type_get(infile) !=
	    NXOT_FILE || nxo_type_get(term) != NXOT_STRING) {
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	}
	display = (struct cw_display *)nxa_malloc(nx_nxa_get(nx),
	    sizeof(struct cw_display));

	/* Create a reference to the display. */
	tnxo = nxo_stack_push(tstack);
	nxo_hook_new(tnxo, nx, display, NULL, display_p_ref_iter,
	    display_p_delete);

	/* Set the hook tag. */
	tag = nxo_hook_tag_get(tnxo);
	nxo_name_new(tag, nx, "display", sizeof("display") - 1, FALSE);
	nxo_attr_set(tag, NXOA_EXECUTABLE);

	/*
	 * Create a reference to this operator in order to prevent the module
	 * from being prematurely unloaded.
	 */
	nxo_no_new(&display->hook);
	nxo_dup(&display->hook, nxo_stack_get(estack));

	/* Create references to the arguments. */
	nxo_no_new(&display->term);
	nxo_string_cstring(&display->term, term, a_thread);

	nxo_no_new(&display->infile);
	nxo_dup(&display->infile, infile);

	nxo_no_new(&display->outfile);
	nxo_dup(&display->outfile, outfile);

	display->in = fdopen(nxo_file_fd_get(&display->infile), "r");
	if (display->in == NULL) {
		nxo_thread_nerror(a_thread, NXN_invalidfileaccess);
		nxo_stack_pop(tstack);
		return;
	}

	display->out = fdopen(nxo_file_fd_get(&display->outfile), "a");
	if (display->out == NULL) {
		nxo_thread_nerror(a_thread, NXN_invalidfileaccess);
		nxo_stack_pop(tstack);
		return;
	}

	display->screen = newterm(nxo_string_get(&display->term), display->out,
	    display->in);
	if (display->screen == NULL) {
		nxo_thread_nerror(a_thread, NXN_unregistered);
		nxo_stack_pop(tstack);
		return;
	}

	/* Clean up the stacks. */
	nxo_dup(term, tnxo);
	nxo_stack_npop(ostack, 2);
	nxo_stack_pop(tstack);
}

/* %=display= display_start - */
void
slate_display_start(void *a_data, cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *nxo;
	cw_nxn_t		error;
	struct cw_display	*display;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);
	error = display_type(nxo);
	if (error) {
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

/* %=display= display_stop - */
void
slate_display_stop(void *a_data, cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *nxo;
	cw_nxn_t		error;
	struct cw_display	*display;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);
	error = display_type(nxo);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}
	display = (struct cw_display *)nxo_hook_data_get(nxo);

	set_term(display->screen);
	endwin();
}

/* %=display= display_redisplay - */
void
slate_display_redisplay(void *a_data, cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *nxo;
	cw_nxn_t		error;
	struct cw_display	*display;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(nxo, ostack, a_thread);
	error = display_type(nxo);
	if (error) {
		nxo_thread_nerror(a_thread, error);
		return;
	}
	display = (struct cw_display *)nxo_hook_data_get(nxo);

	set_term(display->screen);
	refresh();
}
