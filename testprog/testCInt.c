#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <math.h>
#include <sys/time.h>

#include <screening.h>

struct ShellPair {
    int MP;
    int NQ;
    double absValue;
};

int main(int argc, char **argv) {
    double totalcalls = 0;
    int *shellptr;
    int *shellid;
    int *shellrid;
    double *shellvalue;
    struct timeval tv1;
    struct timeval tv2;
    double totalintls = 0;

    if (argc != 3) {
        printf("Usage: %s <basisset> <xyz>\n", argv[0]);
        return 0;
    }
    FILE *ref_data_file = fopen ("ivalues.ref", "r");
    if (ref_data_file == NULL) {
        fprintf(stderr, "ivalues.ref does not exist\n");
        exit (0);
    }
    int errcount; 
    errcount = 0;
    // load basis set   
    BasisSet_t basis;
    CInt_createBasisSet(&basis);
    CInt_loadBasisSet(basis, argv[1], argv[2]);
    int nnz;
    schwartz_screening(basis, &shellptr, &shellid, &shellrid, &shellvalue, &nnz);

    printf("Molecule info:\n");
    printf("  #Atoms\t= %d\n", CInt_getNumAtoms(basis));
    printf("  #Shells\t= %d\n", CInt_getNumShells(basis));
    printf("  #Funcs\t= %d\n", CInt_getNumFuncs(basis));
    printf("  #OccOrb\t= %d\n", CInt_getNumOccOrb(basis));

    ERD_t erd;
    CInt_createERD(basis, &erd, 1);

    printf("Computing integrals ...\n");
    const int ns = CInt_getNumShells(basis);
    int dimMax = CInt_getMaxShellDim(basis);
    double *integrals0 = (double *)malloc(sizeof(double) * dimMax * dimMax * dimMax * dimMax);
    assert(integrals0 != NULL);

    int numShellPairs = 0;
    for (int i = 0; i < shellptr[ns]; i++) {
        const int M = shellrid[i];
        const int N = shellid[i];
        if (M <= N)
            numShellPairs += 1;
    }
    struct ShellPair* shellPairs = (struct ShellPair*)malloc(sizeof(struct ShellPair) * numShellPairs);
    uint32_t shellIndex = 0;
    for (int i = 0; i < shellptr[ns]; i++) {
        const int M = shellrid[i];
        const int N = shellid[i];
        if (M <= N) {
            shellPairs[shellIndex].MP = M;
            shellPairs[shellIndex].NQ = N;
            shellPairs[shellIndex].absValue = fabs(shellvalue[i]);
            shellIndex++;
        }
    }

    double timepass = 0.0;
    for (int i = 0; i < numShellPairs; i++) {
        const int M = shellPairs[i].MP;
        const int N = shellPairs[i].NQ;
        const double absValueMN = shellPairs[i].absValue;
        for (int j = 0; j < numShellPairs; j++) {
            const int P = shellPairs[j].MP;
            const int Q = shellPairs[j].NQ;
            if ((M + N) > (P + Q))
                continue;

            const double absValuePQ = shellPairs[j].absValue;
            if (absValueMN * absValuePQ < TOLSRC * TOLSRC)
                continue;

            totalcalls += 1;
            gettimeofday(&tv1, NULL);

            double *integrals;
            int nints;
            CInt_computeShellQuartet(basis, erd, 0, M, N, P, Q, &integrals, &nints);

            gettimeofday(&tv2, NULL);
            timepass += (tv2.tv_sec - tv1.tv_sec) + (tv2.tv_usec - tv1.tv_usec) * 1.0e-6;
            totalintls = totalintls + nints;
            
            int nints0;
            fread (&nints0, sizeof(int), 1, ref_data_file);
            if (nints0 != 0) {
                fread (integrals0, sizeof(double), nints0, ref_data_file);
            }
            // compare results
            if (nints == 0 && nints0 == 0) {
                continue;
            } else if (nints != 0 && nints0 == 0) {
                for (int k = 0; k < nints; k++) {
                    if (integrals[k] > 1e-10) {
                        printf ("ERROR: %d %d %d %d: %le %le\n",
                            M, N, P, Q, 0.0, integrals[k]);
                        errcount++;
                    }
                }
            } else if (nints == 0 && nints0 != 0) {
                for (int k = 0; k < nints0; k++) {
                    if (integrals0[k] > 1e-10) {
                        printf ("ERROR: %d %d %d %d: %le %le\n",
                            M, N, P, Q, integrals0[k], 0.0);
                        errcount++;
                    }
                }
            } else if (nints == nints0 && nints != 0) {
                for (int k = 0; k < nints0; k++) {
                    if ((fabs(integrals0[k]) < 1.0e-6) ||
                        (fabs(integrals[k]) < 1.0e-6))
                    {
                        if (fabs(integrals0[k] - integrals[k]) > 1e-10) {
                            printf ("1 ERROR: %d %d %d %d: %le %le\n",
                                M, N, P, Q, integrals0[k], integrals[k]);
                            errcount++;
                        }
                    } else {
                        if ((fabs(integrals0[k] - integrals[k])/fabs(integrals0[k]) > 1.0e-6) &&
                            (errcount < 10))
                        {
                            printf ("2 ERROR: %d %d %d %d: %le %le: %le\n",
                                M, N, P, Q, integrals0[k], integrals[k],
                                fabs(integrals0[k] - integrals[k])/fabs(integrals0[k]));
                            errcount++;
                        }

                    }
                }   
            } else {
                printf ("ERROR: nints0 %d nints %d\n", nints0, nints);
            }

            if (errcount > 0) {
                goto end;
            }
        }
    }

    printf("Done\n");
    printf("\n");
    printf("Number of calls: %.6le, Number of integrals: %.6le\n", totalcalls, totalintls);
    printf("Total time: %.4lf secs\n", timepass);
    printf("Average time per call: %.3le us\n", 1000.0 * 1000.0 * timepass / totalcalls);

end:
    free(integrals0);
    CInt_destroyERD(erd);
    CInt_destroyBasisSet(basis);
    free(shellptr);
    free(shellid);
    free(shellvalue);
    free(shellrid);
    
    fclose(ref_data_file);
    return 0;
}
