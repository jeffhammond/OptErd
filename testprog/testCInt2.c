#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#ifdef _OPENMP
#include <omp.h>
#endif
#include <math.h>
#include <sys/time.h>

#include <screening.h>
#include <erd_profile.h>

struct ShellPair {
    int MP;
    int NQ;
    double absValue;
};

static uint64_t get_cpu_frequency(void) {
    const uint64_t start_clock = __rdtsc();
    sleep(1);
    const uint64_t end_clock = __rdtsc();
    return end_clock - start_clock;
}

#define USE_CUSTOM_RAND

#ifdef USE_CUSTOM_RAND
static inline uint32_t atomic_rand() {
    static volatile __attribute__((align(64))) uint32_t seed[16] = { 1 };
    
    uint32_t x = seed[0];
    uint32_t y;
    for (;;) {
        /* XorShift RNG */
        y = x ^ (x << 13);
        y = y ^ (y >> 17);
        y = y ^ (y << 5);

        const uint32_t newX = __sync_val_compare_and_swap(&seed[0], x, y);
        if (newX != x)
            x = newX;
        else
            break;
    }
    return y;
}
#else
static inline int atomic_rand() {
    return rand();
}
#endif

int main (int argc, char **argv)
{
    int nnz;
    int *shellptr;
    int *shellid;
    int *shellrid;
    double *shellvalue;
    if (argc != 5) {
        printf ("Usage: %s <basisset> <xyz> <fraction> <nthreads>\n", argv[0]);
        return -1;
    }

    const uint64_t freq = get_cpu_frequency();

    const double fraction = atof(argv[3]);
    assert(fraction > 0.0 && fraction <= 1.0);
    const int nthreads = atoi(argv[4]);
    #ifdef _OPENMP
    omp_set_num_threads(nthreads);
    #else
    assert(nthreads == 1);
    #endif
    
    // load basis set
    BasisSet_t basis;
    CInt_createBasisSet(&basis);
    CInt_loadBasisSet(basis, argv[1], argv[2]);
    schwartz_screening(basis, &shellptr, &shellid, &shellrid, &shellvalue, &nnz);

    printf("Molecule info:\n");
    printf("  #Atoms\t= %d\n", CInt_getNumAtoms(basis));
    printf("  #Shells\t= %d\n", CInt_getNumShells(basis));
    printf("  #Funcs\t= %d\n", CInt_getNumFuncs(basis));
    printf("  #OccOrb\t= %d\n", CInt_getNumOccOrb(basis));
    printf("  nthreads\t= %d\n", nthreads);

    ERD_t erd;
    CInt_createERD(basis, &erd, nthreads);

    double* totalcalls = (double *) malloc(sizeof (double) * nthreads * 64);
    assert(totalcalls != NULL);
    double* totalnintls = (double *) malloc(sizeof (double) * nthreads * 64);
    assert(totalnintls != NULL);

    #pragma omp parallel for
    for (int i = 0; i < nthreads; i++) {
        totalcalls[i * 64] = 0.0;
        totalnintls[i * 64] = 0.0;
    }
    printf("Computing integrals ...\n");

    // reset profiler
    erd_reset_profile();

    //printf ("max memory footprint per thread = %lf KB\n",
    //    CInt_getMaxMemory (erd[0])/1024.0);

    const uint32_t shellCount = CInt_getNumShells(basis);

    srand(1234);
    /* In (fraction) cases rand() returns value not greater than computationThreshold */
    #ifdef USE_CUSTOM_RAND
    const uint32_t computationThresholdMax = 0xFFFFFFFEu;
    const uint32_t computationThreshold = lround(fraction * computationThresholdMax);
    #else
    const int computationThresholdMax = RAND_MAX;
    const int computationThreshold = lround(fraction * computationThresholdMax);
    #endif

    const uint64_t start_clock = __rdtsc();
        
    #pragma omp parallel
    {
        #ifdef _OPENMP
        const int tid = omp_get_thread_num();
        #else
        const int tid = 0;
        #endif

        #pragma omp for schedule(dynamic)
        for (uint32_t shellIndexM = 0; shellIndexM < shellCount; shellIndexM++) {
            const uint32_t shellIndexNStart = shellptr[shellIndexM];
            const uint32_t shellIndexNEnd = shellptr[shellIndexM+1];
            for (uint32_t shellIndexP = 0; shellIndexP != shellCount; shellIndexP++) {
                const uint32_t shellIndexQStart = shellptr[shellIndexP];
                const uint32_t shellIndexQEnd = shellptr[shellIndexP+1];


                /* Prepare indices */
                for (uint32_t shellIndexNOffset = shellIndexNStart; shellIndexNOffset != shellIndexNEnd; shellIndexNOffset++) {
                    const uint32_t shellIndexN = shellid[shellIndexNOffset];
                    if (shellIndexM > shellIndexN)
                        continue;

                    const double shellValueMN = shellvalue[shellIndexNOffset];
                    for (uint32_t shellIndexQOffset = shellIndexQStart; shellIndexQOffset != shellIndexQEnd; shellIndexQOffset++) {
                        const uint32_t shellIndexQ = shellid[shellIndexQOffset];
                        if (shellIndexP > shellIndexQ)
                            continue;

                        if (shellIndexM + shellIndexN > shellIndexP + shellIndexQ)
                            continue;

                        /* Sample random integer. With probability (fraction) process the shell quartet. */
                        if ((computationThreshold == computationThresholdMax) || (atomic_rand() <= computationThreshold)) {
                            double *integrals;
                            int nints;
                            CInt_computeShellQuartet(basis, erd, tid, shellIndexM, shellIndexN, shellIndexP, shellIndexQ, &integrals, &nints);

                            totalcalls[tid * 64] += 1;
                            totalnintls[tid * 64] += nints;
                        }
                    }
                }
            }
        }
    }
    const uint64_t end_clock = __rdtsc();

    for (int i = 1; i < nthreads; i++) {
        totalcalls[0 * 64] = totalcalls[0 * 64] + totalcalls[i * 64];
        totalnintls[0 * 64] = totalnintls[0 * 64] + totalnintls[i * 64];
    } 
    const uint64_t total_ticks = end_clock - start_clock;
    const double timepass = ((double) total_ticks) / freq;

    printf("Done\n");
    printf("\n");
    printf("Number of calls: %.6le, Number of integrals: %.6le\n", totalcalls[0], totalnintls[0]);
    printf("Total GigaTicks: %.3lf, freq = %.3lf GHz\n", (double) (total_ticks) * 1.0e-9, (double)freq/1.0e9);
    printf("Total time: %.4lf secs\n", timepass);
    printf("Average time per call: %.3le us\n", 1000.0 * 1000.0 * timepass / totalcalls[0]);

    // use 1 if thread timing is not required
    erd_print_profile(1);

    CInt_destroyERD(erd);
    free(totalcalls);
    free(totalnintls);
    free(shellptr);
    free(shellid);
    free(shellvalue);
    free(shellrid);

    CInt_destroyBasisSet(basis);

    return 0;
}