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
 * Public interface for the tree and treen (tree node) classes.
 *
 ****************************************************************************/

/* Pseudo-opaque types. */
typedef struct cw_tree_s cw_tree_t;
typedef struct cw_treen_s cw_treen_t;

struct cw_tree_s
{
  cw_bool_t is_malloced;
#ifdef _CW_REENTRANT
  cw_bool_t is_thread_safe;
  cw_mtx_t lock;
#endif
  cw_treen_t * root;
};

struct cw_treen_s
{
#ifdef _CW_REENTRANT
  cw_bool_t is_thread_safe;
  cw_mtx_t lock;
#endif
  void * data;
  cw_treen_t * parent;
  cw_uint32_t num_children;
  cw_treen_t ** children;
};

#define tree_new _CW_NS_ANY(tree_new)
#define tree_delete _CW_NS_ANY(tree_delete)
#define tree_get_root_ptr _CW_NS_ANY(tree_get_root_ptr)
#define tree_set_root_ptr _CW_NS_ANY(tree_set_root_ptr)

#define treen_new _CW_NS_ANY(treen_new)
#define treen_delete _CW_NS_ANY(treen_delete)
#define treen_get_parent_ptr _CW_NS_ANY(tree_get_parent_ptr)
#define treen_set_parent_ptr _CW_NS_ANY(tree_set_parent_ptr)
#define treen_get_data_ptr _CW_NS_ANY(tree_get_data_ptr)
#define treen_set_data_ptr _CW_NS_ANY(tree_set_data_ptr)
#define treen_get_num_children _CW_NS_ANY(treen_get_num_children)
#define treen_ins_child _CW_NS_ANY(tree_ins_child)
#define treen_del_child _CW_NS_ANY(tree_del_child)
#define treen_get_child_ptr _CW_NS_ANY(treen_get_child_ptr)
#define treen_set_child_ptr _CW_NS_ANY(treen_set_child_ptr)

#ifdef _CW_REENTRANT
cw_tree_t * tree_new(cw_tree_t * a_tree_o, cw_bool_t a_is_thread_safe);
#else
cw_tree_t * tree_new(cw_tree_t * a_tree_o);
#endif
void tree_delete(cw_tree_t * a_tree_o);
cw_treen_t * tree_get_root_ptr(cw_tree_t * a_tree_o);
void tree_set_root_ptr(cw_tree_t * a_tree_o, cw_treen_t * a_treen_o);

cw_treen_t * treen_new(cw_bool_t a_is_thread_safe);
void treen_delete(cw_treen_t * a_treen_o);
cw_treen_t * treen_get_parent_ptr(cw_treen_t * a_treen_o);
void treen_set_parent_ptr(cw_treen_t * a_treen_o, cw_treen_t * a_parent);
void * treen_get_data_ptr(cw_treen_t * a_treen_o);
void treen_set_data_ptr(cw_treen_t * a_treen_o, void * a_data);
cw_uint32_t treen_get_num_children(cw_treen_t * a_treen_o);
cw_bool_t treen_ins_child(cw_treen_t * a_treen_o, cw_treen_t * a_child,
			  cw_uint32_t a_position);
cw_bool_t treen_del_child(cw_treen_t * a_treen_o, cw_uint32_t a_position,
			  cw_treen_t ** a_child);
cw_bool_t treen_get_child_ptr(cw_treen_t * a_treen_o, cw_uint32_t a_position,
			      cw_treen_t ** a_child);
cw_bool_t treen_set_child_ptr(cw_treen_t * a_treen_o, cw_uint32_t a_position,
			      cw_treen_t * a_child);
