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

#include <libonyx/libonyx.h>
#include "modpane_defs.h"

#include <termios.h>

#include "cl.h"
#include "tm.h"
#include "ds.h"
#include "pn.h"

#include "display.h"
#include "pane.h"

/* Iteration counts for destruction of various object types. */
#define MODPANE_GC_ITER_MODULE	1

#define MODPANE_METHOD(clas, name) {#name, modpane_##clas##_##name}

struct cw_modpane_method
{
    const cw_uint8_t *name;
    cw_nxo_handle_eval_t *eval_f;
};

void
modpane_class_init(cw_nxo_t *a_thread, const cw_uint8_t *a_name,
		   const struct cw_modpane_method *a_methods,
		   cw_uint32_t a_nmethods, void *a_opaque, cw_nxo_t *r_class);

cw_nxn_t
modpane_instance_kind(cw_nxo_t *a_instance, cw_nxo_t *a_class);

void
modpane_init(void *a_arg, cw_nxo_t *a_thread);
