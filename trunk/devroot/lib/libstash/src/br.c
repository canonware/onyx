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
 * $Revision: 233 $
 * $Date: 1998-09-23 16:22:28 -0700 (Wed, 23 Sep 1998) $
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
 * lost, even during unexpected crashes.  This necessarily has a
 * performance impact, so the br also allows relaxation of these
 * constraints for cases where performance is more important than recovery.
 * Both write policies can be used at the same time without any worries of
 * br corruption, though data can be lost during crashes for those
 * transactions that use the relaxed commit API.
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
 * instances are of constant size (can't expand or contract).
 *
 * There can be overlapping mappings.  This is important because it allows
 * consolidation of mappings as a br evolves, as well as changing the
 * hardware on which data is stored without taking the br offline.  Say for
 * example that an installation starts off with only a relatively small
 * portion of the V2P map backed by a brbs.  Sometime later, additional
 * storage is needed, so more of the V2P map is backed by a different brbs.
 * If this goes on for a while, it may be desireable to replace the
 * multiple small backings with one big backing.  Since we can't just
 * delete the V2P and start over, instead we can add an additional backing
 * to the entire range.  Once that backing is committed (all data is also
 * stored on the new backing), the other backings can be removed.
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
 *
 * The P2V maps paddrs to vaddrs.  This table is interleaved in the DS, as
 * shown below.  Each P2V block contains all the metadata (other than that
 * stored in the V2P) for an "extent group".  An "extent" is nothing more
 * than a group of blocks.  The special aspect of extents is that they
 * provide a means of allocating sets of contiguous blocks.
 *
 * Each extent group consists of one metadata block (unuseable space)
 * followed by as many blocks as the metadata block is capable of handling
 * (minus one).  The metadata block needs 8 bytes of space for each data
 * block.  So, for 8 kB blocks, one extent group has 1023 data blocks,
 * which is one block short of 8 MB.
 *
 * Extents themselves consist of 2^N blocks, where N is less than
 * log_2(blocks in an extent group).  So, for 8 kB pages (8 MB extent
 * groups), the largest possible extent is 512 blocks (4 MB).
 *
 * Following is a diagram of what a P2V entry looks like for 8 kB blocks:
 *
 * bit
 * 63 62                                                    13   12           0
 *  \ |                                                       \ /             |
 *   UXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXx xxTT TTTT TTTT
 *
 * U == free bit (0 == free, 1 == used)
 * X == block vaddr
 * x == unused
 * T == extent tree bits
 *
 * The U, X and x bits are self-explanatory.  The T bits take some
 * explaining.  As mentioned above, the number of blocks in an extent is
 * always a power of 2.  This allows extents to be subdivided an
 * recombined.  Such subdivisions can be represented as a sort of tree.
 * For 8 kB blocks, the tree could look like the following.  The tree on
 * the left is the least branched that a tree can be, due to the metadata
 * block.
 *
 *                            X                           X
 *                           / \                         / \
 *                          X   512                     X   512
 *                         / \                         / \______
 *                        X   256                     X         X
 *                       / \                         / \       / \
 *                      X   128                     X   128 128   X
 *                     / \                         / \           / \
 *                    X   64                      X   64        X   64
 *                   / \                         / \           / \
 *                  X   32                      X   32        X   32
 *                 / \                         / \           / \
 *                X   16                      X   16       16   X
 *               / \                         / \             __/ \__
 *              X   8                       X   8           X       X
 *             / \                         / \___          / \     / \
 *            X   4                       X      X        4   4   X   4
 *           / \                         / \    / \                  / \
 *          X   2                       X   2  2   2                2   X
 *         / \                         / \                             / \
 * metadata   1                metadata   1                           1   1
 *
 * The T bits in the P2V entries that correspond to the first block of each
 * extent are used to encode the size of the extents.  For example, if the
 * T bits are set to 0x200, that indicates a 512 block extent, and 0x004
 * would indicate a 4 block extent.
 *
 * Deallocated extents can be recombined with their neighbors (if the
 * neighbors are also free) in logarithmic time.  This makes part of the
 * extent fragmentation problem go away.  Since all data blocks are
 * virtually addressed, compaction threads can work during idle time.  With
 * one or more compactor threads running, fragmentation can essentially
 * disappear as a problem, though for highly dynamic file stores,
 * fragmentation may never surpass a less than ideal steady state.  Still,
 * given enough free disk space, fragmentation should never become a
 * serious problem.
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
 * XXX Provisions need to be made for grouping multiple block writes as a
 * single transaction.
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

#include <string.h>

/****************************************************************************
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
    bzero(retval, sizeof(cw_br_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_br_o;
    bzero(retval, sizeof(cw_br_t));
    retval->is_malloced = FALSE;
  }

  rwl_new(&retval->rw_lock);
  res_new(&retval->res);
  /* Non-thread-safe hash table, since we're already taking care of the
   * locking. */
  oh_new(&retval->vaddr_hash, FALSE);
  list_new(&retval->spare_brblk_list, FALSE);
  
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_new()");
  }
  return retval;
}

/****************************************************************************
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
  res_delete(&a_br_o->res);
  {
    cw_uint32_t i;

    for (i = 0; i < 3; i++)
    {
      if (a_br_o->res_files[i] != NULL)
      {
	_cw_free(a_br_o->res_files[i]);
      }
    }
  }
  oh_delete(&a_br_o->vaddr_hash);
  if (a_br_o->map != NULL)
  {
    _cw_free(a_br_o->map);
  }
  list_delete(&a_br_o->spare_brblk_list);
  
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
 *
 * Create a new br.  The resource filenames must be set before calling this
 * function.
 *
 ****************************************************************************/
cw_bool_t
br_create(cw_br_t * a_br_o, cw_uint32_t a_block_size)
{
  cw_bool_t retval = FALSE;
#define _STASH_BR_BS_STRLEN 17
  char block_size[_STASH_BR_BS_STRLEN];

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
      case 0x200: /* 512 B */
      case 0x400:
      case 0x800:
      case 0x1000:
      case 0x2000: /* 8 kB */
      case 0x4000:
      case 0x8000:
      case 0x10000:
      case 0x20000:
      case 0x40000:
      case 0x80000: /* 512 kB */
      {
	int bytes_written;
	
	bytes_written = snprintf(block_size, _STASH_BR_BS_STRLEN,
				 "block_size:%x", a_block_size);
	_cw_assert(bytes_written < _STASH_BR_BS_STRLEN);
#undef _STASH_BR_BS_STRING
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
    if (res_dump(&a_br_o->res, a_br_o->res_files[0])
	|| res_dump(&a_br_o->res, a_br_o->res_files[1])
	|| res_dump(&a_br_o->res, a_br_o->res_files[2]))
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
 *
 * Set arguments to point to the names of the resource files.  The string
 * pointers point to the internal copies, so they must not be modified by
 * the caller.
 *
 ****************************************************************************/
void
br_get_res_files(cw_br_t * a_br_o, char * a_res_files[])
{
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_get_res_files()");
  }
  _cw_check_ptr(a_br_o);
  _cw_check_ptr(a_res_files);
/*   rwl_rlock(&a_br_o->rw_lock); */

  a_res_files[0] = a_br_o->res_files[0];
  a_res_files[1] = a_br_o->res_files[1];
  a_res_files[2] = a_br_o->res_files[2];
  
/*   rwl_runlock(&a_br_o->rw_lock); */
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_get_res_files()");
  }
}

/****************************************************************************
 *
 * Set the resource filenames.  An internal copy of the strings is made, so
 * the caller is responsible for deallocating space used by the argument
 * strings.
 *
 ****************************************************************************/
cw_bool_t
br_set_res_files(cw_br_t * a_br_o, char * a_res_files[])
{
  cw_bool_t retval;

  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_set_res_files()");
  }
  _cw_check_ptr(a_br_o);
  _cw_check_ptr(a_res_files);
  rwl_wlock(&a_br_o->rw_lock);

  if (a_br_o->is_open)
  {
    /* XXX This isn't really acceptable, since it would mean taking the br
     * offline just to change the resource files. */
    retval = TRUE;
  }
  else
  {
    cw_uint32_t i;
    
    retval = FALSE;

    for (i = 0; i < 3; i++)
    {
      if (a_res_files[i] != NULL)
      {
	if (a_br_o->res_files[i] == NULL)
	{
	  a_br_o->res_files[i] = (char *) _cw_malloc(strlen(a_res_files[i])
						     + 1);
	}
	else
	{
	  a_br_o->res_files[i] = (char *) _cw_realloc(a_br_o->res_files[i],
						   strlen(a_res_files[i]) + 1);
	}
	strcpy(a_br_o->res_files[i], a_res_files[i]);
      }
    }
  }
  
  rwl_wunlock(&a_br_o->rw_lock);
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_set_res_files()");
  }
  return retval;
}

/****************************************************************************
 *
 * Checks whether the br was shut down properly.  If not, rolls all pending
 * updates forward or backward in order to put the br in a consistent
 * state.
 *
 ****************************************************************************/
cw_bool_t
br_fsck(cw_br_t * a_br_o)
{
  cw_bool_t retval;

  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_fsck()");
  }
  _cw_check_ptr(a_br_o);
  rwl_wlock(&a_br_o->rw_lock);

  if (a_br_o->is_open)
  {
    retval = TRUE;
  }
  else
  {
    retval = br_p_fsck(a_br_o);
  }
    
  rwl_wunlock(&a_br_o->rw_lock);
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_fsck()");
  }
  return retval;
}

/****************************************************************************
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

  /* XXX */
  _cw_error("Unimplemented");
  
  rwl_runlock(&a_br_o->rw_lock);
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_dump()");
  }
}

/****************************************************************************
 *
 * Return the size of the brblk cache (number of blocks).
 *
 ****************************************************************************/
cw_uint64_t
br_get_num_cache_brblks(cw_br_t * a_br_o)
{
  cw_uint64_t retval;
  
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_get_num_cache_brblks()");
  }
  _cw_check_ptr(a_br_o);
/*   rwl_rlock(&a_br_o->rw_lock); */

  retval = a_br_o->num_cache_brblks;
  
/*   rwl_runlock(&a_br_o->rw_lock); */
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_get_num_cache_brblks()");
  }
  return retval;
}

/****************************************************************************
 *
 * Set the size of the brblk cache.
 *
 ****************************************************************************/
cw_bool_t
br_set_num_cache_brblks(cw_br_t * a_br_o, cw_uint64_t a_num_brblks)
{
  cw_bool_t retval;
  
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_set_num_cache_brblks()");
  }
  _cw_check_ptr(a_br_o);
  rwl_rlock(&a_br_o->rw_lock);

  retval = 0; /* XXX */
  
  rwl_runlock(&a_br_o->rw_lock);
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_set_num_cache_brblks()");
  }
  return retval;
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
  rwl_wlock(&a_br_o->rw_lock);

  if (br_fsck(a_br_o))
  {
    retval = TRUE;
  }
  else
  {
    retval = FALSE; /* XXX */
    
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
 *
 * Removes a brbs from the repository.  First though, if the brbs is mapped
 * in the DS and portions of the brbs have no overlapping mappings, all
 * valid blocks are moved off of the file or device.  If there is not
 * enough space to do so, the call will eventually fail.
 *
 ****************************************************************************/
cw_bool_t
br_rm_brbs(cw_br_t * a_br_o, cw_brbs_t * a_brbs_o)
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

/****************************************************************************
 *
 * Check br consistency, and fix anything that needs fixed.
 *
 ****************************************************************************/
cw_bool_t
br_p_fsck(cw_br_t * a_br_o)
{
  cw_bool_t retval = FALSE, is_clean = FALSE;
  cw_bool_t fix_file[3];
  cw_bool_t error[3];
  cw_res_t t_res[3];
  cw_uint32_t which_valid;

  fix_file[0] = FALSE;
  fix_file[1] = FALSE;
  fix_file[2] = FALSE;
  
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_p_fsck()");
  }
  _cw_check_ptr(a_br_o->res_files[0]);
  _cw_check_ptr(a_br_o->res_files[1]);
  _cw_check_ptr(a_br_o->res_files[2]);
  
  res_new(&t_res[0]);
  res_new(&t_res[1]);
  res_new(&t_res[2]);
  
  error[0] = res_merge_file(&t_res[0], a_br_o->res_files[0]);
  error[1] = res_merge_file(&t_res[1], a_br_o->res_files[1]);
  error[2] = res_merge_file(&t_res[2], a_br_o->res_files[2]);

  if (error[0] || error[1] || error[2])
  {
    /* At least one resource file can't be read.  Try to recover. */
      
    if ((error[0] && error[1])
	|| (error[0] && error[2])
	|| (error[1] && error[2]))
    {
      /* Unresolvable error.  There may still be one valid resource file, 
       * but the standard procedures br uses ensure that at least two are 
       * intact at any given time.  As such, there's no way to know
       * whether to trust a single resource file. */
      log_eprintf(g_log_o, NULL, 0, "br_p_fsck",
		  "Errors reading at least 2 resource files\n");
      retval = TRUE;
      goto RETURN;
    }
    else
    {
      if (error[0])
      {
	if (res_is_equal(&t_res[1], &t_res[2]))
	{
	  /* The resource files are valid.  Assume the crash happened
	   * while updating file A.  Roll backward and use file C.*/
	  which_valid = 2;
	  fix_file[0] = TRUE;
	}
	else
	{
	  /* Something bad happened.  An error in file A indicates it was
	   * being written to during the crash, which means files B and C
	   * should be identical. */
	  log_eprintf(g_log_o, NULL, 0, "br_p_fsck",
		      "Error reading \"%s\", and \"%s\" and \"%s\" disagree\n",
		      a_br_o->res_files[0], a_br_o->res_files[1],
		      a_br_o->res_files[2]);
	  retval = TRUE;
	  goto RETURN;
	}
      }
      else if (error[1])
      {
	/* Files A and C are likely (but not for sure) different.  Roll
	 * forward and use file A. */
	which_valid = 0;
	fix_file[1] = TRUE;
	fix_file[2] = TRUE;
      }
      else /* (error[2]) */
      {
	if (res_is_equal(&t_res[0], &t_res[1]))
	{
	  /* Assume the crash happened while updating file C.  Files A
	   * and B agree, so roll forward and use file A. */
	  which_valid = 0;
	  fix_file[2] = TRUE;
	}
	else
	{
	  /* Something bad happened.  An error in file C indicates it was 
	   * being written to, which means files A and B should be
	   * identical. */
	  log_eprintf(g_log_o, NULL, 0, "br_p_fsck",
		      "Error reading \"%s\", and \"%s\" and \"%s\" disagree\n",
		      a_br_o->res_files[2], a_br_o->res_files[0],
		      a_br_o->res_files[1]);
	  retval = TRUE;
	  goto RETURN;
	}
      }
    }
  }
  else if (res_is_equal(&t_res[0], &t_res[1])
	   && res_is_equal(&t_res[0], &t_res[2])
	   && strcmp(res_get_res_val(&t_res[0], "clean"), "yes"))
  {
    /* The br was shut down cleanly. */
    is_clean = TRUE;
    which_valid = 0;
  }
  else if (res_is_equal(&t_res[0], &t_res[1])
	   && res_is_equal(&t_res[0], &t_res[2]))
  {
    /* All copies are valid. */
    which_valid = 0;
  }
  else if (res_is_equal(&t_res[0], &t_res[1]))
  {
    /* First and second copies are valid. */
    which_valid = 0;
    fix_file[2] = TRUE;
  }
  else if (res_is_equal(&t_res[1], &t_res[2]))
  {
    /* Second and third copies are valid. */
    which_valid = 1;
    fix_file[0] = TRUE;
  }
  else
  {
    /* All three copies are probably different, so roll forward.  Use the
     * first copy. */
    which_valid = 0;
    fix_file[1] = TRUE;
    fix_file[2] = TRUE;
  }

  if (is_clean == FALSE)
  {
    /* Copy valid resources into &a_br_o->res and synchronize the res files. */
    res_clear(&a_br_o->res);

    if (res_merge_file(&a_br_o->res, a_br_o->res_files[which_valid]))
    {
      _cw_error("Unexpected error");
    }
    {
      cw_uint32_t i, next;

      /* This looks like a bunch of unnecessary complexity, but care must
       * be taken that the res files are modified in an order that allows
       * a subsequent run of this function to recover from write failures. */
      for (i = 0, next = (which_valid + 1) % 3;
	   i < 2;
	   i++, next = (next + 1) % 3)
      {	
	if (fix_file[next])
	{
	  if (res_dump(&a_br_o->res, a_br_o->res_files[next]))
	  {
	    log_eprintf(g_log_o, NULL, 0, "br_p_fsck",
			"Error writing to file \"%s\"\n",
			a_br_o->res_files[next]);
	    retval = TRUE;
	    goto RETURN;
	  }
	}
      }
    }
    
    /* Map the brbs instances into the paddr space. */

    /* Find all valid TB buffers and roll forward the blocks they
       correspond to, if necessary.  Mark each TP buffer valid only _after_
       rolling forward its corresponding block. */

    /* Sync up the V2P mirror. */

    /* Clear the interim counters for the free lists and add their values
     * to the main counters (not in that order =) ). */

    /* Close the brbs instances. */

    /* Set the clean resource to "yes", and write all three resource
     * files. */
    res_merge_list(&a_br_o->res, "clean:yes", NULL);
    {
      cw_uint32_t i;

      for (i = 0; i < 3; i++)
      {
	
	if (res_dump(&a_br_o->res, a_br_o->res_files[i]))
	{
	  log_eprintf(g_log_o, NULL, 0, "br_p_fsck",
		      "Error writing to file \"%s\"\n",
		      a_br_o->res_files[i]);
	  retval = TRUE;
	  goto RETURN;
	}
      }
    }
  }
  
 RETURN:
  res_delete(&t_res[0]);
  res_delete(&t_res[1]);
  res_delete(&t_res[2]);
    
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_p_fsck()");
  }
  return retval;
}

/****************************************************************************
 *
 * Map a brbs into the paddr space.
 *
 ****************************************************************************/
cw_bool_t
br_p_map_brbs(cw_br_t * a_br_o, cw_brbs_t * a_brbs_o, cw_uint64_t a_base_addr)
{
  cw_bool_t retval;

  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_p_map_brbs()");
  }
  
  retval = TRUE; /* XXX */
  
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_p_map_brbs()");
  }
  return retval;
}

/****************************************************************************
 *
 * Unmap a brbs.
 *
 ****************************************************************************/
cw_bool_t
br_p_unmap_brbs(cw_br_t * a_br_o, cw_brbs_t * a_brbs_o)
{
  cw_bool_t retval;

  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_p_map_brbs()");
  }
  
  retval = TRUE; /* XXX */
  
  if (_cw_pmatch(_STASH_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_p_map_brbs()");
  }
  return retval;
}
