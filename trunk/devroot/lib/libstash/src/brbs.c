/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 94 $
 * $Date: 1998-06-26 17:18:43 -0700 (Fri, 26 Jun 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#define _INC_BRBS_H_
#define _INC_STRING_H_
#define _INC_FCNTL_H_
#define _INC_SYS_TYPES_H_
#define _INC_SYS_STAT_H_
#define _INC_SYS_ERRNO_H_
#define _INC_SYS_DISKLABEL_H_
#include <config.h>

#include <brbs_priv.h>

/****************************************************************************
 * <<< Description >>>
 *
 * Constructor.
 *
 ****************************************************************************/
cw_brbs_t * 
brbs_new(cw_brbs_t * a_brbs_o)
{
  cw_brbs_t * retval;

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_new()");
  }
  if (a_brbs_o == NULL)
  {
    retval = (cw_brbs_t *) _cw_malloc(sizeof(cw_brbs_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_brbs_o;
    retval->is_malloced = FALSE;
  }

  rwl_new(&retval->rw_lock);
  retval->is_open = FALSE;
  retval->filename = NULL;
  retval->fd = -1;
  retval->is_raw = FALSE;
/*   retval->sect_size = 0; */
  retval->is_dynamic = FALSE;
  retval->max_size = 0;

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_new()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Destructor.
 *
 ****************************************************************************/
void 
brbs_delete(cw_brbs_t * a_brbs_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_delete()");
  }
  _cw_check_ptr(a_brbs_o);

  if (a_brbs_o->is_open)
  {
    brbs_close(a_brbs_o);
  }
  
  rwl_delete(&a_brbs_o->rw_lock);

  if (a_brbs_o->is_malloced == TRUE)
  {
    _cw_free(a_brbs_o);
  }

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_delete()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Returns whether the backing store is open.
 *
 ****************************************************************************/
cw_bool_t 
brbs_is_open(cw_brbs_t * a_brbs_o)
{
  cw_bool_t retval;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_is_open()");
  }
  _cw_check_ptr(a_brbs_o);
/*   rwl_rlock(&a_brbs_o->rw_lock); */

  retval = a_brbs_o->is_open;

/*   rwl_runlock(&a_brbs_o->rw_lock); */
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_is_open()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Opens a file or raw device to use for read and write operations.  
 *
 ****************************************************************************/
cw_bool_t
brbs_open(cw_brbs_t * a_brbs_o)
{
  cw_bool_t retval = FALSE;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_open()");
  }
  _cw_check_ptr(a_brbs_o);
  rwl_wlock(&a_brbs_o->rw_lock);

  if ((a_brbs_o->is_open)
      || (a_brbs_o->filename == NULL))
  {
    retval = TRUE;
  }
  else
  {
    /* Open the file. */
    a_brbs_o->fd = open(a_brbs_o->filename, O_RDWR, 0);
    if (a_brbs_o->fd == -1)
    {
      if (errno == EACCES)
      {
	/* We're not allowed to do the operation.  Return an error. */
	retval = TRUE;
	log_leprintf(g_log_o, __FILE__, __LINE__, "brbs_open",
		     "open() error: %s\n", strerror(errno));
      }
      else if (errno == ENOENT)
      {
	/* The file doesn't exist.  Try to create it. */
	a_brbs_o->fd = open(a_brbs_o->filename, O_RDWR | O_CREAT, 0600);
	if (a_brbs_o->fd == -1)
	{
	  retval = TRUE;
	  log_leprintf(g_log_o, __FILE__, __LINE__, "brbs_open",
		       "open() error: %s\n", strerror(errno));
	}
	else
	{
	  a_brbs_o->is_open = TRUE;
	}
      }
    }
    else
    {
      a_brbs_o->is_open = TRUE;
    }

    if (a_brbs_o->is_open == TRUE)
    {
      /* Figure out whether this is a normal file, or a raw device. */
      brbs_p_get_is_raw(a_brbs_o);
      
      if (a_brbs_o->is_raw == TRUE)
      {
	/* Raw device. */
	a_brbs_o->is_raw = TRUE;
	
	brbs_p_get_sector_size(a_brbs_o);
      }
      else
      {
	/* Normal file. */
	a_brbs_o->is_raw = FALSE;
      }
    }
  }

  rwl_wunlock(&a_brbs_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_open()");
  }
  return retval;
}

/****************************************************************************
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
cw_bool_t 
brbs_close(cw_brbs_t * a_brbs_o)
{
  cw_bool_t retval;

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_close()");
  }
  _cw_check_ptr(a_brbs_o);
  rwl_wlock(&a_brbs_o->rw_lock);

  if (a_brbs_o->is_open)
  {
    if (close(a_brbs_o->fd) == -1)
    {
      retval = TRUE;
      log_leprintf(g_log_o, __FILE__, __LINE__, "brbs_close",
		   "close() error: %s\n", strerror(errno));
    }
    else
    {
      retval = FALSE;
    }
  }
  else
  {
    retval = TRUE;
  }

  rwl_wunlock(&a_brbs_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_close()");
  }
  return retval;
}

/****************************************************************************
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
char * 
brbs_get_filename(cw_brbs_t * a_brbs_o)
{
  char * retval;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_get_filename()");
  }
  _cw_check_ptr(a_brbs_o);
/*   rwl_rlock(&a_brbs_o->rw_lock); */

  /* Note that the value can change at any time! */
  retval = a_brbs_o->filename;

/*   rwl_runlock(&a_brbs_o->rw_lock); */
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_get_filename()");
  }
  return retval;
}

/****************************************************************************
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
cw_bool_t
brbs_set_filename(cw_brbs_t * a_brbs_o, char * a_filename)
{
  cw_bool_t retval;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_set_filename()");
  }
  _cw_check_ptr(a_brbs_o);
  _cw_check_ptr(a_filename);
  rwl_wlock(&a_brbs_o->rw_lock);

  if (a_brbs_o->is_open == TRUE)
  {
    retval = TRUE;
  }
  else
  {
    retval = FALSE;
    a_brbs_o->filename = (char *) _cw_realloc(a_brbs_o->filename,
					      strlen(a_filename) + 1);
    strcpy(a_brbs_o->filename, a_filename);
  }

  rwl_wunlock(&a_brbs_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_set_filename()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Returns whether the backing store is dynamically resizeable.
 *
 ****************************************************************************/
cw_bool_t 
brbs_get_is_dynamic(cw_brbs_t * a_brbs_o)
{
  cw_bool_t retval;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_get_is_dynamic()");
  }
  _cw_check_ptr(a_brbs_o);
  rwl_rlock(&a_brbs_o->rw_lock);

  retval = a_brbs_o->is_dynamic;

  rwl_runlock(&a_brbs_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_get_is_dynamic()");
  }

  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * As long as the backing store isn't an open raw device, set
 * a_brbs_o->is_dynamic.
 *
 ****************************************************************************/
cw_bool_t 
brbs_set_is_dynamic(cw_brbs_t * a_brbs_o, cw_bool_t a_is_dynamic)
{
  cw_bool_t retval;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_set_is_dynamic()");
  }
  _cw_check_ptr(a_brbs_o);
  rwl_wlock(&a_brbs_o->rw_lock);

  if ((a_brbs_o->is_open == TRUE) && (a_brbs_o->is_raw == TRUE))
  {
    retval = TRUE;
  }
  else
  {
    retval = FALSE;
    a_brbs_o->is_dynamic = a_is_dynamic;
  }

  rwl_wunlock(&a_brbs_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_set_is_dynamic()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Return a_brbs_o->max_size.
 *
 ****************************************************************************/
cw_uint64_t 
brbs_get_max_size(cw_brbs_t * a_brbs_o)
{
  cw_uint64_t retval;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_get_max_size()");
  }
  _cw_check_ptr(a_brbs_o);
  rwl_rlock(&a_brbs_o->rw_lock);

  retval = a_brbs_o->max_size;

  rwl_runlock(&a_brbs_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_get_max_size()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * As long as the backing store isn't an open raw device, set
 * a_brbs_o->max_size.
 *
 ****************************************************************************/
cw_bool_t 
brbs_set_max_size(cw_brbs_t * a_brbs_o, cw_uint64_t a_max_size)
{
  cw_bool_t retval;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_set_max_size()");
  }
  _cw_check_ptr(a_brbs_o);
  rwl_wlock(&a_brbs_o->rw_lock);

  if ((a_brbs_o->is_open == TRUE) && (a_brbs_o->is_raw == TRUE))
  {
    retval = TRUE;
  }
  else
  {
    retval = FALSE;
    a_brbs_o->max_size = a_max_size;
  }

  rwl_wunlock(&a_brbs_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_set_max_size()");
  }
  return retval;
}

/****************************************************************************
 * <<< Arguments >>>
 *
 * a_offset : Offset in bytes from the beginning of the file.
 *
 * <<< Return Value >>>
 *
 * TRUE == error.
 *
 * <<< Description >>>
 *
 * Reads a block into a_block.
 *
 ****************************************************************************/
cw_bool_t 
brbs_block_read(cw_brbs_t * a_brbs_o, cw_uint64_t a_offset,
		cw_brblk_t * a_brblk_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_block_read()");
  }
  _cw_check_ptr(a_brbs_o);
  _cw_check_ptr(a_brblk_o);
  rwl_rlock(&a_brbs_o->rw_lock);

/*   _cw_assert(a_brblk_o->block_size > 0); */

/*   if (a_brblk_o->block_size */

  rwl_runlock(&a_brbs_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_block_read()");
  }
}

/****************************************************************************
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
cw_bool_t 
brbs_block_write(cw_brbs_t * a_brbs_o, cw_uint64_t a_offset,
		 cw_brblk_t * a_brblk_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_block_write()");
  }
  _cw_check_ptr(a_brbs_o);
  _cw_check_ptr(a_brblk_o);
  rwl_wlock(&a_brbs_o->rw_lock);



  rwl_wunlock(&a_brbs_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_block_write()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Determines the sector size of a device and sets a_brbs_o->sect_size
 * accordingly.
 *
 ****************************************************************************/
void
brbs_p_get_sector_size(cw_brbs_t * a_brbs_o)
{
  struct disklabel dlp;

  a_brbs_o->sect_size = 0;
  
  if (ioctl(a_brbs_o->fd, DIOCGDINFO, &dlp) < 0)
  {
#define _CW_BUF_POWER 13
    cw_uint8_t buf[1 << _CW_BUF_POWER];
    int i;
    
    log_leprintf(g_log_o, __FILE__, __LINE__, "brbs_p_get_sector_size",
		 "ioctl() error: %s\n", strerror(errno));

    /* Try to find the sector size the gross way. */
    for (i = 1; (i <= _CW_BUF_POWER); i++)
#undef _CW_BUF_POWER
    {
      if (read(a_brbs_o->fd, &buf, 1 << (i - 1)) >= 0)
      {
	a_brbs_o->sect_size = 1 << (i - 1);
	break;
      }
    }
  }
  else
  {
    a_brbs_o->sect_size = dlp.d_secsize;
  }
  
  _cw_assert(a_brbs_o->sect_size > 0);
}

/****************************************************************************
 * <<< Description >>>
 *
 * Determines whether a file is a "character special" device (raw) and sets 
 * a_brbs_o->is_raw accordingly.
 *
 ****************************************************************************/
void
brbs_p_get_is_raw(cw_brbs_t * a_brbs_o)
{
  struct stat sb;

  if (-1 == fstat(a_brbs_o->fd, &sb))
  {
    log_leprintf(g_log_o, __FILE__, __LINE__, "brbs_p_get_is_raw",
		 "fstat() error: %s\n", strerror(errno));
  }
  else
  {
    if (S_ISCHR(sb.st_mode))
    {
      a_brbs_o->is_raw = TRUE;

      /* Figure out how big the device is. */
      a_brbs_o->max_size = lseek(a_brbs_o->fd, 0, SEEK_END);
      _cw_assert(a_brbs_o->max_size != -1);
    }
    else
    {
      a_brbs_o->is_raw = FALSE;
    }
  }
}
