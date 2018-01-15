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
#include <data.h>
#include <ctype.h>

#include "tokenizer.h"
#include "parser.h"

/* RE Format parser defines */
#define	YYCTYPE		char
#define	YYCURSOR	cursor
#define	YYMARKER	marker

/*!re2c /*!ignore!re2c Set RE2C Options */
	re2c:yyfill:enable=0;
*/

/*!re2c /*!ignore!re2c Useful Regular Expressions */
	EOI = [\000];
	EOL = [\012] | [\015];
	ANY	= [\000-\377];
	ALPHA	= ([a-zA-Z_] | ":" | "#" | "@");
	DIGIT	= [0-9];
	DECIMAL = ((DIGIT+ ("." DIGIT+)?) | ("." DIGIT+));
	FLOAT =  DECIMAL ('e' ("-"|"+")? DIGIT+)?;
	UNITS = ALPHA*;
	TEXT = (ALPHA | DIGIT );
	WSP = (" ") | [\011];
	VARIABLE = ALPHA TEXT*;
	CONTINUE = EOL+ "+";
*/

static token_ * tokenNew(int type, double **variable, double constant,
		double *minDiv)
{
	token_ *r;
	r = malloc(sizeof(token_));
	ReturnNULLIf(r == NULL, "Malloc Failed");
	r->type = type;
	r->variable = variable;
	r->constant = constant;
	r->minDiv = minDiv;
	return r;
}


int tokenSolve(token_ *r, tokenSolveArgs_ *args)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(args == NULL);

	if(args->diffVariable != NULL) {
		if((r->type == TOKEN_VARIABLE) &&
				(*(r->variable) == *(args->diffVariable))) {
			Debug("%s", ParseTokenName(TOKEN_DIFF_VARIABLE));
			Parse(args->parser, TOKEN_DIFF_VARIABLE, r, args->solution);
			ReturnErrIf(isnan(*args->solution), "Parser failed");
			return 0;
		}
	}

	Debug("%s", ParseTokenName(r->type));
	Parse(args->parser, r->type, r, args->solution);
	ReturnErrIf(isnan(*args->solution), "Parser failed");
	return 0;
}

int tokenDestroy(token_ *r)
{
	if(r != NULL) {
		free(r);
	}
	return 0;
}

#define Attach(tok, var, con, minDiv) \
	GotoFailedIf(listAdd(tokens, tokenNew(tok, var, con, minDiv), NULL))

list_ * tokenizerNew(char *buffer, tokenizerGetVarPtr getVarFunc,
		void *private, double *minDiv)
{
	list_ *tokens = NULL;
	char *cursor = NULL, *marker = NULL, *start = NULL, *tmp, *comma, *lparen;
	double **variable;
	int i, c;

	ReturnNULLIf(buffer == NULL);
	ReturnNULLIf(getVarFunc == NULL);

	Debug("Creating tokenizer");
	tokens = listNew(tokens);
	ReturnNULLIf(tokens == NULL);
	cursor = buffer;
	marker = buffer;

parse:
	start = cursor;
/*!re2c /*!ignore!re2c Parse Block */
	(WSP+ | CONTINUE)	{ goto parse; }
	EOI 	{ Attach(0, NULL, 0, NULL);						goto passed; }
	'abs('	{ Attach(TOKEN_ABS, NULL, 0, minDiv);		goto parse; }
	'acosh('{ Attach(TOKEN_ACOSH, NULL, 0, minDiv);	goto parse; }
	'acos('	{ Attach(TOKEN_ACOS, NULL, 0, minDiv);		goto parse; }
	'asinh('{ Attach(TOKEN_ASINH, NULL, 0, minDiv);	goto parse; }
	'asin('	{ Attach(TOKEN_ASIN, NULL, 0, minDiv);		goto parse; }
	'atanh('{ Attach(TOKEN_ATANH, NULL, 0, minDiv);	goto parse; }
	'atan('	{ Attach(TOKEN_ATAN, NULL, 0, minDiv);		goto parse; }
	'cosh('	{ Attach(TOKEN_COSH, NULL, 0, minDiv);		goto parse; }
	'cos('	{ Attach(TOKEN_COS, NULL, 0, minDiv);		goto parse; }
	'exp('	{ Attach(TOKEN_EXP, NULL, 0, minDiv);		goto parse; }
	'ln('	{ Attach(TOKEN_LN, NULL, 0, minDiv);		goto parse; }
	'log('	{ Attach(TOKEN_LOG, NULL, 0, minDiv);		goto parse; }
	'sinh('	{ Attach(TOKEN_SINH, NULL, 0, minDiv);		goto parse; }
	'sin('	{ Attach(TOKEN_SIN, NULL, 0, minDiv);		goto parse; }
	'sqrt('	{ Attach(TOKEN_SQRT, NULL, 0, minDiv);		goto parse; }
	'tan('	{ Attach(TOKEN_TAN, NULL, 0, minDiv);		goto parse; }
	'uramp('{ Attach(TOKEN_URAMP, NULL, 0, minDiv);	goto parse; }
	'u('	{ Attach(TOKEN_U, NULL, 0, minDiv);		goto parse; }
	'if('	{ Attach(TOKEN_IF, NULL, 0, minDiv);		goto parse; }
	"+" 	{ Attach(TOKEN_PLUS, NULL, 0, minDiv);		goto parse; }
	"-" 	{ Attach(TOKEN_MINUS, NULL, 0, minDiv);	goto parse; }
	"*" 	{ Attach(TOKEN_TIMES, NULL, 0, minDiv);	goto parse; }
	"/" 	{ Attach(TOKEN_DIVIDE, NULL, 0, minDiv);	goto parse; }
	"^" 	{ Attach(TOKEN_POWER, NULL, 0, minDiv);	goto parse; }
	"(" 	{ Attach(TOKEN_LPAREN, NULL, 0, NULL);		goto parse; }
	")" 	{ Attach(TOKEN_RPAREN, NULL, 0, NULL);		goto parse; }
	">" 	{ Attach(TOKEN_GREATERTHAN, NULL, 0, NULL);		goto parse; }
	"<" 	{ Attach(TOKEN_LESSTHAN, NULL, 0, NULL);		goto parse; }
	"=" 	{ Attach(TOKEN_EQUAL, NULL, 0, NULL);			goto parse; }
	"!" 	{ Attach(TOKEN_NOT, NULL, 0, NULL);			goto parse; }
	"&" 	{ Attach(TOKEN_AND, NULL, 0, NULL);		goto parse; }
	"|" 	{ Attach(TOKEN_OR, NULL, 0, NULL);		goto parse; }
	FLOAT 'T' UNITS { Attach(TOKEN_CONSTANT, NULL, atof(start)*1e12, NULL); goto parse; }
	FLOAT 'G' UNITS { Attach(TOKEN_CONSTANT, NULL, atof(start)*1e9, NULL); goto parse; }
	FLOAT 'k' UNITS { Attach(TOKEN_CONSTANT, NULL, atof(start)*1e3, NULL); goto parse; }
	FLOAT 'u' UNITS { Attach(TOKEN_CONSTANT, NULL, atof(start)*1e-6, NULL); goto parse; }
	FLOAT 'n' UNITS { Attach(TOKEN_CONSTANT, NULL, atof(start)*1e-9, NULL); goto parse; }
	FLOAT 'p' UNITS { Attach(TOKEN_CONSTANT, NULL, atof(start)*1e-12, NULL); goto parse; }
	FLOAT 'f' UNITS { Attach(TOKEN_CONSTANT, NULL, atof(start)*1e-15, NULL); goto parse; }
	FLOAT 'Meg' UNITS { Attach(TOKEN_CONSTANT, NULL, atof(start)*1e6, NULL); goto parse; }
	FLOAT 'mil' UNITS { Attach(TOKEN_CONSTANT, NULL, atof(start)*2.54e-5, NULL); goto parse; }
	FLOAT 'm' UNITS { Attach(TOKEN_CONSTANT, NULL, atof(start)*1e-3, NULL); goto parse; }
	FLOAT UNITS { Attach(TOKEN_CONSTANT, NULL, atof(start), NULL); goto parse; }
	'v(' TEXT+ ',' TEXT+ ')' {
		/* TODO: This is a little inefficient, could clean it up someday, use
			re2c, there's no reason to go over this twice... but then the
			performance hit is so small who cares? */
		for(lparen = start; *lparen != '('; lparen++);
		for(comma = lparen; *comma != ','; comma++);

		/* First get left variable */
		tmp = malloc(comma - start + 2);
		GotoFailedIf(tmp == NULL);
		strncpy(tmp, start, (comma - start));
		/* change function name to lower case */
		for(i = 0; tmp[i] != '('; i++) {
			c = tolower(tmp[i]);
			GotoFailedIf(c == EOF);
			tmp[i] = c;
		}
		tmp[comma - start] = ')';
		tmp[comma - start + 1] = 0x0;
		variable = getVarFunc(tmp, private);
		GotoFailedIf(variable == NULL);
		Attach(TOKEN_LPAREN, NULL, 0, NULL)
		Attach(TOKEN_VARIABLE, variable, 0, NULL);

		/* Minus */
		Attach(TOKEN_MINUS, NULL, 0, minDiv);

		/* the right variable */
		tmp = malloc((lparen - start) + (cursor - comma) + 2);
		GotoFailedIf(tmp == NULL);
		/* NOTE: tmp is freed by the function it's passed to */
		strncpy(tmp, start, (lparen - start + 1));
		Debug("%s", tmp);
		strncpy(&tmp[(lparen - start)+1], comma+1, (cursor - comma));
		Debug("%s", tmp);
		/* change function name to lower case */
		for(i = 0; tmp[i] != '('; i++) {
			c = tolower(tmp[i]);
			GotoFailedIf(c == EOF);
			tmp[i] = c;
		}
		tmp[lparen - start + cursor - comma] = 0x0;
		variable = getVarFunc(tmp, private);
		GotoFailedIf(variable == NULL);
		Attach(TOKEN_VARIABLE, variable, 0, NULL);
		Attach(TOKEN_RPAREN, NULL, 0, NULL);
		goto parse;
	}
	('v' | 'i') '('TEXT+')' {
		tmp = malloc(cursor - start + 1);
		GotoFailedIf(tmp == NULL);
		/* NOTE: tmp is freed by the function it's passed to */
		strncpy(tmp, start, (cursor - start));
		/* change function name to lower case */
		for(i = 0; tmp[i] != '('; i++) {
			c = tolower(tmp[i]);
			GotoFailedIf(c == EOF);
			tmp[i] = c;
		}
		tmp[cursor - start] = 0x0;
		variable = getVarFunc(tmp, private);
		GotoFailedIf(variable == NULL);
		Attach(TOKEN_VARIABLE, variable, 0, NULL);
		goto parse;
	}
	VARIABLE {
		tmp = malloc(cursor - start + 1);
		GotoFailedIf(tmp == NULL);
		/* NOTE: tmp is freed by the function it's passed to */
		strncpy(tmp, start, (cursor - start));
		tmp[cursor - start] = 0x0;
		variable = getVarFunc(tmp, private);
		GotoFailedIf(variable == NULL);
		Attach(TOKEN_VARIABLE, variable, 0, NULL);
		goto parse;
	}
	ANY 	{ Error("Unexpected character %s", cursor-1);	goto failed; }
*/

passed:
	return tokens;
failed:
	if(tokens != NULL) {
		if(listDestroy(&tokens, (listDestroy_)tokenDestroy)) {
			Warn("Failed to destroy token list");
		}
	}
	return NULL;
}
