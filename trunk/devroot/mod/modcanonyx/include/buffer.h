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

#define	_CW_BUFFER_CHUNK_SIZE	4096

typedef struct cw_buffer_s cw_buffer_t;
typedef struct cw_chunk_s cw_chunk_t;
typedef cw_uint32_t cw_char_t;
typedef struct cw_mark_s cw_mark_t;
typedef struct cw_extent_s cw_extent_t;
typedef struct cw_mod_s cw_mod_t;

struct cw_buffer_s {
	cw_mtx_t	mtx;

	cw_nxo_t	self;
	cw_nxo_t	name;		/* Unique name, key in buffers dict. */

	cw_nxo_t	source;		/* URL (string) / base buffer (hook). */

	cw_nxo_t	modtime;	/* Timestamp as of last file read. */
	cw_nxo_t	savetime;	/* Time of last file save. */
	cw_nxo_t	autotime;	/* Time of last file autosave. */

	ql_head(cw_mark_t) marks;	/* Ordered list of marks. */
	ql_head(cw_extent_t) extents;	/* Ordered list of extents. */

	qs_head(cw_mod_t) undo;		/* stack of undo mods. */
	qs_head(cw_mod_t) redo;		/* stack of redo mods. */

	cw_uint32_t	nchars;		/* Number of characters in buffer. */
	cw_uint32_t	nchunks;	/* Number of data chunks. */
	cw_chunk_t	*chunks;	/* Array of data chunks. */
};

struct cw_chunk_s {
	cw_char_t	data[_CW_BUFFER_CHUNK_SIZE];
	cw_uint32_t	gap_off;
	cw_uint32_t	gap_len;
};

struct cw_mark_s {
	cw_nxo_t	*buffer;
	cw_uint32_t	offset;

	cw_mtx_t	mtx;
	cw_cnd_t	cnd;
};

struct cw_extent_s {
	cw_nxo_t	*buffer;
	cw_nxo_t	*mark_beg;	/* Mark at beginning of extent. */
	cw_nxo_t	*mark_end;	/* Mark at end of extent. */

	cw_mtx_t	mtx;
	cw_cnd_t	cnd;
};

struct cw_mod_s {
	cw_uint32_t	offset;
	/* XXX Actual event. */
};

void	canonyx_buffer(cw_nxo_t *a_thread);

void	canonyx_buffer_name(cw_nxo_t *a_thread);
void	canonyx_buffer_modtime(cw_nxo_t *a_thread);
void	canonyx_buffer_savetime(cw_nxo_t *a_thread);
void	canonyx_buffer_autotime(cw_nxo_t *a_thread);
void	canonyx_buffer_(cw_nxo_t *a_thread);
void	canonyx_buffer_(cw_nxo_t *a_thread);
void	canonyx_buffer_(cw_nxo_t *a_thread);
void	canonyx_buffer_(cw_nxo_t *a_thread);
void	canonyx_buffer_(cw_nxo_t *a_thread);
void	canonyx_buffer_count(cw_nxo_t *a_thread);
