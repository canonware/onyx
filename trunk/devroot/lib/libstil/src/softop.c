static void
stil_p_soft_init(cw_stil_t *a_stil)
{
	_cw_stil_code(&a_stil->stilt, "
/search
{dup type /stringtype ne {errordict /typecheck get eval} if 1 index type /stringtype ne {errordict /typecheck get eval} if dup length 2 index length le {0 1 3 index length 3 index length sub {dup 3 index exch 3 index length getinterval dup length 1 sub 0 1 3 2 roll {dup 2 index exch get exch 4 index exch get ne {false exit} if} for dup type /booleantype eq {pop pop pop} {3 2 roll pop 3 1 roll dup 2 index exch 0 exch getinterval 3 1 roll 3 index length add dup 2 index length exch sub getinterval 3 1 roll true exit} ifelse} for dup type /booleantype ne {pop false} if} {pop false} ifelse}
/system
{fork dup 0 eq {errordict begin /handleerror {quit} def end pop exec} {exch pop waitpid} ifelse}
systemdict begin 2 {bind def} repeat end
");
}
