/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 177 $
 * $Date: 1998-08-29 20:58:01 -0700 (Sat, 29 Aug 1998) $
 *
 * <<< Description >>>
 *
 * Test the list class.
 *
 ****************************************************************************/

#define _INC_GLOB_H_
#define _INC_LIST_H_
#include <libstash.h>

#define NUM_ITEMS 50

int
main()
{
  char * strings[NUM_ITEMS], * str_ptr;
  cw_list_t * list1, list2;
  int i;
  
  glob_new();

  for (i = 0; i < NUM_ITEMS; i++)
  {
    strings[i] = (char *) _cw_malloc(NUM_ITEMS);
    sprintf(strings[i], "This is string %d", i);
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
  
  log_printf(g_log_o, "hpop()ping from list1 and hpush()ing to list2\n");
  
  for (i = 0; i < NUM_ITEMS; i++)
  {
/*     log_printf(g_log_o, "i == %d\n", i); */
/*     list_dump(list1); */
/*     list_dump(&list2); */
    str_ptr = (char *) list_hpop(list1);
    _cw_check_ptr(str_ptr);
    log_printf(g_log_o, "%s\n", str_ptr);
    list_hpush(&list2, (void *) str_ptr);
  }

  _cw_assert(list_count(list1) == 0);
  _cw_assert(list_count(&list2) == NUM_ITEMS);

  log_printf(g_log_o, "tpop()ping from list2 and hpush()ing to list2\n");
  
  for (i = 0; i < NUM_ITEMS; i++)
  {
    str_ptr = (char *) list_tpop(&list2);
    _cw_check_ptr(str_ptr);
    log_printf(g_log_o, "%s\n", str_ptr);
    list_hpush(&list2, (void *) str_ptr);
  }

  _cw_assert(list_count(list1) == 0);
  log_printf(g_log_o, "list2->count == %d\n", list_count(&list2));
  _cw_assert(list_count(&list2) == NUM_ITEMS);

  log_printf(g_log_o, "tpop()ping from list2 and tpush()ing to list1\n");
  
  for (i = 0; i < NUM_ITEMS; i++)
  {
/*     log_printf(g_log_o, "i == %d\n", i); */
/*     list_dump(list1); */
/*     list_dump(&list2); */
    str_ptr = (char *) list_tpop(&list2);
    _cw_check_ptr(str_ptr);
    log_printf(g_log_o, "%s\n", str_ptr);
    list_tpush(list1, (void *) str_ptr);
  }

  _cw_assert(list_count(list1) == NUM_ITEMS);
  _cw_assert(list_count(&list2) == 0);
  
  list_delete(list1);
  list_delete(&list2);
    
  glob_delete();
  
  return 0;
}
