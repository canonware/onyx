/* -*-mode:c-*-
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
 * Test the matrix class.
 *
 ****************************************************************************/

#define _INC_GLOB_H_
#define _INC_MATRIX_H_
#include <libstash_r.h>

#include <limits.h>

typedef struct
{
  cw_uint32_t x_pos;
  cw_uint32_t y_pos;
} map_el_t;

cw_bool_t is_min_span_tree(cw_matrix_t *, cw_uint32_t);
cw_uint32_t reduce(cw_matrix_t *);
void remove_essential(cw_matrix_t *, cw_uint32_t);
cw_uint32_t recurse(cw_matrix_t *, cw_uint32_t, cw_uint32_t);

cw_uint64_t g_tries = 0;

int
main(int argc, char * argv[])
{
  cw_matrix_t graph, cover, * graphs[1296]; /* Hard-coded for (n <= 6). */
  cw_uint32_t num_nodes, num_graphs, num_edges, num_min_graphs;
  map_el_t * map;

  glob_new();

#if (0)
  if (argc != 2)
  {
    log_printf(g_log_o, "Usage: %s <num_nodes>\n", argv[0]);
    exit(1);
  }
  num_nodes = strtol(argv[1], (char **) NULL, 10);
#endif

  num_nodes = 5;
  
  log_printf(g_log_o, "num_nodes == %lu\n", num_nodes);
  if (num_nodes > 6)
  {
    log_printf(g_log_o,
	       "Due to variable limits, we can't do (num_nodes > 6)\n");
    exit(1);
  }
  num_edges = num_nodes * ((num_nodes - 1) / 2)
    + (((num_nodes + 1) / 2) * ((num_nodes + 1) % 2));
  log_printf(g_log_o, "num_edges == %lu\n", num_edges);
  num_graphs = 1 << (num_nodes * ((num_nodes - 1) / 2)
		     + (((num_nodes + 1) / 2) * ((num_nodes + 1) % 2)));
  log_printf(g_log_o, "num_graphs == %lu\n", num_graphs);
  
  /* Create adjacency matrix, given number of nodes. */
  matrix_new(&graph);
  matrix_init(&graph, num_nodes, num_nodes, TRUE);

  /* We're mapping edges as such:
   * {0, 1, ..., n} --> {(0, 1), (0, 2), ... , (0, n - 1),
   *                     (1, 2), (1, 3), ... , (1, n - 1),
   *                     ................................,
   *                     (n - 4, n - 3), ... , (n - 2, n - 3),
   *                     (n - 3, n - 2), (n - 2, n - 2),
   *                     (n - 2, n - 1)}
   *
   * Of course, since this is an undirected graph, we need to turn on the
   * other direction for each edge as well. */

  map = (map_el_t *) _cw_malloc(sizeof(map_el_t) * num_edges);

  {
    cw_uint32_t x, y, curr_map_el = 0;
    
    for (y = 0; y < num_nodes; y++)
    {
      for (x = 0; x < num_nodes; x++)
      {
	if (x > y)
	{
	  map[curr_map_el].x_pos = x;
	  map[curr_map_el].y_pos = y;

	  curr_map_el++;
	  if (curr_map_el > num_edges)
	  {
	    break;
	  }
	}
      }
    }
  }

  /* Now, cycle through all the possible graphs, to find out how many
   * minimal spanning trees there are. */
  {
    cw_uint32_t i;
    
    num_min_graphs = 0;
    for (i = 0; i < num_graphs; i++)
    {
      cw_uint32_t curr_bit;
    
      matrix_init(&graph, num_nodes, num_nodes, TRUE);
    
      for (curr_bit = 0; curr_bit < 28; curr_bit++)
      {
	if (i & (1 << curr_bit))
	{
	  matrix_set_element(&graph, map[curr_bit].x_pos,
			     map[curr_bit].y_pos, TRUE);
	  matrix_set_element(&graph, map[curr_bit].y_pos,
			     map[curr_bit].x_pos, TRUE);
	}
      }
      if (is_min_span_tree(&graph, num_nodes))
      {
	/* 	log_printf(g_log_o, "Ding.\n"); */
	graphs[num_min_graphs] = matrix_copy(&graph);
	num_min_graphs++;
      }
    }
  }
  
  log_printf(g_log_o, "Matrix size == %lu x %lu\n", num_graphs,
	     num_min_graphs);

  /* Create the big bad matrix. */
  matrix_new(&cover);
  matrix_init(&cover, num_graphs, num_min_graphs, TRUE);

  /* Now fill in the matrix.  The minimum spanning trees are in graphs[].*/
  {
    cw_uint32_t y, i, j, k, curr_blanks, super_graph, sub_graph,
      digits[15], blanks[10]; /* Hard-coded for (n <= 6). */

    /* Do this for every mimimum spanning tree. */
    for (y = 0; y < num_min_graphs; y++)
    {
      /* Copy graph into digits. */
      for (j = 0, curr_blanks = 0; j < num_edges; j++)
      {
	digits[j] = matrix_get_element(graphs[y], map[j].x_pos,
				       map[j].y_pos);
	/* Record which digits are blank. */
	if (digits[j] == 0)
	{
	  blanks[curr_blanks] = j;
	  curr_blanks++;
	}
      }

      /* Iterate through all of the possible subset/superset graphs. */
      for (j = 0; j < (1 << curr_blanks); j++)
      {
	/* Initialize sub_graph and super_graph, so that all we have left
	 * to do is | in the additional counter bits. */
	sub_graph = 0;
	super_graph = 0;
	for (k = 0; k < num_edges; k++)
	{
	  super_graph |= (digits[k] << k);
	}
	
	/* j contains our bit counter.  Extract the bits from j and
	 * interleave them into super_graph and sub_graph. */
	for (i = 0; i < curr_blanks; i++)
	{
	  if (j & (1 << i))
	  {
	    super_graph |= 1 << blanks[i];
	    sub_graph |= 1 << blanks[i];
	  }
	}
	/* Okay, we now have the column numbers in cover that need turned
	 * on. */
	matrix_set_element(&cover, super_graph, y, 1);
	matrix_set_element(&cover, sub_graph, y, 1);
      }
    }
  }

  /* We've got the complete matrix in cover now. */

  {
    cw_uint32_t num_essentials;
    
    num_essentials = reduce(&cover);
    log_printf(g_log_o, "%u essentials\n", num_essentials);
    matrix_rebuild(&cover);
    log_printf(g_log_o, "Matrix size == %lu x %lu\n",
	       matrix_get_x_size(&cover), matrix_get_y_size(&cover));

    recurse(&cover, num_essentials, matrix_get_y_size(&cover));
  }
  
  matrix_delete(&graph);
  matrix_delete(&cover);
  _cw_free(map);
  glob_delete();
  return 0;
  /* End of main(). */
}

cw_bool_t
is_min_span_tree(cw_matrix_t * a_matrix_o, cw_uint32_t a_num_nodes)
{
  cw_bool_t retval = TRUE;
  cw_uint32_t * class;
  
  _cw_check_ptr(a_matrix_o);

  class = (cw_uint32_t *) _cw_malloc(sizeof(cw_uint32_t) * a_num_nodes);
  bzero(class, (sizeof(cw_uint32_t) * a_num_nodes));

  /* Are there exactly (a_num_nodes - 1) edges? */
  {
    cw_uint32_t i, j, num_on_total, num_on_row;

    for (j = 0, num_on_total = 0; j < a_num_nodes; j++)
    {
      for (i = 0, num_on_row = 0; i < a_num_nodes; i++)
      {
	if (matrix_get_element(a_matrix_o, i, j) == TRUE)
	{
	  num_on_row++;
	  num_on_total++;
	}
      }
      class[num_on_row]++;
    }

    if ((num_on_total / 2) != (a_num_nodes - 1))
    {
      retval = FALSE;
      goto RETURN;
    }
  }

  /* Run Dijkstra's algorithm to make sure we have a fully connected
   * graph. */
  {
    cw_bool_t * have_visited;
    cw_uint32_t * weights, * curr_path, curr_leg, i, j, new_i;
    cw_sint32_t least, curr_path_ele;

    /* Create current weights array and zero it out. */
    weights = (cw_uint32_t *) _cw_malloc(sizeof(cw_uint32_t) * a_num_nodes);
    curr_path = (cw_uint32_t *) _cw_malloc(sizeof(cw_uint32_t) * a_num_nodes);
    have_visited = (cw_bool_t *) _cw_malloc(sizeof(cw_bool_t) * a_num_nodes);
    for (i = 0; i < a_num_nodes; i++)
    {
      weights[i] = 0x7fffffff;
      curr_path[i] = 0;
      have_visited[i] = FALSE;
    }
    
    /*     bzero(weights, (sizeof(cw_uint32_t) * a_num_nodes)); */

    /* Main loop. */
    for (curr_leg = 0, i = 0, curr_path_ele = -1;
	 curr_leg < a_num_nodes;
	 curr_leg++, i = new_i)
    {
      new_i = 0;
      least = 0x7fffffff;
      have_visited[i] = TRUE;
      curr_path_ele++;
      curr_path[curr_path_ele] = i;
      
      /* Relax all nodes. */
      for (j = 0; j < a_num_nodes; j++)
      {
	if (matrix_get_element(a_matrix_o, i, j) != 0)
	{
	  if (curr_leg + matrix_get_element(a_matrix_o, i, j) < weights[j])
	  {
	    weights[j] = curr_leg + matrix_get_element(a_matrix_o, i, j);
	  }
	}
	if ((have_visited[j] == FALSE)
	    && (matrix_get_element(a_matrix_o, i, j) != 0)
	    && (matrix_get_element(a_matrix_o, i, j) < least))
	{
	  least = matrix_get_element(a_matrix_o, i, j);
	  new_i = j;
	}
      }
      if ((new_i == 0) && (curr_leg < a_num_nodes - 1))
      {
	cw_uint32_t k;
	
	/* We can't go anywhere else from the current node.  Start backing
	 * up on the current path to try to find an alternate path. */

	for (curr_path_ele--;
	     (curr_path_ele >= 0) && (new_i == 0);
	     curr_path_ele--)
	{
	  for (k = 0; k < a_num_nodes; k++)
	  {
	    if ((have_visited[k] == FALSE)
		&& (matrix_get_element(a_matrix_o,
				       k, curr_path[curr_path_ele])
		    != 0)
		)
	    {
	      curr_path_ele++;
	      new_i = k;
	      break;
	    }
	  }
	}
	if (new_i == 0)
	{
	  retval = FALSE;
	  goto CLEANUP;
	}
      }
    }

  CLEANUP:    
    _cw_free(weights);
    _cw_free(curr_path);
    _cw_free(have_visited);
  }

 RETURN:
  _cw_free(class);
    
  return retval;
}

cw_uint32_t
reduce(cw_matrix_t * a_m)
{
  cw_uint32_t num_essentials = 0;
  cw_bool_t did_reduce;

  /* Loop and do three reduction steps until we make a full pass through
   * all three steps without any change. */

  /* Main reduction loop. */
  did_reduce = TRUE;
  while ((did_reduce == TRUE)
	 && (matrix_get_x_size(a_m) > 1)
	 && (matrix_get_y_size(a_m) > 1))
  {
    did_reduce = FALSE;
      
    /* Check for row covering. */
    {
      cw_uint32_t x, y1, y2, y1_el, y2_el;
      cw_bool_t y1_is_subset, y2_is_subset;

      for (y1 = 0;
	   (y1 < matrix_get_y_size(a_m) - 1) && (did_reduce == FALSE);
	   y1++)
      {
	for (y2 = y1 + 1;
	     (y2 < matrix_get_y_size(a_m)) && (did_reduce == FALSE);
	     y2++)
	{
	  y1_is_subset = TRUE;
	  y2_is_subset = TRUE;
	  for (x = 0; x < matrix_get_x_size(a_m); x++)
	  {
	    y1_el = matrix_get_element(a_m, x, y1);
	    y2_el = matrix_get_element(a_m, x, y2);
	    if (y1_el < y2_el)
	    {
	      y2_is_subset = FALSE;
	      if (y1_is_subset == FALSE)
	      {
		/* No good, might as well quit comparing now. */
		break;
	      }
	    }
	    else if (y1_el > y2_el)
	    {
	      y1_is_subset = FALSE;
	      if (y2_is_subset == FALSE)
	      {
		/* No good, might as well quit comparing now. */
		break;
	      }
	    }
	  }
	  /* If either y1_is_subset or y2_is_subset is TRUE, we can
	   * remove a row. */
	  if (y1_is_subset == TRUE)
	  {
	    /* Remove y1. */
	    did_reduce = TRUE;

	    matrix_remove_row(a_m, y1);
	  }
	  else if (y2_is_subset == TRUE)
	  {
	    /* Remove y2. */
	    did_reduce = TRUE;

	    matrix_remove_row(a_m, y2);
	  }
	}
      }
    }

    /* Check for column matching. */
    if (did_reduce == FALSE)
    {
      cw_uint32_t x1, x2, y, x1_el, x2_el;
      cw_bool_t x_is_equal;
      
      for (x1 = 0; x1 < matrix_get_x_size(a_m) - 1; x1++)
      {
	for (x2 = x1 + 1; x2 < matrix_get_x_size(a_m); x2++)
	{
	  x_is_equal = TRUE;

	  for (y = 0; y < matrix_get_y_size(a_m); y++)
	  {
	    x1_el = matrix_get_element(a_m, x1, y);
	    x2_el = matrix_get_element(a_m, x2, y);
	    if (x1_el != x2_el)
	    {
	      x_is_equal = FALSE;
	      /* No good, might as well quit comparing now. */
	      break;
	    }
	  }
	  if (x_is_equal)
	  {
	    /* Remove x2. */
	    did_reduce = TRUE;

	    matrix_remove_column(a_m, x2);
	  }
	}
      }
    }

    /* Check for essentials. */
    if (did_reduce == FALSE)
    {
      cw_uint32_t x, y;
      cw_uint32_t num_on, essential;
	
      for (x = essential = 0; x < matrix_get_x_size(a_m); x++)
      {
	for (y = 0, num_on = 0; y < matrix_get_y_size(a_m); y++)
	{
	  if (matrix_get_element(a_m, x, y))
	  {
	    num_on++;
	    if (num_on > 1)
	    {
	      break;
	    }
	    essential = y;
	  }
	}
	if (num_on == 0)
	{
	  log_printf(g_log_o, "Empty column %u (iteration %u)\n",
		     x, x);
	  exit(1);
	}

	if (num_on == 1)
	{
	  did_reduce = TRUE;

	  remove_essential(a_m, essential);
	  num_essentials++;
	}
      }
    }
  }
  
  return num_essentials;
}

void
remove_essential(cw_matrix_t * a_m,
		 cw_uint32_t a_essential)
{
  cw_uint32_t i;

  for (i = 0; i < matrix_get_x_size(a_m); i++)
  {
    if (matrix_get_element(a_m, i, a_essential) != 0)
    {
      matrix_remove_column(a_m, i);
      i--; /* Don't skip over next column. */
    }
  }
}

cw_uint32_t
recurse(cw_matrix_t * a_m, cw_uint32_t a_curr_selected, cw_uint32_t a_best)
{
  cw_matrix_t * t_m;
  cw_uint32_t j, curr_essentials;

  /* For each row in a_m, select the row, reduce the matrix, and recurse if
   * no solution yet and we've not selected as many rows as the current
   * best solution. */
  for (j = 0; j < matrix_get_y_size(a_m); j++)
  {
    t_m = matrix_copy(a_m);

    remove_essential(t_m, j);

    curr_essentials = reduce(t_m);
    
    if ((matrix_get_x_size(t_m) > 0)
	&& (a_curr_selected + curr_essentials + 2 < a_best))
    {
      /* We haven't found a solution, but we're not over the current best
       * yet, so keep recursing. */
      
      a_best = recurse(t_m, a_curr_selected + curr_essentials + 1, a_best);
    }
    else if ((matrix_get_x_size(t_m) == 0)
	     && (a_curr_selected + curr_essentials + 1 < a_best))
    {
      char buf[65];
      
      /* Found a new solution. */
      g_tries++;
      log_printf(g_log_o, "Solution with %u rows (try %s)\n",
		 a_curr_selected + curr_essentials + 1,
		 log_print_uint64(g_tries, 10, buf));
      a_best = a_curr_selected + curr_essentials;
    }
    else
    {
      /* Don't search any deeper, since there's no chance of finding a
       * better solution. */
      g_tries++;
      if (g_tries % 2600 == 0)
      {
	char buf[65];
	log_printf(g_log_o, "Try %s\n", log_print_uint64(g_tries, 10, buf));
	exit(0);
      }
    }

    matrix_delete(t_m);
  }
  
  return a_best;
}
