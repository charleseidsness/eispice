/* This file was automatically generated.  Do not edit! */
#define ParseTOKENTYPE  token_*
#define ParseARG_PDECL , double *solution
void Parse(void *yyp,int yymajor,ParseTOKENTYPE yyminor ParseARG_PDECL);
void ParseFree(void *p,void(*freeProc)(void *));
void *ParseAlloc(void *(*mallocProc)(size_t));
#if !defined(NDEBUG)
void ParseTrace(FILE *TraceFILE,char *zTracePrompt);
#endif
#define ParseARG_STORE yypParser->solution  = solution
#define ParseARG_FETCH  double *solution  = yypParser->solution
#define ParseARG_SDECL  double *solution ;
#define TOKEN_OR                             38
#define TOKEN_AND                            37
#define TOKEN_EQUAL                          36
#define TOKEN_LESSTHAN                       35
#define TOKEN_GREATERTHAN                    34
#define TOKEN_DIFF_VARIABLE                  33
#define TOKEN_IF                             32
#define TOKEN_CONSTANT                       31
#define TOKEN_ZERO_VARIABLE                  30
#define TOKEN_VARIABLE                       29
#define TOKEN_EVAL                           28
#define TOKEN_DIFF                           27
#define TOKEN_RPAREN                         26
#define TOKEN_LPAREN                         25
#define TOKEN_U                              24
#define TOKEN_URAMP                          23
#define TOKEN_TAN                            22
#define TOKEN_SQRT                           21
#define TOKEN_SIN                            20
#define TOKEN_SINH                           19
#define TOKEN_LOG                            18
#define TOKEN_LN                             17
#define TOKEN_EXP                            16
#define TOKEN_COS                            15
#define TOKEN_COSH                           14
#define TOKEN_ATAN                           13
#define TOKEN_ATANH                          12
#define TOKEN_ASIN                           11
#define TOKEN_ASINH                          10
#define TOKEN_ACOS                            9
#define TOKEN_ACOSH                           8
#define TOKEN_ABS                             7
#define TOKEN_NOT                             6
#define TOKEN_POWER                           5
#define TOKEN_TIMES                           4
#define TOKEN_DIVIDE                          3
#define TOKEN_MINUS                           2
#define TOKEN_PLUS                            1
#define INTERFACE 0
