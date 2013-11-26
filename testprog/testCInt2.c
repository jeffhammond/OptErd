#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>

#include "CInt.h"

extern uint64_t erd__set_ij_kl_pairs_ticks;

int main (int argc, char **argv) {
	BasisSet_t basis;
	ERD_t *erd;
	int ns;
	int i;
	int nthreads;    
	double *totalcalls = 0;
	double *totalnintls = 0;

	struct timeval tv1;
	struct timeval tv2;
	double timepass;

	if (argc != 4) {
		printf ("Usage: %s <basisset> <xyz> <nthreads>\n", argv[0]);
		return -1;
	}
	nthreads = atoi(argv[3]);
	omp_set_num_threads (nthreads);
	
	// load basis set
	CInt_createBasisSet (&basis);
	CInt_loadBasisSet (basis, argv[1], argv[2]);
	
	printf ("Molecule info:\n");
	printf ("  #Atoms\t= %d\n", CInt_getNumAtoms (basis));
	printf ("  #Shells\t= %d\n", CInt_getNumShells (basis));
	printf ("  #Funcs\t= %d\n", CInt_getNumFuncs (basis));
	printf ("  #OccOrb\t= %d\n", CInt_getNumOccOrb (basis));
	printf ("  nthreads\t= %d\n", nthreads);

	erd = (ERD_t *)malloc (sizeof(ERD_t) * nthreads);
	assert (erd != NULL);
	totalcalls = (double *)malloc (sizeof(double) * nthreads * 64);
	assert (totalcalls != NULL);
	totalnintls = (double *)malloc (sizeof(double) * nthreads * 64);
	assert (totalnintls != NULL);
	
	#pragma omp parallel for
	for (i = 0; i < nthreads; i++)
	{
		CInt_createERD (basis, &(erd[i]));
		totalcalls[i * 64] = 0.0;
		totalnintls[i * 64] = 0.0;
	}
	
	printf ("Computing integrals ...\n");   
	ns = CInt_getNumShells (basis);
	timepass = 0.0;
	gettimeofday (&tv1, NULL);
	#pragma omp parallel
	{
		unsigned long long n;
		int tid;
		int M;
		int N;
		int P;
		int Q;
		double *integrals;
		int nints;

		tid = omp_get_thread_num ();
		
		#pragma omp for schedule(dynamic)
		for (n = 0; n < ns*ns*ns*ns; n++)
		{
			M = n/(ns*ns*ns);
			N = (n%(ns*ns*ns))/(ns*ns);
			P = ((n%(ns*ns*ns))%(ns*ns))/ns;
			Q = ((n%(ns*ns*ns))%(ns*ns))%ns;

			if (M > N || P > Q || (M+N) > (P+Q))
				continue;
			
			totalcalls[tid*64]  = totalcalls[tid*64] + 1;
										
			CInt_computeShellQuartet (basis, erd[tid], M, N, P, Q, &integrals, &nints);
									 
			totalnintls[tid*64] = totalnintls[tid*64] + nints;
		}
		
	}

	for (i = 1; i < nthreads; i++)
	{
		totalcalls[0*64] = totalcalls[0*64] + totalcalls[i*64];
		totalnintls[0*64] = totalnintls[0*64] + totalnintls[i*64];
	}
	gettimeofday (&tv2, NULL);
	timepass = (tv2.tv_sec - tv1.tv_sec) +
		(tv2.tv_usec - tv1.tv_usec) / 1000.0 / 1000.0;
	printf ("Done\n");
	printf ("\n");
	printf ("Number of calls: %.6le, Number of integrals: %.6le\n",
		totalcalls[0], totalnintls[0]);
	printf ("Total time: %.4lf secs\n", timepass);
	printf ("Average time per call: %.3le us\n", 1000.0*1000.0*timepass/totalcalls[0]);
	printf ("GigaTicks in erd__set_ij_kl_pairs: %.3lf\n", (double)(erd__set_ij_kl_pairs_ticks) * 1.0e-9);

	for (i = 0; i < nthreads; i++)
	{
		CInt_destroyERD (erd[i]);
	}
	free (erd);
	free (totalcalls);
	free (totalnintls);

	CInt_destroyBasisSet (basis);

	return 0;
}