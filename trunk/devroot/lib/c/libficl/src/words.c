/*******************************************************************
** w o r d s . c
** Forth Inspired Command Language
** ANS Forth CORE word-set written in C
** Author: John Sadler (john_sadler@alum.mit.edu)
** Created: 19 July 1997
** 
*******************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "libficl/libficl.h"
#include "libficl/math64.h"

static void colonParen(FICL_VM *pVM);
static void literalIm(FICL_VM *pVM);
static void interpWord(FICL_VM *pVM, STRINGINFO si);

/*
** Control structure building words use these
** strings' addresses as markers on the stack to 
** check for structure completion.
*/
static char doTag[]    = "do";
static char colonTag[] = "colon";
static char leaveTag[] = "leave";

static char destTag[]  = "target";
static char origTag[]  = "origin";

/*
** Pointers to various words in the dictionary
** -- initialized by ficlCompileCore, below --
** for use by compiling words. Colon definitions
** in ficl are lists of pointers to words. A bit
** simple-minded...
*/
static FICL_WORD *pBranchParen  = NULL;
static FICL_WORD *pComma        = NULL;
static FICL_WORD *pDoParen      = NULL;
static FICL_WORD *pDoesParen    = NULL;
static FICL_WORD *pExitParen    = NULL;
static FICL_WORD *pIfParen      = NULL;
static FICL_WORD *pInterpret    = NULL;
static FICL_WORD *pLitParen     = NULL;
static FICL_WORD *pTwoLitParen  = NULL;
static FICL_WORD *pLoopParen    = NULL;
static FICL_WORD *pPLoopParen   = NULL;
static FICL_WORD *pQDoParen     = NULL;
static FICL_WORD *pSemiParen    = NULL;
static FICL_WORD *pStore        = NULL;
static FICL_WORD *pStringLit    = NULL;
static FICL_WORD *pType         = NULL;

#if FICL_WANT_LOCALS
static FICL_WORD *pGetLocalParen= NULL;
static FICL_WORD *pGet2LocalParen= NULL;
static FICL_WORD *pGetLocal0    = NULL;
static FICL_WORD *pGetLocal1    = NULL;
static FICL_WORD *pToLocalParen = NULL;
static FICL_WORD *pTo2LocalParen = NULL;
static FICL_WORD *pToLocal0     = NULL;
static FICL_WORD *pToLocal1     = NULL;
static FICL_WORD *pLinkParen    = NULL;
static FICL_WORD *pUnLinkParen  = NULL;
static int nLocals = 0;
static CELL *pMarkLocals = NULL;

static void doLocalIm(FICL_VM *pVM);
static void do2LocalIm(FICL_VM *pVM);

#endif


/*
** C O N T R O L   S T R U C T U R E   B U I L D E R S
**
** Push current dict location for later branch resolution.
** The location may be either a branch target or a patch address...
*/
static void markBranch(FICL_DICT *dp, FICL_VM *pVM, char *tag)
{
    stackPushPtr(pVM->pStack, dp->here);
    stackPushPtr(pVM->pStack, tag);
    return;
}

static void markControlTag(FICL_VM *pVM, char *tag)
{
    stackPushPtr(pVM->pStack, tag);
    return;
}

static void matchControlTag(FICL_VM *pVM, char *tag)
{
    char *cp = (char *)stackPopPtr(pVM->pStack);
    if ( strcmp(cp, tag) )
    {
        vmThrowErr(pVM, "Error -- unmatched control structure \"%s\"", tag);
    }

    return;
}

/*
** Expect a branch target address on the param stack,
** compile a literal offset from the current dict location
** to the target address
*/
static void resolveBackBranch(FICL_DICT *dp, FICL_VM *pVM, char *tag)
{
    long offset;
    CELL *patchAddr;

    matchControlTag(pVM, tag);

    patchAddr = (CELL *)stackPopPtr(pVM->pStack);
    offset = patchAddr - dp->here;
    dictAppendCell(dp, LVALUEtoCELL(offset));

    return;
}


/*
** Expect a branch patch address on the param stack,
** compile a literal offset from the patch location
** to the current dict location
*/
static void resolveForwardBranch(FICL_DICT *dp, FICL_VM *pVM, char *tag)
{
    long offset;
    CELL *patchAddr;

    matchControlTag(pVM, tag);

    patchAddr = (CELL *)stackPopPtr(pVM->pStack);
    offset = dp->here - patchAddr;
    *patchAddr = LVALUEtoCELL(offset);

    return;
}

/*
** Match the tag to the top of the stack. If success,
** sopy "here" address into the cell whose address is next
** on the stack. Used by do..leave..loop.
*/
static void resolveAbsBranch(FICL_DICT *dp, FICL_VM *pVM, char *tag)
{
    CELL *patchAddr;
    char *cp;

    cp = stackPopPtr(pVM->pStack);
    if (strcmp(cp, tag))
    {
        vmTextOut(pVM, "Warning -- Unmatched control word: ", 0);
        vmTextOut(pVM, tag, 1);
    }

    patchAddr = (CELL *)stackPopPtr(pVM->pStack);
    *patchAddr = LVALUEtoCELL(dp->here);

    return;
}


/**************************************************************************
                        i s N u m b e r
** Attempts to convert the NULL terminated string in the VM's pad to 
** a number using the VM's current base. If successful, pushes the number
** onto the param stack and returns TRUE. Otherwise, returns FALSE.
**************************************************************************/

static int isNumber(FICL_VM *pVM, STRINGINFO si)
{
    FICL_INT accum     = 0;
    char isNeg      = FALSE;
    unsigned base   = pVM->base;
    char *cp        = SI_PTR(si);
    FICL_COUNT count= (FICL_COUNT)SI_COUNT(si);
    unsigned ch;
    unsigned digit;

    if (*cp == '-')
    {
        cp++;
        count--;
        isNeg = TRUE;
    }
    else if ((cp[0] == '0') && (cp[1] == 'x'))
    {               /* detect 0xNNNN format for hex numbers */
        cp += 2;
        count -= 2;
        base = 16;
    }

    if (count == 0)
        return FALSE;

    while (count-- && ((ch = *cp++) != '\0'))
    {
        if (!isalnum(ch))
            return FALSE;

        digit = ch - '0';

        if (digit > 9)
            digit = tolower(ch) - 'a' + 10;

        if (digit >= base)
            return FALSE;

        accum = accum * base + digit;
    }

    if (isNeg)
        accum = -accum;

    stackPushINT(pVM->pStack, accum);

    return TRUE;
}


static void ficlIsNum(FICL_VM *pVM)
{
	STRINGINFO si;
	FICL_INT ret;

	SI_SETLEN(si, stackPopINT(pVM->pStack));
	SI_SETPTR(si, stackPopPtr(pVM->pStack));
	ret = isNumber(pVM, si) ? FICL_TRUE : FICL_FALSE;
	stackPushINT(pVM->pStack, ret);
	return;
}

/**************************************************************************
                        a d d   &   f r i e n d s
** 
**************************************************************************/

static void add(FICL_VM *pVM)
{
    FICL_INT i;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 2, 1);
#endif
    i = stackPopINT(pVM->pStack);
    i += stackGetTop(pVM->pStack).i;
    stackSetTop(pVM->pStack, LVALUEtoCELL(i));
    return;
}

static void sub(FICL_VM *pVM)
{
    FICL_INT i;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 2, 1);
#endif
    i = stackPopINT(pVM->pStack);
    i = stackGetTop(pVM->pStack).i - i;
    stackSetTop(pVM->pStack, LVALUEtoCELL(i));
    return;
}

static void mul(FICL_VM *pVM)
{
    FICL_INT i;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 2, 1);
#endif
    i = stackPopINT(pVM->pStack);
    i *= stackGetTop(pVM->pStack).i;
    stackSetTop(pVM->pStack, LVALUEtoCELL(i));
    return;
}

static void negate(FICL_VM *pVM)
{
    FICL_INT i;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 1, 1);
#endif
    i = -stackPopINT(pVM->pStack);
    stackPushINT(pVM->pStack, i);
    return;
}

static void ficlDiv(FICL_VM *pVM)
{
    FICL_INT i;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 2, 1);
#endif
    i = stackPopINT(pVM->pStack);
    i = stackGetTop(pVM->pStack).i / i;
    stackSetTop(pVM->pStack, LVALUEtoCELL(i));
    return;
}

/*
** slash-mod        CORE ( n1 n2 -- n3 n4 )
** Divide n1 by n2, giving the single-cell remainder n3 and the single-cell
** quotient n4. An ambiguous condition exists if n2 is zero. If n1 and n2
** differ in sign, the implementation-defined result returned will be the
** same as that returned by either the phrase
** >R S>D R> FM/MOD or the phrase >R S>D R> SM/REM . 
** NOTE: Ficl complies with the second phrase (symmetric division)
*/
static void slashMod(FICL_VM *pVM)
{
    DPINT n1;
    FICL_INT n2;
    INTQR qr;

#if FICL_ROBUST > 1
    vmCheckStack(pVM, 2, 2);
#endif
    n2    = stackPopINT(pVM->pStack);
    n1.lo = stackPopINT(pVM->pStack);
    i64Extend(n1);

    qr = m64SymmetricDivI(n1, n2);
    stackPushINT(pVM->pStack, qr.rem);
    stackPushINT(pVM->pStack, qr.quot);
    return;
}

static void onePlus(FICL_VM *pVM)
{
    FICL_INT i;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 1, 1);
#endif
    i = stackGetTop(pVM->pStack).i;
    i += 1;
    stackSetTop(pVM->pStack, LVALUEtoCELL(i));
    return;
}

static void oneMinus(FICL_VM *pVM)
{
    FICL_INT i;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 1, 1);
#endif
    i = stackGetTop(pVM->pStack).i;
    i -= 1;
    stackSetTop(pVM->pStack, LVALUEtoCELL(i));
    return;
}

static void twoMul(FICL_VM *pVM)
{
    FICL_INT i;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 1, 1);
#endif
    i = stackGetTop(pVM->pStack).i;
    i *= 2;
    stackSetTop(pVM->pStack, LVALUEtoCELL(i));
    return;
}

static void twoDiv(FICL_VM *pVM)
{
    FICL_INT i;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 1, 1);
#endif
    i = stackGetTop(pVM->pStack).i;
    i >>= 1;
    stackSetTop(pVM->pStack, LVALUEtoCELL(i));
    return;
}

static void mulDiv(FICL_VM *pVM)
{
    FICL_INT x, y, z;
    DPINT prod;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 3, 1);
#endif
    z = stackPopINT(pVM->pStack);
    y = stackPopINT(pVM->pStack);
    x = stackPopINT(pVM->pStack);

    prod = m64MulI(x,y);
    x    = m64SymmetricDivI(prod, z).quot;

    stackPushINT(pVM->pStack, x);
    return;
}


static void mulDivRem(FICL_VM *pVM)
{
    FICL_INT x, y, z;
    DPINT prod;
    INTQR qr;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 3, 2);
#endif
    z = stackPopINT(pVM->pStack);
    y = stackPopINT(pVM->pStack);
    x = stackPopINT(pVM->pStack);

    prod = m64MulI(x,y);
    qr   = m64SymmetricDivI(prod, z);

    stackPushINT(pVM->pStack, qr.rem);
    stackPushINT(pVM->pStack, qr.quot);
    return;
}


/**************************************************************************
                        b y e
** TOOLS
** Signal the system to shut down - this causes ficlExec to return
** VM_USEREXIT. The rest is up to you.
**************************************************************************/

static void bye(FICL_VM *pVM)
{
    vmThrow(pVM, VM_USEREXIT);
    return;
}


/**************************************************************************
                        c o l o n   d e f i n i t i o n s
** Code to begin compiling a colon definition
** This function sets the state to COMPILE, then creates a
** new word whose name is the next word in the input stream
** and whose code is colonParen.
**************************************************************************/

static void colon(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();
    STRINGINFO si = vmGetWord(pVM);

    pVM->state = COMPILE;
    markControlTag(pVM, colonTag);
    dictAppendWord2(dp, si, colonParen, FW_DEFAULT | FW_SMUDGE);
#if FICL_WANT_LOCALS
    nLocals = 0;
#endif
    return;
}


/**************************************************************************
                        c o l o n P a r e n
** This is the code that executes a colon definition. It assumes that the
** virtual machine is running a "next" loop (See the vm.c
** for its implementation of member function vmExecute()). The colon
** code simply copies the address of the first word in the list of words
** to interpret into IP after saving its old value. When we return to the
** "next" loop, the virtual machine will call the code for each word in 
** turn.
**
**************************************************************************/
       
static void colonParen(FICL_VM *pVM)
{
    IPTYPE tempIP = (IPTYPE) (pVM->runningWord->param);
    vmPushIP(pVM, tempIP);

    return;
}


/**************************************************************************
                        s e m i c o l o n C o I m
** 
** IMMEDIATE code for ";". This function sets the state to INTERPRET and
** terminates a word under compilation by appending code for "(;)" to
** the definition. TO DO: checks for leftover branch target tags on the
** return stack and complains if any are found.
**************************************************************************/
static void semiParen(FICL_VM *pVM)
{
    vmPopIP(pVM);
    return;
}


static void semicolonCoIm(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();

    assert(pSemiParen);
    matchControlTag(pVM, colonTag);

#if FICL_WANT_LOCALS
    assert(pUnLinkParen);
    if (nLocals > 0)
    {
        FICL_DICT *pLoc = ficlGetLoc();
        dictEmpty(pLoc, pLoc->pForthWords->size);
        dictAppendCell(dp, LVALUEtoCELL(pUnLinkParen));
    }
    nLocals = 0;
#endif

    dictAppendCell(dp, LVALUEtoCELL(pSemiParen));
    pVM->state = INTERPRET;
    dictUnsmudge(dp);
    return;
}


/**************************************************************************
                        e x i t
** CORE
** This function simply pops the previous instruction
** pointer and returns to the "next" loop. Used for exiting from within
** a definition. Note that exitParen is identical to semiParen - they
** are in two different functions so that "see" can correctly identify
** the end of a colon definition, even if it uses "exit".
**************************************************************************/
static void exitParen(FICL_VM *pVM)
{
    vmPopIP(pVM);
    return;
}

static void exitCoIm(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();
    assert(pExitParen);
    IGNORE(pVM);

#if FICL_WANT_LOCALS
    if (nLocals > 0)
    {
        dictAppendCell(dp, LVALUEtoCELL(pUnLinkParen));
    }
#endif
    dictAppendCell(dp, LVALUEtoCELL(pExitParen));
    return;
}


/**************************************************************************
                        c o n s t a n t P a r e n
** This is the run-time code for "constant". It simply returns the 
** contents of its word's first data cell.
**
**************************************************************************/

void constantParen(FICL_VM *pVM)
{
    FICL_WORD *pFW = pVM->runningWord;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 0, 1);
#endif
    stackPush(pVM->pStack, pFW->param[0]);
    return;
}

void twoConstParen(FICL_VM *pVM)
{
    FICL_WORD *pFW = pVM->runningWord;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 0, 2);
#endif
    stackPush(pVM->pStack, pFW->param[0]); /* lo */
    stackPush(pVM->pStack, pFW->param[1]); /* hi */
    return;
}


/**************************************************************************
                        c o n s t a n t
** IMMEDIATE
** Compiles a constant into the dictionary. Constants return their
** value when invoked. Expects a value on top of the parm stack.
**************************************************************************/

static void constant(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();
    STRINGINFO si = vmGetWord(pVM);

#if FICL_ROBUST > 1
    vmCheckStack(pVM, 1, 0);
#endif
    dictAppendWord2(dp, si, constantParen, FW_DEFAULT);
    dictAppendCell(dp, stackPop(pVM->pStack));
    return;
}


static void twoConstant(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();
    STRINGINFO si = vmGetWord(pVM);
    CELL c;
    
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 2, 0);
#endif
    c = stackPop(pVM->pStack);
    dictAppendWord2(dp, si, twoConstParen, FW_DEFAULT);
    dictAppendCell(dp, stackPop(pVM->pStack));
    dictAppendCell(dp, c);
    return;
}


/**************************************************************************
                        d i s p l a y C e l l
** Drop and print the contents of the cell at the top of the param
** stack
**************************************************************************/

static void displayCell(FICL_VM *pVM)
{
    CELL c;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 1, 0);
#endif
    c = stackPop(pVM->pStack);
    ltoa((c).i, pVM->pad, pVM->base);
    strcat(pVM->pad, " ");
    vmTextOut(pVM, pVM->pad, 0);
    return;
}

static void uDot(FICL_VM *pVM)
{
    FICL_UNS u;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 1, 0);
#endif
    u = stackPopUNS(pVM->pStack);
    ultoa(u, pVM->pad, pVM->base);
    strcat(pVM->pad, " ");
    vmTextOut(pVM, pVM->pad, 0);
    return;
}


static void hexDot(FICL_VM *pVM)
{
    FICL_UNS u;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 1, 0);
#endif
    u = stackPopUNS(pVM->pStack);
    ultoa(u, pVM->pad, 16);
    strcat(pVM->pad, " ");
    vmTextOut(pVM, pVM->pad, 0);
    return;
}


/**************************************************************************
                        d i s p l a y S t a c k
** Display the parameter stack (code for ".s")
**************************************************************************/

static void displayStack(FICL_VM *pVM)
{
    int d = stackDepth(pVM->pStack);
    int i;
    CELL *pCell;

    vmCheckStack(pVM, 0, 0);

    if (d == 0)
        vmTextOut(pVM, "(Stack Empty)", 1);
    else
    {
        pCell = pVM->pStack->sp;
        for (i = 0; i < d; i++)
        {
            vmTextOut(pVM, ltoa((*--pCell).i, pVM->pad, pVM->base), 1);
        }
    }
}


/**************************************************************************
                        d u p   &   f r i e n d s
** 
**************************************************************************/

static void depth(FICL_VM *pVM)
{
    int i;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 0, 1);
#endif
    i = stackDepth(pVM->pStack);
    stackPushINT(pVM->pStack, i);
    return;
}


static void drop(FICL_VM *pVM)
{
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 1, 0);
#endif
    stackDrop(pVM->pStack, 1);
    return;
}


static void twoDrop(FICL_VM *pVM)
{
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 2, 0);
#endif
    stackDrop(pVM->pStack, 2);
    return;
}


static void dup(FICL_VM *pVM)
{
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 1, 2);
#endif
    stackPick(pVM->pStack, 0);
    return;
}


static void twoDup(FICL_VM *pVM)
{
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 2, 4);
#endif
    stackPick(pVM->pStack, 1);
    stackPick(pVM->pStack, 1);
    return;
}


static void over(FICL_VM *pVM)
{
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 2, 3);
#endif
    stackPick(pVM->pStack, 1);
    return;
}

static void twoOver(FICL_VM *pVM)
{
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 4, 6);
#endif
    stackPick(pVM->pStack, 3);
    stackPick(pVM->pStack, 3);
    return;
}


static void pick(FICL_VM *pVM)
{
    CELL c = stackPop(pVM->pStack);
#if FICL_ROBUST > 1
    vmCheckStack(pVM, c.i+1, c.i+2);
#endif
    stackPick(pVM->pStack, c.i);
    return;
}


static void questionDup(FICL_VM *pVM)
{
    CELL c;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 1, 2);
#endif
    c = stackGetTop(pVM->pStack);

    if (c.i != 0)
        stackPick(pVM->pStack, 0);

    return;
}


static void roll(FICL_VM *pVM)
{
    int i = stackPop(pVM->pStack).i;
    i = (i > 0) ? i : 0;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, i+1, i+1);
#endif
    stackRoll(pVM->pStack, i);
    return;
}


static void minusRoll(FICL_VM *pVM)
{
    int i = stackPop(pVM->pStack).i;
    i = (i > 0) ? i : 0;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, i+1, i+1);
#endif
    stackRoll(pVM->pStack, -i);
    return;
}


static void rot(FICL_VM *pVM)
{
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 3, 3);
#endif
    stackRoll(pVM->pStack, 2);
    return;
}


static void swap(FICL_VM *pVM)
{
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 2, 2);
#endif
    stackRoll(pVM->pStack, 1);
    return;
}


static void twoSwap(FICL_VM *pVM)
{
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 4, 4);
#endif
    stackRoll(pVM->pStack, 3);
    stackRoll(pVM->pStack, 3);
    return;
}


/**************************************************************************
                        e m i t   &   f r i e n d s
** 
**************************************************************************/

static void emit(FICL_VM *pVM)
{
    char *cp = pVM->pad;
    int i;

#if FICL_ROBUST > 1
    vmCheckStack(pVM, 1, 0);
#endif
    i = stackPopINT(pVM->pStack);
    cp[0] = (char)i;
    cp[1] = '\0';
    vmTextOut(pVM, cp, 0);
    return;
}


static void cr(FICL_VM *pVM)
{
    vmTextOut(pVM, "", 1);
    return;
}


static void commentLine(FICL_VM *pVM)
{
    char *cp        = vmGetInBuf(pVM);
    char *pEnd      = vmGetInBufEnd(pVM);
    char ch = *cp;

    while ((cp != pEnd) && (ch != '\r') && (ch != '\n'))
    {
        ch = *++cp;
    }

    /*
    ** Cope with DOS or UNIX-style EOLs -
    ** Check for /r, /n, /r/n, or /n/r end-of-line sequences,
    ** and point cp to next char. If EOL is \0, we're done.
    */
    if (cp != pEnd)
    {
        cp++;

        if ( (cp != pEnd) && (ch != *cp) 
             && ((*cp == '\r') || (*cp == '\n')) )
            cp++;
    }

    vmUpdateTib(pVM, cp);
    return;
}


/*
** paren CORE 
** Compilation: Perform the execution semantics given below.
** Execution: ( "ccc<paren>" -- )
** Parse ccc delimited by ) (right parenthesis). ( is an immediate word. 
** The number of characters in ccc may be zero to the number of characters
** in the parse area. 
** 
*/
static void commentHang(FICL_VM *pVM)
{
    vmParseStringEx(pVM, ')', 0);
    return;
}


/**************************************************************************
                        F E T C H   &   S T O R E
** 
**************************************************************************/

static void fetch(FICL_VM *pVM)
{
    CELL *pCell;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 1, 1);
#endif
    pCell = (CELL *)stackPopPtr(pVM->pStack);
    stackPush(pVM->pStack, *pCell);
    return;
}

/*
** two-fetch    CORE ( a-addr -- x1 x2 )
** Fetch the cell pair x1 x2 stored at a-addr. x2 is stored at a-addr and
** x1 at the next consecutive cell. It is equivalent to the sequence
** DUP CELL+ @ SWAP @ . 
*/
static void twoFetch(FICL_VM *pVM)
{
    CELL *pCell;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 1, 2);
#endif
    pCell = (CELL *)stackPopPtr(pVM->pStack);
    stackPush(pVM->pStack, *pCell++);
    stackPush(pVM->pStack, *pCell);
    swap(pVM);
    return;
}

/*
** store        CORE ( x a-addr -- )
** Store x at a-addr. 
*/
static void store(FICL_VM *pVM)
{
    CELL *pCell;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 2, 0);
#endif
    pCell = (CELL *)stackPopPtr(pVM->pStack);
    *pCell = stackPop(pVM->pStack);
}

/*
** two-store    CORE ( x1 x2 a-addr -- )
** Store the cell pair x1 x2 at a-addr, with x2 at a-addr and x1 at the
** next consecutive cell. It is equivalent to the sequence
** SWAP OVER ! CELL+ ! . 
*/
static void twoStore(FICL_VM *pVM)
{
    CELL *pCell;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 3, 0);
#endif
    pCell = (CELL *)stackPopPtr(pVM->pStack);
    *pCell++    = stackPop(pVM->pStack);
    *pCell      = stackPop(pVM->pStack);
}

static void plusStore(FICL_VM *pVM)
{
    CELL *pCell;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 2, 0);
#endif
    pCell = (CELL *)stackPopPtr(pVM->pStack);
    pCell->i += stackPop(pVM->pStack).i;
}


static void wFetch(FICL_VM *pVM)
{
    UNS16 *pw;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 1, 1);
#endif
    pw = (UNS16 *)stackPopPtr(pVM->pStack);
    stackPushUNS(pVM->pStack, (FICL_UNS)*pw);
    return;
}

static void wStore(FICL_VM *pVM)
{
    UNS16 *pw;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 2, 0);
#endif
    pw = (UNS16 *)stackPopPtr(pVM->pStack);
    *pw = (UNS16)(stackPop(pVM->pStack).u);
}

static void cFetch(FICL_VM *pVM)
{
    UNS8 *pc;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 1, 1);
#endif
    pc = (UNS8 *)stackPopPtr(pVM->pStack);
    stackPushUNS(pVM->pStack, (FICL_UNS)*pc);
    return;
}

static void cStore(FICL_VM *pVM)
{
    UNS8 *pc;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 2, 0);
#endif
    pc = (UNS8 *)stackPopPtr(pVM->pStack);
    *pc = (UNS8)(stackPop(pVM->pStack).u);
}


/**************************************************************************
                        i f C o I m
** IMMEDIATE
** Compiles code for a conditional branch into the dictionary
** and pushes the branch patch address on the stack for later
** patching by ELSE or THEN/ENDIF. 
**************************************************************************/

static void ifCoIm(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();

    assert(pIfParen);

    dictAppendCell(dp, LVALUEtoCELL(pIfParen));
    markBranch(dp, pVM, origTag);
    dictAppendUNS(dp, 1);
    return;
}


/**************************************************************************
                        i f P a r e n
** Runtime code to do "if" or "until": pop a flag from the stack,
** fall through if true, branch if false. Probably ought to be 
** called (not?branch) since it does "branch if false".
**************************************************************************/

static void ifParen(FICL_VM *pVM)
{
    FICL_UNS flag;
    
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 1, 0);
#endif
    flag = stackPopUNS(pVM->pStack);

    if (flag) 
    {                           /* fall through */
        vmBranchRelative(pVM, 1);
    }
    else 
    {                           /* take branch (to else/endif/begin) */
        vmBranchRelative(pVM, (int)(*pVM->ip));
    }

    return;
}


/**************************************************************************
                        e l s e C o I m
** 
** IMMEDIATE -- compiles an "else"...
** 1) Compile a branch and a patch address; the address gets patched
**    by "endif" to point past the "else" code.
** 2) Pop the the "if" patch address
** 3) Patch the "if" branch to point to the current compile address.
** 4) Push the "else" patch address. ("endif" patches this to jump past 
**    the "else" code.
**************************************************************************/

static void elseCoIm(FICL_VM *pVM)
{
    CELL *patchAddr;
    int offset;
    FICL_DICT *dp = ficlGetDict();

    assert(pBranchParen);
                                            /* (1) compile branch runtime */
    dictAppendCell(dp, LVALUEtoCELL(pBranchParen));
    matchControlTag(pVM, origTag);
    patchAddr = 
        (CELL *)stackPopPtr(pVM->pStack);   /* (2) pop "if" patch addr */
    markBranch(dp, pVM, origTag);           /* (4) push "else" patch addr */
    dictAppendUNS(dp, 1);                 /* (1) compile patch placeholder */
    offset = dp->here - patchAddr;
    *patchAddr = LVALUEtoCELL(offset);      /* (3) Patch "if" */

    return;
}


/**************************************************************************
                        b r a n c h P a r e n
** 
** Runtime for "(branch)" -- expects a literal offset in the next
** compilation address, and branches to that location.
**************************************************************************/

static void branchParen(FICL_VM *pVM)
{
    vmBranchRelative(pVM, *(int *)(pVM->ip));
    return;
}


/**************************************************************************
                        e n d i f C o I m
** 
**************************************************************************/

static void endifCoIm(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();
    resolveForwardBranch(dp, pVM, origTag);
    return;
}


/**************************************************************************
                        h a s h
** hash ( c-addr u -- code)
** calculates hashcode of specified string and leaves it on the stack
**************************************************************************/

static void hash(FICL_VM *pVM)
{
	STRINGINFO si;
	SI_SETLEN(si, stackPopUNS(pVM->pStack));
	SI_SETPTR(si, stackPopPtr(pVM->pStack));
	stackPushUNS(pVM->pStack, hashHashCode(si));
    return;
}


/**************************************************************************
                        i n t e r p r e t 
** This is the "user interface" of a Forth. It does the following:
**   while there are words in the VM's Text Input Buffer
**     Copy next word into the pad (vmGetWord)
**     Attempt to find the word in the dictionary (dictLookup)
**     If successful, execute the word.
**     Otherwise, attempt to convert the word to a number (isNumber)
**     If successful, push the number onto the parameter stack.
**     Otherwise, print an error message and exit loop...
**   End Loop
**
** From the standard, section 3.4
** Text interpretation (see 6.1.1360 EVALUATE and 6.1.2050 QUIT) shall
** repeat the following steps until either the parse area is empty or an 
** ambiguous condition exists: 
** a) Skip leading spaces and parse a name (see 3.4.1); 
**************************************************************************/

static void interpret(FICL_VM *pVM)
{
    STRINGINFO si = vmGetWord0(pVM);
    assert(pVM);

    vmBranchRelative(pVM, -1);

    /*
    ** Get next word...if out of text, we're done.
    */
    if (si.count == 0)
    {
        vmThrow(pVM, VM_OUTOFTEXT);
    }

    interpWord(pVM, si);


    return;                 /* back to inner interpreter */
}

/**************************************************************************
** From the standard, section 3.4
** b) Search the dictionary name space (see 3.4.2). If a definition name
** matching the string is found: 
**  1.if interpreting, perform the interpretation semantics of the definition
**  (see 3.4.3.2), and continue at a); 
**  2.if compiling, perform the compilation semantics of the definition
**  (see 3.4.3.3), and continue at a). 
**
** c) If a definition name matching the string is not found, attempt to
** convert the string to a number (see 3.4.1.3). If successful: 
**  1.if interpreting, place the number on the data stack, and continue at a); 
**  2.if compiling, compile code that when executed will place the number on
**  the stack (see 6.1.1780 LITERAL), and continue at a); 
**
** d) If unsuccessful, an ambiguous condition exists (see 3.4.4). 
**************************************************************************/
static void interpWord(FICL_VM *pVM, STRINGINFO si)
{
    FICL_DICT *dp = ficlGetDict();
    FICL_WORD *tempFW;

#if FICL_ROBUST
    dictCheck(dp, pVM, 0);
    vmCheckStack(pVM, 0, 0);
#endif

#if FICL_WANT_LOCALS
    if (nLocals > 0)
    {
        tempFW = dictLookupLoc(dp, si);
    }
    else
#endif
    tempFW = dictLookup(dp, si);

    if (pVM->state == INTERPRET)
    {
        if (tempFW != NULL)
        {
            if (wordIsCompileOnly(tempFW))
            {
                vmThrowErr(pVM, "Error: Compile only!");
            }

            vmExecute(pVM, tempFW);
        }

        else if (!isNumber(pVM, si))
        {
            int i = SI_COUNT(si);
            vmThrowErr(pVM, "%.*s not found", i, SI_PTR(si));
        }
    }

    else /* (pVM->state == COMPILE) */
    {
        if (tempFW != NULL)
        {
            if (wordIsImmediate(tempFW))
            {
                vmExecute(pVM, tempFW);
            }
            else
            {
                dictAppendCell(dp, LVALUEtoCELL(tempFW));
            }
        }
        else if (isNumber(pVM, si))
        {
            literalIm(pVM);
        }
        else
        {
            int i = SI_COUNT(si);
            vmThrowErr(pVM, "%.*s not found", i, SI_PTR(si));
        }
    }

    return;
}


/**************************************************************************
                        l i t e r a l P a r e n
** 
** This is the runtime for (literal). It assumes that it is part of a colon
** definition, and that the next CELL contains a value to be pushed on the
** parameter stack at runtime. This code is compiled by "literal".
**
**************************************************************************/

static void literalParen(FICL_VM *pVM)
{
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 0, 1);
#endif
    stackPushINT(pVM->pStack, *(FICL_INT *)(pVM->ip));
    vmBranchRelative(pVM, 1);
    return;
}

static void twoLitParen(FICL_VM *pVM)
{
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 0, 2);
#endif
    stackPushINT(pVM->pStack, *((FICL_INT *)(pVM->ip)+1));
    stackPushINT(pVM->pStack, *(FICL_INT *)(pVM->ip));
    vmBranchRelative(pVM, 2);
    return;
}


/**************************************************************************
                        l i t e r a l I m
** 
** IMMEDIATE code for "literal". This function gets a value from the stack 
** and compiles it into the dictionary preceded by the code for "(literal)".
** IMMEDIATE
**************************************************************************/

static void literalIm(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();
    assert(pLitParen);

    dictAppendCell(dp, LVALUEtoCELL(pLitParen));
    dictAppendCell(dp, stackPop(pVM->pStack));

    return;
}


static void twoLiteralIm(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();
    assert(pTwoLitParen);

    dictAppendCell(dp, LVALUEtoCELL(pTwoLitParen));
    dictAppendCell(dp, stackPop(pVM->pStack));
    dictAppendCell(dp, stackPop(pVM->pStack));

    return;
}

/**************************************************************************
                        l i s t W o r d s
** 
**************************************************************************/
#define nCOLWIDTH 8
static void listWords(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();
    FICL_HASH *pHash = dp->pSearch[dp->nLists - 1];
    FICL_WORD *wp;
    int nChars = 0;
    int len;
    unsigned i;
    int nWords = 0;
    char *cp;
    char *pPad = pVM->pad;

    for (i = 0; i < pHash->size; i++)
    {
        for (wp = pHash->table[i]; wp != NULL; wp = wp->link, nWords++)
        {
            if (wp->nName == 0) /* ignore :noname defs */
                continue;

            cp = wp->name;
            nChars += sprintf(pPad + nChars, "%s", cp);

            if (nChars > 70)
            {
                pPad[nChars] = '\0';
                nChars = 0;
                vmTextOut(pVM, pPad, 1);
            }
            else
            {
                len = nCOLWIDTH - nChars % nCOLWIDTH;
                while (len-- > 0)
                    pPad[nChars++] = ' ';
            }

            if (nChars > 70)
            {
                pPad[nChars] = '\0';
                nChars = 0;
                vmTextOut(pVM, pPad, 1);
            }
        }
    }

    if (nChars > 0)
    {
        pPad[nChars] = '\0';
        nChars = 0;
        vmTextOut(pVM, pPad, 1);
    }

    sprintf(pVM->pad, "Dictionary: %d words, %ld cells used of %lu total", 
        nWords, dp->here - dp->dict, dp->size);
    vmTextOut(pVM, pVM->pad, 1);
    return;
}


static void listEnv(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetEnv();
    FICL_HASH *pHash = dp->pForthWords;
    FICL_WORD *wp;
    unsigned i;
    int nWords = 0;

    for (i = 0; i < pHash->size; i++)
    {
        for (wp = pHash->table[i]; wp != NULL; wp = wp->link, nWords++)
        {
            vmTextOut(pVM, wp->name, 1);
        }
    }

    sprintf(pVM->pad, "Environment: %d words, %ld cells used of %lu total", 
        nWords, dp->here - dp->dict, dp->size);
    vmTextOut(pVM, pVM->pad, 1);
    return;
}


/**************************************************************************
                        l o g i c   a n d   c o m p a r i s o n s
** 
**************************************************************************/

static void zeroEquals(FICL_VM *pVM)
{
    CELL c;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 1, 1);
#endif
    c.i = FICL_BOOL(stackPopINT(pVM->pStack) == 0);
    stackPush(pVM->pStack, c);
    return;
}

static void zeroLess(FICL_VM *pVM)
{
    CELL c;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 1, 1);
#endif
    c.i = FICL_BOOL(stackPopINT(pVM->pStack) < 0);
    stackPush(pVM->pStack, c);
    return;
}

static void zeroGreater(FICL_VM *pVM)
{
    CELL c;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 1, 1);
#endif
    c.i = FICL_BOOL(stackPopINT(pVM->pStack) > 0);
    stackPush(pVM->pStack, c);
    return;
}

static void isEqual(FICL_VM *pVM)
{
    CELL x, y;

#if FICL_ROBUST > 1
    vmCheckStack(pVM, 2, 1);
#endif
    x = stackPop(pVM->pStack);
    y = stackPop(pVM->pStack);
    stackPushINT(pVM->pStack, FICL_BOOL(x.i == y.i));
    return;
}

static void isLess(FICL_VM *pVM)
{
    CELL x, y;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 2, 1);
#endif
    y = stackPop(pVM->pStack);
    x = stackPop(pVM->pStack);
    stackPushINT(pVM->pStack, FICL_BOOL(x.i < y.i));
    return;
}

static void uIsLess(FICL_VM *pVM)
{
    FICL_UNS u1, u2;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 2, 1);
#endif
    u2 = stackPopUNS(pVM->pStack);
    u1 = stackPopUNS(pVM->pStack);
    stackPushINT(pVM->pStack, FICL_BOOL(u1 < u2));
    return;
}

static void isGreater(FICL_VM *pVM)
{
    CELL x, y;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 2, 1);
#endif
    y = stackPop(pVM->pStack);
    x = stackPop(pVM->pStack);
    stackPushINT(pVM->pStack, FICL_BOOL(x.i > y.i));
    return;
}

static void bitwiseAnd(FICL_VM *pVM)
{
    CELL x, y;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 2, 1);
#endif
    x = stackPop(pVM->pStack);
    y = stackPop(pVM->pStack);
    stackPushINT(pVM->pStack, x.i & y.i);
    return;
}

static void bitwiseOr(FICL_VM *pVM)
{
    CELL x, y;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 2, 1);
#endif
    x = stackPop(pVM->pStack);
    y = stackPop(pVM->pStack);
    stackPushINT(pVM->pStack, x.i | y.i);
    return;
}

static void bitwiseXor(FICL_VM *pVM)
{
    CELL x, y;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 2, 1);
#endif
    x = stackPop(pVM->pStack);
    y = stackPop(pVM->pStack);
    stackPushINT(pVM->pStack, x.i ^ y.i);
    return;
}

static void bitwiseNot(FICL_VM *pVM)
{
    CELL x;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 1, 1);
#endif
    x = stackPop(pVM->pStack);
    stackPushINT(pVM->pStack, ~x.i);
    return;
}


/**************************************************************************
                               D o  /  L o o p
** do -- IMMEDIATE COMPILE ONLY
**    Compiles code to initialize a loop: compile (do), 
**    allot space to hold the "leave" address, push a branch
**    target address for the loop.
** (do) -- runtime for "do"
**    pops index and limit from the p stack and moves them
**    to the r stack, then skips to the loop body.
** loop -- IMMEDIATE COMPILE ONLY
** +loop
**    Compiles code for the test part of a loop:
**    compile (loop), resolve forward branch from "do", and
**    copy "here" address to the "leave" address allotted by "do"
** i,j,k -- COMPILE ONLY
**    Runtime: Push loop indices on param stack (i is innermost loop...)
**    Note: each loop has three values on the return stack:
**    ( R: leave limit index )
**    "leave" is the absolute address of the next cell after the loop
**    limit and index are the loop control variables.
** leave -- COMPILE ONLY
**    Runtime: pop the loop control variables, then pop the
**    "leave" address and jump (absolute) there.
**************************************************************************/

static void doCoIm(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();

    assert(pDoParen);

    dictAppendCell(dp, LVALUEtoCELL(pDoParen));
    /*
    ** Allot space for a pointer to the end
    ** of the loop - "leave" uses this...
    */
    markBranch(dp, pVM, leaveTag);
    dictAppendUNS(dp, 0);
    /*
    ** Mark location of head of loop...
    */
    markBranch(dp, pVM, doTag);

    return;
}


static void doParen(FICL_VM *pVM)
{
    CELL index, limit;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 2, 0);
#endif
    index = stackPop(pVM->pStack);
    limit = stackPop(pVM->pStack);

    /* copy "leave" target addr to stack */
    stackPushPtr(pVM->rStack, *(pVM->ip++));
    stackPush(pVM->rStack, limit);
    stackPush(pVM->rStack, index);

    return;
}


static void qDoCoIm(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();

    assert(pQDoParen);

    dictAppendCell(dp, LVALUEtoCELL(pQDoParen));
    /*
    ** Allot space for a pointer to the end
    ** of the loop - "leave" uses this...
    */
    markBranch(dp, pVM, leaveTag);
    dictAppendUNS(dp, 0);
    /*
    ** Mark location of head of loop...
    */
    markBranch(dp, pVM, doTag);

    return;
}


static void qDoParen(FICL_VM *pVM)
{
    CELL index, limit;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 2, 0);
#endif
    index = stackPop(pVM->pStack);
    limit = stackPop(pVM->pStack);

    /* copy "leave" target addr to stack */
    stackPushPtr(pVM->rStack, *(pVM->ip++));

    if (limit.u == index.u)
    {
        vmPopIP(pVM);
    }
    else
    {
        stackPush(pVM->rStack, limit);
        stackPush(pVM->rStack, index);
    }

    return;
}


/*
** Runtime code to break out of a do..loop construct
** Drop the loop control variables; the branch address
** past "loop" is next on the return stack.
*/
static void leaveCo(FICL_VM *pVM)
{
    /* almost unloop */
    stackDrop(pVM->rStack, 2);
    /* exit */
    vmPopIP(pVM);
    return;
}


static void unloopCo(FICL_VM *pVM)
{
    stackDrop(pVM->rStack, 3);
    return;
}


static void loopCoIm(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();

    assert(pLoopParen);

    dictAppendCell(dp, LVALUEtoCELL(pLoopParen));
    resolveBackBranch(dp, pVM, doTag);
    resolveAbsBranch(dp, pVM, leaveTag);
    return;
}


static void plusLoopCoIm(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();

    assert(pPLoopParen);

    dictAppendCell(dp, LVALUEtoCELL(pPLoopParen));
    resolveBackBranch(dp, pVM, doTag);
    resolveAbsBranch(dp, pVM, leaveTag);
    return;
}


static void loopParen(FICL_VM *pVM)
{
    FICL_INT index = stackGetTop(pVM->rStack).i;
    FICL_INT limit = stackFetch(pVM->rStack, 1).i;

    index++;

    if (index >= limit) 
    {
        stackDrop(pVM->rStack, 3); /* nuke the loop indices & "leave" addr */
        vmBranchRelative(pVM, 1);  /* fall through the loop */
    }
    else 
    {                       /* update index, branch to loop head */
        stackSetTop(pVM->rStack, LVALUEtoCELL(index));
        vmBranchRelative(pVM, *(int *)(pVM->ip));
    }

    return;
}


static void plusLoopParen(FICL_VM *pVM)
{
    FICL_INT index = stackGetTop(pVM->rStack).i;
    FICL_INT limit = stackFetch(pVM->rStack, 1).i;
    FICL_INT increment = stackPop(pVM->pStack).i;
    int flag;

    index += increment;

    if (increment < 0)
        flag = (index < limit);
    else
        flag = (index >= limit);

    if (flag) 
    {
        stackDrop(pVM->rStack, 3); /* nuke the loop indices & "leave" addr */
        vmBranchRelative(pVM, 1);  /* fall through the loop */
    }
    else 
    {                       /* update index, branch to loop head */
        stackSetTop(pVM->rStack, LVALUEtoCELL(index));
        vmBranchRelative(pVM, *(int *)(pVM->ip));
    }

    return;
}


static void loopICo(FICL_VM *pVM)
{
    CELL index = stackGetTop(pVM->rStack);
    stackPush(pVM->pStack, index);

    return;
}


static void loopJCo(FICL_VM *pVM)
{
    CELL index = stackFetch(pVM->rStack, 3);
    stackPush(pVM->pStack, index);

    return;
}


static void loopKCo(FICL_VM *pVM)
{
    CELL index = stackFetch(pVM->rStack, 6);
    stackPush(pVM->pStack, index);

    return;
}


/**************************************************************************
                        r e t u r n   s t a c k
** 
**************************************************************************/

static void toRStack(FICL_VM *pVM)
{
    stackPush(pVM->rStack, stackPop(pVM->pStack));
    return;
}

static void fromRStack(FICL_VM *pVM)
{
    stackPush(pVM->pStack, stackPop(pVM->rStack));
    return;
}

static void fetchRStack(FICL_VM *pVM)
{
    stackPush(pVM->pStack, stackGetTop(pVM->rStack));
    return;
}


/**************************************************************************
                        v a r i a b l e
** 
**************************************************************************/

static void variableParen(FICL_VM *pVM)
{
    FICL_WORD *fw = pVM->runningWord;
    stackPushPtr(pVM->pStack, fw->param);
    return;
}


static void variable(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();
    STRINGINFO si = vmGetWord(pVM);

    dictAppendWord2(dp, si, variableParen, FW_DEFAULT);
    dictAllotCells(dp, 1);
    return;
}



/**************************************************************************
                        b a s e   &   f r i e n d s
** 
**************************************************************************/

static void base(FICL_VM *pVM)
{
    CELL *pBase = (CELL *)(&pVM->base);
    stackPush(pVM->pStack, LVALUEtoCELL(pBase));
    return;
}


static void decimal(FICL_VM *pVM)
{
    pVM->base = 10;
    return;
}


static void hex(FICL_VM *pVM)
{
    pVM->base = 16;
    return;
}


/**************************************************************************
                        a l l o t   &   f r i e n d s
** 
**************************************************************************/

static void allot(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();
    FICL_INT i = stackPopINT(pVM->pStack);
#if FICL_ROBUST
    dictCheck(dp, pVM, i);
#endif
    dictAllot(dp, i);
    return;
}


static void here(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();
    stackPushPtr(pVM->pStack, dp->here);
    return;
}


static void comma(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();
    CELL c = stackPop(pVM->pStack);
    dictAppendCell(dp, c);
    return;
}


static void cComma(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();
    char c = (char)stackPopINT(pVM->pStack);
    dictAppendChar(dp, c);
    return;
}


static void cells(FICL_VM *pVM)
{
    FICL_INT i = stackPopINT(pVM->pStack);
    stackPushINT(pVM->pStack, i * (FICL_INT)sizeof (CELL));
    return;
}


static void cellPlus(FICL_VM *pVM)
{
    char *cp = stackPopPtr(pVM->pStack);
    stackPushPtr(pVM->pStack, cp + sizeof (CELL));
    return;
}


/**************************************************************************
                        t i c k
** tick         CORE ( "<spaces>name" -- xt )
** Skip leading space delimiters. Parse name delimited by a space. Find
** name and return xt, the execution token for name. An ambiguous condition
** exists if name is not found. 
**************************************************************************/
static void tick(FICL_VM *pVM)
{
    FICL_WORD *pFW = NULL;
    STRINGINFO si = vmGetWord(pVM);
    
    pFW = dictLookup(ficlGetDict(), si);
    if (!pFW)
    {
        int i = SI_COUNT(si);
        vmThrowErr(pVM, "%.*s not found", i, SI_PTR(si));
    }
    stackPushPtr(pVM->pStack, pFW);
    return;
}


static void bracketTickCoIm(FICL_VM *pVM)
{
    tick(pVM);
    literalIm(pVM);
    
    return;
}


/**************************************************************************
                        p o s t p o n e
** Lookup the next word in the input stream and compile code to 
** insert it into definitions created by the resulting word
** (defers compilation, even of immediate words)
**************************************************************************/

static void postponeCoIm(FICL_VM *pVM)
{
    FICL_DICT *dp  = ficlGetDict();
    FICL_WORD *pFW;
    assert(pComma);

    tick(pVM);
    pFW = stackGetTop(pVM->pStack).p;
    if (wordIsImmediate(pFW))
    {
        dictAppendCell(dp, stackPop(pVM->pStack));
    }
    else
    {
        literalIm(pVM);
        dictAppendCell(dp, LVALUEtoCELL(pComma));
    }
    
    return;
}



/**************************************************************************
                        e x e c u t e
** Pop an execution token (pointer to a word) off the stack and
** run it
**************************************************************************/

static void execute(FICL_VM *pVM)
{
    FICL_WORD *pFW;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 1, 0);
#endif

    pFW = stackPopPtr(pVM->pStack);
    vmExecute(pVM, pFW);

    return;
}


/**************************************************************************
                        i m m e d i a t e
** Make the most recently compiled word IMMEDIATE -- it executes even
** in compile state (most often used for control compiling words
** such as IF, THEN, etc)
**************************************************************************/

static void immediate(FICL_VM *pVM)
{
    IGNORE(pVM);
    dictSetImmediate(ficlGetDict());
    return;
}


static void compileOnly(FICL_VM *pVM)
{
    IGNORE(pVM);
    dictSetFlags(ficlGetDict(), FW_COMPILE, 0);
    return;
}


/**************************************************************************
                        d o t Q u o t e
** IMMEDIATE word that compiles a string literal for later display
** Compile stringLit, then copy the bytes of the string from the TIB
** to the dictionary. Backpatch the count byte and align the dictionary.
**
** stringlit: Fetch the count from the dictionary, then push the address
** and count on the stack. Finally, update ip to point to the first
** aligned address after the string text.
**************************************************************************/

static void stringLit(FICL_VM *pVM)
{
    FICL_STRING *sp = (FICL_STRING *)(pVM->ip);
    FICL_COUNT count = sp->count;
    char *cp = sp->text;
    stackPushPtr(pVM->pStack, cp);
    stackPushUNS(pVM->pStack, count);
    cp += count + 1;
    cp = alignPtr(cp);
    pVM->ip = (IPTYPE)(void *)cp;
    return;
}

static void dotQuoteCoIm(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();
    dictAppendCell(dp, LVALUEtoCELL(pStringLit));
    dp->here = PTRtoCELL vmGetString(pVM, (FICL_STRING *)dp->here, '\"');
    dictAlign(dp);
    dictAppendCell(dp, LVALUEtoCELL(pType));
    return;
}


static void dotParen(FICL_VM *pVM)
{
    char *pSrc      = vmGetInBuf(pVM);
    char *pEnd      = vmGetInBufEnd(pVM);
    char *pDest     = pVM->pad;
    char ch;

    for (ch = *pSrc; (pEnd != pSrc) && (ch != ')'); ch = *++pSrc)
        *pDest++ = ch;

    *pDest = '\0';
    if ((pEnd != pSrc) && (ch == ')'))
        pSrc++;

    vmTextOut(pVM, pVM->pad, 0);
    vmUpdateTib(pVM, pSrc);
        
    return;
}


/**************************************************************************
                        s l i t e r a l
** STRING 
** Interpretation: Interpretation semantics for this word are undefined.
** Compilation: ( c-addr1 u -- )
** Append the run-time semantics given below to the current definition.
** Run-time:       ( -- c-addr2 u )
** Return c-addr2 u describing a string consisting of the characters
** specified by c-addr1 u during compilation. A program shall not alter
** the returned string. 
**************************************************************************/
static void sLiteralCoIm(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();
    char *cp, *cpDest;
    FICL_UNS u;
    u  = stackPopUNS(pVM->pStack);
    cp = stackPopPtr(pVM->pStack);

    dictAppendCell(dp, LVALUEtoCELL(pStringLit));
    cpDest    = (char *) dp->here;
    *cpDest++ = (char)   u;

    for (; u > 0; --u)
    {
        *cpDest++ = *cp++;
    }

    *cpDest++ = 0;
    dp->here = PTRtoCELL alignPtr(cpDest);
    return;
}


/**************************************************************************
                        s t a t e
** Return the address of the VM's state member (must be sized the
** same as a CELL for this reason)
**************************************************************************/
static void state(FICL_VM *pVM)
{
    stackPushPtr(pVM->pStack, &pVM->state);
    return;
}


/**************************************************************************
                        c r e a t e . . . d o e s >
** Make a new word in the dictionary with the run-time effect of 
** a variable (push my address), but with extra space allotted
** for use by does> .
**************************************************************************/

static void createParen(FICL_VM *pVM)
{
    CELL *pCell = pVM->runningWord->param;
    stackPushPtr(pVM->pStack, pCell+1);
    return;
}


static void create(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();
    STRINGINFO si = vmGetWord(pVM);

    dictAppendWord2(dp, si, createParen, FW_DEFAULT);
    dictAllotCells(dp, 1);
    return;
}


static void doDoes(FICL_VM *pVM)
{
    CELL *pCell = pVM->runningWord->param;
    IPTYPE tempIP = (IPTYPE)((*pCell).p);
    stackPushPtr(pVM->pStack, pCell+1);
    vmPushIP(pVM, tempIP);
    return;
}


static void doesParen(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();
    dp->smudge->code = doDoes;
    dp->smudge->param[0] = LVALUEtoCELL(pVM->ip);
    vmPopIP(pVM);
    return;
}


static void doesCoIm(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();
#if FICL_WANT_LOCALS
    assert(pUnLinkParen);
    if (nLocals > 0)
    {
        FICL_DICT *pLoc = ficlGetLoc();
        dictEmpty(pLoc, pLoc->pForthWords->size);
        dictAppendCell(dp, LVALUEtoCELL(pUnLinkParen));
    }

    nLocals = 0;
#endif
    IGNORE(pVM);

    dictAppendCell(dp, LVALUEtoCELL(pDoesParen));
    return;
}


/**************************************************************************
                        t o   b o d y
** to-body      CORE ( xt -- a-addr )
** a-addr is the data-field address corresponding to xt. An ambiguous
** condition exists if xt is not for a word defined via CREATE. 
**************************************************************************/
static void toBody(FICL_VM *pVM)
{
    FICL_WORD *pFW = stackPopPtr(pVM->pStack);
    stackPushPtr(pVM->pStack, pFW->param + 1);
    return;
}


/*
** from-body       ficl ( a-addr -- xt )
** Reverse effect of >body
*/
static void fromBody(FICL_VM *pVM)
{
    char *ptr = (char *) stackPopPtr(pVM->pStack) - sizeof (FICL_WORD);
    stackPushPtr(pVM->pStack, ptr);
    return;
}


/*
** >name        ficl ( xt -- c-addr u )
** Push the address and length of a word's name given its address
** xt. 
*/
static void toName(FICL_VM *pVM)
{
    FICL_WORD *pFW = stackPopPtr(pVM->pStack);
    stackPushPtr(pVM->pStack, pFW->name);
    stackPushUNS(pVM->pStack, pFW->nName);
    return;
}


/**************************************************************************
                        l b r a c k e t   e t c
** 
**************************************************************************/

static void lbracketCoIm(FICL_VM *pVM)
{
    pVM->state = INTERPRET;
    return;
}


static void rbracket(FICL_VM *pVM)
{
    pVM->state = COMPILE;
    return;
}


/**************************************************************************
                        p i c t u r e d   n u m e r i c   w o r d s
**
** less-number-sign CORE ( -- )
** Initialize the pictured numeric output conversion process. 
** (clear the pad)
**************************************************************************/
static void lessNumberSign(FICL_VM *pVM)
{
    FICL_STRING *sp = PTRtoSTRING pVM->pad;
    sp->count = 0;
    return;
}

/*
** number-sign      CORE ( ud1 -- ud2 )
** Divide ud1 by the number in BASE giving the quotient ud2 and the remainder
** n. (n is the least-significant digit of ud1.) Convert n to external form
** and add the resulting character to the beginning of the pictured numeric
** output  string. An ambiguous condition exists if # executes outside of a
** <# #> delimited number conversion. 
*/
static void numberSign(FICL_VM *pVM)
{
    FICL_STRING *sp = PTRtoSTRING pVM->pad;
    DPUNS u;
    UNS16 rem;
    
    u   = u64Pop(pVM->pStack);
    rem = m64UMod(&u, (UNS16)(pVM->base));
    sp->text[sp->count++] = digit_to_char(rem);
    u64Push(pVM->pStack, u);
    return;
}

/*
** number-sign-greater CORE ( xd -- c-addr u )
** Drop xd. Make the pictured numeric output string available as a character
** string. c-addr and u specify the resulting character string. A program
** may replace characters within the string. 
*/
static void numberSignGreater(FICL_VM *pVM)
{
    FICL_STRING *sp = PTRtoSTRING pVM->pad;
    sp->text[sp->count] = '\0';
    strrev(sp->text);
    stackDrop(pVM->pStack, 2);
    stackPushPtr(pVM->pStack, sp->text);
    stackPushUNS(pVM->pStack, sp->count);
    return;
}

/*
** number-sign-s    CORE ( ud1 -- ud2 )
** Convert one digit of ud1 according to the rule for #. Continue conversion
** until the quotient is zero. ud2 is zero. An ambiguous condition exists if
** #S executes outside of a <# #> delimited number conversion. 
** TO DO: presently does not use ud1 hi cell - use it!
*/
static void numberSignS(FICL_VM *pVM)
{
    FICL_STRING *sp = PTRtoSTRING pVM->pad;
    DPUNS u;
    UNS16 rem;

    u = u64Pop(pVM->pStack);

    do 
    {
        rem = m64UMod(&u, (UNS16)(pVM->base));
        sp->text[sp->count++] = digit_to_char(rem);
    } 
    while (u.hi || u.lo);

    u64Push(pVM->pStack, u);
    return;
}

/*
** HOLD             CORE ( char -- )
** Add char to the beginning of the pictured numeric output string. An ambiguous
** condition exists if HOLD executes outside of a <# #> delimited number conversion.
*/
static void hold(FICL_VM *pVM)
{
    FICL_STRING *sp = PTRtoSTRING pVM->pad;
    int i = stackPopINT(pVM->pStack);
    sp->text[sp->count++] = (char) i;
    return;
}

/*
** SIGN             CORE ( n -- )
** If n is negative, add a minus sign to the beginning of the pictured
** numeric output string. An ambiguous condition exists if SIGN
** executes outside of a <# #> delimited number conversion. 
*/
static void sign(FICL_VM *pVM)
{
    FICL_STRING *sp = PTRtoSTRING pVM->pad;
    int i = stackPopINT(pVM->pStack);
    if (i < 0)
        sp->text[sp->count++] = '-';
    return;
}


/**************************************************************************
                        t o   N u m b e r
** to-number CORE ( ud1 c-addr1 u1 -- ud2 c-addr2 u2 )
** ud2 is the unsigned result of converting the characters within the
** string specified by c-addr1 u1 into digits, using the number in BASE,
** and adding each into ud1 after multiplying ud1 by the number in BASE.
** Conversion continues left-to-right until a character that is not
** convertible, including any + or -, is encountered or the string is
** entirely converted. c-addr2 is the location of the first unconverted
** character or the first character past the end of the string if the string
** was entirely converted. u2 is the number of unconverted characters in the
** string. An ambiguous condition exists if ud2 overflows during the
** conversion. 
**************************************************************************/
static void toNumber(FICL_VM *pVM)
{
    FICL_UNS count  = stackPopUNS(pVM->pStack);
    char *cp        = (char *)stackPopPtr(pVM->pStack);
    DPUNS accum;
    FICL_UNS base   = pVM->base;
    FICL_UNS ch;
    FICL_UNS digit;

    accum = u64Pop(pVM->pStack);

    for (ch = *cp; count > 0; ch = *++cp, count--)
    {
        if (ch < '0')
            break;

        digit = ch - '0';

        if (digit > 9)
            digit = tolower(ch) - 'a' + 10;
        /* 
        ** Note: following test also catches chars between 9 and a
        ** because 'digit' is unsigned! 
        */
        if (digit >= base)
            break;

        accum = m64Mac(accum, base, digit);
    }

    u64Push(pVM->pStack, accum);
    stackPushPtr  (pVM->pStack, cp);
    stackPushUNS(pVM->pStack, count);

    return;
}



/**************************************************************************
                        q u i t   &   a b o r t
** quit CORE   ( -- )  ( R:  i*x -- )
** Empty the return stack, store zero in SOURCE-ID if it is present, make
** the user input device the input source, and enter interpretation state. 
** Do not display a message. Repeat the following: 
**
**   Accept a line from the input source into the input buffer, set >IN to
**   zero, and interpret. 
**   Display the implementation-defined system prompt if in
**   interpretation state, all processing has been completed, and no
**   ambiguous condition exists. 
**************************************************************************/

static void quit(FICL_VM *pVM)
{
    vmThrow(pVM, VM_QUIT);
    return;
}


static void ficlAbort(FICL_VM *pVM)
{
    vmThrow(pVM, VM_ABORT);
    return;
}


/**************************************************************************
                        a c c e p t
** accept       CORE ( c-addr +n1 -- +n2 )
** Receive a string of at most +n1 characters. An ambiguous condition
** exists if +n1 is zero or greater than 32,767. Display graphic characters
** as they are received. A program that depends on the presence or absence
** of non-graphic characters in the string has an environmental dependency.
** The editing functions, if any, that the system performs in order to
** construct the string are implementation-defined. 
**
** (Although the standard text doesn't say so, I assume that the intent 
** of 'accept' is to store the string at the address specified on
** the stack.)
** Implementation: if there's more text in the TIB, use it. Otherwise
** throw out for more text. Copy characters up to the max count into the
** address given, and return the number of actual characters copied.
** 
** Note (sobral) this may not be the behavior you'd expect if you're
** trying to get user input at load time!
**************************************************************************/
static void accept(FICL_VM *pVM)
{
    FICL_INT count;
    char *cp;
    char *pBuf      = vmGetInBuf(pVM);
    char *pEnd      = vmGetInBufEnd(pVM);
    FICL_INT len       = pEnd - pBuf;

    if (len == 0)
        vmThrow(pVM, VM_RESTART);

    /*
    ** Now we have something in the text buffer - use it 
    */
    count = stackPopINT(pVM->pStack);
    cp    = stackPopPtr(pVM->pStack);

    len = (count < len) ? count : len;
    strncpy(cp, vmGetInBuf(pVM), len);
    pBuf += len;
    vmUpdateTib(pVM, pBuf);
    stackPushINT(pVM->pStack, len);

    return;
}


/**************************************************************************
                        a l i g n
** 6.1.0705 ALIGN       CORE ( -- )
** If the data-space pointer is not aligned, reserve enough space to
** align it. 
**************************************************************************/
static void align(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();
    IGNORE(pVM);
    dictAlign(dp);
    return;
}


/**************************************************************************
                        a l i g n e d
** 
**************************************************************************/
static void aligned(FICL_VM *pVM)
{
    void *addr = stackPopPtr(pVM->pStack);
    stackPushPtr(pVM->pStack, alignPtr(addr));
    return;
}


/**************************************************************************
                        b e g i n   &   f r i e n d s
** Indefinite loop control structures
** A.6.1.0760 BEGIN 
** Typical use: 
**      : X ... BEGIN ... test UNTIL ;
** or 
**      : X ... BEGIN ... test WHILE ... REPEAT ;
**************************************************************************/
static void beginCoIm(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();
    markBranch(dp, pVM, destTag);
    return;
}

static void untilCoIm(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();

    assert(pIfParen);

    dictAppendCell(dp, LVALUEtoCELL(pIfParen));
    resolveBackBranch(dp, pVM, destTag);
    return;
}

static void whileCoIm(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();

    assert(pIfParen);

    dictAppendCell(dp, LVALUEtoCELL(pIfParen));
    markBranch(dp, pVM, origTag);
    twoSwap(pVM);
    dictAppendUNS(dp, 1);
    return;
}

static void repeatCoIm(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();

    assert(pBranchParen);
    dictAppendCell(dp, LVALUEtoCELL(pBranchParen));

    /* expect "begin" branch marker */
    resolveBackBranch(dp, pVM, destTag);
    /* expect "while" branch marker */
    resolveForwardBranch(dp, pVM, origTag);
    return;
}


static void againCoIm(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();

    assert(pBranchParen);
    dictAppendCell(dp, LVALUEtoCELL(pBranchParen));

    /* expect "begin" branch marker */
    resolveBackBranch(dp, pVM, destTag);
    return;
}


/**************************************************************************
                        c h a r   &   f r i e n d s
** 6.1.0895 CHAR    CORE ( "<spaces>name" -- char )
** Skip leading space delimiters. Parse name delimited by a space.
** Put the value of its first character onto the stack. 
**
** bracket-char     CORE 
** Interpretation: Interpretation semantics for this word are undefined.
** Compilation: ( "<spaces>name" -- )
** Skip leading space delimiters. Parse name delimited by a space.
** Append the run-time semantics given below to the current definition. 
** Run-time: ( -- char )
** Place char, the value of the first character of name, on the stack. 
**************************************************************************/
static void ficlChar(FICL_VM *pVM)
{
    STRINGINFO si = vmGetWord(pVM);
    stackPushUNS(pVM->pStack, (FICL_UNS)(si.cp[0]));

    return;
}

static void charCoIm(FICL_VM *pVM)
{
    ficlChar(pVM);
    literalIm(pVM);
    return;
}

/**************************************************************************
                        c h a r P l u s
** char-plus        CORE ( c-addr1 -- c-addr2 )
** Add the size in address units of a character to c-addr1, giving c-addr2. 
**************************************************************************/
static void charPlus(FICL_VM *pVM)
{
    char *cp = stackPopPtr(pVM->pStack);
    stackPushPtr(pVM->pStack, cp + 1);
    return;
}

/**************************************************************************
                        c h a r s
** chars        CORE ( n1 -- n2 )
** n2 is the size in address units of n1 characters. 
** For most processors, this function can be a no-op. To guarantee
** portability, we'll multiply by sizeof (char).
**************************************************************************/
#if defined (_M_IX86)
#pragma warning(disable: 4127)
#endif
static void ficlChars(FICL_VM *pVM)
{
    if (sizeof (char) > 1)
    {
        FICL_INT i = stackPopINT(pVM->pStack);
        stackPushINT(pVM->pStack, i * sizeof (char));
    }
    /* otherwise no-op! */
    return;
}
#if defined (_M_IX86)
#pragma warning(default: 4127)
#endif
 

/**************************************************************************
                        c o u n t
** COUNT    CORE ( c-addr1 -- c-addr2 u )
** Return the character string specification for the counted string stored
** at c-addr1. c-addr2 is the address of the first character after c-addr1.
** u is the contents of the character at c-addr1, which is the length in
** characters of the string at c-addr2. 
**************************************************************************/
static void count(FICL_VM *pVM)
{
    FICL_STRING *sp = stackPopPtr(pVM->pStack);
    stackPushPtr(pVM->pStack, sp->text);
    stackPushUNS(pVM->pStack, sp->count);
    return;
}

/**************************************************************************
                        e n v i r o n m e n t ?
** environment-query CORE ( c-addr u -- false | i*x true )
** c-addr is the address of a character string and u is the string's
** character count. u may have a value in the range from zero to an
** implementation-defined maximum which shall not be less than 31. The
** character string should contain a keyword from 3.2.6 Environmental
** queries or the optional word sets to be checked for correspondence
** with an attribute of the present environment. If the system treats the
** attribute as unknown, the returned flag is false; otherwise, the flag
** is true and the i*x returned is of the type specified in the table for
** the attribute queried. 
**************************************************************************/
static void environmentQ(FICL_VM *pVM)
{
    FICL_DICT *envp = ficlGetEnv();
    FICL_COUNT  len = (FICL_COUNT)stackPopUNS(pVM->pStack);
    char        *cp =  stackPopPtr(pVM->pStack);
    FICL_WORD  *pFW;
    STRINGINFO si;


    &len;       /* silence compiler warning... */
    SI_PSZ(si, cp);
    pFW = dictLookup(envp, si);

    if (pFW != NULL)
    {
        vmExecute(pVM, pFW);
        stackPushINT(pVM->pStack, FICL_TRUE);
    }
    else
    {
        stackPushINT(pVM->pStack, FICL_FALSE);
    }

    return;
}

/**************************************************************************
                        e v a l u a t e
** EVALUATE CORE ( i*x c-addr u -- j*x )
** Save the current input source specification. Store minus-one (-1) in
** SOURCE-ID if it is present. Make the string described by c-addr and u
** both the input source and input buffer, set >IN to zero, and interpret.
** When the parse area is empty, restore the prior input source
** specification. Other stack effects are due to the words EVALUATEd. 
**
**************************************************************************/
static void evaluate(FICL_VM *pVM)
{
    FICL_INT count = stackPopINT(pVM->pStack);
    char *cp    = stackPopPtr(pVM->pStack);
    CELL id;
    int result;

    id = pVM->sourceID;
    pVM->sourceID.i = -1;
    result = ficlExecC(pVM, cp, count);
    pVM->sourceID = id;
    if (result != VM_OUTOFTEXT)
        vmThrow(pVM, result);

    return;
}


/**************************************************************************
                        s t r i n g   q u o t e
** Intrpreting: get string delimited by a quote from the input stream,
** copy to a scratch area, and put its count and address on the stack.
** Compiling: compile code to push the address and count of a string
** literal, compile the string from the input stream, and align the dict
** pointer.
**************************************************************************/
static void stringQuoteIm(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();

    if (pVM->state == INTERPRET)
    {
        FICL_STRING *sp = (FICL_STRING *) dp->here;
        vmGetString(pVM, sp, '\"');
        stackPushPtr(pVM->pStack, sp->text);
        stackPushUNS(pVM->pStack, sp->count);
    }
    else    /* COMPILE state */
    {
        dictAppendCell(dp, LVALUEtoCELL(pStringLit));
        dp->here = PTRtoCELL vmGetString(pVM, (FICL_STRING *)dp->here, '\"');
        dictAlign(dp);
    }

    return;
}


/**************************************************************************
                        t y p e
** Pop count and char address from stack and print the designated string.
**************************************************************************/
static void type(FICL_VM *pVM)
{
    FICL_UNS count = stackPopUNS(pVM->pStack);
    char *cp    = stackPopPtr(pVM->pStack);

    /* 
    ** Since we don't have an output primitive for a counted string
    ** (oops), make sure the string is null terminated. If not, copy
    ** and terminate it.
    */
    if (cp[count] != '\0')
    {
        char *pDest = (char *)ficlGetDict()->here;
        if (cp != pDest)
            strncpy(pDest, cp, count);

        pDest[count] = '\0';
        cp = pDest;
    }

    vmTextOut(pVM, cp, 0);
    return;
}

/**************************************************************************
                        w o r d
** word CORE ( char "<chars>ccc<char>" -- c-addr )
** Skip leading delimiters. Parse characters ccc delimited by char. An
** ambiguous condition exists if the length of the parsed string is greater
** than the implementation-defined length of a counted string. 
** 
** c-addr is the address of a transient region containing the parsed word
** as a counted string. If the parse area was empty or contained no
** characters other than the delimiter, the resulting string has a zero
** length. A space, not included in the length, follows the string. A
** program may replace characters within the string. 
** NOTE! Ficl also NULL-terminates the dest string.
**************************************************************************/
static void ficlWord(FICL_VM *pVM)
{
    FICL_STRING *sp = (FICL_STRING *)pVM->pad;
    char      delim = (char)stackPopINT(pVM->pStack);
    STRINGINFO   si;
    
    si = vmParseStringEx(pVM, delim, 1);

    if (SI_COUNT(si) > nPAD-1)
        SI_SETLEN(si, nPAD-1);

    sp->count = (FICL_COUNT)SI_COUNT(si);
    strncpy(sp->text, SI_PTR(si), SI_COUNT(si));
    strcat(sp->text, " ");

    stackPushPtr(pVM->pStack, sp);
    return;
}


/**************************************************************************
                        p a r s e - w o r d
** ficl   PARSE-WORD  ( <spaces>name -- c-addr u )
** Skip leading spaces and parse name delimited by a space. c-addr is the
** address within the input buffer and u is the length of the selected 
** string. If the parse area is empty, the resulting string has a zero length.
**************************************************************************/
static void parseNoCopy(FICL_VM *pVM)
{
    STRINGINFO si = vmGetWord0(pVM);
    stackPushPtr(pVM->pStack, SI_PTR(si));
    stackPushUNS(pVM->pStack, SI_COUNT(si));
    return;
}


/**************************************************************************
                        p a r s e
** CORE EXT  ( char "ccc<char>" -- c-addr u )
** Parse ccc delimited by the delimiter char. 
** c-addr is the address (within the input buffer) and u is the length of 
** the parsed string. If the parse area was empty, the resulting string has
** a zero length. 
** NOTE! PARSE differs from WORD: it does not skip leading delimiters.
**************************************************************************/
static void parse(FICL_VM *pVM)
{
    STRINGINFO si;
	char delim      = (char)stackPopINT(pVM->pStack);

	si = vmParseStringEx(pVM, delim, 0);
    stackPushPtr(pVM->pStack, SI_PTR(si));
    stackPushUNS(pVM->pStack, SI_COUNT(si));
    return;
}


/**************************************************************************
                        f i l l
** CORE ( c-addr u char -- )
** If u is greater than zero, store char in each of u consecutive
** characters of memory beginning at c-addr. 
**************************************************************************/
static void fill(FICL_VM *pVM)
{
    char ch  = (char)stackPopINT(pVM->pStack);
    FICL_UNS  u = stackPopUNS(pVM->pStack);
    char *cp = (char *)stackPopPtr(pVM->pStack);

    while (u > 0)
    {
        *cp++ = ch;
        u--;
    }

    return;
}


/**************************************************************************
                        f i n d
** FIND CORE ( c-addr -- c-addr 0  |  xt 1  |  xt -1 )
** Find the definition named in the counted string at c-addr. If the
** definition is not found, return c-addr and zero. If the definition is
** found, return its execution token xt. If the definition is immediate,
** also return one (1), otherwise also return minus-one (-1). For a given
** string, the values returned by FIND while compiling may differ from
** those returned while not compiling. 
**************************************************************************/
static void find(FICL_VM *pVM)
{
    FICL_STRING *sp = stackPopPtr(pVM->pStack);
    FICL_WORD *pFW;
    STRINGINFO si;

    SI_PFS(si, sp);
    pFW = dictLookup(ficlGetDict(), si);
    if (pFW)
    {
        stackPushPtr(pVM->pStack, pFW);
        stackPushINT(pVM->pStack, (wordIsImmediate(pFW) ? 1 : -1));
    }
    else
    {
        stackPushPtr(pVM->pStack, sp);
        stackPushUNS(pVM->pStack, 0);
    }
    return;
}



/**************************************************************************
                        f m S l a s h M o d
** f-m-slash-mod CORE ( d1 n1 -- n2 n3 )
** Divide d1 by n1, giving the floored quotient n3 and the remainder n2.
** Input and output stack arguments are signed. An ambiguous condition
** exists if n1 is zero or if the quotient lies outside the range of a
** single-cell signed integer. 
**************************************************************************/
static void fmSlashMod(FICL_VM *pVM)
{
    DPINT d1;
    FICL_INT n1;
    INTQR qr;

    n1    = stackPopINT(pVM->pStack);
    d1 = i64Pop(pVM->pStack);
    qr = m64FlooredDivI(d1, n1);
    stackPushINT(pVM->pStack, qr.rem);
    stackPushINT(pVM->pStack, qr.quot);
    return;
}


/**************************************************************************
                        s m S l a s h R e m
** s-m-slash-rem CORE ( d1 n1 -- n2 n3 )
** Divide d1 by n1, giving the symmetric quotient n3 and the remainder n2.
** Input and output stack arguments are signed. An ambiguous condition
** exists if n1 is zero or if the quotient lies outside the range of a
** single-cell signed integer. 
**************************************************************************/
static void smSlashRem(FICL_VM *pVM)
{
    DPINT d1;
    FICL_INT n1;
    INTQR qr;

    n1    = stackPopINT(pVM->pStack);
    d1 = i64Pop(pVM->pStack);
    qr = m64SymmetricDivI(d1, n1);
    stackPushINT(pVM->pStack, qr.rem);
    stackPushINT(pVM->pStack, qr.quot);
    return;
}


static void ficlMod(FICL_VM *pVM)
{
    DPINT d1;
    FICL_INT n1;
    INTQR qr;

    n1    = stackPopINT(pVM->pStack);
    d1.lo = stackPopINT(pVM->pStack);
    i64Extend(d1);
    qr = m64SymmetricDivI(d1, n1);
    stackPushINT(pVM->pStack, qr.rem);
    return;
}


/**************************************************************************
                        u m S l a s h M o d
** u-m-slash-mod CORE ( ud u1 -- u2 u3 )
** Divide ud by u1, giving the quotient u3 and the remainder u2.
** All values and arithmetic are unsigned. An ambiguous condition
** exists if u1 is zero or if the quotient lies outside the range of a
** single-cell unsigned integer. 
*************************************************************************/
static void umSlashMod(FICL_VM *pVM)
{
    DPUNS ud;
    FICL_UNS u1;
    UNSQR qr;

    u1    = stackPopUNS(pVM->pStack);
    ud    = u64Pop(pVM->pStack);
    qr    = ficlLongDiv(ud, u1);
    stackPushUNS(pVM->pStack, qr.rem);
    stackPushUNS(pVM->pStack, qr.quot);
    return;
}


/**************************************************************************
                        l s h i f t
** l-shift CORE ( x1 u -- x2 )
** Perform a logical left shift of u bit-places on x1, giving x2.
** Put zeroes into the least significant bits vacated by the shift.
** An ambiguous condition exists if u is greater than or equal to the
** number of bits in a cell. 
**
** r-shift CORE ( x1 u -- x2 )
** Perform a logical right shift of u bit-places on x1, giving x2.
** Put zeroes into the most significant bits vacated by the shift. An
** ambiguous condition exists if u is greater than or equal to the
** number of bits in a cell. 
**************************************************************************/
static void lshift(FICL_VM *pVM)
{
    FICL_UNS nBits = stackPopUNS(pVM->pStack);
    FICL_UNS x1    = stackPopUNS(pVM->pStack);

    stackPushUNS(pVM->pStack, x1 << nBits);
    return;
}


static void rshift(FICL_VM *pVM)
{
    FICL_UNS nBits = stackPopUNS(pVM->pStack);
    FICL_UNS x1    = stackPopUNS(pVM->pStack);

    stackPushUNS(pVM->pStack, x1 >> nBits);
    return;
}


/**************************************************************************
                        m S t a r
** m-star CORE ( n1 n2 -- d )
** d is the signed product of n1 times n2. 
**************************************************************************/
static void mStar(FICL_VM *pVM)
{
    FICL_INT n2 = stackPopINT(pVM->pStack);
    FICL_INT n1 = stackPopINT(pVM->pStack);
    DPINT d;
    
    d = m64MulI(n1, n2);
    i64Push(pVM->pStack, d);
    return;
}


static void umStar(FICL_VM *pVM)
{
    FICL_UNS u2 = stackPopUNS(pVM->pStack);
    FICL_UNS u1 = stackPopUNS(pVM->pStack);
    DPUNS ud;
    
    ud = ficlLongMul(u1, u2);
    u64Push(pVM->pStack, ud);
    return;
}


/**************************************************************************
                        m a x   &   m i n
** 
**************************************************************************/
static void ficlMax(FICL_VM *pVM)
{
    FICL_INT n2 = stackPopINT(pVM->pStack);
    FICL_INT n1 = stackPopINT(pVM->pStack);

    stackPushINT(pVM->pStack, (n1 > n2) ? n1 : n2);
    return;
}

static void ficlMin(FICL_VM *pVM)
{
    FICL_INT n2 = stackPopINT(pVM->pStack);
    FICL_INT n1 = stackPopINT(pVM->pStack);

    stackPushINT(pVM->pStack, (n1 < n2) ? n1 : n2);
    return;
}


/**************************************************************************
                        m o v e
** CORE ( addr1 addr2 u -- )
** If u is greater than zero, copy the contents of u consecutive address
** units at addr1 to the u consecutive address units at addr2. After MOVE
** completes, the u consecutive address units at addr2 contain exactly
** what the u consecutive address units at addr1 contained before the move. 
** NOTE! This implementation assumes that a char is the same size as
**       an address unit.
**************************************************************************/
static void move(FICL_VM *pVM)
{
    FICL_UNS u     = stackPopUNS(pVM->pStack);
    char *addr2 = stackPopPtr(pVM->pStack);
    char *addr1 = stackPopPtr(pVM->pStack);

    if (u == 0) 
        return;
    /*
    ** Do the copy carefully, so as to be
    ** correct even if the two ranges overlap
    */
    if (addr1 >= addr2)
    {
        for (; u > 0; u--)
            *addr2++ = *addr1++;
    }
    else
    {
        addr2 += u-1;
        addr1 += u-1;
        for (; u > 0; u--)
            *addr2-- = *addr1--;
    }

    return;
}


/**************************************************************************
                        r e c u r s e
** 
**************************************************************************/
static void recurseCoIm(FICL_VM *pVM)
{
    FICL_DICT *pDict = ficlGetDict();

    IGNORE(pVM);
    dictAppendCell(pDict, LVALUEtoCELL(pDict->smudge));
    return;
}


/**************************************************************************
                        s t o d
** s-to-d CORE ( n -- d )
** Convert the number n to the double-cell number d with the same
** numerical value. 
**************************************************************************/
static void sToD(FICL_VM *pVM)
{
    FICL_INT s = stackPopINT(pVM->pStack);

    /* sign extend to 64 bits.. */
    stackPushINT(pVM->pStack, s);
    stackPushINT(pVM->pStack, (s < 0) ? -1 : 0);
    return;
}


/**************************************************************************
                        s o u r c e
** CORE ( -- c-addr u )
** c-addr is the address of, and u is the number of characters in, the
** input buffer. 
**************************************************************************/
static void source(FICL_VM *pVM)
{
    stackPushPtr(pVM->pStack, pVM->tib.cp);
    stackPushINT(pVM->pStack, vmGetInBufLen(pVM));
    return;
}


/**************************************************************************
                        v e r s i o n
** non-standard...
**************************************************************************/
static void ficlVersion(FICL_VM *pVM)
{
    vmTextOut(pVM, "ficl Version " FICL_VER, 1);
    return;
}


/**************************************************************************
                        t o I n
** to-in CORE
**************************************************************************/
static void toIn(FICL_VM *pVM)
{
    stackPushPtr(pVM->pStack, &pVM->tib.index);
    return;
}


/**************************************************************************
                        d e f i n i t i o n s
** SEARCH ( -- )
** Make the compilation word list the same as the first word list in the
** search order. Specifies that the names of subsequent definitions will
** be placed in the compilation word list. Subsequent changes in the search
** order will not affect the compilation word list. 
**************************************************************************/
static void definitions(FICL_VM *pVM)
{
    FICL_DICT *pDict = ficlGetDict();

    assert(pDict);
    if (pDict->nLists < 1)
    {
        vmThrowErr(pVM, "DEFINITIONS error - empty search order");
    }

    pDict->pCompile = pDict->pSearch[pDict->nLists-1];
    return;
}


/**************************************************************************
                        f o r t h - w o r d l i s t
** SEARCH ( -- wid )
** Return wid, the identifier of the word list that includes all standard
** words provided by the implementation. This word list is initially the
** compilation word list and is part of the initial search order. 
**************************************************************************/
static void forthWordlist(FICL_VM *pVM)
{
    FICL_HASH *pHash = ficlGetDict()->pForthWords;
    stackPushPtr(pVM->pStack, pHash);
    return;
}


/**************************************************************************
                        g e t - c u r r e n t
** SEARCH ( -- wid )
** Return wid, the identifier of the compilation word list. 
**************************************************************************/
static void getCurrent(FICL_VM *pVM)
{
    ficlLockDictionary(TRUE);
    stackPushPtr(pVM->pStack, ficlGetDict()->pCompile);
    ficlLockDictionary(FALSE);
    return;
}


/**************************************************************************
                        g e t - o r d e r
** SEARCH ( -- widn ... wid1 n )
** Returns the number of word lists n in the search order and the word list
** identifiers widn ... wid1 identifying these word lists. wid1 identifies
** the word list that is searched first, and widn the word list that is
** searched last. The search order is unaffected.
**************************************************************************/
static void getOrder(FICL_VM *pVM)
{
    FICL_DICT *pDict = ficlGetDict();
    int nLists = pDict->nLists;
    int i;

    ficlLockDictionary(TRUE);
    for (i = 0; i < nLists; i++)
    {
        stackPushPtr(pVM->pStack, pDict->pSearch[i]);
    }

    stackPushUNS(pVM->pStack, nLists);
    ficlLockDictionary(FALSE);
    return;
}


/**************************************************************************
                        s e a r c h - w o r d l i s t
** SEARCH ( c-addr u wid -- 0 | xt 1 | xt -1 )
** Find the definition identified by the string c-addr u in the word list
** identified by wid. If the definition is not found, return zero. If the
** definition is found, return its execution token xt and one (1) if the
** definition is immediate, minus-one (-1) otherwise. 
**************************************************************************/
static void searchWordlist(FICL_VM *pVM)
{
    STRINGINFO si;
    UNS16 hashCode;
    FICL_WORD *pFW;
    FICL_HASH *pHash = stackPopPtr(pVM->pStack);

    si.count         = (FICL_COUNT)stackPopUNS(pVM->pStack);
    si.cp            = stackPopPtr(pVM->pStack);
    hashCode         = hashHashCode(si);

    ficlLockDictionary(TRUE);
    pFW = hashLookup(pHash, si, hashCode);
    ficlLockDictionary(FALSE);

    if (pFW)
    {
        stackPushPtr(pVM->pStack, pFW);
        stackPushINT(pVM->pStack, (wordIsImmediate(pFW) ? 1 : -1));
    }
    else
    {
        stackPushUNS(pVM->pStack, 0);
    }

    return;
}


/**************************************************************************
                        s e t - c u r r e n t
** SEARCH ( wid -- )
** Set the compilation word list to the word list identified by wid. 
**************************************************************************/
static void setCurrent(FICL_VM *pVM)
{
    FICL_HASH *pHash = stackPopPtr(pVM->pStack);
    FICL_DICT *pDict = ficlGetDict();
    ficlLockDictionary(TRUE);
    pDict->pCompile = pHash;
    ficlLockDictionary(FALSE);
    return;
}


/**************************************************************************
                        s e t - o r d e r
** SEARCH ( widn ... wid1 n -- )
** Set the search order to the word lists identified by widn ... wid1.
** Subsequently, word list wid1 will be searched first, and word list
** widn searched last. If n is zero, empty the search order. If n is minus
** one, set the search order to the implementation-defined minimum
** search order. The minimum search order shall include the words
** FORTH-WORDLIST and SET-ORDER. A system shall allow n to
** be at least eight.
**************************************************************************/
static void setOrder(FICL_VM *pVM)
{
    int i;
    int nLists = stackPopINT(pVM->pStack);
    FICL_DICT *dp = ficlGetDict();

    if (nLists > FICL_DEFAULT_VOCS)
    {
        vmThrowErr(pVM, "set-order error: list would be too large");
    }

    ficlLockDictionary(TRUE);

    if (nLists >= 0)
    {
        dp->nLists = nLists;
        for (i = nLists-1; i >= 0; --i)
        {
            dp->pSearch[i] = stackPopPtr(pVM->pStack);
        }
    }
    else
    {
        dictResetSearchOrder(dp);
    }

    ficlLockDictionary(FALSE);
    return;
}


/**************************************************************************
                        w o r d l i s t
** SEARCH ( -- wid )
** Create a new empty word list, returning its word list identifier wid.
** The new word list may be returned from a pool of preallocated word
** lists or may be dynamically allocated in data space. A system shall
** allow the creation of at least 8 new word lists in addition to any
** provided as part of the system. 
** Notes: 
** 1. ficl creates a new single-list hash in the dictionary and returns
**    its address.
** 2. ficl-wordlist takes an arg off the stack indicating the number of
**    hash entries in the wordlist. Ficl 2.02 and later define WORDLIST as
**    : wordlist 1 ficl-wordlist ;
**************************************************************************/
static void wordlist(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();
    FICL_HASH *pHash;
    FICL_UNS nBuckets;
    
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 1, 1);
#endif
    nBuckets = stackPopUNS(pVM->pStack);

    dictAlign(dp);
    pHash    = (FICL_HASH *)dp->here;
    dictAllot(dp, sizeof (FICL_HASH) 
        + (nBuckets-1) * sizeof (FICL_WORD *));

    pHash->size = nBuckets;
    hashReset(pHash);

    stackPushPtr(pVM->pStack, pHash);
    return;
}


/**************************************************************************
                        S E A R C H >
** ficl  ( -- wid )
** Pop wid off the search order. Error if the search order is empty
**************************************************************************/
static void searchPop(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();
    int nLists;

    ficlLockDictionary(TRUE);
    nLists = dp->nLists;
    if (nLists == 0)
    {
        vmThrowErr(pVM, "search> error: empty search order");
    }
    stackPushPtr(pVM->pStack, dp->pSearch[--dp->nLists]);
    ficlLockDictionary(FALSE);
    return;
}


/**************************************************************************
                        > S E A R C H
** ficl  ( wid -- )
** Push wid onto the search order. Error if the search order is full.
**************************************************************************/
static void searchPush(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();

    ficlLockDictionary(TRUE);
    if (dp->nLists > FICL_DEFAULT_VOCS)
    {
        vmThrowErr(pVM, ">search error: search order overflow");
    }
    dp->pSearch[dp->nLists++] = stackPopPtr(pVM->pStack);
    ficlLockDictionary(FALSE);
    return;
}


/**************************************************************************
                        c o l o n N o N a m e
** CORE EXT ( C:  -- colon-sys )  ( S:  -- xt )
** Create an unnamed colon definition and push its address.
** Change state to compile.
**************************************************************************/
static void colonNoName(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();
    FICL_WORD *pFW;
    STRINGINFO si;

    SI_SETLEN(si, 0);
    SI_SETPTR(si, NULL);

    pVM->state = COMPILE;
    pFW = dictAppendWord2(dp, si, colonParen, FW_DEFAULT | FW_SMUDGE);
    stackPushPtr(pVM->pStack, pFW);
    markControlTag(pVM, colonTag);
    return;
}


/**************************************************************************
                        u s e r   V a r i a b l e
** user  ( u -- )  "<spaces>name"  
** Get a name from the input stream and create a user variable
** with the name and the index supplied. The run-time effect
** of a user variable is to push the address of the indexed cell
** in the running vm's user array. 
**
** User variables are vm local cells. Each vm has an array of
** FICL_USER_CELLS of them when FICL_WANT_USER is nonzero.
** Ficl's user facility is implemented with two primitives,
** "user" and "(user)", a variable ("nUser") (in softcore.c) that 
** holds the index of the next free user cell, and a redefinition
** (also in softcore) of "user" that defines a user word and increments
** nUser.
**************************************************************************/
#if FICL_WANT_USER
static void userParen(FICL_VM *pVM)
{
    FICL_INT i = pVM->runningWord->param[0].i;
    stackPushPtr(pVM->pStack, &pVM->user[i]);
    return;
}


static void userVariable(FICL_VM *pVM)
{
    FICL_DICT *dp = ficlGetDict();
    STRINGINFO si = vmGetWord(pVM);
    CELL c;

    c = stackPop(pVM->pStack);
    if (c.i >= FICL_USER_CELLS)
    {
        vmThrowErr(pVM, "Error - out of user space");
    }

    dictAppendWord2(dp, si, userParen, FW_DEFAULT);
    dictAppendCell(dp, c);
    return;
}
#endif


/**************************************************************************
                        t o V a l u e
** CORE EXT 
** Interpretation: ( x "<spaces>name" -- )
** Skip leading spaces and parse name delimited by a space. Store x in 
** name. An ambiguous condition exists if name was not defined by VALUE. 
** NOTE: In ficl, VALUE is an alias of CONSTANT
**************************************************************************/
static void toValue(FICL_VM *pVM)
{
    STRINGINFO si = vmGetWord(pVM);
    FICL_DICT *dp = ficlGetDict();
    FICL_WORD *pFW;

#if FICL_WANT_LOCALS
    if ((nLocals > 0) && (pVM->state == COMPILE))
    {
        FICL_DICT *pLoc = ficlGetLoc();
        pFW = dictLookup(pLoc, si);
        if (pFW && (pFW->code == doLocalIm))
        {
            dictAppendCell(dp, LVALUEtoCELL(pToLocalParen));
            dictAppendCell(dp, LVALUEtoCELL(pFW->param[0]));
            return;
        }
		else if (pFW && pFW->code == do2LocalIm)
		{
            dictAppendCell(dp, LVALUEtoCELL(pTo2LocalParen));
            dictAppendCell(dp, LVALUEtoCELL(pFW->param[0]));
            return;
		}
    }
#endif

    assert(pStore);

    pFW = dictLookup(dp, si);
    if (!pFW)
    {
        int i = SI_COUNT(si);
        vmThrowErr(pVM, "%.*s not found", i, SI_PTR(si));
    }

    if (pVM->state == INTERPRET)
        pFW->param[0] = stackPop(pVM->pStack);
    else        /* compile code to store to word's param */
    {
        stackPushPtr(pVM->pStack, &pFW->param[0]);
        literalIm(pVM);
        dictAppendCell(dp, LVALUEtoCELL(pStore));
    }
    return;
}


#if FICL_WANT_LOCALS
/**************************************************************************
                        l i n k P a r e n
** ( -- )
** Link a frame on the return stack, reserving nCells of space for
** locals - the value of nCells is the next cell in the instruction
** stream.
**************************************************************************/
static void linkParen(FICL_VM *pVM)
{
    FICL_INT nLink = *(FICL_INT *)(pVM->ip);
    vmBranchRelative(pVM, 1);
    stackLink(pVM->rStack, nLink);
    return;
}


static void unlinkParen(FICL_VM *pVM)
{
    stackUnlink(pVM->rStack);
    return;
}


/**************************************************************************
                        d o L o c a l I m
** Immediate - cfa of a local while compiling - when executed, compiles
** code to fetch the value of a local given the local's index in the
** word's pfa
**************************************************************************/
static void getLocalParen(FICL_VM *pVM)
{
    FICL_INT nLocal = *(FICL_INT *)(pVM->ip++);
    stackPush(pVM->pStack, pVM->rStack->pFrame[nLocal]);
    return;
}


static void toLocalParen(FICL_VM *pVM)
{
    FICL_INT nLocal = *(FICL_INT *)(pVM->ip++);
    pVM->rStack->pFrame[nLocal] = stackPop(pVM->pStack);
    return;
}


static void getLocal0(FICL_VM *pVM)
{
    stackPush(pVM->pStack, pVM->rStack->pFrame[0]);
    return;
}


static void toLocal0(FICL_VM *pVM)
{
    pVM->rStack->pFrame[0] = stackPop(pVM->pStack);
    return;
}


static void getLocal1(FICL_VM *pVM)
{
    stackPush(pVM->pStack, pVM->rStack->pFrame[1]);
    return;
}


static void toLocal1(FICL_VM *pVM)
{
    pVM->rStack->pFrame[1] = stackPop(pVM->pStack);
    return;
}


/*
** Each local is recorded in a private locals dictionary as a 
** word that does doLocalIm at runtime. DoLocalIm compiles code
** into the client definition to fetch the value of the 
** corresponding local variable from the return stack.
** The private dictionary gets initialized at the end of each block
** that uses locals (in ; and does> for example).
*/
static void doLocalIm(FICL_VM *pVM)
{
    FICL_DICT *pDict = ficlGetDict();
    int nLocal = pVM->runningWord->param[0].i;

    if (pVM->state == INTERPRET)
    {
        stackPush(pVM->pStack, pVM->rStack->pFrame[nLocal]);
    }
    else
    {
        
        if (nLocal == 0)
        {
            dictAppendCell(pDict, LVALUEtoCELL(pGetLocal0));
        }
        else if (nLocal == 1)
        {
            dictAppendCell(pDict, LVALUEtoCELL(pGetLocal1));
        }
        else
        {
            dictAppendCell(pDict, LVALUEtoCELL(pGetLocalParen));
            dictAppendCell(pDict, LVALUEtoCELL(nLocal));
        }
    }
    return;
}


/**************************************************************************
                        l o c a l P a r e n
** paren-local-paren LOCAL 
** Interpretation: Interpretation semantics for this word are undefined.
** Execution: ( c-addr u -- )
** When executed during compilation, (LOCAL) passes a message to the 
** system that has one of two meanings. If u is non-zero,
** the message identifies a new local whose definition name is given by
** the string of characters identified by c-addr u. If u is zero,
** the message is last local and c-addr has no significance. 
**
** The result of executing (LOCAL) during compilation of a definition is
** to create a set of named local identifiers, each of which is
** a definition name, that only have execution semantics within the scope
** of that definition's source. 
**
** local Execution: ( -- x )
**
** Push the local's value, x, onto the stack. The local's value is
** initialized as described in 13.3.3 Processing locals and may be
** changed by preceding the local's name with TO. An ambiguous condition
** exists when local is executed while in interpretation state. 
**************************************************************************/
static void localParen(FICL_VM *pVM)
{
    FICL_DICT *pDict = ficlGetDict();
    STRINGINFO si;
    SI_SETLEN(si, stackPopUNS(pVM->pStack));
    SI_SETPTR(si, (char *)stackPopPtr(pVM->pStack));

    if (SI_COUNT(si) > 0)
    {   /* add a local to the **locals** dict and update nLocals */
        FICL_DICT *pLoc = ficlGetLoc();
        if (nLocals >= FICL_MAX_LOCALS)
        {
            vmThrowErr(pVM, "Error: out of local space");
        }

        dictAppendWord2(pLoc, si, doLocalIm, FW_COMPIMMED);
        dictAppendCell(pLoc,  LVALUEtoCELL(nLocals));

        if (nLocals == 0)
        {   /* compile code to create a local stack frame */
            dictAppendCell(pDict, LVALUEtoCELL(pLinkParen));
            /* save location in dictionary for #locals */
            pMarkLocals = pDict->here;
            dictAppendCell(pDict, LVALUEtoCELL(nLocals));
            /* compile code to initialize first local */
            dictAppendCell(pDict, LVALUEtoCELL(pToLocal0));
        }
        else if (nLocals == 1)
        {
            dictAppendCell(pDict, LVALUEtoCELL(pToLocal1));
        }
        else
        {
            dictAppendCell(pDict, LVALUEtoCELL(pToLocalParen));
            dictAppendCell(pDict, LVALUEtoCELL(nLocals));
        }

        nLocals++;
    }
    else if (nLocals > 0)
    {       /* write nLocals to (link) param area in dictionary */
        *(FICL_INT *)pMarkLocals = nLocals;
    }

    return;
}


static void get2LocalParen(FICL_VM *pVM)
{
    FICL_INT nLocal = *(FICL_INT *)(pVM->ip++);
    stackPush(pVM->pStack, pVM->rStack->pFrame[nLocal]);
    stackPush(pVM->pStack, pVM->rStack->pFrame[nLocal+1]);
    return;
}


static void do2LocalIm(FICL_VM *pVM)
{
    FICL_DICT *pDict = ficlGetDict();
    int nLocal = pVM->runningWord->param[0].i;

    if (pVM->state == INTERPRET)
    {
        stackPush(pVM->pStack, pVM->rStack->pFrame[nLocal]);
        stackPush(pVM->pStack, pVM->rStack->pFrame[nLocal+1]);
    }
    else
    {
        dictAppendCell(pDict, LVALUEtoCELL(pGet2LocalParen));
        dictAppendCell(pDict, LVALUEtoCELL(nLocal));
    }
    return;
}


static void to2LocalParen(FICL_VM *pVM)
{
    FICL_INT nLocal = *(FICL_INT *)(pVM->ip++);
    pVM->rStack->pFrame[nLocal+1] = stackPop(pVM->pStack);
    pVM->rStack->pFrame[nLocal]   = stackPop(pVM->pStack);
    return;
}


static void twoLocalParen(FICL_VM *pVM)
{
    FICL_DICT *pDict = ficlGetDict();
    STRINGINFO si;
    SI_SETLEN(si, stackPopUNS(pVM->pStack));
    SI_SETPTR(si, (char *)stackPopPtr(pVM->pStack));

    if (SI_COUNT(si) > 0)
    {   /* add a local to the **locals** dict and update nLocals */
        FICL_DICT *pLoc = ficlGetLoc();
        if (nLocals >= FICL_MAX_LOCALS)
        {
            vmThrowErr(pVM, "Error: out of local space");
        }

        dictAppendWord2(pLoc, si, do2LocalIm, FW_COMPIMMED);
        dictAppendCell(pLoc,  LVALUEtoCELL(nLocals));

        if (nLocals == 0)
        {   /* compile code to create a local stack frame */
            dictAppendCell(pDict, LVALUEtoCELL(pLinkParen));
            /* save location in dictionary for #locals */
            pMarkLocals = pDict->here;
            dictAppendCell(pDict, LVALUEtoCELL(nLocals));
        }

		dictAppendCell(pDict, LVALUEtoCELL(pTo2LocalParen));
        dictAppendCell(pDict, LVALUEtoCELL(nLocals));

        nLocals += 2;
    }
    else if (nLocals > 0)
    {       /* write nLocals to (link) param area in dictionary */
        *(FICL_INT *)pMarkLocals = nLocals;
    }

    return;
}


#endif
/**************************************************************************
                        setParentWid
** FICL
** setparentwid   ( parent-wid wid -- )
** Set WID's link field to the parent-wid. search-wordlist will 
** iterate through all the links when finding words in the child wid.
**************************************************************************/
static void setParentWid(FICL_VM *pVM)
{
    FICL_HASH *parent, *child;
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 2, 0);
#endif
    child  = (FICL_HASH *)stackPopPtr(pVM->pStack);
    parent = (FICL_HASH *)stackPopPtr(pVM->pStack);

    child->link = parent;
    return;
}


/**************************************************************************
                        s e e 
** TOOLS ( "<spaces>name" -- )
** Display a human-readable representation of the named word's definition.
** The source of the representation (object-code decompilation, source
** block, etc.) and the particular form of the display is implementation
** defined. 
** NOTE: these funcs come late in the file because they reference all
** of the word-builder funcs without declaring them again. Call me lazy.
**************************************************************************/
/*
** isAFiclWord
** Vet a candidate pointer carefully to make sure
** it's not some chunk o' inline data...
** It has to have a name, and it has to look
** like it's in the dictionary address range.
** NOTE: this excludes :noname words!
*/
static int isAFiclWord(FICL_WORD *pFW)
{
    FICL_DICT *pd  = ficlGetDict();

    if (!dictIncludes(pd, pFW))
       return 0;

    if (!dictIncludes(pd, pFW->name))
        return 0;

    return ((pFW->nName > 0) && (pFW->name[pFW->nName] == '\0'));
}

/*
** seeColon (for proctologists only)
** Walks a colon definition, decompiling
** on the fly. Knows about primitive control structures.
*/
static void seeColon(FICL_VM *pVM, CELL *pc)
{
    for (; pc->p != pSemiParen; pc++)
    {
        FICL_WORD *pFW = (FICL_WORD *)(pc->p);

        if (isAFiclWord(pFW))
        {
            if      (pFW->code == literalParen)
            {
                CELL v = *++pc;
                if (isAFiclWord(v.p))
                {
                    FICL_WORD *pLit = (FICL_WORD *)v.p;
                    sprintf(pVM->pad, "    literal %.*s (%#lx)", 
                        pLit->nName, pLit->name, v.u);
                }
                else
                    sprintf(pVM->pad, "    literal %ld (%#lx)", v.i, v.u);
            }
            else if (pFW->code == stringLit) 
            {
                FICL_STRING *sp = (FICL_STRING *)(void *)++pc;
                pc = (CELL *)alignPtr(sp->text + sp->count + 1) - 1;
                sprintf(pVM->pad, "    s\" %.*s\"", sp->count, sp->text);
            }
            else if (pFW->code == ifParen) 
            {
                CELL c = *++pc;
                if (c.i > 0)
                    sprintf(pVM->pad, "    if / while (branch rel %ld)", c.i);
                else
                    sprintf(pVM->pad, "    until (branch rel %ld)", c.i);
            }
            else if (pFW->code == branchParen) 
            {
                CELL c = *++pc;
                if (c.i > 0)
                    sprintf(pVM->pad, "    else (branch rel %ld)", c.i);
                else
                    sprintf(pVM->pad, "    repeat (branch rel %ld)", c.i);
            }
            else if (pFW->code == qDoParen) 
            {
                CELL c = *++pc;
                sprintf(pVM->pad, "    ?do (leave abs %#lx)", c.u);
            }
            else if (pFW->code == doParen) 
            {
                CELL c = *++pc;
                sprintf(pVM->pad, "    do (leave abs %#lx)", c.u);
            }
            else if (pFW->code == loopParen) 
            {
                CELL c = *++pc;
                sprintf(pVM->pad, "    loop (branch rel %#ld)", c.i);
            }
            else if (pFW->code == plusLoopParen) 
            {
                CELL c = *++pc;
                sprintf(pVM->pad, "    +loop (branch rel %#ld)", c.i);
            }
            else /* default: print word's name */
            {
                sprintf(pVM->pad, "    %.*s", pFW->nName, pFW->name);
            }
 
            vmTextOut(pVM, pVM->pad, 1);
        }
        else /* probably not a word - punt and print value */
        {
            sprintf(pVM->pad, "    %ld (%#lx)", pc->i, pc->u);
            vmTextOut(pVM, pVM->pad, 1);
        }
    }

    vmTextOut(pVM, ";", 1);
}

/*
** Here's the outer part of the decompiler. It's 
** just a big nested conditional that checks the
** CFA of the word to decompile for each kind of
** known word-builder code, and tries to do 
** something appropriate. If the CFA is not recognized,
** just indicate that it is a primitive.
*/
static void see(FICL_VM *pVM)
{
    FICL_WORD *pFW;

    tick(pVM);
    pFW = (FICL_WORD *)stackPopPtr(pVM->pStack);

    if (pFW->code == colonParen) 
    {
        sprintf(pVM->pad, ": %.*s", pFW->nName, pFW->name);
        vmTextOut(pVM, pVM->pad, 1);
        seeColon(pVM, pFW->param);
    }
    else if (pFW->code == doDoes)
    {
        vmTextOut(pVM, "does>", 1);
        seeColon(pVM, (CELL *)pFW->param->p);
    }
    else if (pFW->code ==  createParen)
    {
        vmTextOut(pVM, "create", 1);
    }
    else if (pFW->code == variableParen)
    {
        sprintf(pVM->pad, "variable = %ld (%#lx)", 
            pFW->param->i, pFW->param->u);
        vmTextOut(pVM, pVM->pad, 1);
    }
    else if (pFW->code == userParen)
    {
        sprintf(pVM->pad, "user variable %ld (%#lx)", 
            pFW->param->i, pFW->param->u);
        vmTextOut(pVM, pVM->pad, 1);
    }
    else if (pFW->code == constantParen)
    {
        sprintf(pVM->pad, "constant = %ld (%#lx)", 
            pFW->param->i, pFW->param->u);
        vmTextOut(pVM, pVM->pad, 1);
    }
    else 
    {
        vmTextOut(pVM, "primitive", 1);
    }

    if (pFW->flags & FW_IMMEDIATE)
    {
        vmTextOut(pVM, "immediate", 1);
    }

    return;
}


/**************************************************************************
                        c o m p a r e 
** STRING ( c-addr1 u1 c-addr2 u2 -- n )
** Compare the string specified by c-addr1 u1 to the string specified by
** c-addr2 u2. The strings are compared, beginning at the given addresses,
** character by character, up to the length of the shorter string or until a
** difference is found. If the two strings are identical, n is zero. If the two
** strings are identical up to the length of the shorter string, n is minus-one
** (-1) if u1 is less than u2 and one (1) otherwise. If the two strings are not
** identical up to the length of the shorter string, n is minus-one (-1) if the 
** first non-matching character in the string specified by c-addr1 u1 has a
** lesser numeric value than the corresponding character in the string specified
** by c-addr2 u2 and one (1) otherwise. 
**************************************************************************/
static void compareString(FICL_VM *pVM)
{
    char *cp1, *cp2;
    FICL_UNS u1, u2, uMin;
    int n = 0;

    vmCheckStack(pVM, 4, 1);
    u2  = stackPopUNS(pVM->pStack);
    cp2 = (char *)stackPopPtr(pVM->pStack);
    u1  = stackPopUNS(pVM->pStack);
    cp1 = (char *)stackPopPtr(pVM->pStack);

    uMin = (u1 < u2)? u1 : u2;
    for ( ; (uMin > 0) && (n == 0); uMin--)
    {
        n = (int)(*cp1++ - *cp2++);
    }

    if (n == 0)
        n = (int)(u1 - u2);

    if (n < 0) 
        n = -1;
    else if (n > 0)
        n = 1;

    stackPushINT(pVM->pStack, n);
    return;
}


/**************************************************************************
                        r e f i l l
** CORE EXT   ( -- flag )
** Attempt to fill the input buffer from the input source, returning a true
** flag if successful. 
** When the input source is the user input device, attempt to receive input
** into the terminal input buffer. If successful, make the result the input
** buffer, set >IN to zero, and return true. Receipt of a line containing no
** characters is considered successful. If there is no input available from
** the current input source, return false. 
** When the input source is a string from EVALUATE, return false and
** perform no other action. 
**************************************************************************/
static void refill(FICL_VM *pVM)
{
    FICL_INT ret = (pVM->sourceID.i == -1) ? FICL_FALSE : FICL_TRUE;
    stackPushINT(pVM->pStack, ret);
    if (ret)
        vmThrow(pVM, VM_OUTOFTEXT);
    return;
}


/**************************************************************************
                        f o r g e t
** TOOLS EXT  ( "<spaces>name" -- )
** Skip leading space delimiters. Parse name delimited by a space.
** Find name, then delete name from the dictionary along with all
** words added to the dictionary after name. An ambiguous
** condition exists if name cannot be found. 
** 
** If the Search-Order word set is present, FORGET searches the
** compilation word list. An ambiguous condition exists if the
** compilation word list is deleted. 
**************************************************************************/
static void forgetWid(FICL_VM *pVM)
{
    FICL_DICT *pDict = ficlGetDict();
    FICL_HASH *pHash;

    pHash = (FICL_HASH *)stackPopPtr(pVM->pStack);
    hashForget(pHash, pDict->here);

    return;
}


static void forget(FICL_VM *pVM)
{
    void *where;
    FICL_DICT *pDict = ficlGetDict();
    FICL_HASH *pHash = pDict->pCompile;

    tick(pVM);
    where = ((FICL_WORD *)stackPopPtr(pVM->pStack))->name;
    hashForget(pHash, where);
    pDict->here = PTRtoCELL where;

    return;
}


/**************************************************************************
                        freebsd exception handling words
** Catch, from ANS Forth standard. Installs a safety net, then EXECUTE
** the word in ToS. If an exception happens, restore the state to what
** it was before, and pushes the exception value on the stack. If not,
** push zero.
**
** Notice that Catch implements an inner interpreter. This is ugly,
** but given how ficl works, it cannot be helped. The problem is that
** colon definitions will be executed *after* the function returns,
** while "code" definitions will be executed immediately. I considered
** other solutions to this problem, but all of them shared the same
** basic problem (with added disadvantages): if ficl ever changes it's
** inner thread modus operandi, one would have to fix this word.
**
** More comments can be found throughout catch's code.
**
** Daniel C. Sobral Jan 09/1999
** sadler may 2000 -- revised to follow ficl.c:ficlExecXT.
**************************************************************************/

static void ficlCatch(FICL_VM *pVM)
{
    static FICL_WORD *pQuit = NULL;

    int         except;
    jmp_buf     vmState;
    FICL_VM     VM;
    FICL_STACK  pStack;
    FICL_STACK  rStack;
    FICL_WORD   *pFW;

    if (!pQuit)
        pQuit = ficlLookup("exit-inner");

    assert(pVM);
    assert(pQuit);
    

    /*
    ** Get xt.
    ** We need this *before* we save the stack pointer, or
    ** we'll have to pop one element out of the stack after
    ** an exception. I prefer to get done with it up front. :-)
    */
#if FICL_ROBUST > 1
    vmCheckStack(pVM, 1, 0);
#endif
    pFW = stackPopPtr(pVM->pStack);

    /* 
    ** Save vm's state -- a catch will not back out environmental
    ** changes.
    **
    ** We are *not* saving dictionary state, since it is
    ** global instead of per vm, and we are not saving
    ** stack contents, since we are not required to (and,
    ** thus, it would be useless). We save pVM, and pVM
    ** "stacks" (a structure containing general information
    ** about it, including the current stack pointer).
    */
    memcpy((void*)&VM, (void*)pVM, sizeof(FICL_VM));
    memcpy((void*)&pStack, (void*)pVM->pStack, sizeof(FICL_STACK));
    memcpy((void*)&rStack, (void*)pVM->rStack, sizeof(FICL_STACK));

    /*
    ** Give pVM a jmp_buf
    */
    pVM->pState = &vmState;

    /*
    ** Safety net
    */
    except = setjmp(vmState);

    switch (except)
	{
		/*
		** Setup condition - push poison pill so that the VM throws
		** VM_INNEREXIT if the XT terminates normally, then execute
		** the XT
		*/
	case 0:
		vmPushIP(pVM, &pQuit);			/* Open mouth, insert emetic */
        vmExecute(pVM, pFW);
        vmInnerLoop(pVM);
		break;

		/*
		** Normal exit from XT - lose the poison pill, 
		** restore old setjmp vector and push a zero. 
		*/
	case VM_INNEREXIT:
        vmPopIP(pVM);                   /* Gack - hurl poison pill */
        pVM->pState = VM.pState;        /* Restore just the setjmp vector */
        stackPushINT(pVM->pStack, 0);   /* Push 0 -- everything is ok */
		break;

		/*
		** Some other exception got thrown - restore pre-existing VM state
		** and push the exception code
		*/
	default:
        /* Restore vm's state */
        memcpy((void*)pVM, (void*)&VM, sizeof(FICL_VM));
        memcpy((void*)pVM->pStack, (void*)&pStack, sizeof(FICL_STACK));
        memcpy((void*)pVM->rStack, (void*)&rStack, sizeof(FICL_STACK));

        stackPushINT(pVM->pStack, except);/* Push error */
		break;
	}
}

/*
 * Throw --  From ANS Forth standard.
 *
 * Throw takes the ToS and, if that's different from zero,
 * returns to the last executed catch context. Further throws will
 * unstack previously executed "catches", in LIFO mode.
 *
 * Daniel C. Sobral Jan 09/1999
 */

static void ficlThrow(FICL_VM *pVM)
{
    int except;
    
    except = stackPopINT(pVM->pStack);

    if (except)
        vmThrow(pVM, except);
}


static void ansAllocate(FICL_VM *pVM)
{
    size_t size;
    void *p;

    size = stackPopINT(pVM->pStack);
    p = ficlMalloc(size);
    stackPushPtr(pVM->pStack, p);
    if (p)
        stackPushINT(pVM->pStack, 0);
    else
        stackPushINT(pVM->pStack, 1);
}


static void ansFree(FICL_VM *pVM)
{
    void *p;

    p = stackPopPtr(pVM->pStack);
    ficlFree(p);
    stackPushINT(pVM->pStack, 0);
}


static void ansResize(FICL_VM *pVM)
{
    size_t size;
    void *new, *old;

    size = stackPopINT(pVM->pStack);
    old = stackPopPtr(pVM->pStack);
    new = ficlRealloc(old, size);
    if (new) 
    {
        stackPushPtr(pVM->pStack, new);
        stackPushINT(pVM->pStack, 0);
    } 
    else 
    {
        stackPushPtr(pVM->pStack, old);
        stackPushINT(pVM->pStack, 1);
    }
}


/*
** exit-inner 
** Signals execXT that an inner loop has completed
*/
static void ficlExitInner(FICL_VM *pVM)
{
    vmThrow(pVM, VM_INNEREXIT);
}


/**************************************************************************
                        d n e g a t e
** DOUBLE   ( d1 -- d2 )
** d2 is the negation of d1. 
**************************************************************************/
static void dnegate(FICL_VM *pVM)
{
    DPINT i = i64Pop(pVM->pStack);
    i = m64Negate(i);
    i64Push(pVM->pStack, i);

    return;
}


#if 0
/**************************************************************************
                        
** 
**************************************************************************/
static void funcname(FICL_VM *pVM)
{
    IGNORE(pVM);
    return;
}


#endif
/**************************************************************************
                        f i c l C o m p i l e C o r e
** Builds the primitive wordset and the environment-query namespace.
**************************************************************************/

void ficlCompileCore(FICL_DICT *dp)
{
    assert (dp);

    /*
    ** CORE word set
    ** see softcore.c for definitions of: abs bl space spaces abort"
    */
    pStore =
    dictAppendWord(dp, "!",         store,          FW_DEFAULT);
    dictAppendWord(dp, "#",         numberSign,     FW_DEFAULT);
    dictAppendWord(dp, "#>",        numberSignGreater,FW_DEFAULT);
    dictAppendWord(dp, "#s",        numberSignS,    FW_DEFAULT);
    dictAppendWord(dp, "\'",        tick,           FW_DEFAULT);
    dictAppendWord(dp, "(",         commentHang,    FW_IMMEDIATE);
    dictAppendWord(dp, "*",         mul,            FW_DEFAULT);
    dictAppendWord(dp, "*/",        mulDiv,         FW_DEFAULT);
    dictAppendWord(dp, "*/mod",     mulDivRem,      FW_DEFAULT);
    dictAppendWord(dp, "+",         add,            FW_DEFAULT);
    dictAppendWord(dp, "+!",        plusStore,      FW_DEFAULT);
    dictAppendWord(dp, "+loop",     plusLoopCoIm,   FW_COMPIMMED);
    pComma =
    dictAppendWord(dp, ",",         comma,          FW_DEFAULT);
    dictAppendWord(dp, "-",         sub,            FW_DEFAULT);
    dictAppendWord(dp, ".",         displayCell,    FW_DEFAULT);
    dictAppendWord(dp, ".\"",       dotQuoteCoIm,   FW_COMPIMMED);
    dictAppendWord(dp, "/",         ficlDiv,        FW_DEFAULT);
    dictAppendWord(dp, "/mod",      slashMod,       FW_DEFAULT);
    dictAppendWord(dp, "0<",        zeroLess,       FW_DEFAULT);
    dictAppendWord(dp, "0=",        zeroEquals,     FW_DEFAULT);
    dictAppendWord(dp, "0>",        zeroGreater,    FW_DEFAULT);
    dictAppendWord(dp, "1+",        onePlus,        FW_DEFAULT);
    dictAppendWord(dp, "1-",        oneMinus,       FW_DEFAULT);
    dictAppendWord(dp, "2!",        twoStore,       FW_DEFAULT);
    dictAppendWord(dp, "2*",        twoMul,         FW_DEFAULT);
    dictAppendWord(dp, "2/",        twoDiv,         FW_DEFAULT);
    dictAppendWord(dp, "2@",        twoFetch,       FW_DEFAULT);
    dictAppendWord(dp, "2drop",     twoDrop,        FW_DEFAULT);
    dictAppendWord(dp, "2dup",      twoDup,         FW_DEFAULT);
    dictAppendWord(dp, "2over",     twoOver,        FW_DEFAULT);
    dictAppendWord(dp, "2swap",     twoSwap,        FW_DEFAULT);
    dictAppendWord(dp, ":",         colon,          FW_DEFAULT);
    dictAppendWord(dp, ";",         semicolonCoIm,  FW_COMPIMMED);
    dictAppendWord(dp, "<",         isLess,         FW_DEFAULT);
    dictAppendWord(dp, "<#",        lessNumberSign, FW_DEFAULT);
    dictAppendWord(dp, "=",         isEqual,        FW_DEFAULT);
    dictAppendWord(dp, ">",         isGreater,      FW_DEFAULT);
    dictAppendWord(dp, ">body",     toBody,         FW_DEFAULT);
    dictAppendWord(dp, ">in",       toIn,           FW_DEFAULT);
    dictAppendWord(dp, ">number",   toNumber,       FW_DEFAULT);
    dictAppendWord(dp, ">r",        toRStack,       FW_DEFAULT);
    dictAppendWord(dp, "?dup",      questionDup,    FW_DEFAULT);
    dictAppendWord(dp, "@",         fetch,          FW_DEFAULT);
    dictAppendWord(dp, "abort",     ficlAbort,      FW_DEFAULT);
    dictAppendWord(dp, "accept",    accept,         FW_DEFAULT);
    dictAppendWord(dp, "align",     align,          FW_DEFAULT);
    dictAppendWord(dp, "aligned",   aligned,        FW_DEFAULT);
    dictAppendWord(dp, "allot",     allot,          FW_DEFAULT);
    dictAppendWord(dp, "and",       bitwiseAnd,     FW_DEFAULT);
    dictAppendWord(dp, "base",      base,           FW_DEFAULT);
    dictAppendWord(dp, "begin",     beginCoIm,      FW_COMPIMMED);
    dictAppendWord(dp, "c!",        cStore,         FW_DEFAULT);
    dictAppendWord(dp, "c,",        cComma,         FW_DEFAULT);
    dictAppendWord(dp, "c@",        cFetch,         FW_DEFAULT);
    dictAppendWord(dp, "cell+",     cellPlus,       FW_DEFAULT);
    dictAppendWord(dp, "cells",     cells,          FW_DEFAULT);
    dictAppendWord(dp, "char",      ficlChar,       FW_DEFAULT);
    dictAppendWord(dp, "char+",     charPlus,       FW_DEFAULT);
    dictAppendWord(dp, "chars",     ficlChars,      FW_DEFAULT);
    dictAppendWord(dp, "constant",  constant,       FW_DEFAULT);
    dictAppendWord(dp, "count",     count,          FW_DEFAULT);
    dictAppendWord(dp, "cr",        cr,             FW_DEFAULT);
    dictAppendWord(dp, "create",    create,         FW_DEFAULT);
    dictAppendWord(dp, "decimal",   decimal,        FW_DEFAULT);
    dictAppendWord(dp, "depth",     depth,          FW_DEFAULT);
    dictAppendWord(dp, "do",        doCoIm,         FW_COMPIMMED);
    dictAppendWord(dp, "does>",     doesCoIm,       FW_COMPIMMED);
    dictAppendWord(dp, "drop",      drop,           FW_DEFAULT);
    dictAppendWord(dp, "dup",       dup,            FW_DEFAULT);
    dictAppendWord(dp, "else",      elseCoIm,       FW_COMPIMMED);
    dictAppendWord(dp, "emit",      emit,           FW_DEFAULT);
    dictAppendWord(dp, "environment?", environmentQ,FW_DEFAULT);
    dictAppendWord(dp, "evaluate",  evaluate,       FW_DEFAULT);
    dictAppendWord(dp, "execute",   execute,        FW_DEFAULT);
    dictAppendWord(dp, "exit",      exitCoIm,       FW_COMPIMMED);
    dictAppendWord(dp, "fill",      fill,           FW_DEFAULT);
    dictAppendWord(dp, "find",      find,           FW_DEFAULT);
    dictAppendWord(dp, "fm/mod",    fmSlashMod,     FW_DEFAULT);
    dictAppendWord(dp, "here",      here,           FW_DEFAULT);
    dictAppendWord(dp, "hex",       hex,            FW_DEFAULT);
    dictAppendWord(dp, "hold",      hold,           FW_DEFAULT);
    dictAppendWord(dp, "i",         loopICo,        FW_COMPILE);
    dictAppendWord(dp, "if",        ifCoIm,         FW_COMPIMMED);
    dictAppendWord(dp, "immediate", immediate,      FW_DEFAULT);
    dictAppendWord(dp, "invert",    bitwiseNot,     FW_DEFAULT);
    dictAppendWord(dp, "j",         loopJCo,        FW_COMPILE);
    dictAppendWord(dp, "k",         loopKCo,        FW_COMPILE);
    dictAppendWord(dp, "leave",     leaveCo,        FW_COMPILE);
    dictAppendWord(dp, "literal",   literalIm,      FW_IMMEDIATE);
    dictAppendWord(dp, "loop",      loopCoIm,       FW_COMPIMMED);
    dictAppendWord(dp, "lshift",    lshift,         FW_DEFAULT);
    dictAppendWord(dp, "m*",        mStar,          FW_DEFAULT);
    dictAppendWord(dp, "max",       ficlMax,        FW_DEFAULT);
    dictAppendWord(dp, "min",       ficlMin,        FW_DEFAULT);
    dictAppendWord(dp, "mod",       ficlMod,        FW_DEFAULT);
    dictAppendWord(dp, "move",      move,           FW_DEFAULT);
    dictAppendWord(dp, "negate",    negate,         FW_DEFAULT);
    dictAppendWord(dp, "or",        bitwiseOr,      FW_DEFAULT);
    dictAppendWord(dp, "over",      over,           FW_DEFAULT);
    dictAppendWord(dp, "postpone",  postponeCoIm,   FW_COMPIMMED);
    dictAppendWord(dp, "quit",      quit,           FW_DEFAULT);
    dictAppendWord(dp, "r>",        fromRStack,     FW_DEFAULT);
    dictAppendWord(dp, "r@",        fetchRStack,    FW_DEFAULT);
    dictAppendWord(dp, "recurse",   recurseCoIm,    FW_COMPIMMED);
    dictAppendWord(dp, "repeat",    repeatCoIm,     FW_COMPIMMED);
    dictAppendWord(dp, "rot",       rot,            FW_DEFAULT);
    dictAppendWord(dp, "rshift",    rshift,         FW_DEFAULT);
    dictAppendWord(dp, "s\"",       stringQuoteIm,  FW_IMMEDIATE);
    dictAppendWord(dp, "s>d",       sToD,           FW_DEFAULT);
    dictAppendWord(dp, "sign",      sign,           FW_DEFAULT);
    dictAppendWord(dp, "sm/rem",    smSlashRem,     FW_DEFAULT);
    dictAppendWord(dp, "source",    source,         FW_DEFAULT);
    dictAppendWord(dp, "state",     state,          FW_DEFAULT);
    dictAppendWord(dp, "swap",      swap,           FW_DEFAULT);
    dictAppendWord(dp, "then",      endifCoIm,      FW_COMPIMMED);
    pType =
    dictAppendWord(dp, "type",      type,           FW_DEFAULT);
    dictAppendWord(dp, "u.",        uDot,           FW_DEFAULT);
    dictAppendWord(dp, "u<",        uIsLess,        FW_DEFAULT);
    dictAppendWord(dp, "um*",       umStar,         FW_DEFAULT);
    dictAppendWord(dp, "um/mod",    umSlashMod,     FW_DEFAULT);
    dictAppendWord(dp, "unloop",    unloopCo,       FW_COMPILE);
    dictAppendWord(dp, "until",     untilCoIm,      FW_COMPIMMED);
    dictAppendWord(dp, "variable",  variable,       FW_DEFAULT);
    dictAppendWord(dp, "while",     whileCoIm,      FW_COMPIMMED);
    dictAppendWord(dp, "word",      ficlWord,       FW_DEFAULT);
    dictAppendWord(dp, "xor",       bitwiseXor,     FW_DEFAULT);
    dictAppendWord(dp, "[",         lbracketCoIm,   FW_COMPIMMED);
    dictAppendWord(dp, "[\']",      bracketTickCoIm,FW_COMPIMMED);
    dictAppendWord(dp, "[char]",    charCoIm,       FW_COMPIMMED);
    dictAppendWord(dp, "]",         rbracket,       FW_DEFAULT);
    /* 
    ** CORE EXT word set...
    ** see softcore.c for other definitions
    */
    dictAppendWord(dp, ".(",        dotParen,       FW_DEFAULT);
    dictAppendWord(dp, ":noname",   colonNoName,    FW_DEFAULT);
    dictAppendWord(dp, "?do",       qDoCoIm,        FW_COMPIMMED);
    dictAppendWord(dp, "again",     againCoIm,      FW_COMPIMMED);
    dictAppendWord(dp, "parse",     parse,          FW_DEFAULT);
    dictAppendWord(dp, "pick",      pick,           FW_DEFAULT);
    dictAppendWord(dp, "roll",      roll,           FW_DEFAULT);
    dictAppendWord(dp, "refill",    refill,         FW_DEFAULT);
    dictAppendWord(dp, "to",        toValue,        FW_IMMEDIATE);
    dictAppendWord(dp, "value",     constant,       FW_DEFAULT);
    dictAppendWord(dp, "\\",        commentLine,    FW_IMMEDIATE);


    /*
    ** Set CORE environment query values
    */
    ficlSetEnv("/counted-string",   FICL_STRING_MAX);
    ficlSetEnv("/hold",             nPAD);
    ficlSetEnv("/pad",              nPAD);
    ficlSetEnv("address-unit-bits", 8);
    ficlSetEnv("core",              FICL_TRUE);
    ficlSetEnv("core-ext",          FICL_FALSE);
    ficlSetEnv("floored",           FICL_FALSE);
    ficlSetEnv("max-char",          UCHAR_MAX);
    ficlSetEnvD("max-d",            0x7fffffff, 0xffffffff );
    ficlSetEnv("max-n",             0x7fffffff);
    ficlSetEnv("max-u",             0xffffffff);
    ficlSetEnvD("max-ud",           0xffffffff, 0xffffffff);
    ficlSetEnv("return-stack-cells",FICL_DEFAULT_STACK);
    ficlSetEnv("stack-cells",       FICL_DEFAULT_STACK);

    /*
    ** DOUBLE word set (partial)
    */
    dictAppendWord(dp, "2constant", twoConstant,    FW_IMMEDIATE);
    dictAppendWord(dp, "2literal",  twoLiteralIm,   FW_IMMEDIATE);
    dictAppendWord(dp, "dnegate",   dnegate,        FW_DEFAULT);


    /*
    ** EXCEPTION word set
    */
    dictAppendWord(dp, "catch",     ficlCatch,      FW_DEFAULT);
    dictAppendWord(dp, "throw",     ficlThrow,      FW_DEFAULT);

    ficlSetEnv("exception",         FICL_TRUE);
    ficlSetEnv("exception-ext",     FICL_FALSE); /* abort" does not comply yet */

    /*
    ** LOCAL and LOCAL EXT
    ** see softcore.c for implementation of locals|
    */
#if FICL_WANT_LOCALS
    pLinkParen = 
    dictAppendWord(dp, "(link)",    linkParen,      FW_COMPILE);
    pUnLinkParen = 
    dictAppendWord(dp, "(unlink)",  unlinkParen,    FW_COMPILE);
    dictAppendWord(dp, "doLocal",   doLocalIm,      FW_COMPIMMED);
    pGetLocalParen =
    dictAppendWord(dp, "(@local)",  getLocalParen,  FW_COMPILE);
    pToLocalParen =
    dictAppendWord(dp, "(toLocal)", toLocalParen,   FW_COMPILE);
    pGetLocal0 =
    dictAppendWord(dp, "(@local0)", getLocal0,      FW_COMPILE);
    pToLocal0 =
    dictAppendWord(dp, "(toLocal0)",toLocal0,       FW_COMPILE);
    pGetLocal1 =
    dictAppendWord(dp, "(@local1)", getLocal1,      FW_COMPILE);
    pToLocal1 =
    dictAppendWord(dp, "(toLocal1)",toLocal1,       FW_COMPILE);
    dictAppendWord(dp, "(local)",   localParen,     FW_COMPILE);

    pGet2LocalParen =
    dictAppendWord(dp, "(@2local)", get2LocalParen, FW_COMPILE);
    pTo2LocalParen =
    dictAppendWord(dp, "(to2Local)",to2LocalParen,  FW_COMPILE);
    dictAppendWord(dp, "(2local)",  twoLocalParen,  FW_COMPILE);

    ficlSetEnv("locals",            FICL_TRUE);
    ficlSetEnv("locals-ext",        FICL_TRUE);
    ficlSetEnv("#locals",           FICL_MAX_LOCALS);
#endif

    /*
    ** Optional MEMORY-ALLOC word set
    */

    dictAppendWord(dp, "allocate",  ansAllocate,    FW_DEFAULT);
    dictAppendWord(dp, "free",      ansFree,        FW_DEFAULT);
    dictAppendWord(dp, "resize",    ansResize,      FW_DEFAULT);
    
    ficlSetEnv("memory-alloc",      FICL_TRUE);
    ficlSetEnv("memory-alloc-ext",  FICL_FALSE);

    /*
    ** optional SEARCH-ORDER word set 
    */
    dictAppendWord(dp, ">search",   searchPush,     FW_DEFAULT);
    dictAppendWord(dp, "search>",   searchPop,      FW_DEFAULT);
    dictAppendWord(dp, "definitions",
                                    definitions,    FW_DEFAULT);
    dictAppendWord(dp, "forth-wordlist",  
                                    forthWordlist,  FW_DEFAULT);
    dictAppendWord(dp, "get-current",  
                                    getCurrent,     FW_DEFAULT);
    dictAppendWord(dp, "get-order", getOrder,       FW_DEFAULT);
    dictAppendWord(dp, "search-wordlist",  
                                    searchWordlist, FW_DEFAULT);
    dictAppendWord(dp, "set-current",  
                                    setCurrent,     FW_DEFAULT);
    dictAppendWord(dp, "set-order", setOrder,       FW_DEFAULT);
    dictAppendWord(dp, "ficl-wordlist", wordlist,   FW_DEFAULT);

    /*
    ** Set SEARCH environment query values
    */
    ficlSetEnv("search-order",      FICL_TRUE);
    ficlSetEnv("search-order-ext",  FICL_TRUE);
    ficlSetEnv("wordlists",         FICL_DEFAULT_VOCS);

    /*
    ** TOOLS and TOOLS EXT
    */
    dictAppendWord(dp, ".s",        displayStack,   FW_DEFAULT);
    dictAppendWord(dp, "bye",       bye,            FW_DEFAULT);
    dictAppendWord(dp, "forget",    forget,         FW_DEFAULT);
    dictAppendWord(dp, "see",       see,            FW_DEFAULT);
    dictAppendWord(dp, "words",     listWords,      FW_DEFAULT);

    /*
    ** Set TOOLS environment query values
    */
    ficlSetEnv("tools",            FICL_TRUE);
    ficlSetEnv("tools-ext",        FICL_FALSE);

    /*
    ** Ficl extras
    */
    dictAppendWord(dp, ".env",      listEnv,        FW_DEFAULT);
    dictAppendWord(dp, ".hash",     dictHashSummary,FW_DEFAULT);
    dictAppendWord(dp, ".ver",      ficlVersion,    FW_DEFAULT);
    dictAppendWord(dp, "-roll",     minusRoll,      FW_DEFAULT);
    dictAppendWord(dp, ">name",     toName,         FW_DEFAULT);
    dictAppendWord(dp, "body>",     fromBody,       FW_DEFAULT);
    dictAppendWord(dp, "compare",   compareString,  FW_DEFAULT);   /* STRING */
    dictAppendWord(dp, "compile-only",
                                    compileOnly,    FW_DEFAULT);
    dictAppendWord(dp, "endif",     endifCoIm,      FW_COMPIMMED);
    dictAppendWord(dp, "forget-wid",forgetWid,      FW_DEFAULT);
	dictAppendWord(dp, "hash",      hash,           FW_DEFAULT);
	dictAppendWord(dp, "number?",   ficlIsNum,      FW_DEFAULT);
    dictAppendWord(dp, "parse-word",parseNoCopy,    FW_DEFAULT);
    dictAppendWord(dp, "sliteral",  sLiteralCoIm,   FW_COMPIMMED); /* STRING */
    dictAppendWord(dp, "wid-set-super", 
                                    setParentWid,   FW_DEFAULT);
    dictAppendWord(dp, "w@",        wFetch,         FW_DEFAULT);
    dictAppendWord(dp, "w!",        wStore,         FW_DEFAULT);
    dictAppendWord(dp, "x.",        hexDot,         FW_DEFAULT);
#if FICL_WANT_USER
    dictAppendWord(dp, "(user)",    userParen,      FW_DEFAULT);
    dictAppendWord(dp, "user",      userVariable,   FW_DEFAULT);
#endif
    /*
    ** internal support words
    */
    pExitParen =
    dictAppendWord(dp, "(exit)",    exitParen,      FW_COMPILE);
    pSemiParen =
    dictAppendWord(dp, "(;)",       semiParen,      FW_COMPILE);
    pLitParen = 
    dictAppendWord(dp, "(literal)", literalParen,   FW_COMPILE);
    pTwoLitParen = 
    dictAppendWord(dp, "(2literal)",twoLitParen,    FW_COMPILE);
    pStringLit =
    dictAppendWord(dp, "(.\")",     stringLit,      FW_COMPILE);
    pIfParen =
    dictAppendWord(dp, "(if)",      ifParen,        FW_COMPILE);
    pBranchParen =
    dictAppendWord(dp, "(branch)",  branchParen,    FW_COMPILE);
    pDoParen =
    dictAppendWord(dp, "(do)",      doParen,        FW_COMPILE);
    pDoesParen =
    dictAppendWord(dp, "(does>)",   doesParen,      FW_COMPILE);
    pQDoParen =
    dictAppendWord(dp, "(?do)",     qDoParen,       FW_COMPILE);
    pLoopParen =
    dictAppendWord(dp, "(loop)",    loopParen,      FW_COMPILE);
    pPLoopParen =
    dictAppendWord(dp, "(+loop)",   plusLoopParen,  FW_COMPILE);
    pInterpret =
    dictAppendWord(dp, "interpret", interpret,      FW_DEFAULT);
    dictAppendWord(dp, "(variable)",variableParen,  FW_COMPILE);
    dictAppendWord(dp, "(constant)",constantParen,  FW_COMPILE);
    dictAppendWord(dp, "exit-inner",ficlExitInner,  FW_DEFAULT);

    assert(dictCellsAvail(dp) > 0);
    return;
}

