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

#include "modnxe_defs.h"

#include <libonyx/libonyx.h>

#include "msgq.h"
#include "buf.h"

#include "buffer.h"

#define	ENTRY(name)	{#name, nxe_##name}

struct cw_nxe_entry {
	const cw_uint8_t	*op_n;
	cw_op_t			*op_f;
};

void	nxe_ops_init(cw_nxo_t *a_thread, const struct cw_nxe_entry *a_entries,
    cw_uint32_t a_nentries);
