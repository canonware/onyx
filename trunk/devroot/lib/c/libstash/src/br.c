/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 185 $
 * $Date: 1998-08-30 22:42:04 -0700 (Sun, 30 Aug 1998) $
 *
 * <<< Description >>>
 *
 *  Block repository (br) implementation.
 *
 * << Summary of acronyms >>
 *
 * vaddr     : virtual address (block repository address space)
 * paddr     : physical address (block repository address space)
 * BS        : block size
 * V2P       : vaddr to paddr map
 * P2V       : paddr to vaddr map
 * TB        : temporary buffer space
 * DS        : data space
 * V2P_BASE  : base address of V2P
 * P2V_BASE  : base address of P2V
 * TB_BASE   : base address of TB
 * DS_BASE   : base address of DS
 * 
 * << Logical br layout >>
 *
 *  Section name                        paddr
 *  ------------                        -----
 *
 * /----------------------------------\ V2P_BASE == 0x0000000000000000
 * |                                  |
 * | vaddr --> paddr map              |
 * | and vaddr --> paddr map snapshot |
 * | (V2P)                            |
 * |                                  |
 * |----------------------------------| P2V_BASE == V2P_BASE + (2^67 / BS)
 * |                                  |
 * | paddr --> vaddr map              |
 * | (P2V)                            |
 * |                                  |
 * |----------------------------------| TB_BASE == P2V_BASE + (2^66 / BS)
 * |                                  |
 * | temporary buffers                |
 * | (TB)                             |
 * |                                  |
 * :                                  :
 * :                                  :
 * :                                  :
 * :                                  :
 * |                                  |
 * |----------------------------------| 0x8000000000000000
 * |                                  |
 * | data space                       |
 * | (DS)                             |
 * |                                  |
 * :                                  :
 * :                                  :
 * :                                  :
 * :                                  :
 * :                                  :
 * |                                  |
 * \----------------------------------/ 0xffffffffffffffff
 *
 * << Example values for BS == 8kB (2^13) >>
 *
 * V2P_BASE  == 0x0000000000000000
 * P2V_BASE  == 0x0040000000000000
 * TB_BASE   == 0x0060000000000000
 * DS_BASE   == 0x8000000000000000
 *
 * << Summary of br features >>
 *
 * The br is essentially a filesystem, with some simplifications, and some
 * additions.  Through the help of the block repository file (brf) class, a
 * simple file API is available.  The br class only deals with data blocks
 * though.
 *
 * Most of the complexity of the br is necessitated by the need for
 * atomicity guarantees on block updates.  All block writes are done in
 * such a way that other than due to media failure, committed data is never
 * lost, even during unexpected crashes.  This necessarily has a large
 * performance impact, so the br also allows relaxation of these
 * constraints for cases where performance is more important than
 * recovery.  Both write policies can be used at the same time without any
 * worries of br corruption, though data can be lost during crashes for
 * those transactions that use the relaxed commit API.
 *
 * The br provides complex locking modes for blocks.  This facilitates
 * implementation of complex concurrent algorithms such as the JOE-tree
 * (jt) class implements, as well as the brf class mentioned above.
 *
 * << Virtual address lookups >>
 *
 * Virtual addresses are broken into two components.  The example below
 * assumes 8kB pages.
 *
 * bit
 * 63 62                                                    13   12           0
 *  \ |                                                       \ /             |
 *   ?XXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXx xxxx xxxx xxxx
 *
 * The least significant 13 (x) bits are the offset into the block.  The
 * most significant 50 (X) bits are the virtual block address.  Bit 63 is
 * always 0, since the data blocks start at 0x8000000000000000, and there
 * can never be enough physical blocks to require a 1 in the most
 * significant bit of the vaddr.  Since the minimum block size in the br is
 * 512 bytes, the least significant 8 bytes in the V2P and P2V translations
 * are never used.  This leaves at least 9 bits per translation available
 * for additional metadata.
 *
 * << Mapping of block repository backing stores (brbs) >>
 *
 * The brbs class encapsulates the fundamental operations on files and raw
 * devices that are necessary for reading and writing persistent data.  The
 * br uses brbs instances by mapping them into the paddr space.  brbs
 * instances are of constant size, and they can be broken up and mapped
 * into multiple portions of the paddr space.
 *
 * There can be one overlapping mappings at any given time in the V2P.
 * This is important because it allows consolidation of mappings as a br
 * evolves.  Say for example that an installation starts off with only a
 * relatively small portion of the V2P map backed by a brbs.  Sometime
 * later, additional storage is needed, so more of the V2P map is backed by
 * a different brbs.  If this goes on for a while, it may be desireable to
 * replace the multiple small backings with one big backing.  Since we
 * can't just delete the V2P and start over, instead we can add an
 * additional backing to the entire range.  Once that backing is committed
 * (all data is also stored on the new backing), the other backings can be
 * removed.
 *
 * Each brbs instance is responsible for mapping the relevant portion of
 * the P2B, so there is no need for overlapping mappings there.  For the
 * DS, other algorithms can be employed to empty a backing, such as moving
 * data blocks from one backing to another and reflecting the moves in the
 * maps.  However, for the V2P, there is no other way to consolidate and
 * remove backings.
 *
 * << Brief explanation of the br components and logical sections >>
 *
 * The br physically consists of the resource file (triple redundant) and
 * one or more backing stores.  The backing stores are mapped into the br's
 * paddr space.  The paddr space is logically divided into 5 sections.
 * Following are short discriptions of the physical components and logical
 * sections.
 *
 * < resource file >
 *
 * All information necessary to get the br up and running is stored in the
 * resource file, in a format that the res class can read and write.
 * During startup, all three copies of the resource file are checked to
 * make sure that the br is in a consistent state.  Whenever changes are
 * made to the resource file, they are sequentially written out to the
 * three copies so that the changes can be rolled forward or backward in
 * case of a crash.
 *
 * < paddr space >
 *
 * The paddr space is a 64 bit address space into which brbs instances are
 * mapped.  This is where all working data and metadata is stored, other
 * than the bootstrapping information in the resource file.
 *
 * < V2P >
 *
 * The V2P contains a map of vaddr to paddr translations.  Each entry is 16
 * bytes (2 * 64 bits).  The first 8 bytes of each entry is the "primary
 * translation".  The last 8 bytes is the "mirror translation" and is
 * explained below.  Entries are sequentially numbered from V2P_BASE,
 * starting at 0.  vaddr 0 is not used (invalid, illegal) in a normal way 
 * in order to facilitate discovering programming bugs.
 *
 * Free vaddrs are chained together in a free list.  The head and tail of
 * the list, as well as the list size is stored immediately after the last
 * valid vaddr entry.
 *
 * Once any part of the V2P has been backed by a brbs instance, backing can
 * never be removed.  This means that as br usage increases, V2P backing
 * must be increased, but can never be removed.  This is a design tradeoff
 * that avoids considerable overhead and complexity.  Hopefully it is
 * acceptable, since most usage will increase over time, without any
 * permanent decreases in space usage.  If this turns out to have
 * significant negative implications, it may be possible to compact the V2P
 * offline, and perhaps even compact with a smooth transition to the
 * compacted br without ever going offline.  I'll leave these algorithms
 * unimplemented unless the need arises, however.
 *
 * Each V2P entry contains a mirror translation that can be frozen to allow
 * online snapshot backups to be taken.  While the snapshot is being taken,
 * all valid data blocks are left unmodified.  To make this possible, the
 * br switches to a different write algorithm while a backup is in
 * progress.  All block updates are written to new blocks and the V2P is
 * modified to reflect these changes.  Once the backup is complete, all the
 * old blocks pointed to in the mirror mirror entries but not in the
 * primary entries are deallocated.  Then the mirror is brought back in
 * sync with the primary entries.
 *
 * < P2V >
 *
 * The P2V maps paddrs to vaddrs.  This table has embedded in it free
 * lists similar to that in the V2P, except that there is one free list for
 * each brbs mapping.  The head, tail and list size info is stored
 * immediately following the last translation.
 *
 * < TB >
 *
 * The TB contains temporary buffers.  Each buffer is 2 blocks.
 *
 * < DS >
 *
 * The DS contains data blocks.
 *
 ****************************************************************************/

#define _INC_BR_H_
#include <libstash.h>
#include <br_priv.h>

/****************************************************************************
 * <<< Description >>>
 *
 * br constructor.
 *
 ****************************************************************************/
cw_br_t *
br_new(cw_br_t * a_br_o)
{
  cw_br_t * retval;
  
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_new()");
  }

  if (a_br_o == NULL)
  {
    retval = (cw_br_t *) _cw_malloc(sizeof(cw_br_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_br_o;
    retval->is_malloced = FALSE;
  }

  rwl_new(&retval->rw_lock);

  /* Initialize member variables. */
  retval->is_open = FALSE;
  
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_new()");
  }
  return NULL; /* XXX */
}

/****************************************************************************
 * <<< Description >>>
 *
 * br destructor.
 *
 ****************************************************************************/
void
br_delete(cw_br_t * a_br_o)
{
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_delete()");
  }
  _cw_check_ptr(a_br_o);

  if (a_br_o->is_open)
  {
    if (br_close(a_br_o))
    {
      log_leprintf(g_log_o, __FILE__, __LINE__, "br_delete",
		   "Error in br_close()\n");
    }
  }

  rwl_delete(&a_br_o->rw_lock);
  
  if (a_br_o->is_malloced)
  {
    _cw_free(a_br_o);
  }
  
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_delete()");
  }
}

/****************************************************************************
 * <<< Return Value >>>
 *
 * TRUE == repository open.
 * FALSE == repository closed.
 *
 ****************************************************************************/
cw_bool_t
br_is_open(cw_br_t * a_br_o)
{
  cw_bool_t retval;
  
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_is_open()");
  }
  _cw_check_ptr(a_br_o);
  rwl_rlock(&a_br_o->rw_lock);

  retval = a_br_o->is_open;
  
  rwl_runlock(&a_br_o->rw_lock);
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_is_open()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Opens the backing store for a br instance.  The config file for the
 * repository must be a plain file.
 *
 ****************************************************************************/
cw_bool_t
br_open(cw_br_t * a_br_o, char * a_filename)
{
  cw_bool_t retval;
  
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_open()");
  }
  _cw_check_ptr(a_br_o);
  rwl_wlock(&a_br_o->rw_lock);

  /* Open a_filename. */
  /* Get size of config space. */
  /* Parse config space (res format). */
  /* Create and initialize internal data structures. */

  retval = TRUE; /* XXX */
  
  rwl_wunlock(&a_br_o->rw_lock);
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_open()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Closes the backing store for the repository.  All dirty buffers are
 * flushed.
 *
 ****************************************************************************/
cw_bool_t
br_close(cw_br_t * a_br_o)
{
  cw_bool_t retval;
  
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_close()");
  }
  _cw_check_ptr(a_br_o);
  rwl_wlock(&a_br_o->rw_lock);
  
  retval = TRUE; /* XXX */
  
  rwl_wunlock(&a_br_o->rw_lock);
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_close()");
  }
  return retval;
}

/****************************************************************************
 * <<< Return Value >>>
 *
 * Size of block in bytes.
 *
 ****************************************************************************/
cw_uint64_t
br_get_block_size(cw_br_t * a_br_o)
{
  cw_uint64_t retval;
  
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_get_block_size()");
  }
  _cw_check_ptr(a_br_o);
  rwl_rlock(&a_br_o->rw_lock);
  
  retval = 0; /* XXX */
  
  rwl_runlock(&a_br_o->rw_lock);
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_get_block_size()");
  }
  return retval;
}
			    
/****************************************************************************
 * <<< Description >>>
 *
 * Adds a file or device as backing store for the block repository.
 *
 ****************************************************************************/
cw_bool_t
br_add_brbs(cw_br_t * a_br_o, cw_brbs_t * a_brbs_o, cw_uint64_t a_base_addr)
{
  cw_bool_t retval;
  
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_add_brbs()");
  }
  _cw_check_ptr(a_br_o);
  rwl_wlock(&a_br_o->rw_lock);
  
  retval = TRUE; /* XXX */
  
  rwl_wunlock(&a_br_o->rw_lock);
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_add_brbs()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Set *a_brbs_o to the brbs with filename *a_filename, if it exists, and
 * return FALSE.  Otherwise, return TRUE.
 *
 ****************************************************************************/
cw_bool_t
br_get_brbs_p(cw_br_t * a_br_o, char * a_filename, cw_brbs_t ** a_brbs_o)
{
  cw_bool_t retval;
  
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_get_brbs_p()");
  }
  _cw_check_ptr(a_br_o);
  rwl_rlock(&a_br_o->rw_lock);
  
  retval = TRUE; /* XXX */
  
  rwl_runlock(&a_br_o->rw_lock);
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_get_brbs_p()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Removes a file or device from the repository.  First though, all valid
 * blocks are moved off of the file or device.  If there is not enough
 * space to do so, the call will eventually fail.
 *
 ****************************************************************************/
cw_bool_t
br_rm_file(cw_br_t * a_br_o, char * a_filename)
{
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_rm_file()");
  }
  _cw_check_ptr(a_br_o);
  rwl_wlock(&a_br_o->rw_lock);
  

  
  rwl_wunlock(&a_br_o->rw_lock);
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_rm_file()");
  }
  return TRUE; /* XXX */
}

/****************************************************************************
 * <<< Description >>>
 *
 * Allocates a block as used and points *a_brblk_o to it.  The block is
 * returned with a t-lock to assure that no other threads grab a lock on
 * the block before the creating thread gets a chance to.  This is
 * exceedingly unlikely, but is nonetheless necessary for correctness. 
 *
 ****************************************************************************/
cw_bool_t
br_block_create(cw_br_t * a_br_o, cw_brblk_t ** a_brblk_o)
{
  cw_bool_t retval;
  
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_block_destroy()");
  }
  _cw_check_ptr(a_br_o);
  rwl_rlock(&a_br_o->rw_lock);
  
  retval = TRUE; /* XXX */
  
  rwl_runlock(&a_br_o->rw_lock);
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_block_destroy()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Permanently removes a block from the repository.  The physical block is
 * reclaimed for future use.
 *
 ****************************************************************************/
cw_bool_t
br_block_destroy(cw_br_t * a_br_o, cw_brblk_t * a_brblk_o)
{
  cw_bool_t retval;
  
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_block_destroy()");
  }
  _cw_check_ptr(a_br_o);
  rwl_rlock(&a_br_o->rw_lock);
  
  retval = TRUE; /* XXX */
  
  rwl_runlock(&a_br_o->rw_lock);
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_block_destroy()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * s-locks a block, as well as pulling it into cache.
 *
 ****************************************************************************/
cw_bool_t
br_block_slock(cw_br_t * a_br_o,
	       cw_uint64_t a_logical_addr,
	       cw_brblk_t ** a_brblk_o)
{
  cw_bool_t retval;
  
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_block_slock()");
  }
  _cw_check_ptr(a_br_o);
  rwl_rlock(&a_br_o->rw_lock);
  
  retval = TRUE; /* XXX */
  
  rwl_runlock(&a_br_o->rw_lock);
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_block_slock()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * t-locks a block, as well as pulling it into cache.
 *
 ****************************************************************************/
cw_bool_t
br_block_tlock(cw_br_t * a_br_o,
	       cw_uint64_t a_logical_addr,
	       cw_brblk_t ** a_brblk_o)
{
  cw_bool_t retval;
  
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_block_tlock()");
  }
  _cw_check_ptr(a_br_o);
  rwl_rlock(&a_br_o->rw_lock);
  
  retval = TRUE; /* XXX */
  
  rwl_runlock(&a_br_o->rw_lock);
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_block_tlock()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Flushes the block pointed to by a_brblk_o.  The caller must have already 
 * attained an s or t lock fo the block, or else the results of this
 * function call are undefined, and likely to wreak serious havoc.
 *
 ****************************************************************************/
cw_bool_t
br_block_flush(cw_br_t * a_br_o, cw_brblk_t * a_brblk_o)
{
  cw_bool_t retval;
  
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_block_flush()");
  }
  _cw_check_ptr(a_br_o);
  rwl_rlock(&a_br_o->rw_lock);
  
  retval = TRUE; /* XXX */
  
  rwl_runlock(&a_br_o->rw_lock);
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_block_flush()");
  }
  return retval;
}
