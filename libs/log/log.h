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

/* Description:
 * 	This header file includes a handful of error checking and logging macros
 *	that are used through-out this application.
 *
 * Usage:
 *	- The message level can be selected by setting MESSAGE_LEVEL before
 *		including this header file, i.e.:
 *			#define MESSAGE_LEVEL 2
 *			#include "log.h"
 * - The supported message levels are:
 *		4 - Print everything
 *		3 - Mask Debug / Mark Messages
 *		2 - Additionally mask Info Messages
 *		1 - Additionally mask Warning Messages
 *		0 - Mask all messages
 * - There are two global variables used by these macros, fderr and fdlog
 *		which are pointers to opened logging files. The c file with the main
 *		function should have the LogMaster keyword following the include, i.e.:
 *			#include "log.h"
 *			LogMaster;
 *		so that the global varibles will be defined.
 *
 * Macros:
 * NOTE: (args...) below indicates a printf-like argument set, i.e. ("%s", str)
 *	MOD(x,y)
 *		-- modulus of x to y, same as '%' opertor but can be used in Return...
 *			functions, the % operator confuses printf
 *	LOG_FDERR
 *		-- FILE pointer for the error log file
 *	LOG_FDLOG
 *		-- FILE pointer for the log file
 *	OpenErrorFile(file)
 *		-- Opens a file for printing error and warning messges to, if not 
 *			included stderr will be used.
 *	CloseErrorFile	
 *		-- Closes the error file, if it was opened.
 *	OpenLogFile(file) 
 *		-- Opens a file for printing info and debug messges to, if not 
 *			included stdout will be used.
 *	CloseLogFile
 *		-- Closes the error file, if it was opened.
 * 	Error(args...)
 *		-- Prints an error message, including line number and file name.
 *	Warn(args...)
 *		-- Prints an waring message, including line number and file name.
 *	Info(args...)
 *		-- Prints an info message, with no formatting.
 *	Debug(args...)
 *		-- Prints an debug message, including line number and file name.
 *	Mark
 *		-- Prints an Mark label, including line number and file name.
 *	Text(args...)
 *		-- Prints unformatted text, doesn't include a carriage-return.
 *	ReturnErrIf(expr, args...)
 *		-- Prints an Error message including the expression and returns a -1
 *			if the expression is true.
 *	ReturnErr(args...)
 *		-- Prints an Error message and returns a -1.
 *	ReturnErrAndFreeIf(ptr, expr, args...)
 *		-- Prints an Error message including the expression, frees the memory
 *			pointed to by ptr and returns a -1 if the expression is true.
 *	ReturnNULLIf(expr, args...)
 *		-- Prints an Error message including the expression and returns a NULL
 *			if the expression is true.
 *	ReturnNULL(args...)
 *		-- Prints an Error message and returns a NULL.
 *	ReturnNULLAndFreeIf(ptr, expr, args...)
 *		-- Prints an Error message including the expression, frees the memory
 *			pointed to by ptr and returns a NULL if the expression is true.
 *	ReturnNaNIf(expr, args...)
 *		-- Prints an Error message including the expression and returns a nan
 *			if the expression is true. 
 *		-- must include math.h to use
 *	ReturnNaN(args...)
 *		-- Prints an Error message and returns a nan. 
 *		-- must include math.h to use
 *	GotoFailedIf(expr, args...)
 *		-- Prints an Error message including the expression and "goto failed"
 *			if the expression is true.
 *	ExitFailureIf(expr, args...)
 *		-- Prints an Error message including the expression and termintes the
 *			application with a "exit(EXIT_FAILURE)" if the expression is true. 
 *			To be used in the main function.
 *	ExitFailure(args...)
 *		-- Prints an Error message and termintes the application with a 
 *			"exit(EXIT_FAILURE)". To be used in the main function.
 *	ExitSuccess
 *		-- Exits an application with no error codes, i.e. "exit(EXIT_SUCCESS)"
 *	LogInfo()
 *		-- Prints out version info for log library.
 */

#ifndef LOG_H
#define LOG_H

#define LOG_MAJOR_VERSION		1
#define LOG_MINOR_VERSION		9

#ifndef ML
#	define MESSAGE_LEVEL	3
#else
#	define MESSAGE_LEVEL	ML
#endif


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define M2STRX(a)	#a
#define M2STR(a)	M2STRX(a)
#define __LINESTR__	M2STR(__LINE__)

#define MOD(x,y) x%y

#define LOG_FDERR (fderr ? fderr : stderr)
#define LOG_FDLOG (fdlog ? fdlog : stdout)

#if MESSAGE_LEVEL > 0
extern FILE *fderr;
#	define LogMaster \
	FILE *fderr = NULL; \
	FILE *fdlog = NULL
#	define OpenErrorFile(file) \
	fderr = fopen(file, "w")
#	define CloseErrorFile \
	if(fderr != NULL) { \
		fclose(fderr); \
		fderr = NULL; \
	}
#	define Error(args...) \
		fprintf(LOG_FDERR, \
				"ERROR:" M2STR(LIBNAME) " "__FILE__":"__LINESTR__"\t" args);\
		fprintf(LOG_FDERR, "\n");\
		(void)fflush(LOG_FDERR)
#else
#	define LogMaster
#	define OpenErrorFile(file)
#	define CloseErrorFile
#	define Error(args...)
#endif

#if MESSAGE_LEVEL > 1
#	define Warn(args...) \
		fprintf(LOG_FDERR, \
				"WARNING:" M2STR(LIBNAME) " "__FILE__":"__LINESTR__"\t" args);\
		fprintf(LOG_FDERR, "\n");\
		(void)fflush(LOG_FDERR)
#else
#	define Warn(args...)
#endif

#if MESSAGE_LEVEL > 2
extern FILE *fdlog;
#	define OpenLogFile(file) \
	fdlog = fopen(file, "w")
#	define CloseLogFile \
	if(fdlog != NULL) { \
		fclose(fdlog); \
		fdlog = NULL; \
	}
#	define Info(args...) \
		fprintf(LOG_FDLOG, args);\
		fprintf(LOG_FDLOG, "\n");\
		(void)fflush(LOG_FDLOG)
#else
#	define OpenLogFile(file)
#	define CloseLogFile
#	define Info(args...)
#endif

#if MESSAGE_LEVEL > 3
#	define Debug(args...) \
		fprintf(LOG_FDLOG, \
				"DEBUG:" M2STR(LIBNAME) " "__FILE__":"__LINESTR__"\t" args);\
		fprintf(LOG_FDLOG, "\n");\
		(void)fflush(LOG_FDLOG)
#	define Mark \
		fprintf(LOG_FDLOG, \
				">>>> MARK:" M2STR(LIBNAME) " "__FILE__":"__LINESTR__ "<<<<");\
		fprintf(LOG_FDLOG, "\n");\
		(void)fflush(LOG_FDLOG)
#	define Text(args...) \
		fprintf(LOG_FDLOG, args);	
#else
#	define Debug(args...)
#	define Text(args...)
#	define Mark
#endif

#define WarnIf(expr, args...) \
	if(expr) {\
		Warn(#expr " -- " args);\
	}

#define ReturnErrIf(expr, args...) \
	if(expr) {\
		Error(#expr " -- " args);\
		return -1;\
	}

#define ReturnErr(args...) \
	Error(args);\
	return -1

#define ReturnErrAndFreeIf(ptr, expr, args...) \
	if(expr) {\
		Error(#expr " -- " args);\
		free(ptr);\
		return -1;\
	}

#define ReturnNULLIf(expr, args...) \
	if(expr) {\
		Error(#expr " -- " args);\
		return NULL;\
	}

#define ReturnNULL(args...) \
	Error(args);\
	return NULL

#define ReturnNULLAndFreeIf(ptr, expr, args...) \
	if(expr) {\
		Error(#expr " -- " args);\
		free(ptr);\
		return NULL;\
	}

#ifdef  M_E
#define ReturnNaNIf(expr, args...) \
	if(expr) {\
		Error(#expr " -- " args);\
		return sqrtf(-1.f);\
	}

#define ReturnNaN(args...) \
	Error(args);\
	return sqrtf(-1.f)

#endif

#define GotoFailedIf(expr, args...) \
	if (expr) {\
		Error(#expr " -- " args);\
		goto failed;\
	}

#define ExitFailureIf(expr, args...) \
	if(expr) {\
		Error(#expr " -- " args);\
		exit(EXIT_FAILURE);\
	}

#define ExitFailure(args...) \
	Error(args);\
	exit(EXIT_FAILURE)

#define ExitSuccess \
	exit(EXIT_SUCCESS)

#define LogInfo() \
	Info("Log Library %i.%i", LOG_MAJOR_VERSION, LOG_MINOR_VERSION); \
	Info("Compiled " __DATE__ " at " __TIME__); \
	Info("(c) 2006 Cooper Street Innovations Inc.")

#endif
