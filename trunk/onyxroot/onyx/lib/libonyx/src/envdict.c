/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: Onyx <Version = onyx>
 *
 ******************************************************************************/

#include "../include/libonyx/libonyx.h"

#define	CW_NX_ENVDICT_SIZE	128

void
envdict_l_populate(cw_nxo_t *a_dict, cw_nxo_t *a_tkey, cw_nxo_t *a_tval,
		   cw_nx_t *a_nx, char **a_envp)
{
    int i;
    char *val_str;
    cw_uint8_t *t_str;
    cw_uint32_t key_len, val_len;

    nxo_dict_new(a_dict, a_nx, TRUE, CW_NX_ENVDICT_SIZE);

    if (a_envp != NULL)
    {
	/*
	 * Iterate through key/value strings in the environment vector
	 * and insert them into the dictionary.
	 */
	for (i = 0; a_envp[i] != NULL; i++)
	{
	    /* Find the '=' that separates key and value. */
	    val_str = strchr(a_envp[i], '=');
	    key_len = val_str - a_envp[i];
	    val_str++;

	    /* Create key. */
	    nxo_name_new(a_tkey, a_nx, a_envp[i], key_len, FALSE);

	    /* Create value. */
	    val_len = strlen(val_str);
	    nxo_string_new(a_tval, a_nx, TRUE, val_len);
	    t_str = nxo_string_get(a_tval);
#ifdef CW_THREADS
	    nxo_string_lock(a_tval);
#endif
	    memcpy(t_str, val_str, val_len);
#ifdef CW_THREADS
	    nxo_string_unlock(a_tval);
#endif

	    /* Insert into dictionary. */
	    nxo_dict_def(a_dict, a_nx, a_tkey, a_tval);
	}
    }
}
