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
 * a_max_spare_bufcs : Maximum number of bufc's to cache in one pezz block.
 *                     If more than this number of bufc's is needed, the sockb
 *                     code will attempt to allocate another block of bufc's.
 *                     Note that once allocated, the space is not freed until
 *                     sockb_shutdown() is called.
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

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_mq : Pointer to a mq, or NULL.
 *
 * a_sockfd : A socket desccriptor number.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : Memory allocation error.
 *
 * <<< Description >>>
 *
 * Register such that whenever data is available for reading on a_sockfd, or
 * a_sockfd is closed by the sockb thread, a message is sent to a_mq.  The
 * message is simply a socket descriptor number embedded in the mq's data
 * pointer (ew gross), in order to avoid memory allocation complexity.  To stop
 * receiving notifications, pass a NULL mq pointer.
 *
 * Note: Only one mq at a time can be notified of events on a particular
 * sockfd.
 *
 ****************************************************************************/
cw_bool_t
sockb_in_notify(cw_mq_t * a_mq, int a_sockfd);
