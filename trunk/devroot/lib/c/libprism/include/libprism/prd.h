/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

typedef struct cw_prd_s cw_prd_t;

struct cw_prd_s {
	cw_prw_t	*root;
};

cw_prd_t *prd_new(cw_prd_t *a_prd, cw_uint32_t a_rows, cw_uint32_t a_cols, const
    cw_uint8_t *a_term, cw_uint8_t *a_dev);
void	prd_delete(cw_prd_t *a_prd);
void	prd_suspend(cw_prd_t *a_prd);
void	prd_resume(cw_prd_t *a_prd);

cw_uint32_t prd_cmodel_get(cw_prd_t *a_prd);

void	prd_refresh(cw_prd_t *a_prd);

cw_prw_t *prd_root(cw_prd_t *a_prd);

void	prd_size_get(cw_prd_t *a_prd, cw_uint32_t *r_rows, cw_uint32_t *r_cols);
void	prd_size_set(cw_prd_t *a_prd, cw_uint32_t a_rows, cw_uint32_t a_cols);

void	prd_pos_get(cw_prd_t *a_prd, cw_uint32_t *r_row, cw_uint32_t *r_col);
cw_bool_t prd_pos_set(cw_prd_t *a_prd, cw_uint32_t a_row, cw_uint32_t a_col);
