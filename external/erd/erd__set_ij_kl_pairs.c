#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <yepPredefines.h>

extern double erd__dsqmin_line_segments_ (double *, double *,
										  double *, double *,
										  double *, double *,
										  double *, double *,
										  double *, double *,
										  double *, double *);

#define MIN(a,b)    ((a) > (b) ? (b) : (a))

#define TOL 1e-14

extern double mytime[10];

uint64_t erd__set_ij_kl_pairs_ticks = 0;

static YEP_NOINLINE void set_pairs (int *npgtoa, int *npgtob,
					   int *atomab, int *equalab,
					   int *swaprs, double *rnabsq,
					   double *alphaa, double *alphab,
					   double *ftable, int *mgrid,
					   double *tmax, double *tstep,
					   double *tvstep,
					   int *nij, int *prima, int *primb,
					   double *rho, double qmin, double smaxcd, double rminsq,
					   double *mytable)
{
	int i;
	int i1;
	int i2;
	int j;
	double a;
	double b;
	double ab;
	double p;
	double pinv;
	double pqpinv;
	double t;
	double ssssmx;
	int tgrid;
	double f0;
	double delta;
	double rhoab;
	double ssss1[128] __attribute__((aligned(128)));
	double ssss2[128] __attribute__((aligned(128)));
	double *alphai;
	double *alphaj;
	int *primi;
	int *primj;
	int endi;
	int endj;
		 
	*nij = 0;
	if (*equalab)
	{
		i1 = *npgtoa;
		for (i = 0; i < i1; i++)
		{
			a = alphaa[i];
			i2 = i;
			for (j = 0; j <= i2; j++)
			{
				b = alphab[j];
				p = a + b;
				ab = a * b;
				pinv = 1.0 / p;
				pqpinv = 1.0 / (p + qmin);
				t = rminsq * p * qmin * pqpinv;
				ssssmx = pow (ab, 0.75) * smaxcd * pinv
					* sqrt (pqpinv);
				
			  //  if (t == 0.0)
		   //     {
			//        f0 = 1.0;
			//    }
			f0 = sqrt (M_PI / t) * 0.5;
				if (t <= *tmax)
				{
				   tgrid = (int)(t * *tvstep + .5);
				 //   f0 = mytable[tgrid];
				   delta = tgrid * *tstep - t;
				   f0 = (((((  ftable[(*mgrid + 1) * tgrid + 6] * delta *
							  0.166666666666667
							  + ftable[(*mgrid + 1) * tgrid + 5]) * delta * 0.2
							  + ftable[(*mgrid + 1) * tgrid + 4]) * delta * 0.25
							  + ftable[(*mgrid + 1) * tgrid + 3]) * delta *
							  0.333333333333333
							  + ftable[(*mgrid + 1) * tgrid + 2]) * delta * 0.5
							  + ftable[(*mgrid + 1) * tgrid + 1]) * delta
							  + ftable[(*mgrid + 1) * tgrid + 0];
				}
				if (ssssmx * f0 >= TOL)
				{
					rho[*nij] = 1.0;
					prima[*nij] = i + 1;
					primb[*nij] = j + 1;
					(*nij)++;
				}
			}
		}
	}
	else
	{
		if (*swaprs)
		{
			endi = *npgtob;
			endj = *npgtoa;
			alphai = alphab;
			alphaj = alphaa;
			primi = primb;
			primj = prima;
		}
		else
		{
			endi = *npgtoa;
			endj = *npgtob;
			alphai = alphaa;
			alphaj = alphab;
			primi = prima;
			primj = primb;
		}
		
		if (*atomab)
		{
			for (i = 0; i < endi; i++)
			{
				a = alphai[i];
				#pragma simd
				for (j = 0; j < endj; j++)
				{
					__assume_aligned(alphaj, 64);
					b = alphaj[j];
					p = a + b;
					ab = a * b;
					pinv = 1.0 / p;
					pqpinv = 1.0 / (p + qmin);
					t = rminsq * p * qmin * pqpinv;
					ssssmx = pow (ab, 0.75) * smaxcd * pinv * sqrt (pqpinv);
			   //     f0 = sqrt (M_PI / t) * 0.5;
			   //     if (t == 0.0)
			   //     {
				 //       f0 = 1.0;
			   //     }
			   f0 = sqrt (M_PI / t) * 0.5;
					 if (t <= *tmax)
					{
			   //         tgrid = (int) (t * *tvstep + 0.5);
						tgrid = (int) (t * *tvstep + .5);
			 //  f0 = mytable[tgrid];
						delta = tgrid * *tstep - t;
						f0 = (((((ftable[(*mgrid + 1) * tgrid + 6] * delta *
							  0.166666666666667
							  + ftable[(*mgrid + 1) * tgrid + 5]) * delta * 0.2
							  + ftable[(*mgrid + 1) * tgrid + 4]) * delta * 0.25
							  + ftable[(*mgrid + 1) * tgrid + 3]) * delta *
							  0.333333333333333
							  + ftable[(*mgrid + 1) * tgrid + 2]) * delta * 0.5
							  + ftable[(*mgrid + 1) * tgrid + 1]) * delta
							  + ftable[(*mgrid + 1) * tgrid];
					}
					ssss1[j] = ssssmx * f0;
				}
				for (j = 0; j < endj; j++)
				{                
					if (ssss1[j] >= TOL)
					{
						rho[*nij] = 1.0;
						primi[*nij] = i + 1;
						primj[*nij] = j + 1;
						(*nij)++;
					}
				}
			}
		}
		else
		{
			for (i = 0; i < endi; i++)
			{
				a = alphai[i];
				#pragma simd
				for (j = 0; j < endj; j++)
				{
					__assume_aligned(alphaj, 64);
					b = alphaj[j];
					p = a + b;
					ab = a * b;
					pinv = 1.0 / p;
					pqpinv = 1.0 / (p + qmin);
					rhoab = exp (-ab * *rnabsq * pinv);
					t = rminsq * p * qmin * pqpinv;
					ssssmx = pow (ab, 0.75) * rhoab * smaxcd * pinv *
							sqrt (pqpinv);
				//    f0 = sqrt (M_PI / t) * 0.5;
				 //   if (t == 0.0)
				//    {
			   //         f0 = 1.0;
				//    }
				f0 = sqrt (M_PI / t) * 0.5;
			   if (t <= *tmax)
					{
					 //   tgrid = (int) (t * *tvstep + .5);
					   tgrid = (((int) (t * *tvstep + .5)));
				f0 = mytable[tgrid];
					   delta = tgrid * *tstep - t;
						f0 = (((((ftable[(*mgrid + 1) * tgrid + 6] * delta *
							  0.166666666666667
							  + ftable[(*mgrid + 1) * tgrid + 5]) * delta * 0.2
							  + ftable[(*mgrid + 1) * tgrid + 4]) * delta * 0.25
							  + ftable[(*mgrid + 1) * tgrid + 3]) * delta *
							  0.333333333333333
							  + ftable[(*mgrid + 1) * tgrid + 2]) * delta * 0.5
							  + ftable[(*mgrid + 1) * tgrid + 1]) * delta
							  + ftable[(*mgrid + 1) * tgrid];
					//    printf ("f0 %lf\n", f0);
					}
					ssss1[j] = ssssmx * f0;
					ssss2[j] = rhoab;
				}
				for (j = 0; j < endj; j++)
				{                 
					if (ssss1[j] >= TOL)
					{
						rho[*nij] = ssss2[j];
						primi[*nij] = i + 1;
						primj[*nij] = j + 1;
						(*nij)++;
					}
				}
			}
		}
	}
}

static int erd__set_ij_kl_pairs(int *YEP_RESTRICT npgtoa, int *YEP_RESTRICT npgtob,
	int *YEP_RESTRICT npgtoc, int *YEP_RESTRICT npgtod, int *YEP_RESTRICT npgtoab,
	int *YEP_RESTRICT npgtocd, int *YEP_RESTRICT atomab, int *YEP_RESTRICT atomcd,
	int *YEP_RESTRICT equalab, int *YEP_RESTRICT equalcd, int *YEP_RESTRICT swaprs,
	int *YEP_RESTRICT swaptu, double *YEP_RESTRICT xa, double *YEP_RESTRICT ya,
	double *YEP_RESTRICT za, double *YEP_RESTRICT xb, double *YEP_RESTRICT yb,
	double *YEP_RESTRICT zb, double *YEP_RESTRICT xc, double *YEP_RESTRICT yc,
	double *YEP_RESTRICT zc, double *YEP_RESTRICT xd, double *YEP_RESTRICT yd,
	double *YEP_RESTRICT zd, double *YEP_RESTRICT rnabsq,
	double *YEP_RESTRICT rncdsq, double *YEP_RESTRICT prefact,
	double *YEP_RESTRICT alphaa, double *YEP_RESTRICT alphab,
	double *YEP_RESTRICT alphac, double *YEP_RESTRICT alphad,
	double *YEP_RESTRICT ftable, int *YEP_RESTRICT mgrid, int *YEP_RESTRICT ngrid,
	double *YEP_RESTRICT tmax, double *YEP_RESTRICT tstep,
	double *YEP_RESTRICT tvstep, int *YEP_RESTRICT screen, int *YEP_RESTRICT empty,
	int *YEP_RESTRICT nij, int *YEP_RESTRICT nkl, int *YEP_RESTRICT prima,
	int *YEP_RESTRICT primb, int *YEP_RESTRICT primc, int *YEP_RESTRICT primd,
	double *YEP_RESTRICT rho)
{
	/* System generated locals */
	int i;
	int i1;
	int i2;
	int j;
	int k;
	int l;

	double d1;
	double d2;
	double a;
	double b;
	double c;
	double d;    
	double pmin;
	double qmin;
	double pinv;
	double qinv;
	double abmin;
	double cdmin;   
	double smaxab;
	double smaxcd;
	double rminsq;
	double mytable[64];

	const uint64_t ticks_start = __rdtsc();
	*empty = 0;
#if 0
	for (i = 0; i <= 46; i++) {
		double delta;
		int tgrid;
		double f0;
		 tgrid = (int) (i * *tvstep + .5);
		 delta = tgrid * *tstep - i;
		 f0 = (((((  ftable[(*mgrid + 1) * tgrid + 6] * delta *
					 0.166666666666667
				   + ftable[(*mgrid + 1) * tgrid + 5]) * delta * 0.2
				   + ftable[(*mgrid + 1) * tgrid + 4]) * delta * 0.25
				   + ftable[(*mgrid + 1) * tgrid + 3]) * delta *
				   0.333333333333333
				   + ftable[(*mgrid + 1) * tgrid + 2]) * delta * 0.5
				   + ftable[(*mgrid + 1) * tgrid + 1]) * delta
				   + ftable[(*mgrid + 1) * tgrid + 0];
		 mytable[i] = f0;
	}
	#endif


	// if not screening
	if (!(*screen))
	{
		*nij = 0;
		if (*equalab)
		{
			i1 = *npgtoa;
			for (i = 0; i < i1; i++)
			{
				i2 = i;
				for (j = 0; j <= i2; j++)
				{
					rho[*nij] = 1.00;
					prima[*nij] = i + 1;
					primb[*nij] = j + 1;
					(*nij)++;
				}
			}
		}
		else
		{
			if (*swaprs)
			{
				if (*atomab)
				{
					i1 = *npgtob;
					for (j = 0; j < i1; j++)
					{
						i2 = *npgtoa;
						for (i = 0; i < i2; i++)
						{
							rho[*nij] = 1.0;
							prima[*nij] = i + 1;
							primb[*nij] = j + 1;
							(*nij)++;
						}
					}
				}
				else
				{
					i1 = *npgtob;
					for (j = 0; j < i1; j++)
					{
						b = alphab[j];
						i2 = *npgtoa;
						for (i = 0; i < i2; i++)
						{
							a = alphaa[i];
							rho[*nij] = exp (-a * b * *rnabsq / (a + b));
							prima[*nij] = i + 1;
							primb[*nij] = j + 1;
							(*nij)++;
						}
					}
				}
			}
			else
			{
				if (*atomab)
				{
					i1 = *npgtoa;
					for (i = 0; i < i1; i++)
					{
						i2 = *npgtob;
						for (j = 0; j < i2; j++)
						{                         
							rho[*nij] = 1.0;
							prima[*nij] = i + 1;
							primb[*nij] = j + 1;
							(*nij)++;
						}
					}
				}
				else
				{
					i1 = *npgtoa;
					for (i = 0; i < i1; i++)
					{
						a = alphaa[i];
						i2 = *npgtob;
						for (j = 0; j < i2; j++)
						{
							b = alphab[j];                            
							rho[*nij] = exp (-a * b * *rnabsq / (a + b));
							prima[*nij] = i + 1;
							primb[*nij] = j + 1;
							(*nij)++;
						}
					}
				}
			}
		}
		
		*nkl = 0;
		if (*equalcd)
		{
			i1 = *npgtoc;
			for (k = 0; k < i1; k++)
			{
				i2 = k;
				for (l = 0; l <= i2; l++)
				{      
					rho[*nij + *nkl] = 1.0;
					primc[*nkl] = k + 1;
					primd[*nkl] = l + 1;
					(*nkl)++;
				}
			}
		}
		else
		{
			if (*swaptu)
			{
				if (*atomcd)
				{
					i1 = *npgtod;
					for (l = 0; l < i1; l++)
					{
						i2 = *npgtoc;
						for (k = 0; k < i2; k++)
						{                      
							rho[*nij + *nkl] = 1.0;
							primc[*nkl] = k + 1;
							primd[*nkl] = l + 1;
							(*nkl)++;
						}
					}
				}
				else
				{
					i1 = *npgtod;
					for (l = 0; l < i1; l++)
					{
						d = alphad[l];
						i2 = *npgtoc;
						for (k = 0; k < i2; k++)
						{
							c = alphac[k];
							rho[*nij + *nkl] =
								exp (-c * d * *rncdsq / (c + d));
							primc[*nkl] = k + 1;
							primd[*nkl] = l + 1;
							(*nkl)++;
						}
					}
				}
			}
			else
			{
				if (*atomcd)
				{
					i1 = *npgtoc;
					for (k = 0; k < i1; k++)
					{
						i2 = *npgtod;
						for (l = 0; l < i2; l++)
						{
							rho[*nij + *nkl] = 1.0;
							primc[*nkl] = k + 1;
							primd[*nkl] = l + 1;
							(*nkl)++;
						}
					}
				}
				else
				{
					i1 = *npgtoc;
					for (k = 0; k < i1; k++)
					{
						c = alphac[k];
						i2 = *npgtod;
						for (l = 0; l < i2; l++)
						{
							d = alphad[l];
							rho[*nij + *nkl] =
								exp (-c * d * *rncdsq / (c + d));
							primc[*nkl] = k + 1;
							primd[*nkl] = l + 1;
							(*nkl)++;
						}
					}
				}
			}
		}
	}

	// compute min
	rminsq = erd__dsqmin_line_segments_ (xa, ya, za, xb, yb, zb, xc, yc, zc,
										 xd, yd, zd);
	
	a = alphaa[0];
	i1 = *npgtoa;
	for (i = 1; i < i1; i++)
	{
		d1 = a;
		d2 = alphaa[i];
		a = MIN (d1, d2);
	}
	b = alphab[0];
	i1 = *npgtob;
	for (i = 1; i < i1; i++)
	{
		d1 = b;
		d2 = alphab[i];
		b = MIN (d1, d2);
	}
	c = alphac[0];
	i1 = *npgtoc;
	for (i = 1; i < i1; i++)
	{
		d1 = c;
		d2 = alphac[i];
		c = MIN (d1, d2);
	}
	d = alphad[0];
	i1 = *npgtod;
	for (i = 1; i < i1; i++)
	{
		d1 = d;
		d2 = alphad[i];
		d = MIN (d1, d2);
	}
	pmin = a + b;
	qmin = c + d;
	abmin = a * b;
	cdmin = c * d;
	pinv = 1.0 / pmin;
	qinv = 1.0 / qmin;   
	smaxab = *prefact * pow (abmin, 0.75) *
		exp (-abmin * *rnabsq * pinv) * pinv;
	smaxcd = *prefact * pow (cdmin, 0.75) *
		exp (-cdmin * *rncdsq * qinv) * qinv;

	/* ...perform K2 primitive screening on A,B part. */
	set_pairs (npgtoa, npgtob, atomab, equalab, swaprs, rnabsq,
		alphaa, alphab, ftable, mgrid, tmax, tstep, tvstep,
		nij, prima, primb, rho, qmin, smaxcd, rminsq, mytable);
	if (*nij == 0) {
		*empty = 1;
		const uint64_t ticks_elapsed = __rdtsc() - ticks_start;
		erd__set_ij_kl_pairs_ticks += ticks_elapsed;
		return 0;
	}

	set_pairs (npgtoc, npgtod, atomcd, equalcd, swaptu, rncdsq,
		alphac, alphad, ftable, mgrid, tmax, tstep, tvstep,
		nkl, primc, primd, &(rho[*nij]), pmin, smaxab, rminsq, mytable);
	if (*nkl == 0) {
		*empty = 1;
		const uint64_t ticks_elapsed = __rdtsc() - ticks_start;
		erd__set_ij_kl_pairs_ticks += ticks_elapsed;
		return 0;
	}

	const uint64_t ticks_elapsed = __rdtsc() - ticks_start;
	erd__set_ij_kl_pairs_ticks += ticks_elapsed;
	return 0;
}

int erd__set_ij_kl_pairs_(int *YEP_RESTRICT npgtoa, int *YEP_RESTRICT npgtob,
	int *YEP_RESTRICT npgtoc, int *YEP_RESTRICT npgtod, int *YEP_RESTRICT npgtoab,
	int *YEP_RESTRICT npgtocd, int *YEP_RESTRICT atomab, int *YEP_RESTRICT atomcd,
	int *YEP_RESTRICT equalab, int *YEP_RESTRICT equalcd, int *YEP_RESTRICT swaprs,
	int *YEP_RESTRICT swaptu, double *YEP_RESTRICT xa, double *YEP_RESTRICT ya,
	double *YEP_RESTRICT za, double *YEP_RESTRICT xb, double *YEP_RESTRICT yb,
	double *YEP_RESTRICT zb, double *YEP_RESTRICT xc, double *YEP_RESTRICT yc,
	double *YEP_RESTRICT zc, double *YEP_RESTRICT xd, double *YEP_RESTRICT yd,
	double *YEP_RESTRICT zd, double *YEP_RESTRICT rnabsq,
	double *YEP_RESTRICT rncdsq, double *YEP_RESTRICT prefact,
	double *YEP_RESTRICT alphaa, double *YEP_RESTRICT alphab,
	double *YEP_RESTRICT alphac, double *YEP_RESTRICT alphad,
	double *YEP_RESTRICT ftable, int *YEP_RESTRICT mgrid, int *YEP_RESTRICT ngrid,
	double *YEP_RESTRICT tmax, double *YEP_RESTRICT tstep,
	double *YEP_RESTRICT tvstep, int *YEP_RESTRICT screen, int *YEP_RESTRICT empty,
	int *YEP_RESTRICT nij, int *YEP_RESTRICT nkl, int *YEP_RESTRICT prima,
	int *YEP_RESTRICT primb, int *YEP_RESTRICT primc, int *YEP_RESTRICT primd,
	double *YEP_RESTRICT rho)
{
	erd__set_ij_kl_pairs(npgtoa, npgtob,
		npgtoc, npgtod, npgtoab,
		npgtocd, atomab, atomcd,
		equalab, equalcd, swaprs,
		swaptu, xa, ya,
		za, xb, yb,
		zb, xc, yc,
		zc, xd, yd,
		zd, rnabsq,
		rncdsq, prefact,
		alphaa, alphab,
		alphac, alphad,
		ftable, mgrid, ngrid,
		tmax, tstep,
		tvstep, screen, empty,
		nij, nkl, prima,
		primb, primc, primd,
		rho);
}

