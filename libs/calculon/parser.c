/* Driver template for the LEMON parser generator.
** The author disclaims copyright to this source code.
*/
/* First off, code is include which follows the "include" declaration
** in the input file. */
#include <stdio.h>
#line 1 "parser.lem"

/*
 * Copyright (C) 2006 Cooper Street Innovations Inc.
 *	Charles Eidsness    <charles@cooper-street.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */


	#include <math.h>
	#include <log.h>
	#include "tokenizer.h"
	#include "parser.h"

	#define Ln(x)	log(fabs(x))
	#define Div(x,y,m) \
		((fabs(y) > (m))?((x)/(y)):(((y) > 0)?(x)/(m):(x)/(-(m))))

	typedef struct {
		double f;	/* value of function */
		double d;	/* value of derivative */
	} derivData;
#line 46 "parser.c"
#include "parser.h"
/* Next is all token values, in a form suitable for use by makeheaders.
** This section will be null unless lemon is run with the -m switch.
*/
/*
** These constants (all generated automatically by the parser generator)
** specify the various kinds of tokens (terminals) that the parser
** understands.
**
** Each symbol here is a terminal symbol in the grammar.
*/
#if INTERFACE
#define TOKEN_PLUS                            1
#define TOKEN_MINUS                           2
#define TOKEN_DIVIDE                          3
#define TOKEN_TIMES                           4
#define TOKEN_POWER                           5
#define TOKEN_NOT                             6
#define TOKEN_ABS                             7
#define TOKEN_ACOSH                           8
#define TOKEN_ACOS                            9
#define TOKEN_ASINH                          10
#define TOKEN_ASIN                           11
#define TOKEN_ATANH                          12
#define TOKEN_ATAN                           13
#define TOKEN_COSH                           14
#define TOKEN_COS                            15
#define TOKEN_EXP                            16
#define TOKEN_LN                             17
#define TOKEN_LOG                            18
#define TOKEN_SINH                           19
#define TOKEN_SIN                            20
#define TOKEN_SQRT                           21
#define TOKEN_TAN                            22
#define TOKEN_URAMP                          23
#define TOKEN_U                              24
#define TOKEN_LPAREN                         25
#define TOKEN_RPAREN                         26
#define TOKEN_DIFF                           27
#define TOKEN_EVAL                           28
#define TOKEN_VARIABLE                       29
#define TOKEN_ZERO_VARIABLE                  30
#define TOKEN_CONSTANT                       31
#define TOKEN_IF                             32
#define TOKEN_DIFF_VARIABLE                  33
#define TOKEN_GREATERTHAN                    34
#define TOKEN_LESSTHAN                       35
#define TOKEN_EQUAL                          36
#define TOKEN_AND                            37
#define TOKEN_OR                             38
#endif
/* Make sure the INTERFACE macro is defined.
*/
#ifndef INTERFACE
# define INTERFACE 1
#endif
/* The next thing included is series of defines which control
** various aspects of the generated parser.
**    YYCODETYPE         is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 terminals
**                       and nonterminals.  "int" is used otherwise.
**    YYNOCODE           is a number of type YYCODETYPE which corresponds
**                       to no legal terminal or nonterminal number.  This
**                       number is used to fill in empty slots of the hash
**                       table.
**    YYFALLBACK         If defined, this indicates that one or more tokens
**                       have fall-back values which should be used if the
**                       original value of the token will not parse.
**    YYACTIONTYPE       is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 rules and
**                       states combined.  "int" is used otherwise.
**    ParseTOKENTYPE     is the data type used for minor tokens given
**                       directly to the parser from the tokenizer.
**    YYMINORTYPE        is the data type used for all minor tokens.
**                       This is typically a union of many types, one of
**                       which is ParseTOKENTYPE.  The entry in the union
**                       for base tokens is called "yy0".
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.  If
**                       zero the stack is dynamically sized using realloc()
**    ParseARG_SDECL     A static variable declaration for the %extra_argument
**    ParseARG_PDECL     A parameter declaration for the %extra_argument
**    ParseARG_STORE     Code to store %extra_argument into yypParser
**    ParseARG_FETCH     Code to extract %extra_argument from yypParser
**    YYNSTATE           the combined number of states.
**    YYNRULE            the number of rules in the grammar
**    YYERRORSYMBOL      is the code number of the error symbol.  If not
**                       defined, then do no error processing.
*/
#define YYCODETYPE unsigned char
#define YYNOCODE 47
#define YYACTIONTYPE unsigned short int
#if INTERFACE
#define ParseTOKENTYPE  token_*
#endif
typedef union {
  ParseTOKENTYPE yy0;
  derivData yy41;
  double yy76;
  int yy93;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#if INTERFACE
#define ParseARG_SDECL  double *solution ;
#define ParseARG_PDECL , double *solution
#define ParseARG_FETCH  double *solution  = yypParser->solution
#define ParseARG_STORE yypParser->solution  = solution
#endif
#define YYNSTATE 227
#define YYNRULE 85
#define YYERRORSYMBOL 39
#define YYERRSYMDT yy93
#define YY_NO_ACTION      (YYNSTATE+YYNRULE+2)
#define YY_ACCEPT_ACTION  (YYNSTATE+YYNRULE+1)
#define YY_ERROR_ACTION   (YYNSTATE+YYNRULE)

/* Next are that tables used to determine what action to take based on the
** current state and lookahead token.  These tables are used to implement
** functions that take a state number and lookahead value and return an
** action integer.
**
** Suppose the action integer is N.  Then the action is determined as
** follows
**
**   0 <= N < YYNSTATE                  Shift N.  That is, push the lookahead
**                                      token onto the stack and goto state N.
**
**   YYNSTATE <= N < YYNSTATE+YYNRULE   Reduce by rule N-YYNSTATE.
**
**   N == YYNSTATE+YYNRULE              A syntax error has occurred.
**
**   N == YYNSTATE+YYNRULE+1            The parser accepts its input.
**
**   N == YYNSTATE+YYNRULE+2            No such action.  Denotes unused
**                                      slots in the yy_action[] table.
**
** The action table is constructed as a single large table named yy_action[].
** Given state S and lookahead X, the action is computed as
**
**      yy_action[ yy_shift_ofst[S] + X ]
**
** If the index value yy_shift_ofst[S]+X is out of range or if the value
** yy_lookahead[yy_shift_ofst[S]+X] is not equal to X or if yy_shift_ofst[S]
** is equal to YY_SHIFT_USE_DFLT, it means that the action is not in the table
** and that yy_default[S] should be used instead.
**
** The formula above is for computing the action when the lookahead is
** a terminal symbol.  If the lookahead is a non-terminal (as occurs after
** a reduce action) then the yy_reduce_ofst[] array is used in place of
** the yy_shift_ofst[] array and YY_REDUCE_USE_DFLT is used in place of
** YY_SHIFT_USE_DFLT.
**
** The following are the tables generated in this section:
**
**  yy_action[]        A single table containing all actions.
**  yy_lookahead[]     A table containing the lookahead for each entry in
**                     yy_action.  Used to detect hash collisions.
**  yy_shift_ofst[]    For each state, the offset into yy_action for
**                     shifting terminals.
**  yy_reduce_ofst[]   For each state, the offset into yy_action for
**                     shifting non-terminals after a reduce.
**  yy_default[]       Default action for each state.
*/
static const YYACTIONTYPE yy_action[] = {
 /*     0 */    51,  130,   54,   52,   55,   56,   57,   58,   59,   60,
 /*    10 */    61,   62,   63,   64,   65,   66,   67,   68,   69,   70,
 /*    20 */    71,   72,   73,   49,   53,   16,   11,  172,  173,  174,
 /*    30 */     6,   22,   20,   81,  170,  132,    4,   26,   27,   28,
 /*    40 */    29,   30,   31,   32,   33,   34,   35,   36,   37,   38,
 /*    50 */    39,   40,   41,   42,   43,    2,   79,  163,  123,  200,
 /*    60 */   125,  201,    1,  199,   22,   20,  129,   24,   21,   25,
 /*    70 */    26,   27,   28,   29,   30,   31,   32,   33,   34,   35,
 /*    80 */    36,   37,   38,   39,   40,   41,   42,   43,   18,   23,
 /*    90 */    81,  166,  200,  139,  201,    1,  199,   22,   20,   44,
 /*   100 */    81,  222,  102,   26,   27,   28,   29,   30,   31,   32,
 /*   110 */    33,   34,   35,   36,   37,   38,   39,   40,   41,   42,
 /*   120 */    43,   18,   81,  169,  138,  200,  158,  201,    1,  199,
 /*   130 */    51,  145,   45,  159,    9,   56,   57,   58,   59,   60,
 /*   140 */    61,   62,   63,   64,   65,   66,   67,   68,   69,   70,
 /*   150 */    71,   72,   73,    7,   80,  157,   51,  172,  173,  174,
 /*   160 */     6,   56,   57,   58,   59,   60,   61,   62,   63,   64,
 /*   170 */    65,   66,   67,   68,   69,   70,   71,   72,   73,   49,
 /*   180 */    78,  150,  103,  172,  173,  174,    6,   51,   80,  153,
 /*   190 */    74,  137,   56,   57,   58,   59,   60,   61,   62,   63,
 /*   200 */    64,   65,   66,   67,   68,   69,   70,   71,   72,   73,
 /*   210 */    49,   80,  195,  143,  172,  173,  174,    6,   22,   20,
 /*   220 */   160,   75,   80,  156,   26,   27,   28,   29,   30,   31,
 /*   230 */    32,   33,   34,   35,   36,   37,   38,   39,   40,   41,
 /*   240 */    42,   43,   18,   80,  226,  104,  200,  105,  201,    1,
 /*   250 */   199,   51,  151,  154,  106,   55,   56,   57,   58,   59,
 /*   260 */    60,   61,   62,   63,   64,   65,   66,   67,   68,   69,
 /*   270 */    70,   71,   72,   73,   49,   53,  164,  167,  172,  173,
 /*   280 */   174,    6,   50,   48,   54,   52,   55,  149,   17,   19,
 /*   290 */    24,   21,   25,  162,   50,   48,   54,   52,   55,  149,
 /*   300 */    17,   19,   24,   21,   25,  162,   53,  175,  107,  122,
 /*   310 */    25,  171,   23,  202,  313,   14,   15,  148,   53,  108,
 /*   320 */   109,   12,   13,  161,   23,  110,  111,   14,   15,  148,
 /*   330 */    23,  112,  113,   12,   13,  161,   50,   48,   54,   52,
 /*   340 */    55,   50,   48,   54,   52,   55,   50,   48,   54,   52,
 /*   350 */    55,   50,   48,   54,   52,   55,  114,  115,  116,  117,
 /*   360 */    53,  175,  118,  119,  120,   53,  176,  121,  131,  133,
 /*   370 */    53,  177,  134,  135,   82,   53,  178,   50,   48,   54,
 /*   380 */    52,   55,   50,   48,   54,   52,   55,   50,   48,   54,
 /*   390 */    52,   55,   50,   48,   54,   52,   55,  136,  146,  140,
 /*   400 */    83,   53,  179,  141,  147,   84,   53,  180,   85,   86,
 /*   410 */    87,   53,  181,   88,   89,   90,   53,  182,   50,   48,
 /*   420 */    54,   52,   55,   50,   48,   54,   52,   55,   50,   48,
 /*   430 */    54,   52,   55,   50,   48,   54,   52,   55,   91,   92,
 /*   440 */    93,   94,   53,  183,   95,   96,   97,   53,  184,   98,
 /*   450 */    99,  100,   53,  185,  101,  124,  126,   53,  186,   50,
 /*   460 */    48,   54,   52,   55,   50,   48,   54,   52,   55,   50,
 /*   470 */    48,   54,   52,   55,   50,   48,   54,   52,   55,  127,
 /*   480 */   128,   53,   76,   53,  187,   77,  142,  152,   53,  188,
 /*   490 */     8,  155,  196,   53,  189,   10,  165,  197,   53,  190,
 /*   500 */    50,   48,   54,   52,   55,   50,   48,   54,   52,   55,
 /*   510 */    50,   48,   54,   52,   55,   50,   48,   54,   52,   55,
 /*   520 */   198,   23,   46,   47,   53,  191,  144,    3,  168,   53,
 /*   530 */   192,  223,    5,  314,   53,  193,  224,  225,  314,   53,
 /*   540 */   194,   17,   19,   24,   21,   25,   17,   19,   24,   21,
 /*   550 */    25,   17,   19,   24,   21,   25,   17,   19,   24,   21,
 /*   560 */    25,  314,  314,  314,  314,   23,  202,  314,  314,  314,
 /*   570 */    23,  203,  314,  314,  314,   23,  204,  314,  314,  314,
 /*   580 */    23,  205,   17,   19,   24,   21,   25,   17,   19,   24,
 /*   590 */    21,   25,   17,   19,   24,   21,   25,   17,   19,   24,
 /*   600 */    21,   25,  314,  314,  314,  314,   23,  206,  314,  314,
 /*   610 */   314,   23,  207,  314,  314,  314,   23,  208,  314,  314,
 /*   620 */   314,   23,  209,   17,   19,   24,   21,   25,   17,   19,
 /*   630 */    24,   21,   25,   17,   19,   24,   21,   25,   17,   19,
 /*   640 */    24,   21,   25,  314,  314,  314,  314,   23,  210,  314,
 /*   650 */   314,  314,   23,  211,  314,  314,  314,   23,  212,  314,
 /*   660 */   314,  314,   23,  213,   17,   19,   24,   21,   25,   17,
 /*   670 */    19,   24,   21,   25,   17,   19,   24,   21,   25,   17,
 /*   680 */    19,   24,   21,   25,  314,  314,  314,  314,   23,  214,
 /*   690 */   314,  314,  314,   23,  215,  314,  314,  314,   23,  216,
 /*   700 */   314,  314,  314,   23,  217,   17,   19,   24,   21,   25,
 /*   710 */    17,   19,   24,   21,   25,   17,   19,   24,   21,   25,
 /*   720 */    17,   19,   24,   21,   25,  314,  314,  314,  314,   23,
 /*   730 */   218,  314,  314,  314,   23,  219,  314,  314,  314,   23,
 /*   740 */   220,  314,  314,  314,   23,  221,   50,   48,   54,   52,
 /*   750 */    55,   17,   19,   24,   21,   25,  314,  314,  314,  314,
 /*   760 */   314,  314,  314,  314,  314,  314,  314,  314,  314,  314,
 /*   770 */    53,  314,  314,  314,  314,   23,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */     2,   43,    3,    4,    5,    7,    8,    9,   10,   11,
 /*    10 */    12,   13,   14,   15,   16,   17,   18,   19,   20,   21,
 /*    20 */    22,   23,   24,   25,   25,   27,   28,   29,   30,   31,
 /*    30 */    32,    1,    2,   43,   44,   43,    6,    7,    8,    9,
 /*    40 */    10,   11,   12,   13,   14,   15,   16,   17,   18,   19,
 /*    50 */    20,   21,   22,   23,   24,   25,   43,   44,   40,   29,
 /*    60 */    40,   31,   32,   33,    1,    2,   43,    3,    4,    5,
 /*    70 */     7,    8,    9,   10,   11,   12,   13,   14,   15,   16,
 /*    80 */    17,   18,   19,   20,   21,   22,   23,   24,   25,   25,
 /*    90 */    43,   44,   29,   43,   31,   32,   33,    1,    2,   36,
 /*   100 */    43,   44,   43,    7,    8,    9,   10,   11,   12,   13,
 /*   110 */    14,   15,   16,   17,   18,   19,   20,   21,   22,   23,
 /*   120 */    24,   25,   43,   44,   43,   29,   43,   31,   32,   33,
 /*   130 */     2,   43,   36,   43,    6,    7,    8,    9,   10,   11,
 /*   140 */    12,   13,   14,   15,   16,   17,   18,   19,   20,   21,
 /*   150 */    22,   23,   24,   25,   40,   41,    2,   29,   30,   31,
 /*   160 */    32,    7,    8,    9,   10,   11,   12,   13,   14,   15,
 /*   170 */    16,   17,   18,   19,   20,   21,   22,   23,   24,   25,
 /*   180 */    40,   41,   43,   29,   30,   31,   32,    2,   40,   41,
 /*   190 */    36,   40,    7,    8,    9,   10,   11,   12,   13,   14,
 /*   200 */    15,   16,   17,   18,   19,   20,   21,   22,   23,   24,
 /*   210 */    25,   40,   41,   43,   29,   30,   31,   32,    1,    2,
 /*   220 */    43,   36,   40,   41,    7,    8,    9,   10,   11,   12,
 /*   230 */    13,   14,   15,   16,   17,   18,   19,   20,   21,   22,
 /*   240 */    23,   24,   25,   40,   41,   43,   29,   43,   31,   32,
 /*   250 */    33,    2,   37,   38,   43,    5,    7,    8,    9,   10,
 /*   260 */    11,   12,   13,   14,   15,   16,   17,   18,   19,   20,
 /*   270 */    21,   22,   23,   24,   25,   25,   37,   38,   29,   30,
 /*   280 */    31,   32,    1,    2,    3,    4,    5,    6,    1,    2,
 /*   290 */     3,    4,    5,    6,    1,    2,    3,    4,    5,    6,
 /*   300 */     1,    2,    3,    4,    5,    6,   25,   26,   43,   40,
 /*   310 */     5,   42,   25,   26,   45,   34,   35,   36,   25,   43,
 /*   320 */    43,   34,   35,   36,   25,   43,   43,   34,   35,   36,
 /*   330 */    25,   43,   43,   34,   35,   36,    1,    2,    3,    4,
 /*   340 */     5,    1,    2,    3,    4,    5,    1,    2,    3,    4,
 /*   350 */     5,    1,    2,    3,    4,    5,   43,   43,   43,   43,
 /*   360 */    25,   26,   43,   43,   43,   25,   26,   43,   43,   43,
 /*   370 */    25,   26,   43,   43,   40,   25,   26,    1,    2,    3,
 /*   380 */     4,    5,    1,    2,    3,    4,    5,    1,    2,    3,
 /*   390 */     4,    5,    1,    2,    3,    4,    5,   40,   40,   40,
 /*   400 */    40,   25,   26,   40,   40,   40,   25,   26,   40,   40,
 /*   410 */    40,   25,   26,   40,   40,   40,   25,   26,    1,    2,
 /*   420 */     3,    4,    5,    1,    2,    3,    4,    5,    1,    2,
 /*   430 */     3,    4,    5,    1,    2,    3,    4,    5,   40,   40,
 /*   440 */    40,   40,   25,   26,   40,   40,   40,   25,   26,   40,
 /*   450 */    40,   40,   25,   26,   40,   40,   40,   25,   26,    1,
 /*   460 */     2,    3,    4,    5,    1,    2,    3,    4,    5,    1,
 /*   470 */     2,    3,    4,    5,    1,    2,    3,    4,    5,   40,
 /*   480 */    40,   25,   36,   25,   26,   36,   26,   37,   25,   26,
 /*   490 */    25,   38,   26,   25,   26,   25,   37,   26,   25,   26,
 /*   500 */     1,    2,    3,    4,    5,    1,    2,    3,    4,    5,
 /*   510 */     1,    2,    3,    4,    5,    1,    2,    3,    4,    5,
 /*   520 */    26,   25,   36,   36,   25,   26,   26,   25,   38,   25,
 /*   530 */    26,   26,   25,   46,   25,   26,   26,   26,   46,   25,
 /*   540 */    26,    1,    2,    3,    4,    5,    1,    2,    3,    4,
 /*   550 */     5,    1,    2,    3,    4,    5,    1,    2,    3,    4,
 /*   560 */     5,   46,   46,   46,   46,   25,   26,   46,   46,   46,
 /*   570 */    25,   26,   46,   46,   46,   25,   26,   46,   46,   46,
 /*   580 */    25,   26,    1,    2,    3,    4,    5,    1,    2,    3,
 /*   590 */     4,    5,    1,    2,    3,    4,    5,    1,    2,    3,
 /*   600 */     4,    5,   46,   46,   46,   46,   25,   26,   46,   46,
 /*   610 */    46,   25,   26,   46,   46,   46,   25,   26,   46,   46,
 /*   620 */    46,   25,   26,    1,    2,    3,    4,    5,    1,    2,
 /*   630 */     3,    4,    5,    1,    2,    3,    4,    5,    1,    2,
 /*   640 */     3,    4,    5,   46,   46,   46,   46,   25,   26,   46,
 /*   650 */    46,   46,   25,   26,   46,   46,   46,   25,   26,   46,
 /*   660 */    46,   46,   25,   26,    1,    2,    3,    4,    5,    1,
 /*   670 */     2,    3,    4,    5,    1,    2,    3,    4,    5,    1,
 /*   680 */     2,    3,    4,    5,   46,   46,   46,   46,   25,   26,
 /*   690 */    46,   46,   46,   25,   26,   46,   46,   46,   25,   26,
 /*   700 */    46,   46,   46,   25,   26,    1,    2,    3,    4,    5,
 /*   710 */     1,    2,    3,    4,    5,    1,    2,    3,    4,    5,
 /*   720 */     1,    2,    3,    4,    5,   46,   46,   46,   46,   25,
 /*   730 */    26,   46,   46,   46,   25,   26,   46,   46,   46,   25,
 /*   740 */    26,   46,   46,   46,   25,   26,    1,    2,    3,    4,
 /*   750 */     5,    1,    2,    3,    4,    5,   46,   46,   46,   46,
 /*   760 */    46,   46,   46,   46,   46,   46,   46,   46,   46,   46,
 /*   770 */    25,   46,   46,   46,   46,   25,
};
#define YY_SHIFT_USE_DFLT (-3)
#define YY_SHIFT_MAX 170
static const short yy_shift_ofst[] = {
 /*     0 */    -2,   30,   30,   30,   30,   30,  128,  128,  128,  128,
 /*    10 */   128,  128,   63,   96,  154,  185,  217,  217,  217,  217,
 /*    20 */   217,  217,  217,  217,  217,  217,  217,  217,  217,  217,
 /*    30 */   217,  217,  217,  217,  217,  217,  217,  217,  217,  217,
 /*    40 */   217,  217,  217,  217,  217,  217,  217,  217,  249,  249,
 /*    50 */   249,  249,  249,  249,  249,  249,  249,  249,  249,  249,
 /*    60 */   249,  249,  249,  249,  249,  249,  249,  249,  249,  249,
 /*    70 */   249,  249,  249,  249,  249,  249,  249,  249,  281,  287,
 /*    80 */   293,  299,  335,  340,  345,  350,  376,  381,  386,  391,
 /*    90 */   417,  422,  427,  432,  458,  463,  468,  473,  499,  504,
 /*   100 */   509,  514,  540,  545,  550,  555,  581,  586,  591,  596,
 /*   110 */   622,  627,  632,  637,  663,  668,  673,  678,  704,  709,
 /*   120 */   714,  719,  745,  745,  745,  745,  745,  745,  745,  750,
 /*   130 */   750,  750,  750,  750,  750,  750,   -1,   -1,   64,   64,
 /*   140 */   250,  250,  215,  305,  239,  305,  456,  456,  446,  449,
 /*   150 */   460,  450,  465,  466,  453,  470,  471,  494,  496,  496,
 /*   160 */   496,  486,  487,  500,  459,  502,  505,  490,  507,  510,
 /*   170 */   511,
};
#define YY_REDUCE_USE_DFLT (-43)
#define YY_REDUCE_MAX 77
static const short yy_reduce_ofst[] = {
 /*     0 */   269,  -10,   13,   47,   57,   79,  114,  140,  148,  171,
 /*    10 */   182,  203,  -42,   -8,   18,   20,   23,   50,   59,   81,
 /*    20 */    83,   88,   90,  139,  170,  177,  202,  204,  211,  265,
 /*    30 */   276,  277,  282,  283,  288,  289,  313,  314,  315,  316,
 /*    40 */   319,  320,  321,  324,  325,  326,  329,  330,  151,  334,
 /*    50 */   357,  358,  359,  360,  363,  364,  365,  368,  369,  370,
 /*    60 */   373,  374,  375,  398,  399,  400,  401,  404,  405,  406,
 /*    70 */   409,  410,  411,  414,  415,  416,  439,  440,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   312,  312,  312,  312,  312,  312,  312,  312,  312,  312,
 /*    10 */   312,  312,  312,  312,  312,  312,  312,  312,  312,  312,
 /*    20 */   312,  312,  312,  312,  312,  312,  312,  312,  312,  312,
 /*    30 */   312,  312,  312,  312,  312,  312,  312,  312,  312,  312,
 /*    40 */   312,  312,  312,  312,  312,  312,  312,  312,  312,  312,
 /*    50 */   312,  312,  312,  312,  312,  312,  312,  312,  312,  312,
 /*    60 */   312,  312,  312,  312,  312,  312,  312,  312,  312,  312,
 /*    70 */   312,  312,  312,  312,  312,  312,  312,  312,  312,  312,
 /*    80 */   312,  312,  312,  312,  312,  312,  312,  312,  312,  312,
 /*    90 */   312,  312,  312,  312,  312,  312,  312,  312,  312,  312,
 /*   100 */   312,  312,  312,  312,  312,  312,  312,  312,  312,  312,
 /*   110 */   312,  312,  312,  312,  312,  312,  312,  312,  312,  312,
 /*   120 */   312,  312,  228,  293,  295,  294,  296,  297,  298,  229,
 /*   130 */   303,  305,  304,  306,  307,  308,  237,  236,  268,  267,
 /*   140 */   238,  240,  292,  271,  302,  269,  235,  241,  312,  312,
 /*   150 */   312,  312,  312,  312,  312,  312,  312,  312,  265,  266,
 /*   160 */   272,  312,  312,  312,  312,  312,  312,  312,  312,  312,
 /*   170 */   312,  227,  231,  232,  233,  234,  239,  242,  243,  244,
 /*   180 */   245,  246,  247,  248,  249,  250,  251,  252,  253,  254,
 /*   190 */   255,  256,  257,  258,  259,  299,  300,  301,  260,  261,
 /*   200 */   262,  263,  264,  270,  273,  274,  275,  276,  277,  278,
 /*   210 */   279,  280,  281,  282,  283,  284,  285,  286,  287,  288,
 /*   220 */   289,  290,  309,  310,  311,  291,  230,
};
#define YY_SZ_ACTTAB (int)(sizeof(yy_action)/sizeof(yy_action[0]))

/* The next table maps tokens into fallback tokens.  If a construct
** like the following:
**
**      %fallback ID X Y Z.
**
** appears in the grammer, then ID becomes a fallback token for X, Y,
** and Z.  Whenever one of the tokens X, Y, or Z is input to the parser
** but it does not parse, the type of the token is changed to ID and
** the parse is retried before an error is thrown.
*/
#ifdef YYFALLBACK
static const YYCODETYPE yyFallback[] = {
};
#endif /* YYFALLBACK */

/* The following structure represents a single element of the
** parser's stack.  Information stored includes:
**
**   +  The state number for the parser at this level of the stack.
**
**   +  The value of the token stored at this level of the stack.
**      (In other words, the "major" token.)
**
**   +  The semantic value stored at this level of the stack.  This is
**      the information used by the action routines in the grammar.
**      It is sometimes called the "minor" token.
*/
struct yyStackEntry {
  int stateno;       /* The state-number */
  int major;         /* The major token value.  This is the code
                     ** number for the token at this stack level */
  YYMINORTYPE minor; /* The user-supplied minor token value.  This
                     ** is the value of the token  */
};
typedef struct yyStackEntry yyStackEntry;

/* The state of the parser is completely contained in an instance of
** the following structure */
struct yyParser {
  int yyidx;                    /* Index of top element in stack */
  int yyerrcnt;                 /* Shifts left before out of the error */
  ParseARG_SDECL                /* A place to hold %extra_argument */
#if YYSTACKDEPTH<=0
  int yystksz;                  /* Current side of the stack */
  yyStackEntry *yystack;        /* The parser's stack */
#else
  yyStackEntry yystack[YYSTACKDEPTH];  /* The parser's stack */
#endif
};
typedef struct yyParser yyParser;

#ifndef NDEBUG
#include <stdio.h>
static FILE *yyTraceFILE = 0;
static char *yyTracePrompt = 0;
#endif /* NDEBUG */

#ifndef NDEBUG
/*
** Turn parser tracing on by giving a stream to which to write the trace
** and a prompt to preface each trace message.  Tracing is turned off
** by making either argument NULL
**
** Inputs:
** <ul>
** <li> A FILE* to which trace output should be written.
**      If NULL, then tracing is turned off.
** <li> A prefix string written at the beginning of every
**      line of trace output.  If NULL, then tracing is
**      turned off.
** </ul>
**
** Outputs:
** None.
*/
void ParseTrace(FILE *TraceFILE, char *zTracePrompt){
  yyTraceFILE = TraceFILE;
  yyTracePrompt = zTracePrompt;
  if( yyTraceFILE==0 ) yyTracePrompt = 0;
  else if( yyTracePrompt==0 ) yyTraceFILE = 0;
}
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static const char *const yyTokenName[] = {
  "$",             "PLUS",          "MINUS",         "DIVIDE",
  "TIMES",         "POWER",         "NOT",           "ABS",
  "ACOSH",         "ACOS",          "ASINH",         "ASIN",
  "ATANH",         "ATAN",          "COSH",          "COS",
  "EXP",           "LN",            "LOG",           "SINH",
  "SIN",           "SQRT",          "TAN",           "URAMP",
  "U",             "LPAREN",        "RPAREN",        "DIFF",
  "EVAL",          "VARIABLE",      "ZERO_VARIABLE",  "CONSTANT",
  "IF",            "DIFF_VARIABLE",  "GREATERTHAN",   "LESSTHAN",
  "EQUAL",         "AND",           "OR",            "error",
  "expr",          "eval",          "ans",           "diff",
  "diff_eval",     "program",
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "program ::= ans",
 /*   1 */ "ans ::= expr",
 /*   2 */ "ans ::= DIFF diff",
 /*   3 */ "ans ::= EVAL eval",
 /*   4 */ "expr ::= VARIABLE",
 /*   5 */ "expr ::= ZERO_VARIABLE",
 /*   6 */ "expr ::= CONSTANT",
 /*   7 */ "expr ::= LPAREN expr RPAREN",
 /*   8 */ "expr ::= MINUS expr",
 /*   9 */ "expr ::= expr MINUS expr",
 /*  10 */ "expr ::= expr PLUS expr",
 /*  11 */ "expr ::= expr TIMES expr",
 /*  12 */ "expr ::= expr LPAREN expr RPAREN",
 /*  13 */ "expr ::= expr DIVIDE expr",
 /*  14 */ "expr ::= expr POWER expr",
 /*  15 */ "expr ::= ABS expr RPAREN",
 /*  16 */ "expr ::= ACOSH expr RPAREN",
 /*  17 */ "expr ::= ACOS expr RPAREN",
 /*  18 */ "expr ::= ASINH expr RPAREN",
 /*  19 */ "expr ::= ASIN expr RPAREN",
 /*  20 */ "expr ::= ATANH expr RPAREN",
 /*  21 */ "expr ::= ATAN expr RPAREN",
 /*  22 */ "expr ::= COSH expr RPAREN",
 /*  23 */ "expr ::= COS expr RPAREN",
 /*  24 */ "expr ::= EXP expr RPAREN",
 /*  25 */ "expr ::= LN expr RPAREN",
 /*  26 */ "expr ::= LOG expr RPAREN",
 /*  27 */ "expr ::= SINH expr RPAREN",
 /*  28 */ "expr ::= SIN expr RPAREN",
 /*  29 */ "expr ::= SQRT expr RPAREN",
 /*  30 */ "expr ::= TAN expr RPAREN",
 /*  31 */ "expr ::= URAMP expr RPAREN",
 /*  32 */ "expr ::= U expr RPAREN",
 /*  33 */ "expr ::= IF eval RPAREN",
 /*  34 */ "diff ::= DIFF_VARIABLE",
 /*  35 */ "diff ::= VARIABLE",
 /*  36 */ "diff ::= CONSTANT",
 /*  37 */ "diff ::= LPAREN diff RPAREN",
 /*  38 */ "diff ::= MINUS diff",
 /*  39 */ "diff ::= PLUS diff",
 /*  40 */ "diff ::= diff PLUS diff",
 /*  41 */ "diff ::= diff MINUS diff",
 /*  42 */ "diff ::= diff TIMES diff",
 /*  43 */ "diff ::= diff LPAREN diff RPAREN",
 /*  44 */ "diff ::= diff DIVIDE diff",
 /*  45 */ "diff ::= diff POWER diff",
 /*  46 */ "diff ::= ABS diff RPAREN",
 /*  47 */ "diff ::= ACOSH diff RPAREN",
 /*  48 */ "diff ::= ACOS diff RPAREN",
 /*  49 */ "diff ::= ASINH diff RPAREN",
 /*  50 */ "diff ::= ASIN diff RPAREN",
 /*  51 */ "diff ::= ATANH diff RPAREN",
 /*  52 */ "diff ::= ATAN diff RPAREN",
 /*  53 */ "diff ::= COSH diff RPAREN",
 /*  54 */ "diff ::= COS diff RPAREN",
 /*  55 */ "diff ::= EXP diff RPAREN",
 /*  56 */ "diff ::= LN diff RPAREN",
 /*  57 */ "diff ::= LOG diff RPAREN",
 /*  58 */ "diff ::= SINH diff RPAREN",
 /*  59 */ "diff ::= SIN diff RPAREN",
 /*  60 */ "diff ::= SQRT diff RPAREN",
 /*  61 */ "diff ::= TAN diff RPAREN",
 /*  62 */ "diff ::= URAMP diff RPAREN",
 /*  63 */ "diff ::= U diff RPAREN",
 /*  64 */ "diff ::= IF diff_eval RPAREN",
 /*  65 */ "eval ::= LPAREN eval RPAREN",
 /*  66 */ "eval ::= expr GREATERTHAN expr",
 /*  67 */ "eval ::= expr LESSTHAN expr",
 /*  68 */ "eval ::= expr GREATERTHAN EQUAL expr",
 /*  69 */ "eval ::= expr LESSTHAN EQUAL expr",
 /*  70 */ "eval ::= expr EQUAL EQUAL expr",
 /*  71 */ "eval ::= expr NOT EQUAL expr",
 /*  72 */ "eval ::= NOT eval",
 /*  73 */ "eval ::= LPAREN eval RPAREN AND AND LPAREN eval RPAREN",
 /*  74 */ "eval ::= LPAREN eval RPAREN OR OR LPAREN eval RPAREN",
 /*  75 */ "diff_eval ::= LPAREN diff_eval RPAREN",
 /*  76 */ "diff_eval ::= diff GREATERTHAN diff",
 /*  77 */ "diff_eval ::= diff LESSTHAN diff",
 /*  78 */ "diff_eval ::= diff GREATERTHAN EQUAL diff",
 /*  79 */ "diff_eval ::= diff LESSTHAN EQUAL diff",
 /*  80 */ "diff_eval ::= diff EQUAL EQUAL diff",
 /*  81 */ "diff_eval ::= diff NOT EQUAL diff",
 /*  82 */ "diff_eval ::= NOT diff_eval",
 /*  83 */ "diff_eval ::= LPAREN diff_eval RPAREN AND AND LPAREN diff_eval RPAREN",
 /*  84 */ "diff_eval ::= LPAREN diff_eval RPAREN OR OR LPAREN diff_eval RPAREN",
};
#endif /* NDEBUG */


#if YYSTACKDEPTH<=0
/*
** Try to increase the size of the parser stack.
*/
static void yyGrowStack(yyParser *p){
  int newSize;
  yyStackEntry *pNew;

  newSize = p->yystksz*2 + 100;
  pNew = realloc(p->yystack, newSize*sizeof(pNew[0]));
  if( pNew ){
    p->yystack = pNew;
    p->yystksz = newSize;
#ifndef NDEBUG
    if( yyTraceFILE ){
      fprintf(yyTraceFILE,"%sStack grows to %d entries!\n",
              yyTracePrompt, p->yystksz);
    }
#endif
  }
}
#endif

/*
** This function allocates a new parser.
** The only argument is a pointer to a function which works like
** malloc.
**
** Inputs:
** A pointer to the function used to allocate memory.
**
** Outputs:
** A pointer to a parser.  This pointer is used in subsequent calls
** to Parse and ParseFree.
*/
void *ParseAlloc(void *(*mallocProc)(size_t)){
  yyParser *pParser;
  pParser = (yyParser*)(*mallocProc)( (size_t)sizeof(yyParser) );
  if( pParser ){
    pParser->yyidx = -1;
#if YYSTACKDEPTH<=0
    yyGrowStack(pParser);
#endif
  }
  return pParser;
}

/* The following function deletes the value associated with a
** symbol.  The symbol can be either a terminal or nonterminal.
** "yymajor" is the symbol code, and "yypminor" is a pointer to
** the value.
*/
static void yy_destructor(YYCODETYPE yymajor, YYMINORTYPE *yypminor){
  switch( yymajor ){
    /* Here is inserted the actions which take place when a
    ** terminal or non-terminal is destroyed.  This can happen
    ** when the symbol is popped from the stack during a
    ** reduce or during error processing or when a parser is
    ** being destroyed before it is finished parsing.
    **
    ** Note: during a reduce, the only symbols destroyed are those
    ** which appear on the RHS of the rule, but which are not used
    ** inside the C code.
    */
    default:  break;   /* If no destructor action specified: do nothing */
  }
}

/*
** Pop the parser's stack once.
**
** If there is a destructor routine associated with the token which
** is popped from the stack, then call it.
**
** Return the major token number for the symbol popped.
*/
static int yy_pop_parser_stack(yyParser *pParser){
  YYCODETYPE yymajor;
  yyStackEntry *yytos = &pParser->yystack[pParser->yyidx];

  if( pParser->yyidx<0 ) return 0;
#ifndef NDEBUG
  if( yyTraceFILE && pParser->yyidx>=0 ){
    fprintf(yyTraceFILE,"%sPopping %s\n",
      yyTracePrompt,
      yyTokenName[yytos->major]);
  }
#endif
  yymajor = yytos->major;
  yy_destructor( yymajor, &yytos->minor);
  pParser->yyidx--;
  return yymajor;
}

/*
** Deallocate and destroy a parser.  Destructors are all called for
** all stack elements before shutting the parser down.
**
** Inputs:
** <ul>
** <li>  A pointer to the parser.  This should be a pointer
**       obtained from ParseAlloc.
** <li>  A pointer to a function used to reclaim memory obtained
**       from malloc.
** </ul>
*/
void ParseFree(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
){
  yyParser *pParser = (yyParser*)p;
  if( pParser==0 ) return;
  while( pParser->yyidx>=0 ) yy_pop_parser_stack(pParser);
#if YYSTACKDEPTH<=0
  free(pParser->yystack);
#endif
  (*freeProc)((void*)pParser);
}

/*
** Find the appropriate action for a parser given the terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_shift_action(
  yyParser *pParser,        /* The parser */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
  int stateno = pParser->yystack[pParser->yyidx].stateno;

  if( stateno>YY_SHIFT_MAX || (i = yy_shift_ofst[stateno])==YY_SHIFT_USE_DFLT ){
    return yy_default[stateno];
  }
  if( iLookAhead==YYNOCODE ){
    return YY_NO_ACTION;
  }
  i += iLookAhead;
  if( i<0 || i>=YY_SZ_ACTTAB || yy_lookahead[i]!=iLookAhead ){
    if( iLookAhead>0 ){
#ifdef YYFALLBACK
      int iFallback;            /* Fallback token */
      if( iLookAhead<sizeof(yyFallback)/sizeof(yyFallback[0])
             && (iFallback = yyFallback[iLookAhead])!=0 ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE, "%sFALLBACK %s => %s\n",
             yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[iFallback]);
        }
#endif
        return yy_find_shift_action(pParser, iFallback);
      }
#endif
#ifdef YYWILDCARD
      {
        int j = i - iLookAhead + YYWILDCARD;
        if( j>=0 && j<YY_SZ_ACTTAB && yy_lookahead[j]==YYWILDCARD ){
#ifndef NDEBUG
          if( yyTraceFILE ){
            fprintf(yyTraceFILE, "%sWILDCARD %s => %s\n",
               yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[YYWILDCARD]);
          }
#endif /* NDEBUG */
          return yy_action[j];
        }
      }
#endif /* YYWILDCARD */
    }
    return yy_default[stateno];
  }else{
    return yy_action[i];
  }
}

/*
** Find the appropriate action for a parser given the non-terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_reduce_action(
  int stateno,              /* Current state number */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
  /* int stateno = pParser->yystack[pParser->yyidx].stateno; */

  if( stateno>YY_REDUCE_MAX ||
      (i = yy_reduce_ofst[stateno])==YY_REDUCE_USE_DFLT ){
    return yy_default[stateno];
  }
  if( iLookAhead==YYNOCODE ){
    return YY_NO_ACTION;
  }
  i += iLookAhead;
  if( i<0 || i>=YY_SZ_ACTTAB || yy_lookahead[i]!=iLookAhead ){
    return yy_default[stateno];
  }else{
    return yy_action[i];
  }
}

/*
** The following routine is called if the stack overflows.
*/
static void yyStackOverflow(yyParser *yypParser, YYMINORTYPE *yypMinor){
   ParseARG_FETCH;
   yypParser->yyidx--;
#ifndef NDEBUG
   if( yyTraceFILE ){
     fprintf(yyTraceFILE,"%sStack Overflow!\n",yyTracePrompt);
   }
#endif
   while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
   /* Here code is inserted which will execute if the parser
   ** stack every overflows */
#line 50 "parser.lem"
 Error("Parser stack overflow"); *solution = sqrtf(-1.f);
#line 853 "parser.c"
   ParseARG_STORE; /* Suppress warning about unused %extra_argument var */
}

/*
** Perform a shift action.
*/
static void yy_shift(
  yyParser *yypParser,          /* The parser to be shifted */
  int yyNewState,               /* The new state to shift in */
  int yyMajor,                  /* The major token to shift in */
  YYMINORTYPE *yypMinor         /* Pointer ot the minor token to shift in */
){
  yyStackEntry *yytos;
  yypParser->yyidx++;
#if YYSTACKDEPTH>0
  if( yypParser->yyidx>=YYSTACKDEPTH ){
    yyStackOverflow(yypParser, yypMinor);
    return;
  }
#else
  if( yypParser->yyidx>=yypParser->yystksz ){
    yyGrowStack(yypParser);
    if( yypParser->yyidx>=yypParser->yystksz ){
      yyStackOverflow(yypParser, yypMinor);
      return;
    }
  }
#endif
  yytos = &yypParser->yystack[yypParser->yyidx];
  yytos->stateno = yyNewState;
  yytos->major = yyMajor;
  yytos->minor = *yypMinor;
#ifndef NDEBUG
  if( yyTraceFILE && yypParser->yyidx>0 ){
    int i;
    fprintf(yyTraceFILE,"%sShift %d\n",yyTracePrompt,yyNewState);
    fprintf(yyTraceFILE,"%sStack:",yyTracePrompt);
    for(i=1; i<=yypParser->yyidx; i++)
      fprintf(yyTraceFILE," %s",yyTokenName[yypParser->yystack[i].major]);
    fprintf(yyTraceFILE,"\n");
  }
#endif
}

/* The following table contains information about every rule that
** is used during the reduce.
*/
static const struct {
  YYCODETYPE lhs;         /* Symbol on the left-hand side of the rule */
  unsigned char nrhs;     /* Number of right-hand side symbols in the rule */
} yyRuleInfo[] = {
  { 45, 1 },
  { 42, 1 },
  { 42, 2 },
  { 42, 2 },
  { 40, 1 },
  { 40, 1 },
  { 40, 1 },
  { 40, 3 },
  { 40, 2 },
  { 40, 3 },
  { 40, 3 },
  { 40, 3 },
  { 40, 4 },
  { 40, 3 },
  { 40, 3 },
  { 40, 3 },
  { 40, 3 },
  { 40, 3 },
  { 40, 3 },
  { 40, 3 },
  { 40, 3 },
  { 40, 3 },
  { 40, 3 },
  { 40, 3 },
  { 40, 3 },
  { 40, 3 },
  { 40, 3 },
  { 40, 3 },
  { 40, 3 },
  { 40, 3 },
  { 40, 3 },
  { 40, 3 },
  { 40, 3 },
  { 40, 3 },
  { 43, 1 },
  { 43, 1 },
  { 43, 1 },
  { 43, 3 },
  { 43, 2 },
  { 43, 2 },
  { 43, 3 },
  { 43, 3 },
  { 43, 3 },
  { 43, 4 },
  { 43, 3 },
  { 43, 3 },
  { 43, 3 },
  { 43, 3 },
  { 43, 3 },
  { 43, 3 },
  { 43, 3 },
  { 43, 3 },
  { 43, 3 },
  { 43, 3 },
  { 43, 3 },
  { 43, 3 },
  { 43, 3 },
  { 43, 3 },
  { 43, 3 },
  { 43, 3 },
  { 43, 3 },
  { 43, 3 },
  { 43, 3 },
  { 43, 3 },
  { 43, 3 },
  { 41, 3 },
  { 41, 3 },
  { 41, 3 },
  { 41, 4 },
  { 41, 4 },
  { 41, 4 },
  { 41, 4 },
  { 41, 2 },
  { 41, 8 },
  { 41, 8 },
  { 44, 3 },
  { 44, 3 },
  { 44, 3 },
  { 44, 4 },
  { 44, 4 },
  { 44, 4 },
  { 44, 4 },
  { 44, 2 },
  { 44, 8 },
  { 44, 8 },
};

static void yy_accept(yyParser*);  /* Forward Declaration */

/*
** Perform a reduce action and the shift that must immediately
** follow the reduce.
*/
static void yy_reduce(
  yyParser *yypParser,         /* The parser */
  int yyruleno                 /* Number of the rule by which to reduce */
){
  int yygoto;                     /* The next state */
  int yyact;                      /* The next action */
  YYMINORTYPE yygotominor;        /* The LHS of the rule reduced */
  yyStackEntry *yymsp;            /* The top of the parser's stack */
  int yysize;                     /* Amount to pop the stack */
  ParseARG_FETCH;
  yymsp = &yypParser->yystack[yypParser->yyidx];
#ifndef NDEBUG
  if( yyTraceFILE && yyruleno>=0
        && yyruleno<(int)(sizeof(yyRuleName)/sizeof(yyRuleName[0])) ){
    fprintf(yyTraceFILE, "%sReduce [%s].\n", yyTracePrompt,
      yyRuleName[yyruleno]);
  }
#endif /* NDEBUG */

  /* Silence complaints from purify about yygotominor being uninitialized
  ** in some cases when it is copied into the stack after the following
  ** switch.  yygotominor is uninitialized when a rule reduces that does
  ** not set the value of its left-hand side nonterminal.  Leaving the
  ** value of the nonterminal uninitialized is utterly harmless as long
  ** as the value is never used.  So really the only thing this code
  ** accomplishes is to quieten purify.
  **
  ** 2007-01-16:  The wireshark project (www.wireshark.org) reports that
  ** without this code, their parser segfaults.  I'm not sure what there
  ** parser is doing to make this happen.  This is the second bug report
  ** from wireshark this week.  Clearly they are stressing Lemon in ways
  ** that it has not been previously stressed...  (SQLite ticket #2172)
  */
  memset(&yygotominor, 0, sizeof(yygotominor));


  switch( yyruleno ){
  /* Beginning here are the reduction cases.  A typical example
  ** follows:
  **   case 0:
  **  #line <lineno> <grammarfile>
  **     { ... }           // User supplied code
  **  #line <lineno> <thisfile>
  **     break;
  */
      case 0:
#line 60 "parser.lem"
{ Debug("Solution: %e", yymsp[0].minor.yy76); *solution = yymsp[0].minor.yy76; }
#line 1046 "parser.c"
        break;
      case 1:
      case 3:
#line 62 "parser.lem"
{ yygotominor.yy76 = yymsp[0].minor.yy76; }
#line 1052 "parser.c"
        break;
      case 2:
#line 63 "parser.lem"
{ yygotominor.yy76 = yymsp[0].minor.yy41.d; }
#line 1057 "parser.c"
        break;
      case 4:
#line 66 "parser.lem"
{ yygotominor.yy76 = **yymsp[0].minor.yy0->variable; }
#line 1062 "parser.c"
        break;
      case 5:
#line 67 "parser.lem"
{ yygotominor.yy76 = 0.0; }
#line 1067 "parser.c"
        break;
      case 6:
#line 68 "parser.lem"
{ yygotominor.yy76 = yymsp[0].minor.yy0->constant; }
#line 1072 "parser.c"
        break;
      case 7:
      case 65:
#line 69 "parser.lem"
{ yygotominor.yy76 = yymsp[-1].minor.yy76; }
#line 1078 "parser.c"
        break;
      case 8:
#line 70 "parser.lem"
{ yygotominor.yy76 = -1*yymsp[0].minor.yy76; }
#line 1083 "parser.c"
        break;
      case 9:
#line 71 "parser.lem"
{ yygotominor.yy76 = yymsp[-2].minor.yy76 - yymsp[0].minor.yy76; }
#line 1088 "parser.c"
        break;
      case 10:
#line 72 "parser.lem"
{ yygotominor.yy76 = yymsp[-2].minor.yy76 + yymsp[0].minor.yy76; }
#line 1093 "parser.c"
        break;
      case 11:
#line 73 "parser.lem"
{ yygotominor.yy76 = yymsp[-2].minor.yy76 * yymsp[0].minor.yy76; }
#line 1098 "parser.c"
        break;
      case 12:
#line 74 "parser.lem"
{ yygotominor.yy76 = yymsp[-3].minor.yy76 * yymsp[-1].minor.yy76; }
#line 1103 "parser.c"
        break;
      case 13:
#line 75 "parser.lem"
{ yygotominor.yy76 = Div(yymsp[-2].minor.yy76,yymsp[0].minor.yy76,(*yymsp[-1].minor.yy0->minDiv)); }
#line 1108 "parser.c"
        break;
      case 14:
#line 76 "parser.lem"
{ yygotominor.yy76 = pow(yymsp[-2].minor.yy76,yymsp[0].minor.yy76); }
#line 1113 "parser.c"
        break;
      case 15:
#line 77 "parser.lem"
{ yygotominor.yy76 = fabs(yymsp[-1].minor.yy76); }
#line 1118 "parser.c"
        break;
      case 16:
#line 78 "parser.lem"
{ yygotominor.yy76 = acosh(yymsp[-1].minor.yy76); }
#line 1123 "parser.c"
        break;
      case 17:
#line 79 "parser.lem"
{ yygotominor.yy76 = acos(yymsp[-1].minor.yy76); }
#line 1128 "parser.c"
        break;
      case 18:
#line 80 "parser.lem"
{ yygotominor.yy76 = asinh(yymsp[-1].minor.yy76); }
#line 1133 "parser.c"
        break;
      case 19:
#line 81 "parser.lem"
{ yygotominor.yy76 = asin(yymsp[-1].minor.yy76); }
#line 1138 "parser.c"
        break;
      case 20:
#line 82 "parser.lem"
{ yygotominor.yy76 = atanh(yymsp[-1].minor.yy76); }
#line 1143 "parser.c"
        break;
      case 21:
#line 83 "parser.lem"
{ yygotominor.yy76 = atan(yymsp[-1].minor.yy76); }
#line 1148 "parser.c"
        break;
      case 22:
#line 84 "parser.lem"
{ yygotominor.yy76 = cosh(yymsp[-1].minor.yy76); }
#line 1153 "parser.c"
        break;
      case 23:
#line 85 "parser.lem"
{ yygotominor.yy76 = cos(yymsp[-1].minor.yy76); }
#line 1158 "parser.c"
        break;
      case 24:
#line 86 "parser.lem"
{ yygotominor.yy76 = exp(yymsp[-1].minor.yy76); }
#line 1163 "parser.c"
        break;
      case 25:
#line 87 "parser.lem"
{ yygotominor.yy76 = Ln(yymsp[-1].minor.yy76); }
#line 1168 "parser.c"
        break;
      case 26:
#line 88 "parser.lem"
{ yygotominor.yy76 = log10(yymsp[-1].minor.yy76); }
#line 1173 "parser.c"
        break;
      case 27:
#line 89 "parser.lem"
{ yygotominor.yy76 = sinh(yymsp[-1].minor.yy76); }
#line 1178 "parser.c"
        break;
      case 28:
#line 90 "parser.lem"
{ yygotominor.yy76 = sin(yymsp[-1].minor.yy76); }
#line 1183 "parser.c"
        break;
      case 29:
#line 91 "parser.lem"
{ yygotominor.yy76 = sqrt(yymsp[-1].minor.yy76); }
#line 1188 "parser.c"
        break;
      case 30:
#line 92 "parser.lem"
{ yygotominor.yy76 = tan(yymsp[-1].minor.yy76); }
#line 1193 "parser.c"
        break;
      case 31:
#line 93 "parser.lem"
{ yygotominor.yy76 = ((yymsp[-1].minor.yy76 > 0) ? yymsp[-1].minor.yy76 : 0); }
#line 1198 "parser.c"
        break;
      case 32:
#line 94 "parser.lem"
{ yygotominor.yy76 = ((yymsp[-1].minor.yy76 > 0) ? 1 : 0); }
#line 1203 "parser.c"
        break;
      case 33:
#line 95 "parser.lem"
{ yygotominor.yy76 = ((yymsp[-1].minor.yy76) ? 1 : 0);}
#line 1208 "parser.c"
        break;
      case 34:
#line 97 "parser.lem"
{ yygotominor.yy41.f = **yymsp[0].minor.yy0->variable; yygotominor.yy41.d = 1.0; }
#line 1213 "parser.c"
        break;
      case 35:
#line 98 "parser.lem"
{ yygotominor.yy41.f = **yymsp[0].minor.yy0->variable; yygotominor.yy41.d = 0.0; }
#line 1218 "parser.c"
        break;
      case 36:
#line 99 "parser.lem"
{ yygotominor.yy41.f = yymsp[0].minor.yy0->constant; yygotominor.yy41.d = 0.0; }
#line 1223 "parser.c"
        break;
      case 37:
#line 100 "parser.lem"
{ yygotominor.yy41.f = yymsp[-1].minor.yy41.f; yygotominor.yy41.d = yymsp[-1].minor.yy41.d; }
#line 1228 "parser.c"
        break;
      case 38:
#line 101 "parser.lem"
{ yygotominor.yy41.f = -1*yymsp[0].minor.yy41.f; yygotominor.yy41.d = -1*yymsp[0].minor.yy41.d; }
#line 1233 "parser.c"
        break;
      case 39:
#line 102 "parser.lem"
{ yygotominor.yy41.f = yymsp[0].minor.yy41.f; yygotominor.yy41.d = yymsp[0].minor.yy41.d; }
#line 1238 "parser.c"
        break;
      case 40:
#line 103 "parser.lem"
{ yygotominor.yy41.f = yymsp[-2].minor.yy41.f + yymsp[0].minor.yy41.f; yygotominor.yy41.d = yymsp[-2].minor.yy41.d + yymsp[0].minor.yy41.d; }
#line 1243 "parser.c"
        break;
      case 41:
#line 104 "parser.lem"
{ yygotominor.yy41.f = yymsp[-2].minor.yy41.f - yymsp[0].minor.yy41.f; yygotominor.yy41.d = yymsp[-2].minor.yy41.d - yymsp[0].minor.yy41.d; }
#line 1248 "parser.c"
        break;
      case 42:
#line 105 "parser.lem"
{ yygotominor.yy41.f = yymsp[-2].minor.yy41.f * yymsp[0].minor.yy41.f; yygotominor.yy41.d = yymsp[-2].minor.yy41.d*yymsp[0].minor.yy41.f + yymsp[-2].minor.yy41.f*yymsp[0].minor.yy41.d; }
#line 1253 "parser.c"
        break;
      case 43:
#line 106 "parser.lem"
{ yygotominor.yy41.f = yymsp[-3].minor.yy41.f * yymsp[-1].minor.yy41.f; yygotominor.yy41.d = yymsp[-3].minor.yy41.d*yymsp[-1].minor.yy41.f + yymsp[-3].minor.yy41.f*yymsp[-1].minor.yy41.d; }
#line 1258 "parser.c"
        break;
      case 44:
#line 107 "parser.lem"
{ yygotominor.yy41.f = Div(yymsp[-2].minor.yy41.f,yymsp[0].minor.yy41.f,(*yymsp[-1].minor.yy0->minDiv)); yygotominor.yy41.d = Div((yymsp[-2].minor.yy41.d*yymsp[0].minor.yy41.f - yymsp[-2].minor.yy41.f*yymsp[0].minor.yy41.d),(yymsp[0].minor.yy41.f*yymsp[0].minor.yy41.f),(*yymsp[-1].minor.yy0->minDiv)); }
#line 1263 "parser.c"
        break;
      case 45:
#line 108 "parser.lem"
{ yygotominor.yy41.f = pow(yymsp[-2].minor.yy41.f,yymsp[0].minor.yy41.f); yygotominor.yy41.d = pow(yymsp[-2].minor.yy41.f,yymsp[0].minor.yy41.f) * ( yymsp[-2].minor.yy41.d * Div(yymsp[0].minor.yy41.f, yymsp[-2].minor.yy41.f, *yymsp[-1].minor.yy0->minDiv) + (yymsp[-2].minor.yy41.f ? yymsp[0].minor.yy41.d * Ln(yymsp[-2].minor.yy41.f) : 0)); }
#line 1268 "parser.c"
        break;
      case 46:
#line 109 "parser.lem"
{ yygotominor.yy41.f = fabs(yymsp[-1].minor.yy41.f); yygotominor.yy41.d = Div(yymsp[-1].minor.yy41.f,fabs(yymsp[-1].minor.yy41.f),(*yymsp[-2].minor.yy0->minDiv))*yymsp[-1].minor.yy41.d; }
#line 1273 "parser.c"
        break;
      case 47:
#line 110 "parser.lem"
{ yygotominor.yy41.f = acosh(yymsp[-1].minor.yy41.f); yygotominor.yy41.d = sinh(yymsp[-1].minor.yy41.f)*yymsp[-1].minor.yy41.d; }
#line 1278 "parser.c"
        break;
      case 48:
#line 111 "parser.lem"
{ yygotominor.yy41.f = acos(yymsp[-1].minor.yy41.f); yygotominor.yy41.d = Div(-1,sqrt(1-yymsp[-1].minor.yy41.f*yymsp[-1].minor.yy41.f),(*yymsp[-2].minor.yy0->minDiv))*yymsp[-1].minor.yy41.d; }
#line 1283 "parser.c"
        break;
      case 49:
#line 112 "parser.lem"
{ yygotominor.yy41.f = asinh(yymsp[-1].minor.yy41.f); yygotominor.yy41.d = Div(1,sqrt(1+yymsp[-1].minor.yy41.f*yymsp[-1].minor.yy41.f),(*yymsp[-2].minor.yy0->minDiv))*yymsp[-1].minor.yy41.d; }
#line 1288 "parser.c"
        break;
      case 50:
#line 113 "parser.lem"
{ yygotominor.yy41.f = asin(yymsp[-1].minor.yy41.f); yygotominor.yy41.d = Div(1,sqrt(1-yymsp[-1].minor.yy41.f*yymsp[-1].minor.yy41.f),(*yymsp[-2].minor.yy0->minDiv))*yymsp[-1].minor.yy41.d; }
#line 1293 "parser.c"
        break;
      case 51:
#line 114 "parser.lem"
{ yygotominor.yy41.f = atanh(yymsp[-1].minor.yy41.f); yygotominor.yy41.d = Div(1,(1-yymsp[-1].minor.yy41.f*yymsp[-1].minor.yy41.f),(*yymsp[-2].minor.yy0->minDiv))*yymsp[-1].minor.yy41.d; }
#line 1298 "parser.c"
        break;
      case 52:
#line 115 "parser.lem"
{ yygotominor.yy41.f = atan(yymsp[-1].minor.yy41.f); yygotominor.yy41.d = Div(1,(1+yymsp[-1].minor.yy41.f*yymsp[-1].minor.yy41.f),(*yymsp[-2].minor.yy0->minDiv))*yymsp[-1].minor.yy41.d; }
#line 1303 "parser.c"
        break;
      case 53:
#line 116 "parser.lem"
{ yygotominor.yy41.f = cosh(yymsp[-1].minor.yy41.f); yygotominor.yy41.d = sinh(yymsp[-1].minor.yy41.f)*yymsp[-1].minor.yy41.d; }
#line 1308 "parser.c"
        break;
      case 54:
#line 117 "parser.lem"
{ yygotominor.yy41.f = cos(yymsp[-1].minor.yy41.f); yygotominor.yy41.d = -1*sin(yymsp[-1].minor.yy41.f)*yymsp[-1].minor.yy41.d; }
#line 1313 "parser.c"
        break;
      case 55:
#line 118 "parser.lem"
{ yygotominor.yy41.f = exp(yymsp[-1].minor.yy41.f);  yygotominor.yy41.d = exp(yymsp[-1].minor.yy41.f)*yymsp[-1].minor.yy41.d; }
#line 1318 "parser.c"
        break;
      case 56:
#line 119 "parser.lem"
{ yygotominor.yy41.f = Ln(yymsp[-1].minor.yy41.f); yygotominor.yy41.d = Div(1,yymsp[-1].minor.yy41.f,(*yymsp[-2].minor.yy0->minDiv))*yymsp[-1].minor.yy41.d; }
#line 1323 "parser.c"
        break;
      case 57:
#line 120 "parser.lem"
{ yygotominor.yy41.f = log10(yymsp[-1].minor.yy41.f); yygotominor.yy41.d = Div(1,(yymsp[-1].minor.yy41.f*Ln(10)),(*yymsp[-2].minor.yy0->minDiv))*yymsp[-1].minor.yy41.d; }
#line 1328 "parser.c"
        break;
      case 58:
#line 121 "parser.lem"
{ yygotominor.yy41.f = sinh(yymsp[-1].minor.yy41.f); yygotominor.yy41.d = cosh(yymsp[-1].minor.yy41.f)*yymsp[-1].minor.yy41.d; }
#line 1333 "parser.c"
        break;
      case 59:
#line 122 "parser.lem"
{ yygotominor.yy41.f = sin(yymsp[-1].minor.yy41.f); yygotominor.yy41.d = cos(yymsp[-1].minor.yy41.f)*yymsp[-1].minor.yy41.d; }
#line 1338 "parser.c"
        break;
      case 60:
#line 123 "parser.lem"
{ yygotominor.yy41.f = sqrt(yymsp[-1].minor.yy41.f); yygotominor.yy41.d = Div(1,(2*sqrt(yymsp[-1].minor.yy41.f)),(*yymsp[-2].minor.yy0->minDiv))*yymsp[-1].minor.yy41.d; }
#line 1343 "parser.c"
        break;
      case 61:
#line 124 "parser.lem"
{ yygotominor.yy41.f = tan(yymsp[-1].minor.yy41.f); yygotominor.yy41.d = Div(1,cos(yymsp[-1].minor.yy41.f),(*yymsp[-2].minor.yy0->minDiv))*Div(1,cos(yymsp[-1].minor.yy41.f),(*yymsp[-2].minor.yy0->minDiv))*yymsp[-1].minor.yy41.d; }
#line 1348 "parser.c"
        break;
      case 62:
#line 125 "parser.lem"
{ yygotominor.yy41.f = ((yymsp[-1].minor.yy41.f > 0) ? yymsp[-1].minor.yy41.f : 0); yygotominor.yy41.d = (((yymsp[-1].minor.yy41.f == 0)? 0.5 : ((yymsp[-1].minor.yy41.f > 0) ? 1.0 : 0))*yymsp[-1].minor.yy41.d); }
#line 1353 "parser.c"
        break;
      case 63:
#line 126 "parser.lem"
{ yygotominor.yy41.f = ((yymsp[-1].minor.yy41.f > 0) ? 1 : 0); yygotominor.yy41.d = ((yymsp[-1].minor.yy41.f == 0)? 1/(*yymsp[-2].minor.yy0->minDiv) : 0.0); }
#line 1358 "parser.c"
        break;
      case 64:
#line 127 "parser.lem"
{ yygotominor.yy41.f = (yymsp[-1].minor.yy41.f) ? 1 : 0; yygotominor.yy41.d = 0.0; }
#line 1363 "parser.c"
        break;
      case 66:
#line 130 "parser.lem"
{ yygotominor.yy76 = (yymsp[-2].minor.yy76 > yymsp[0].minor.yy76) ? 1 : 0; }
#line 1368 "parser.c"
        break;
      case 67:
#line 131 "parser.lem"
{ yygotominor.yy76 = (yymsp[-2].minor.yy76 < yymsp[0].minor.yy76) ? 1 : 0; }
#line 1373 "parser.c"
        break;
      case 68:
#line 132 "parser.lem"
{ yygotominor.yy76 = (yymsp[-3].minor.yy76 >= yymsp[0].minor.yy76) ? 1 : 0; }
#line 1378 "parser.c"
        break;
      case 69:
#line 133 "parser.lem"
{ yygotominor.yy76 = (yymsp[-3].minor.yy76 <= yymsp[0].minor.yy76) ? 1 : 0; }
#line 1383 "parser.c"
        break;
      case 70:
#line 134 "parser.lem"
{ yygotominor.yy76 = (yymsp[-3].minor.yy76 == yymsp[0].minor.yy76) ? 1 : 0; }
#line 1388 "parser.c"
        break;
      case 71:
#line 135 "parser.lem"
{ yygotominor.yy76 = (yymsp[-3].minor.yy76 != yymsp[0].minor.yy76) ? 1 : 0; }
#line 1393 "parser.c"
        break;
      case 72:
#line 136 "parser.lem"
{ yygotominor.yy76 = (yymsp[0].minor.yy76 == 0) ? 1 : 0; }
#line 1398 "parser.c"
        break;
      case 73:
#line 137 "parser.lem"
{ yygotominor.yy76 = ((yymsp[-6].minor.yy76 == 1) && (yymsp[-1].minor.yy76 == 1)) ? 1 : 0; }
#line 1403 "parser.c"
        break;
      case 74:
#line 138 "parser.lem"
{ yygotominor.yy76 = ((yymsp[-6].minor.yy76 == 1) || (yymsp[-1].minor.yy76 == 1)) ? 1 : 0; }
#line 1408 "parser.c"
        break;
      case 75:
#line 140 "parser.lem"
{ yygotominor.yy41.f = yymsp[-1].minor.yy41.f; }
#line 1413 "parser.c"
        break;
      case 76:
#line 141 "parser.lem"
{ yygotominor.yy41.f = (yymsp[-2].minor.yy41.f > yymsp[0].minor.yy41.f) ? 1 : 0; }
#line 1418 "parser.c"
        break;
      case 77:
#line 142 "parser.lem"
{ yygotominor.yy41.f = (yymsp[-2].minor.yy41.f < yymsp[0].minor.yy41.f) ? 1 : 0; }
#line 1423 "parser.c"
        break;
      case 78:
#line 143 "parser.lem"
{ yygotominor.yy41.f = (yymsp[-3].minor.yy41.f >= yymsp[0].minor.yy41.f) ? 1 : 0; }
#line 1428 "parser.c"
        break;
      case 79:
#line 144 "parser.lem"
{ yygotominor.yy41.f = (yymsp[-3].minor.yy41.f <= yymsp[0].minor.yy41.f) ? 1 : 0; }
#line 1433 "parser.c"
        break;
      case 80:
#line 145 "parser.lem"
{ yygotominor.yy41.f = (yymsp[-3].minor.yy41.f == yymsp[0].minor.yy41.f) ? 1 : 0; }
#line 1438 "parser.c"
        break;
      case 81:
#line 146 "parser.lem"
{ yygotominor.yy41.f = (yymsp[-3].minor.yy41.f != yymsp[0].minor.yy41.f) ? 1 : 0; }
#line 1443 "parser.c"
        break;
      case 82:
#line 147 "parser.lem"
{ yygotominor.yy41.f = (yymsp[0].minor.yy41.f == 0) ? 1 : 0; }
#line 1448 "parser.c"
        break;
      case 83:
#line 148 "parser.lem"
{ yygotominor.yy41.f = ((yymsp[-6].minor.yy41.f == 1) && (yymsp[-1].minor.yy41.f == 1)) ? 1 : 0; }
#line 1453 "parser.c"
        break;
      case 84:
#line 149 "parser.lem"
{ yygotominor.yy41.f = ((yymsp[-6].minor.yy41.f == 1) || (yymsp[-1].minor.yy41.f == 1)) ? 1 : 0; }
#line 1458 "parser.c"
        break;
  };
  yygoto = yyRuleInfo[yyruleno].lhs;
  yysize = yyRuleInfo[yyruleno].nrhs;
  yypParser->yyidx -= yysize;
  yyact = yy_find_reduce_action(yymsp[-yysize].stateno,yygoto);
  if( yyact < YYNSTATE ){
#ifdef NDEBUG
    /* If we are not debugging and the reduce action popped at least
    ** one element off the stack, then we can push the new element back
    ** onto the stack here, and skip the stack overflow test in yy_shift().
    ** That gives a significant speed improvement. */
    if( yysize ){
      yypParser->yyidx++;
      yymsp -= yysize-1;
      yymsp->stateno = yyact;
      yymsp->major = yygoto;
      yymsp->minor = yygotominor;
    }else
#endif
    {
      yy_shift(yypParser,yyact,yygoto,&yygotominor);
    }
  }else if( yyact == YYNSTATE + YYNRULE + 1 ){
    yy_accept(yypParser);
  }
}

/*
** The following code executes when the parse fails
*/
static void yy_parse_failed(
  yyParser *yypParser           /* The parser */
){
  ParseARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sFail!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser fails */
#line 49 "parser.lem"
 Error("Parser is lost..."); *solution = sqrtf(-1.f);
#line 1505 "parser.c"
  ParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following code executes when a syntax error first occurs.
*/
static void yy_syntax_error(
  yyParser *yypParser,           /* The parser */
  int yymajor,                   /* The major type of the error token */
  YYMINORTYPE yyminor            /* The minor type of the error token */
){
  ParseARG_FETCH;
#define TOKEN (yyminor.yy0)
#line 48 "parser.lem"
 Error("Syntax Error"); *solution = sqrtf(-1.f);
#line 1522 "parser.c"
  ParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following is executed when the parser accepts
*/
static void yy_accept(
  yyParser *yypParser           /* The parser */
){
  ParseARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sAccept!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser accepts */
#line 47 "parser.lem"
 Debug("Parsing Complete");
#line 1544 "parser.c"
  ParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/* The main parser program.
** The first argument is a pointer to a structure obtained from
** "ParseAlloc" which describes the current state of the parser.
** The second argument is the major token number.  The third is
** the minor token.  The fourth optional argument is whatever the
** user wants (and specified in the grammar) and is available for
** use by the action routines.
**
** Inputs:
** <ul>
** <li> A pointer to the parser (an opaque structure.)
** <li> The major token number.
** <li> The minor token number.
** <li> An option argument of a grammar-specified type.
** </ul>
**
** Outputs:
** None.
*/
void Parse(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  ParseTOKENTYPE yyminor       /* The value for the token */
  ParseARG_PDECL               /* Optional %extra_argument parameter */
){
  YYMINORTYPE yyminorunion;
  int yyact;            /* The parser action. */
  int yyendofinput;     /* True if we are at the end of input */
  int yyerrorhit = 0;   /* True if yymajor has invoked an error */
  yyParser *yypParser;  /* The parser */

  /* (re)initialize the parser, if necessary */
  yypParser = (yyParser*)yyp;
  if( yypParser->yyidx<0 ){
#if YYSTACKDEPTH<=0
    if( yypParser->yystksz <=0 ){
      memset(&yyminorunion, 0, sizeof(yyminorunion));
      yyStackOverflow(yypParser, &yyminorunion);
      return;
    }
#endif
    yypParser->yyidx = 0;
    yypParser->yyerrcnt = -1;
    yypParser->yystack[0].stateno = 0;
    yypParser->yystack[0].major = 0;
  }
  yyminorunion.yy0 = yyminor;
  yyendofinput = (yymajor==0);
  ParseARG_STORE;

#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sInput %s\n",yyTracePrompt,yyTokenName[yymajor]);
  }
#endif

  do{
    yyact = yy_find_shift_action(yypParser,yymajor);
    if( yyact<YYNSTATE ){
      yy_shift(yypParser,yyact,yymajor,&yyminorunion);
      yypParser->yyerrcnt--;
      if( yyendofinput && yypParser->yyidx>=0 ){
        yymajor = 0;
      }else{
        yymajor = YYNOCODE;
      }
    }else if( yyact < YYNSTATE + YYNRULE ){
      yy_reduce(yypParser,yyact-YYNSTATE);
    }else if( yyact == YY_ERROR_ACTION ){
      int yymx;
#ifndef NDEBUG
      if( yyTraceFILE ){
        fprintf(yyTraceFILE,"%sSyntax Error!\n",yyTracePrompt);
      }
#endif
#ifdef YYERRORSYMBOL
      /* A syntax error has occurred.
      ** The response to an error depends upon whether or not the
      ** grammar defines an error token "ERROR".
      **
      ** This is what we do if the grammar does define ERROR:
      **
      **  * Call the %syntax_error function.
      **
      **  * Begin popping the stack until we enter a state where
      **    it is legal to shift the error symbol, then shift
      **    the error symbol.
      **
      **  * Set the error count to three.
      **
      **  * Begin accepting and shifting new tokens.  No new error
      **    processing will occur until three tokens have been
      **    shifted successfully.
      **
      */
      if( yypParser->yyerrcnt<0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yymx = yypParser->yystack[yypParser->yyidx].major;
      if( yymx==YYERRORSYMBOL || yyerrorhit ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE,"%sDiscard input token %s\n",
             yyTracePrompt,yyTokenName[yymajor]);
        }
#endif
        yy_destructor(yymajor,&yyminorunion);
        yymajor = YYNOCODE;
      }else{
         while(
          yypParser->yyidx >= 0 &&
          yymx != YYERRORSYMBOL &&
          (yyact = yy_find_reduce_action(
                        yypParser->yystack[yypParser->yyidx].stateno,
                        YYERRORSYMBOL)) >= YYNSTATE
        ){
          yy_pop_parser_stack(yypParser);
        }
        if( yypParser->yyidx < 0 || yymajor==0 ){
          yy_destructor(yymajor,&yyminorunion);
          yy_parse_failed(yypParser);
          yymajor = YYNOCODE;
        }else if( yymx!=YYERRORSYMBOL ){
          YYMINORTYPE u2;
          u2.YYERRSYMDT = 0;
          yy_shift(yypParser,yyact,YYERRORSYMBOL,&u2);
        }
      }
      yypParser->yyerrcnt = 3;
      yyerrorhit = 1;
#else  /* YYERRORSYMBOL is not defined */
      /* This is what we do if the grammar does not define ERROR:
      **
      **  * Report an error message, and throw away the input token.
      **
      **  * If the input token is $, then fail the parse.
      **
      ** As before, subsequent error messages are suppressed until
      ** three input tokens have been successfully shifted.
      */
      if( yypParser->yyerrcnt<=0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yypParser->yyerrcnt = 3;
      yy_destructor(yymajor,&yyminorunion);
      if( yyendofinput ){
        yy_parse_failed(yypParser);
      }
      yymajor = YYNOCODE;
#endif
    }else{
      yy_accept(yypParser);
      yymajor = YYNOCODE;
    }
  }while( yymajor!=YYNOCODE && yypParser->yyidx>=0 );
  return;
}
