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
 * Memory barrier implementation.
 *
 ******************************************************************************/

#ifndef CW_THREADS
/* No memory barriers are necessary for single-threaded code. */
#define mb_write()
#else

#ifndef CW_USE_INLINES
CW_INLINE void
mb_write(void);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_MB_C_))

#if (defined(CW_CPU_I586) || defined(CW_CPU_I686))
/* According to the Intel Architecture Software Developer's Manual, current
 * processors execute instructions in order from the perspective of other
 * processors in a multiprocessor system, but 1) Intel reserves the right to
 * change that, and 2) the compiler's optimizer could re-order instructions if
 * there weren't some form of barrier.  Therefore, even if running on an
 * architecture that does not need memory barriers (everything through i686), an
 * "optimizer barrier" is necessary. */
CW_INLINE void
mb_write(void)
{
    asm volatile ("push %%eax;"
		  "push %%ebx;"
		  "push %%ecx;"
		  "push %%edx;"
		  "xor  %%eax,%%eax;"
		  "cpuid;"
		  "pop  %%edx;"
		  "pop  %%ecx;"
		  "pop  %%ebx;"
		  "pop  %%eax;"
		  : /* Outputs. */
		  : /* Inputs. */
		  : "memory" /* Clobbers. */
		  );
}
#elif (defined(CW_CPU_I386) || defined(CW_CPU_I486))
CW_INLINE void
mb_write(void)
{
    /* The compiler cannot optimize this away. */
    static volatile void *p = NULL;
}
#elif (defined(CW_CPU_POWERPC))
CW_INLINE void
mb_write(void)
{
    asm volatile ("eieio"
		  : /* Outputs. */
		  : /* Inputs. */
		  : "memory" /* Clobbers. */
		  );
}
#else
/* This is much slower than a simple memory barrier, but the semantics of
 * mutex unlock make this work. */
CW_INLINE void
mb_write(void)
{
    cw_mtx_t mtx;

    mtx_new(&mtx);
    mtx_lock(&mtx);
    mtx_unlock(&mtx);
    mtx_delete(&mtx);
}
#endif
#endif

#endif /* (defined(CW_USE_INLINES) || defined(CW_MB_C_)) */
