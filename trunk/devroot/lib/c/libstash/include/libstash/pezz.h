/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * <<< Description >>>
 *
 * Public interface for the pezz class.  pezz provides cached allocation for
 * many equal-size buffers.  It does incremental block allocation, then carves
 * buffers from those blocks.  No memory is freed until pezz_delete() is called.
 *
 ****************************************************************************/

/* Pseudo-opaque type. */
typedef struct cw_pezz_s cw_pezz_t;

struct cw_pezz_s
{
  cw_bool_t is_malloced;
#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
  cw_uint32_t magic;
#endif
#ifdef _CW_REENTRANT
  cw_mtx_t lock;
#endif

  /* Size of one buffer, from the user's perspective. */
  cw_uint32_t buffer_size;

  /* Number of buffers in one block.  One block is
   * (buffer_size * block_num_buffers) bytes. */
  cw_uint32_t block_num_buffers;
  
  /* Pointer to an array of base addresses for the memory blocks from which
   * memory is allocated. */
  void ** mem_blocks;

  /* Pointer to an array of base addresses for the memory blocks that are used
   * for ring structures. */
  cw_ring_t ** ring_blocks;

  /* Number of blocks allocated (number of elements in the mem_blocks[] and
   * ring_blocks[] arrays. */
  cw_uint32_t num_blocks;
  
  /* Ring seam for spare (unallocated) buffers. */
  cw_ring_t * spare_buffers;

  /* Ring seam for spare rings.  This ring has no associated data, and is merely
   * a cache of ring structures to be used for insertion into the spare_buffers
   * ring. */
  cw_ring_t * spare_rings;
};

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_pezz : Pointer to space for a pezz, or NULL.
 *
 * a_buffer_size : Size of buffers to allocate and return from pezz_get().
 *
 * a_num_buffers : Number of buffers to allocate space for each time a new
 *                 memory block is allocated.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a pezz, or NULL.
 *          NULL : Memory allocation error.
 *
 * <<< Description >>>
 *
 * Constructor.
 *
 ****************************************************************************/
cw_pezz_t *
pezz_new(cw_pezz_t * a_pezz, cw_uint32_t a_buffer_size,
	 cw_uint32_t a_num_buffers);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_pezz : Pointer to a pezz.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Destructor.
 *
 ****************************************************************************/
void
pezz_delete(cw_pezz_t * a_pezz);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_pezz : Pointer to a pezz.
 *
 * <<< Output(s) >>>
 *
 * retval : Size of buffers that a_pezz is using.
 *
 * <<< Description >>>
 *
 * Return the size of the buffers that a_pezz is using.
 *
 ****************************************************************************/
cw_uint32_t
pezz_get_buffer_size(cw_pezz_t * a_pezz);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_pezz : Pointer to a pezz.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a memory buffer.
 *
 * <<< Description >>>
 *
 * Return a memory buffer.
 *
 ****************************************************************************/
void *
pezz_get(cw_pezz_t * a_pezz);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_pezz : Pointer to a pezz.
 *
 * a_buffer : Pointer to a memory buffer.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Put back (deallocate) a_buffer.
 *
 ****************************************************************************/
void
pezz_put(void * a_pezz, void * a_buffer);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_pezz : Pointer to a pezz.
 *
 * a_prefix : Pointer to a string which is used as a prefix for all output.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Dump the internal state of a_pezz to cw_g_out.
 *
 ****************************************************************************/
void
pezz_dump(cw_pezz_t * a_pezz, const char * a_prefix);
