/****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * <<< Description >>>
 *
 * Test the mq (message queue) class.
 *
 ****************************************************************************/

#include "../include/libstash/libstash.h"

int
main()
{
	libstash_init();
	_cw_out_put("Test begin\n");

	/* mq_new(), mq_delete(). */
	{
		cw_mq_t	mq, *mq_p;

		_cw_assert(mq_new(&mq, sizeof(cw_uint32_t)) == &mq);
		mq_delete(&mq);

		mq_p = mq_new(NULL, sizeof(cw_uint64_t));
		_cw_check_ptr(mq_p);
		mq_delete(mq_p);
	}

	/* mq_tryget(), mq_put(). */
	{
		cw_mq_t		*mq_p;
		cw_uint8_t	i = 1, j = 2, k = 3, t;

		mq_p = mq_new(NULL, sizeof(cw_uint8_t));
		_cw_check_ptr(mq_p);

		_cw_assert(mq_tryget(mq_p, &t));
		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_tryget(mq_p, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(mq_p, &t));

		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_put(mq_p, j) == 0);
		_cw_assert(mq_put(mq_p, k) == 0);
		_cw_assert(mq_tryget(mq_p, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_tryget(mq_p, &t) == FALSE);
		_cw_assert(t == j);
		_cw_assert(mq_tryget(mq_p, &t) == FALSE);
		_cw_assert(t == k);
		_cw_assert(mq_tryget(mq_p, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(mq_p, &t));

		mq_delete(mq_p);
	}
	{
		cw_mq_t		*mq_p;
		cw_uint16_t	i = 1, j = 2, k = 3, t;

		mq_p = mq_new(NULL, sizeof(cw_uint16_t));
		_cw_check_ptr(mq_p);

		_cw_assert(mq_tryget(mq_p, &t));
		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_tryget(mq_p, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(mq_p, &t));

		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_put(mq_p, j) == 0);
		_cw_assert(mq_put(mq_p, k) == 0);
		_cw_assert(mq_tryget(mq_p, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_tryget(mq_p, &t) == FALSE);
		_cw_assert(t == j);
		_cw_assert(mq_tryget(mq_p, &t) == FALSE);
		_cw_assert(t == k);
		_cw_assert(mq_tryget(mq_p, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(mq_p, &t));

		mq_delete(mq_p);
	}
	{
		cw_mq_t		*mq_p;
		cw_uint32_t	i = 1, j = 2, k = 3, t;

		mq_p = mq_new(NULL, sizeof(cw_uint32_t));
		_cw_check_ptr(mq_p);

		_cw_assert(mq_tryget(mq_p, &t));
		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_tryget(mq_p, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(mq_p, &t));

		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_put(mq_p, j) == 0);
		_cw_assert(mq_put(mq_p, k) == 0);
		_cw_assert(mq_tryget(mq_p, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_tryget(mq_p, &t) == FALSE);
		_cw_assert(t == j);
		_cw_assert(mq_tryget(mq_p, &t) == FALSE);
		_cw_assert(t == k);
		_cw_assert(mq_tryget(mq_p, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(mq_p, &t));

		mq_delete(mq_p);
	}
	{
		cw_mq_t		*mq_p;
		cw_uint64_t	i = 1, j = 2, k = 3, t;

		mq_p = mq_new(NULL, sizeof(cw_uint64_t));
		_cw_check_ptr(mq_p);

		_cw_assert(mq_tryget(mq_p, &t));
		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_tryget(mq_p, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(mq_p, &t));

		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_put(mq_p, j) == 0);
		_cw_assert(mq_put(mq_p, k) == 0);
		_cw_assert(mq_tryget(mq_p, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_tryget(mq_p, &t) == FALSE);
		_cw_assert(t == j);
		_cw_assert(mq_tryget(mq_p, &t) == FALSE);
		_cw_assert(t == k);
		_cw_assert(mq_tryget(mq_p, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(mq_p, &t));

		mq_delete(mq_p);
	}

	/* mq_timedget(), mq_put(). */
	{
		cw_mq_t		*mq_p;
		cw_uint8_t	i = 1, j = 2, k = 3, t;
		struct timespec	timeout = {0, 10000};

		mq_p = mq_new(NULL, sizeof(cw_uint8_t));
		_cw_check_ptr(mq_p);

		_cw_assert(mq_timedget(mq_p, &timeout));
		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_timedget(mq_p, &timeout, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_timedget(mq_p, &timeout));

		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_put(mq_p, j) == 0);
		_cw_assert(mq_put(mq_p, k) == 0);
		_cw_assert(mq_timedget(mq_p, &timeout, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_timedget(mq_p, &timeout, &t) == FALSE);
		_cw_assert(t == j);
		_cw_assert(mq_timedget(mq_p, &timeout, &t) == FALSE);
		_cw_assert(t == k);
		_cw_assert(mq_timedget(mq_p, &timeout, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_timedget(mq_p, &timeout, &t));

		mq_delete(mq_p);
	}
	{
		cw_mq_t		*mq_p;
		cw_uint16_t	i = 1, j = 2, k = 3, t;
		struct timespec	timeout = {0, 10000};

		mq_p = mq_new(NULL, sizeof(cw_uint16_t));
		_cw_check_ptr(mq_p);

		_cw_assert(mq_timedget(mq_p, &timeout));
		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_timedget(mq_p, &timeout, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_timedget(mq_p, &timeout));

		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_put(mq_p, j) == 0);
		_cw_assert(mq_put(mq_p, k) == 0);
		_cw_assert(mq_timedget(mq_p, &timeout, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_timedget(mq_p, &timeout, &t) == FALSE);
		_cw_assert(t == j);
		_cw_assert(mq_timedget(mq_p, &timeout, &t) == FALSE);
		_cw_assert(t == k);
		_cw_assert(mq_timedget(mq_p, &timeout, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_timedget(mq_p, &timeout, &t));

		mq_delete(mq_p);
	}
	{
		cw_mq_t		*mq_p;
		cw_uint32_t	i = 1, j = 2, k = 3, t;
		struct timespec	timeout = {0, 10000};

		mq_p = mq_new(NULL, sizeof(cw_uint32_t));
		_cw_check_ptr(mq_p);

		_cw_assert(mq_timedget(mq_p, &timeout));
		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_timedget(mq_p, &timeout, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_timedget(mq_p, &timeout));

		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_put(mq_p, j) == 0);
		_cw_assert(mq_put(mq_p, k) == 0);
		_cw_assert(mq_timedget(mq_p, &timeout, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_timedget(mq_p, &timeout, &t) == FALSE);
		_cw_assert(t == j);
		_cw_assert(mq_timedget(mq_p, &timeout, &t) == FALSE);
		_cw_assert(t == k);
		_cw_assert(mq_timedget(mq_p, &timeout, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_timedget(mq_p, &timeout, &t));

		mq_delete(mq_p);
	}
	{
		cw_mq_t		*mq_p;
		cw_uint64_t	i = 1, j = 2, k = 3, t;
		struct timespec	timeout = {0, 10000};

		mq_p = mq_new(NULL, sizeof(cw_uint64_t));
		_cw_check_ptr(mq_p);

		_cw_assert(mq_timedget(mq_p, &timeout));
		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_timedget(mq_p, &timeout, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_timedget(mq_p, &timeout));

		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_put(mq_p, j) == 0);
		_cw_assert(mq_put(mq_p, k) == 0);
		_cw_assert(mq_timedget(mq_p, &timeout, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_timedget(mq_p, &timeout, &t) == FALSE);
		_cw_assert(t == j);
		_cw_assert(mq_timedget(mq_p, &timeout, &t) == FALSE);
		_cw_assert(t == k);
		_cw_assert(mq_timedget(mq_p, &timeout, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_timedget(mq_p, &timeout, &t));

		mq_delete(mq_p);
	}

	/* mq_get(), mq_put(), mq_tryget(). */
	{
		cw_mq_t		*mq_p;
		cw_uint8_t	i = 1, j = 2, k = 3, t;

		mq_p = mq_new(NULL, sizeof(cw_uint8_t));
		_cw_check_ptr(mq_p);

		_cw_assert(mq_tryget(mq_p, &t));
		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_get(mq_p, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(mq_p, &t));

		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_put(mq_p, j) == 0);
		_cw_assert(mq_put(mq_p, k) == 0);
		_cw_assert(mq_get(mq_p, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_get(mq_p, &t) == FALSE);
		_cw_assert(t == j);
		_cw_assert(mq_get(mq_p, &t) == FALSE);
		_cw_assert(t == k);
		_cw_assert(mq_get(mq_p, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(mq_p, &t));

		mq_delete(mq_p);
	}
	{
		cw_mq_t		*mq_p;
		cw_uint16_t	i = 1, j = 2, k = 3, t;

		mq_p = mq_new(NULL, sizeof(cw_uint16_t));
		_cw_check_ptr(mq_p);

		_cw_assert(mq_tryget(mq_p, &t));
		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_get(mq_p, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(mq_p, &t));

		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_put(mq_p, j) == 0);
		_cw_assert(mq_put(mq_p, k) == 0);
		_cw_assert(mq_get(mq_p, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_get(mq_p, &t) == FALSE);
		_cw_assert(t == j);
		_cw_assert(mq_get(mq_p, &t) == FALSE);
		_cw_assert(t == k);
		_cw_assert(mq_get(mq_p, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(mq_p, &t));

		mq_delete(mq_p);
	}
	{
		cw_mq_t		*mq_p;
		cw_uint32_t	i = 1, j = 2, k = 3, t;

		mq_p = mq_new(NULL, sizeof(cw_uint32_t));
		_cw_check_ptr(mq_p);

		_cw_assert(mq_tryget(mq_p, &t));
		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_get(mq_p, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(mq_p, &t));

		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_put(mq_p, j) == 0);
		_cw_assert(mq_put(mq_p, k) == 0);
		_cw_assert(mq_get(mq_p, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_get(mq_p, &t) == FALSE);
		_cw_assert(t == j);
		_cw_assert(mq_get(mq_p, &t) == FALSE);
		_cw_assert(t == k);
		_cw_assert(mq_get(mq_p, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(mq_p, &t));

		mq_delete(mq_p);
	}
	{
		cw_mq_t		*mq_p;
		cw_uint64_t	i = 1, j = 2, k = 3, t;

		mq_p = mq_new(NULL, sizeof(cw_uint64_t));
		_cw_check_ptr(mq_p);

		_cw_assert(mq_tryget(mq_p, &t));
		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_get(mq_p, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(mq_p, &t));

		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_put(mq_p, j) == 0);
		_cw_assert(mq_put(mq_p, k) == 0);
		_cw_assert(mq_get(mq_p, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_get(mq_p, &t) == FALSE);
		_cw_assert(t == j);
		_cw_assert(mq_get(mq_p, &t) == FALSE);
		_cw_assert(t == k);
		_cw_assert(mq_get(mq_p, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(mq_p, &t));

		mq_delete(mq_p);
	}

	/* mq_start_get(), mq_stop_get(), mq_tryget(), mq_put(). */
	{
		cw_mq_t		*mq_p;
		cw_uint8_t	i = 1, j = 2, k = 3, t;

		mq_p = mq_new(NULL, sizeof(cw_uint8_t));
		_cw_check_ptr(mq_p);

		_cw_assert(mq_stop_get(mq_p) == FALSE);
		_cw_assert(mq_stop_get(mq_p));
		_cw_assert(mq_tryget(mq_p, &t));

		_cw_assert(mq_start_get(mq_p) == FALSE);
		_cw_assert(mq_start_get(mq_p));
		_cw_assert(mq_tryget(mq_p, &t));

		_cw_assert(mq_put(mq_p, i) == 0);

		_cw_assert(mq_stop_get(mq_p) == FALSE);
		_cw_assert(mq_stop_get(mq_p));
		_cw_assert(mq_tryget(mq_p, &t));
		_cw_assert(mq_get(mq_p, &t));

		_cw_assert(mq_start_get(mq_p) == FALSE);
		_cw_assert(mq_start_get(mq_p));
		_cw_assert(mq_tryget(mq_p, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(mq_p, &t));

		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_put(mq_p, j) == 0);
		_cw_assert(mq_put(mq_p, k) == 0);

		mq_delete(mq_p);
	}
	{
		cw_mq_t		*mq_p;
		cw_uint16_t	i = 1, j = 2, k = 3, t;

		mq_p = mq_new(NULL, sizeof(cw_uint16_t));
		_cw_check_ptr(mq_p);

		_cw_assert(mq_stop_get(mq_p) == FALSE);
		_cw_assert(mq_stop_get(mq_p));
		_cw_assert(mq_tryget(mq_p, &t));

		_cw_assert(mq_start_get(mq_p) == FALSE);
		_cw_assert(mq_start_get(mq_p));
		_cw_assert(mq_tryget(mq_p, &t));

		_cw_assert(mq_put(mq_p, i) == 0);

		_cw_assert(mq_stop_get(mq_p) == FALSE);
		_cw_assert(mq_stop_get(mq_p));
		_cw_assert(mq_tryget(mq_p, &t));
		_cw_assert(mq_get(mq_p, &t));

		_cw_assert(mq_start_get(mq_p) == FALSE);
		_cw_assert(mq_start_get(mq_p));
		_cw_assert(mq_tryget(mq_p, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(mq_p, &t));

		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_put(mq_p, j) == 0);
		_cw_assert(mq_put(mq_p, k) == 0);

		mq_delete(mq_p);
	}
	{
		cw_mq_t		*mq_p;
		cw_uint32_t	i = 1, j = 2, k = 3, t;

		mq_p = mq_new(NULL, sizeof(cw_uint32_t));
		_cw_check_ptr(mq_p);

		_cw_assert(mq_stop_get(mq_p) == FALSE);
		_cw_assert(mq_stop_get(mq_p));
		_cw_assert(mq_tryget(mq_p, &t));

		_cw_assert(mq_start_get(mq_p) == FALSE);
		_cw_assert(mq_start_get(mq_p));
		_cw_assert(mq_tryget(mq_p, &t));

		_cw_assert(mq_put(mq_p, i) == 0);

		_cw_assert(mq_stop_get(mq_p) == FALSE);
		_cw_assert(mq_stop_get(mq_p));
		_cw_assert(mq_tryget(mq_p, &t));
		_cw_assert(mq_get(mq_p, &t));

		_cw_assert(mq_start_get(mq_p) == FALSE);
		_cw_assert(mq_start_get(mq_p));
		_cw_assert(mq_tryget(mq_p, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(mq_p, &t));

		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_put(mq_p, j) == 0);
		_cw_assert(mq_put(mq_p, k) == 0);

		mq_delete(mq_p);
	}
	{
		cw_mq_t		*mq_p;
		cw_uint64_t	i = 1, j = 2, k = 3, t;

		mq_p = mq_new(NULL, sizeof(cw_uint64_t));
		_cw_check_ptr(mq_p);

		_cw_assert(mq_stop_get(mq_p) == FALSE);
		_cw_assert(mq_stop_get(mq_p));
		_cw_assert(mq_tryget(mq_p, &t));

		_cw_assert(mq_start_get(mq_p) == FALSE);
		_cw_assert(mq_start_get(mq_p));
		_cw_assert(mq_tryget(mq_p, &t));

		_cw_assert(mq_put(mq_p, i) == 0);

		_cw_assert(mq_stop_get(mq_p) == FALSE);
		_cw_assert(mq_stop_get(mq_p));
		_cw_assert(mq_tryget(mq_p, &t));
		_cw_assert(mq_get(mq_p, &t));

		_cw_assert(mq_start_get(mq_p) == FALSE);
		_cw_assert(mq_start_get(mq_p));
		_cw_assert(mq_tryget(mq_p, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(mq_p, &t));

		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_put(mq_p, j) == 0);
		_cw_assert(mq_put(mq_p, k) == 0);

		mq_delete(mq_p);
	}

	/* mq_start_put(), mq_stop_put(), mq_tryget(), mq_put(). */
	{
		cw_mq_t		*mq_p;
		cw_uint8_t	i = 1, j = 2, k = 3, t;

		mq_p = mq_new(NULL, sizeof(cw_uint8_t));
		_cw_check_ptr(mq_p);

		_cw_assert(mq_tryget(mq_p, &t));
		_cw_assert(mq_stop_put(mq_p) == FALSE);
		_cw_assert(mq_stop_put(mq_p));
		_cw_assert(mq_tryget(mq_p, &t));

		_cw_assert(1 == mq_put(mq_p, i));

		_cw_assert(mq_start_put(mq_p) == FALSE);
		_cw_assert(mq_start_put(mq_p));
		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_put(mq_p, j) == 0);

		_cw_assert(mq_stop_put(mq_p) == FALSE);
		_cw_assert(mq_stop_put(mq_p));
		_cw_assert(mq_put(mq_p, k) == 1);

		_cw_assert(mq_tryget(mq_p, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(mq_p, &t) == FALSE);
		_cw_assert(t == j);
		_cw_assert(mq_tryget(mq_p, &t));

		mq_delete(mq_p);
	}
	{
		cw_mq_t		*mq_p;
		cw_uint16_t	i = 1, j = 2, k = 3, t;

		mq_p = mq_new(NULL, sizeof(cw_uint16_t));
		_cw_check_ptr(mq_p);

		_cw_assert(mq_tryget(mq_p, &t));
		_cw_assert(mq_stop_put(mq_p) == FALSE);
		_cw_assert(mq_stop_put(mq_p));
		_cw_assert(mq_tryget(mq_p, &t));

		_cw_assert(1 == mq_put(mq_p, i));

		_cw_assert(mq_start_put(mq_p) == FALSE);
		_cw_assert(mq_start_put(mq_p));
		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_put(mq_p, j) == 0);

		_cw_assert(mq_stop_put(mq_p) == FALSE);
		_cw_assert(mq_stop_put(mq_p));
		_cw_assert(mq_put(mq_p, k) == 1);

		_cw_assert(mq_tryget(mq_p, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(mq_p, &t) == FALSE);
		_cw_assert(t == j);
		_cw_assert(mq_tryget(mq_p, &t));

		mq_delete(mq_p);
	}
	{
		cw_mq_t		*mq_p;
		cw_uint32_t	i = 1, j = 2, k = 3, t;

		mq_p = mq_new(NULL, sizeof(cw_uint32_t));
		_cw_check_ptr(mq_p);

		_cw_assert(mq_tryget(mq_p, &t));
		_cw_assert(mq_stop_put(mq_p) == FALSE);
		_cw_assert(mq_stop_put(mq_p));
		_cw_assert(mq_tryget(mq_p, &t));

		_cw_assert(1 == mq_put(mq_p, i));

		_cw_assert(mq_start_put(mq_p) == FALSE);
		_cw_assert(mq_start_put(mq_p));
		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_put(mq_p, j) == 0);

		_cw_assert(mq_stop_put(mq_p) == FALSE);
		_cw_assert(mq_stop_put(mq_p));
		_cw_assert(mq_put(mq_p, k) == 1);

		_cw_assert(mq_tryget(mq_p, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(mq_p, &t) == FALSE);
		_cw_assert(t == j);
		_cw_assert(mq_tryget(mq_p, &t));

		mq_delete(mq_p);
	}
	{
		cw_mq_t		*mq_p;
		cw_uint64_t	i = 1, j = 2, k = 3, t;

		mq_p = mq_new(NULL, sizeof(cw_uint64_t));
		_cw_check_ptr(mq_p);

		_cw_assert(mq_tryget(mq_p, &t));
		_cw_assert(mq_stop_put(mq_p) == FALSE);
		_cw_assert(mq_stop_put(mq_p));
		_cw_assert(mq_tryget(mq_p, &t));

		_cw_assert(1 == mq_put(mq_p, i));

		_cw_assert(mq_start_put(mq_p) == FALSE);
		_cw_assert(mq_start_put(mq_p));
		_cw_assert(mq_put(mq_p, i) == 0);
		_cw_assert(mq_put(mq_p, j) == 0);

		_cw_assert(mq_stop_put(mq_p) == FALSE);
		_cw_assert(mq_stop_put(mq_p));
		_cw_assert(mq_put(mq_p, k) == 1);

		_cw_assert(mq_tryget(mq_p, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(mq_p, &t) == FALSE);
		_cw_assert(t == j);
		_cw_assert(mq_tryget(mq_p, &t));

		mq_delete(mq_p);
	}

	_cw_out_put("Test end\n");
	libstash_shutdown();
	return 0;
}
