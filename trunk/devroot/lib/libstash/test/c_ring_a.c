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
 * Test the ring class.
 *
 ****************************************************************************/

#include "../include/libstash/libstash.h"

int
main()
{
	libstash_init();
	_cw_out_put("Test begin\n");

	/* ring_new(), ring_delete(). */
	{
		cw_ring_t ring_a;

		ring_new(&ring_a);
		ring_delete(&ring_a);
	}

	/* ring_get_data(), ring_set_data(). */
	{
		cw_ring_t ring;
		char   *str = "Hi";

		ring_new(&ring);

		ring_set_data(&ring, str);
		_cw_assert(str == ring_get_data(&ring));

		ring_delete(&ring);
	}

	/* ring_meld(), ring_cut(), ring_split(). */
	{
		cw_ring_t *ring_a, *ring_b, *t_ring;
		char    str[11] = "abcdefghij", c;
		cw_uint32_t i;

		ring_a = (cw_ring_t *)_cw_malloc(sizeof(cw_ring_t));
		ring_new(ring_a);
		_cw_assert(ring_a == ring_cut(ring_a));

		for (i = 0; i < 10; i++) {
			t_ring = (cw_ring_t *)_cw_malloc(sizeof(cw_ring_t));
			ring_new(t_ring);
			ring_set_data(t_ring, (void *)&str[i]);
			ring_meld(ring_a, t_ring);
		}
		ring_b = ring_a;
		ring_a = ring_cut(ring_b);
		ring_delete(ring_b);
		_cw_free(ring_b);

		_cw_out_put("ring_a contents (1): ");
		t_ring = ring_a;
		do {
			c = *(char *)ring_get_data(t_ring);
			_cw_out_put("[c] ", c);
			t_ring = ring_next(t_ring);
		} while (t_ring != ring_a);
		_cw_out_put("\n");

		for (i = 0, ring_b = ring_a; i < 2; i++) {
			ring_b = ring_next(ring_b);
		}

		ring_split(ring_a, ring_b);

		_cw_out_put("ring_a contents (2): ");
		t_ring = ring_a;
		do {
			c = *(char *)ring_get_data(t_ring);
			_cw_out_put("[c] ", c);
			t_ring = ring_next(t_ring);
		} while (t_ring != ring_a);
		_cw_out_put("\n");

		ring_split(ring_a, ring_a);

		_cw_out_put("ring_a contents (3): ");
		t_ring = ring_a;
		do {
			c = *(char *)ring_get_data(t_ring);
			_cw_out_put("[c] ", c);
			t_ring = ring_next(t_ring);
		} while (t_ring != ring_a);
		_cw_out_put("\n");

		_cw_out_put("ring_b contents (4): ");
		t_ring = ring_b;
		do {
			c = *(char *)ring_get_data(t_ring);
			_cw_out_put("[c] ", c);
			t_ring = ring_next(t_ring);
		} while (t_ring != ring_b);
		_cw_out_put("\n");

		t_ring = ring_next(ring_a);
		ring_split(ring_a, t_ring);
		ring_delete(t_ring);
		_cw_free(t_ring);
		_cw_out_put("ring_a contents (5): ");
		t_ring = ring_a;
		do {
			c = *(char *)ring_get_data(t_ring);
			_cw_out_put("[c] ", c);
			t_ring = ring_next(t_ring);
		} while (t_ring != ring_a);
		_cw_out_put("\n");

		t_ring = ring_next(ring_a);
		ring_split(ring_a, t_ring);
		_cw_out_put("ring_a contents (6): ");
		t_ring = ring_a;
		do {
			c = *(char *)ring_get_data(t_ring);
			_cw_out_put("[c] ", c);
			t_ring = ring_next(t_ring);
		} while (t_ring != ring_a);
		_cw_out_put("\n");

		do {
			t_ring = ring_a;
			ring_a = ring_cut(t_ring);
			ring_delete(t_ring);
			_cw_free(t_ring);
		} while (t_ring != ring_a);

		do {
			t_ring = ring_b;
			ring_b = ring_cut(t_ring);
			ring_delete(t_ring);
			_cw_free(t_ring);
		} while (t_ring != ring_b);
	}

	/* ring_next(), ring_prev(). */
	{
		cw_ring_t ring_a, ring_b;

		ring_new(&ring_a);
		ring_new(&ring_b);

		_cw_assert(&ring_a == ring_next(&ring_a));
		_cw_assert(&ring_a == ring_prev(&ring_a));

		ring_meld(&ring_a, &ring_b);
		_cw_assert(&ring_b == ring_next(&ring_a));
		_cw_assert(&ring_b == ring_prev(&ring_a));
		_cw_assert(&ring_a == ring_next(&ring_b));
		_cw_assert(&ring_a == ring_prev(&ring_b));

		ring_delete(&ring_a);
		ring_delete(&ring_b);
	}

	_cw_out_put("Test end\n");
	libstash_shutdown();
	return 0;
}
