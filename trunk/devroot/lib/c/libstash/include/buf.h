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
 *
 *
 ****************************************************************************/

#ifndef _BUF_H_
#define _BUF_H_

typedef struct cw_buf_s cw_buf_t;
typedef struct cw_bufel_s cw_bufel_t;

struct cw_buf_s
{
  cw_bool_t is_malloced;
#ifdef _CW_REENTRANT
  cw_bool_t is_threadsafe;
  cw_mtx_t lock;
#endif
  cw_uint32_t size;
  cw_list_t bufels;
};

struct cw_bufel_s
{
  cw_bool_t is_malloced;
  cw_uint32_t buf_size;
  cw_uint32_t beg_offset;
  cw_uint32_t end_offset;
  cw_uint32_t * buf;
};

#define buf_new _CW_NS_ANY(buf_new)
#define buf_delete _CW_NS_ANY(buf_delete)
#define buf_get_size _CW_NS_ANY(buf_get_size)
#define buf_append_buf _CW_NS_ANY(buf_append_buf)
#define buf_rm_head_bufel _CW_NS_ANY(buf_rm_head_bufel)
#define buf_append_bufel _CW_NS_ANY(buf_append_bufel)

#define bufel_new _CW_NS_ANY(bufel_new)
#define bufel_delete _CW_NS_ANY(bufel_delete)
#define bufel_get_size _CW_NS_ANY(bufel_get_size)
#define bufel_set_size _CW_NS_ANY(bufel_set_size)
#define bufel_get_beg_offset _CW_NS_ANY(bufel_get_beg_offset)
#define bufel_set_beg_offset _CW_NS_ANY(bufel_set_beg_offset)
#define bufel_get_end_offset _CW_NS_ANY(bufel_get_end_offset)
#define bufel_set_end_offset _CW_NS_ANY(bufel_set_end_offset)
#define bufel_get_uint8 _CW_NS_ANY(bufel_get_uint8)
#define bufel_set_uint8 _CW_NS_ANY(bufel_set_uint8)
#define bufel_get_uint32 _CW_NS_ANY(bufel_get_uint32)
#define bufel_set_uint32 _CW_NS_ANY(bufel_set_uint32)

#ifdef _CW_REENTRANT
cw_buf_t * buf_new(cw_buf_t * a_buf_o, cw_bool_t a_is_threadsafe);
#else
cw_buf_t * buf_new(cw_buf_t * a_buf_o);
#endif
void buf_delete(cw_buf_t * a_buf_o);

cw_uint32_t buf_get_size(cw_buf_t * a_buf_o);

void buf_append_buf(cw_buf_t * a_a, cw_buf_t * a_b);

cw_bufel_t * buf_rm_head_bufel(cw_buf_t * a_buf_o);
void buf_append_bufel(cw_buf_t * a_buf_o, cw_bufel_t * a_bufel_o);

cw_bufel_t * bufel_new(cw_bufel_t * a_bufel_o);
void bufel_delete(cw_bufel_t * a_bufel_o);

cw_uint32_t bufel_get_size(cw_bufel_t * a_bufel_o);
cw_bool_t bufel_set_size(cw_bufel_t * a_bufel_o, cw_uint32_t a_size);

cw_uint32_t bufel_get_beg_offset(cw_bufel_t * a_bufel_o);
void bufel_set_beg_offset(cw_bufel_t * a_bufel_o, cw_uint32_t a_offset);
cw_uint32_t bufel_get_end_offset(cw_bufel_t * a_bufel_o);
void bufel_set_end_offset(cw_bufel_t * a_bufel_o, cw_uint32_t a_offset);

cw_uint8_t bufel_get_uint8(cw_bufel_t * a_bufel_o, cw_uint32_t a_offset);
void bufel_set_uint8(cw_bufel_t * a_bufel_o, cw_uint32_t a_offset,
		     cw_uint8_t a_val);
cw_uint32_t bufel_get_uint32(cw_bufel_t * a_bufel_o, cw_uint32_t a_offset);
void bufel_set_uint32(cw_bufel_t * a_bufel_o, cw_uint32_t a_offset,
		      cw_uint32_t a_val);

#endif /* _BUF_H_ */
