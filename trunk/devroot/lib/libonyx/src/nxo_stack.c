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
 * A stack is stored as an array of nxo pointers, with slop room at both the
 * beginning and end of the array.  The slop room makes typical push/pop
 * operations very fast on average.  Additionally, the slop room is kept large
 * enough that repeated stack rotations are also reasonably fast, even in
 * pathological cases.
 *
 * Since stacks are central to Onyx, a great deal of care is taken to avoid
 * locking wherever possible.  This is hard, since the garbage collector can
 * preempt mutator threads at any time.  Therefore, the stack code makes heavy
 * use of memory barriers, along with a mirror array of nxo pointers, to make
 * sure that no matter where the stack code is preempted, the garbage collector
 * can reliably iterate over stack contents.
 *
 * Stack objects are allocated on a per-element basis, and a certain number of
 * elements are cached to avoid allocation/deallocation overhead in the common
 * case.  Doing chunked allocation of stack elements would be slightly more
 * memory efficient (and probably more cache friendly) in the common case, but
 * would require extra code complexity in the critical paths of pushing and
 * popping.
 *
 * By keeping the re-allocation algorithm simple, we are able to make common
 * stack operations very fast.
 *
 ******************************************************************************/

/* Compile non-inlined functions if not using inlines. */
#define CW_NXO_STACK_C_

#include "../include/libonyx/libonyx.h"
#include "../include/libonyx/nxa_l.h"
#include "../include/libonyx/nxo_l.h"
#include "../include/libonyx/nxo_stack_l.h"

void
nxo_stack_new(cw_nxo_t *a_nxo, bool a_locking, uint32_t a_mincount)
{
    cw_nxoe_stack_t *stack;

    stack = (cw_nxoe_stack_t *) nxa_malloc(sizeof(cw_nxoe_stack_t));

    nxoe_l_new(&stack->nxoe, NXOT_STACK, a_locking);
#ifdef CW_THREADS
    if (a_locking)
    {
	mtx_new(&stack->lock);
    }
#endif

    stack->nspare = 0;

    if (a_mincount < CW_LIBONYX_STACK_MINCOUNT)
    {
	stack->ahmin = stack->ahlen = CW_LIBONYX_STACK_MINCOUNT * 2;
    }
    else
    {
	stack->ahmin = stack->ahlen = a_mincount * 2;
    }

    stack->abase = 0;
    stack->abeg = stack->aend = stack->ahmin / 2;

    xep_begin();
    xep_try
    {
	stack->a = (cw_nxo_t **) nxa_malloc(stack->ahlen * 2 *
					    sizeof(cw_nxo_t *));
    }
    xep_catch (CW_ONYXX_OOM)
    {
	nxa_free(stack, sizeof(cw_nxoe_stack_t));
    }
    xep_end();

#ifdef CW_THREADS
    stack->rstate = RSTATE_NONE;
#endif
    stack->rbase = stack->ahlen;
    stack->r = stack->a;

    nxo_no_new(a_nxo);
    a_nxo->o.nxoe = (cw_nxoe_t *) stack;
    nxo_p_type_set(a_nxo, NXOT_STACK);

    nxa_l_gc_register((cw_nxoe_t *) stack);
}

CW_P_INLINE void
nxoe_p_stack_grow(cw_nxoe_stack_t *a_stack, uint32_t a_beg_pad,
		  uint32_t a_end_pad)
{
    uint32_t rhlen, count, pcount;

    /* Protect the current array. */
    rhlen = a_stack->ahlen;
    a_stack->rbase = a_stack->abase;
    a_stack->rbeg = a_stack->abeg;
    a_stack->rend = a_stack->aend;
#ifdef CW_THREADS
    mb_write();
    a_stack->rstate = RSTATE_RONLY;
    mb_write();
#endif

    /* Determine how large to make the new array. */
    count = a_stack->rend - a_stack->rbeg;
    pcount = count + a_beg_pad + a_end_pad;
    for (;
	 a_stack->ahlen < pcount * 2;
	 a_stack->ahlen *= 2)
    {
	/* Do nothing. */
    }

    /* Allocate a new array and copy. */
    a_stack->a = (cw_nxo_t **) nxa_malloc(a_stack->ahlen * 2 *
					  sizeof(cw_nxo_t *));
    a_stack->abase = 0;
    a_stack->abeg = ((a_stack->ahlen - pcount) / 2) + a_beg_pad;
    a_stack->aend = a_stack->abeg + count;
    memcpy(&a_stack->a[a_stack->abase + a_stack->abeg],
	   &a_stack->r[a_stack->rbase + a_stack->rbeg],
	   count * sizeof(cw_nxo_t *));

#ifdef CW_THREADS
    /* Unprotect. */
    mb_write();
    a_stack->rstate = RSTATE_NONE;
    mb_write();
#endif

    /* Update r*. */
    a_stack->rbase = a_stack->ahlen;
    nxa_free(a_stack->r, rhlen * 2 * sizeof(cw_nxo_t *));
    a_stack->r = a_stack->a;
}

/* Copy the entire array, centering the valid elements as though there were
 * a_beg_pad leading elements and a_end_pad trailing elements. */
CW_P_INLINE void
nxoe_p_stack_center(cw_nxoe_stack_t *a_stack, uint32_t a_beg_pad,
		    uint32_t a_end_pad)
{
    uint32_t trbase, count, pcount;

    /* Protect the region that is being moved. */
    trbase = a_stack->rbase;
    a_stack->rbase = a_stack->abase;
    a_stack->rbeg = a_stack->abeg;
    a_stack->rend = a_stack->aend;
#ifdef CW_THREADS
    mb_write();
    a_stack->rstate = RSTATE_RONLY;
    mb_write();
#endif

    /* Center. */
    count = a_stack->rend - a_stack->rbeg;
    pcount = count + a_beg_pad + a_end_pad;
    a_stack->abase = trbase;
    cw_assert(pcount <= a_stack->ahlen);
    a_stack->abeg = ((a_stack->ahlen - pcount) / 2) + a_beg_pad;
    a_stack->aend = a_stack->abeg + count;
    cw_assert(a_stack->aend < a_stack->ahlen);
    memcpy(&a_stack->a[a_stack->abase + a_stack->abeg],
	   &a_stack->r[a_stack->rbase + a_stack->rbeg],
	   count * sizeof(cw_nxo_t *));

#ifdef CW_THREADS
    /* Unprotect. */
    mb_write();
    a_stack->rstate = RSTATE_NONE;
#endif
}

void
nxo_stack_copy(cw_nxo_t *a_to, cw_nxo_t *a_from)
{
    cw_nxoe_stack_t *to, *fr;
    cw_nxo_t *nxo;
    uint32_t to_count, fr_count, i;

    cw_check_ptr(a_to);
    cw_dassert(a_to->magic == CW_NXO_MAGIC);

    to = (cw_nxoe_stack_t *) a_to->o.nxoe;
    cw_dassert(to->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(to->nxoe.type == NXOT_STACK);

    cw_check_ptr(a_from);
    cw_dassert(a_from->magic == CW_NXO_MAGIC);

    fr = (cw_nxoe_stack_t *) a_from->o.nxoe;
    cw_dassert(fr->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(fr->nxoe.type == NXOT_STACK);

    /* Grow or recenter a_to if necessary. */
    to_count = to->aend - to->abeg;
    fr_count = fr->aend - fr->abeg;
    if (to_count + fr_count > to->ahlen / 2)
    {
	nxoe_p_stack_grow(to, fr_count, 0);
    }
    else if (fr_count > to->abeg)
    {
	nxoe_p_stack_center(to, fr_count, 0);
    }

    /* Iteratively create nxo's, insert them into a_to, and dup the nxo contents
     * from a_from. */
    for (i = 0; to->nspare && i < fr_count; i++)
    {
	to->nspare--;
	nxo = to->spare[to->nspare];

	nxo_no_new(nxo);
	nxo_dup(nxo, fr->a[fr->abase + fr->abeg + i]);
	to->a[to->abase + to->abeg - fr_count + i] = nxo;
    }
    for (; i < fr_count; i++)
    {
	nxo = (cw_nxo_t *) nxa_malloc(sizeof(cw_nxo_t));

	nxo_no_new(nxo);
	nxo_dup(nxo, fr->a[fr->abase + fr->abeg + i]);
	to->a[to->abase + to->abeg - fr_count + i] = nxo;
    }

    /* Now that a_from's contents have been copied, adjust a_to's array bounds.
     * */
    mb_write();
    to->abeg -= fr_count;
}

void
nxoe_p_stack_shrink(cw_nxoe_stack_t *a_stack)
{
    uint32_t rhlen, count;

    /* Protect the current array. */
    rhlen = a_stack->ahlen;
    a_stack->rbase = a_stack->abase;
    a_stack->rbeg = a_stack->abeg;
    a_stack->rend = a_stack->aend;
#ifdef CW_THREADS
    mb_write();
    a_stack->rstate = RSTATE_RONLY;
    mb_write();
#endif

    /* Determine how large to make the new array. */
    for (count = a_stack->aend - a_stack->abeg;
	 a_stack->ahlen > count * 2 && a_stack->ahlen > a_stack->ahmin;
	 a_stack->ahlen /= 2)
    {
	/* Do nothing. */
    }

    /* Allocate a new array and copy. */
    a_stack->a = (cw_nxo_t **) nxa_malloc(a_stack->ahlen * 2 *
					  sizeof(cw_nxo_t *));
    a_stack->abase = 0;
    a_stack->abeg = (a_stack->ahlen - count) / 2;
    a_stack->aend = a_stack->abeg + count;
    memcpy(&a_stack->a[a_stack->abase + a_stack->abeg],
	   &a_stack->r[a_stack->rbase + a_stack->rbeg],
	   count * sizeof(cw_nxo_t *));

#ifdef CW_THREADS
    /* Unprotect. */
    mb_write();
    a_stack->rstate = RSTATE_NONE;
    mb_write();
#endif

    /* Update r*. */
    a_stack->rbase = a_stack->ahlen;
    nxa_free(a_stack->r, rhlen * 2 * sizeof(cw_nxo_t *));
    a_stack->r = a_stack->a;
}

/* The following functions handle special cases for various stack operations.
 * They are done as separate functions to keep the inline functions as small and
 * branch-free as possible. */

#ifdef CW_THREADS
uint32_t
nxoe_p_stack_count_locking(cw_nxoe_stack_t *a_stack)
{
    uint32_t retval;
    cw_nxoe_stack_t *stack;

    mtx_lock(&a_stack->lock);
    retval = stack->aend - stack->abeg;
    mtx_unlock(&a_stack->lock);

    return retval;
}
#endif

cw_nxo_t *
nxoe_p_stack_push_hard(cw_nxoe_stack_t *a_stack)
{
    cw_nxo_t *retval;

    /* Grow if more than half of the available slots will be filled after
     * insertion.  Otherwise, shift the array contents. */
    if (a_stack->aend - a_stack->abeg + 1 > a_stack->ahlen / 2)
    {
	nxoe_p_stack_grow(a_stack, 1, 0);
    }
    else
    {
	nxoe_p_stack_center(a_stack, 1, 0);
    }

    /* Allocate new object. */
    if (a_stack->nspare)
    {
	a_stack->nspare--;
	retval = a_stack->spare[a_stack->nspare];
    }
    else
    {
	retval = (cw_nxo_t *) nxa_malloc(sizeof(cw_nxo_t));
    }

    return retval;
}

#ifdef CW_THREADS
cw_nxo_t *
nxoe_p_stack_push_locking(cw_nxoe_stack_t *a_stack)
{
    cw_nxo_t *retval;

    mtx_lock(&a_stack->lock);
    retval = nxoe_p_stack_push(a_stack);
    mtx_unlock(&a_stack->lock);

    return retval;
}
#endif

cw_nxo_t *
nxoe_p_stack_bpush_hard(cw_nxoe_stack_t *a_stack)
{
    cw_nxo_t *retval;

    /* Grow if more than half of the available slots will be filled after
     * insertion.  Otherwise, shift the array contents. */
    if (a_stack->aend - a_stack->abeg + 1 > a_stack->ahlen / 2)
    {
	/* Grow. */
	nxoe_p_stack_grow(a_stack, 0, 1);
    }
    else
    {
	/* Shift the array contents. */
	nxoe_p_stack_center(a_stack, 0, 1);
    }

    /* Allocate and insert new object. */
    if (a_stack->nspare)
    {
	a_stack->nspare--;
	retval = a_stack->spare[a_stack->nspare];
    }
    else
    {
	retval = (cw_nxo_t *) nxa_malloc(sizeof(cw_nxo_t));
    }

    return retval;
}

#ifdef CW_THREADS
cw_nxo_t *
nxoe_p_stack_bpush_locking(cw_nxoe_stack_t *a_stack)
{
    cw_nxo_t *retval;

    mtx_lock(&a_stack->lock);
    retval = nxoe_p_stack_bpush(a_stack);
    mtx_unlock(&a_stack->lock);

    return retval;
}
#endif

#ifdef CW_THREADS
bool
nxoe_p_stack_pop_locking(cw_nxoe_stack_t *a_stack)
{
    bool retval;

    mtx_lock(&a_stack->lock);
    retval = nxoe_p_stack_pop(a_stack);
    mtx_unlock(&a_stack->lock);

    return retval;
}
#endif

#ifdef CW_THREADS
bool
nxoe_p_stack_bpop_locking(cw_nxoe_stack_t *a_stack)
{
    bool retval;

    mtx_lock(&a_stack->lock);
    retval = nxoe_p_stack_bpop(a_stack);
    mtx_unlock(&a_stack->lock);

    return retval;
}
#endif

void
nxoe_p_stack_npop_hard(cw_nxoe_stack_t *a_stack, uint32_t a_count)
{
    uint32_t i;

    /* Save spares. */
    for (i = 0; a_stack->nspare < CW_LIBONYX_STACK_CACHE && i < a_count; i++)
    {
	a_stack->spare[a_stack->nspare]
	    = a_stack->a[a_stack->abase + a_stack->abeg - a_count + i];
#ifdef CW_DBG
	memset(a_stack->spare[a_stack->nspare], 0x5a, sizeof(cw_nxo_t));
#endif
	a_stack->nspare++;
    }

    /* Discard nxo's since the spares array is full. */
    for (; i < a_count; i++)
    {
	nxa_free(a_stack->a[a_stack->abase + a_stack->abeg - a_count + i],
		 sizeof(cw_nxo_t));
    }
}

#ifdef CW_THREADS
bool
nxoe_p_stack_npop_locking(cw_nxoe_stack_t *a_stack, uint32_t a_count)
{
    bool retval;

    mtx_lock(&a_stack->lock);
    retval = nxoe_p_stack_npop(a_stack, a_count);
    mtx_unlock(&a_stack->lock);

    return retval;
}
#endif

void
nxoe_p_stack_nbpop_hard(cw_nxoe_stack_t *a_stack, uint32_t a_count)
{
    uint32_t i;

    /* Save spares. */
    for (i = 0; a_stack->nspare < CW_LIBONYX_STACK_CACHE && i < a_count; i++)
    {
	a_stack->spare[a_stack->nspare]
	    = a_stack->a[a_stack->abase + a_stack->aend + i];
#ifdef CW_DBG
	memset(a_stack->spare[a_stack->nspare], 0x5a, sizeof(cw_nxo_t));
#endif
	a_stack->nspare++;
    }

    /* Discard nxo's since the spares array is full. */
    for (; i < a_count; i++)
    {
	nxa_free(a_stack->a[a_stack->abase + a_stack->aend + i],
		 sizeof(cw_nxo_t));
    }
}

#ifdef CW_THREADS
bool
nxoe_p_stack_nbpop_locking(cw_nxoe_stack_t *a_stack, uint32_t a_count)
{
    bool retval;

    mtx_lock(&a_stack->lock);
    retval = nxoe_p_stack_nbpop(a_stack, a_count);
    mtx_unlock(&a_stack->lock);

    return retval;
}
#endif

#ifdef CW_THREADS
cw_nxo_t *
nxoe_p_stack_get_locking(cw_nxoe_stack_t *a_stack)
{
    cw_nxo_t *retval;

    mtx_lock(&a_stack->lock);
    retval = nxoe_p_stack_get(a_stack);
    mtx_unlock(&a_stack->lock);

    return retval;
}
#endif

#ifdef CW_THREADS
cw_nxo_t *
nxoe_p_stack_bget_locking(cw_nxoe_stack_t *a_stack)
{
    cw_nxo_t *retval;

    mtx_lock(&a_stack->lock);
    retval = nxoe_p_stack_bget(a_stack);
    mtx_unlock(&a_stack->lock);

    return retval;
}
#endif

#ifdef CW_THREADS
cw_nxo_t *
nxoe_p_stack_nget_locking(cw_nxoe_stack_t *a_stack, uint32_t a_index)
{
    cw_nxo_t *retval;
    
    mtx_lock(&a_stack->lock);
    retval = nxoe_p_stack_nget(a_stack, a_index);
    mtx_unlock(&a_stack->lock);

    return retval;
}
#endif

#ifdef CW_THREADS
cw_nxo_t *
nxoe_p_stack_nbget_locking(cw_nxoe_stack_t *a_stack, uint32_t a_index)
{
    cw_nxo_t *retval;
    
    mtx_lock(&a_stack->lock);
    retval = nxoe_p_stack_nbget(a_stack, a_index);
    mtx_unlock(&a_stack->lock);

    return retval;
}
#endif

#ifdef CW_THREADS
bool
nxoe_p_stack_exch_locking(cw_nxoe_stack_t *a_stack)
{
    bool retval;
    
    mtx_lock(&a_stack->lock);
    retval = nxoe_p_stack_exch(a_stack);
    mtx_unlock(&a_stack->lock);

    return retval;
}
#endif

#ifdef CW_THREADS
void
nxoe_p_stack_rot_locking(cw_nxoe_stack_t *a_stack, int32_t a_amount)
{
    mtx_lock(&a_stack->lock);
    nxoe_p_stack_rot(a_stack, a_amount);
    mtx_unlock(&a_stack->lock);
}
#endif

#ifdef CW_THREADS
bool
nxoe_p_stack_roll_locking(cw_nxoe_stack_t *a_stack, uint32_t a_count,
			  int32_t a_amount)
{
    bool retval;

    mtx_lock(&a_stack->lock);
    retval = nxoe_p_stack_roll(a_stack, a_count, a_amount);
    mtx_unlock(&a_stack->lock);

    return retval;
}
#endif
