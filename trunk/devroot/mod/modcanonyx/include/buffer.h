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

typedef struct cw_buffer_s cw_buffer_t;

struct cw_buffer_s {
	cw_nxo_t	self;
	cw_nxo_t	*filename;
};

void	canonyx_buffer(cw_nxo_t *a_thread);
void	canonyx_buffer_file_open(cw_nxo_t *a_thread);
