/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 120 $
 * $Date: 1998-07-01 17:22:16 -0700 (Wed, 01 Jul 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#ifndef _BRBS_H_
#define _BRBS_H_

/* Pseudo-opaque type. */
typedef struct cw_brbs_s cw_brbs_t;

struct cw_brbs_s
{
  cw_bool_t is_malloced;
  cw_rwl_t rw_lock;

  cw_bool_t is_open;
  char * filename;
  int fd;

  cw_uint64_t size; /* Size in bytes. */

  cw_bool_t is_raw; /* Is this a raw device? */
  cw_uint32_t sect_size;
};

/* Namespace definitions for brbs. */
#define brbs_new _CW_NS_CMN(brbs_new)
#define brbs_delete _CW_NS_CMN(brbs_delete)
#define brbs_is_open _CW_NS_CMN(brbs_is_open)
#define brbs_open _CW_NS_CMN(brbs_open)
#define brbs_close _CW_NS_CMN(brbs_close)
#define brbs_create _CW_NS_CMN(brbs_create)
#define brbs_destroy _CW_NS_CMN(brbs_destroy)
#define brbs_get_filename _CW_NS_CMN(brbs_get_filename)
#define brbs_set_filename _CW_NS_CMN(brbs_set_filename)
#define brbs_get_size _CW_NS_CMN(brbs_get_size)
#define brbs_get_is_raw _CW_NS_CMN(brbs_get_is_raw)
#define brbs_get_sect_size _CW_NS_CMN(brbs_get_sect_size)
#define brbs_block_read _CW_NS_CMN(brbs_block_read)
#define brbs_block_write _CW_NS_CMN(brbs_block_write)
/* #define brbs_ _CW_NS_CMN(brbs_) */

cw_brbs_t * brbs_new(cw_brbs_t * a_brbs_o);
void brbs_delete(cw_brbs_t * a_brbs_o);

cw_bool_t brbs_is_open(cw_brbs_t * a_brbs_o);

cw_bool_t brbs_open(cw_brbs_t * a_brbs_o);
cw_bool_t brbs_close(cw_brbs_t * a_brbs_o);
cw_bool_t brbs_create(cw_brbs_t * a_brbs, cw_uint64_t a_size);
cw_bool_t brbs_destroy(cw_brbs_t * a_brbs);

char * brbs_get_filename(cw_brbs_t * a_brbs_o);
cw_bool_t brbs_set_filename(cw_brbs_t * a_brbs_o, char * a_filename);

cw_uint64_t brbs_get_size(cw_brbs_t * a_brbs_o);
cw_bool_t brbs_get_is_raw(cw_brbs_t * a_brbs_o);
cw_uint32_t brbs_get_sect_size(cw_brbs_t * a_brbs_o);

cw_bool_t brbs_block_read(cw_brbs_t * a_brbs_o, cw_uint64_t a_offset,
			  cw_brblk_t * a_brblk_o);
cw_bool_t brbs_block_write(cw_brbs_t * a_brbs_o, cw_uint64_t a_offset,
			   cw_brblk_t * a_brblk_o);

#endif /* _BRBS_H_ */
