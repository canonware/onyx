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
 * Test the list class.
 *
 ****************************************************************************/

#define _LIBSTASH_USE_LIST
#include <libstash/libstash_r.h>

int
main()
{
  libstash_init();
  out_put(cw_g_out, "Test begin\n");

  /* list_item_new(), list_item_delete(). */
  {
    cw_list_item_t * list_item;

    list_item = list_item_new();
    _cw_check_ptr(list_item);
    list_item_delete(list_item);
  }

  /* list_item_get(), list_item_set(). */
  {
    cw_list_item_t * list_item;

    list_item = list_item_new();

    _cw_assert(NULL == list_item_get(list_item));
    list_item_set(list_item, (void *) list_item);
    _cw_assert((void *) list_item == list_item_get(list_item));
    list_item_set(list_item, NULL);
    _cw_assert(NULL == list_item_get(list_item));

    list_item_delete(list_item);
  }

  /* list_new(), list_delete(). */
  {
    cw_list_t list_a, * list_b;

    _cw_assert(&list_a == list_new(&list_a, TRUE));
    list_delete(&list_a);

    _cw_assert(&list_a == list_new(&list_a, FALSE));
    list_delete(&list_a);

    list_b = list_new(NULL, TRUE);
    _cw_check_ptr(list_b);
    list_delete(list_b);
    
    list_b = list_new(NULL, FALSE);
    _cw_check_ptr(list_b);
    list_delete(list_b);
  }

  /* list_catenate_list(),
   * list_count(). */
  {
    cw_list_t * list_a, * list_b;
    cw_sint32_t ints[13], i;

    list_a = list_new(NULL, TRUE);
    list_b = list_new(NULL, TRUE);

    _cw_assert(0 == list_count(list_a));
    _cw_assert(0 == list_count(list_b));

    /* Push all the ints onto the lists. */
    for (i = 0; i < 6; i++)
    {
      ints[i] = i;
      _cw_assert(i == list_count(list_a));
      _cw_assert(NULL != list_hpush(list_a, &ints[i]));
      _cw_assert((i + 1) == list_count(list_a));
      _cw_assert(&ints[i] == list_hpeek(list_a));
      _cw_assert((i + 1) == list_count(list_a));
      _cw_assert(&ints[i] == list_hpop(list_a));
      _cw_assert(i == list_count(list_a));
      _cw_assert(NULL != list_hpush(list_a, &ints[i]));
      _cw_assert((i + 1) == list_count(list_a));
    }
    for (;i < 13; i++)
    {
      ints[i] = i;
      _cw_assert((i - 6) == list_count(list_b));
      _cw_assert(NULL != list_hpush(list_b, &ints[i]));
      _cw_assert(((i - 6) + 1) == list_count(list_b));
      _cw_assert(&ints[i] == list_hpeek(list_b));
      _cw_assert(((i - 6) + 1) == list_count(list_b));
      _cw_assert(&ints[i] == list_hpop(list_b));
      _cw_assert((i - 6) == list_count(list_b));
      _cw_assert(NULL != list_hpush(list_b, &ints[i]));
      _cw_assert(((i - 6) + 1) == list_count(list_b));
    }

    _cw_assert(6 == list_count(list_a));
    _cw_assert(7 == list_count(list_b));

    list_catenate_list(list_a, list_b);
    _cw_assert(13 == list_count(list_a));
    _cw_assert(0 == list_count(list_b));
    
    list_catenate_list(list_a, list_b);
    _cw_assert(13 == list_count(list_a));
    _cw_assert(0 == list_count(list_b));
    
    list_catenate_list(list_b, list_a);
    _cw_assert(0 == list_count(list_a));
    _cw_assert(13 == list_count(list_b));
    
    list_delete(list_a);
    list_delete(list_b);
  }

  /* list_hpush(), list_hpop, list_hpeek(),
   * list_count(). */
  {
    cw_list_t * list;
    cw_sint32_t ints[13], i;

    list = list_new(NULL, TRUE);

    _cw_assert(0 == list_count(list));

    /* Push all the ints onto the list. */
    for (i = 0; i < 13; i++)
    {
      ints[i] = i;
      _cw_assert(i == list_count(list));
      _cw_assert(NULL != list_hpush(list, &ints[i]));
      _cw_assert((i + 1) == list_count(list));
      _cw_assert(&ints[i] == list_hpeek(list));
      _cw_assert((i + 1) == list_count(list));
      _cw_assert(&ints[i] == list_hpop(list));
      _cw_assert(i == list_count(list));
      _cw_assert(NULL != list_hpush(list, &ints[i]));
      _cw_assert((i + 1) == list_count(list));
    }

    /* Pop all the ints off the list. */
    for (i = 12; i >= 0; i--)
    {
      _cw_assert((i + 1) == list_count(list));
      _cw_assert(&ints[i] == list_hpeek(list));
      _cw_assert((i + 1) == list_count(list));
      _cw_assert(&ints[i] == list_hpop(list));
      _cw_assert(i == list_count(list));
      _cw_assert(NULL != list_hpush(list, &ints[i]));
      _cw_assert((i + 1) == list_count(list));
      _cw_assert(&ints[i] == list_hpop(list));
      _cw_assert(i == list_count(list));
    }

    /* Leave something in the list. */
    list_hpush(list, &ints[0]);
    
    list_delete(list);
  }

  /* list_tpush(), list_tpop(), list_tpeek(),
   * list_purge_spares(),
   * list_count(). */
  {
    cw_list_t * list;
    cw_sint32_t ints[13], i;

    list = list_new(NULL, TRUE);

    _cw_assert(0 == list_count(list));

    /* Push all the ints onto the list. */
    for (i = 0; i < 13; i++)
    {
      ints[i] = i;
      _cw_assert(i == list_count(list));
      _cw_assert(NULL != list_tpush(list, &ints[i]));
      _cw_assert((i + 1) == list_count(list));
      _cw_assert(&ints[i] == list_tpeek(list));
      _cw_assert((i + 1) == list_count(list));
      _cw_assert(&ints[i] == list_tpop(list));
      _cw_assert(i == list_count(list));
      _cw_assert(NULL != list_tpush(list, &ints[i]));
      _cw_assert((i + 1) == list_count(list));
    }

    /* Pop all the ints off the list. */
    for (i = 12; i >= 0; i--)
    {
      _cw_assert((i + 1) == list_count(list));
      _cw_assert(&ints[i] == list_tpeek(list));
      _cw_assert((i + 1) == list_count(list));
      _cw_assert(&ints[i] == list_tpop(list));
      _cw_assert(i == list_count(list));
      _cw_assert(NULL != list_tpush(list, &ints[i]));
      _cw_assert((i + 1) == list_count(list));
      _cw_assert(&ints[i] == list_tpop(list));
      _cw_assert(i == list_count(list));
    }

    list_purge_spares(list);
      
    /* Leave something in the list. */
    list_tpush(list, &ints[0]);
    list_tpush(list, &ints[1]);

    list_delete(list);
  }

  /* list_get_next(), list_get_prev(). */
  {
    cw_list_t * list;
    cw_list_item_t * items[3], * item;

    list = list_new(NULL, TRUE);

    items[0] = list_tpush(list, NULL);
    items[1] = list_tpush(list, NULL);
    items[2] = list_tpush(list, NULL);

    item = list_get_next(list, NULL);
    _cw_assert(item == items[0]);
    item = list_get_next(list, item);
    _cw_assert(item == items[1]);
    item = list_get_next(list, item);
    _cw_assert(item == items[2]);
    item = list_get_next(list, item);
    _cw_assert(item == NULL);
    
    item = list_get_prev(list, NULL);
    _cw_assert(item == items[2]);
    item = list_get_prev(list, item);
    _cw_assert(item == items[1]);
    item = list_get_prev(list, item);
    _cw_assert(item == items[0]);
    item = list_get_prev(list, item);
    _cw_assert(item == NULL);
    
    list_delete(list);
  }

  /* list_insert_before(),
   * list_count(). */
  {
    cw_list_t * list;
    cw_list_item_t * items[11];
    cw_uint32_t ints_a[11], ints_b[11], i;

    list = list_new(NULL, TRUE);

    /* Push the ints onto the list. */
    for (i = 0; i < 11; i++)
    {
      ints_a[i] = i;
      _cw_assert(i == list_count(list));
      items[i] = list_hpush(list, &ints_a[i]);
      _cw_assert((i + 1) == list_count(list));
    }

    /* Insert before each int in the list. */
    for (i = 0; i < 11; i++)
    {
      ints_b[i] = i + 11;
      _cw_assert((i + 11) == list_count(list));
      _cw_assert(NULL != list_insert_before(list, items[i], &ints_b[i]));
      _cw_assert((i + 12) == list_count(list));
    }

    /* Push and pop from each end of the list to make sure the head and tail are
     * still okay.  If they aren't, the problem will show up later. */
    list_hpush(list, list_hpop(list));
    list_tpush(list, list_tpop(list));

    /* Make sure that things are in the proper order in the list. */
    for (i = 0; i < 11; i++)
    {
      _cw_assert(i == *(cw_uint32_t *) list_tpop(list));
      _cw_assert((i + 11) == *(cw_uint32_t *) list_tpop(list));
    }

    _cw_assert(0 == list_count(list));

    list_delete(list);
  }

  /* list_insert_after(),
   * list_purge_spares(),
   * list_count(). */
  {
    cw_list_t * list;
    cw_list_item_t * items[11];
    cw_uint32_t ints_a[11], ints_b[11], i;

    list = list_new(NULL, TRUE);

    /* Push the ints onto the list. */
    for (i = 0; i < 11; i++)
    {
      ints_a[i] = i;
      _cw_assert(i == list_count(list));
      items[i] = list_tpush(list, &ints_a[i]);
      _cw_assert((i + 1) == list_count(list));
    }

    /* Insert before each int in the list. */
    for (i = 0; i < 11; i++)
    {
      ints_b[i] = i + 11;
      _cw_assert((i + 11) == list_count(list));
      _cw_assert(NULL != list_insert_after(list, items[i], &ints_b[i]));
      _cw_assert((i + 12) == list_count(list));
    }

    /* Push and pop from each end of the list to make sure the head and tail are
     * still okay.  If they aren't, the problem will show up later. */
    list_hpush(list, list_hpop(list));
    list_tpush(list, list_tpop(list));

    /* Make sure that things are in the proper order in the list. */
    for (i = 0; i < 11; i++)
    {
      _cw_assert(i == *(cw_uint32_t *) list_hpop(list));
      _cw_assert((i + 11) == *(cw_uint32_t *) list_hpop(list));
    }

    list_purge_spares(list);
    
    _cw_assert(0 == list_count(list));

    list_delete(list);
  }

  /* list_remove_item(),
   * list_count(). */
  {
    cw_list_t * list;
    cw_list_item_t * items[11];
    cw_uint32_t ints[11], i;

    list = list_new(NULL, TRUE);

    /* Push the ints onto the list. */
    for (i = 0; i < 11; i++)
    {
      ints[i] = i;
      _cw_assert(i == list_count(list));
      items[i] = list_tpush(list, &ints[i]);
      _cw_assert((i + 1) == list_count(list));
    }

    _cw_assert(11 == list_count(list));
    _cw_assert(ints[0] == *(cw_uint32_t *) list_remove_item(list,
							    (void *) &ints[0]));
    _cw_assert(10 == list_count(list));
    _cw_assert(ints[10] == *(cw_uint32_t *) list_remove_item(list, 
							     (void *)
							     &ints[10]));
    _cw_assert(9 == list_count(list));
    _cw_assert(ints[5] == *(cw_uint32_t *) list_remove_item(list, 
							    (void *) &ints[5]));
    _cw_assert(8 == list_count(list));

    _cw_assert(ints[1] == *(cw_uint32_t *) list_remove_item(list, 
							    (void *) &ints[1]));
    _cw_assert(7 == list_count(list));
    _cw_assert(ints[9] == *(cw_uint32_t *) list_remove_item(list, 
							    (void *) &ints[9]));
    _cw_assert(6 == list_count(list));
    _cw_assert(ints[6] == *(cw_uint32_t *) list_remove_item(list, 
							    (void *) &ints[6]));
    _cw_assert(5 == list_count(list));
    
    _cw_assert(ints[2] == *(cw_uint32_t *) list_remove_item(list, 
							    (void *) &ints[2]));
    _cw_assert(4 == list_count(list));
    _cw_assert(ints[8] == *(cw_uint32_t *) list_remove_item(list, 
							    (void *) &ints[8]));
    _cw_assert(3 == list_count(list));
    _cw_assert(ints[7] == *(cw_uint32_t *) list_remove_item(list, 
							    (void *) &ints[7]));
    _cw_assert(2 == list_count(list));
    
    _cw_assert(ints[3] == *(cw_uint32_t *) list_remove_item(list, 
							    (void *) &ints[3]));
    _cw_assert(1 == list_count(list));
    _cw_assert(ints[4] == *(cw_uint32_t *) list_remove_item(list, 
							    (void *) &ints[4]));
    _cw_assert(0 == list_count(list));
    
    list_delete(list);
  }
  
  /* list_remove_container(),
   * list_count(). */
  {
    cw_list_t * list;
    cw_list_item_t * items[11];
    cw_uint32_t ints[11], i;

    list = list_new(NULL, TRUE);

    /* Push the ints onto the list. */
    for (i = 0; i < 11; i++)
    {
      ints[i] = i;
      _cw_assert(i == list_count(list));
      items[i] = list_tpush(list, &ints[i]);
      _cw_assert((i + 1) == list_count(list));
    }

    /* Remove all the items in the list. */
    for (i = 0; i < 11; i++)
    {
      _cw_assert((11 - i) == list_count(list));
      _cw_assert(ints[i] ==
		 *(cw_uint32_t *) list_remove_container(list, items[i]));
    }

    _cw_assert(0 == list_count(list));

    list_delete(list);
  }
  
#define NUM_ITEMS 50
  {
    char * strings[NUM_ITEMS], * str_ptr;
    cw_list_t * list1, list2;
    int i;
  

    for (i = 0; i < NUM_ITEMS; i++)
    {
      strings[i] = (char *) _cw_malloc(NUM_ITEMS);
      out_put_s(cw_g_out, strings[i], "This is string [i32]", i);
    }

    list1 = list_new(NULL, FALSE);
    list_new(&list2, FALSE);
    _cw_assert(list_count(list1) == 0);
    _cw_assert(list_count(&list2) == 0);
	     
    for (i = 0; i < NUM_ITEMS; i++)
    {
      _cw_assert(list_count(list1) == i);

      list_tpush(list1, (void *) strings[i]);
    }
    _cw_assert(list_count(list1) == NUM_ITEMS);
  
    out_put(cw_g_out, "hpop()ping from list1 and hpush()ing to list2\n");
  
    for (i = 0; i < NUM_ITEMS; i++)
    {
      str_ptr = (char *) list_hpop(list1);
      _cw_check_ptr(str_ptr);
      out_put(cw_g_out, "[s]\n", str_ptr);
      list_hpush(&list2, (void *) str_ptr);
    }

    _cw_assert(list_count(list1) == 0);
    _cw_assert(list_count(&list2) == NUM_ITEMS);

    out_put(cw_g_out, "tpop()ping from list2 and hpush()ing to list2\n");
  
    for (i = 0; i < NUM_ITEMS; i++)
    {
      str_ptr = (char *) list_tpop(&list2);
      _cw_check_ptr(str_ptr);
      out_put(cw_g_out, "[s]\n", str_ptr);
      list_hpush(&list2, (void *) str_ptr);
    }

    _cw_assert(list_count(list1) == 0);
    out_put(cw_g_out, "list2->count == [i64]\n", list_count(&list2));
    
    _cw_assert(list_count(&list2) == NUM_ITEMS);

    out_put(cw_g_out, "tpop()ping from list2 and tpush()ing to list1\n");
  
    for (i = 0; i < NUM_ITEMS; i++)
    {
      str_ptr = (char *) list_tpop(&list2);
      _cw_check_ptr(str_ptr);
      out_put(cw_g_out, "[s]\n", str_ptr);
      list_tpush(list1, (void *) str_ptr);
    }

    _cw_assert(list_count(list1) == NUM_ITEMS);
    _cw_assert(list_count(&list2) == 0);
  
    list_delete(list1);
    list_delete(&list2);

    for (i = 0; i < NUM_ITEMS; i++)
    {
      _cw_free(strings[i]);
    }
  }
  
  out_put(cw_g_out, "Test end\n");
  libstash_shutdown();
  
  return 0;
}
