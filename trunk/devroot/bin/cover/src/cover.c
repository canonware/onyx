/******************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#include "cover.h"

#include <limits.h>

typedef struct
{
	cw_uint32_t x_pos;
	cw_uint32_t y_pos;
} map_el_t;

/* Function prototypes. */
cw_uint32_t	fitness(cw_matrix_t *a_matrix, cw_uint32_t a_row_score,
    cw_uint32_t a_col_score);
cw_uint32_t	row_score(gene_t *a_gene, cw_uint32_t a_gene_length);
cw_uint32_t	gene_pack_col_score(pack_t *a_pack, gene_t *a_gene);
cw_matrix_t	*build_full_matrix(cw_uint32_t a_nnodes, cw_matrix_t ***
    r_graphs);
cw_bool_t	is_min_span_tree(cw_matrix_t *, cw_uint32_t);
cw_uint32_t	reduce(cw_matrix_t *a_m, cw_matrix_t ** r_x_index, cw_matrix_t
    ** r_y_index);
cw_bool_t	reduce_rows(cw_matrix_t *a_m, cw_matrix_t *a_x_index,
    cw_matrix_t *a_y_index);
cw_bool_t	reduce_columns(cw_matrix_t *a_m, cw_matrix_t *a_x_index,
    cw_matrix_t *a_y_index);
cw_uint32_t	reduce_essentials(cw_matrix_t *a_m, cw_matrix_t *a_x_index,
    cw_matrix_t *a_y_index);

void		remove_essential(cw_matrix_t *a_m, cw_matrix_t *a_x_index,
    cw_matrix_t *a_y_index, cw_uint32_t a_essential);
void		genetic_cover(cw_matrix_t *matrix, cw_matrix_t *x_index,
    cw_matrix_t *y_index);
cw_uint32_t	genetic_score(cw_matrix_t *matrix, cw_matrix_t *a_y_index,
    pack_t *a_pack, gene_t *genes, cw_uint32_t a_gene_size);
void		genetic_reproduce(cw_matrix_t *matrix, cw_matrix_t *a_y_index,
    pack_t *a_pack, gene_t *genes, gene_t *tga, cw_uint32_t a_gene_size,
    cw_uint32_t a_score_sum);
void		usage(const char *a_progname);
const char	*basename(const char *a_str);

/* Global variables. */
cw_uint64_t	g_tries = 0;
cw_uint32_t	opt_nnodes = NUM_NODES;
cw_uint32_t	opt_ngens = _GENERATIONS;
cw_uint32_t	opt_seed = _SEED;
cw_uint32_t	opt_mutate = _MUTATE_PROBABILITY_INV;
cw_uint32_t	opt_psize = _POP_SIZE;
cw_uint32_t	opt_key = FALSE;
cw_uint32_t	opt_matrix = FALSE;

int
main(int argc, char **argv)
{
	int		retval;
	cw_matrix_t	*matrix = NULL;
	cw_matrix_t	*x_index, *y_index;
	cw_matrix_t	**graphs = NULL;
	cw_uint32_t	num_essentials;

	int		c;
	cw_bool_t	cl_error = FALSE;
	cw_bool_t	opt_help = FALSE;

	libstash_init();

	/* Parse command line. */
	while (-1 != (c = getopt(argc, argv, "hktn:g:s:m:p:"))) {
		switch (c) {
		case 'h':
			opt_help = TRUE;
			break;
		case 'k':
			opt_key = TRUE;
			break;
		case 't':
			opt_matrix = TRUE;
			break;
		case 'n':
			opt_nnodes = strtoul(optarg, NULL, 10);
			break;
		case 'g':
			opt_ngens = strtoul(optarg, NULL, 10);
			break;
		case 's':
			opt_seed = strtoul(optarg, NULL, 10);
			break;
		case 'm':
			opt_mutate = strtoul(optarg, NULL, 10);
			break;
		case 'p':
			opt_psize = strtoul(optarg, NULL, 10);
			break;
		default:
			cl_error = TRUE;
			break;
		}
	}

	if ((cl_error) || (optind < argc)) {
		_cw_out_put("Unrecognized option(s)\n");
		usage(basename(argv[0]));
		retval = 1;
		goto RETURN;
	}

	if (opt_help) {
		usage(basename(argv[0]));
		retval = 0;
		goto RETURN;
	}

	if (opt_nnodes > 6)
		_cw_out_put("Too many nodes (maximum 6)\n");

	_cw_out_put("opt_nnodes == [i]\n", opt_nnodes);

	matrix = build_full_matrix(opt_nnodes, &graphs);

	if (opt_key && graphs != NULL) {
		cw_uint32_t	i;

		for (i = 0; graphs[i] != NULL; i++) {
			_cw_out_put("Row [i]:\n", i);
			matrix_dump(graphs[i], "\t", FALSE);
		}
	}

	if (opt_matrix) {
		_cw_out_put("Matrix before reduction:\n");
		matrix_dump(matrix, "before: ", TRUE);
	}

	num_essentials = reduce(matrix, &x_index, &y_index);
	_cw_out_put("[i] essentials\n", num_essentials);
	matrix_rebuild(matrix);
	_cw_out_put("Matrix size == [i] x [i]\n",
	    matrix_get_x_size(matrix), matrix_get_y_size(matrix));

	if (opt_key && graphs != NULL) {
		cw_uint32_t	i;

		for (i = 0; graphs[i] != NULL; i++) {
			_cw_out_put("Row [i]:\n", i);
			matrix_dump(graphs[matrix_get_element(y_index, 0, i)],
			    "\t", FALSE);
		}
	}

	if (opt_matrix) {
		_cw_out_put("Matrix after reduction:\n");
		matrix_dump(matrix, "after: ", TRUE);
	}

	genetic_cover(matrix, x_index, y_index);

	matrix_delete(x_index);
	matrix_delete(y_index);

	retval = 0;

	RETURN:
	if (matrix != NULL) {
		if (NULL != graphs) {
			cw_uint32_t	i;

			for (i = 0; graphs[i] != NULL; i++)
				matrix_delete(graphs[i]);

			_cw_free(graphs);
		}

		matrix_delete(matrix);
	}
	libstash_shutdown();
	return retval;
}

cw_uint32_t
fitness(cw_matrix_t *a_matrix, cw_uint32_t a_row_score,
    cw_uint32_t a_col_score)
{
	cw_uint32_t retval;
	cw_uint32_t r, c, cp;

	r = matrix_get_y_size(a_matrix) + 1;
	c = matrix_get_x_size(a_matrix);
	cp = c + 1;

	retval = (c * c / (r - a_row_score) / (r - a_row_score)
	    / (cp - a_col_score) / (cp - a_col_score));

	return retval;
}

/*
 * Calculate the number of rows that a gene covers.
 */
cw_uint32_t
row_score(gene_t *a_gene_o, cw_uint32_t a_gene_length)
{
	cw_uint32_t i, retval;

	for (i = retval = 0; i < a_gene_length; i++)
		retval += ! gene_get_locus(a_gene_o, i);
	return retval;
}

/*
 * Calculate the number of columns that a gene covers.
 */
cw_uint32_t
gene_pack_col_score(pack_t *a_pack, gene_t *a_gene)
{
	/*
	 * gcc 2.95.2 somehow screws up the inner loop of the first version such
	 * that it doesn't terminate when it should.  A bit of fiddling shows
	 * that the second and third versions, while slightly slower, aren't
	 * subject to gcc's wrath.
	 */
#if (0)
	cw_uint32_t	retval, i, j, splat;

	for (i = retval = 0; i < a_pack->x_size; i++) {
		for (j = splat = 0;
		     j < a_pack->y_size;
		     j++) {
			splat |= (a_pack->pack[i * a_pack->y_size + j]
			    & ((cw_uint32_t *)a_gene->gene)[j]);
		}
		if (splat)
			retval++;
	}

	return retval;
#elif (0)
	cw_uint32_t	retval, i, j, splat;

	for (i = retval = 0; i < a_pack->x_size; i++) {
		splat = 0;
		for (j = 0;
		     j < a_pack->y_size;
		     j++) {
			if ((a_pack->pack[i * a_pack->y_size + j]
			    & ((cw_uint32_t *)a_gene->gene)[j]))
				splat = 1;
		}
		retval += splat;
	}

	return retval;
#else
	cw_uint32_t	retval, i, j;

	for (i = retval = 0; i < a_pack->x_size; i++) {
		for (j = 0;
		     j < a_pack->y_size;
		     j++) {
			if ((a_pack->pack[i * a_pack->y_size + j]
			    & ((cw_uint32_t *)a_gene->gene)[j])) {
				retval++;
				break;
			}
		}
	}

	return retval;
#endif
}

cw_matrix_t *
build_full_matrix(cw_uint32_t a_nnodes, cw_matrix_t *** r_graphs)
{
	cw_matrix_t	*graph, *cover, **graphs;
	cw_uint32_t	num_graphs, num_edges, num_min_graphs;
	map_el_t	*map;

	/* Hard-coded for (n <= 6). */
	graphs = (cw_matrix_t **) _cw_calloc(1296, sizeof(cw_matrix_t *));
	bzero(graphs, 1296 * sizeof(cw_matrix_t *));
	*r_graphs = graphs;

	num_edges = a_nnodes * ((a_nnodes - 1) / 2)
	    + (((a_nnodes + 1) / 2) * ((a_nnodes + 1) % 2));
	_cw_out_put("num_edges == [i]\n", num_edges);
	num_graphs = 1 << (a_nnodes * ((a_nnodes - 1) / 2)
	    + (((a_nnodes + 1) / 2) * ((a_nnodes + 1) % 2)));
	_cw_out_put("num_graphs == [i]\n", num_graphs);

	/* Create adjacency matrix, given number of nodes. */
	graph = matrix_new(NULL);
	matrix_init(graph, a_nnodes, a_nnodes, TRUE);

	/*
	 * We're mapping edges as such:
	 * {0, 1, ..., n} --> {(0, 1), (0, 2), ... , (0, n - 1),
	 *                     (1, 2), (1, 3), ... , (1, n - 1),
	 *                     ................................,
	 *                     (n - 4, n - 3), ... , (n - 2, n - 3),
	 *                     (n - 3, n - 2), (n - 2, n - 2),
	 *                     (n - 2, n - 1)}
	 *
	 * Of course, since this is an undirected graph, we need to turn on the
	 * other direction for each edge as well.
	 */

	map = (map_el_t *) _cw_malloc(sizeof(map_el_t) * num_edges);

	{
		cw_uint32_t	x, y, curr_map_el = 0;

		for (y = 0; y < a_nnodes; y++) {
			for (x = 0; x < a_nnodes; x++) {
				if (x > y) {
					map[curr_map_el].x_pos = x;
					map[curr_map_el].y_pos = y;

					curr_map_el++;
					if (curr_map_el > num_edges)
						break;
				}
			}
		}
	}

	/*
	 * Now, cycle through all the possible graphs, to find out how many
	 * minimal spanning trees there are.
	 */
	{
		cw_uint32_t	i;

		num_min_graphs = 0;
		for (i = 0; i < num_graphs; i++) {
			cw_uint32_t	curr_bit;

			matrix_init(graph, a_nnodes, a_nnodes, TRUE);

			for (curr_bit = 0; curr_bit < 28; curr_bit++) {
				if (i & (1 << curr_bit)) {
					matrix_set_element(graph,
					    map[curr_bit].x_pos,
					    map[curr_bit].y_pos, TRUE);
					matrix_set_element(graph,
					    map[curr_bit].y_pos,
					    map[curr_bit].x_pos, TRUE);
				}
			}
			if (is_min_span_tree(graph, a_nnodes)) {
				graphs[num_min_graphs] = matrix_copy(graph);
				num_min_graphs++;
			}
		}
	}

	_cw_out_put("Matrix size == [i] x [i]\n", num_graphs,
	    num_min_graphs);

	/* Create the big bad matrix. */
	cover = matrix_new(NULL);
	matrix_init(cover, num_graphs, num_min_graphs, TRUE);

	/*
	 * Now fill in the matrix.  The minimum spanning trees are in
	 * graphs[].
	 */
	{
		cw_uint32_t	y, i, j, k, curr_blanks, super_graph, sub_graph;
		cw_uint32_t	digits[15], blanks[10]; /*
							 * Hard-coded for (n <=
							 * 6).
							 */

		/* Do this for every mimimum spanning tree. */
		for (y = 0; y < num_min_graphs; y++) {
			/* Copy graph into digits. */
			for (j = 0, curr_blanks = 0; j < num_edges; j++) {
				digits[j] = matrix_get_element(graphs[y],
				    map[j].x_pos, map[j].y_pos);
				/* Record which digits are blank. */
				if (digits[j] == 0) {
					blanks[curr_blanks] = j;
					curr_blanks++;
				}
			}

			/*
			 * Iterate through all of the possible subset/superset
			 * graphs.
			 */
			for (j = 0; j < (1 << curr_blanks); j++) {
				/*
				 * Initialize sub_graph and super_graph, so that
				 * all we have left to do is | in the additional
				 * counter bits.
				 */
				sub_graph = 0;
				super_graph = 0;
				for (k = 0; k < num_edges; k++)
					super_graph |= (digits[k] << k);
	
				/*
				 * j contains our bit counter.  Extract the bits
				 * from j and interleave them into super_graph
				 * and sub_graph.
				 */
				for (i = 0; i < curr_blanks; i++) {
					if (j & (1 << i)) {
						super_graph |= 1 << blanks[i];
						sub_graph |= 1 << blanks[i];
					}
				}

				/*
				 * Okay, we now have the column numbers in cover
				 * that need turned on.
				 */
				matrix_set_element(cover, super_graph, y, 1);
				matrix_set_element(cover, sub_graph, y, 1);
			}
		}
	}

	/* We've got the complete matrix in cover now. */

	matrix_delete(graph);
	_cw_free(map);
	return cover;
}

cw_bool_t
is_min_span_tree(cw_matrix_t *a_matrix, cw_uint32_t a_nnodes)
{
	cw_bool_t	retval = TRUE;
	cw_uint32_t	*class;

	_cw_check_ptr(a_matrix);

	class = (cw_uint32_t *) _cw_malloc(sizeof(cw_uint32_t) * a_nnodes);
	bzero(class, (sizeof(cw_uint32_t) * a_nnodes));

	/* Are there exactly (a_nnodes - 1) edges? */
	{
		cw_uint32_t	i, j, num_on_total, num_on_row;

		for (j = 0, num_on_total = 0; j < a_nnodes; j++) {
			for (i = 0, num_on_row = 0; i < a_nnodes; i++) {
				if (matrix_get_element(a_matrix, i, j)) {
					num_on_row++;
					num_on_total++;
				}
			}
			class[num_on_row]++;
		}

		if ((num_on_total / 2) != (a_nnodes - 1)) {
			retval = FALSE;
			goto RETURN;
		}
	}

	/*
	 * Run Dijkstra's algorithm to make sure we have a fully connected
	 * graph.
	 */
	{
		cw_bool_t	*have_visited;
		cw_uint32_t	*weights, *curr_path, curr_leg, i, j, new_i;
		cw_sint32_t	least, curr_path_ele;

		/* Create current weights array and zero it out. */
		weights = (cw_uint32_t *)_cw_malloc(sizeof(cw_uint32_t) *
		    a_nnodes);
		curr_path = (cw_uint32_t *)_cw_malloc(sizeof(cw_uint32_t) *
		    a_nnodes);
		have_visited = (cw_bool_t *)_cw_malloc(sizeof(cw_bool_t) *
		    a_nnodes);
		for (i = 0; i < a_nnodes; i++) {
			weights[i] = 0x7fffffff;
			curr_path[i] = 0;
			have_visited[i] = FALSE;
		}

/*  		bzero(weights, (sizeof(cw_uint32_t) * a_nnodes)); */

		/* Main loop. */
		for (curr_leg = 0, i = 0, curr_path_ele = -1;
		     curr_leg < a_nnodes;
		     curr_leg++, i = new_i) {
			new_i = 0;
			least = 0x7fffffff;
			have_visited[i] = TRUE;
			curr_path_ele++;
			curr_path[curr_path_ele] = i;

			/* Relax all nodes. */
			for (j = 0; j < a_nnodes; j++) {
				if (matrix_get_element(a_matrix, i, j) != 0) {
					if (curr_leg +
					    matrix_get_element(a_matrix, i, j) <
					    weights[j]) {
						weights[j] = curr_leg +
						    matrix_get_element(a_matrix,
						    i, j);
					}
				}
				if ((have_visited[j] == FALSE)
				    && (matrix_get_element(a_matrix, i, j) != 0)
				    && (matrix_get_element(a_matrix, i, j) <
				    least)) {
					least = matrix_get_element(a_matrix, i,
					    j);
					new_i = j;
				}
			}
			if ((new_i == 0) && (curr_leg < a_nnodes - 1)) {
				cw_uint32_t	k;
	
				/*
				 * We can't go anywhere else from the current
				 * node.  Start backing up on the current path
				 * to try to find an alternate path.
				 */
				for (curr_path_ele--;
				     (curr_path_ele >= 0) && (new_i == 0);
				     curr_path_ele--) {
					for (k = 0; k < a_nnodes; k++) {
						if ((have_visited[k] == FALSE)
						    &&
						    (matrix_get_element(a_matrix,
						    k, curr_path[curr_path_ele])
						    != 0)) {
							curr_path_ele++;
							new_i = k;
							break;
						}
					}
				}
				if (new_i == 0) {
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
reduce(cw_matrix_t *a_m, cw_matrix_t **r_x_index, cw_matrix_t **r_y_index)
{
	cw_bool_t	did_reduce;
	cw_matrix_t	*x_index, *y_index;
	cw_uint32_t	nessentials = 0;
	cw_uint32_t	i;

	/*
	 * Create indices that can be used to figure out the origins of rows and
	 * colums.  Every time a row or column is removed from the big matrix,
	 * so should that row or column be removed from the indices.
	 */
	x_index = matrix_new(NULL);
	matrix_init(x_index, matrix_get_x_size(a_m), 1, FALSE);
	for (i = 0; i < matrix_get_x_size(x_index); i++)
		matrix_set_element(x_index, i, 0, i);
	*r_x_index = x_index;

	y_index = matrix_new(NULL);
	matrix_init(y_index, 1, matrix_get_y_size(a_m), FALSE);
	for (i = 0; i < matrix_get_y_size(y_index); i++)
		matrix_set_element(y_index, 0, i, i);
	*r_y_index = y_index;

	/*
	 * Loop and do three reduction steps until we make a full pass through
	 * all three steps without any change.
	 */

	/* Main reduction loop. */
	did_reduce = TRUE;
	while ((did_reduce)
	    && (matrix_get_x_size(a_m) > 1)
	    && (matrix_get_y_size(a_m) > 1)) {
		/* Check for row covering. */
		_cw_out_put("rows        [i|w:5] x [i|w:4] --> ",
		    matrix_get_x_size(a_m), matrix_get_y_size(a_m));
		did_reduce = reduce_rows(a_m, x_index, y_index);
		_cw_out_put("[i|w:5] x [i|w:4]\n", matrix_get_x_size(a_m),
		    matrix_get_y_size(a_m));

		/* Check for column matching. */
		if (did_reduce == FALSE) {
			_cw_out_put("columns     [i|w:5] x [i|w:4] --> ",
			    matrix_get_x_size(a_m), matrix_get_y_size(a_m));
			did_reduce = reduce_columns(a_m, x_index, y_index);
			_cw_out_put("[i|w:5] x [i|w:4]\n",
			    matrix_get_x_size(a_m), matrix_get_y_size(a_m));
		}

		/* Check for essentials. */
		if (did_reduce == FALSE) {
			cw_uint32_t	xbefore, ybefore, essentials;

			xbefore = matrix_get_x_size(a_m);
			ybefore = matrix_get_y_size(a_m);
			_cw_out_put("essentials:");
			essentials = reduce_essentials(a_m, x_index, y_index);
			if (essentials > 0) {
				did_reduce = TRUE;
				nessentials += essentials;
			}
			_cw_out_put("\nessentials  [i|w:5] x [i|w:4] -->"
			    " [i|w:5] x [i|w:4]\n",
			    xbefore, ybefore, matrix_get_x_size(a_m),
			    matrix_get_y_size(a_m));
		}
	}

	return nessentials;
}

cw_bool_t
reduce_rows(cw_matrix_t *a_m, cw_matrix_t *a_x_index, cw_matrix_t *a_y_index)
{
	cw_bool_t	did_reduce = FALSE;
	cw_uint32_t	x, y1, y2, y1_el, y2_el;
	cw_bool_t	y1_is_subset, y2_is_subset;

	for (y1 = 0;
	     (y1 < matrix_get_y_size(a_m) - 1) && (did_reduce == FALSE);
	     y1++) {
		for (y2 = y1 + 1;
		     (y2 < matrix_get_y_size(a_m)) && (did_reduce == FALSE);
		     y2++) {
			y1_is_subset = TRUE;
			y2_is_subset = TRUE;
			for (x = 0; x < matrix_get_x_size(a_m); x++) {
				y1_el = matrix_get_element(a_m, x, y1);
				y2_el = matrix_get_element(a_m, x, y2);
				if (y1_el < y2_el) {
					y2_is_subset = FALSE;
					if (y1_is_subset == FALSE) {
						/*
						 * No good, might as well quit
						 * comparing now.
						 */
						break;
					}
				} else if (y1_el > y2_el) {
					y1_is_subset = FALSE;
					if (y2_is_subset == FALSE) {
						/*
						 * No good, might as well quit
						 * comparing now.
						 */
						break;
					}
				}
			}
				/*
				 * If either y1_is_subset or y2_is_subset is
				 * TRUE, we can remove a row.
				 */
			if (y1_is_subset) {
				/* Remove y1. */
				did_reduce = TRUE;

				matrix_remove_row(a_m, y1);
				matrix_remove_row(a_y_index, y1);
			} else if (y2_is_subset) {
				/* Remove y2. */
				did_reduce = TRUE;

				matrix_remove_row(a_m, y2);
				matrix_remove_row(a_y_index, y2);
			}
		}
	}

	return did_reduce;
}

cw_bool_t
reduce_columns(cw_matrix_t *a_m, cw_matrix_t *a_x_index, cw_matrix_t *a_y_index)
{
	cw_bool_t	did_reduce = FALSE;
	cw_uint32_t	x1, x2, y, x1_el, x2_el;
	cw_uint32_t	i, j;
	cw_bool_t	x_is_equal;
	cw_matrix_t	*t_matrix;

	/*
	 * Make a copy of a_m in column-major order to improve cache
	 * locality.
	 */
	t_matrix = matrix_new(NULL);
	matrix_init(t_matrix, matrix_get_y_size(a_m),matrix_get_x_size(a_m),
	    TRUE);

	for (j = 0; j < matrix_get_y_size(a_m); j++) {
		for (i = 0; i < matrix_get_x_size(a_m); i++) {
			matrix_set_element(t_matrix, j, i,
			    matrix_get_element(a_m, i, j));
		}
	}

	for (x1 = 0; x1 < matrix_get_y_size(t_matrix) - 1; x1++) {
		for (x2 = x1 + 1; x2 < matrix_get_y_size(t_matrix); x2++) {
			x_is_equal = TRUE;

			for (y = 0; y < matrix_get_x_size(t_matrix); y++) {
				x1_el = matrix_get_element(t_matrix, y, x1);
				x2_el = matrix_get_element(t_matrix, y, x2);
				if (x1_el != x2_el) {
					x_is_equal = FALSE;
					/*
					 * No good, might as well quit comparing
					 * now.
					 */
					break;
				}
			}
			if (x_is_equal) {
				/* Remove x2. */
				did_reduce = TRUE;

				matrix_remove_row(t_matrix, x2);
				matrix_remove_column(a_m, x2);
				matrix_remove_column(a_x_index, x2);
			}
		}
	}

	matrix_delete(t_matrix);
	return did_reduce;
}

cw_uint32_t
reduce_essentials(cw_matrix_t *a_m, cw_matrix_t *a_x_index, cw_matrix_t
    *a_y_index)
{
	cw_uint32_t	nessentials = 0;
	cw_uint32_t	x, y;
	cw_uint32_t	num_on, essential;
	
	for (x = essential = 0; x < matrix_get_x_size(a_m); x++) {
		for (y = 0, num_on = 0; y < matrix_get_y_size(a_m); y++) {
			if (matrix_get_element(a_m, x, y)) {
				num_on++;
				if (num_on > 1)
					break;
				essential = y;
			}
		}
		_cw_assert(num_on > 0);

		if (num_on == 1) {
			_cw_out_put(" [i]", matrix_get_element(a_y_index, 0,
			    essential));
			remove_essential(a_m, a_x_index, a_y_index, essential);
			nessentials++;
		}
	}
	return nessentials;
}

void
remove_essential(cw_matrix_t *a_m, cw_matrix_t *a_x_index,
    cw_matrix_t *a_y_index, cw_uint32_t a_essential)
{
	cw_uint32_t	i;

	for (i = 0; i < matrix_get_x_size(a_m); i++) {
		if (matrix_get_element(a_m, i, a_essential) != 0) {
			matrix_remove_column(a_m, i);
			matrix_remove_column(a_x_index, i);
			i--; /* Don't skip over next column. */
		}
	}

	matrix_remove_row(a_m, a_essential);
	matrix_remove_row(a_y_index, a_essential);
}

/*
 * Use a genetic algorithm to create generations of partial solutions.
 */
void
genetic_cover(cw_matrix_t *matrix, cw_matrix_t *x_index, cw_matrix_t *y_index)
{
	cw_uint32_t	i, j;
	gene_t		*genes1, *genes2, *genes, *tga, *tgb;
	cw_uint32_t	gene_size, score_sum;
	pack_t		pack;

	genes1 = (gene_t *) _cw_malloc(opt_psize * sizeof(gene_t));
	genes2 = (gene_t *) _cw_malloc(opt_psize * sizeof(gene_t));
	for (i = 0; i < opt_psize; i++) {
		gene_new(&genes1[i], matrix_get_y_size(matrix));
		gene_new(&genes2[i], matrix_get_y_size(matrix));
	}

	/* Randomly fill all the bits in the genes. */
	genes = genes1;
	tga = genes2;
	srandom(opt_seed);
	_cw_out_put("pool size: [i], generations: [i], crossover == [i]%, "
		"mutate == 1/[i], opt_seed == [i]\n", opt_psize, opt_ngens,
	    _CROSSOVER_PROBABILITY, opt_mutate, opt_seed);

	for (i = 0; i < opt_psize; i++) {
		for (j = 0; j < matrix_get_y_size(matrix); j++)
			gene_set_locus(&genes[i], j, random() & 0x1);
	}

	pack_new(&pack, matrix_get_x_size(matrix),
	    matrix_get_y_size(matrix));
	/*
	 * Create the packed version of the reduced matrix.  This only works for
	 * n=5, but the fitness function is _much_ faster when using this.
	 */
	for (i = 0; i < matrix_get_x_size(matrix); i++) {
		for (j = 0; j < matrix_get_y_size(matrix); j++) {
			if (matrix_get_element(matrix, i, j)) {
				pack_set_el(pack, i, j);
				_cw_assert(pack_get_el(pack, i, j) == 1);
			} else {
				_cw_assert(pack_get_el(pack, i, j) == 0);
			}
		}
	}

	gene_size = matrix_get_y_size(matrix);
	/* Generation loop. */
	for (i = 0; i < opt_ngens; i++) {

		_cw_out_put("Generation [i]\n", i);

/*  		_cw_out_put("Scoring...\n", i); */
		score_sum = genetic_score(matrix, y_index, &pack, genes,
		    gene_size);

/*  		_cw_out_put("Reproducing...\n", i); */
		genetic_reproduce(matrix, y_index, &pack, genes, tga,
		    gene_size, score_sum);

		/* Switch to the new gene pool. */
		tgb = genes;
		genes = tga;
		tga = tgb;
	}

	for (i = 0; i < opt_psize; i++) {
		gene_delete(&genes1[i]);
		gene_delete(&genes2[i]);
	}
	_cw_free(genes1);
	_cw_free(genes2);
	pack_delete(&pack);
}

/*
 * Score all of the genes, keeping a running total of the scores.
 */
cw_uint32_t
genetic_score(cw_matrix_t *matrix, cw_matrix_t *a_y_index, pack_t *a_pack,
    gene_t *genes, cw_uint32_t a_gene_size)
{
	cw_uint32_t	i, j;
	cw_uint32_t	gen_best = a_gene_size;
	cw_uint32_t	gen_total;
	cw_uint32_t	gen_best_score;
	cw_uint32_t	num_solutions;
	cw_uint32_t	score_sum;
	cw_uint32_t	row, col;

	for (j = score_sum = gen_best_score = gen_total = num_solutions = 0; j <
	    opt_psize; j++) {
		row = row_score(&genes[j], a_gene_size);
		col = gene_pack_col_score(a_pack, &genes[j]);
	
		genes[j].score = fitness(matrix, row, col);
		score_sum += genes[j].score;
		genes[j].total = score_sum;

		if (matrix_get_x_size(matrix) == col) {
			static cw_uint32_t	best = 0xffffffff;

			if ((a_gene_size - row) < best) {
				best = (a_gene_size - row);
				_cw_out_put("---------------------------------"
				    "--------------\n");
				_cw_out_put("<<< genes[[[i]] covers using "
				    "[i|s:s] rows >>>\n",
				    j, matrix_get_y_size(matrix) - row);
				_cw_out_put("Rows:");
				for (i = 0; i <
					 matrix_get_y_size(matrix); i++) {
					if (gene_get_locus(&genes[j],
					    i)) {
						_cw_out_put(" [i]",
						    matrix_get_element(a_y_index,
						    0, i));
					}
				}
				_cw_out_put("\n");
	    
				_cw_out_put("---------------------------------"
				    "--------------\n");
			}
			if ((a_gene_size - row) < gen_best) {
				gen_best = (a_gene_size - row);
				gen_best_score = genes[j].score;
			}
			gen_total += (matrix_get_y_size(matrix) - row);
			num_solutions++;
		}
	}

	_cw_out_put("  Solutions/Total = [i]/[i], Best Solution = [i], "
	    "Average solution = [i]\n", num_solutions, opt_psize,
	    gen_best, num_solutions ? (gen_total / num_solutions) : 0);
	_cw_out_put("  Best score = [i], Average score = [i]\n",
	    gen_best_score, score_sum / opt_psize);

	return score_sum;
}

/*
 * Create a new generation.  Randomly select two genes, then create children
 * with crossover and mutation.
 */
void
genetic_reproduce(cw_matrix_t *matrix, cw_matrix_t *a_y_index, pack_t *a_pack,
    gene_t *genes, gene_t *tga, cw_uint32_t a_gene_size, cw_uint32_t
    a_score_sum)
{
	cw_uint32_t	j, k, l, z, sel_a, sel_b, offset;

	for (j = 0; j < opt_psize; j += 2) {
		/* Select first gene. */
		offset = random() % a_score_sum;
	  
		for (l = (opt_psize / 2) - 1, z = 2, k = opt_psize >> z; ;
		     z++, k = ((opt_psize >> z) > 0)  ? (opt_psize >> z) : 1) {
			if (offset > genes[l].total) {
				/* Jump forward. */
				l += k;
			} else {
				if (l == 0) {
					/*
					 * We're at the first element, so this
					 * is it.
					 */
					break;
				} else {
					if (offset > genes[l - 1].total) {
						/* This is it. */
						break;
					} else {
						/* Need to jump backward. */
						l -= k;
					}
				}
			}
		}
		sel_a = l;
	  
				/* Select second gene. */
		offset = random() % a_score_sum;
		for (l = (opt_psize / 2) - 1,
			 z = 2,
			 k = opt_psize >> z;
		     ;
		     z++,
		     k = ((opt_psize >> z) > 0)  ? (opt_psize >> z) : 1) {
			if (offset > genes[l].total) {
				/* Jump forward. */
				l += k;
			} else {
				if (l == 0) {
					/*
					 * We're at the first element, so this
					 * is it.
					 */
					break;
				} else {
					if (offset > genes[l - 1].total) {
						/* This is it. */
						break;
					} else {
						/* Need to jump backward. */
						l -= k;
					}
				}
			}
		}
		sel_b = l;

		/* Copy the old genes into place in the new pool. */
		gene_copy(&tga[j], &genes[sel_a]);
		gene_copy(&tga[j + 1], &genes[sel_b]);

		/* Maybe do crossover. */
		if ((random() % 100) < _CROSSOVER_PROBABILITY) {
			cw_uint32_t m, crossover_point, temp;

			crossover_point = random() % a_gene_size;

			for (m = 0; m < a_gene_size; m++) {
				if (m < crossover_point) {
					/*
					 * Before crossover point.  Don't need
					 * to do anything.
					 */
				} else {
					/* After crossover point. */
					temp = gene_get_locus(&tga[j], m);
					gene_set_locus(&tga[j], m,
					    gene_get_locus(&tga[j + 1], m));
					gene_set_locus(&tga[j + 1], m, temp);
				}
			}
		}

				/* Mutate. */
		for (k = 0; k < a_gene_size; k++) {
			if ((random() % opt_mutate) == 0)
				gene_set_locus(&tga[j], k, !
				    gene_get_locus(&tga[j], k));
		}
		for (k = 0; k < a_gene_size; k++) {
			if ((random() % opt_mutate) == 0)
				gene_set_locus(&tga[j + 1], k, !
				    gene_get_locus(&tga[j + 1], k));
		}
	}
}
	
void
usage(const char *a_progname)
{
	_cw_out_put(
	    "[s] usage:\n"
	    "    [s] -h\n"
	    "    [s] [[-k] [[-t] [[-n <nnodes>] [[-g <ngens>] [[-s <seed>] "
	    "[[-c <crossover>] [[-m <mutate>] [[-p <psize>]\n"
	    "\n"
	    "    Option               | Description\n"
	    "    ---------------------+--------------------------------------"
	    "---------------\n"
	    "    -h                   | Print usage and exit.\n"
	    "    -k                   | Print row --> graph key.\n"
	    "    -t                   | Print matrices.\n"
	    "    -n <nnodes>          | Use a graph with <nnodes> nodes.\n"
	    "                         | (Default is [i].)\n"
	    "    -g <ngens>           | Process <ngens> generations.\n"
	    "                         | (Default is [i].)\n"
	    "    -s <seed>            | Use initial random seed <seed>.\n"
	    "                         | (Default is [i].)\n"
	    "    -c <crossover>       | Do crossover with <crossover> percent"
	    " probability.\n"
	    "                         | (Default is [i]%.)\n"
	    "    -m <mutate>          | Mutate with probability 1/<mutate>.\n"
	    "                         | (Default is 1/[i].)\n"
	    "    -p <psize>           | Population size\n"
	    "                         | (Default is [i]\n"
	    ,
	    a_progname, a_progname, a_progname,
	    NUM_NODES, _GENERATIONS, _SEED,
	    _CROSSOVER_PROBABILITY, _MUTATE_PROBABILITY_INV, _POP_SIZE
	     );
}

/* Doesn't strip trailing '/' characters. */
const char *
basename(const char *a_str)
{
	const char	*retval = NULL;
	cw_uint32_t	i;

	_cw_check_ptr(a_str);

	i = strlen(a_str);
	if (i > 0) {
		for (i--; /* Back up to last non-null character. */
		     i > 0;
		     i--) {
			if (a_str[i] == '/') {
				retval = &a_str[i + 1];
				break;
			}
		}
	}

	if (retval == NULL)
		retval = a_str;

	return retval;
}
