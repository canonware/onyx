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
 * $Revision: 136 $
 * $Date: 1998-07-10 13:11:48 -0700 (Fri, 10 Jul 1998) $
 *
 * <<< Description >>>
 *
 * Block repository (br) implementation.  The br consists of four distinct
 * sections, which are spread across multiple files.  Logically, the br
 * looks like:
 *
 * /----------------------------------\
 * |                                  |
 * | config space (cs)                |
 * |                                  |
 * |----------------------------------| 0x0000000000000000
 * |                                  |
 * | v_addr lookup table (vlt)        |
 * |                                  |
 * |                                  |
 * |----------------------------------| (2^64 / (8 * page_size + 1)) 
 * |                                  | div page_size
 * | data blocks (pdb)                |
 * |                                  |
 * |                                  |
 * |                                  |
 * |                                  |
 * |                                  |
 * |                                  |
 * |                                  |
 * |----------------------------------|
 * |                                  |
 * | v_addr lookup table cache (vltc) |
 * |                                  |
 * \----------------------------------/ 0xffffffffffffffff
 *
 * However, the config space is actually a separate file, and the remainder
 * of the br is composed of one or more plain files and raw devices.  These
 * sections are mapped into a unified address space.  That is, files and
 * devices are mapped into an address space that the br sits on top of.  It
 * is possible to have unmapped holes in the br_addr space, and mappings
 * can overlap.
 *
 * In order to prevent having to shuffle sections around to allow expansion
 * and contraction, the sections are set at fixed br_addr's during creation
 * of the repository.  The vltc starts at the top of the address space
 * (0xffffffffffffffff) and grows downward.  Since we know in advance the
 * following:
 *
 *       (vlt + pdb + vltc) <= 2^64 bytes
 *                     vltc >= 0 bytes
 *              sizeof(vlt) == (sizeof(pdb) / page_size) / 8
 *
 * we can calculate the base address of pdb:
 *
 *       base_addr(pdb) == sizeof(vlt) == ((2^64 - vltc) / (8 * page_size + 1))
 *                                        div page_size
 *
 * and
 *
 *       sizeof(pdb) == (((2^64 - vltc) * 8 * page_size) / (8 * page_size + 1))
 *                      div page_size
 *
 * So, assuming a page size of 8kB and vltc of 0B:
 *
 *   base_addr(pdb) == 281470681800704 == FFFF0000E000
 *   sizeof(pdb) == 18446462603027742720 == 0xffff0000ffff0000
 *
 ****************************************************************************
 *
 * This hard coding of section offsets if all fine and good, except that
 * there are some pretty definite holes in the address space.  Since we
 * want the option of using a single backing store (bs) (file or raw
 * device) for all three sections, there needs to be a way of mapping
 * chunks of a bs to separate address ranges.
 *
 * So, that is what br does.  Each backing store is mapped to one or more
 * br_addr ranges.
 *
 * Note that once a portion of the address range dedicated to the vlt is
 * mapped, it cannot be unmapped.  The only way to remove a backing store
 * that is partially or completely mapped to the vtl address range is to
 * first map one or more backing stores to create a redundant copy of that
 * portion of the vlt that we wish to remove a backing store mapping for.
 *
 * Backing stores that have been mapped entirely within the pdb can be
 * removed from the repository by moving all valid blocks to other backing
 * stores and changing the logical to physical address mappings accordingly.
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
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
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
  retval-> is_open = FALSE;
  
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
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
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
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
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
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
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_is_open()");
  }
  _cw_check_ptr(a_br_o);
  rwl_rlock(&a_br_o->rw_lock);

  retval = a_br_o->is_open;
  
  rwl_runlock(&a_br_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
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
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_open()");
  }
  _cw_check_ptr(a_br_o);
  rwl_wlock(&a_br_o->rw_lock);

  /* Open a_filename. */
  /* Get size of config space. */
  /* Parse config space (res format). */
  /* Create and initialize internal data structures. */
  
  
  rwl_wunlock(&a_br_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_open()");
  }
  return TRUE; /* XXX */
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
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_close()");
  }
  _cw_check_ptr(a_br_o);
  rwl_wlock(&a_br_o->rw_lock);
  

  
  rwl_wunlock(&a_br_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_close()");
  }
  return TRUE; /* XXX */
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
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_get_block_size()");
  }
  _cw_check_ptr(a_br_o);
  rwl_rlock(&a_br_o->rw_lock);
  

  
  rwl_runlock(&a_br_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_get_block_size()");
  }
  return 0; /* XXX */
}
			    
/****************************************************************************
 * <<< Description >>>
 *
 * Adds a file or device as backing store for the block repository.
 *
 ****************************************************************************/
cw_bool_t
br_add_file(cw_br_t * a_br_o, char * a_filename,
	    cw_bool_t a_is_raw, cw_bool_t a_can_overlap,
	    cw_bool_t a_is_dynamic,
	    cw_uint64_t a_base_addr, cw_uint64_t a_max_size)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_add_file()");
  }
  _cw_check_ptr(a_br_o);
  rwl_wlock(&a_br_o->rw_lock);
  

  
  rwl_wunlock(&a_br_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_add_file()");
  }
  return TRUE; /* XXX */
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
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_rm_file()");
  }
  _cw_check_ptr(a_br_o);
  rwl_wlock(&a_br_o->rw_lock);
  

  
  rwl_wunlock(&a_br_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_rm_file()");
  }
  return TRUE; /* XXX */
}

/****************************************************************************
 * <<< Description >>>
 *
 * Allocates a block as used and points *a_brblk_o to it.
 *
 ****************************************************************************/
cw_bool_t
br_block_create(cw_br_t * a_br_o, cw_brblk_t ** a_brblk_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_block_destroy()");
  }
  _cw_check_ptr(a_br_o);
  rwl_rlock(&a_br_o->rw_lock);
  

  
  rwl_runlock(&a_br_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_block_destroy()");
  }
  return TRUE; /* XXX */
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
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_block_destroy()");
  }
  _cw_check_ptr(a_br_o);
  rwl_rlock(&a_br_o->rw_lock);
  

  
  rwl_runlock(&a_br_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_block_destroy()");
  }
  return TRUE; /* XXX */
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
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_block_slock()");
  }
  _cw_check_ptr(a_br_o);
  rwl_rlock(&a_br_o->rw_lock);
  

  
  rwl_runlock(&a_br_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_block_slock()");
  }
  return NULL; /* XXX */
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
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_block_tlock()");
  }
  _cw_check_ptr(a_br_o);
  rwl_rlock(&a_br_o->rw_lock);
  

  
  rwl_runlock(&a_br_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_block_tlock()");
  }
  return NULL; /* XXX */
}
