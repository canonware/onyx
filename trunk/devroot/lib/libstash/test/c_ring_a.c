/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
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

#define _LIBSTASH_USE_RING
#include <libstash/libstash_r.h>

int
main()
{
  
  libstash_init();
  log_printf(cw_g_log, "Test begin\n");

  /* ring_new(), ring_delete(). */
  {
    cw_ring_t * ring_a, ring_b;

    ring_a = ring_new(NULL, NULL, NULL);
    _cw_check_ptr(ring_a);
    ring_delete(ring_a);

    ring_a = ring_new((cw_ring_t *) _cw_malloc(sizeof(cw_ring_t)),
		      mem_dealloc,
		      cw_g_mem);
    _cw_check_ptr(ring_a);
    ring_delete(ring_a);

    _cw_assert(&ring_b == ring_new(&ring_b, NULL, NULL));
    ring_delete(&ring_b);
  }

  /* ring_get_data(), ring_set_data(). */
  {
    cw_ring_t * ring;
    char * str = "Hi";

    ring = ring_new(NULL, NULL, NULL);

    ring_set_data(ring, str);
    _cw_assert(str == ring_get_data(ring));

    ring_delete(ring);
  }

  /* ring_meld(), ring_cut(), ring_split(). */
  {
    cw_ring_t * ring_a, * ring_b, * t_ring;
    char str[11] = "abcdefghij", c;
    cw_uint32_t i;

    ring_a = ring_new(NULL, NULL, NULL);

    for (i = 0; i < 10; i++)
    {
      ring_b = ring_new(NULL, NULL, NULL);
      ring_set_data(ring_b, (void *) &str[i]);
      ring_meld(ring_a, ring_b);
    }
    ring_b = ring_a;
    ring_a = ring_cut(ring_b);
    ring_delete(ring_b);

    log_printf(cw_g_log, "ring_a contents (1): ");
    t_ring = ring_a;
    do 
    {
      c = *(char *) ring_get_data(t_ring);
      log_printf(cw_g_log, "%c ", c);
      t_ring = ring_next(t_ring);
    } while (t_ring != ring_a);
    log_printf(cw_g_log, "\n");

    for (i = 0, ring_b = ring_a; i < 2; i++)
    {
      ring_b = ring_next(ring_b);
    }

    ring_split(ring_a, ring_b);
    
    log_printf(cw_g_log, "ring_a contents (2): ");
    t_ring = ring_a;
    do 
    {
      c = *(char *) ring_get_data(t_ring);
      log_printf(cw_g_log, "%c ", c);
      t_ring = ring_next(t_ring);
    } while (t_ring != ring_a);
    log_printf(cw_g_log, "\n");

    log_printf(cw_g_log, "ring_b contents (3): ");
    t_ring = ring_b;
    do 
    {
      c = *(char *) ring_get_data(t_ring);
      log_printf(cw_g_log, "%c ", c);
      t_ring = ring_next(t_ring);
    } while (t_ring != ring_b);
    log_printf(cw_g_log, "\n");

    t_ring = ring_next(ring_a);
    ring_split(ring_a, t_ring);
    ring_delete(t_ring);
    log_printf(cw_g_log, "ring_a contents (4): ");
    t_ring = ring_a;
    do 
    {
      c = *(char *) ring_get_data(t_ring);
      log_printf(cw_g_log, "%c ", c);
      t_ring = ring_next(t_ring);
    } while (t_ring != ring_a);
    log_printf(cw_g_log, "\n");

    t_ring = ring_next(ring_a);
    ring_split(ring_a, t_ring);
    log_printf(cw_g_log, "ring_a contents (5): ");
    t_ring = ring_a;
    do 
    {
      c = *(char *) ring_get_data(t_ring);
      log_printf(cw_g_log, "%c ", c);
      t_ring = ring_next(t_ring);
    } while (t_ring != ring_a);
    log_printf(cw_g_log, "\n");
    
    ring_delete(ring_a);
    ring_delete(ring_b);
  }

  /* ring_next(), ring_prev(). */
  {
    cw_ring_t * ring_a, * ring_b;

    ring_a = ring_new(NULL, NULL, NULL);
    ring_b = ring_new(NULL, NULL, NULL);

    _cw_assert(ring_a == ring_next(ring_a));
    _cw_assert(ring_a == ring_prev(ring_a));

    ring_meld(ring_a, ring_b);
    _cw_assert(ring_b == ring_next(ring_a));
    _cw_assert(ring_b == ring_prev(ring_a));
    _cw_assert(ring_a == ring_next(ring_b));
    _cw_assert(ring_a == ring_prev(ring_b));
    
    ring_delete(ring_a);
  }

  log_printf(cw_g_log, "Test end\n");
  libstash_shutdown();
  return 0;
}
