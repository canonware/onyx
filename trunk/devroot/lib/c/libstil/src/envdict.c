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

#include "../include/libstil/libstil.h"

#define	_CW_STIL_ENVDICT_SIZE	128

void
envdict_l_populate(cw_stilo_t *a_dict, cw_stil_t *a_stil, char **a_envp)
{
	int		i;
	char		*key_str, *val_str;
	cw_uint8_t	*t_str;
	cw_uint32_t	key_len, val_len;
	cw_stilo_t	key_stilo, val_stilo;

	stilo_dict_new(a_dict, a_stil, _CW_STIL_ENVDICT_SIZE);

	if (a_envp != NULL) {
		/*
		 * Iterate through key/value strings in the environment vector
		 * and insert them into the dictionary.
		 */
		for (i = 0; a_envp[i] != NULL; i++) {
			/* Break the key and value apart. */
			val_str = a_envp[i];
			key_str = strsep(&val_str, "=");
			key_len = val_str - key_str - 1;
			val_len = strlen(val_str);

			/* Create key. */
			stilo_name_new(&key_stilo, a_stil, key_str, key_len,
			    FALSE);

			/* Create value. */
			stilo_string_new(&val_stilo, a_stil, val_len);
			t_str = stilo_string_get(&val_stilo);
			memcpy(t_str, val_str, val_len);

			/* Insert into dictionary. */
			stilo_dict_def(a_dict, a_stil, &key_stilo, &val_stilo);
		}
	}
}
