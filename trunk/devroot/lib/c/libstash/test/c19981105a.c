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
 * Test of the tree and treen classes.
 *
 ****************************************************************************/

#define _INC_GLOB_H_
#define _INC_TREE_H_
#include <libstash_r.h>

int
main()
{
  cw_tree_t tree_a, * tree_b;
  cw_treen_t * treen_a, * treen_b;
  
  glob_new();

  _cw_assert(&tree_a == tree_new(&tree_a, FALSE));
  _cw_assert(NULL == tree_get_root_ptr(&tree_a));
  tree_delete(&tree_a);

  glob_delete();
  return 0;
}
