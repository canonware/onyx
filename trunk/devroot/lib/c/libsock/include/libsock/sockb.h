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
 * Public interface for the sockb class.  sockb is of little interest to the
 * user, other than that it has to be manually initialized and shut down with
 * sockb_init() and sockb_shutdown().
 *
 * sockb takes care of all of the socket reading and writing details.  The sock
 * class relies on sockb extensively.  sock's and the sockb back end thread
 * communicate via three message queues.  The sockb thread makes various
 * callbacks to the sock's.
 *
 * Implementation of the sockb class.  sockb handles all of the reading/writing
 * of data from/to sockets in a separate thread.  Since there only needs to be
 * one sockb instance, a global (g_sockb) is implicitly used by the methods in
 * this class.
 *
 * The goal of sockb is to centrally handle all of the select() waiting in one
 * thread to improve performance, at the same time as making reading and writing
 * seem completely asynchronous at the sock interface level.
 *
 ****************************************************************************/

/* Opaque type. */
typedef struct cw_sockb_s cw_sockb_t;

/* Global variable. */
extern cw_sockb_t * g_sockb;

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_bufel_size : Size of bufel to use for memory buffering.
 *
 * a_max_spare_bufels : Maximum number of bufel's to cache.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : Memory allocation failure.
 *
 * <<< Description>>>
 *
 * Global initializer for g_sockb.
 *
 ****************************************************************************/
cw_bool_t
sockb_init(cw_uint32_t a_bufel_size, cw_uint32_t a_max_spare_bufels);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * None.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description>>>
 *
 * Global shutdown for g_sockb.
 *
 ****************************************************************************/
void
sockb_shutdown(void);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * None.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a cw_bufc_t instance, or NULL.
 *          NULL : Memory allocation error.
 *
 * <<< Description>>>
 *
 * Get a bufc, using internal pezz's.  bufc's are guaranteed to be of the size
 * specified by the a_bufel_size parameter passed to sockb_init().
 *
 ****************************************************************************/
cw_bufc_t *
sockb_get_spare_bufc(void);
