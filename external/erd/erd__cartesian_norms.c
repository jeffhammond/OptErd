#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "erd.h"


#pragma offload_attribute(push, target(mic))

/* ------------------------------------------------------------------------ */
/*  OPERATION   : ERD__CARTESIAN_NORMS */
/*  MODULE      : ELECTRON REPULSION INTEGRALS DIRECT */
/*  MODULE-ID   : ERD */
/*  SUBROUTINES : none */
/*  DESCRIPTION : This operation generates all partial cartesian */
/*                normalization factors. The cartesian normalization */
/*                factors are xyz-exponent dependent and are given for */
/*                a specific monomial as: */
/*                                    ______________________ */
/*                     l m n         /       2^(2L) */
/*                    x y z -->     / ----------------------- */
/*                                \/ (2l-1)!!(2m-1)!!(2n-1)!! */
/*                where L = l+m+n. The best way to deal with these */
/*                factors for a complete set of monomials for fixed L */
/*                is to split up each factor into its l-,m- and n- */
/*                component: */
/*                       _______        _______        _______ */
/*                      / 2^(2l)       / 2^(2m)       / 2^(2n) */
/*                     / -------  *   / -------  *   / ------- */
/*                   \/ (2l-1)!!    \/ (2m-1)!!    \/ (2n-1)!! */
/*                and to precalculate each possible partial factor */
/*                only once for the range l = 0,1,...,L, where L denotes */
/*                the maximum shell quantum number that can possibly */
/*                arise during integral evaluation. */
/*                Note, that the first two factors for l = 0 and 1 */
/*                are equal to 1 and 2 and in fact these are the only */
/*                ones that are needed for s- and p-functions. */
/*                  Input: */
/*                       L         =  maximum shell quantum number */
/*                  Output: */
/*                       NORM (I)  =  cartesian norms from I=0,1,...,L */
/* ------------------------------------------------------------------------ */
int erd__cartesian_norms (int l, double *norm)
{
    int i;
    double odd;

    norm[0] = 1.0;
    norm[1] = 2.0;
    odd = 1.0;
    for (i = 2; i <= l; ++i)
    {
        odd += 2.0;
        norm[i] = norm[i - 1] * 2.0 / sqrt (odd);
    }

    return 0;
}

#pragma offload_attribute(pop)
