/*******************************************************************
** v m . c
** Forth Inspired Command Language - virtual machine methods
** Author: John Sadler (john_sadler@alum.mit.edu)
** Created: 19 July 1997
** 
*******************************************************************/
/*
** This file implements the virtual machine of FICL. Each virtual
** machine retains the state of an interpreter. A virtual machine
** owns a pair of stacks for parameters and return addresses, as
** well as a pile of state variables and the two dedicated registers
** of the interp.
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include "libficl/libficl.h"

static char digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";


/**************************************************************************
                        v m B r a n c h R e l a t i v e 
** 
**************************************************************************/
void vmBranchRelative(FICL_VM *pVM, int offset)
{
    pVM->ip += offset;
    return;
}


/**************************************************************************
                        v m C r e a t e
** 
**************************************************************************/
FICL_VM *vmCreate(FICL_VM *pVM, unsigned nPStack, unsigned nRStack)
{
    if (pVM == NULL)
    {
        pVM = (FICL_VM *)ficlMalloc(sizeof (FICL_VM));
        assert (pVM);
        memset(pVM, 0, sizeof (FICL_VM));
    }

    if (pVM->pStack)
        stackDelete(pVM->pStack);
    pVM->pStack = stackCreate(nPStack);

    if (pVM->rStack)
        stackDelete(pVM->rStack);
    pVM->rStack = stackCreate(nRStack);

    pVM->textOut = ficlTextOut;

    vmReset(pVM);
    return pVM;
}


/**************************************************************************
                        v m D e l e t e
** 
**************************************************************************/
void vmDelete (FICL_VM *pVM)
{
    if (pVM)
    {
        ficlFree(pVM->pStack);
        ficlFree(pVM->rStack);
        ficlFree(pVM);
    }

    return;
}


/**************************************************************************
                        v m E x e c u t e
** Sets up the specified word to be run by the inner interpreter.
** Executes the word's code part immediately, but in the case of
** colon definition, the definition itself needs the inner interp
** to complete. This does not happen until control reaches ficlExec
**************************************************************************/
void vmExecute(FICL_VM *pVM, FICL_WORD *pWord)
{
    pVM->runningWord = pWord;
    pWord->code(pVM);
    return;
}


/**************************************************************************
                        v m I n n e r L o o p
** the mysterious inner interpreter...
** This loop is the address interpreter that makes colon definitions
** work. Upon entry, it assumes that the IP points to an entry in 
** a definition (the body of a colon word). It runs one word at a time
** until something does vmThrow. The catcher for this is expected to exist
** in the calling code.
** vmThrow gets you out of this loop with a longjmp()
** Visual C++ 5 chokes on this loop in Release mode. Aargh.
**************************************************************************/
#if INLINE_INNER_LOOP == 0
void vmInnerLoop(FICL_VM *pVM)
{
    M_INNER_LOOP(pVM);
}
#endif

/**************************************************************************
                        v m G e t S t r i n g
** Parses a string out of the VM input buffer and copies up to the first
** FICL_STRING_MAX characters to the supplied destination buffer, a
** FICL_STRING. The destination string is NULL terminated.
** 
** Returns the address of the first unused character in the dest buffer.
**************************************************************************/
char *vmGetString(FICL_VM *pVM, FICL_STRING *spDest, char delimiter)
{
    STRINGINFO si = vmParseString(pVM, delimiter);

    if (SI_COUNT(si) > FICL_STRING_MAX)
    {
        SI_SETLEN(si, FICL_STRING_MAX);
    }

    strncpy(spDest->text, SI_PTR(si), SI_COUNT(si));
    spDest->text[SI_COUNT(si)] = '\0';
    spDest->count = (FICL_COUNT)SI_COUNT(si);

    return spDest->text + SI_COUNT(si) + 1;
}


/**************************************************************************
                        v m G e t W o r d
** vmGetWord calls vmGetWord0 repeatedly until it gets a string with 
** non-zero length.
**************************************************************************/
STRINGINFO vmGetWord(FICL_VM *pVM)
{
    STRINGINFO si = vmGetWord0(pVM);

    if (SI_COUNT(si) == 0)
    {
        vmThrow(pVM, VM_RESTART);
    }

    return si;
}


/**************************************************************************
                        v m G e t W o r d 0
** Skip leading whitespace and parse a space delimited word from the tib.
** Returns the start address and length of the word. Updates the tib
** to reflect characters consumed, including the trailing delimiter.
** If there's nothing of interest in the tib, returns zero. This function
** does not use vmParseString because it uses isspace() rather than a
** single  delimiter character.
**************************************************************************/
STRINGINFO vmGetWord0(FICL_VM *pVM)
{
    char *pSrc      = vmGetInBuf(pVM);
    char *pEnd      = vmGetInBufEnd(pVM);
    STRINGINFO si;
    UNS32 count = 0;
    char ch;

    pSrc = skipSpace(pSrc, pEnd);
    SI_SETPTR(si, pSrc);

    for (ch = *pSrc; (pEnd != pSrc) && !isspace(ch); ch = *++pSrc)
    {
        count++;
    }

    SI_SETLEN(si, count);

    if ((pEnd != pSrc) && isspace(ch))    /* skip one trailing delimiter */
        pSrc++;

    vmUpdateTib(pVM, pSrc);

    return si;
}


/**************************************************************************
                        v m G e t W o r d T o P a d
** Does vmGetWord0 and copies the result to the pad as a NULL terminated
** string. Returns the length of the string. If the string is too long 
** to fit in the pad, it is truncated.
**************************************************************************/
int vmGetWordToPad(FICL_VM *pVM)
{
    STRINGINFO si;
    char *cp = (char *)pVM->pad;
    si = vmGetWord0(pVM);

    if (SI_COUNT(si) > nPAD)
        SI_SETLEN(si, nPAD);

    strncpy(cp, SI_PTR(si), SI_COUNT(si));
    cp[SI_COUNT(si)] = '\0';
    return (int)(SI_COUNT(si));
}


/**************************************************************************
                        v m P a r s e S t r i n g
** Parses a string out of the input buffer using the delimiter
** specified. Skips leading delimiters, marks the start of the string,
** and counts characters to the next delimiter it encounters. It then 
** updates the vm input buffer to consume all these chars, including the
** trailing delimiter. 
** Returns the address and length of the parsed string, not including the
** trailing delimiter.
**************************************************************************/
STRINGINFO vmParseString(FICL_VM *pVM, char delim)
{
    STRINGINFO si;
    char *pSrc      = vmGetInBuf(pVM);
    char *pEnd      = vmGetInBufEnd(pVM);
    char ch;

    while ((pSrc != pEnd) && (*pSrc == delim))  /* skip lead delimiters */
        pSrc++;

    SI_SETPTR(si, pSrc);    /* mark start of text */

    for (ch = *pSrc; (pSrc != pEnd)
                  && (ch != delim)
                  && (ch != '\r') 
                  && (ch != '\n'); ch = *++pSrc)
    {
        ;                   /* find next delimiter or end of line */
    }

                            /* set length of result */
    SI_SETLEN(si, pSrc - SI_PTR(si));

    if ((pSrc != pEnd) && (*pSrc == delim))     /* gobble trailing delimiter */
        pSrc++;

    vmUpdateTib(pVM, pSrc);
    return si;
}


/**************************************************************************
                        v m P o p I P
** 
**************************************************************************/
void vmPopIP(FICL_VM *pVM)
{
    pVM->ip = (IPTYPE)(stackPopPtr(pVM->rStack));
    return;
}


/**************************************************************************
                        v m P u s h I P
** 
**************************************************************************/
void vmPushIP(FICL_VM *pVM, IPTYPE newIP)
{
    stackPushPtr(pVM->rStack, (void *)pVM->ip);
    pVM->ip = newIP;
    return;
}


/**************************************************************************
                        v m P u s h T i b
** Binds the specified input string to the VM and clears >IN (the index)
**************************************************************************/
void vmPushTib(FICL_VM *pVM, char *text, FICL_INT nChars, TIB *pSaveTib)
{
    if (pSaveTib)
    {
        *pSaveTib = pVM->tib;
    }

    pVM->tib.cp = text;
    pVM->tib.end = text + nChars;
    pVM->tib.index = 0;
}


void vmPopTib(FICL_VM *pVM, TIB *pTib)
{
    if (pTib)
    {
        pVM->tib = *pTib;
    }
    return;
}


/**************************************************************************
                        v m Q u i t
** 
**************************************************************************/
void vmQuit(FICL_VM *pVM)
{
    static FICL_WORD *pInterp = NULL;
    if (!pInterp)
        pInterp = ficlLookup("interpret");
    assert(pInterp);

    stackReset(pVM->rStack);
    pVM->fRestart    = 0;
    pVM->ip          = &pInterp;
    pVM->runningWord = pInterp;
    pVM->state       = INTERPRET;
    pVM->tib.cp      = NULL;
    pVM->tib.end     = NULL;
    pVM->tib.index   = 0;
    pVM->pad[0]      = '\0';
    pVM->sourceID.i  = 0;
    return;
}


/**************************************************************************
                        v m R e s e t 
** 
**************************************************************************/
void vmReset(FICL_VM *pVM)
{
    vmQuit(pVM);
    stackReset(pVM->pStack);
    pVM->base        = 10;
    return;
}


/**************************************************************************
                        v m S e t T e x t O u t
** Binds the specified output callback to the vm. If you pass NULL,
** binds the default output function (ficlTextOut)
**************************************************************************/
void vmSetTextOut(FICL_VM *pVM, OUTFUNC textOut)
{
    if (textOut)
        pVM->textOut = textOut;
    else
        pVM->textOut = ficlTextOut;

    return;
}


/**************************************************************************
                        v m T e x t O u t
** Feeds text to the vm's output callback
**************************************************************************/
void vmTextOut(FICL_VM *pVM, char *text, int fNewline)
{
    assert(pVM);
    assert(pVM->textOut);
    (pVM->textOut)(pVM, text, fNewline);

    return;
}


/**************************************************************************
                        v m T h r o w
** 
**************************************************************************/
void vmThrow(FICL_VM *pVM, int except)
{
    if (pVM->pState)
        longjmp(*(pVM->pState), except);
}


void vmThrowErr(FICL_VM *pVM, char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    vsprintf(pVM->pad, fmt, va);
    vmTextOut(pVM, pVM->pad, 1);
    va_end(va);
    longjmp(*(pVM->pState), VM_ERREXIT);
}


/**************************************************************************
                        w o r d I s I m m e d i a t e
** 
**************************************************************************/
int wordIsImmediate(FICL_WORD *pFW)
{
    return ((pFW != NULL) && (pFW->flags & FW_IMMEDIATE));
}


/**************************************************************************
                        w o r d I s C o m p i l e O n l y
** 
**************************************************************************/
int wordIsCompileOnly(FICL_WORD *pFW)
{
    return ((pFW != NULL) && (pFW->flags & FW_COMPILE));
}


/**************************************************************************
                        s t r r e v
** 
**************************************************************************/
char *strrev( char *string )    
{                               /* reverse a string in-place */
    int i = strlen(string);
    char *p1 = string;          /* first char of string */
    char *p2 = string + i - 1;  /* last non-NULL char of string */
    char c;

    if (i > 1)
    {
        while (p1 < p2)
        {
            c = *p2;
            *p2 = *p1;
            *p1 = c;
            p1++; p2--;
        }
    }
        
    return string;
}


/**************************************************************************
                        d i g i t _ t o _ c h a r
** 
**************************************************************************/
char digit_to_char(int value)
{
    return digits[value];
}


/**************************************************************************
                        i s P o w e r O f T w o
** Tests whether supplied argument is an integer power of 2 (2**n)
** where 32 > n > 1, and returns n if so. Otherwise returns zero.
**************************************************************************/
int isPowerOfTwo(FICL_UNS u)
{
    int i = 1;
    FICL_UNS t = 2;

    for (; ((t <= u) && (t != 0)); i++, t <<= 1)
    {
        if (u == t)
            return i;
    }

    return 0;
}


/**************************************************************************
                        l t o a
** 
**************************************************************************/
char *ltoa( FICL_INT value, char *string, int radix )
{                               /* convert long to string, any base */
    char *cp = string;
    int sign = ((radix == 10) && (value < 0));
    int pwr;

    assert(radix > 1);
    assert(radix < 37);
    assert(string);

    pwr = isPowerOfTwo((FICL_UNS)radix);

    if (sign)
        value = -value;

    if (value == 0)
        *cp++ = '0';
    else if (pwr != 0)
    {
        FICL_UNS v = (FICL_UNS) value;
        FICL_UNS mask = (FICL_UNS) ~(-1 << pwr);
        while (v)
        {
            *cp++ = digits[v & mask];
            v >>= pwr;
        }
    }
    else
    {
        UNSQR result;
        DPUNS v;
        v.hi = 0;
        v.lo = (FICL_UNS)value;
        while (v.lo)
        {
            result = ficlLongDiv(v, (FICL_UNS)radix);
            *cp++ = digits[result.rem];
            v.lo = result.quot;
        }
    }

    if (sign)
        *cp++ = '-';

    *cp++ = '\0';

    return strrev(string);
}


/**************************************************************************
                        u l t o a
** 
**************************************************************************/
char *ultoa(FICL_UNS value, char *string, int radix )
{                               /* convert long to string, any base */
    char *cp = string;
    DPUNS ud;
    UNSQR result;

    assert(radix > 1);
    assert(radix < 37);
    assert(string);

    if (value == 0)
        *cp++ = '0';
    else
    {
        ud.hi = 0;
        ud.lo = value;
        result.quot = value;

        while (ud.lo)
        {
            result = ficlLongDiv(ud, (UNS32)radix);
            ud.lo = result.quot;
            *cp++ = digits[result.rem];
        }
    }

    *cp++ = '\0';

    return strrev(string);
}


/**************************************************************************
                        c a s e F o l d
** Case folds a NULL terminated string in place. All characters
** get converted to lower case.
**************************************************************************/
char *caseFold(char *cp)
{
    char *oldCp = cp;

    while (*cp)
    {
        if (isupper(*cp))
            *cp = (char)tolower(*cp);
        cp++;
    }

    return oldCp;
}


/**************************************************************************
                        s t r i n c m p
** 
**************************************************************************/
int strincmp(char *cp1, char *cp2, FICL_COUNT count)
{
    int i = 0;
    char c1, c2;

    for (c1 = *cp1, c2 = *cp2;
        ((i == 0) && count && c1 && c2);
        c1 = *++cp1, c2 = *++cp2, count--)
    {
        i = tolower(c1) - tolower(c2);
    }

    return i;
}



/**************************************************************************
                        s k i p S p a c e
** Given a string pointer, returns a pointer to the first non-space
** char of the string, or to the NULL terminator if no such char found.
** If the pointer reaches "end" first, stop there. Pass NULL to 
** suppress this behavior.
**************************************************************************/
char *skipSpace(char *cp, char *end)
{
    assert(cp);

    while ((cp != end) && isspace(*cp))
        cp++;

    return cp;
}


