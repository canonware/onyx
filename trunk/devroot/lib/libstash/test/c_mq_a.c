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
		cw_mq_t mq, *mq_p;

		_cw_assert(&mq == mq_new(&mq));
		mq_delete(&mq);

		mq_p = mq_new(NULL);
		_cw_check_ptr(mq_p);
		mq_delete(mq_p);
	}

	/* mq_tryget(), mq_put(). */
	{
		cw_mq_t *mq_p;
		cw_uint32_t i, j, k;

		mq_p = mq_new(NULL);
		_cw_check_ptr(mq_p);

		_cw_assert(NULL == mq_tryget(mq_p));
		_cw_assert(0 == mq_put(mq_p, (const void *)&i));
		_cw_assert((void *)&i == mq_tryget(mq_p));
		_cw_assert(NULL == mq_tryget(mq_p));

		_cw_assert(0 == mq_put(mq_p, (const void *)&i));
		_cw_assert(0 == mq_put(mq_p, (const void *)&j));
		_cw_assert(0 == mq_put(mq_p, (const void *)&k));
		_cw_assert((void *)&i == mq_tryget(mq_p));
		_cw_assert(0 == mq_put(mq_p, (const void *)&i));
		_cw_assert((void *)&j == mq_tryget(mq_p));
		_cw_assert((void *)&k == mq_tryget(mq_p));
		_cw_assert((void *)&i == mq_tryget(mq_p));
		_cw_assert(NULL == mq_tryget(mq_p));

		mq_delete(mq_p);
	}

	/* mq_get(), mq_put(), mq_tryget(). */
	{
		cw_mq_t *mq_p;
		cw_uint32_t i, j, k;

		mq_p = mq_new(NULL);
		_cw_check_ptr(mq_p);

		_cw_assert(NULL == mq_tryget(mq_p));
		_cw_assert(0 == mq_put(mq_p, (const void *)&i));
		_cw_assert((void *)&i == mq_get(mq_p));
		_cw_assert(NULL == mq_tryget(mq_p));

		_cw_assert(0 == mq_put(mq_p, (const void *)&i));
		_cw_assert(0 == mq_put(mq_p, (const void *)&j));
		_cw_assert(0 == mq_put(mq_p, (const void *)&k));
		_cw_assert((void *)&i == mq_get(mq_p));
		_cw_assert(0 == mq_put(mq_p, (const void *)&i));
		_cw_assert((void *)&j == mq_get(mq_p));
		_cw_assert((void *)&k == mq_get(mq_p));
		_cw_assert((void *)&i == mq_get(mq_p));
		_cw_assert(NULL == mq_tryget(mq_p));

		mq_delete(mq_p);
	}

	/* mq_start_get(), mq_stop_get(), mq_tryget(), mq_put(). */
	{
		cw_mq_t *mq_p;
		cw_uint32_t i, j, k;

		mq_p = mq_new(NULL);
		_cw_check_ptr(mq_p);

		_cw_assert(FALSE == mq_stop_get(mq_p));
		_cw_assert(TRUE == mq_stop_get(mq_p));
		_cw_assert(NULL == mq_tryget(mq_p));

		_cw_assert(FALSE == mq_start_get(mq_p));
		_cw_assert(TRUE == mq_start_get(mq_p));
		_cw_assert(NULL == mq_tryget(mq_p));

		_cw_assert(0 == mq_put(mq_p, (const void *)&i));

		_cw_assert(FALSE == mq_stop_get(mq_p));
		_cw_assert(TRUE == mq_stop_get(mq_p));
		_cw_assert(NULL == mq_tryget(mq_p));
		_cw_assert(NULL == mq_get(mq_p));

		_cw_assert(FALSE == mq_start_get(mq_p));
		_cw_assert(TRUE == mq_start_get(mq_p));
		_cw_assert((void *)&i == mq_tryget(mq_p));
		_cw_assert(NULL == mq_tryget(mq_p));

		_cw_assert(0 == mq_put(mq_p, (const void *)&i));
		_cw_assert(0 == mq_put(mq_p, (const void *)&j));
		_cw_assert(0 == mq_put(mq_p, (const void *)&k));

		mq_delete(mq_p);
	}

	/* mq_start_put(), mq_stop_put(), mq_try_get(), mq_put(). */
	{
		cw_mq_t *mq_p;
		cw_uint32_t i, j, k;

		mq_p = mq_new(NULL);
		_cw_check_ptr(mq_p);

		_cw_assert(NULL == mq_tryget(mq_p));
		_cw_assert(FALSE == mq_stop_put(mq_p));
		_cw_assert(TRUE == mq_stop_put(mq_p));
		_cw_assert(NULL == mq_tryget(mq_p));

		_cw_assert(1 == mq_put(mq_p, (const void *)&i));

		_cw_assert(FALSE == mq_start_put(mq_p));
		_cw_assert(TRUE == mq_start_put(mq_p));
		_cw_assert(0 == mq_put(mq_p, (const void *)&i));
		_cw_assert(0 == mq_put(mq_p, (const void *)&j));

		_cw_assert(FALSE == mq_stop_put(mq_p));
		_cw_assert(TRUE == mq_stop_put(mq_p));
		_cw_assert(1 == mq_put(mq_p, (const void *)&k));

		_cw_assert((void *)&i == mq_tryget(mq_p));
		_cw_assert((void *)&j == mq_tryget(mq_p));
		_cw_assert(NULL == mq_tryget(mq_p));

		mq_delete(mq_p);
	}

	_cw_out_put("Test end\n");
	libstash_shutdown();
	return 0;
}
