%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% <Copyright = jasone>
% <License>
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% Version: <Version>
%
% This file is processed by softop_gen.st to produce a more compact embedded
% version in softop.c.  Due to bootstrapping issues, this cannot be done
% automatically as part of the build process.  In order to make changes to this
% file take effect, either type:
%
%   cook stil_bootstrap
%
% or
%
%   cat softop.st | stil softop_gen.st > softop.c
%
% The format of the code in this file is fragile.  Only add definitions of the
% format:
%
%   /procname {...} bind def
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

systemdict begin

% :: and : implement a simple mechanism for qualified namespaces.  For example,
% one can type:
%
%   ::/systemdict/gcdict/dump :
%
/:: {
	mark
} bind def

/: {
	% Prime the loop.
	counttomark
	dup 0 eq {
		pop
		errordict begin unmatchedmark end
	}{
		-1 roll load
		{
			counttomark

			% If there are no more names to look up, break out of
			% the loop.
			dup 1 le {
				pop
				exit
			} if

			% Collapse the two top level names.
			-1 roll get
		} loop

		exch pop
		eval
	} ifelse
} bind def

% <lock> <proc> monitor -
/monitor {
	% Validate arguments.
	1 index type /mutextype ne {
		errordict begin typecheck end
	} if

	1 index lock
	eval
	unlock
} bind def

% - promptstring <string>
/promptstring {
	count `s:' exch cvs catenate `> ' catenate
} bind def

% - product <string>
/product {
	`Canonware stil'
} bind def

/pstack {
	ostack {
		1 sprint
	} foreach
	flush
} bind def

%
% <string> <pattern> search <post> <pattern> <pre> true
%                    search <string> false
%
/search {
	% Make sure the arguments are strings.
	% Pattern.
	dup type /stringtype ne {
		errordict begin typecheck end
	} if
	% String.
	1 index type /stringtype ne {
		errordict begin typecheck end
	} if

	% Make sure substring isn't longer than string.
	dup length 2 index length le {
		% Use a nested looping algorithm.  The outer loop advances the
		% offset <s_i> at which to start the search in the string <s>,
		% and the inner loop compares byte by byte with the pattern <p>.
		%
		% Take care not to cause a rangecheck error.
		0 1 3 index length 3 index length sub {
			% <s> <p> <s_i>

			% Create a substring <s_sub> of <s> that is the same
			% length as <p>, starting at <s_i>.
			dup 3 index exch 3 index length getinterval

			% <s> <p> <s_i> <s_sub>
			dup length 1 sub 0 1
			3 2 roll
			{
				% <s> <p> <s_i> <s_sub> <offset>
				dup 2 index exch get
				exch 4 index exch get
				ne {
					% Not equal.  Exit the inner loop.
					false exit
				} if
			} for

			% Check to see if the substring match failed.
			dup type /booleantype eq {
				% <s> <p> <s_i> <s_sub> false
				% Failure.  Clean up.
				pop pop pop
			}{
				% <s> <p> <s_i> <s_sub>
				% Success.
				3 2 roll pop
				% <s> <s_i> <s_sub>

				% Contstruct pre string <s_pre>.
				3 1 roll dup 2 index exch 0 exch
				% <s_sub> <s> <s_i> <s> 0 <s_i>
				getinterval
				% <s_sub> <s> <s_i> <s_pre>
				3 1 roll
				% <s_sub> <s_pre> <s> <s_i>

				% Contstruct post string <s_post>.
				3 index length add dup
				2 index length exch sub
				getinterval
				% <s_sub> <s_pre> <s_post>
				3 1 roll

				true
				% <s_post> <s_sub> <s_pre> true
				exit
			} ifelse
		} for

		% Check to see if we succeeded.  If not, set up the stack to
		% return failure.
		dup type /booleantype ne {
			% Failure.
			pop false
		} if
	}{
		% Substring is too long.
		pop false
	} ifelse
} bind def

% [(<path>) (<args>)...] system <exit_code>
/system {
	fork
	dup 0 eq {
		errordict begin
		/handleerror {quit} def
		end
		pop exec
	}{
		exch pop waitpid
	} ifelse
} bind def

% - version <string>
/version {
	`<Version>'
} bind def

end % systemdict
