/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: Slate <Version = slate>
 *
 ******************************************************************************/

#include <libonyx/libonyx.h>
#include "modslate_defs.h"

#include "rb.h"
#include "buf.h"
#include "hist.h"

#include "buffer.h"
#include "frame.h"
#include "window.h"

/* Iteration counts for destruction of various object types.  Dependencies:
 *
 * buffer  : marker extent
 * module  : buffer
 */
#define MODSLATE_GC_ITER_BUFFER	1
#define MODSLATE_GC_ITER_MODULE	2

#define MODSLATE_METHOD(clas, name) {#name, modslate_##clas##_##name}

// XXX
#define MODSLATE_ENTRY(name) {#name, modslate_##name}

struct cw_modslate_method
{
    const cw_uint8_t *name;
    cw_nxo_handle_eval_t *eval_f;
};

// XXX
struct cw_modslate_entry
{
    const cw_uint8_t *name;
    cw_nxo_handle_eval_t *eval_f;
};

void
modslate_class_init(cw_nxo_t *a_thread, const cw_uint8_t *a_name,
		    const struct cw_modslate_method *a_methods,
		    cw_uint32_t a_nmethods, void *a_opaque, cw_nxo_t *r_class);

// XXX
void
modslate_handles_init(cw_nxo_t *a_thread,
		      const struct cw_modslate_entry *a_entries,
		      cw_uint32_t a_nentries);

cw_nxn_t
modslate_instance_kind(cw_nxo_t *a_instance, cw_nxo_t *a_class);

// XXX
cw_nxn_t
modslate_handle_type(cw_nxo_t *a_handle, const cw_uint8_t *a_type);

void
modslate_handle_p(void *a_data, cw_nxo_t *a_thread, const cw_uint8_t *a_type);

void
modslate_init(void *a_arg, cw_nxo_t *a_thread);
