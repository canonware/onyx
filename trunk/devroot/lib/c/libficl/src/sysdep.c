/*******************************************************************
** s y s d e p . c
** Forth Inspired Command Language
** Author: John Sadler (john_sadler@alum.mit.edu)
** Created: 16 Oct 1997
** Implementations of FICL external interface functions... 
*******************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "libstash/libstash.h"
#include "libficl/libficl.h"

cw_bool_t g_lock_valid = FALSE;
cw_mtx_t g_dict_lock;

#if PORTABLE_LONGMULDIV == 0
DPUNS ficlLongMul(FICL_UNS x, FICL_UNS y)
{
    DPUNS q;
    u_int64_t qx;

    qx = (u_int64_t)x * (u_int64_t) y;

    q.hi = (u_int32_t)( qx >> 32 );
    q.lo = (u_int32_t)( qx & 0xFFFFFFFFL);

    return q;
}

UNSQR ficlLongDiv(DPUNS q, FICL_UNS y)
{
    UNSQR result;
    u_int64_t qx, qh;

    qh = q.hi;
    qx = (qh << 32) | q.lo;

    result.quot = qx / y;
    result.rem  = qx % y;

    return result;
}

#endif

void  ficlTextOut(FICL_VM *pVM, char *msg, int fNewline)
{
    if (fNewline)
        puts(msg);
    else
        fputs(msg, stdout);
}

void *ficlMalloc (size_t size)
{
  return _cw_malloc(size);
}

void  ficlFree   (void *p)
{
  _cw_free(p);
}

void *ficlRealloc(void *p, size_t size)
{
  return _cw_realloc(p, size);
}

void
ficlSysdepInit(void)
{
  if (g_lock_valid == FALSE)
  {
    mtx_new(&g_dict_lock);
  }
}

void
ficlSysdepTerm(void)
{
  if (g_lock_valid)
  {
    mtx_delete(&g_dict_lock);
    g_lock_valid = FALSE;
  }
}

/*
** Stub function for dictionary access control - does nothing
** by default, user can redefine to guarantee exclusive dict
** access to a single thread for updates. All dict update code
** is guaranteed to be bracketed as follows:
** ficlLockDictionary(TRUE);
** <code that updates dictionary>
** ficlLockDictionary(FALSE);
**
** Returns zero if successful, nonzero if unable to acquire lock
** before timeout (optional - could also block forever)
*/
#if FICL_MULTITHREAD
int ficlLockDictionary(short fLock)
{
  if (fLock)
  {
    /* Lock. */
    mtx_lock(&g_dict_lock);
  }
  else
  {
    /* Unlock. */
    mtx_unlock(&g_dict_lock);
  }
  
  return 0;
}
#endif /* FICL_MULTITHREAD */
