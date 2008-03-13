/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: Onyx <Version = onyx>
 *
 * Test the mq (message queue) class.
 *
 ******************************************************************************/

#include "../include/libonyx/libonyx.h"

int
main()
{
    libonyx_init(0, NULL, NULL);
    fprintf(stderr, "Test begin\n");

    /* mq_new(), mq_delete(). */
    {
	cw_mq_t mq;

	mq_new(&mq, cw_g_mema, sizeof(uint32_t));
	mq_delete(&mq);

	mq_new(&mq, cw_g_mema, sizeof(uint64_t));
	mq_delete(&mq);
    }

    /* mq_tryget(), mq_put(). */
    {
	cw_mq_t mq;
	uint8_t i = 1, j = 2, k = 3, t;

	mq_new(&mq, cw_g_mema, sizeof(uint8_t));

	cw_assert(mq_tryget(&mq, &t));
	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_tryget(&mq, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_tryget(&mq, &t));

	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_put(&mq, j) == 0);
	cw_assert(mq_put(&mq, k) == 0);
	cw_assert(mq_tryget(&mq, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_tryget(&mq, &t) == false);
	cw_assert(t == j);
	cw_assert(mq_tryget(&mq, &t) == false);
	cw_assert(t == k);
	cw_assert(mq_tryget(&mq, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_tryget(&mq, &t));

	mq_delete(&mq);
    }
    {
	cw_mq_t mq;
	uint16_t i = 1, j = 2, k = 3, t;

	mq_new(&mq, cw_g_mema, sizeof(uint16_t));

	cw_assert(mq_tryget(&mq, &t));
	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_tryget(&mq, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_tryget(&mq, &t));

	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_put(&mq, j) == 0);
	cw_assert(mq_put(&mq, k) == 0);
	cw_assert(mq_tryget(&mq, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_tryget(&mq, &t) == false);
	cw_assert(t == j);
	cw_assert(mq_tryget(&mq, &t) == false);
	cw_assert(t == k);
	cw_assert(mq_tryget(&mq, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_tryget(&mq, &t));

	mq_delete(&mq);
    }
    {
	cw_mq_t mq;
	uint32_t i = 1, j = 2, k = 3, t;

	mq_new(&mq, cw_g_mema, sizeof(uint32_t));

	cw_assert(mq_tryget(&mq, &t));
	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_tryget(&mq, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_tryget(&mq, &t));

	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_put(&mq, j) == 0);
	cw_assert(mq_put(&mq, k) == 0);
	cw_assert(mq_tryget(&mq, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_tryget(&mq, &t) == false);
	cw_assert(t == j);
	cw_assert(mq_tryget(&mq, &t) == false);
	cw_assert(t == k);
	cw_assert(mq_tryget(&mq, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_tryget(&mq, &t));

	mq_delete(&mq);
    }
    {
	cw_mq_t mq;
	uint64_t i = 1, j = 2, k = 3, t;

	mq_new(&mq, cw_g_mema, sizeof(uint64_t));

	cw_assert(mq_tryget(&mq, &t));
	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_tryget(&mq, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_tryget(&mq, &t));

	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_put(&mq, j) == 0);
	cw_assert(mq_put(&mq, k) == 0);
	cw_assert(mq_tryget(&mq, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_tryget(&mq, &t) == false);
	cw_assert(t == j);
	cw_assert(mq_tryget(&mq, &t) == false);
	cw_assert(t == k);
	cw_assert(mq_tryget(&mq, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_tryget(&mq, &t));

	mq_delete(&mq);
    }

    /* mq_timedget(), mq_put(). */
    {
	cw_mq_t mq;
	uint8_t i = 1, j = 2, k = 3, t;
	struct timespec timeout = {0, 10000};

	mq_new(&mq, cw_g_mema, sizeof(uint8_t));

	cw_assert(mq_timedget(&mq, &timeout));
	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_timedget(&mq, &timeout, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_timedget(&mq, &timeout));

	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_put(&mq, j) == 0);
	cw_assert(mq_put(&mq, k) == 0);
	cw_assert(mq_timedget(&mq, &timeout, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_timedget(&mq, &timeout, &t) == false);
	cw_assert(t == j);
	cw_assert(mq_timedget(&mq, &timeout, &t) == false);
	cw_assert(t == k);
	cw_assert(mq_timedget(&mq, &timeout, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_timedget(&mq, &timeout, &t));

	mq_delete(&mq);
    }
    {
	cw_mq_t mq;
	uint16_t i = 1, j = 2, k = 3, t;
	struct timespec timeout = {0, 10000};

	mq_new(&mq, cw_g_mema, sizeof(uint16_t));

	cw_assert(mq_timedget(&mq, &timeout));
	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_timedget(&mq, &timeout, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_timedget(&mq, &timeout));

	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_put(&mq, j) == 0);
	cw_assert(mq_put(&mq, k) == 0);
	cw_assert(mq_timedget(&mq, &timeout, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_timedget(&mq, &timeout, &t) == false);
	cw_assert(t == j);
	cw_assert(mq_timedget(&mq, &timeout, &t) == false);
	cw_assert(t == k);
	cw_assert(mq_timedget(&mq, &timeout, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_timedget(&mq, &timeout, &t));

	mq_delete(&mq);
    }
    {
	cw_mq_t mq;
	uint32_t i = 1, j = 2, k = 3, t;
	struct timespec timeout = {0, 10000};

	mq_new(&mq, cw_g_mema, sizeof(uint32_t));

	cw_assert(mq_timedget(&mq, &timeout));
	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_timedget(&mq, &timeout, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_timedget(&mq, &timeout));

	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_put(&mq, j) == 0);
	cw_assert(mq_put(&mq, k) == 0);
	cw_assert(mq_timedget(&mq, &timeout, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_timedget(&mq, &timeout, &t) == false);
	cw_assert(t == j);
	cw_assert(mq_timedget(&mq, &timeout, &t) == false);
	cw_assert(t == k);
	cw_assert(mq_timedget(&mq, &timeout, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_timedget(&mq, &timeout, &t));

	mq_delete(&mq);
    }
    {
	cw_mq_t mq;
	uint64_t i = 1, j = 2, k = 3, t;
	struct timespec timeout = {0, 10000};

	mq_new(&mq, cw_g_mema, sizeof(uint64_t));

	cw_assert(mq_timedget(&mq, &timeout));
	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_timedget(&mq, &timeout, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_timedget(&mq, &timeout));

	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_put(&mq, j) == 0);
	cw_assert(mq_put(&mq, k) == 0);
	cw_assert(mq_timedget(&mq, &timeout, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_timedget(&mq, &timeout, &t) == false);
	cw_assert(t == j);
	cw_assert(mq_timedget(&mq, &timeout, &t) == false);
	cw_assert(t == k);
	cw_assert(mq_timedget(&mq, &timeout, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_timedget(&mq, &timeout, &t));

	mq_delete(&mq);
    }

    /* mq_get(), mq_put(), mq_tryget(). */
    {
	cw_mq_t mq;
	uint8_t i = 1, j = 2, k = 3, t;

	mq_new(&mq, cw_g_mema, sizeof(uint8_t));

	cw_assert(mq_tryget(&mq, &t));
	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_get(&mq, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_tryget(&mq, &t));

	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_put(&mq, j) == 0);
	cw_assert(mq_put(&mq, k) == 0);
	cw_assert(mq_get(&mq, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_get(&mq, &t) == false);
	cw_assert(t == j);
	cw_assert(mq_get(&mq, &t) == false);
	cw_assert(t == k);
	cw_assert(mq_get(&mq, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_tryget(&mq, &t));

	mq_delete(&mq);
    }
    {
	cw_mq_t mq;
	uint16_t i = 1, j = 2, k = 3, t;

	mq_new(&mq, cw_g_mema, sizeof(uint16_t));

	cw_assert(mq_tryget(&mq, &t));
	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_get(&mq, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_tryget(&mq, &t));

	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_put(&mq, j) == 0);
	cw_assert(mq_put(&mq, k) == 0);
	cw_assert(mq_get(&mq, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_get(&mq, &t) == false);
	cw_assert(t == j);
	cw_assert(mq_get(&mq, &t) == false);
	cw_assert(t == k);
	cw_assert(mq_get(&mq, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_tryget(&mq, &t));

	mq_delete(&mq);
    }
    {
	cw_mq_t mq;
	uint32_t i = 1, j = 2, k = 3, t;

	mq_new(&mq, cw_g_mema, sizeof(uint32_t));

	cw_assert(mq_tryget(&mq, &t));
	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_get(&mq, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_tryget(&mq, &t));

	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_put(&mq, j) == 0);
	cw_assert(mq_put(&mq, k) == 0);
	cw_assert(mq_get(&mq, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_get(&mq, &t) == false);
	cw_assert(t == j);
	cw_assert(mq_get(&mq, &t) == false);
	cw_assert(t == k);
	cw_assert(mq_get(&mq, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_tryget(&mq, &t));

	mq_delete(&mq);
    }
    {
	cw_mq_t mq;
	uint64_t i = 1, j = 2, k = 3, t;

	mq_new(&mq, cw_g_mema, sizeof(uint64_t));

	cw_assert(mq_tryget(&mq, &t));
	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_get(&mq, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_tryget(&mq, &t));

	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_put(&mq, j) == 0);
	cw_assert(mq_put(&mq, k) == 0);
	cw_assert(mq_get(&mq, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_get(&mq, &t) == false);
	cw_assert(t == j);
	cw_assert(mq_get(&mq, &t) == false);
	cw_assert(t == k);
	cw_assert(mq_get(&mq, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_tryget(&mq, &t));

	mq_delete(&mq);
    }

    /* mq_get_start(), mq_get_stop(), mq_tryget(), mq_put(). */
    {
	cw_mq_t mq;
	uint8_t i = 1, j = 2, k = 3, t;

	mq_new(&mq, cw_g_mema, sizeof(uint8_t));

	cw_assert(mq_get_stop(&mq) == false);
	cw_assert(mq_get_stop(&mq));
	cw_assert(mq_tryget(&mq, &t));

	cw_assert(mq_get_start(&mq) == false);
	cw_assert(mq_get_start(&mq));
	cw_assert(mq_tryget(&mq, &t));

	cw_assert(mq_put(&mq, i) == 0);

	cw_assert(mq_get_stop(&mq) == false);
	cw_assert(mq_get_stop(&mq));
	cw_assert(mq_tryget(&mq, &t));
	cw_assert(mq_get(&mq, &t));

	cw_assert(mq_get_start(&mq) == false);
	cw_assert(mq_get_start(&mq));
	cw_assert(mq_tryget(&mq, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_tryget(&mq, &t));

	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_put(&mq, j) == 0);
	cw_assert(mq_put(&mq, k) == 0);

	mq_delete(&mq);
    }
    {
	cw_mq_t mq;
	uint16_t i = 1, j = 2, k = 3, t;

	mq_new(&mq, cw_g_mema, sizeof(uint16_t));

	cw_assert(mq_get_stop(&mq) == false);
	cw_assert(mq_get_stop(&mq));
	cw_assert(mq_tryget(&mq, &t));

	cw_assert(mq_get_start(&mq) == false);
	cw_assert(mq_get_start(&mq));
	cw_assert(mq_tryget(&mq, &t));

	cw_assert(mq_put(&mq, i) == 0);

	cw_assert(mq_get_stop(&mq) == false);
	cw_assert(mq_get_stop(&mq));
	cw_assert(mq_tryget(&mq, &t));
	cw_assert(mq_get(&mq, &t));

	cw_assert(mq_get_start(&mq) == false);
	cw_assert(mq_get_start(&mq));
	cw_assert(mq_tryget(&mq, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_tryget(&mq, &t));

	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_put(&mq, j) == 0);
	cw_assert(mq_put(&mq, k) == 0);

	mq_delete(&mq);
    }
    {
	cw_mq_t mq;
	uint32_t i = 1, j = 2, k = 3, t;

	mq_new(&mq, cw_g_mema, sizeof(uint32_t));

	cw_assert(mq_get_stop(&mq) == false);
	cw_assert(mq_get_stop(&mq));
	cw_assert(mq_tryget(&mq, &t));

	cw_assert(mq_get_start(&mq) == false);
	cw_assert(mq_get_start(&mq));
	cw_assert(mq_tryget(&mq, &t));

	cw_assert(mq_put(&mq, i) == 0);

	cw_assert(mq_get_stop(&mq) == false);
	cw_assert(mq_get_stop(&mq));
	cw_assert(mq_tryget(&mq, &t));
	cw_assert(mq_get(&mq, &t));

	cw_assert(mq_get_start(&mq) == false);
	cw_assert(mq_get_start(&mq));
	cw_assert(mq_tryget(&mq, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_tryget(&mq, &t));

	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_put(&mq, j) == 0);
	cw_assert(mq_put(&mq, k) == 0);

	mq_delete(&mq);
    }
    {
	cw_mq_t mq;
	uint64_t i = 1, j = 2, k = 3, t;

	mq_new(&mq, cw_g_mema, sizeof(uint64_t));

	cw_assert(mq_get_stop(&mq) == false);
	cw_assert(mq_get_stop(&mq));
	cw_assert(mq_tryget(&mq, &t));

	cw_assert(mq_get_start(&mq) == false);
	cw_assert(mq_get_start(&mq));
	cw_assert(mq_tryget(&mq, &t));

	cw_assert(mq_put(&mq, i) == 0);

	cw_assert(mq_get_stop(&mq) == false);
	cw_assert(mq_get_stop(&mq));
	cw_assert(mq_tryget(&mq, &t));
	cw_assert(mq_get(&mq, &t));

	cw_assert(mq_get_start(&mq) == false);
	cw_assert(mq_get_start(&mq));
	cw_assert(mq_tryget(&mq, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_tryget(&mq, &t));

	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_put(&mq, j) == 0);
	cw_assert(mq_put(&mq, k) == 0);

	mq_delete(&mq);
    }

    /* mq_put_start(), mq_put_stop(), mq_tryget(), mq_put(). */
    {
	cw_mq_t mq;
	uint8_t i = 1, j = 2, k = 3, t;

	mq_new(&mq, cw_g_mema, sizeof(uint8_t));

	cw_assert(mq_tryget(&mq, &t));
	cw_assert(mq_put_stop(&mq) == false);
	cw_assert(mq_put_stop(&mq));
	cw_assert(mq_tryget(&mq, &t));

	cw_assert(1 == mq_put(&mq, i));

	cw_assert(mq_put_start(&mq) == false);
	cw_assert(mq_put_start(&mq));
	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_put(&mq, j) == 0);

	cw_assert(mq_put_stop(&mq) == false);
	cw_assert(mq_put_stop(&mq));
	cw_assert(mq_put(&mq, k) == 1);

	cw_assert(mq_tryget(&mq, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_tryget(&mq, &t) == false);
	cw_assert(t == j);
	cw_assert(mq_tryget(&mq, &t));

	mq_delete(&mq);
    }
    {
	cw_mq_t mq;
	uint16_t i = 1, j = 2, k = 3, t;

	mq_new(&mq, cw_g_mema, sizeof(uint16_t));

	cw_assert(mq_tryget(&mq, &t));
	cw_assert(mq_put_stop(&mq) == false);
	cw_assert(mq_put_stop(&mq));
	cw_assert(mq_tryget(&mq, &t));

	cw_assert(1 == mq_put(&mq, i));

	cw_assert(mq_put_start(&mq) == false);
	cw_assert(mq_put_start(&mq));
	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_put(&mq, j) == 0);

	cw_assert(mq_put_stop(&mq) == false);
	cw_assert(mq_put_stop(&mq));
	cw_assert(mq_put(&mq, k) == 1);

	cw_assert(mq_tryget(&mq, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_tryget(&mq, &t) == false);
	cw_assert(t == j);
	cw_assert(mq_tryget(&mq, &t));

	mq_delete(&mq);
    }
    {
	cw_mq_t mq;
	uint32_t i = 1, j = 2, k = 3, t;

	mq_new(&mq, cw_g_mema, sizeof(uint32_t));

	cw_assert(mq_tryget(&mq, &t));
	cw_assert(mq_put_stop(&mq) == false);
	cw_assert(mq_put_stop(&mq));
	cw_assert(mq_tryget(&mq, &t));

	cw_assert(1 == mq_put(&mq, i));

	cw_assert(mq_put_start(&mq) == false);
	cw_assert(mq_put_start(&mq));
	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_put(&mq, j) == 0);

	cw_assert(mq_put_stop(&mq) == false);
	cw_assert(mq_put_stop(&mq));
	cw_assert(mq_put(&mq, k) == 1);

	cw_assert(mq_tryget(&mq, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_tryget(&mq, &t) == false);
	cw_assert(t == j);
	cw_assert(mq_tryget(&mq, &t));

	mq_delete(&mq);
    }
    {
	cw_mq_t mq;
	uint64_t i = 1, j = 2, k = 3, t;

	mq_new(&mq, cw_g_mema, sizeof(uint64_t));

	cw_assert(mq_tryget(&mq, &t));
	cw_assert(mq_put_stop(&mq) == false);
	cw_assert(mq_put_stop(&mq));
	cw_assert(mq_tryget(&mq, &t));

	cw_assert(1 == mq_put(&mq, i));

	cw_assert(mq_put_start(&mq) == false);
	cw_assert(mq_put_start(&mq));
	cw_assert(mq_put(&mq, i) == 0);
	cw_assert(mq_put(&mq, j) == 0);

	cw_assert(mq_put_stop(&mq) == false);
	cw_assert(mq_put_stop(&mq));
	cw_assert(mq_put(&mq, k) == 1);

	cw_assert(mq_tryget(&mq, &t) == false);
	cw_assert(t == i);
	cw_assert(mq_tryget(&mq, &t) == false);
	cw_assert(t == j);
	cw_assert(mq_tryget(&mq, &t));

	mq_delete(&mq);
    }

    fprintf(stderr, "Test end\n");
    libonyx_shutdown();
    return 0;
}
