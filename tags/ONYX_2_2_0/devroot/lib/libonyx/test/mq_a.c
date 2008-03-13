/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Test the mq (message queue) class.
 *
 ******************************************************************************/

#include "../include/libonyx/libonyx.h"

int
main()
{
	libonyx_init();
	fprintf(stderr, "Test begin\n");

	/* mq_new(), mq_delete(). */
	{
		cw_mq_t	mq;

		mq_new(&mq, cw_g_mem, sizeof(cw_uint32_t));
		mq_delete(&mq);

		mq_new(&mq, cw_g_mem, sizeof(cw_uint64_t));
		mq_delete(&mq);
	}

	/* mq_tryget(), mq_put(). */
	{
		cw_mq_t		mq;
		cw_uint8_t	i = 1, j = 2, k = 3, t;

		mq_new(&mq, cw_g_mem, sizeof(cw_uint8_t));

		_cw_assert(mq_tryget(&mq, &t));
		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_tryget(&mq, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(&mq, &t));

		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_put(&mq, j) == 0);
		_cw_assert(mq_put(&mq, k) == 0);
		_cw_assert(mq_tryget(&mq, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_tryget(&mq, &t) == FALSE);
		_cw_assert(t == j);
		_cw_assert(mq_tryget(&mq, &t) == FALSE);
		_cw_assert(t == k);
		_cw_assert(mq_tryget(&mq, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(&mq, &t));

		mq_delete(&mq);
	}
	{
		cw_mq_t		mq;
		cw_uint16_t	i = 1, j = 2, k = 3, t;

		mq_new(&mq, cw_g_mem, sizeof(cw_uint16_t));

		_cw_assert(mq_tryget(&mq, &t));
		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_tryget(&mq, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(&mq, &t));

		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_put(&mq, j) == 0);
		_cw_assert(mq_put(&mq, k) == 0);
		_cw_assert(mq_tryget(&mq, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_tryget(&mq, &t) == FALSE);
		_cw_assert(t == j);
		_cw_assert(mq_tryget(&mq, &t) == FALSE);
		_cw_assert(t == k);
		_cw_assert(mq_tryget(&mq, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(&mq, &t));

		mq_delete(&mq);
	}
	{
		cw_mq_t		mq;
		cw_uint32_t	i = 1, j = 2, k = 3, t;

		mq_new(&mq, cw_g_mem, sizeof(cw_uint32_t));

		_cw_assert(mq_tryget(&mq, &t));
		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_tryget(&mq, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(&mq, &t));

		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_put(&mq, j) == 0);
		_cw_assert(mq_put(&mq, k) == 0);
		_cw_assert(mq_tryget(&mq, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_tryget(&mq, &t) == FALSE);
		_cw_assert(t == j);
		_cw_assert(mq_tryget(&mq, &t) == FALSE);
		_cw_assert(t == k);
		_cw_assert(mq_tryget(&mq, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(&mq, &t));

		mq_delete(&mq);
	}
	{
		cw_mq_t		mq;
		cw_uint64_t	i = 1, j = 2, k = 3, t;

		mq_new(&mq, cw_g_mem, sizeof(cw_uint64_t));

		_cw_assert(mq_tryget(&mq, &t));
		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_tryget(&mq, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(&mq, &t));

		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_put(&mq, j) == 0);
		_cw_assert(mq_put(&mq, k) == 0);
		_cw_assert(mq_tryget(&mq, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_tryget(&mq, &t) == FALSE);
		_cw_assert(t == j);
		_cw_assert(mq_tryget(&mq, &t) == FALSE);
		_cw_assert(t == k);
		_cw_assert(mq_tryget(&mq, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(&mq, &t));

		mq_delete(&mq);
	}

	/* mq_timedget(), mq_put(). */
	{
		cw_mq_t		mq;
		cw_uint8_t	i = 1, j = 2, k = 3, t;
		struct timespec	timeout = {0, 10000};

		mq_new(&mq, cw_g_mem, sizeof(cw_uint8_t));

		_cw_assert(mq_timedget(&mq, &timeout));
		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_timedget(&mq, &timeout, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_timedget(&mq, &timeout));

		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_put(&mq, j) == 0);
		_cw_assert(mq_put(&mq, k) == 0);
		_cw_assert(mq_timedget(&mq, &timeout, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_timedget(&mq, &timeout, &t) == FALSE);
		_cw_assert(t == j);
		_cw_assert(mq_timedget(&mq, &timeout, &t) == FALSE);
		_cw_assert(t == k);
		_cw_assert(mq_timedget(&mq, &timeout, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_timedget(&mq, &timeout, &t));

		mq_delete(&mq);
	}
	{
		cw_mq_t		mq;
		cw_uint16_t	i = 1, j = 2, k = 3, t;
		struct timespec	timeout = {0, 10000};

		mq_new(&mq, cw_g_mem, sizeof(cw_uint16_t));

		_cw_assert(mq_timedget(&mq, &timeout));
		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_timedget(&mq, &timeout, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_timedget(&mq, &timeout));

		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_put(&mq, j) == 0);
		_cw_assert(mq_put(&mq, k) == 0);
		_cw_assert(mq_timedget(&mq, &timeout, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_timedget(&mq, &timeout, &t) == FALSE);
		_cw_assert(t == j);
		_cw_assert(mq_timedget(&mq, &timeout, &t) == FALSE);
		_cw_assert(t == k);
		_cw_assert(mq_timedget(&mq, &timeout, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_timedget(&mq, &timeout, &t));

		mq_delete(&mq);
	}
	{
		cw_mq_t		mq;
		cw_uint32_t	i = 1, j = 2, k = 3, t;
		struct timespec	timeout = {0, 10000};

		mq_new(&mq, cw_g_mem, sizeof(cw_uint32_t));

		_cw_assert(mq_timedget(&mq, &timeout));
		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_timedget(&mq, &timeout, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_timedget(&mq, &timeout));

		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_put(&mq, j) == 0);
		_cw_assert(mq_put(&mq, k) == 0);
		_cw_assert(mq_timedget(&mq, &timeout, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_timedget(&mq, &timeout, &t) == FALSE);
		_cw_assert(t == j);
		_cw_assert(mq_timedget(&mq, &timeout, &t) == FALSE);
		_cw_assert(t == k);
		_cw_assert(mq_timedget(&mq, &timeout, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_timedget(&mq, &timeout, &t));

		mq_delete(&mq);
	}
	{
		cw_mq_t		mq;
		cw_uint64_t	i = 1, j = 2, k = 3, t;
		struct timespec	timeout = {0, 10000};

		mq_new(&mq, cw_g_mem, sizeof(cw_uint64_t));

		_cw_assert(mq_timedget(&mq, &timeout));
		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_timedget(&mq, &timeout, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_timedget(&mq, &timeout));

		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_put(&mq, j) == 0);
		_cw_assert(mq_put(&mq, k) == 0);
		_cw_assert(mq_timedget(&mq, &timeout, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_timedget(&mq, &timeout, &t) == FALSE);
		_cw_assert(t == j);
		_cw_assert(mq_timedget(&mq, &timeout, &t) == FALSE);
		_cw_assert(t == k);
		_cw_assert(mq_timedget(&mq, &timeout, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_timedget(&mq, &timeout, &t));

		mq_delete(&mq);
	}

	/* mq_get(), mq_put(), mq_tryget(). */
	{
		cw_mq_t		mq;
		cw_uint8_t	i = 1, j = 2, k = 3, t;

		mq_new(&mq, cw_g_mem, sizeof(cw_uint8_t));

		_cw_assert(mq_tryget(&mq, &t));
		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_get(&mq, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(&mq, &t));

		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_put(&mq, j) == 0);
		_cw_assert(mq_put(&mq, k) == 0);
		_cw_assert(mq_get(&mq, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_get(&mq, &t) == FALSE);
		_cw_assert(t == j);
		_cw_assert(mq_get(&mq, &t) == FALSE);
		_cw_assert(t == k);
		_cw_assert(mq_get(&mq, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(&mq, &t));

		mq_delete(&mq);
	}
	{
		cw_mq_t		mq;
		cw_uint16_t	i = 1, j = 2, k = 3, t;

		mq_new(&mq, cw_g_mem, sizeof(cw_uint16_t));

		_cw_assert(mq_tryget(&mq, &t));
		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_get(&mq, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(&mq, &t));

		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_put(&mq, j) == 0);
		_cw_assert(mq_put(&mq, k) == 0);
		_cw_assert(mq_get(&mq, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_get(&mq, &t) == FALSE);
		_cw_assert(t == j);
		_cw_assert(mq_get(&mq, &t) == FALSE);
		_cw_assert(t == k);
		_cw_assert(mq_get(&mq, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(&mq, &t));

		mq_delete(&mq);
	}
	{
		cw_mq_t		mq;
		cw_uint32_t	i = 1, j = 2, k = 3, t;

		mq_new(&mq, cw_g_mem, sizeof(cw_uint32_t));

		_cw_assert(mq_tryget(&mq, &t));
		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_get(&mq, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(&mq, &t));

		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_put(&mq, j) == 0);
		_cw_assert(mq_put(&mq, k) == 0);
		_cw_assert(mq_get(&mq, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_get(&mq, &t) == FALSE);
		_cw_assert(t == j);
		_cw_assert(mq_get(&mq, &t) == FALSE);
		_cw_assert(t == k);
		_cw_assert(mq_get(&mq, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(&mq, &t));

		mq_delete(&mq);
	}
	{
		cw_mq_t		mq;
		cw_uint64_t	i = 1, j = 2, k = 3, t;

		mq_new(&mq, cw_g_mem, sizeof(cw_uint64_t));

		_cw_assert(mq_tryget(&mq, &t));
		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_get(&mq, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(&mq, &t));

		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_put(&mq, j) == 0);
		_cw_assert(mq_put(&mq, k) == 0);
		_cw_assert(mq_get(&mq, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_get(&mq, &t) == FALSE);
		_cw_assert(t == j);
		_cw_assert(mq_get(&mq, &t) == FALSE);
		_cw_assert(t == k);
		_cw_assert(mq_get(&mq, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(&mq, &t));

		mq_delete(&mq);
	}

	/* mq_get_start(), mq_get_stop(), mq_tryget(), mq_put(). */
	{
		cw_mq_t		mq;
		cw_uint8_t	i = 1, j = 2, k = 3, t;

		mq_new(&mq, cw_g_mem, sizeof(cw_uint8_t));

		_cw_assert(mq_get_stop(&mq) == FALSE);
		_cw_assert(mq_get_stop(&mq));
		_cw_assert(mq_tryget(&mq, &t));

		_cw_assert(mq_get_start(&mq) == FALSE);
		_cw_assert(mq_get_start(&mq));
		_cw_assert(mq_tryget(&mq, &t));

		_cw_assert(mq_put(&mq, i) == 0);

		_cw_assert(mq_get_stop(&mq) == FALSE);
		_cw_assert(mq_get_stop(&mq));
		_cw_assert(mq_tryget(&mq, &t));
		_cw_assert(mq_get(&mq, &t));

		_cw_assert(mq_get_start(&mq) == FALSE);
		_cw_assert(mq_get_start(&mq));
		_cw_assert(mq_tryget(&mq, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(&mq, &t));

		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_put(&mq, j) == 0);
		_cw_assert(mq_put(&mq, k) == 0);

		mq_delete(&mq);
	}
	{
		cw_mq_t		mq;
		cw_uint16_t	i = 1, j = 2, k = 3, t;

		mq_new(&mq, cw_g_mem, sizeof(cw_uint16_t));

		_cw_assert(mq_get_stop(&mq) == FALSE);
		_cw_assert(mq_get_stop(&mq));
		_cw_assert(mq_tryget(&mq, &t));

		_cw_assert(mq_get_start(&mq) == FALSE);
		_cw_assert(mq_get_start(&mq));
		_cw_assert(mq_tryget(&mq, &t));

		_cw_assert(mq_put(&mq, i) == 0);

		_cw_assert(mq_get_stop(&mq) == FALSE);
		_cw_assert(mq_get_stop(&mq));
		_cw_assert(mq_tryget(&mq, &t));
		_cw_assert(mq_get(&mq, &t));

		_cw_assert(mq_get_start(&mq) == FALSE);
		_cw_assert(mq_get_start(&mq));
		_cw_assert(mq_tryget(&mq, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(&mq, &t));

		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_put(&mq, j) == 0);
		_cw_assert(mq_put(&mq, k) == 0);

		mq_delete(&mq);
	}
	{
		cw_mq_t		mq;
		cw_uint32_t	i = 1, j = 2, k = 3, t;

		mq_new(&mq, cw_g_mem, sizeof(cw_uint32_t));

		_cw_assert(mq_get_stop(&mq) == FALSE);
		_cw_assert(mq_get_stop(&mq));
		_cw_assert(mq_tryget(&mq, &t));

		_cw_assert(mq_get_start(&mq) == FALSE);
		_cw_assert(mq_get_start(&mq));
		_cw_assert(mq_tryget(&mq, &t));

		_cw_assert(mq_put(&mq, i) == 0);

		_cw_assert(mq_get_stop(&mq) == FALSE);
		_cw_assert(mq_get_stop(&mq));
		_cw_assert(mq_tryget(&mq, &t));
		_cw_assert(mq_get(&mq, &t));

		_cw_assert(mq_get_start(&mq) == FALSE);
		_cw_assert(mq_get_start(&mq));
		_cw_assert(mq_tryget(&mq, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(&mq, &t));

		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_put(&mq, j) == 0);
		_cw_assert(mq_put(&mq, k) == 0);

		mq_delete(&mq);
	}
	{
		cw_mq_t		mq;
		cw_uint64_t	i = 1, j = 2, k = 3, t;

		mq_new(&mq, cw_g_mem, sizeof(cw_uint64_t));

		_cw_assert(mq_get_stop(&mq) == FALSE);
		_cw_assert(mq_get_stop(&mq));
		_cw_assert(mq_tryget(&mq, &t));

		_cw_assert(mq_get_start(&mq) == FALSE);
		_cw_assert(mq_get_start(&mq));
		_cw_assert(mq_tryget(&mq, &t));

		_cw_assert(mq_put(&mq, i) == 0);

		_cw_assert(mq_get_stop(&mq) == FALSE);
		_cw_assert(mq_get_stop(&mq));
		_cw_assert(mq_tryget(&mq, &t));
		_cw_assert(mq_get(&mq, &t));

		_cw_assert(mq_get_start(&mq) == FALSE);
		_cw_assert(mq_get_start(&mq));
		_cw_assert(mq_tryget(&mq, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(&mq, &t));

		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_put(&mq, j) == 0);
		_cw_assert(mq_put(&mq, k) == 0);

		mq_delete(&mq);
	}

	/* mq_put_start(), mq_put_stop(), mq_tryget(), mq_put(). */
	{
		cw_mq_t		mq;
		cw_uint8_t	i = 1, j = 2, k = 3, t;

		mq_new(&mq, cw_g_mem, sizeof(cw_uint8_t));

		_cw_assert(mq_tryget(&mq, &t));
		_cw_assert(mq_put_stop(&mq) == FALSE);
		_cw_assert(mq_put_stop(&mq));
		_cw_assert(mq_tryget(&mq, &t));

		_cw_assert(1 == mq_put(&mq, i));

		_cw_assert(mq_put_start(&mq) == FALSE);
		_cw_assert(mq_put_start(&mq));
		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_put(&mq, j) == 0);

		_cw_assert(mq_put_stop(&mq) == FALSE);
		_cw_assert(mq_put_stop(&mq));
		_cw_assert(mq_put(&mq, k) == 1);

		_cw_assert(mq_tryget(&mq, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(&mq, &t) == FALSE);
		_cw_assert(t == j);
		_cw_assert(mq_tryget(&mq, &t));

		mq_delete(&mq);
	}
	{
		cw_mq_t		mq;
		cw_uint16_t	i = 1, j = 2, k = 3, t;

		mq_new(&mq, cw_g_mem, sizeof(cw_uint16_t));

		_cw_assert(mq_tryget(&mq, &t));
		_cw_assert(mq_put_stop(&mq) == FALSE);
		_cw_assert(mq_put_stop(&mq));
		_cw_assert(mq_tryget(&mq, &t));

		_cw_assert(1 == mq_put(&mq, i));

		_cw_assert(mq_put_start(&mq) == FALSE);
		_cw_assert(mq_put_start(&mq));
		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_put(&mq, j) == 0);

		_cw_assert(mq_put_stop(&mq) == FALSE);
		_cw_assert(mq_put_stop(&mq));
		_cw_assert(mq_put(&mq, k) == 1);

		_cw_assert(mq_tryget(&mq, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(&mq, &t) == FALSE);
		_cw_assert(t == j);
		_cw_assert(mq_tryget(&mq, &t));

		mq_delete(&mq);
	}
	{
		cw_mq_t		mq;
		cw_uint32_t	i = 1, j = 2, k = 3, t;

		mq_new(&mq, cw_g_mem, sizeof(cw_uint32_t));

		_cw_assert(mq_tryget(&mq, &t));
		_cw_assert(mq_put_stop(&mq) == FALSE);
		_cw_assert(mq_put_stop(&mq));
		_cw_assert(mq_tryget(&mq, &t));

		_cw_assert(1 == mq_put(&mq, i));

		_cw_assert(mq_put_start(&mq) == FALSE);
		_cw_assert(mq_put_start(&mq));
		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_put(&mq, j) == 0);

		_cw_assert(mq_put_stop(&mq) == FALSE);
		_cw_assert(mq_put_stop(&mq));
		_cw_assert(mq_put(&mq, k) == 1);

		_cw_assert(mq_tryget(&mq, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(&mq, &t) == FALSE);
		_cw_assert(t == j);
		_cw_assert(mq_tryget(&mq, &t));

		mq_delete(&mq);
	}
	{
		cw_mq_t		mq;
		cw_uint64_t	i = 1, j = 2, k = 3, t;

		mq_new(&mq, cw_g_mem, sizeof(cw_uint64_t));

		_cw_assert(mq_tryget(&mq, &t));
		_cw_assert(mq_put_stop(&mq) == FALSE);
		_cw_assert(mq_put_stop(&mq));
		_cw_assert(mq_tryget(&mq, &t));

		_cw_assert(1 == mq_put(&mq, i));

		_cw_assert(mq_put_start(&mq) == FALSE);
		_cw_assert(mq_put_start(&mq));
		_cw_assert(mq_put(&mq, i) == 0);
		_cw_assert(mq_put(&mq, j) == 0);

		_cw_assert(mq_put_stop(&mq) == FALSE);
		_cw_assert(mq_put_stop(&mq));
		_cw_assert(mq_put(&mq, k) == 1);

		_cw_assert(mq_tryget(&mq, &t) == FALSE);
		_cw_assert(t == i);
		_cw_assert(mq_tryget(&mq, &t) == FALSE);
		_cw_assert(t == j);
		_cw_assert(mq_tryget(&mq, &t));

		mq_delete(&mq);
	}

	fprintf(stderr, "Test end\n");
	libonyx_shutdown();
	return 0;
}
