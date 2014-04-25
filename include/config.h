#pragma once

#define ERD_RECORD_K15_DPADD_FLOPS 0
#define ERD_RECORD_K15_DPMUL_FLOPS 0
#define ERD_RECORD_K15_DPFMA_FLOPS 0
#define ERD_RECORD_K15_DPDIVSQRT_FLOPS 0
#define ERD_RECORD_K15_DPANY_FLOPS 0
#define ERD_RECORD_WSM_PACKEDDP_UOPS 0
#define ERD_RECORD_WSM_SCALARDP_UOPS 0
#define ERD_RECORD_C2D_PACKEDDP_UOPS 0
#define ERD_RECORD_C2D_SCALARDP_UOPS 0
#define ERD_RECORD_FLOPS \
    (ERD_RECORD_K15_DPADD_FLOPS || \
    ERD_RECORD_K15_DPMUL_FLOPS || \
    ERD_RECORD_K15_DPFMA_FLOPS || \
    ERD_RECORD_K15_DPDIVSQRT_FLOPS || \
    ERD_RECORD_K15_DPANY_FLOPS || \
    ERD_RECORD_C2D_PACKEDDP_UOPS || \
    ERD_RECORD_C2D_SCALARDP_UOPS || \
    ERD_RECORD_WSM_PACKEDDP_UOPS || \
    ERD_RECORD_WSM_SCALARDP_UOPS)
