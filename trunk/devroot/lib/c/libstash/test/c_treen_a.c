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
 * Test of the tree and treen classes.
 *
 ****************************************************************************/

#include <libstash/libstash_r.h>

int
main()
{
  cw_treen_t * treen_a, * treen_b, * treens[7];
  cw_uint32_t i;
  
  libstash_init();
  out_put(cw_g_out, "Test begin\n");

  treen_a = treen_new();
  _cw_check_ptr(treen_a);
  
  treen_b = treen_new_r();
  _cw_check_ptr(treen_b);

  _cw_assert(FALSE == treen_link_child(treen_a, treen_b, 0));

  treen_delete(treen_a);

  for (i = 0; i < 7; i++)
  {
    treens[i] = treen_new_r();
    _cw_check_ptr(treens);
  }
  
  /* Link the treens as such:
   *             0
   *            / \
   *           /   \
   *          /     \
   *         /       \
   *        /         \
   *       1           2
   *      / \         / \
   *     /   \       /   \
   *    3     4     5     6
   */
  _cw_assert(FALSE == treen_link_child(treens[0], treens[1], 0));
  _cw_assert(FALSE == treen_link_child(treens[0], treens[2], 1));
  _cw_assert(FALSE == treen_link_child(treens[1], treens[3], 0));
  _cw_assert(FALSE == treen_link_child(treens[1], treens[4], 1));
  _cw_assert(FALSE == treen_link_child(treens[2], treens[5], 0));
  _cw_assert(FALSE == treen_link_child(treens[2], treens[6], 1));

  /* Move part of the tree. */
  _cw_assert(FALSE == treen_unlink_child(treens[0], 1, &treen_a));
  _cw_assert(treen_a == treens[2]);
  _cw_assert(FALSE == treen_link_child(treens[1], treens[2], 0));

  treen_delete(treens[0]);

  out_put(cw_g_out, "Test end\n");
  libstash_shutdown();
  return 0;
}
