/* -*-mode:c-*-
 ****************************************************************************
 *
 * Copyright (c) 1998
 * Jason Evans <jasone@canonware.com>.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer as
 *    the first lines of this file unmodified.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY JASON EVANS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL JASON EVANS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 63 $
 * $Date: 1998-05-01 21:19:49 -0700 (Fri, 01 May 1998) $
 *
 * <<< Description >>>
 *
 * Private header for bhp class (binomial heap).
 *
 ****************************************************************************/

#ifndef _BHP_PRIV_H_
#define _BHP_PRIV_H_

#define bhp_bin_link _CW_NS_CMN(bhp_bin_link)
#define bhp_merge _CW_NS_CMN(bhp_merge)

void bhp_bin_link(cw_bhpi_t * a_root, cw_bhpi_t * a_non_root);
void bhp_merge(cw_bhp_t * a_bhp_o, cw_bhp_t * a_other);
cw_sint32_t bhp_priority_compare(cw_bhpi_t * a_a, cw_bhpi_t * a_b);

#endif /* _BHP_PRIV_H_ */
