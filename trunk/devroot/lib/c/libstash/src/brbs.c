/* -*-mode:c-*-
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
 * Block repository backing store class.  This class encapsulates a file or 
 * raw device and provides read and write functions that behave the same
 * regardless of the underlying backing store.
 *
 * XXX A function that writes multiple blocks could speed up the br's
 * buffer writes considerably.
 *
 ****************************************************************************/

#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <sys/errno.h>
#ifdef _ARCH_FREEBSD
#  include <sys/disklabel.h>
#endif

#define _INC_BRBS_H_
#include <libstash.h>
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

  if (_cw_pmatch(_STASH_DBG_R_BRBS_FUNC))
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
  retval->size = 0;
  retval->is_raw = FALSE;
  retval->sect_size = 0;

  if (_cw_pmatch(_STASH_DBG_R_BRBS_FUNC))
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
  if (_cw_pmatch(_STASH_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_delete()");
  }
  _cw_check_ptr(a_brbs_o);

  if (a_brbs_o->is_open)
  {
    brbs_close(a_brbs_o);
  }
  
  rwl_delete(&a_brbs_o->rw_lock);

  if (a_brbs_o->filename != NULL)
  {
    _cw_free(a_brbs_o->filename);
  }

  if (a_brbs_o->is_malloced == TRUE)
  {
    _cw_free(a_brbs_o);
  }

  if (_cw_pmatch(_STASH_DBG_R_BRBS_FUNC))
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
  
  if (_cw_pmatch(_STASH_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_is_open()");
  }
  _cw_check_ptr(a_brbs_o);
/*   rwl_rlock(&a_brbs_o->rw_lock); */

  retval = a_brbs_o->is_open;

/*   rwl_runlock(&a_brbs_o->rw_lock); */
  if (_cw_pmatch(_STASH_DBG_R_BRBS_FUNC))
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
  
  if (_cw_pmatch(_STASH_DBG_R_BRBS_FUNC))
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
	if (_cw_fmatch(_STASH_DBG_R_BRBS_ERROR))
	{
	  log_leprintf(g_log_o, __FILE__, __LINE__, "brbs_open",
		       "open() error: %s\n", strerror(errno));
	}
      }
      else if (errno == ENOENT)
      {
	/* The file doesn't exist.  Try to create it. */
	a_brbs_o->fd = open(a_brbs_o->filename, O_RDWR | O_CREAT, 0600);
	if (a_brbs_o->fd == -1)
	{
	  /* Error opening file. */
	  retval = TRUE;
	  if (_cw_fmatch(_STASH_DBG_R_BRBS_ERROR))
	  {
	    log_leprintf(g_log_o, __FILE__, __LINE__, "brbs_open",
			 "open() error: %s\n", strerror(errno));
	  }
	}
	else
	{
	  /* Success opening file. */
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
      if (brbs_p_get_is_raw(a_brbs_o))
      {
	retval = TRUE;
      }
      else
      {
	if (a_brbs_o->is_raw == TRUE)
	{
	  /* Raw device. */
	  a_brbs_o->is_raw = TRUE;
	
	  retval = brbs_p_get_raw_info(a_brbs_o);
	}
	else
	{
	  /* Normal file. */
	  a_brbs_o->is_raw = FALSE;
    
	  /* Figure out how big the file or device is. */
	  a_brbs_o->size = lseek(a_brbs_o->fd, 0, SEEK_END);
	  if (a_brbs_o->size == -1)
	  {
	    retval = TRUE;
	  }
	}
      }
    }
  }

  rwl_wunlock(&a_brbs_o->rw_lock);
  if (_cw_pmatch(_STASH_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_open()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * If a file is open, close it.
 *
 ****************************************************************************/
cw_bool_t 
brbs_close(cw_brbs_t * a_brbs_o)
{
  cw_bool_t retval;

  if (_cw_pmatch(_STASH_DBG_R_BRBS_FUNC))
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
      if (_cw_fmatch(_STASH_DBG_R_BRBS_ERROR))
      {
	log_leprintf(g_log_o, __FILE__, __LINE__, "brbs_close",
		     "close() error: %s\n", strerror(errno));
      }
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
  if (_cw_pmatch(_STASH_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_close()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Create a new file backing store of size a_size.
 *
 ****************************************************************************/
cw_bool_t
brbs_create(cw_brbs_t * a_brbs_o, cw_uint64_t a_size)
{
  cw_bool_t retval;

  /* XXX Implement. */
  retval = TRUE; /* XXX */
  
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Destroy (unlink) a file backing store.  The file must be currently open.
 *
 ****************************************************************************/
cw_bool_t
brbs_destroy(cw_brbs_t * a_brbs_o)
{
  cw_bool_t retval;
  
  /* XXX Implement. */
  retval = TRUE; /* XXX */

  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Get the current filename.
 *
 ****************************************************************************/
char * 
brbs_get_filename(cw_brbs_t * a_brbs_o)
{
  char * retval;
  
  if (_cw_pmatch(_STASH_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_get_filename()");
  }
  _cw_check_ptr(a_brbs_o);
/*   rwl_rlock(&a_brbs_o->rw_lock); */

  /* Note that the value can change at any time! */
  retval = a_brbs_o->filename;

/*   rwl_runlock(&a_brbs_o->rw_lock); */
  if (_cw_pmatch(_STASH_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_get_filename()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * If a file isn't already open, set the filename.
 *
 ****************************************************************************/
cw_bool_t
brbs_set_filename(cw_brbs_t * a_brbs_o, char * a_filename)
{
  cw_bool_t retval;
  
  if (_cw_pmatch(_STASH_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_set_filename()");
  }
  _cw_check_ptr(a_brbs_o);
  _cw_check_ptr(a_filename);
  rwl_wlock(&a_brbs_o->rw_lock);

  if (a_brbs_o->is_open == TRUE)
  {
    /* Already open; can't change the filename while open. */
    retval = TRUE;
  }
  else
  {
    retval = FALSE;
    if (a_brbs_o->filename == NULL)
    {
      a_brbs_o->filename = (char *) _cw_malloc(strlen(a_filename) + 1);
    }
    else
    {
      a_brbs_o->filename = (char *) _cw_realloc(a_brbs_o->filename,
						strlen(a_filename) + 1);
    }
    
    strcpy(a_brbs_o->filename, a_filename);
  }

  rwl_wunlock(&a_brbs_o->rw_lock);
  if (_cw_pmatch(_STASH_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_set_filename()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Return a_brbs_o->size.
 *
 ****************************************************************************/
cw_uint64_t 
brbs_get_size(cw_brbs_t * a_brbs_o)
{
  cw_uint64_t retval;
  
  if (_cw_pmatch(_STASH_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_get_size()");
  }
  _cw_check_ptr(a_brbs_o);
/*   rwl_rlock(&a_brbs_o->rw_lock); */

  retval = a_brbs_o->size;

/*   rwl_runlock(&a_brbs_o->rw_lock); */
  if (_cw_pmatch(_STASH_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_get_size()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Returns whether the file is a raw device.
 *
 ****************************************************************************/
cw_bool_t
brbs_get_is_raw(cw_brbs_t * a_brbs_o)
{
  cw_bool_t retval;

  if (_cw_pmatch(_STASH_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_get_()");
  }
  _cw_check_ptr(a_brbs_o);
/*   rwl_rlock(&a_brbs_o->rw_lock); */

  retval = a_brbs_o->is_raw;

/*   rwl_runlock(&a_brbs_o->rw_lock); */
  if (_cw_pmatch(_STASH_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_get_()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Returns the sector size (only valid if using a raw device).
 *
 ****************************************************************************/
cw_uint32_t
brbs_get_sect_size(cw_brbs_t * a_brbs_o)
{
  cw_bool_t retval;

  if (_cw_pmatch(_STASH_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_get_()");
  }
  _cw_check_ptr(a_brbs_o);
/*   rwl_rlock(&a_brbs_o->rw_lock); */

  retval = a_brbs_o->sect_size;

/*   rwl_runlock(&a_brbs_o->rw_lock); */
  if (_cw_pmatch(_STASH_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_get_()");
  }
  return retval;
}

/****************************************************************************
 * <<< Arguments >>>
 *
 * a_offset : Offset in bytes from the beginning of the file.
 *
 * <<< Description >>>
 *
 * Reads a block into a_brblk_o.
 *
 ****************************************************************************/
cw_bool_t 
brbs_block_read(cw_brbs_t * a_brbs_o, cw_uint64_t a_offset,
		cw_brblk_t * a_brblk_o)
{
  ssize_t error;
  cw_bool_t retval;
  
  if (_cw_pmatch(_STASH_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_block_read()");
  }
  _cw_check_ptr(a_brbs_o);
  rwl_rlock(&a_brbs_o->rw_lock);

  _cw_assert(brblk_get_buf_size(a_brblk_o) > 0);

  if (-1 == lseek(a_brbs_o->fd, a_offset, SEEK_SET))
  {
    /* Error seeking. */
    retval = TRUE;
    if (_cw_fmatch(_STASH_DBG_R_BRBS_ERROR))
    {
      log_leprintf(g_log_o, __FILE__, __LINE__, "brbs_block_read",
		   "lseek() error: %s\n", strerror(errno));
    }
  }
  else
  {
    error = read(a_brbs_o->fd,
		 (void *) brblk_get_buf_p(a_brblk_o),
		 brblk_get_buf_size(a_brblk_o));
    if (error == -1)
    {
      /* Error reading. */
      retval = TRUE;
      if (_cw_fmatch(_STASH_DBG_R_BRBS_ERROR))
      {
	log_leprintf(g_log_o, __FILE__, __LINE__, "brbs_block_read",
		     "read() error: %s\n", strerror(errno));
      }
    }
    else if (error != brblk_get_buf_size(a_brblk_o))
    {
      /* Partial read error. */
      retval = TRUE;
      if (_cw_fmatch(_STASH_DBG_R_BRBS_ERROR))
      {
	log_leprintf(g_log_o, __FILE__, __LINE__, "brbs_block_read",
		     "Incomplete read");
      }
    }
    else
    {
      /* Success. */
      retval = FALSE;
    }
  }
  
  rwl_runlock(&a_brbs_o->rw_lock);
  if (_cw_pmatch(_STASH_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_block_read()");
  }
  return retval;
}

/****************************************************************************
 * <<< Arguments >>>
 *
 * a_offset : Offset in bytes from the beginning of the file.
 *
 * <<< Description >>>
 *
 * Writes a block.
 *
 ****************************************************************************/
cw_bool_t 
brbs_block_write(cw_brbs_t * a_brbs_o, cw_uint64_t a_offset,
		 cw_brblk_t * a_brblk_o)
{
  ssize_t error;
  cw_bool_t retval;
  
  if (_cw_pmatch(_STASH_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_block_write()");
  }
  _cw_check_ptr(a_brbs_o);
  rwl_wlock(&a_brbs_o->rw_lock);

  _cw_assert(brblk_get_buf_size(a_brblk_o) > 0);

  if (-1 == lseek(a_brbs_o->fd, a_offset, SEEK_SET))
  {
    /* Error seeking. */
    retval = TRUE;
    if (_cw_fmatch(_STASH_DBG_R_BRBS_ERROR))
    {
      log_leprintf(g_log_o, __FILE__, __LINE__, "brbs_block_write",
		   "lseek() error: %s\n", strerror(errno));
    }
  }
  else
  {
    error = write(a_brbs_o->fd,
		  (void *) brblk_get_buf_p(a_brblk_o),
		  brblk_get_buf_size(a_brblk_o));
    if (error == -1)
    {
      /* Error writing. */
      retval = TRUE;
      if (_cw_fmatch(_STASH_DBG_R_BRBS_ERROR))
      {
	log_leprintf(g_log_o, __FILE__, __LINE__, "brbs_block_write",
		     "write() error: %s\n", strerror(errno));
      }
    }
    else if (error != brblk_get_buf_size(a_brblk_o))
    {
      /* Partial write error. */
      retval = TRUE;
      if (_cw_fmatch(_STASH_DBG_R_BRBS_ERROR))
      {
	log_leprintf(g_log_o, __FILE__, __LINE__, "brbs_block_write",
		     "Incomplete write");
      }
    }
    else
    {
      /* Success. */
      retval = FALSE;
    }
  }

  rwl_wunlock(&a_brbs_o->rw_lock);
  if (_cw_pmatch(_STASH_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_block_write()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Determines the sector size of a device and sets a_brbs_o->sect_size
 * accordingly.  Also finds out the size in bytes of the device and sets
 * a_brbs_o->size accordingly.
 *
 ****************************************************************************/
cw_bool_t
brbs_p_get_raw_info(cw_brbs_t * a_brbs_o)
{
#ifdef _ARCH_FREEBSD
  cw_bool_t retval;
  struct disklabel dlp;
  
  if (_cw_pmatch(_STASH_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_p_get_raw_info()");
  }
  
  if (ioctl(a_brbs_o->fd, DIOCGDINFO, &dlp) < 0)
  {
    retval = TRUE;
    
    if (_cw_fmatch(_STASH_DBG_R_BRBS_ERROR))
    {
      log_leprintf(g_log_o, __FILE__, __LINE__, "brbs_p_get_raw_info",
		   "ioctl() error: %s\n", strerror(errno));
    }
  }
  else
  {
    cw_uint32_t i, slice = '\0';

    retval = FALSE;

    /* Figure out which slice we're in. */
    for (i = strlen(a_brbs_o->filename) - 1;
	 i > 0;
	 i--)
    {
      if ((a_brbs_o->filename[i] >= 'a') && (a_brbs_o->filename[i] <= 'h'))
      {
	slice = a_brbs_o->filename[i] - 'a';
	break;
      }
    }
    _cw_assert(slice != '\0');
  
    a_brbs_o->sect_size = dlp.d_secsize;
    a_brbs_o->size = dlp.d_partitions[slice].p_size * dlp.d_secsize;
  }

  _cw_assert(a_brbs_o->sect_size > 0);
  if (_cw_pmatch(_STASH_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_p_get_raw_info()");
  }
  return retval;
#else
  return TRUE;
#endif
}

/****************************************************************************
 * <<< Description >>>
 *
 * Determines whether a file is a "character special" device (raw) and sets 
 * a_brbs_o->is_raw accordingly.
 *
 ****************************************************************************/
cw_bool_t
brbs_p_get_is_raw(cw_brbs_t * a_brbs_o)
{
  cw_bool_t retval = FALSE;
  struct stat sb;

  if (_cw_pmatch(_STASH_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_p_get_is_raw()");
  }
  if (-1 == fstat(a_brbs_o->fd, &sb))
  {
    retval = TRUE;
    if (_cw_fmatch(_STASH_DBG_R_BRBS_ERROR))
    {
      log_leprintf(g_log_o, __FILE__, __LINE__, "brbs_p_get_is_raw",
		   "fstat() error: %s\n", strerror(errno));
    }
  }
  else
  {
    if (S_ISCHR(sb.st_mode))
    {
      a_brbs_o->is_raw = TRUE;
    }
    else
    {
      a_brbs_o->is_raw = FALSE;
    }
  }
  if (_cw_pmatch(_STASH_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_p_get_is_raw()");
  }
  return retval;
}
