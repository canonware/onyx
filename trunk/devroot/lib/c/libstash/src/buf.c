/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#include "../include/libstash/libstash.h"

#ifdef _CW_OS_FREEBSD
#include <sys/types.h>
#include <sys/uio.h>
#endif

#ifdef _LIBSTASH_DBG
#define _CW_BUF_MAGIC 0xb00f0001
#define _CW_BUFEL_MAGIC 0xb00f0002
#define _CW_BUFC_MAGIC 0xb00f0003
#endif

static cw_buf_t	*buf_p_new(cw_buf_t *a_buf, cw_mem_t *a_mem, cw_bool_t
    a_is_threadsafe);
static void	buf_p_cumulative_index_rebuild(cw_buf_t *a_buf);
static void	buf_p_data_position_get(cw_buf_t *a_buf, cw_uint32_t a_offset,
    cw_uint32_t *a_array_element, cw_uint32_t *a_bufel_offset);
static cw_bool_t buf_p_array_fit(cw_buf_t *a_buf, cw_uint32_t a_min_array_size);
static cw_bool_t buf_p_buf_catenate(cw_buf_t *a_a, cw_buf_t *a_b, cw_bool_t
    a_preserve);
#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
static void	buf_p_array_copy(cw_buf_t *a_a, cw_buf_t *a_b, cw_uint32_t
    a_num_elements, cw_uint32_t a_a_start, cw_uint32_t a_b_start, cw_bool_t
    a_is_destructive);
#else
static void	buf_p_array_copy(cw_buf_t *a_a, cw_buf_t *a_b, cw_uint32_t
    a_num_elements, cw_uint32_t a_a_start, cw_uint32_t a_b_start);
#endif
static cw_bool_t buf_p_writeable_range_make(cw_buf_t *a_buf, cw_uint32_t
    a_offset, cw_uint32_t a_length);
static void	bufc_p_dump(cw_bufc_t *a_bufc, const char *a_prefix);
static cw_bool_t bufc_p_is_writeable(cw_bufc_t *a_bufc);
static cw_uint32_t bufc_p_ref_count_get(cw_bufc_t *a_bufc);
static void	bufc_p_ref_increment(cw_bufc_t *a_bufc);

cw_buf_t *
buf_new(cw_buf_t *a_buf, cw_mem_t *a_mem)
{
	return buf_p_new(a_buf, a_mem, FALSE);
}

cw_buf_t *
buf_new_r(cw_buf_t *a_buf, cw_mem_t *a_mem)
{
	return buf_p_new(a_buf, a_mem, TRUE);
}

void
buf_delete(cw_buf_t *a_buf)
{
	cw_uint32_t	i;

	_cw_check_ptr(a_buf);

	if (a_buf->is_threadsafe)
		mtx_delete(&a_buf->lock);
	for (i = 0; i < a_buf->array_num_valid; i++) {
		if (a_buf->bufel_array[(i + a_buf->array_start) &
		    (a_buf->array_size - 1)].bufc != NULL) {
			bufc_delete(a_buf->bufel_array[(i + a_buf->array_start)
			    & (a_buf->array_size - 1)].bufc);
		}
#ifdef _LIBSTASH_DBG
/*  		memset(&a_buf->bufel_array[(i + a_buf->array_start) */
/*  		    & (a_buf->array_size - 1)], */
/*  		    0x5a, */
/*  		    sizeof(cw_bufel_t)); */
		memset(&a_buf->bufel_array[(i + a_buf->array_start)
		    & (a_buf->array_size - 1)], 0, sizeof(cw_bufel_t));
#endif
	}

	if (a_buf->bufel_array != a_buf->static_bufel_array)
		_cw_mem_free(a_buf->mem, a_buf->bufel_array);
	if (a_buf->cumulative_index != a_buf->static_cumulative_index)
		_cw_mem_free(a_buf->mem, a_buf->cumulative_index);
	if (a_buf->iov != a_buf->static_iov)
		_cw_mem_free(a_buf->mem, a_buf->iov);
	if (a_buf->is_malloced)
		_cw_mem_free(a_buf->mem, a_buf);
}

void
buf_dump(cw_buf_t *a_buf, const char *a_prefix)
{
	cw_uint32_t	i;

	_cw_check_ptr(a_buf);
	_cw_check_ptr(a_prefix);

	if (a_buf->is_threadsafe)
		mtx_lock(&a_buf->lock);

	_cw_out_put("[s]| buf_dump()\n", a_prefix);
#ifdef _LIBSTASH_DBG
	_cw_out_put("[s]|--> magic_a : 0x[i|b:16]\n", a_prefix, a_buf->magic_a);
	_cw_out_put("[s]|--> magic_b : 0x[i|b:16]\n", a_prefix, a_buf->magic_b);
	_cw_out_put("[s]|--> size_of : [i]\n", a_prefix, a_buf->size_of);
#endif
	_cw_out_put("[s]|--> is_malloced : [s]\n", a_prefix,
	    (a_buf->is_malloced) ? "TRUE" : "FALSE");
	_cw_out_put("[s]|--> is_threadsafe : [s]\n", a_prefix,
	    (a_buf->is_threadsafe) ? "TRUE" : "FALSE");
	_cw_out_put("[s]|--> size : [i]\n", a_prefix, a_buf->size);
	_cw_out_put("[s]|--> array_size : [i]\n", a_prefix, a_buf->array_size);
	_cw_out_put("[s]|--> array_num_valid : [i]\n", a_prefix,
	    a_buf->array_num_valid);
	_cw_out_put("[s]|--> array_start : [i]\n", a_prefix,
	    a_buf->array_start);
	_cw_out_put("[s]|--> array_end : [i]\n", a_prefix, a_buf->array_end);
	_cw_out_put("[s]|--> is_cumulative_valid : [s]\n",
	    a_prefix, (a_buf->is_cumulative_valid) ? "TRUE" : "FALSE");
	_cw_out_put("[s]|--> is_cached_bufel_valid : [s]\n", a_prefix,
	    (a_buf->is_cached_bufel_valid) ? "TRUE" : "FALSE");
	_cw_out_put("[s]|--> cached_bufel : [i]\n", a_prefix,
	    a_buf->cached_bufel);

	for (i = 0; i < a_buf->array_size; i++) {
		_cw_out_put("[s]|\\\n"
		    "[s]| |--> cumulative_index[[[i]] : [i]\n"
		    "[s]| |--> bufel_array[[[i]] : \n"
		    "[s]|  \\\n",
		    a_prefix, a_prefix, i, a_buf->cumulative_index[i], a_prefix,
		    i, a_prefix);

		/* Dump bufel. */
#ifdef _LIBSTASH_DBG
		_cw_out_put("[s]|   |--> magic_a : 0x[i|b:16]\n", a_prefix,
		    a_buf->bufel_array[i].magic_a);
		_cw_out_put("[s]|   |--> magic_b : 0x[i|b:16]\n", a_prefix,
		    a_buf->bufel_array[i].magic_b);
		_cw_out_put("[s]|   |--> size_of : [i]\n", a_prefix,
		    a_buf->bufel_array[i].size_of);
#endif
		_cw_out_put("[s]|   |--> beg_offset : [i]\n", a_prefix,
		    a_buf->bufel_array[i].beg_offset);
		_cw_out_put("[s]|   |--> end_offset : [i]\n", a_prefix,
		    a_buf->bufel_array[i].end_offset);
#ifdef _LIBSTASH_DBG
		if ((a_buf->bufel_array[i].bufc != NULL) &&
		    (a_buf->bufel_array[i].magic_a == _CW_BUFEL_MAGIC) &&
		    (a_buf->bufel_array[i].size_of == sizeof(cw_bufel_t)) &&
		    (a_buf->bufel_array[i].magic_b == _CW_BUFEL_MAGIC))
#else
		if (a_buf->bufel_array[i].bufc != NULL)
#endif
		{
			char	*sub_prefix;

			_cw_out_put("[s]|   |--> bufc : 0x[i|b:16]\n"
			    "[s]|    \\\n", a_prefix,
			    a_buf->bufel_array[i].bufc, a_prefix);

			sub_prefix = _cw_mem_malloc(a_buf->mem, strlen(a_prefix)
			    + 7);
			if (sub_prefix == NULL)
				bufc_p_dump(a_buf->bufel_array[i].bufc, "...");
			else {
				_cw_out_put_s(sub_prefix, "[s]|     ",
				    a_prefix);
				bufc_p_dump(a_buf->bufel_array[i].bufc,
				    sub_prefix);
				_cw_mem_free(a_buf->mem, sub_prefix);
			}
		} else {
			_cw_out_put("[s]|   \\--> bufc : 0x[i|b:16]"
			    " (invalid)\n", a_prefix,
			    a_buf->bufel_array[i].bufc);
		}
	}

	if (a_buf->is_threadsafe)
		mtx_unlock(&a_buf->lock);
}

cw_uint32_t
buf_size_get(cw_buf_t *a_buf)
{
	cw_uint32_t	retval;

	_cw_check_ptr(a_buf);
	_cw_assert(a_buf->magic_a == _CW_BUF_MAGIC);
	_cw_assert(a_buf->size_of == sizeof(cw_buf_t));
	_cw_assert(a_buf->magic_b == _CW_BUF_MAGIC);

	if (a_buf->is_threadsafe)
		mtx_lock(&a_buf->lock);
	retval = a_buf->size;

	if (a_buf->is_threadsafe)
		mtx_unlock(&a_buf->lock);
	return retval;
}

cw_uint32_t
buf_num_bufels_get(cw_buf_t *a_buf)
{
	cw_uint32_t retval;

	_cw_check_ptr(a_buf);
	_cw_assert(a_buf->magic_a == _CW_BUF_MAGIC);
	_cw_assert(a_buf->size_of == sizeof(cw_buf_t));
	_cw_assert(a_buf->magic_b == _CW_BUF_MAGIC);

	if (a_buf->is_threadsafe)
		mtx_lock(&a_buf->lock);

	retval = a_buf->array_num_valid;

	if (a_buf->is_threadsafe)
		mtx_unlock(&a_buf->lock);
	return retval;
}

const struct iovec *
buf_iovec_get(cw_buf_t *a_buf, cw_uint32_t a_max_data, cw_bool_t a_is_sys_iovec,
    int *r_iovec_count)
{
	cw_uint32_t	array_index, num_bytes;
	int		i;

	_cw_check_ptr(a_buf);
	_cw_assert(a_buf->magic_a == _CW_BUF_MAGIC);
	_cw_assert(a_buf->size_of == sizeof(cw_buf_t));
	_cw_assert(a_buf->magic_b == _CW_BUF_MAGIC);
	if (a_buf->is_threadsafe) {
		mtx_lock(&a_buf->lock);
	}
	for (i = num_bytes = 0; (i < a_buf->array_num_valid) && (num_bytes <
	    a_max_data); i++) {
		array_index = (a_buf->array_start + i) & (a_buf->array_size -
		    1);

		a_buf->iov[i].iov_base = (((char
		    *)a_buf->bufel_array[array_index].bufc->buf) +
		    a_buf->bufel_array[array_index].beg_offset);
		a_buf->iov[i].iov_len = (size_t)
		    (a_buf->bufel_array[array_index].end_offset -
		    a_buf->bufel_array[array_index].beg_offset);

		num_bytes += a_buf->iov[i].iov_len;
	}

	/* Adjust the iovec size downward if necessary. */
	if (num_bytes > a_max_data)
		a_buf->iov[i - 1].iov_len -= (num_bytes - a_max_data);
	if (a_is_sys_iovec && (i > _CW_MAX_IOV))
		*r_iovec_count = _CW_MAX_IOV;
	else
		*r_iovec_count = i;

	if (a_buf->is_threadsafe)
		mtx_unlock(&a_buf->lock);
	return a_buf->iov;
}

cw_bool_t
buf_buf_catenate(cw_buf_t *a_a, cw_buf_t *a_b, cw_bool_t a_preserve)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_a);
	_cw_assert(a_a->magic_a == _CW_BUF_MAGIC);
	_cw_assert(a_a->size_of == sizeof(cw_buf_t));
	_cw_assert(a_a->magic_b == _CW_BUF_MAGIC);
	_cw_check_ptr(a_b);
	_cw_assert(a_b->magic_a == _CW_BUF_MAGIC);
	_cw_assert(a_b->size_of == sizeof(cw_buf_t));
	_cw_assert(a_b->magic_b == _CW_BUF_MAGIC);
	_cw_assert(a_a != a_b);

	if (a_a->is_threadsafe)
		mtx_lock(&a_a->lock);
	if (a_b->is_threadsafe)
		mtx_lock(&a_b->lock);

	retval = buf_p_buf_catenate(a_a, a_b, a_preserve);

	if (a_b->is_threadsafe)
		mtx_unlock(&a_b->lock);
	if (a_a->is_threadsafe)
		mtx_unlock(&a_a->lock);
	return retval;
}

cw_bool_t
buf_split(cw_buf_t *a_a, cw_buf_t *a_b, cw_uint32_t a_offset)
{
	cw_bool_t	retval;
	cw_uint32_t	array_element, bufel_offset, num_bufels_to_move;
	cw_uint32_t	i, a_a_index, a_b_index;

	_cw_check_ptr(a_a);
	_cw_assert(a_a->magic_a == _CW_BUF_MAGIC);
	_cw_assert(a_a->size_of == sizeof(cw_buf_t));
	_cw_assert(a_a->magic_b == _CW_BUF_MAGIC);
	_cw_check_ptr(a_b);
	_cw_assert(a_b->magic_a == _CW_BUF_MAGIC);
	_cw_assert(a_b->size_of == sizeof(cw_buf_t));
	_cw_assert(a_b->magic_b == _CW_BUF_MAGIC);
	_cw_assert(a_offset <= buf_size_get(a_b));
	_cw_assert(a_a != a_b);

	if (a_a->is_threadsafe)
		mtx_lock(&a_a->lock);
	if (a_b->is_threadsafe)
		mtx_lock(&a_b->lock);

	if ((a_offset > 0) && (a_offset < a_b->size)) {
		buf_p_data_position_get(a_b, a_offset, &array_element,
		    &bufel_offset);

		num_bufels_to_move = (((array_element >= a_b->array_start) ?
		    array_element - a_b->array_start : (array_element +
		    a_b->array_size - a_b->array_start)));
		if (bufel_offset != 0)
			num_bufels_to_move++;
		/*
		 * Make sure that a_a's array is big enough.  Doing this here
		 * instead of after trying to do a bufel merge means that we
		 * might be expanding the arrays unnecessarily.  However, error
		 * recovery after the bufel merge is particularly nasty.
		 */
		if (buf_p_array_fit(a_a, a_a->array_num_valid +
		    num_bufels_to_move)) {
			retval = TRUE;
			goto RETURN;
		}
		/* Try to merge first bufel of a_b and last bufel of a_a. */
		if ((num_bufels_to_move > 1) && (a_a->array_num_valid > 0)) {
			cw_uint32_t	last_element_index;

			last_element_index = ((a_a->array_end + a_a->array_size
			    - 1) & (a_a->array_size - 1));

			if ((a_a->bufel_array[last_element_index].bufc->buf !=
			    NULL) &&
			    (a_a->bufel_array[last_element_index].bufc->buf ==
			    a_b->bufel_array[a_b->array_start].bufc->buf) &&
			    (a_a->bufel_array[last_element_index].end_offset ==
			    a_b->bufel_array[a_b->array_start].beg_offset)) {
				/*
				 * These two bufel's reference the same bufc,
				 * and the buffer regions they refer to are
				 * consecutive and adjacent.  Merge.
				 */
				a_a->bufel_array[last_element_index].end_offset
				    = (a_a->bufel_array[last_element_index].end_offset
				    + (a_b->bufel_array[a_b->array_start].end_offset
				    - a_b->bufel_array[a_b->array_start].beg_offset));
				

				a_a->size +=
				    (a_b->bufel_array[a_b->array_start].end_offset
				    - a_b->bufel_array[a_b->array_start].beg_offset);
				    a_a->cumulative_index[last_element_index]
				    = a_a->size;

				num_bufels_to_move--;

				/*
				 * Need to decrement the bufc's reference count.
				 */
				bufc_delete(a_b->bufel_array[a_b->array_start].bufc);

#ifdef _LIBSTASH_DBG
				memset(&a_b->bufel_array[a_b->array_start], 0,
				    sizeof(cw_bufel_t));
				memset(&a_b->cumulative_index[a_b->array_start],
				    0, sizeof(cw_uint32_t));
#endif
				a_b->array_start = (a_b->array_start + 1) &
				    (a_b->array_size - 1);
				a_b->array_num_valid--;
			}
		} else if ((num_bufels_to_move == 1) && (a_a->array_num_valid >
		    0)) {
			cw_uint32_t	last_element_index;

			last_element_index = ((a_a->array_end +
			    a_a->array_size - 1) & (a_a->array_size - 1));

			if ((a_a->bufel_array[last_element_index].bufc->buf !=
			    NULL) &&
			    (a_a->bufel_array[last_element_index].bufc->buf ==
			    a_b->bufel_array[a_b->array_start].bufc->buf) &&
			    (a_a->bufel_array[last_element_index].end_offset ==
			    a_b->bufel_array[a_b->array_start].beg_offset)) {
				/*
				 * These two bufel's reference the same bufc,
				 * and the buffer regions they refer to are
				 * consecutive and adjacent.  Merge a_b into
				 * a_a.
				 */
				a_a->bufel_array[last_element_index].end_offset
				    = (a_a->bufel_array[last_element_index].end_offset
				    + (a_b->bufel_array[a_b->array_start].end_offset
				    - a_b->bufel_array[a_b->array_start].beg_offset));

				a_a->size += a_offset;
				a_a->cumulative_index[last_element_index] =
				    a_a->size;

				num_bufels_to_move--;

				if ((a_b->bufel_array[a_b->array_start].end_offset
				    - a_b->bufel_array[a_b->array_start].beg_offset)
				    == a_offset) {
					/*
					 * Need to decrement the bufc's
					 * reference count.
					 */
					bufc_delete(a_b->bufel_array[a_b->array_start].bufc);
#ifdef _LIBSTASH_DBG
					memset(&a_b->bufel_array[a_b->array_start],
					    0, sizeof(cw_bufel_t));
					memset(&a_b->cumulative_index[a_b->array_start],
					    0, sizeof(cw_uint32_t));
#endif
					a_b->array_start = (a_b->array_start +
					    1) & (a_b->array_size - 1);
					a_b->array_num_valid--;
				} else {
					a_b->bufel_array[a_b->array_start].beg_offset
					    = a_b->bufel_array[a_b->array_start].beg_offset
					    + a_offset;
					a_a->bufel_array[last_element_index].end_offset
					    = (a_a->bufel_array[last_element_index].end_offset
					    - (a_b->bufel_array[a_b->array_start].end_offset
						- a_b->bufel_array[a_b->array_start].beg_offset));
				}
			}
		}
		if (num_bufels_to_move > 0) {
#ifdef _LIBSTASH_DBG
			/*
			 * Non-destructively copy all the bufel's we care about.
			 */
			buf_p_array_copy(a_a, a_b, num_bufels_to_move,
			    a_a->array_end, a_b->array_start, FALSE);
			/*
			 * Destructively copy all but perhaps the last bufel, in
			 * order to zero out a_b's copy.
			 */
			buf_p_array_copy(a_a, a_b, num_bufels_to_move -
			    (bufel_offset == 0 ? 0 : 1), a_a->array_end,
			    a_b->array_start, TRUE);
#else
			buf_p_array_copy(a_a, a_b, num_bufels_to_move,
			    a_a->array_end, a_b->array_start);
#endif

			/*
			 * Iterate through the bufel's in a_b and move them to
			 * a_a, up to and including the bufel where the split
			 * occurs.
			 */
			for (i = 0, a_a_index = a_a->array_end, a_b_index =
			    a_b->array_start; i < num_bufels_to_move; i++) {
				a_a_index = (i + a_a->array_end) &
				    (a_a->array_size - 1);
				a_b_index = (i + a_b->array_start) &
				    (a_b->array_size - 1);

				a_a->size +=
				    (a_a->bufel_array[a_a_index].end_offset -
				    a_a->bufel_array[a_a_index].beg_offset);
				a_a->cumulative_index[a_a_index] = a_a->size;
			}

			/* Deal with the bufel that the split is in. */
			if (bufel_offset != 0) {
#ifdef _LIBSTASH_DBG
				/*
				 * Copy the bufel back to a_b, since the data
				 * are split and the original bufel must still
				 * remain valid.
				 */
				memcpy(&a_b->bufel_array[a_b_index],
				    &a_a->bufel_array[a_a_index],
				    sizeof(cw_bufel_t));
#endif
				/*
				 * Decrement a_a->size, since we don't want the
				 * whole bufc.
				 */
				a_a->size -=
				    (a_a->bufel_array[a_a_index].end_offset -
				    a_a->bufel_array[a_a_index].beg_offset);

				/*
				 * Increment the reference count for the buffer,
				 * and set the offsets appropriately for both
				 * bufel's.
				 */
				bufc_p_ref_increment(a_a->bufel_array[a_a_index].bufc);

				a_a->bufel_array[a_a_index].end_offset =
				    bufel_offset;
				a_b->bufel_array[a_b_index].beg_offset =
				    bufel_offset;

				a_a->size +=
				    (a_a->bufel_array[a_a_index].end_offset -
				    a_a->bufel_array[a_a_index].beg_offset);
				a_a->cumulative_index[a_a_index] = a_a->size;
			}
			/* Make a_a's and a_b's states consistent. */
			a_a->array_num_valid += num_bufels_to_move;
			a_a->array_end = (a_a_index + 1) & (a_a->array_size -
			    1);

			a_b->array_num_valid -= num_bufels_to_move;
			if (bufel_offset != 0) {
				a_b->array_num_valid++;
				a_b->array_start = a_b_index;
			} else
				a_b->array_start = (a_b_index + 1) &
				     (a_b->array_size - 1);
		}
		a_b->size -= a_offset;
		a_b->is_cumulative_valid = FALSE;
		a_b->is_cached_bufel_valid = FALSE;

		if ((a_b->array_num_valid == 0) && (a_b->array_size !=
		    _LIBSTASH_BUF_ARRAY_MIN_SIZE)) {
/*  			a_b->array_start = 0; */
/*  			a_b->array_end = 0; */
			a_b->is_cumulative_valid = TRUE;
			a_b->is_cached_bufel_valid = FALSE;
/*  			a_b->cached_bufel = 0; */
		}
	} else if ((a_offset > 0) && (a_offset == a_b->size)) {
		/* Same as catenation. */
		if (buf_p_buf_catenate(a_a, a_b, FALSE)) {
			retval = TRUE;
			goto RETURN;
		}
	}

	retval = FALSE;
	RETURN:
	if (a_b->is_threadsafe)
		mtx_unlock(&a_b->lock);
	if (a_a->is_threadsafe)
		mtx_unlock(&a_a->lock);
	return retval;
}

cw_bool_t
buf_bufc_prepend(cw_buf_t *a_buf, cw_bufc_t *a_bufc, cw_uint32_t a_beg_offset,
    cw_uint32_t a_end_offset)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_buf);
	_cw_assert(a_buf->magic_a == _CW_BUF_MAGIC);
	_cw_assert(a_buf->size_of == sizeof(cw_buf_t));
	_cw_assert(a_buf->magic_b == _CW_BUF_MAGIC);
	_cw_check_ptr(a_bufc);
	_cw_assert(a_bufc->magic_a == _CW_BUFC_MAGIC);
	_cw_assert(a_bufc->size_of == sizeof(cw_bufc_t));
	_cw_assert(a_bufc->magic_b == _CW_BUFC_MAGIC);
	_cw_assert(a_end_offset <= a_bufc->buf_size);
	_cw_assert(a_beg_offset <= a_end_offset);
	if (a_buf->is_threadsafe)
		mtx_lock(&a_buf->lock);

	if (a_bufc->buf != NULL) {
		/* Try to merge a_bufc into the first bufel in a_buf. */
		if ((a_buf->array_num_valid > 0) &&
		    (a_buf->bufel_array[a_buf->array_start].bufc->buf ==
		    a_bufc->buf) &&
		    (a_buf->bufel_array[a_buf->array_start].end_offset ==
		    a_beg_offset)) {
			/*
			 * Two references to the same bufc, and the buffer
			 * regions they refer to are consecutive and adjacent.
			 * Merge.
			 */
			a_buf->bufel_array[a_buf->array_start].end_offset =
			    (a_buf->bufel_array[a_buf->array_start].end_offset
			    + (a_end_offset - a_beg_offset));

			a_buf->size += (a_end_offset - a_beg_offset);

			a_buf->cumulative_index[a_buf->array_start] =
			    a_buf->size;
		} else {
			if (buf_p_array_fit(a_buf, a_buf->array_num_valid +
			    1)) {
				retval = TRUE;
				goto RETURN;
			}
			/* Now prepend the bufel. */
			a_buf->array_start = (((a_buf->array_start +
			    a_buf->array_size) - 1) & (a_buf->array_size - 1));
			a_buf->array_num_valid++;

#ifdef _LIBSTASH_DBG
			a_buf->bufel_array[a_buf->array_start].magic_a =
			    _CW_BUFEL_MAGIC;
			a_buf->bufel_array[a_buf->array_start].size_of =
			    sizeof(cw_bufel_t);
			a_buf->bufel_array[a_buf->array_start].magic_b =
			    _CW_BUFEL_MAGIC;
#endif
			a_buf->bufel_array[a_buf->array_start].beg_offset =
			    a_beg_offset;
			a_buf->bufel_array[a_buf->array_start].end_offset =
			    a_end_offset;
			a_buf->bufel_array[a_buf->array_start].bufc = a_bufc;
			bufc_p_ref_increment(a_bufc);

			a_buf->size += (a_end_offset - a_beg_offset);
			a_buf->is_cumulative_valid = FALSE;
			a_buf->is_cached_bufel_valid = FALSE;
		}
	}
	retval = FALSE;

	RETURN:
	if (a_buf->is_threadsafe)
		mtx_unlock(&a_buf->lock);
	return retval;
}

cw_bool_t
buf_bufc_append(cw_buf_t *a_buf, cw_bufc_t *a_bufc, cw_uint32_t a_beg_offset,
    cw_uint32_t a_end_offset)
{
	cw_bool_t	retval;
	cw_bool_t	did_bufel_merge = FALSE;

	_cw_check_ptr(a_buf);
	_cw_assert(a_buf->magic_a == _CW_BUF_MAGIC);
	_cw_assert(a_buf->size_of == sizeof(cw_buf_t));
	_cw_assert(a_buf->magic_b == _CW_BUF_MAGIC);
	_cw_check_ptr(a_bufc);
	_cw_assert(a_bufc->magic_a == _CW_BUFC_MAGIC);
	_cw_assert(a_bufc->size_of == sizeof(cw_bufc_t));
	_cw_assert(a_bufc->magic_b == _CW_BUFC_MAGIC);
	_cw_assert(a_end_offset <= a_bufc->buf_size);
	_cw_assert(a_beg_offset <= a_end_offset);

	if (a_buf->is_threadsafe)
		mtx_lock(&a_buf->lock);

	if (a_bufc->buf != NULL) {
		if (a_buf->array_num_valid > 0) {
			cw_uint32_t	last_element_index;

			last_element_index = ((a_buf->array_end +
			    a_buf->array_size - 1) & (a_buf->array_size - 1));

			if ((a_buf->array_num_valid > 0) &&
			    (a_buf->bufel_array[last_element_index].bufc->buf ==
			    a_bufc->buf) &&
			    (a_buf->bufel_array[last_element_index].end_offset
			    == a_beg_offset)) {
				/*
				 * Two references to the same bufc, and the
				 * buffer regions they refer to are consecutive
				 * and adjacent.  Merge.
				 */
				a_buf->bufel_array[last_element_index].end_offset
				    = (a_buf->bufel_array[last_element_index].end_offset
				    + (a_end_offset - a_beg_offset));

				a_buf->size += (a_end_offset - a_beg_offset);

				a_buf->cumulative_index[last_element_index] =
				    a_buf->size;

				did_bufel_merge = TRUE;
			}
		}
		if (did_bufel_merge == FALSE) {
			if (buf_p_array_fit(a_buf, a_buf->array_num_valid +
			    1)) {
				retval = TRUE;
				goto RETURN;
			}
			/* Now append the bufel. */
#ifdef _LIBSTASH_DBG
			a_buf->bufel_array[a_buf->array_end].magic_a =
			    _CW_BUFEL_MAGIC;
			a_buf->bufel_array[a_buf->array_end].size_of =
			    sizeof(cw_bufel_t);
			a_buf->bufel_array[a_buf->array_end].magic_b =
			    _CW_BUFEL_MAGIC;
#endif
			a_buf->bufel_array[a_buf->array_end].beg_offset =
			    a_beg_offset;
			a_buf->bufel_array[a_buf->array_end].end_offset =
			    a_end_offset;
			a_buf->bufel_array[a_buf->array_end].bufc = a_bufc;
			bufc_p_ref_increment(a_bufc);

			a_buf->array_num_valid++;
			a_buf->size += (a_end_offset - a_beg_offset);
			a_buf->cumulative_index[a_buf->array_end] =
			    a_buf->size;

			a_buf->array_end = ((a_buf->array_end + 1) &
			    (a_buf->array_size - 1));
		}
	}
	retval = FALSE;

	RETURN:
	if (a_buf->is_threadsafe)
		mtx_unlock(&a_buf->lock);
	return retval;
}

cw_bool_t
buf_head_data_release(cw_buf_t *a_buf, cw_uint32_t a_amount)
{
	cw_bool_t	retval;
	cw_uint32_t	array_index, bufel_valid_data, amount_left;

	_cw_check_ptr(a_buf);
	_cw_assert(a_buf->magic_a == _CW_BUF_MAGIC);
	_cw_assert(a_buf->size_of == sizeof(cw_buf_t));
	_cw_assert(a_buf->magic_b == _CW_BUF_MAGIC);

	if (a_buf->is_threadsafe)
		mtx_lock(&a_buf->lock);

	if (a_amount == 0)
		retval = FALSE;
	else if (a_amount > a_buf->size)
		retval = TRUE;
	else {
		for (array_index = a_buf->array_start, amount_left = a_amount;
		    amount_left > 0; array_index = (array_index + 1) &
		    (a_buf->array_size - 1)) {
			bufel_valid_data =
			    (a_buf->bufel_array[array_index].end_offset -
			    a_buf->bufel_array[array_index].beg_offset);

			if (bufel_valid_data <= amount_left) {
				/* Need to get rid of the bufel. */
				if (a_buf->bufel_array[array_index].bufc !=
				    NULL)
					bufc_delete(a_buf->bufel_array[array_index].bufc);
#ifdef _LIBSTASH_DBG
				memset(&a_buf->bufel_array[array_index], 0,
				    sizeof(cw_bufel_t));
#endif

				a_buf->array_start = (array_index + 1) &
				    (a_buf->array_size - 1);
				a_buf->array_num_valid--;
				amount_left -= bufel_valid_data;
			} else {/* if (bufel_valid_data > amount_left) */
				/* This will finish things up. */
				a_buf->bufel_array[array_index].beg_offset =
				    a_buf->bufel_array[array_index].beg_offset
				    + amount_left;
				amount_left = 0;
			}
		}

		/* Adjust the buf size. */
		a_buf->size -= a_amount;

		if ((a_buf->array_num_valid == 0) && (a_buf->array_size !=
		    _LIBSTASH_BUF_ARRAY_MIN_SIZE)) {
/*  			a_buf->array_start = 0; */
/*  			a_buf->array_end = 0; */
			a_buf->is_cumulative_valid = TRUE;
/*  			a_buf->cached_bufel = 0; */
	} else
			a_buf->is_cumulative_valid = FALSE;

		a_buf->is_cached_bufel_valid = FALSE;

		retval = FALSE;
	}

	if (a_buf->is_threadsafe)
		mtx_unlock(&a_buf->lock);
	return retval;
}

cw_bool_t
buf_tail_data_release(cw_buf_t *a_buf, cw_uint32_t a_amount)
{
	cw_bool_t	retval;
	cw_uint32_t	array_index, bufel_valid_data, amount_left;

	_cw_check_ptr(a_buf);
	_cw_assert(a_buf->magic_a == _CW_BUF_MAGIC);
	_cw_assert(a_buf->size_of == sizeof(cw_buf_t));
	_cw_assert(a_buf->magic_b == _CW_BUF_MAGIC);
	if (a_buf->is_threadsafe)
		mtx_lock(&a_buf->lock);

	if (a_amount == 0)
		retval = FALSE;
	else if (a_amount > a_buf->size)
		retval = TRUE;
	else {
		for (array_index = (a_buf->array_end + a_buf->array_size - 1) &
		    (a_buf->array_size - 1), amount_left = a_amount;
		    amount_left > 0; array_index = (array_index +
		    a_buf->array_size - 1) & (a_buf->array_size - 1)) {
			bufel_valid_data =
			    (a_buf->bufel_array[array_index].end_offset -
			    a_buf->bufel_array[array_index].beg_offset);

			if (bufel_valid_data <= amount_left) {
				/* Need to get rid of the bufel. */
				if (a_buf->bufel_array[array_index].bufc !=
				    NULL)
					bufc_delete(a_buf->bufel_array[array_index].bufc);
#ifdef _LIBSTASH_DBG
				memset(&a_buf->bufel_array[array_index], 0,
				    sizeof(cw_bufel_t));
#endif

				a_buf->array_end = array_index;
				a_buf->array_num_valid--;
				amount_left -= bufel_valid_data;
			} else {/* if (bufel_valid_data > amount_left) */
				/* This will finish things up. */
				a_buf->bufel_array[array_index].end_offset =
				    (a_buf->bufel_array[array_index].end_offset
				    - amount_left);
				amount_left = 0;
			}
		}

		/* Adjust the buf size. */
		a_buf->size -= a_amount;

		if ((a_buf->array_num_valid == 0) && (a_buf->array_size !=
		    _LIBSTASH_BUF_ARRAY_MIN_SIZE)) {
/*  			a_buf->array_start = 0; */
/*  			a_buf->array_end = 0; */
			a_buf->is_cumulative_valid = TRUE;
			a_buf->is_cached_bufel_valid = FALSE;
/*  			a_buf->cached_bufel = 0; */
		}
		retval = FALSE;
	}

	if (a_buf->is_threadsafe)
		mtx_unlock(&a_buf->lock);
	return retval;
}

cw_uint8_t
buf_uint8_get(cw_buf_t *a_buf, cw_uint32_t a_offset)
{
	cw_uint8_t	retval;
	cw_uint32_t	array_element, bufel_offset;

	_cw_check_ptr(a_buf);
	_cw_assert(a_buf->magic_a == _CW_BUF_MAGIC);
	_cw_assert(a_buf->size_of == sizeof(cw_buf_t));
	_cw_assert(a_buf->magic_b == _CW_BUF_MAGIC);
	_cw_assert(a_offset < a_buf->size);

	if (a_buf->is_threadsafe)
		mtx_lock(&a_buf->lock);

	buf_p_data_position_get(a_buf, a_offset, &array_element, &bufel_offset);

	retval = *(a_buf->bufel_array[array_element].bufc->buf + bufel_offset);

	if (a_buf->is_threadsafe)
		mtx_unlock(&a_buf->lock);
	return retval;
}

cw_uint32_t
buf_uint32_get(cw_buf_t *a_buf, cw_uint32_t a_offset)
{
	cw_uint32_t	retval, array_element, bufel_offset;

	_cw_check_ptr(a_buf);
	_cw_assert(a_buf->magic_a == _CW_BUF_MAGIC);
	_cw_assert(a_buf->size_of == sizeof(cw_buf_t));
	_cw_assert(a_buf->magic_b == _CW_BUF_MAGIC);
	_cw_assert((a_offset + 3) < a_buf->size);

	if (a_buf->is_threadsafe)
		mtx_lock(&a_buf->lock);

	/*
	 * Prepare a byte for logical or into retval. o: Offset from
	 * bufel_offset. s: Number of bytes to left shift.
	 */
#define _LIBSTASH_BUF_OR_BYTE(o, s)					\
	((cw_uint32_t) *(a_buf->bufel_array[array_element].bufc->buf	\
	    + bufel_offset + (o)) << ((s) << 3))

	buf_p_data_position_get(a_buf, a_offset, &array_element, &bufel_offset);

	if (bufel_offset + 3 <
	    a_buf->bufel_array[array_element].end_offset) {
#ifdef WORDS_BIGENDIAN
		retval = _LIBSTASH_BUF_OR_BYTE(0, 3);
		retval |= _LIBSTASH_BUF_OR_BYTE(1, 2);
		retval |= _LIBSTASH_BUF_OR_BYTE(2, 1);
		retval |= _LIBSTASH_BUF_OR_BYTE(3, 0);
#else
		retval = _LIBSTASH_BUF_OR_BYTE(0, 0);
		retval |= _LIBSTASH_BUF_OR_BYTE(1, 1);
		retval |= _LIBSTASH_BUF_OR_BYTE(2, 2);
		retval |= _LIBSTASH_BUF_OR_BYTE(3, 3);
#endif
	} else {
#ifdef WORDS_BIGENDIAN
		retval = _LIBSTASH_BUF_OR_BYTE(0, 3);
		buf_p_data_position_get(a_buf, a_offset + 1, &array_element,
		    &bufel_offset);
		retval |= _LIBSTASH_BUF_OR_BYTE(0, 2);
		buf_p_data_position_get(a_buf, a_offset + 2, &array_element,
		    &bufel_offset);
		retval |= _LIBSTASH_BUF_OR_BYTE(0, 1);
		buf_p_data_position_get(a_buf, a_offset + 3, &array_element,
		    &bufel_offset);
		retval |= _LIBSTASH_BUF_OR_BYTE(0, 0);
#else
		retval = _LIBSTASH_BUF_OR_BYTE(0, 0);
		buf_p_data_position_get(a_buf, a_offset + 1, &array_element,
		    &bufel_offset);
		retval |= _LIBSTASH_BUF_OR_BYTE(0, 1);
		buf_p_data_position_get(a_buf, a_offset + 2, &array_element,
		    &bufel_offset);
		retval |= _LIBSTASH_BUF_OR_BYTE(0, 2);
		buf_p_data_position_get(a_buf, a_offset + 3, &array_element,
		    &bufel_offset);
		retval |= _LIBSTASH_BUF_OR_BYTE(0, 3);
#endif
	}
#undef _LIBSTASH_BUF_OR_BYTE

	if (a_buf->is_threadsafe)
		mtx_unlock(&a_buf->lock);
	return retval;
}

cw_uint64_t
buf_uint64_get(cw_buf_t *a_buf, cw_uint32_t a_offset)
{
	cw_uint64_t	retval;
	cw_uint32_t	array_element, bufel_offset;

	_cw_check_ptr(a_buf);
	_cw_assert(a_buf->magic_a == _CW_BUF_MAGIC);
	_cw_assert(a_buf->size_of == sizeof(cw_buf_t));
	_cw_assert(a_buf->magic_b == _CW_BUF_MAGIC);
	_cw_assert((a_offset + 7) < a_buf->size);

	if (a_buf->is_threadsafe)
		mtx_lock(&a_buf->lock);

	/*
	 * Prepare a byte for logical or into retval. o: Offset from
	 * bufel_offset. s: Number of bytes to left shift.
	 */
#define _LIBSTASH_BUF_OR_BYTE(o, s)					\
	((cw_uint64_t) *(a_buf->bufel_array[array_element].bufc->buf	\
	    + bufel_offset + (o)) << ((s) << 3))

	buf_p_data_position_get(a_buf, a_offset, &array_element, &bufel_offset);

	if (bufel_offset + 7 < a_buf->bufel_array[array_element].end_offset) {
#ifdef WORDS_BIGENDIAN
		retval = _LIBSTASH_BUF_OR_BYTE(0, 7);
		retval |= _LIBSTASH_BUF_OR_BYTE(1, 6);
		retval |= _LIBSTASH_BUF_OR_BYTE(2, 5);
		retval |= _LIBSTASH_BUF_OR_BYTE(3, 4);
		retval |= _LIBSTASH_BUF_OR_BYTE(4, 3);
		retval |= _LIBSTASH_BUF_OR_BYTE(5, 2);
		retval |= _LIBSTASH_BUF_OR_BYTE(6, 1);
		retval |= _LIBSTASH_BUF_OR_BYTE(7, 0);
#else
		retval = _LIBSTASH_BUF_OR_BYTE(0, 0);
		retval |= _LIBSTASH_BUF_OR_BYTE(1, 1);
		retval |= _LIBSTASH_BUF_OR_BYTE(2, 2);
		retval |= _LIBSTASH_BUF_OR_BYTE(3, 3);
		retval |= _LIBSTASH_BUF_OR_BYTE(4, 4);
		retval |= _LIBSTASH_BUF_OR_BYTE(5, 5);
		retval |= _LIBSTASH_BUF_OR_BYTE(6, 6);
		retval |= _LIBSTASH_BUF_OR_BYTE(7, 7);
#endif
	} else {
#ifdef WORDS_BIGENDIAN
		retval = _LIBSTASH_BUF_OR_BYTE(0, 7);
		buf_p_data_position_get(a_buf, a_offset + 1, &array_element,
		    &bufel_offset);
		retval |= _LIBSTASH_BUF_OR_BYTE(0, 6);
		buf_p_data_position_get(a_buf, a_offset + 2, &array_element,
		    &bufel_offset);
		retval |= _LIBSTASH_BUF_OR_BYTE(0, 5);
		buf_p_data_position_get(a_buf, a_offset + 3, &array_element,
		    &bufel_offset);
		retval |= _LIBSTASH_BUF_OR_BYTE(0, 4);
		buf_p_data_position_get(a_buf, a_offset + 4, &array_element,
		    &bufel_offset);
		retval |= _LIBSTASH_BUF_OR_BYTE(0, 3);
		buf_p_data_position_get(a_buf, a_offset + 5, &array_element,
		    &bufel_offset);
		retval |= _LIBSTASH_BUF_OR_BYTE(0, 2);
		buf_p_data_position_get(a_buf, a_offset + 6, &array_element,
		    &bufel_offset);
		retval |= _LIBSTASH_BUF_OR_BYTE(0, 1);
		buf_p_data_position_get(a_buf, a_offset + 7, &array_element,
		    &bufel_offset);
		retval |= _LIBSTASH_BUF_OR_BYTE(0, 0);
#else
		retval = _LIBSTASH_BUF_OR_BYTE(0, 0);
		buf_p_data_position_get(a_buf, a_offset + 1, &array_element,
		    &bufel_offset);
		retval |= _LIBSTASH_BUF_OR_BYTE(0, 1);
		buf_p_data_position_get(a_buf, a_offset + 2, &array_element,
		    &bufel_offset);
		retval |= _LIBSTASH_BUF_OR_BYTE(0, 2);
		buf_p_data_position_get(a_buf, a_offset + 3, &array_element,
		    &bufel_offset);
		retval |= _LIBSTASH_BUF_OR_BYTE(0, 3);
		buf_p_data_position_get(a_buf, a_offset + 4, &array_element,
		    &bufel_offset);
		retval |= _LIBSTASH_BUF_OR_BYTE(0, 4);
		buf_p_data_position_get(a_buf, a_offset + 5, &array_element,
		    &bufel_offset);
		retval |= _LIBSTASH_BUF_OR_BYTE(0, 5);
		buf_p_data_position_get(a_buf, a_offset + 6, &array_element,
		    &bufel_offset);
		retval |= _LIBSTASH_BUF_OR_BYTE(0, 6);
		buf_p_data_position_get(a_buf, a_offset + 7, &array_element,
		    &bufel_offset);
		retval |= _LIBSTASH_BUF_OR_BYTE(0, 7);
#endif
	}
#undef _LIBSTASH_BUF_OR_BYTE

	if (a_buf->is_threadsafe)
		mtx_unlock(&a_buf->lock);
	return retval;
}

cw_bool_t
buf_uint8_set(cw_buf_t *a_buf, cw_uint32_t a_offset, cw_uint8_t a_val)
{
	cw_bool_t	retval;
	cw_uint32_t	array_element, bufel_offset;

	_cw_check_ptr(a_buf);
	_cw_assert(a_buf->magic_a == _CW_BUF_MAGIC);
	_cw_assert(a_buf->size_of == sizeof(cw_buf_t));
	_cw_assert(a_buf->magic_b == _CW_BUF_MAGIC);
	_cw_assert(a_offset <= a_buf->size);

	if (a_buf->is_threadsafe)
		mtx_lock(&a_buf->lock);

	if (buf_p_writeable_range_make(a_buf, a_offset, sizeof(cw_uint8_t))) {
		retval = TRUE;
		goto RETURN;
	}
	buf_p_data_position_get(a_buf, a_offset, &array_element, &bufel_offset);

	a_buf->bufel_array[array_element].bufc->buf[bufel_offset] = a_val;

	retval = FALSE;
	RETURN:
	if (a_buf->is_threadsafe)
		mtx_unlock(&a_buf->lock);
	return retval;
}

cw_bool_t
buf_uint32_set(cw_buf_t *a_buf, cw_uint32_t a_offset, cw_uint32_t a_val)
{
	cw_bool_t	retval;
	cw_uint32_t	array_element, bufel_offset;

	_cw_check_ptr(a_buf);
	_cw_assert(a_buf->magic_a == _CW_BUF_MAGIC);
	_cw_assert(a_buf->size_of == sizeof(cw_buf_t));
	_cw_assert(a_buf->magic_b == _CW_BUF_MAGIC);
	_cw_assert(a_offset <= a_buf->size);

	if (a_buf->is_threadsafe)
		mtx_lock(&a_buf->lock);

	if (buf_p_writeable_range_make(a_buf, a_offset, sizeof(cw_uint32_t))) {
		retval = TRUE;
		goto RETURN;
	}
	/* Get a byte from a_val and put in in the least significant byte. */
#define _LIBSTASH_BUF_BYTE(s) ((a_val >> ((s) << 3)) & 0xff)

	buf_p_data_position_get(a_buf, a_offset, &array_element, &bufel_offset);

	if (bufel_offset + 3 < a_buf->bufel_array[array_element].end_offset) {
		/* The whole thing is in one bufel. */
#ifdef WORDS_BIGENDIAN
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
		    = _LIBSTASH_BUF_BYTE(3);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset + 1]
		    = _LIBSTASH_BUF_BYTE(2);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset + 2]
		    = _LIBSTASH_BUF_BYTE(1);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset + 3]
		    = _LIBSTASH_BUF_BYTE(0);
#else
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
		    = _LIBSTASH_BUF_BYTE(0);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset + 1]
		    = _LIBSTASH_BUF_BYTE(1);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset + 2]
		    = _LIBSTASH_BUF_BYTE(2);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset + 3]
		    = _LIBSTASH_BUF_BYTE(3);
#endif
	} else {
		/* Split across two or more bufels. */
#ifdef WORDS_BIGENDIAN
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
		    = _LIBSTASH_BUF_BYTE(3);
		buf_p_data_position_get(a_buf, a_offset + 1, &array_element,
		    &bufel_offset);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
		    = _LIBSTASH_BUF_BYTE(2);
		buf_p_data_position_get(a_buf, a_offset + 2, &array_element,
		    &bufel_offset);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
		    = _LIBSTASH_BUF_BYTE(1);
		buf_p_data_position_get(a_buf, a_offset + 3, &array_element,
		    &bufel_offset);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
		    = _LIBSTASH_BUF_BYTE(0);
#else
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
		    = _LIBSTASH_BUF_BYTE(0);
		buf_p_data_position_get(a_buf, a_offset + 1, &array_element,
		    &bufel_offset);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
		    = _LIBSTASH_BUF_BYTE(1);
		buf_p_data_position_get(a_buf, a_offset + 2, &array_element,
		    &bufel_offset);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
		    = _LIBSTASH_BUF_BYTE(2);
		buf_p_data_position_get(a_buf, a_offset + 3, &array_element,
		    &bufel_offset);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
		    = _LIBSTASH_BUF_BYTE(3);
#endif
	}
#undef _LIBSTASH_BUF_BYTE

	retval = FALSE;
	RETURN:
	if (a_buf->is_threadsafe)
		mtx_unlock(&a_buf->lock);
	return retval;
}

cw_bool_t
buf_uint64_set(cw_buf_t *a_buf, cw_uint32_t a_offset, cw_uint64_t a_val)
{
	cw_bool_t	retval;
	cw_uint32_t	array_element, bufel_offset;

	_cw_check_ptr(a_buf);
	_cw_assert(a_buf->magic_a == _CW_BUF_MAGIC);
	_cw_assert(a_buf->size_of == sizeof(cw_buf_t));
	_cw_assert(a_buf->magic_b == _CW_BUF_MAGIC);
	_cw_assert(a_offset <= a_buf->size);

	if (a_buf->is_threadsafe)
		mtx_lock(&a_buf->lock);

	if (buf_p_writeable_range_make(a_buf, a_offset, sizeof(cw_uint64_t))) {
		retval = TRUE;
		goto RETURN;
	}
	/* Get a byte from a_val and put in in the least significant byte. */
#define _LIBSTASH_BUF_BYTE(s) ((a_val >> ((s) << 3)) & 0xff)

	buf_p_data_position_get(a_buf, a_offset, &array_element, &bufel_offset);

	if (bufel_offset + 7 < a_buf->bufel_array[array_element].end_offset) {
		/* The whole thing is in one bufel. */
#ifdef WORDS_BIGENDIAN
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
		    = _LIBSTASH_BUF_BYTE(7);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset + 1]
		    = _LIBSTASH_BUF_BYTE(6);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset + 2]
		    = _LIBSTASH_BUF_BYTE(5);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset + 3]
		    = _LIBSTASH_BUF_BYTE(4);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset + 4]
		    = _LIBSTASH_BUF_BYTE(3);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset + 5]
		    = _LIBSTASH_BUF_BYTE(2);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset + 6]
		    = _LIBSTASH_BUF_BYTE(1);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset + 7]
		    = _LIBSTASH_BUF_BYTE(0);
#else
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
		    = _LIBSTASH_BUF_BYTE(0);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset + 1]
		    = _LIBSTASH_BUF_BYTE(1);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset + 2]
		    = _LIBSTASH_BUF_BYTE(2);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset + 3]
		    = _LIBSTASH_BUF_BYTE(3);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset + 4]
		    = _LIBSTASH_BUF_BYTE(4);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset + 5]
		    = _LIBSTASH_BUF_BYTE(5);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset + 6]
		    = _LIBSTASH_BUF_BYTE(6);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset + 7]
		    = _LIBSTASH_BUF_BYTE(7);
#endif
	} else {
		/* Split across two or more bufels. */
#ifdef WORDS_BIGENDIAN
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
		    = _LIBSTASH_BUF_BYTE(7);
		buf_p_data_position_get(a_buf, a_offset + 1, &array_element,
		    &bufel_offset);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
		    = _LIBSTASH_BUF_BYTE(6);
		buf_p_data_position_get(a_buf, a_offset + 2, &array_element,
		    &bufel_offset);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
		    = _LIBSTASH_BUF_BYTE(5);
		buf_p_data_position_get(a_buf, a_offset + 3, &array_element,
		    &bufel_offset);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
		    = _LIBSTASH_BUF_BYTE(4);
		buf_p_data_position_get(a_buf, a_offset + 4, &array_element,
		    &bufel_offset);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
		    = _LIBSTASH_BUF_BYTE(3);
		buf_p_data_position_get(a_buf, a_offset + 5, &array_element,
		    &bufel_offset);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
		    = _LIBSTASH_BUF_BYTE(2);
		buf_p_data_position_get(a_buf, a_offset + 6, &array_element,
		    &bufel_offset);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
		    = _LIBSTASH_BUF_BYTE(1);
		buf_p_data_position_get(a_buf, a_offset + 7, &array_element,
		    &bufel_offset);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
		    = _LIBSTASH_BUF_BYTE(0);
#else
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
		    = _LIBSTASH_BUF_BYTE(0);
		buf_p_data_position_get(a_buf, a_offset + 1, &array_element,
		    &bufel_offset);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
		    = _LIBSTASH_BUF_BYTE(1);
		buf_p_data_position_get(a_buf, a_offset + 2, &array_element,
		    &bufel_offset);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
		    = _LIBSTASH_BUF_BYTE(2);
		buf_p_data_position_get(a_buf, a_offset + 3, &array_element,
		    &bufel_offset);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
		    = _LIBSTASH_BUF_BYTE(3);
		buf_p_data_position_get(a_buf, a_offset + 4, &array_element,
		    &bufel_offset);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
		    = _LIBSTASH_BUF_BYTE(4);
		buf_p_data_position_get(a_buf, a_offset + 5, &array_element,
		    &bufel_offset);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
		    = _LIBSTASH_BUF_BYTE(5);
		buf_p_data_position_get(a_buf, a_offset + 6, &array_element,
		    &bufel_offset);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
		    = _LIBSTASH_BUF_BYTE(6);
		buf_p_data_position_get(a_buf, a_offset + 7, &array_element,
		    &bufel_offset);
		a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
		    = _LIBSTASH_BUF_BYTE(7);
#endif
	}
#undef _LIBSTASH_BUF_BYTE

	retval = FALSE;
	RETURN:
	if (a_buf->is_threadsafe)
		mtx_unlock(&a_buf->lock);
	return retval;
}

cw_bool_t
buf_range_set(cw_buf_t *a_buf, cw_uint32_t a_offset, cw_uint32_t a_length,
    cw_uint8_t *a_val, cw_bool_t a_is_writeable)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_buf);
	_cw_assert(a_buf->magic_a == _CW_BUF_MAGIC);
	_cw_assert(a_buf->size_of == sizeof(cw_buf_t));
	_cw_assert(a_buf->magic_b == _CW_BUF_MAGIC);
	_cw_assert(a_offset <= a_buf->size);

	if (a_buf->is_threadsafe)
		mtx_lock(&a_buf->lock);

	/*
	 * If a_val is writeable and it's being appended to the buf, tack it on
	 * to the end of the buf, rather than copying.
	 */
	if (a_is_writeable && (a_offset == a_buf->size)) {
		cw_bufc_t	*bufc;

		bufc = bufc_new(NULL, a_buf->mem, NULL, NULL);
		if (bufc == NULL) {
			retval = TRUE;
			goto RETURN;
		}
		bufc_buffer_set(bufc, (void *)a_val, a_length, TRUE, NULL,
		    NULL);

		if (buf_p_array_fit(a_buf, a_buf->array_num_valid + 1)) {
			bufc_delete(bufc);
			retval = TRUE;
			goto RETURN;
		}
		/* Initialize bufel. */
/*  		memset(&a_buf->bufel_array[a_buf->array_end], 0, */
/*  		    sizeof(cw_bufel_t)); */
#ifdef _LIBSTASH_DBG
		a_buf->bufel_array[a_buf->array_end].magic_a = _CW_BUFEL_MAGIC;
		a_buf->bufel_array[a_buf->array_end].size_of =
		    sizeof(cw_bufel_t);
		a_buf->bufel_array[a_buf->array_end].magic_b = _CW_BUFEL_MAGIC;
#endif
		a_buf->bufel_array[a_buf->array_end].beg_offset = 0;
		a_buf->bufel_array[a_buf->array_end].end_offset = a_length;
		a_buf->bufel_array[a_buf->array_end].bufc = bufc;

		a_buf->size += a_length;
		a_buf->array_num_valid++;
		/* Do this in case the cumulative index is valid. */
		a_buf->cumulative_index[a_buf->array_end] = a_buf->size;
		a_buf->array_end = ((a_buf->array_end + 1) &
		    (a_buf->array_size - 1));
	} else {
		cw_uint32_t	bytes_copied, array_element, bufel_offset;

		if (buf_p_writeable_range_make(a_buf, a_offset, a_length)) {
			retval = TRUE;
			goto RETURN;
		}
		/* March through the bufel_array and memcpy in a_val. */
		bytes_copied = 0;
		while (bytes_copied < a_length) {
			buf_p_data_position_get(a_buf, a_offset +
			    bytes_copied, &array_element, &bufel_offset);
			if (((a_buf->bufel_array[array_element].end_offset -
			    bufel_offset) + bytes_copied) > a_length) {
				/*
				 * There's more than enough room to finish up
				 * with the current bufel.
				 */
				memcpy(a_buf->bufel_array[array_element].bufc->buf
				    + bufel_offset, a_val + bytes_copied,
				    a_length - bytes_copied);
				bytes_copied = a_length;
			} else {
				/* Completely re-write the current bufel. */
				memcpy(a_buf->bufel_array[array_element].bufc->buf
				    + bufel_offset, a_val + bytes_copied,
				    (a_buf->bufel_array[array_element].end_offset
				    - bufel_offset));
				bytes_copied +=
				    (a_buf->bufel_array[array_element].end_offset
				    - bufel_offset);
			}
		}
	}

	retval = FALSE;
	RETURN:
	if (a_buf->is_threadsafe)
		mtx_unlock(&a_buf->lock);
	return retval;
}

static cw_buf_t *
buf_p_new(cw_buf_t *a_buf, cw_mem_t *a_mem, cw_bool_t a_is_threadsafe)
{
	cw_buf_t	*retval;

	if (a_buf == NULL) {
		retval = (cw_buf_t *)_cw_mem_malloc(a_mem, sizeof(cw_buf_t));
		if (retval == NULL)
			goto RETURN;
		retval->is_malloced = TRUE;
	} else {
		retval = a_buf;
		retval->is_malloced = FALSE;
	}

	retval->mem = a_mem;

#ifdef _LIBSTASH_DBG
	retval->magic_a = _CW_BUF_MAGIC;
	retval->size_of = sizeof(cw_buf_t);
	retval->magic_b = _CW_BUF_MAGIC;
#endif

	retval->is_threadsafe = a_is_threadsafe;
	if (retval->is_threadsafe)
		mtx_new(&retval->lock);
	retval->size = 0;

	retval->array_size = _LIBSTASH_BUF_ARRAY_MIN_SIZE;
	retval->array_num_valid = 0;
	retval->array_start = 0;
	retval->array_end = 0;
	retval->is_cumulative_valid = TRUE;
	retval->is_cached_bufel_valid = FALSE;
/*  	retval->cached_bufel = 0; */

	retval->bufel_array = retval->static_bufel_array;
	retval->cumulative_index = retval->static_cumulative_index;
	retval->iov = retval->static_iov;

#ifdef _LIBSTASH_DBG
	memset(retval->bufel_array, 0,
	    _LIBSTASH_BUF_ARRAY_MIN_SIZE * sizeof(cw_bufel_t));
	memset(retval->cumulative_index, 0,
	    _LIBSTASH_BUF_ARRAY_MIN_SIZE * sizeof(cw_uint32_t));
	memset(retval->iov, 0,
	    _LIBSTASH_BUF_ARRAY_MIN_SIZE * sizeof(struct iovec));
#endif

	RETURN:
	return retval;
}

static void
buf_p_cumulative_index_rebuild(cw_buf_t *a_buf)
{
	cw_uint32_t	i, cumulative, index;

	for (i = cumulative = 0; i < a_buf->array_num_valid; i++) {
		index = (i + a_buf->array_start) & (a_buf->array_size - 1);

		cumulative += (a_buf->bufel_array[index].end_offset -
		    a_buf->bufel_array[index].beg_offset);
		a_buf->cumulative_index[index] = cumulative;
	}
	a_buf->is_cumulative_valid = TRUE;
}

static void
buf_p_data_position_get(cw_buf_t *a_buf, cw_uint32_t a_offset, cw_uint32_t
    *a_array_element, cw_uint32_t *a_bufel_offset)
{
	if ((a_buf->is_cached_bufel_valid) && (a_offset <
	    a_buf->cumulative_index[a_buf->cached_bufel]) && (a_offset >=
	    (a_buf->cumulative_index[a_buf->cached_bufel] -
	    (a_buf->bufel_array[a_buf->cached_bufel].end_offset -
	    a_buf->bufel_array[a_buf->cached_bufel].beg_offset)))) {
		/* The cached data position info is valid and useful. */
		*a_array_element = a_buf->cached_bufel;
		*a_bufel_offset =
		    (a_buf->bufel_array[a_buf->cached_bufel].end_offset -
		    (a_buf->cumulative_index[a_buf->cached_bufel] - a_offset));
	} else {
		cw_uint32_t	first, last, index;

		if (a_buf->is_cumulative_valid == FALSE)
			buf_p_cumulative_index_rebuild(a_buf);
		if (a_buf->array_start < a_buf->array_end) {
			/*
			 * bufel_array is not wrapped, which means we can do a
			 * standard binary search without lots of modulus
			 * operations.
			 */
			first = a_buf->array_start;
			last = a_buf->array_end;
		} else if (a_buf->array_end == 0) {
			/*
			 * bufel_array is not wrapped, which means we can do a
			 * standard binary search without lots of modulus
			 * operations.
			 */
			first = a_buf->array_start;
			last = a_buf->array_size;
		} else if (a_buf->cumulative_index[a_buf->array_size - 1] >
		    a_offset) {
			/*
			 * bufel_array is wrapped, but the byte we want is not
			 * in the wrapped portion.
			 */
			first = a_buf->array_start;
			last = a_buf->array_size;
		} else {
			/*
			 * bufel_array is wrapped, and the byte we want is in
			 * the wrapped portion.
			 */
			first = 0;
			last = a_buf->array_end;
		}

		/*
		 * Binary search, where "first" is the index of the first
		 * element in the range to search, and "last" is one plus the
		 * index of the last element in the range to search.
		 */
		if (a_offset < a_buf->cumulative_index[first])
			index = first;
		else {
			for (;;) {
				index = (first + last) >> 1;

				if (a_buf->cumulative_index[index] <=
				    a_offset)
					first = index + 1;
				else if (a_buf->cumulative_index[index - 1] >
				    a_offset)
					last = index;
				else
					break;
			}
		}

		*a_array_element = index;
		*a_bufel_offset = (a_buf->bufel_array[index].end_offset -
		    (a_buf->cumulative_index[index] - a_offset));

		a_buf->is_cached_bufel_valid = TRUE;
		a_buf->cached_bufel = index;
	}
}

static cw_bool_t
buf_p_array_fit(cw_buf_t *a_buf, cw_uint32_t a_min_array_size)
{
	cw_bool_t	retval;
	cw_uint32_t	i;
	void		*t_ptr;

	/*
	 * Make sure a_buf's array is big enough.  Even if we're trying to merge
	 * bufel's, make the array big enough that it doesn't matter how
	 * successful the bufel merging is.
	 */
	if (a_min_array_size > a_buf->array_size) {
		/* Double i until it is big enough to accomodate our needs. */
		for (i = a_buf->array_size << 1; i < a_min_array_size; i <<= 1);

		if (a_buf->bufel_array != a_buf->static_bufel_array) {
			t_ptr = _cw_mem_realloc(a_buf->mem, a_buf->bufel_array,
			    i * sizeof(cw_bufel_t));
			if (t_ptr == NULL) {
				retval = TRUE;
				goto RETURN;
			}
		} else {
			t_ptr = _cw_mem_calloc(a_buf->mem, i,
			    sizeof(cw_bufel_t));
			if (t_ptr == NULL) {
				retval = TRUE;
				goto RETURN;
			}
			memcpy(t_ptr, a_buf->bufel_array,
			    (size_t) (a_buf->array_size * sizeof(cw_bufel_t)));
#ifdef _LIBSTASH_DBG
			memset(a_buf->bufel_array, 0x5a,
			    (a_buf->array_size * sizeof(cw_bufel_t)));
#endif
		}
		a_buf->bufel_array = (cw_bufel_t *)t_ptr;

		if (a_buf->cumulative_index != a_buf->static_cumulative_index) {
			t_ptr = _cw_mem_realloc(a_buf->mem,
			    a_buf->cumulative_index, i * sizeof(cw_uint32_t));
			if (t_ptr == NULL) {
				retval = TRUE;
				goto RETURN;
			}
		} else {
			t_ptr = _cw_mem_calloc(a_buf->mem, i,
			    sizeof(cw_uint32_t));
			if (t_ptr == NULL) {
				retval = TRUE;
				goto RETURN;
			}
			memcpy(t_ptr, a_buf->cumulative_index,
			    (a_buf->array_size * sizeof(cw_uint32_t)));
#ifdef _LIBSTASH_DBG
			memset(a_buf->cumulative_index, 0x5a,
			    (a_buf->array_size * sizeof(cw_uint32_t)));
#endif
		}
		a_buf->cumulative_index = (cw_uint32_t *)t_ptr;

		if (a_buf->iov != a_buf->static_iov) {
			t_ptr = _cw_mem_realloc(a_buf->mem, a_buf->iov, i *
			    sizeof(struct iovec));
			if (t_ptr == NULL) {
				retval = TRUE;
				goto RETURN;
			}
		} else {
			t_ptr = _cw_mem_calloc(a_buf->mem, i, sizeof(struct
			    iovec));
			if (t_ptr == NULL) {
				retval = TRUE;
				goto RETURN;
			}
#ifdef _LIBSTASH_DBG
			memset(a_buf->iov, 0x5a,
			    (a_buf->array_size * sizeof(struct iovec)));
#endif
		}
		a_buf->iov = (struct iovec *)t_ptr;

#ifdef _LIBSTASH_DBG
		memset(&a_buf->bufel_array[a_buf->array_size], 0,
		    ((i - a_buf->array_size) * sizeof(cw_bufel_t)));
		memset(&a_buf->cumulative_index[a_buf->array_size], 0,
		    ((i - a_buf->array_size) * sizeof(cw_uint32_t)));
		memset(&a_buf->iov[a_buf->array_size], 0,
		    ((i - a_buf->array_size) * sizeof(struct iovec)));
#endif

		if ((a_buf->array_start >= a_buf->array_end) &&
		    (a_buf->array_num_valid > 0)) {	/*
							 * array_num_valid check
							 * probably isn't
							 * necessary.
							 */
			/*
			 * The array was wrapped, so we need to move the wrapped
			 * part to sit directly after where the end of the array
			 * used to be.  Since we at least doubled the size of
			 * the array, there is no worry of writing past the end
			 * of the array.
			 */
			memcpy(&a_buf->bufel_array[a_buf->array_size],
			    a_buf->bufel_array,
			    a_buf->array_end * sizeof(cw_bufel_t));
			memcpy(&a_buf->cumulative_index[a_buf->array_size],
			    a_buf->cumulative_index,
			    a_buf->array_end * sizeof(cw_uint32_t));

#ifdef _LIBSTASH_DBG
			/*
			 * Zero the old copy to get rid of the bufel's' magic.
			 */
			memset(a_buf->bufel_array, 0,
			    (a_buf->array_end * sizeof(cw_bufel_t)));
			memset(a_buf->cumulative_index, 0,
			    (a_buf->array_end * sizeof(cw_uint32_t)));
#endif
			a_buf->array_end = a_buf->array_start +
			    a_buf->array_num_valid;
		}
		/*
		 * This must happen last, since the old value is used for some
		 * calculations above.
		 */
		a_buf->array_size = i;
	}

	retval = FALSE;
	RETURN:
	return retval;
}

static cw_bool_t
buf_p_buf_catenate(cw_buf_t *a_a, cw_buf_t *a_b, cw_bool_t a_preserve)
{
	cw_bool_t	retval;
	cw_uint32_t	i, a_a_index, a_b_index;
	cw_uint32_t	did_bufel_merge = 0;

	if (buf_p_array_fit(a_a, a_a->array_num_valid + a_b->array_num_valid)) {
		retval = TRUE;
		goto RETURN;
	}
	/*
	 * Try to merge the last bufel in a_a and the first bufel in a_b into
	 * one bufel in a_a.
	 */
	if ((a_a->array_num_valid > 0) && (a_b->array_num_valid > 0)) {
		cw_uint32_t	last_element_index;

		last_element_index = ((a_a->array_end + a_a->array_size - 1)
		    & (a_a->array_size - 1));

		if ((a_a->bufel_array[last_element_index].bufc->buf != NULL)
		    && (a_a->bufel_array[last_element_index].bufc->buf
			== a_b->bufel_array[a_b->array_start].bufc->buf)
		    && (a_a->bufel_array[last_element_index].end_offset
			== a_b->bufel_array[a_b->array_start].beg_offset)) {
			/*
			 * These two bufel's reference the same bufc, and the
			 * buffer regions they refer to are consecutive and
			 * adjacent.  Merge them.
			 */
			did_bufel_merge = TRUE;

			a_a->bufel_array[last_element_index].end_offset
			    = (a_a->bufel_array[last_element_index].end_offset
			    + (a_b->bufel_array[a_b->array_start].end_offset -
			    a_b->bufel_array[a_b->array_start].beg_offset));

			a_a->size +=
			    (a_b->bufel_array[a_b->array_start].end_offset -
			    a_b->bufel_array[a_b->array_start].beg_offset);
			a_a->cumulative_index[last_element_index] = a_a->size;

			if (a_preserve == FALSE) {
				if (a_b->bufel_array[a_b->array_start].bufc !=
				    NULL)
					bufc_delete(a_b->bufel_array[a_b->array_start].bufc);
#ifdef _LIBSTASH_DBG
				memset(&a_b->bufel_array[a_b->array_start], 0,
				    sizeof(cw_bufel_t));
#endif
			}
		}
	}
#ifdef _LIBSTASH_DBG
	if (did_bufel_merge) {
		buf_p_array_copy(a_a, a_b, a_b->array_num_valid - 1,
		    a_a->array_end, (a_b->array_start + 1) & (a_b->array_size -
		    1), !a_preserve);
	} else {
		buf_p_array_copy(a_a, a_b, a_b->array_num_valid, a_a->array_end,
		    a_b->array_start, !a_preserve);
	}
#else
	if (did_bufel_merge) {
		buf_p_array_copy(a_a, a_b, a_b->array_num_valid - 1,
		    a_a->array_end, (a_b->array_start + 1) & (a_b->array_size -
		    1));
	} else {
		buf_p_array_copy(a_a, a_b, a_b->array_num_valid, a_a->array_end,
		    a_b->array_start);
	}
#endif

	/*
	 * Iterate through a_b's array, creating bufel's in a_a and adding
	 * references to a_b's bufel data.
	 */
	for (i = 0, a_a_index = a_a->array_end, a_b_index = ((a_b->array_start
	    + did_bufel_merge) & (a_b->array_size - 1)); i <
	    a_b->array_num_valid - did_bufel_merge; i++, a_a_index =
	    (a_a_index + 1) & (a_a->array_size - 1), a_b_index = (a_b_index +
	    1) & (a_b->array_size - 1)) {
		a_a->size += (a_a->bufel_array[a_a_index].end_offset -
		    a_a->bufel_array[a_a_index].beg_offset);

		a_a->cumulative_index[a_a_index] = a_a->size;

		if (a_preserve)
			bufc_p_ref_increment(a_a->bufel_array[a_a_index].bufc);
	}

	/* Finish making a_a's state consistent. */
	a_a->array_end = a_a_index;
	a_a->array_num_valid += i;

	/* Make a_b's state consistent if not preserving its state. */
	if (a_preserve == FALSE) {
		a_b->array_start = 0;
		a_b->array_end = 0;
		a_b->size = 0;
		a_b->array_num_valid = 0;
		a_b->is_cumulative_valid = TRUE;
		a_b->is_cached_bufel_valid = FALSE;
/*  		a_b->cached_bufel = 0; */
	}

	retval = FALSE;
	RETURN:
	return retval;
}

#ifdef _LIBSTASH_DBG
static void
buf_p_array_copy(cw_buf_t *a_a, cw_buf_t *a_b, cw_uint32_t a_num_elements,
    cw_uint32_t a_a_start, cw_uint32_t a_b_start, cw_bool_t a_is_destructive)
#else
static void
buf_p_array_copy(cw_buf_t *a_a, cw_buf_t *a_b, cw_uint32_t a_num_elements,
    cw_uint32_t a_a_start, cw_uint32_t a_b_start)
#endif
{
	cw_bool_t	is_a_wrapped, is_b_wrapped;
	cw_uint32_t	first_chunk_size, second_chunk_size, third_chunk_size;

	/*
	 * Do 1 to 3 memcpy()'s of a_b's array to a_a's array, depending on
	 * array alignments.
	 */
	is_a_wrapped = ((a_a_start + a_num_elements)
	    > a_a->array_size) ? TRUE : FALSE;
	is_b_wrapped = ((a_b_start + a_num_elements)
	    > a_b->array_size) ? TRUE : FALSE;

	if ((is_a_wrapped == FALSE) && (is_b_wrapped == FALSE)) {
		/* Simple case; one memcpy() suffices.
		 *
		 *   a_b (src)                           a_a (dest)
		 * /------------\ 0                    /------------\ 0
		 * |            |                      |            |
		 * |            |                      |            |
		 * |------------|                      |------------|
		 * |DDDDDDDDDDDD| \                    |            |
		 * |DDDDDDDDDDDD| |                    |            |
		 * |------------| |                    |------------|
		 * |DDDDDDDDDDDD| |                  / |DDDDDDDDDDDD|
		 * |DDDDDDDDDDDD| |                  | |DDDDDDDDDDDD|
		 * |------------|  >------\          | |------------|
		 * |DDDDDDDDDDDD| |        \         | |DDDDDDDDDDDD|
		 * |DDDDDDDDDDDD| |         \        | |DDDDDDDDDDDD|
		 * |------------| |          \------<  |------------|
		 * |DDDDDDDDDDDD| |                  | |DDDDDDDDDDDD|
		 * |DDDDDDDDDDDD| /                  | |DDDDDDDDDDDD|
		 * |------------|                    | |------------|
		 * |            |                    | |DDDDDDDDDDDD|
		 * |            |                    \ |DDDDDDDDDDDD|
		 * |------------|                      |------------|
		 * |            |                      |            |
		 * |            |                      |            |
		 * |------------|                      |------------| 
		 * |            |                      |            |
		 * |            |                      |            |
		 * \------------/ 8 (a_b->array_size)  \------------/ 8 (a_a->array_size)
		 */
		if (a_num_elements > 0) {
			memcpy(&a_a->bufel_array[a_a_start],
			    &a_b->bufel_array[a_b_start],
			    a_num_elements * sizeof(cw_bufel_t));
		}
#ifdef _LIBSTASH_DBG
		if (a_is_destructive) {
			memset(&a_b->bufel_array[a_b_start], 0,
			    a_num_elements * sizeof(cw_bufel_t));
		}
#endif
	} else if (is_a_wrapped && (is_b_wrapped == FALSE)) {
		/* Two memcpy()'s, since a_b wraps into a_a.
		 *
		 *   a_b (src)                           a_a (dest)
		 * /------------\ 0                    /------------\ 0
		 * |            |                    / |DDDDDDDDDDDD|
		 * |            |                    | |DDDDDDDDDDDD|
		 * |------------| a_b_start     /---<  |------------|
		 * |DDDDDDDDDDDD| \            /     | |DDDDDDDDDDDD|
		 * |DDDDDDDDDDDD| |           /      \ |DDDDDDDDDDDD|
		 * |------------|  >-\       /         |------------|
		 * |DDDDDDDDDDDD| |   \     /          |            |
		 * |DDDDDDDDDDDD| /    \   /           |            |
		 * |------------|       \ /            |------------|
		 * |DDDDDDDDDDDD| \      X             |            |
		 * |DDDDDDDDDDDD| |     / \            |            |
		 * |------------|  >---/   \           |------------|
		 * |DDDDDDDDDDDD| |         \          |            |
		 * |DDDDDDDDDDDD| /          \         |            |
		 * |------------|             \        |------------|
		 * |            |              \       |            |
		 * |            |               \      |            |
		 * |------------|               |      |------------| a_a_start
		 * |            |               |    / |DDDDDDDDDDDD|
		 * |            |               |    | |DDDDDDDDDDDD|
		 * |------------|               \---<  |------------| 
		 * |            |                    | |DDDDDDDDDDDD|
		 * |            |                    \ |DDDDDDDDDDDD|
		 * \------------/ 8 (a_b->array_size)  \------------/ 8 (a_a->array_size)
		 */

		first_chunk_size = a_a->array_size - a_a_start;
		second_chunk_size = a_num_elements - first_chunk_size;

		memcpy(&a_a->bufel_array[a_a_start],
		    &a_b->bufel_array[a_b_start],
		    first_chunk_size * sizeof(cw_bufel_t));
#ifdef _LIBSTASH_DBG
		if (a_is_destructive) {
			memset(&a_b->bufel_array[a_b_start], 0,
			    first_chunk_size * sizeof(cw_bufel_t));
		}
#endif

		memcpy(&a_a->bufel_array[0],
		    &a_b->bufel_array[a_b_start + first_chunk_size],
		    second_chunk_size * sizeof(cw_bufel_t));
#ifdef _LIBSTASH_DBG
		if (a_is_destructive) {
			memset(&a_b->bufel_array[a_b_start + first_chunk_size],
			    0, second_chunk_size * sizeof(cw_bufel_t));
		}
#endif
	} else {
		if ((is_a_wrapped) && ((a_b->array_size - a_b_start) >
		    (a_a->array_size - a_a_start))) {
			/* The first chunk of a_b wraps into a_a.
			 *
			 *   a_b (src)                           a_a (dest)
			 * /------------\ 0                    /------------\ 0
			 * |DDDDDDDDDDDD| \               ___/ |DDDDDDDDDDDD|
			 * |DDDDDDDDDDDD| |              /   \ |DDDDDDDDDDDD|
			 * |------------|  >------\     |      |------------|
			 * |DDDDDDDDDDDD| |        \    |    / |DDDDDDDDDDDD|
			 * |DDDDDDDDDDDD| /         \   |    | |DDDDDDDDDDDD|
			 * |------------|            \--+---<  |------------|
			 * |            |               |    | |DDDDDDDDDDDD|
			 * |            |               |    \ |DDDDDDDDDDDD|
			 * |------------|               |      |------------|
			 * |            |               |      |            |
			 * |            |               |      |            |
			 * |------------|               |      |------------|
			 * |            |               |      |            |
			 * |            |               |      |            |
			 * |------------|               |      |------------|
			 * |            |               |      |            |
			 * |            |               |      |            |
			 * |------------|              /       |------------|
			 * |DDDDDDDDDDDD| \___        /        |            |
			 * |DDDDDDDDDDDD| /   \      /         |            |
			 * |------------|      \----/---\      |------------| 
			 * |DDDDDDDDDDDD| \________/     \___/ |DDDDDDDDDDDD|
			 * |DDDDDDDDDDDD| /                  \ |DDDDDDDDDDDD|
			 * \------------/ 8 (a_b->array_size)  \------------/ 8 (a_a->array_size)
			 */
			first_chunk_size = a_a->array_size - a_a_start;
			second_chunk_size = a_b->array_size - a_b_start -
			    first_chunk_size;
			third_chunk_size = a_num_elements - second_chunk_size -
			    first_chunk_size;

			memcpy(&a_a->bufel_array[a_a_start],
			    &a_b->bufel_array[a_b_start],
			    first_chunk_size * sizeof(cw_bufel_t));
#ifdef _LIBSTASH_DBG
			if (a_is_destructive) {
				memset(&a_b->bufel_array[a_b_start], 0,
				    first_chunk_size * sizeof(cw_bufel_t));
			}
#endif

			memcpy(&a_a->bufel_array[0],
			    &a_b->bufel_array[a_b_start + first_chunk_size],
			    second_chunk_size * sizeof(cw_bufel_t));
#ifdef _LIBSTASH_DBG
			if (a_is_destructive) {
				memset(&a_b->bufel_array[a_b_start +
				    first_chunk_size], 0, second_chunk_size *
				    sizeof(cw_bufel_t));
			}
#endif

			memcpy(&a_a->bufel_array[second_chunk_size],
			    &a_b->bufel_array[0], third_chunk_size *
			    sizeof(cw_bufel_t));
#ifdef _LIBSTASH_DBG
			if (a_is_destructive) {
				memset(&a_b->bufel_array[0], 0,
				    third_chunk_size * sizeof(cw_bufel_t));
			}
#endif
		} else if ((is_a_wrapped) && ((a_b->array_size - a_b_start) <
		    (a_a->array_size - a_a_start))) {
			/* The second chunk of a_b wraps into a_a.
			 *
			 *   a_b (src)                           a_a (dest)
			 * /------------\ 0                    /------------\ 0
			 * |DDDDDDDDDDDD| \___        _______/ |DDDDDDDDDDDD|
			 * |DDDDDDDDDDDD| /   \      /       \ |DDDDDDDDDDDD|
			 * |------------|     |     /          |------------|
			 * |DDDDDDDDDDDD| \___|____/           |            |
			 * |DDDDDDDDDDDD| /   |                |            |
			 * |------------|     |                |------------|
			 * |            |     |                |            |
			 * |            |     |                |            |
			 * |------------|     |                |------------|
			 * |            |     |                |            |
			 * |            |     |                |            |
			 * |------------|     |                |------------|
			 * |            |     |                |            |
			 * |            |     |                |            |
			 * |------------|     |                |------------|
			 * |            |     |              / |DDDDDDDDDDDD|
			 * |            |     |              | |DDDDDDDDDDDD|
			 * |------------|     |      /------<  |------------|
			 * |DDDDDDDDDDDD| \   |     /        | |DDDDDDDDDDDD|
			 * |DDDDDDDDDDDD| |   |    /         \ |DDDDDDDDDDDD|
			 * |------------|  >--|---/            |------------| 
			 * |DDDDDDDDDDDD| |    \_____________/ |DDDDDDDDDDDD|
			 * |DDDDDDDDDDDD| /                  \ |DDDDDDDDDDDD|
			 * \------------/ 8 (a_b->array_size)  \------------/ 8 (a_a->array_size)
			 */
			first_chunk_size = a_b->array_size - a_b_start;
			second_chunk_size = a_a->array_size - a_a_start -
			    first_chunk_size;
			third_chunk_size = a_num_elements - second_chunk_size -
			    first_chunk_size;

			memcpy(&a_a->bufel_array[a_a_start],
			    &a_b->bufel_array[a_b_start],
			    first_chunk_size * sizeof(cw_bufel_t));
#ifdef _LIBSTASH_DBG
			if (a_is_destructive) {
				memset(&a_a->bufel_array[a_b_start], 0,
				    first_chunk_size * sizeof(cw_bufel_t));
			}
#endif

			memcpy(&a_a->bufel_array[a_a_start + first_chunk_size],
			    &a_b->bufel_array[0],
			    second_chunk_size * sizeof(cw_bufel_t));
#ifdef _LIBSTASH_DBG
			if (a_is_destructive) {
				memset(&a_b->bufel_array[0], 0,
				    second_chunk_size * sizeof(cw_bufel_t));
			}
#endif

			memcpy(&a_a->bufel_array[0],
			    &a_b->bufel_array[second_chunk_size],
			    third_chunk_size * sizeof(cw_bufel_t));
#ifdef _LIBSTASH_DBG
			if (a_is_destructive) {
				memset(&a_b->bufel_array[second_chunk_size], 0,
				    third_chunk_size * sizeof(cw_bufel_t));
			}
#endif
		} else {
			/*
			 * Either a_b unwraps into a_a, or the two chunks of
			 * a_b's array that are being copied wrap at the
			 * same point in a_b as when copied to a_a. These
			 * two cases can be treated the same way in the
			 * code.
			 *
			 *   a_b (src)                           a_a (dest)
			 * /------------\ 0                    /------------\ 0
			 * |DDDDDDDDDDDD| \                  / |DDDDDDDDDDDD|
			 * |DDDDDDDDDDDD| |                  | |DDDDDDDDDDDD|
			 * |------------|  >----------------<  |------------|
			 * |DDDDDDDDDDDD| |                  | |DDDDDDDDDDDD|
			 * |DDDDDDDDDDDD| /                  \ |DDDDDDDDDDDD|
			 * |------------|                      |------------|
			 * |            |                      |            |
			 * |            |                      |            |
			 * |------------|                      |------------|
			 * |            |                      |            |
			 * |            |                      |            |
			 * |------------|                      |------------|
			 * |            |                      |            |
			 * |            |                      |            |
			 * |------------|                      |------------|
			 * |            |                      |            |
			 * |            |                      |            |
			 * |------------|                      |------------|
			 * |DDDDDDDDDDDD| \                  / |DDDDDDDDDDDD|
			 * |DDDDDDDDDDDD| |                  | |DDDDDDDDDDDD|
			 * |------------|  >----------------<  |------------| 
			 * |DDDDDDDDDDDD| |                  | |DDDDDDDDDDDD|
			 * |DDDDDDDDDDDD| /                  \ |DDDDDDDDDDDD|
			 * \------------/ 8 (a_b->array_size)  \------------/ 8 (a_a->array_size)
			 *
			 *   a_b (src)                           a_a (dest)
			 * /------------\ 0                    /------------\ 0
			 * |DDDDDDDDDDDD| \                    |            |
			 * |DDDDDDDDDDDD| |                    |            |
			 * |------------|  >-\                 |------------|
			 * |DDDDDDDDDDDD| |   \                |            |
			 * |DDDDDDDDDDDD| /    \               |            |
			 * |------------|       \              |------------|
			 * |            |        \           / |DDDDDDDDDDDD|
			 * |            |         \          | |DDDDDDDDDDDD|
			 * |------------|          \     /--<  |------------|
			 * |            |           \   /    | |DDDDDDDDDDDD|
			 * |            |            \ /     \ |DDDDDDDDDDDD|
			 * |------------|             X        |------------|
			 * |            |            / \     / |DDDDDDDDDDDD|
			 * |            |           /   \    | |DDDDDDDDDDDD|
			 * |------------|          /     \--<  |------------|
			 * |            |         /          | |DDDDDDDDDDDD|
			 * |            |        /           \ |DDDDDDDDDDDD|
			 * |------------|       /              |------------|
			 * |DDDDDDDDDDDD| \    /               |            |
			 * |DDDDDDDDDDDD| |   /                |            |
			 * |------------|  >-/                 |------------| 
			 * |DDDDDDDDDDDD| |                    |            |
			 * |DDDDDDDDDDDD| /                    |            |
			 * \------------/ 8 (a_b->array_size)  \------------/ 8 (a_a->array_size)
			 */
			first_chunk_size = a_b->array_size - a_b_start;
			second_chunk_size = a_num_elements - first_chunk_size;

			memcpy(&a_a->bufel_array[a_a_start],
			    &a_b->bufel_array[a_b_start],
			    first_chunk_size * sizeof(cw_bufel_t));
#ifdef _LIBSTASH_DBG
			if (a_is_destructive) {
				memset(&a_b->bufel_array[a_b_start], 0,
				    first_chunk_size * sizeof(cw_bufel_t));
			}
#endif

			memcpy(&a_a->bufel_array[(a_a_start + first_chunk_size)
				& (a_a->array_size - 1)],
			    &a_b->bufel_array[0],
			    second_chunk_size * sizeof(cw_bufel_t));
#ifdef _LIBSTASH_DBG
			if (a_is_destructive) {
				memset(&a_b->bufel_array[0], 0,
				    second_chunk_size * sizeof(cw_bufel_t));
			}
#endif
		}
	}
}

static cw_bool_t
buf_p_writeable_range_make(cw_buf_t *a_buf, cw_uint32_t a_offset, cw_uint32_t
    a_length)
{
	cw_bool_t	retval;
	cw_uint32_t	first_array_element, last_array_element, bufel_offset;
	cw_uint32_t	i, num_iterations;
	cw_bufel_t	*bufel;
	cw_bufc_t	*bufc;
	void		*buffer;

	_cw_assert(a_length > 0);

	/*
	 * Add extra buffer space to the end of the buf if the writeable range
	 * we're creating extends past the current end of the buf.
	 */
	if (a_offset + a_length > a_buf->size) {
		bufc = bufc_new(NULL, a_buf->mem, (cw_opaque_dealloc_t
		    *)mem_free, cw_g_mem);
		if (bufc == NULL) {
			retval = TRUE;
			goto RETURN;
		}
		buffer = _cw_mem_malloc(a_buf->mem, (a_offset + a_length) -
		    a_buf->size);
		if (buffer == NULL) {
			bufc_delete(bufc);
			retval = TRUE;
			goto RETURN;
		}
		bufc_buffer_set(bufc, buffer, (a_offset + a_length) -
		    a_buf->size, TRUE, (cw_opaque_dealloc_t *)mem_free,
		    cw_g_mem);

		if (buf_p_array_fit(a_buf, a_buf->array_num_valid + 1)) {
			bufc_delete(bufc);
			retval = TRUE;
			goto RETURN;
		}
		/* Initialize bufel. */
/*  		memset(&a_buf->bufel_array[a_buf->array_end], 0, */
/*  		    sizeof(cw_bufel_t)); */
#ifdef _LIBSTASH_DBG
		a_buf->bufel_array[a_buf->array_end].magic_a = _CW_BUFEL_MAGIC;
		a_buf->bufel_array[a_buf->array_end].size_of =
		    sizeof(cw_bufel_t);
		a_buf->bufel_array[a_buf->array_end].magic_b = _CW_BUFEL_MAGIC;
#endif
		a_buf->bufel_array[a_buf->array_end].beg_offset = 0;
		a_buf->bufel_array[a_buf->array_end].end_offset = ((a_offset +
		    a_length) - a_buf->size);
		a_buf->bufel_array[a_buf->array_end].bufc = bufc;

		a_buf->size = a_offset + a_length;
		a_buf->array_num_valid++;
		/* Do this in case the cumulative index is valid. */
		a_buf->cumulative_index[a_buf->array_end] = a_buf->size;
		a_buf->array_end = ((a_buf->array_end + 1) &
		    (a_buf->array_size - 1));
	}
	/*
	 * March through the bufel's we need our own copy of and make them
	 * writeable.
	 *
	 * Note that this algorithm has the potential to fragment memory, but
	 * the functions that use this facility are not normally meant to be
	 * used in situations where a bufc is unwriteable (reference count
	 * greater than one or marked unwriteable).
	 */
	buf_p_data_position_get(a_buf, a_offset, &first_array_element,
	    &bufel_offset);
	buf_p_data_position_get(a_buf, a_offset + a_length - 1,
	    &last_array_element, &bufel_offset);

	num_iterations = (((last_array_element + a_buf->array_size) -
	    first_array_element) & (a_buf->array_size - 1)) + 1;
	for (i = 0; i < num_iterations; i++) {
		bufel = &a_buf->bufel_array[((first_array_element + i) &
		    (a_buf->array_size - 1))];
		if ((bufc_p_is_writeable(bufel->bufc) == FALSE) ||
		    (bufc_p_ref_count_get(bufel->bufc) > 1)) {
			buffer = _cw_mem_malloc(a_buf->mem, bufel->end_offset -
			    bufel->beg_offset);
			if (buffer == NULL) {
				retval = TRUE;
				goto RETURN;
			}
			bufc = bufc_new(NULL, a_buf->mem, NULL, NULL);
			if (bufc == NULL) {
				_cw_mem_free(a_buf->mem, buffer);
				retval = TRUE;
				goto RETURN;
			}
			bufc_buffer_set(bufc, buffer, bufel->end_offset -
			    bufel->beg_offset, TRUE, (cw_opaque_dealloc_t
			    *)mem_free, cw_g_mem);

			memcpy(buffer,
			    bufel->bufc->buf + bufel->beg_offset,
			    bufel->end_offset - bufel->beg_offset);

			bufc_delete(bufel->bufc);
			bufel->bufc = bufc;
			bufel->end_offset -= bufel->beg_offset;
			bufel->beg_offset = 0;
		}
		_cw_assert(bufel->bufc->is_writeable);
		_cw_assert(1 == bufc_p_ref_count_get(bufel->bufc));
	}

	retval = FALSE;
	RETURN:
	return retval;
}

cw_bufc_t *
bufc_new(cw_bufc_t *a_bufc, cw_mem_t *a_mem, cw_opaque_dealloc_t
    *a_dealloc_func, void *a_dealloc_arg)
{
	cw_bufc_t	*retval;

	if (a_bufc == NULL) {
		retval = (cw_bufc_t *)_cw_mem_malloc(a_mem,
		    sizeof(cw_bufc_t));
		if (retval == NULL)
			goto RETURN;
		memset(retval, 0, sizeof(cw_bufc_t));
		retval->dealloc_func = (cw_opaque_dealloc_t *)mem_free;
		retval->dealloc_arg = (void *)a_mem;
	} else {
		retval = a_bufc;
		memset(retval, 0, sizeof(cw_bufc_t));
		retval->dealloc_func = a_dealloc_func;
		retval->dealloc_arg = a_dealloc_arg;
	}
	retval->ref_count = 1;

#ifdef _LIBSTASH_DBG
	retval->magic_a = _CW_BUFC_MAGIC;
	retval->size_of = sizeof(cw_bufc_t);
	retval->magic_b = _CW_BUFC_MAGIC;
#endif

	mtx_new(&retval->lock);

	RETURN:
	return retval;
}

void
bufc_delete(cw_bufc_t *a_bufc)
{
	cw_bool_t	should_delete;

	_cw_check_ptr(a_bufc);
	_cw_assert(a_bufc->magic_a == _CW_BUFC_MAGIC);
	_cw_assert(a_bufc->size_of == sizeof(cw_bufc_t));
	_cw_assert(a_bufc->magic_b == _CW_BUFC_MAGIC);

	mtx_lock(&a_bufc->lock);

	a_bufc->ref_count--;
	if (a_bufc->ref_count == 0) {
		/*
		 * Make a note that we should delete the bufc once we've
		 * released the mutex.
		 */
		should_delete = TRUE;
	} else
		should_delete = FALSE;

	mtx_unlock(&a_bufc->lock);

	if (should_delete) {
		mtx_delete(&a_bufc->lock);

		if (a_bufc->buffer_dealloc_func != NULL) {
			_cw_opaque_dealloc(a_bufc->buffer_dealloc_func,
			    a_bufc->buffer_dealloc_arg, a_bufc->buf);
		}
		if (a_bufc->dealloc_func != NULL) {
			_cw_opaque_dealloc(a_bufc->dealloc_func,
			    a_bufc->dealloc_arg, a_bufc);
		}
	}
}

void
bufc_buffer_set(cw_bufc_t *a_bufc, void *a_buffer, cw_uint32_t a_size, cw_bool_t
    a_is_writeable, cw_opaque_dealloc_t *a_dealloc_func, void *a_dealloc_arg)
{
	_cw_check_ptr(a_bufc);
	_cw_assert(a_bufc->magic_a == _CW_BUFC_MAGIC);
	_cw_assert(a_bufc->size_of == sizeof(cw_bufc_t));
	_cw_assert(a_bufc->magic_b == _CW_BUFC_MAGIC);
	_cw_check_ptr(a_buffer);
	_cw_assert(a_size > 0);

	a_bufc->buf = (cw_uint8_t *)a_buffer;
	a_bufc->buf_size = a_size;
	a_bufc->is_writeable = a_is_writeable;
	a_bufc->buffer_dealloc_func = a_dealloc_func;
	a_bufc->buffer_dealloc_arg = a_dealloc_arg;
}

cw_uint32_t
bufc_size_get(cw_bufc_t *a_bufc)
{
	cw_uint32_t	retval;

	_cw_check_ptr(a_bufc);
	_cw_assert(a_bufc->magic_a == _CW_BUFC_MAGIC);
	_cw_assert(a_bufc->size_of == sizeof(cw_bufc_t));
	_cw_assert(a_bufc->magic_b == _CW_BUFC_MAGIC);

	retval = a_bufc->buf_size;

	return retval;
}

static void
bufc_p_dump(cw_bufc_t *a_bufc, const char *a_prefix)
{
	cw_uint32_t	i;

	_cw_check_ptr(a_bufc);
	_cw_assert(a_bufc->magic_a == _CW_BUFC_MAGIC);
	_cw_assert(a_bufc->size_of == sizeof(cw_bufc_t));
	_cw_assert(a_bufc->magic_b == _CW_BUFC_MAGIC);
	_cw_check_ptr(a_prefix);

	mtx_lock(&a_bufc->lock);

	_cw_out_put("[s]| bufc_dump()\n", a_prefix);
#ifdef _LIBSTASH_DBG
	_cw_out_put("[s]|--> magic_a : 0x[i|b:16]\n", a_prefix,
	    a_bufc->magic_a);
	_cw_out_put("[s]|--> magic_b : 0x[i|b:16]\n", a_prefix,
	    a_bufc->magic_b);
	_cw_out_put("[s]|--> size_of : [i]\n", a_prefix, a_bufc->size_of);
#endif
	_cw_out_put("[s]|--> free_func : 0x[p]\n", a_prefix,
	    a_bufc->dealloc_func);
	_cw_out_put("[s]|--> free_arg : 0x[p]\n", a_prefix,
	    a_bufc->dealloc_arg);
	_cw_out_put("[s]|--> ref_count : [i]\n", a_prefix, a_bufc->ref_count);
	_cw_out_put("[s]|--> is_writeable : [s]\n", a_prefix,
	    a_bufc->is_writeable ? "TRUE" : "FALSE");
	_cw_out_put("[s]|--> buf_size : [i]\n", a_prefix, a_bufc->buf_size);
	_cw_out_put("[s]\\--> buf (0x[i|w:8|p:0|b:16]) : ", a_prefix,
	    a_bufc->buf);

	for (i = 0; i < a_bufc->buf_size; i++) {
		if (i % 16 == 0) {
			_cw_out_put("\n[s]         [[[i|w:4|b:16]] ", a_prefix,
			    i);
		}
		_cw_out_put("[i|w:2|p:0|b:16] ", a_bufc->buf[i]);
	}
	_cw_out_put("\n");

	mtx_unlock(&a_bufc->lock);
}

static cw_bool_t
bufc_p_is_writeable(cw_bufc_t *a_bufc)
{
	_cw_check_ptr(a_bufc);
	_cw_assert(a_bufc->magic_a == _CW_BUFC_MAGIC);
	_cw_assert(a_bufc->size_of == sizeof(cw_bufc_t));
	_cw_assert(a_bufc->magic_b == _CW_BUFC_MAGIC);

	return a_bufc->is_writeable;
}

static cw_uint32_t
bufc_p_ref_count_get(cw_bufc_t *a_bufc)
{
	cw_uint32_t	retval;

	_cw_check_ptr(a_bufc);
	_cw_assert(a_bufc->magic_a == _CW_BUFC_MAGIC);
	_cw_assert(a_bufc->size_of == sizeof(cw_bufc_t));
	_cw_assert(a_bufc->magic_b == _CW_BUFC_MAGIC);

	mtx_lock(&a_bufc->lock);

	retval = a_bufc->ref_count;

	mtx_unlock(&a_bufc->lock);
	return retval;
}

static void
bufc_p_ref_increment(cw_bufc_t *a_bufc)
{
	_cw_check_ptr(a_bufc);
	_cw_assert(a_bufc->magic_a == _CW_BUFC_MAGIC);
	_cw_assert(a_bufc->size_of == sizeof(cw_bufc_t));
	_cw_assert(a_bufc->magic_b == _CW_BUFC_MAGIC);

	mtx_lock(&a_bufc->lock);

	a_bufc->ref_count++;

	mtx_unlock(&a_bufc->lock);
}
