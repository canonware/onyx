/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
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
 * jtl test.
 *
 ****************************************************************************/

#include <libstash/libstash_r.h>

#define _STASH_TEST_NUM_THREADS 100

struct foo_var_s
{
  cw_bool_t is_tlock;
  cw_uint32_t thd_num;
  cw_jtl_t * lock;
  cw_jtl_tq_el_t * tq_el;
};

void * 
thread_entry_func(void * a_arg)
{
  struct foo_var_s * foo_var = (struct foo_var_s *) a_arg;

  if (foo_var->is_tlock == TRUE)
  {
    /* Grab tlock. */
    jtl_tlock(foo_var->lock, foo_var->tq_el);
    usleep(1);
    log_eprintf(g_log_o, NULL, 0, "thread_entry_func",
		"Thread %u has tlock\n", foo_var->thd_num);
    jtl_tunlock(foo_var->lock);
  }
  else
  {
    /* Grab slock. */
    jtl_slock(foo_var->lock);
/*     log_eprintf(g_log_o, NULL, 0, "thread_entry_func", */
/* 		"Thread %u has slock\n", foo_var->thd_num); */

    jtl_2qlock(foo_var->lock);
/*     log_eprintf(g_log_o, NULL, 0, "thread_entry_func", */
/* 		"Thread %u has qlock\n", foo_var->thd_num); */
    jtl_qunlock(foo_var->lock);

    jtl_2rlock(foo_var->lock);
/*     log_eprintf(g_log_o, NULL, 0, "thread_entry_func", */
/* 		"Thread %u has rlock\n", foo_var->thd_num); */
    jtl_runlock(foo_var->lock);

    jtl_2wlock(foo_var->lock);
/*     log_eprintf(g_log_o, NULL, 0, "thread_entry_func", */
/* 		"Thread %u has wlock\n", foo_var->thd_num); */
    jtl_wunlock(foo_var->lock);

    jtl_2xlock(foo_var->lock);
/*     log_eprintf(g_log_o, NULL, 0, "thread_entry_func", */
/* 		"Thread %u has xlock\n", foo_var->thd_num); */
    jtl_xunlock(foo_var->lock);
  } 
/*   log_eprintf(g_log_o, NULL, 0, "thread_entry_func", */
/* 	      "Thread %u is done\n", foo_var->thd_num); */
  _cw_free(foo_var);
    
  return NULL;
}

int
main()
{
  cw_thd_t tl_threads[_STASH_TEST_NUM_THREADS],
    sl_threads[_STASH_TEST_NUM_THREADS];
  cw_jtl_tq_el_t * tq_els[_STASH_TEST_NUM_THREADS];
  cw_jtl_t jtl_a, * jtl_b;
  cw_jtl_tq_el_t * tq_el;
  struct foo_var_s * foo_var;
  cw_uint32_t i;
  
  libstash_init();

  _cw_assert(&jtl_a == jtl_new(&jtl_a));
  jtl_slock(&jtl_a);
  jtl_2qlock(&jtl_a);
  jtl_2rlock(&jtl_a);
  jtl_2qlock(&jtl_a);
  jtl_slock(&jtl_a);
  jtl_2rlock(&jtl_a);
  jtl_qunlock(&jtl_a);
  jtl_qunlock(&jtl_a);
  jtl_2wlock(&jtl_a);
  jtl_2rlock(&jtl_a);
  jtl_wunlock(&jtl_a);
  jtl_runlock(&jtl_a);
  jtl_runlock(&jtl_a);
  jtl_runlock(&jtl_a);
  jtl_2xlock(&jtl_a);
  jtl_sunlock(&jtl_a);
  jtl_xunlock(&jtl_a);
  jtl_sunlock(&jtl_a);
  tq_el = jtl_get_tq_el(&jtl_a);
  jtl_tlock(&jtl_a, tq_el);
  jtl_tunlock(&jtl_a);

  jtl_b = jtl_new(NULL);

  for (i = 0; i < _STASH_TEST_NUM_THREADS; i++)
  {
    tq_els[i] = jtl_get_tq_el(jtl_b);
  }
  
  for (i = 0; i < _STASH_TEST_NUM_THREADS; i++)
  {
    foo_var = (struct foo_var_s *) _cw_malloc(sizeof(struct foo_var_s));
    foo_var->is_tlock = TRUE;
    foo_var->thd_num = i;
    foo_var->tq_el = tq_els[i];
    foo_var->lock = jtl_b;
    thd_new(&tl_threads[i], thread_entry_func, (void *) foo_var);
    
    foo_var = (struct foo_var_s *) _cw_malloc(sizeof(struct foo_var_s));
    foo_var->is_tlock = FALSE;
    foo_var->thd_num = i;
    foo_var->lock = jtl_b;
    thd_new(&sl_threads[i], thread_entry_func, (void *) foo_var);
  }

  for (i = 0; i < _STASH_TEST_NUM_THREADS; i++)
  {
    thd_join(&tl_threads[i]);
    thd_join(&sl_threads[i]);
  }

  jtl_delete(jtl_b);
  
  libstash_shutdown();
  return 0;
}
