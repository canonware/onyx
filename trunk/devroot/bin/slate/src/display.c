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

#include <curses.h>

struct cw_display {
	SCREEN *screen;
};

static const struct cw_slate_entry slate_display_ops[] = {
	SLATE_ENTRY(display),
	SLATE_ENTRY(display_suspend),
	SLATE_ENTRY(display_resume),
	SLATE_ENTRY(display_redisplay)
};

void
slate_display_init(cw_nxo_t *a_thread)
{
	slate_hooks_init(a_thread, slate_display_ops,
	    (sizeof(slate_display_ops) / sizeof(struct cw_slate_entry)));
}

static void
buffer_p_eval(void *a_data, cw_nxo_t *a_thread)
{

}

static cw_nxoe_t *
buffer_p_ref_iter(void *a_data, cw_bool_t a_reset)
{

}

static cw_bool_t
buffer_p_delete(void *a_data, cw_nx_t *a_nx, cw_uint32_t a_iter)
{

}
static cw_nxn_t buffer_p_type(cw_nxo_t *a_nxo);


void
slate_display(void *a_data, cw_nxo_t *a_thread)
{
	initscr();
	cbreak();
	noecho();
	nonl();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
}

void
slate_display_suspend(void *a_data, cw_nxo_t *a_thread)
{

}

void
slate_display_resume(void *a_data, cw_nxo_t *a_thread)
{

}

void
slate_display_redisplay(void *a_data, cw_nxo_t *a_thread)
{

}
