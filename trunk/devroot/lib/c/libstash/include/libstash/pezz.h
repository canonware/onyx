/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * <<< Description >>>
 *
 * Public interface for the pezz class.  pezz is similar to the bufpool class,
 * but differs in that it does all buffer allocation during initialization, and
 * when there are no available buffers, allocation is handled using malloc(),
 * but these malloc()ed buffers cannot be cached.
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

  /* Pointer to base of the memory block where all the buffers and ring
   * structures are. */
  void * mem_base;
  /* Pointer one byte past the end of the memory block pointed to by
   * mem_base. */
  void * mem_end;

  /* Pointer to base of the rings memory block. */
  cw_ring_t * rings;
  
  /* Size of one buffer, from the user's perspective. */
  cw_uint32_t buffer_size;

  /* Max number of buffers available from this pezz. */
  cw_uint32_t num_buffers;

  /* Ring seam for spare buffers. */
  cw_ring_t * spare_buffers;

#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
  /* Counters to keep track of how much overflow this pezz is seeing. */
  cw_uint32_t num_overflow;
  cw_uint32_t max_overflow;
#endif
};

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_pezz : Pointer to space for a pezz, or NULL.
 *
 * a_buffer_size : Size of buffers to allocate and return from pezz_get().
 *
 * a_num_buffers : Number of buffers to pre-allocate.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a pezz.
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
 * Dump the internal state of a_pezz to cw_g_log.
 *
 ****************************************************************************/
void
pezz_dump(cw_pezz_t * a_pezz, const char * a_prefix);
