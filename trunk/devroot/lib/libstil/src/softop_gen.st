%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% <Copyright = jasone>
% <License>
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% Version: <Version>
%
% Read stil code from stdin and output a compacted version to stdout.  The
% output code defines the C function stil_p_soft_init().
%
% This program is run by the 'stil_bootstrap' build system target.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Re-define various operators so that they won't take effect.
/systemdict {} def
/begin {} def
/end {} def
/bind {} def
/def {} def % Do this one last.

% Output the header.
`/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

static void
stil_p_soft_init(cw_stil_t *a_stil)
{
	_cw_stil_code(&a_stil->stilt, "
' print

% Read in the code.
stdin cvx eval

% Stash the number of objects on ostack at the bottom of ostack.
count dup dup 2 add 1 roll

% spop all the objects on the stack from bottom to top (to preserve final
% order).
-1 1 {
	-1 roll
	% Choose a recursion depth large enough to meet or exceed the depth of
	% any nested braces in the code.
	1000 spop
} for

`systemdict begin ' print
2 div cvs print
` {bind def} repeat end
");
}
' print
