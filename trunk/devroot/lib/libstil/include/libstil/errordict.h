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

void	errordict_populate(cw_stilo_t *a_dict, cw_stilt_t *a_stilt);

void	errordict_dictfull(cw_stilt_t *a_stilt);
void	errordict_dictstackoverflow(cw_stilt_t *a_stilt);
void	errordict_dictstackunderflow(cw_stilt_t *a_stilt);
void	errordict_execstackoverflow(cw_stilt_t *a_stilt);
void	errordict_interrupt(cw_stilt_t *a_stilt);
void	errordict_invalidaccess(cw_stilt_t *a_stilt);
void	errordict_invalidcontext(cw_stilt_t *a_stilt);
void	errordict_invalidexit(cw_stilt_t *a_stilt);
void	errordict_invalidfileaccess(cw_stilt_t *a_stilt);
void	errordict_ioerror(cw_stilt_t *a_stilt);
void	errordict_limitcheck(cw_stilt_t *a_stilt);
void	errordict_rangecheck(cw_stilt_t *a_stilt);
void	errordict_stackoverflow(cw_stilt_t *a_stilt);
void	errordict_stackunderflow(cw_stilt_t *a_stilt);
void	errordict_syntaxerror(cw_stilt_t *a_stilt);
void	errordict_timeout(cw_stilt_t *a_stilt);
void	errordict_typecheck(cw_stilt_t *a_stilt);
void	errordict_undefined(cw_stilt_t *a_stilt);
void	errordict_undefinedfilename(cw_stilt_t *a_stilt);
void	errordict_undefinedresource(cw_stilt_t *a_stilt);
void	errordict_undefinedresult(cw_stilt_t *a_stilt);
void	errordict_unmatchedmark(cw_stilt_t *a_stilt);
void	errordict_unregistered(cw_stilt_t *a_stilt);
void	errordict_vmerror(cw_stilt_t *a_stilt);
