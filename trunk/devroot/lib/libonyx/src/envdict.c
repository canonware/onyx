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
envdict_l_populate(cw_nxo_t *a_dict, cw_nx_t *a_nx, char **a_envp)
{
    int i;
    char *val_str;
    cw_uint8_t *t_str;
    cw_uint32_t key_len, val_len;
    cw_nxo_t key_nxo, val_nxo;

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
	    nxo_name_new(&key_nxo, a_nx, a_envp[i], key_len, FALSE);

	    /* Create value. */
	    val_len = strlen(val_str);
	    nxo_string_new(&val_nxo, a_nx, TRUE, val_len);
	    t_str = nxo_string_get(&val_nxo);
#ifdef CW_THREADS
	    nxo_string_lock(&val_nxo);
#endif
	    memcpy(t_str, val_str, val_len);
#ifdef CW_THREADS
	    nxo_string_unlock(&val_nxo);
#endif

	    /* Insert into dictionary. */
	    nxo_dict_def(a_dict, a_nx, &key_nxo, &val_nxo);
	}
    }
}
