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
 * $Revision: 190 $
 * $Date: 1998-09-01 18:14:15 -0700 (Tue, 01 Sep 1998) $
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
 * P2V       : paddr to vaddr map (interleaved into DS)
 * TB        : temporary buffer space
 * DS        : data space
 * V2P_BASE  : base address of V2P
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
 * |----------------------------------| TB_BASE == V2P_BASE + (2^67 / BS)
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
 * | data space and interleaved       |
 * | paddr --> vaddr map (P2V)        |
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
 * << Example values for BS == 8 kB (2^13) >>
 *
 * V2P_BASE  == 0x0000000000000000
 * TB_BASE   == 0x0040000000000000
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
 * assumes BS == 8 kB.
 *
 * bit
 * 63 62                                                    13   12           0
 *  \ |                                                       \ /             |
 *   ?XXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXx xxxx xxxx xxxx
 *
 * The least significant 13 (x) bits are the offset into the block.  The
 * next 50 (X) bits are the virtual block address.  Bit 63 is always 0,
 * since the data blocks start at 0x8000000000000000, and there can never
 * be enough physical blocks to require a 1 in the most significant bit of
 * the vaddr.  Since the minimum block size in the br is 512 bytes, the
 * least significant 8 bytes in the V2P and P2V translations are never
 * used.  This leaves at least 9 bits per translation available for
 * additional metadata.
 *
 * << Mapping of block repository backing stores (brbs) >>
 *
 * The brbs class encapsulates the fundamental operations on files and raw
 * devices that are necessary for reading and writing persistent data.  The
 * br uses brbs instances by mapping them into the paddr space.  brbs
 * instances are of constant size, and they can be broken up and mapped
 * into multiple portions of the paddr space.
 *
 * There can be one overlapping mapping at any given time in the V2P.  This
 * is important because it allows consolidation of mappings as a br
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
 * the case of a crash.
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
 * the free list, as well as the list size is stored immediately after the
 * last valid vaddr entry.
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
 * The P2V maps paddrs to vaddrs.  This table (interleaved in the DS, shown
 * below) has embedded in it free lists similar to that in the V2P, except
 * that there is one free list for each brbs mapping.  The head, tail and
 * list size info is stored in the first 8 bytes of the first 3 P2V blocks, 
 * respectively.  This means that each brbs instance that maps into the DS
 * must be big enough to use at least 3 P2V blocks:
 *
 * ((((block size / 8) - 1) * 2) + 1) * block size
 *
 * For BS == 8 kB:
 *
 * ((((2^13 / 2^3) - 1) * 2) + 1) * 2^13
 *  == 0xffe000 B
 *  == 16769024 B
 *  == 15.99 MB
 *
 * For BS == 512 B:
 *
 * ((((2^9 / 2^3) - 1) * 2) + 1) * 2^9
 *  == 0xfe00 B
 *  == 65024 B
 *  == 63 kB
 *
 * In actuality, there is some additional complexity in how the list size
 * is stored.  Since every time the head or tail changes, it also changes
 * the list size, it would normally be necessary to write the list size
 * every time the list changes.  Instead the least significant 8 bits of
 * the head and tail are used as counters.  Every time the counter wraps
 * around, the list size is updated.  This almost halves the number of
 * writes that are necessary to keep the P2V consistent.
 *
 * DS and P2V interleaving layout:
 *
 * /-------------------------------------\
 * | ((block size / 8) - 1) translations | P2V block
 * |                                     |
 * |-------------------------------------|
 * |                                     | \
 * |-------------------------------------| |
 * |                                     | |
 * |-------------------------------------| |
 * |                                     | |
 * :                                     :  \ 
 * :                                     :   > ((block size / 8) - 1)
 * :                                     :  /  data blocks
 * |-------------------------------------| |
 * |                                     | |
 * |-------------------------------------| |
 * |                                     | /
 * \-------------------------------------/
 *
 * There may be trailing space at the end of the last block of the P2V
 * mapping that is unused.  The physical block addresses that correspond to
 * this unused space cannot be used at all.  In other words, DS backing size
 * is in multiples of (block size / 8) bytes.
 *
 * < TB >
 *
 * The TB contains temporary buffers.  Each buffer is 2 blocks.  The first
 * half of the first block and the last half of the second block contain
 * identical information about the validity of the buffer.  The space in
 * the middle is used to store the block data being buffered.
 *
 * TB buffers are used in the algorithms that guarantee data consistency in
 * the br.  Updates are done in such a fashion as to be able to use (or in
 * some cases discard) TB buffer data to bring the br to a consistent
 * state.
 *
 * In order to reduce the number of writes necessary for a block update,
 * the TB has a rather complex structure.  Buffers are grouped together and 
 * each group is preceded by 3 metadata blocks.
 *
 * Buffer group layout:
 *
 * /-------------------------------------\
 * |                                     | \
 * |-------------------------------------| |
 * |                                     |  > Metadata blocks
 * |-------------------------------------| |
 * |                                     | /
 * |-------------------------------------| 
 * |                                     | Unused block
 * |-------------------------------------| 
 * |                                     | \           \
 * |-------------------------------------|  > Buffer   |
 * |                                     | /           |
 * |-------------------------------------|             |
 * |                                     |             |
 * :                                     :              \ 
 * :                                     :               > ((block size / 8)
 * :                                     :              /   - 4) Buffers
 * |-------------------------------------|             |
 * |                                     | \           |
 * |-------------------------------------|  > Buffer   |
 * |                                     | /           /
 * \-------------------------------------/
 *
 * Each buffer group is ((block size / 8) * block size * 2) bytes.
 * For BS == 8 kB, each buffer group is (2^13 / 2^3) * 2^13 * 2 == 16 MB.
 * For BS == 512 B, each buffer group is (2^9 / 2^3) * 2^9 * 2 == 64 kB.
 *
 * Buffers are 2 blocks each, so that a block of data and metadata can be
 * written in one contiguous write, and also so that there is no contention 
 * for block locks when writing to buffers.  In other words, if the
 * metadata were stored centrally, then multiple threads would often have
 * to wait for locks on the same metadata blocks.
 *
 * Each buffer contains a copy of its metadata both at the beginning and
 * the end of the buffer.  This allows detection of any writes that didn't
 * complete during a crash.  If both copies of the metadata are the same,
 * then the buffer is consistent.
 *
 * The metadata blocks are triple redundant in order to allow crash
 * recovery.  Eash metadata block contains 8 byte entries (paddr values).
 * A paddr value of zero (0x0) indicates that the corresponding buffer is
 * valid if used (non-zeroed) and consistent.  If the paddr value is
 * non-zero, and it is the same as the paddr stored in the buffer's local
 * metadata, and the buffer is consistent, then the buffer is invalid.
 *
 * Following is the typical sequence of events with regard to the TB:
 *
 * 1) Before the br is even brought up, all buffer groups are completely
 *    zeroed.
 * 2) A list of free buffers (all of them) is created in memory.
 * 3) Buffers are taken off the free list as needed and put into a hash
 *    table of used buffers.  Once the user of the buffer is done with it,
 *    preparations are made to mark the buffer as invalid in the metadata
 *    blocks, and the buffer is put in a pending list.  Periodically, the
 *    metadata blocks are updated and the pending list is appended to the
 *    free list.
 *
 * Some precautions must be taken with this scheme.  For example, say that
 * a buffer was previously used to store block 1, and is now marked invalid
 * in the metadata blocks.  Some time later, the same buffer is used to
 * store block 1.  In order to assure that the old buffer contents are
 * never mistakenly thought valid, 1) the buffer must be zeroed, 2) the
 * metadata blocks must be changed to contain a paddr value of 0 for the
 * buffer, and then 3) the buffer can be used.  Alternately, a different
 * buffer could be used, assuming that another one is currently available.
 *
 * < DS >
 *
 * The DS contains data blocks (and the P2V as mentioned above).  Other
 * classes such as jt and brf use special formatting of the blocks to meet
 * their needs, but the br places absolutely no constraints on the contents
 * of the data blocks.
 *
 ****************************************************************************/

#define _INC_BR_H_
#define _INC_BRBS_H_
#define _INC_BRBLK_H_
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
  /* Non-thread-safe hash table, since we're already taking care of the
   * locking. */
  oh_new(&retval->vaddr_hash, FALSE);
  res_new(&retval->res);
  retval->res_file_a
    = retval->res_file_b
    = retval->res_file_c
    = NULL;

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
  oh_delete(&a_br_o->vaddr_hash);
  res_delete(&a_br_o->res);
  if (a_br_o->res_file_a != NULL)
  {
    _cw_free(a_br_o->res_file_a);
  }
  if (a_br_o->res_file_b != NULL)
  {
    _cw_free(a_br_o->res_file_b);
  }
  if (a_br_o->res_file_c != NULL)
  {
    _cw_free(a_br_o->res_file_c);
  }
  
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
 * <<< Description >>>
 *
 * Create a new br.  The resource filenames must be set before calling this
 * function.
 *
 ****************************************************************************/
cw_bool_t
br_create(cw_br_t * a_br_o, cw_uint32_t a_block_size)
{
  cw_bool_t retval = FALSE;
  char block_size[17] = "block_size:", t_str[6];

  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_create()");
  }
  _cw_check_ptr(a_br_o);
  rwl_wlock(&a_br_o->rw_lock);

  if (a_br_o->is_open == FALSE)
  {
    switch (a_block_size)
    {
      case 0x200:
      case 0x400:
      case 0x800:
      case 0x1000:
      case 0x2000:
      case 0x4000:
      case 0x8000:
      case 0x10000:
      {
	sprintf(t_str, "%x", a_block_size);
	_cw_assert(strlen(t_str) <= 5);
	strcat(block_size, t_str);
	break;
      }
      default:
      {
	retval = TRUE;
	goto RETURN;
      }
    }
    
    res_clear(&a_br_o->res);
    res_merge_list(&a_br_o->res,
		   block_size, /* "block_size:?????" */
		   "clean:yes",
		   "brbs_list:.",
		   "brbs_next_designator:0",
		   NULL);
    if (res_dump(&a_br_o->res, a_br_o->res_file_a)
	|| res_dump(&a_br_o->res, a_br_o->res_file_b)
	|| res_dump(&a_br_o->res, a_br_o->res_file_c))
    {
      retval = TRUE;
    }
  }
  else
  {
    retval = TRUE;
  }
  
 RETURN:
  rwl_wunlock(&a_br_o->rw_lock);
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_create()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Set arguments to point to the names of the resource files.  The string
 * pointers point to the internal copies, so they must not be modified by
 * the caller.
 *
 ****************************************************************************/
void
br_get_res_files(cw_br_t * a_br_o, char ** a_res_file_a,
		 char ** a_res_file_b, char ** a_res_file_c)
{
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_get_res_files()");
  }
  _cw_check_ptr(a_br_o);
  rwl_rlock(&a_br_o->rw_lock);

  *a_res_file_a = a_br_o->res_file_a;
  *a_res_file_b = a_br_o->res_file_b;
  *a_res_file_c = a_br_o->res_file_c;
  
  rwl_runlock(&a_br_o->rw_lock);
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_get_res_files()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Set the resource filenames.  An internal copy of the strings is made, so
 * the caller is responsible for deallocating space used by the argument
 * strings.
 *
 ****************************************************************************/
cw_bool_t
br_set_res_files(cw_br_t * a_br_o, char * a_res_file_a,
		 char * a_res_file_b, char * a_res_file_c)
{
  cw_bool_t retval;

  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_set_res_files()");
  }
  _cw_check_ptr(a_br_o);
  rwl_wlock(&a_br_o->rw_lock);

  if (a_res_file_a != NULL)
  {
    if (a_br_o->res_file_a == NULL)
    {
      a_br_o->res_file_a = (char *) _cw_malloc(strlen(a_res_file_a) + 1);
    }
    else
    {
      a_br_o->res_file_a = (char *) _cw_realloc(a_br_o->res_file_a,
						strlen(a_res_file_a) + 1);
    }
    strcpy(a_br_o->res_file_a, a_res_file_a);
  }
  if (a_res_file_b != NULL)
  {
    if (a_br_o->res_file_b == NULL)
    {
      a_br_o->res_file_b = (char *) _cw_malloc(strlen(a_res_file_b) + 1);
    }
    else
    {
      a_br_o->res_file_b = (char *) _cw_realloc(a_br_o->res_file_b,
						strlen(a_res_file_b) + 1);
    }
    strcpy(a_br_o->res_file_b, a_res_file_b);
  }
  if (a_res_file_c != NULL)
  {
    if (a_br_o->res_file_c == NULL)
    {
      a_br_o->res_file_c = (char *) _cw_malloc(strlen(a_res_file_c) + 1);
    }
    else
    {
      a_br_o->res_file_c = (char *) _cw_realloc(a_br_o->res_file_c,
						strlen(a_res_file_c) + 1);
    }
    strcpy(a_br_o->res_file_c, a_res_file_c);
  }
  
  rwl_wunlock(&a_br_o->rw_lock);
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_set_res_files()");
  }
  return retval;
}

/****************************************************************************
 * <<< Return Value >>>
 *
 * TRUE == Unrecoverable error.  Barring media failure and br bugs, this
 *         function will never return TRUE.
 *
 * <<< Description >>>
 *
 * Checks whether the br was shut down properly.  If not, rolls all pending
 * updates forward or backward in order to put the br in a consistent
 * state.
 *
 ****************************************************************************/
cw_bool_t
br_fsck(cw_br_t * a_br_o)
{
  cw_bool_t retval = FALSE;
  cw_res_t t_res_b, t_res_c;

  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_fsck()");
  }
  _cw_check_ptr(a_br_o);
  _cw_check_ptr(a_br_o->res_file_a);
  _cw_check_ptr(a_br_o->res_file_b);
  _cw_check_ptr(a_br_o->res_file_c);
  rwl_wlock(&a_br_o->rw_lock);
  
  res_new(&t_res_b);
  res_new(&t_res_c);

  if (res_merge_file(&a_br_o->res, a_br_o->res_file_a)
      || res_merge_file(&t_res_b, a_br_o->res_file_b)
      || res_merge_file(&t_res_c, a_br_o->res_file_c)
      || !res_is_equal(&a_br_o->res, &t_res_b)
      || !res_is_equal(&a_br_o->res, &t_res_c)
      || strcmp(res_get_res_val(&a_br_o->res, "clean"), "yes")
      )
  {
    _cw_error("Unimplemented");
    
    /* There was an error of some sort.  Chances are that the repository
     * wasn't cleanly shut down.  Recover. */

    /* Figure out which of the resource files are valid. */

    /* Map the brbs instances into the paddr space. */

    /* Find all valid TB buffers and roll forward the blocks they
       correspond to, if necessary.  Mark each TP buffers valid only
       _after_ rolling forward its corresponding block. */

    /* Sync up the V2P mirror. */

    /* Clear the interim counters for the free lists and add their values
     * to the main counters (not in that order =) ). */

    /* Close the brbs instances. */

    /* Set the clean resource to "yes", and write all three resource files. */
  }
    
  res_delete(&t_res_b);
  res_delete(&t_res_c);
    
  rwl_wunlock(&a_br_o->rw_lock);
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_fsck()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Write current status summary to g_log_o.
 *
 ****************************************************************************/
void
br_dump(cw_br_t * a_br_o)
{
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_dump()");
  }
  _cw_check_ptr(a_br_o);
  rwl_rlock(&a_br_o->rw_lock);

  _cw_error("Unimplemented");
  
  rwl_runlock(&a_br_o->rw_lock);
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_dump()");
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
br_open(cw_br_t * a_br_o)
{
  cw_bool_t retval;
  
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_open()");
  }
  _cw_check_ptr(a_br_o);
  _cw_check_ptr(a_res_file_a);
  _cw_check_ptr(a_res_file_b);
  _cw_check_ptr(a_res_file_c);
  rwl_wlock(&a_br_o->rw_lock);

  if (br_fsck(a_br_o))
  {
    retval = TRUE;
  }
  else
  {
  
    /* Get size of config space. */
    /* Parse config space (res format). */
    /* Create and initialize internal data structures. */
  }
  
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
 * Removes a brbs from the repository.  First though, all valid blocks are
 * moved off of the file or device.  If there is not enough space to do so,
 * the call will eventually fail.
 *
 ****************************************************************************/
cw_bool_t
br_rm_brbs(cw_br_t * a_br_o, char * a_filename)
{
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_rm_brbs()");
  }
  _cw_check_ptr(a_br_o);
  rwl_wlock(&a_br_o->rw_lock);
  

  
  rwl_wunlock(&a_br_o->rw_lock);
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_rm_brbs()");
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
