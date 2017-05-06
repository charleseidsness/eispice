/*
 * -- SuperLU routine (version 3.0) --
 * Univ. of California Berkeley, Xerox Palo Alto Research Center,
 * and Lawrence Berkeley National Lab.
 * October 15, 2003
 *
 */
#ifndef __SUPERLU_dSP_DEFS /* allow multiple inclusions */
#define __SUPERLU_dSP_DEFS

/*
 * File name:		dsp_defs.h
 * Purpose:             Sparse matrix types and function prototypes
 * History:
 */

#ifdef _CRAY
#include <fortran.h>
#include <string.h>
#endif

/* Define my integer type int_t */
typedef int int_t; /* default */


/*============================= From slu_Cnames.h ============================*/

/*
 * -- SuperLU routine (version 2.0) --
 * Univ. of California Berkeley, Xerox Palo Alto Research Center,
 * and Lawrence Berkeley National Lab.
 * November 1, 1997
 *
 */
#ifndef __SUPERLU_CNAMES /* allow multiple inclusions */
#define __SUPERLU_CNAMES

/*
 * These macros define how C routines will be called.  ADD_ assumes that
 * they will be called by fortran, which expects C routines to have an
 * underscore postfixed to the name (Suns, and the Intel expect this).
 * NOCHANGE indicates that fortran will be calling, and that it expects
 * the name called by fortran to be identical to that compiled by the C
 * (RS6K's do this).  UPCASE says it expects C routines called by fortran
 * to be in all upcase (CRAY wants this). 
 */

#define ADD_       0
#define ADD__      1
#define NOCHANGE   2
#define UPCASE     3
#define C_CALL     4

#ifdef UpCase
#define F77_CALL_C UPCASE
#endif

#ifdef NoChange
#define F77_CALL_C NOCHANGE
#endif

#ifdef Add_
#define F77_CALL_C ADD_
#endif

#ifdef Add__
#define F77_CALL_C ADD__
#endif

/* Default */
#ifndef F77_CALL_C
#define F77_CALL_C ADD_
#endif


#if (F77_CALL_C == ADD_)
/*
 * These defines set up the naming scheme required to have a fortran 77
 * routine call a C routine
 * No redefinition necessary to have following Fortran to C interface:
 *           FORTRAN CALL               C DECLARATION
 *           call dgemm(...)           void dgemm_(...)
 *
 * This is the default.
 */

#endif

#if (F77_CALL_C == ADD__)
/*
 * These defines set up the naming scheme required to have a fortran 77
 * routine call a C routine 
 * for following Fortran to C interface:
 *           FORTRAN CALL               C DECLARATION
 *           call dgemm(...)           void dgemm__(...)
 */
/* BLAS */
#define sasum_    sasum__
#define isamax_   isamax__
#define scopy_    scopy__
#define sscal_    sscal__
#define sger_     sger__
#define snrm2_    snrm2__
#define ssymv_    ssymv__
#define sdot_     sdot__
#define saxpy_    saxpy__
#define ssyr2_    ssyr2__
#define srot_     srot__
#define sgemv_    sgemv__
#define strsv_    strsv__
#define sgemm_    sgemm__
#define strsm_    strsm__

#define dasum_    dasum__
#define idamax_   idamax__
#define dcopy_    dcopy__
#define dscal_    dscal__
#define dger_     dger__
#define dnrm2_    dnrm2__
#define dsymv_    dsymv__
#define ddot_     ddot__
#define daxpy_    daxpy__
#define dsyr2_    dsyr2__
#define drot_     drot__
#define dgemv_    dgemv__
#define dtrsv_    dtrsv__
#define dgemm_    dgemm__
#define dtrsm_    dtrsm__

#define scasum_   scasum__
#define icamax_   icamax__
#define ccopy_    ccopy__
#define cscal_    cscal__
#define scnrm2_   scnrm2__
#define caxpy_    caxpy__
#define cgemv_    cgemv__
#define ctrsv_    ctrsv__
#define cgemm_    cgemm__
#define ctrsm_    ctrsm__
#define cgerc_    cgerc__
#define chemv_    chemv__
#define cher2_    cher2__

#define dzasum_   dzasum__
#define izamax_   izamax__
#define zcopy_    zcopy__
#define zscal_    zscal__
#define dznrm2_   dznrm2__
#define zaxpy_    zaxpy__
#define zgemv_    zgemv__
#define ztrsv_    ztrsv__
#define zgemm_    zgemm__
#define ztrsm_    ztrsm__
#define zgerc_    zgerc__
#define zhemv_    zhemv__
#define zher2_    zher2__

/* LAPACK */
#define dlamch_   dlamch__
#define slamch_   slamch__
#define xerbla_   xerbla__
#define lsame_    lsame__
#define dlacon_   dlacon__
#define slacon_   slacon__
#define icmax1_   icmax1__
#define scsum1_   scsum1__
#define clacon_   clacon__
#define dzsum1_   dzsum1__
#define izmax1_   izmax1__
#define zlacon_   zlacon__

/* Fortran interface */
#define c_bridge_dgssv_ c_bridge_dgssv__
#define c_fortran_sgssv_ c_fortran_sgssv__
#define c_fortran_dgssv_ c_fortran_dgssv__
#define c_fortran_cgssv_ c_fortran_cgssv__
#define c_fortran_zgssv_ c_fortran_zgssv__
#endif

#if (F77_CALL_C == UPCASE)
/*
 * These defines set up the naming scheme required to have a fortran 77
 * routine call a C routine 
 * following Fortran to C interface:
 *           FORTRAN CALL               C DECLARATION
 *           call dgemm(...)           void DGEMM(...)
 */
/* BLAS */
#define sasum_    SASUM
#define isamax_   ISAMAX
#define scopy_    SCOPY
#define sscal_    SSCAL
#define sger_     SGER
#define snrm2_    SNRM2
#define ssymv_    SSYMV
#define sdot_     SDOT
#define saxpy_    SAXPY
#define ssyr2_    SSYR2
#define srot_     SROT
#define sgemv_    SGEMV
#define strsv_    STRSV
#define sgemm_    SGEMM
#define strsm_    STRSM

#define dasum_    SASUM
#define idamax_   ISAMAX
#define dcopy_    SCOPY
#define dscal_    SSCAL
#define dger_     SGER
#define dnrm2_    SNRM2
#define dsymv_    SSYMV
#define ddot_     SDOT
#define daxpy_    SAXPY
#define dsyr2_    SSYR2
#define drot_     SROT
#define dgemv_    SGEMV
#define dtrsv_    STRSV
#define dgemm_    SGEMM
#define dtrsm_    STRSM

#define scasum_   SCASUM
#define icamax_   ICAMAX
#define ccopy_    CCOPY
#define cscal_    CSCAL
#define scnrm2_   SCNRM2
#define caxpy_    CAXPY
#define cgemv_    CGEMV
#define ctrsv_    CTRSV
#define cgemm_    CGEMM
#define ctrsm_    CTRSM
#define cgerc_    CGERC
#define chemv_    CHEMV
#define cher2_    CHER2

#define dzasum_   SCASUM
#define izamax_   ICAMAX
#define zcopy_    CCOPY
#define zscal_    CSCAL
#define dznrm2_   SCNRM2
#define zaxpy_    CAXPY
#define zgemv_    CGEMV
#define ztrsv_    CTRSV
#define zgemm_    CGEMM
#define ztrsm_    CTRSM
#define zgerc_    CGERC
#define zhemv_    CHEMV
#define zher2_    CHER2

/* LAPACK */
#define dlamch_   DLAMCH
#define slamch_   SLAMCH
#define xerbla_   XERBLA
#define lsame_    LSAME
#define dlacon_   DLACON
#define slacon_   SLACON
#define icmax1_   ICMAX1
#define scsum1_   SCSUM1
#define clacon_   CLACON
#define dzsum1_   DZSUM1
#define izmax1_   IZMAX1
#define zlacon_   ZLACON

/* Fortran interface */
#define c_bridge_dgssv_ C_BRIDGE_DGSSV
#define c_fortran_sgssv_ C_FORTRAN_SGSSV
#define c_fortran_dgssv_ C_FORTRAN_DGSSV
#define c_fortran_cgssv_ C_FORTRAN_CGSSV
#define c_fortran_zgssv_ C_FORTRAN_ZGSSV
#endif

#if (F77_CALL_C == NOCHANGE)
/*
 * These defines set up the naming scheme required to have a fortran 77
 * routine call a C routine 
 * for following Fortran to C interface:
 *           FORTRAN CALL               C DECLARATION
 *           call dgemm(...)           void dgemm(...)
 */
/* BLAS */
#define sasum_    sasum
#define isamax_   isamax
#define scopy_    scopy
#define sscal_    sscal
#define sger_     sger
#define snrm2_    snrm2
#define ssymv_    ssymv
#define sdot_     sdot
#define saxpy_    saxpy
#define ssyr2_    ssyr2
#define srot_     srot
#define sgemv_    sgemv
#define strsv_    strsv
#define sgemm_    sgemm
#define strsm_    strsm

#define dasum_    dasum
#define idamax_   idamax
#define dcopy_    dcopy
#define dscal_    dscal
#define dger_     dger
#define dnrm2_    dnrm2
#define dsymv_    dsymv
#define ddot_     ddot
#define daxpy_    daxpy
#define dsyr2_    dsyr2
#define drot_     drot
#define dgemv_    dgemv
#define dtrsv_    dtrsv
#define dgemm_    dgemm
#define dtrsm_    dtrsm

#define scasum_   scasum
#define icamax_   icamax
#define ccopy_    ccopy
#define cscal_    cscal
#define scnrm2_   scnrm2
#define caxpy_    caxpy
#define cgemv_    cgemv
#define ctrsv_    ctrsv
#define cgemm_    cgemm
#define ctrsm_    ctrsm
#define cgerc_    cgerc
#define chemv_    chemv
#define cher2_    cher2

#define dzasum_   dzasum
#define izamax_   izamax
#define zcopy_    zcopy
#define zscal_    zscal
#define dznrm2_   dznrm2
#define zaxpy_    zaxpy
#define zgemv_    zgemv
#define ztrsv_    ztrsv
#define zgemm_    zgemm
#define ztrsm_    ztrsm
#define zgerc_    zgerc
#define zhemv_    zhemv
#define zher2_    zher2

/* LAPACK */
#define dlamch_   dlamch
#define slamch_   slamch
#define xerbla_   xerbla
#define lsame_    lsame
#define dlacon_   dlacon
#define slacon_   slacon
#define icmax1_   icmax1
#define scsum1_   scsum1
#define clacon_   clacon
#define dzsum1_   dzsum1
#define izmax1_   izmax1
#define zlacon_   zlacon

/* Fortran interface */
#define c_bridge_dgssv_ c_bridge_dgssv
#define c_fortran_sgssv_ c_fortran_sgssv
#define c_fortran_dgssv_ c_fortran_dgssv
#define c_fortran_cgssv_ c_fortran_cgssv
#define c_fortran_zgssv_ c_fortran_zgssv
#endif

#endif /* __SUPERLU_CNAMES */

/*============================= From supermatrix.h ==========================*/

#ifndef __SUPERLU_SUPERMATRIX /* allow multiple inclusions */
#define __SUPERLU_SUPERMATRIX

/********************************************
 * The matrix types are defined as follows. *
 ********************************************/
typedef enum {
    SLU_NC,    /* column-wise, no supernode */
    SLU_NR,    /* row-wize, no supernode */
    SLU_SC,    /* column-wise, supernode */
    SLU_SR,    /* row-wise, supernode */
    SLU_NCP,   /* column-wise, column-permuted, no supernode 
                  (The consecutive columns of nonzeros, after permutation,
		   may not be stored  contiguously.) */
    SLU_DN     /* Fortran style column-wise storage for dense matrix */
} Stype_t;

typedef enum {
    SLU_S,     /* single */
    SLU_D,     /* double */
    SLU_C,     /* single complex */
    SLU_Z      /* double complex */
} Dtype_t;

typedef enum {
    SLU_GE,    /* general */
    SLU_TRLU,  /* lower triangular, unit diagonal */
    SLU_TRUU,  /* upper triangular, unit diagonal */
    SLU_TRL,   /* lower triangular */
    SLU_TRU,   /* upper triangular */
    SLU_SYL,   /* symmetric, store lower half */
    SLU_SYU,   /* symmetric, store upper half */
    SLU_HEL,   /* Hermitian, store lower half */
    SLU_HEU    /* Hermitian, store upper half */
} Mtype_t;

typedef struct {
	Stype_t Stype; /* Storage type: interprets the storage structure 
		   	  pointed to by *Store. */
	Dtype_t Dtype; /* Data type. */
	Mtype_t Mtype; /* Matrix type: describes the mathematical property of 
			  the matrix. */
	int_t  nrow;   /* number of rows */
	int_t  ncol;   /* number of columns */
	void *Store;   /* pointer to the actual storage of the matrix */
} SuperMatrix;

/***********************************************
 * The storage schemes are defined as follows. *
 ***********************************************/

/* Stype == NC (Also known as Harwell-Boeing sparse matrix format) */
typedef struct {
    int_t  nnz;	    /* number of nonzeros in the matrix */
    void   *nzval;  /* pointer to array of nonzero values, packed by column */
    int_t  *rowind; /* pointer to array of row indices of the nonzeros */
    int_t  *colptr; /* pointer to array of beginning of columns in nzval[] 
		       and rowind[]  */
                    /* Note:
		       Zero-based indexing is used;
		       colptr[] has ncol+1 entries, the last one pointing
		       beyond the last column, so that colptr[ncol] = nnz. */
} NCformat;

/* Stype == NR (Also known as row compressed storage (RCS). */
typedef struct {
    int_t nnz;	   /* number of nonzeros in the matrix */
    void  *nzval;  /* pointer to array of nonzero values, packed by row */
    int_t *colind; /* pointer to array of column indices of the nonzeros */
    int_t *rowptr; /* pointer to array of beginning of rows in nzval[] 
                      and colind[]  */
                   /* Note:
		      Zero-based indexing is used;
		      nzval[] and colind[] are of the same length, nnz;
		      rowptr[] has nrow+1 entries, the last one pointing
		      beyond the last column, so that rowptr[nrow] = nnz. */
} NRformat;

/* Stype == SC */
typedef struct {
  int_t  nnz;	     /* number of nonzeros in the matrix */
  int_t  nsuper;     /* number of supernodes, minus 1 */
  void *nzval;       /* pointer to array of nonzero values, packed by column */
  int_t *nzval_colptr;/* pointer to array of beginning of columns in nzval[] */
  int_t *rowind;     /* pointer to array of compressed row indices of 
			rectangular supernodes */
  int_t *rowind_colptr;/* pointer to array of beginning of columns in rowind[] */
  int_t *col_to_sup; /* col_to_sup[j] is the supernode number to which column 
			j belongs; mapping from column to supernode number. */
  int_t *sup_to_col; /* sup_to_col[s] points to the start of the s-th 
			supernode; mapping from supernode number to column.
		        e.g.: col_to_sup: 0 1 2 2 3 3 3 4 4 4 4 4 4 (ncol=12)
		              sup_to_col: 0 1 2 4 7 12           (nsuper=4) */
                     /* Note:
		        Zero-based indexing is used;
		        nzval_colptr[], rowind_colptr[], col_to_sup and
		        sup_to_col[] have ncol+1 entries, the last one
		        pointing beyond the last column.
		        For col_to_sup[], only the first ncol entries are
		        defined. For sup_to_col[], only the first nsuper+2
		        entries are defined. */
} SCformat;

/* Stype == NCP */
typedef struct {
    int_t nnz;	  /* number of nonzeros in the matrix */
    void *nzval;  /* pointer to array of nonzero values, packed by column */
    int_t *rowind;/* pointer to array of row indices of the nonzeros */
		  /* Note: nzval[]/rowind[] always have the same length */
    int_t *colbeg;/* colbeg[j] points to the beginning of column j in nzval[] 
                     and rowind[]  */
    int_t *colend;/* colend[j] points to one past the last element of column
		     j in nzval[] and rowind[]  */
		  /* Note:
		     Zero-based indexing is used;
		     The consecutive columns of the nonzeros may not be 
		     contiguous in storage, because the matrix has been 
		     postmultiplied by a column permutation matrix. */
} NCPformat;

/* Stype == DN */
typedef struct {
    int_t lda;    /* leading dimension */
    void *nzval;  /* array of size lda*ncol to represent a dense matrix */
} DNformat;



/*********************************************************
 * Macros used for easy access of sparse matrix entries. *
 *********************************************************/
#define L_SUB_START(col)     ( Lstore->rowind_colptr[col] )
#define L_SUB(ptr)           ( Lstore->rowind[ptr] )
#define L_NZ_START(col)      ( Lstore->nzval_colptr[col] )
#define L_FST_SUPC(superno)  ( Lstore->sup_to_col[superno] )
#define U_NZ_START(col)      ( Ustore->colptr[col] )
#define U_SUB(ptr)           ( Ustore->rowind[ptr] )


#endif  /* __SUPERLU_SUPERMATRIX */

/*============================= From slu_util.h ============================*/

#ifndef __SUPERLU_UTIL /* allow multiple inclusions */
#define __SUPERLU_UTIL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
#ifndef __STDC__
#include <malloc.h>
#endif
*/
#include <assert.h>

/***********************************************************************
 * Macros
 ***********************************************************************/
#define FIRSTCOL_OF_SNODE(i)	(xsup[i])
/* No of marker arrays used in the symbolic factorization,
   each of size n */
#define NO_MARKER     3
#define NUM_TEMPV(m,w,t,b)  ( SUPERLU_MAX(m, (t + b)*w) )

#ifndef USER_ABORT
#define USER_ABORT(msg) superlu_abort_and_exit(msg)
#endif

#define ABORT(err_msg) \
 { char msg[256];\
   sprintf(msg,"%s at line %d in file %s\n",err_msg,__LINE__, __FILE__);\
   USER_ABORT(msg); }


#ifndef USER_MALLOC
#if 1
#define USER_MALLOC(size) superlu_malloc(size)
#else
/* The following may check out some uninitialized data */
#define USER_MALLOC(size) memset (superlu_malloc(size), '\x0F', size)
#endif
#endif

#define SUPERLU_MALLOC(size) USER_MALLOC(size)

#ifndef USER_FREE
#define USER_FREE(addr) superlu_free(addr)
#endif

#define SUPERLU_FREE(addr) USER_FREE(addr)

#define CHECK_MALLOC(where) {                 \
    extern int superlu_malloc_total;        \
    printf("%s: malloc_total %d Bytes\n",     \
	   where, superlu_malloc_total); \
}

#define SUPERLU_MAX(x, y) 	( (x) > (y) ? (x) : (y) )
#define SUPERLU_MIN(x, y) 	( (x) < (y) ? (x) : (y) )

/***********************************************************************
 * Constants 
 ***********************************************************************/
#define EMPTY	(-1)
/*#define NO	(-1)*/
#define FALSE	0
#define TRUE	1

/***********************************************************************
 * Enumerate types
 ***********************************************************************/
typedef enum {NO, YES}                                          yes_no_t;
typedef enum {DOFACT, SamePattern, SamePattern_SameRowPerm, FACTORED} fact_t;
typedef enum {NOROWPERM, LargeDiag, MY_PERMR}                   rowperm_t;
typedef enum {NATURAL, MMD_ATA, MMD_AT_PLUS_A, COLAMD, MY_PERMC}colperm_t;
typedef enum {NOTRANS, TRANS, CONJ}                             trans_t;
typedef enum {NOEQUIL, ROW, COL, BOTH}                          DiagScale_t;
typedef enum {NOREFINE, SINGLE=1, DOUBLE, EXTRA}                IterRefine_t;
typedef enum {LUSUP, UCOL, LSUB, USUB}                          MemType;
typedef enum {HEAD, TAIL}                                       stack_end_t;
typedef enum {SYSTEM, USER}                                     LU_space_t;

/* 
 * The following enumerate type is used by the statistics variable 
 * to keep track of flop count and time spent at various stages.
 *
 * Note that not all of the fields are disjoint.
 */
typedef enum {
    COLPERM, /* find a column ordering that minimizes fills */
    RELAX,   /* find artificial supernodes */
    ETREE,   /* compute column etree */
    EQUIL,   /* equilibrate the original matrix */
    FACT,    /* perform LU factorization */
    RCOND,   /* estimate reciprocal condition number */
    SOLVE,   /* forward and back solves */
    REFINE,  /* perform iterative refinement */
    TRSV,    /* fraction of FACT spent in xTRSV */
    GEMV,    /* fraction of FACT spent in xGEMV */
    FERR,    /* estimate error bounds after iterative refinement */
    NPHASES  /* total number of phases */
} PhaseType;


/***********************************************************************
 * Type definitions
 ***********************************************************************/
typedef float    flops_t;
typedef unsigned char Logical;

/* 
 *-- This contains the options used to control the solve process.
 *
 * Fact   (fact_t)
 *        Specifies whether or not the factored form of the matrix
 *        A is supplied on entry, and if not, how the matrix A should
 *        be factorizaed.
 *        = DOFACT: The matrix A will be factorized from scratch, and the
 *             factors will be stored in L and U.
 *        = SamePattern: The matrix A will be factorized assuming
 *             that a factorization of a matrix with the same sparsity
 *             pattern was performed prior to this one. Therefore, this
 *             factorization will reuse column permutation vector 
 *             ScalePermstruct->perm_c and the column elimination tree
 *             LUstruct->etree.
 *        = SamePattern_SameRowPerm: The matrix A will be factorized
 *             assuming that a factorization of a matrix with the same
 *             sparsity	pattern and similar numerical values was performed
 *             prior to this one. Therefore, this factorization will reuse
 *             both row and column scaling factors R and C, both row and
 *             column permutation vectors perm_r and perm_c, and the
 *             data structure set up from the previous symbolic factorization.
 *        = FACTORED: On entry, L, U, perm_r and perm_c contain the 
 *              factored form of A. If DiagScale is not NOEQUIL, the matrix
 *              A has been equilibrated with scaling factors R and C.
 *
 * Equil  (yes_no_t)
 *        Specifies whether to equilibrate the system (scale A's row and
 *        columns to have unit norm).
 *
 * ColPerm (colperm_t)
 *        Specifies what type of column permutation to use to reduce fill.
 *        = NATURAL: use the natural ordering 
 *        = MMD_ATA: use minimum degree ordering on structure of A'*A
 *        = MMD_AT_PLUS_A: use minimum degree ordering on structure of A'+A
 *        = COLAMD: use approximate minimum degree column ordering
 *        = MY_PERMC: use the ordering specified in ScalePermstruct->perm_c[]
 *         
 * Trans  (trans_t)
 *        Specifies the form of the system of equations:
 *        = NOTRANS: A * X = B        (No transpose)
 *        = TRANS:   A**T * X = B     (Transpose)
 *        = CONJ:    A**H * X = B     (Transpose)
 *
 * IterRefine (IterRefine_t)
 *        Specifies whether to perform iterative refinement.
 *        = NO: no iterative refinement
 *        = WorkingPrec: perform iterative refinement in working precision
 *        = ExtraPrec: perform iterative refinement in extra precision
 *
 * PrintStat (yes_no_t)
 *        Specifies whether to print the solver's statistics.
 *
 * DiagPivotThresh (double, in [0.0, 1.0]) (only for sequential SuperLU)
 *        Specifies the threshold used for a diagonal entry to be an
 *        acceptable pivot.
 *
 * PivotGrowth (yes_no_t)
 *        Specifies whether to compute the reciprocal pivot growth.
 *
 * ConditionNumber (ues_no_t)
 *        Specifies whether to compute the reciprocal condition number.
 *
 * RowPerm (rowperm_t) (only for SuperLU_DIST)
 *        Specifies whether to permute rows of the original matrix.
 *        = NO: not to permute the rows
 *        = LargeDiag: make the diagonal large relative to the off-diagonal
 *        = MY_PERMR: use the permutation given in ScalePermstruct->perm_r[]
 *           
 * ReplaceTinyPivot (yes_no_t) (only for SuperLU_DIST)
 *        Specifies whether to replace the tiny diagonals by
 *        sqrt(epsilon)*||A|| during LU factorization.
 *
 * SolveInitialized (yes_no_t) (only for SuperLU_DIST)
 *        Specifies whether the initialization has been performed to the
 *        triangular solve.
 *
 * RefineInitialized (yes_no_t) (only for SuperLU_DIST)
 *        Specifies whether the initialization has been performed to the
 *        sparse matrix-vector multiplication routine needed in iterative
 *        refinement.
 */
typedef struct {
    fact_t        Fact;
    yes_no_t      Equil;
    colperm_t     ColPerm;
    trans_t       Trans;
    IterRefine_t  IterRefine;
    yes_no_t      PrintStat;
    yes_no_t      SymmetricMode;
    double        DiagPivotThresh;
    yes_no_t      PivotGrowth;
    yes_no_t      ConditionNumber;
    rowperm_t     RowPerm;
    yes_no_t      ReplaceTinyPivot;
    yes_no_t      SolveInitialized;
    yes_no_t      RefineInitialized;
} superlu_options_t;

typedef struct {
    int     *panel_histo; /* histogram of panel size distribution */
    double  *utime;       /* running time at various phases */
    flops_t *ops;         /* operation count at various phases */
    int     TinyPivots;   /* number of tiny pivots */
    int     RefineSteps;  /* number of iterative refinement steps */
} SuperLUStat_t;


/***********************************************************************
 * Prototypes
 ***********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

extern void    Destroy_SuperMatrix_Store(SuperMatrix *);
extern void    Destroy_CompCol_Matrix(SuperMatrix *);
extern void    Destroy_CompRow_Matrix(SuperMatrix *);
extern void    Destroy_SuperNode_Matrix(SuperMatrix *);
extern void    Destroy_CompCol_Permuted(SuperMatrix *);
extern void    Destroy_Dense_Matrix(SuperMatrix *);
extern void    get_perm_c(int, SuperMatrix *, int *);
extern void    set_default_options(superlu_options_t *options);
extern void    sp_preorder (superlu_options_t *, SuperMatrix*, int*, int*,
			    SuperMatrix*);
extern void    superlu_abort_and_exit(char*);
extern void    *superlu_malloc (size_t);
extern int     *intMalloc (int);
extern int     *intCalloc (int);
extern void    superlu_free (void*);
extern void    SetIWork (int, int, int, int *, int **, int **, int **,
                         int **, int **, int **, int **);
extern int     sp_coletree (int *, int *, int *, int, int, int *);
extern void    relax_snode (const int, int *, const int, int *, int *);
extern void    heap_relax_snode (const int, int *, const int, int *, int *);
extern void    resetrep_col (const int, const int *, int *);
extern int     spcoletree (int *, int *, int *, int, int, int *);
extern int     *TreePostorder (int, int *);
extern double  SuperLU_timer_ ();
extern int     sp_ienv (int);
extern int     lsame_ (char *, char *);
extern int     xerbla_ (char *, int *);
extern void    ifill (int *, int, int);
extern void    snode_profile (int, int *);
extern void    super_stats (int, int *);
extern void    PrintSumm (char *, int, int, int);
extern void    StatInit(SuperLUStat_t *);
extern void    StatPrint (SuperLUStat_t *);
extern void    StatFree(SuperLUStat_t *);
extern void    print_panel_seg(int, int, int, int, int *, int *);
extern void    check_repfnz(int, int, int, int *);

#ifdef __cplusplus
  }
#endif

#endif /* __SUPERLU_UTIL */

/*============================= From slu_ddefs.h ============================*/

/*
 * Global data structures used in LU factorization -
 * 
 *   nsuper: #supernodes = nsuper + 1, numbered [0, nsuper].
 *   (xsup,supno): supno[i] is the supernode no to which i belongs;
 *	xsup(s) points to the beginning of the s-th supernode.
 *	e.g.   supno 0 1 2 2 3 3 3 4 4 4 4 4   (n=12)
 *	        xsup 0 1 2 4 7 12
 *	Note: dfs will be performed on supernode rep. relative to the new 
 *	      row pivoting ordering
 *
 *   (xlsub,lsub): lsub[*] contains the compressed subscript of
 *	rectangular supernodes; xlsub[j] points to the starting
 *	location of the j-th column in lsub[*]. Note that xlsub 
 *	is indexed by column.
 *	Storage: original row subscripts
 *
 *      During the course of sparse LU factorization, we also use
 *	(xlsub,lsub) for the purpose of symmetric pruning. For each
 *	supernode {s,s+1,...,t=s+r} with first column s and last
 *	column t, the subscript set
 *		lsub[j], j=xlsub[s], .., xlsub[s+1]-1
 *	is the structure of column s (i.e. structure of this supernode).
 *	It is used for the storage of numerical values.
 *	Furthermore,
 *		lsub[j], j=xlsub[t], .., xlsub[t+1]-1
 *	is the structure of the last column t of this supernode.
 *	It is for the purpose of symmetric pruning. Therefore, the
 *	structural subscripts can be rearranged without making physical
 *	interchanges among the numerical values.
 *
 *	However, if the supernode has only one column, then we
 *	only keep one set of subscripts. For any subscript interchange
 *	performed, similar interchange must be done on the numerical
 *	values.
 *
 *	The last column structures (for pruning) will be removed
 *	after the numercial LU factorization phase.
 *
 *   (xlusup,lusup): lusup[*] contains the numerical values of the
 *	rectangular supernodes; xlusup[j] points to the starting
 *	location of the j-th column in storage vector lusup[*]
 *	Note: xlusup is indexed by column.
 *	Each rectangular supernode is stored by column-major
 *	scheme, consistent with Fortran 2-dim array storage.
 *
 *   (xusub,ucol,usub): ucol[*] stores the numerical values of
 *	U-columns outside the rectangular supernodes. The row
 *	subscript of nonzero ucol[k] is stored in usub[k].
 *	xusub[i] points to the starting location of column i in ucol.
 *	Storage: new row subscripts; that is subscripts of PA.
 */
typedef struct {
    int     *xsup;    /* supernode and column mapping */
    int     *supno;   
    int     *lsub;    /* compressed L subscripts */
    int	    *xlsub;
    double  *lusup;   /* L supernodes */
    int     *xlusup;
    double  *ucol;    /* U columns */
    int     *usub;
    int	    *xusub;
    int     nzlmax;   /* current max size of lsub */
    int     nzumax;   /*    "    "    "      ucol */
    int     nzlumax;  /*    "    "    "     lusup */
    int     n;        /* number of columns in the matrix */
    LU_space_t MemModel; /* 0 - system malloc'd; 1 - user provided */
} GlobalLU_t;

typedef struct {
    float for_lu;
    float total_needed;
    int   expansions;
} mem_usage_t;

#ifdef __cplusplus
extern "C" {
#endif

/* Driver routines */
extern void
dgssv(superlu_options_t *, SuperMatrix *, int *, int *, SuperMatrix *,
      SuperMatrix *, SuperMatrix *, SuperLUStat_t *, int *);
extern void
dgssvx(superlu_options_t *, SuperMatrix *, int *, int *, int *,
       char *, double *, double *, SuperMatrix *, SuperMatrix *,
       void *, int, SuperMatrix *, SuperMatrix *,
       double *, double *, double *, double *,
       mem_usage_t *, SuperLUStat_t *, int *);

/* Supernodal LU factor related */
extern void
dCreate_CompCol_Matrix(SuperMatrix *, int, int, int, double *,
		       int *, int *, Stype_t, Dtype_t, Mtype_t);
extern void
dCreate_CompRow_Matrix(SuperMatrix *, int, int, int, double *,
		       int *, int *, Stype_t, Dtype_t, Mtype_t);
extern void
dCopy_CompCol_Matrix(SuperMatrix *, SuperMatrix *);
extern void
dCreate_Dense_Matrix(SuperMatrix *, int, int, double *, int,
		     Stype_t, Dtype_t, Mtype_t);
extern void
dCreate_SuperNode_Matrix(SuperMatrix *, int, int, int, double *, 
		         int *, int *, int *, int *, int *,
			 Stype_t, Dtype_t, Mtype_t);
extern void
dCopy_Dense_Matrix(int, int, double *, int, double *, int);

extern void    countnz (const int, int *, int *, int *, GlobalLU_t *);
extern void    fixupL (const int, const int *, GlobalLU_t *);

extern void    dallocateA (int, int, double **, int **, int **);
extern void    dgstrf (superlu_options_t*, SuperMatrix*, double, 
                       int, int, int*, void *, int, int *, int *, 
                       SuperMatrix *, SuperMatrix *, SuperLUStat_t*, int *);
extern int     dsnode_dfs (const int, const int, const int *, const int *,
			     const int *, int *, int *, GlobalLU_t *);
extern int     dsnode_bmod (const int, const int, const int, double *,
                              double *, GlobalLU_t *, SuperLUStat_t*);
extern void    dpanel_dfs (const int, const int, const int, SuperMatrix *,
			   int *, int *, double *, int *, int *, int *,
			   int *, int *, int *, int *, GlobalLU_t *);
extern void    dpanel_bmod (const int, const int, const int, const int,
                           double *, double *, int *, int *,
			   GlobalLU_t *, SuperLUStat_t*);
extern int     dcolumn_dfs (const int, const int, int *, int *, int *, int *,
			   int *, int *, int *, int *, int *, GlobalLU_t *);
extern int     dcolumn_bmod (const int, const int, double *,
			   double *, int *, int *, int,
                           GlobalLU_t *, SuperLUStat_t*);
extern int     dcopy_to_ucol (int, int, int *, int *, int *,
                              double *, GlobalLU_t *);         
extern int     dpivotL (const int, const double, int *, int *, 
                         int *, int *, int *, GlobalLU_t *, SuperLUStat_t*);
extern void    dpruneL (const int, const int *, const int, const int,
			  const int *, const int *, int *, GlobalLU_t *);
extern void    dreadmt (int *, int *, int *, double **, int **, int **);
extern void    dGenXtrue (int, int, double *, int);
extern void    dFillRHS (trans_t, int, double *, int, SuperMatrix *,
			  SuperMatrix *);
extern void    dgstrs (trans_t, SuperMatrix *, SuperMatrix *, int *, int *,
                        SuperMatrix *, SuperLUStat_t*, int *);


/* Driver related */

extern void    dgsequ (SuperMatrix *, double *, double *, double *,
			double *, double *, int *);
extern void    dlaqgs (SuperMatrix *, double *, double *, double,
                        double, double, char *);
extern void    dgscon (char *, SuperMatrix *, SuperMatrix *, 
		         double, double *, SuperLUStat_t*, int *);
extern double   dPivotGrowth(int, SuperMatrix *, int *, 
                            SuperMatrix *, SuperMatrix *);
extern void    dgsrfs (trans_t, SuperMatrix *, SuperMatrix *,
                       SuperMatrix *, int *, int *, char *, double *, 
                       double *, SuperMatrix *, SuperMatrix *,
                       double *, double *, SuperLUStat_t*, int *);

extern int     sp_dtrsv (char *, char *, char *, SuperMatrix *,
			SuperMatrix *, double *, SuperLUStat_t*, int *);
extern int     sp_dgemv (char *, double, SuperMatrix *, double *,
			int, double, double *, int);

extern int     sp_dgemm (char *, char *, int, int, int, double,
			SuperMatrix *, double *, int, double, 
			double *, int);

/* Memory-related */
extern int     dLUMemInit (fact_t, void *, int, int, int, int, int,
			     SuperMatrix *, SuperMatrix *,
			     GlobalLU_t *, int **, double **);
extern void    dSetRWork (int, int, double *, double **, double **);
extern void    dLUWorkFree (int *, double *, GlobalLU_t *);
extern int     dLUMemXpand (int, int, MemType, int *, GlobalLU_t *);

extern double  *doubleMalloc(int);
extern double  *doubleCalloc(int);
extern int     dmemory_usage(const int, const int, const int, const int);
extern int     dQuerySpace (SuperMatrix *, SuperMatrix *, mem_usage_t *);

/* Auxiliary routines */
extern void    dreadhb(int *, int *, int *, double **, int **, int **);
extern void    dCompRow_to_CompCol(int, int, int, double*, int*, int*,
		                   double **, int **, int **);
extern void    dfill (double *, int, double);
extern void    dinf_norm_error (int, SuperMatrix *, double *);
extern void    PrintPerf (SuperMatrix *, SuperMatrix *, mem_usage_t *,
			 double, double, double *, double *, char *);

/* Routines for debugging */
extern void    dPrint_CompCol_Matrix(char *, SuperMatrix *);
extern void    dPrint_SuperNode_Matrix(char *, SuperMatrix *);
extern void    dPrint_Dense_Matrix(char *, SuperMatrix *);
extern void    print_lu_col(char *, int, int, int *, GlobalLU_t *);
extern void    check_tempv(int, double *);

#ifdef __cplusplus
  }
#endif

#endif /* __SUPERLU_dSP_DEFS */

