/*
 * tclAppInit.c --
 *
 *     Provides a default version of the main program and Tcl_AppInit
 *     procedure for Tclpp applications.
 *
 * RCS: $Id: tclAppInit.c,v 1.3 2000/06/29 21:13:31 stefan Exp $
 *
 * Copyright (C) 1998-2000, Stefan Sinnige.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 * For the full GNU Lesser General Public License, see the 'LICENSE' file.
 *
 */

#include <tcl.h>
#ifdef WITH_TK
#   include <tk.h>
#endif
#include <windows.h>
#include <locale.h>
#include "tclpp.h"

static char rcsid[] = "$Id: tclAppInit.c,v 1.3 2000/06/29 21:13:31 stefan Exp $";

static BOOL consoleRequired = TRUE;


/*
 *-------------------------------------------------------------------------
 *
 * setargv --
 *
 *	Parse the Windows command line string into argc/argv.  Done here
 *	because we don't trust the builtin argument parser in crt0.  
 *	Windows applications are responsible for breaking their command
 *	line into arguments.
 *
 *	2N backslashes + quote -> N backslashes + begin quoted string
 *	2N + 1 backslashes + quote -> literal
 *	N backslashes + non-quote -> literal
 *	quote + quote in a quoted string -> single quote
 *	quote + quote not in quoted string -> empty string
 *	quote -> begin quoted string
 *
 * Results:
 *	Fills argcPtr with the number of arguments and argvPtr with the
 *	array of arguments.
 *
 * Side effects:
 *	Memory allocated.
 *
 *--------------------------------------------------------------------------
 */

static void
setargv(argcPtr, argvPtr)
    int *argcPtr;		/* Filled with number of argument strings. */
    char ***argvPtr;	/* Filled with argument strings (malloc'd). */
{
    char *cmdLine, *p, *arg, *argSpace;
    char **argv;
    int argc, size, inquote, copy, slashes;
    
    cmdLine = GetCommandLine();	/* INTL: BUG */

    /*
     * Precompute an overly pessimistic guess at the number of arguments
     * in the command line by counting non-space spans.
     */

    size = 2;
    for (p = cmdLine; *p != '\0'; p++) {
	    if ((*p == ' ') || (*p == '\t')) {	
            /* INTL: ISO space. */
	        size++;
	        while ((*p == ' ') || (*p == '\t')) { 
                /* INTL: ISO space. */
		        p++;
	        }
	        if (*p == '\0') {
		        break;
	        }
	    }
    }
    argSpace = (char *) Tcl_Alloc(
	    (unsigned) (size * sizeof(char *) + strlen(cmdLine) + 1));
    argv = (char **) argSpace;
    argSpace += size * sizeof(char *);
    size--;

    p = cmdLine;
    for (argc = 0; argc < size; argc++) {
	    argv[argc] = arg = argSpace;
	    while ((*p == ' ') || (*p == '\t')) {	
            /* INTL: ISO space. */
	        p++;
	    }
	    if (*p == '\0') {
	        break;
	    }

	    inquote = 0;
	    slashes = 0;
	    while (1) {
	        copy = 1;
	        while (*p == '\\') {
		        slashes++;
		        p++;
	        }
	        if (*p == '"') {
		        if ((slashes & 1) == 0) {
		            copy = 0;
		            if ((inquote) && (p[1] == '"')) {
			            p++;
			            copy = 1;
		            } else {
			            inquote = !inquote;
		            }
                }
                slashes >>= 1;
            }
    
            while (slashes) {
		        *arg = '\\';
		        arg++;
		        slashes--;
	        }
    
	        if ((*p == '\0') || (!inquote && ((*p == ' ') || (*p == '\t')))) { 
                /* INTL: ISO space. */
		        break;
	        }
	        if (copy != 0) {
		        *arg = *p;
		        arg++;
	        }
	        p++;
        }
	    *arg = '\0';
	    argSpace = arg + 1;
    }
    argv[argc] = NULL;

    *argcPtr = argc;
    *argvPtr = argv;
}


/*
 * ----------------------------------------------------------------------------
 *
 * Tcl_AppInit --
 *
 *     This procedure performs application-specific initialization.
 *     Most applications, especially those that incorporate additional
 *     packages, will have their own version of this procedure.
 *
 * Results:
 *     Returns a standard Tclpp completion code, and leaves an error
 *     message in the interpreters result string if an error occurs.
 *
 * Side effects:
 *     Depends on the startup script.
 *
 * ----------------------------------------------------------------------------
 */

static int
Tcl_ShowError(interp)
    Tcl_Interp *interp;         /* Interpreter for application. */
{
#ifdef WITH_TK
    MessageBeep(MB_ICONEXCLAMATION);
    MessageBox(NULL, Tcl_GetStringResult(interp), "Error in Tkpp",
        MB_ICONSTOP | MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);
    ExitProcess(1);
#endif
    return TCL_ERROR;
}

int
Tcl_AppInit(interp)
    Tcl_Interp *interp;         /* Interpreter for application. */
{
    if (Tcl_Init(interp) == TCL_ERROR) {
        return Tcl_ShowError(interp);
    }
#ifdef WITH_TK
    if (Tk_Init(interp) == TCL_ERROR) {
        return Tcl_ShowError(interp);
    }
    Tcl_StaticPackage(interp, "Tk", Tk_Init, Tk_SafeInit);

    /*
     * Initialize the console only if we are running interactively
     */

    if (consoleRequired) {
        if (Tk_CreateConsoleWindow(interp) == TCL_ERROR) {
            return Tcl_ShowError(interp);
        }

    }
#endif

    /*
     * Call the init procedures for included packages.  Each call should
     * look like this:
     *
     * if (Mod_Init(interp) == TCL_ERROR) {
     *     return TCL_ERROR;
     * }
     *
     * where "Mod" is the name of the module.
     */

    if (Tclpp_Init(interp) == TCL_ERROR) {
        return Tcl_ShowError(interp);
    }

    /*
     * Specify a user-specific startup file to invoke if the application
     * is run interactively.  Typically the startup file is "~/.apprc"
     * where "app" is the name of the application.  If this line is deleted
     * then no user-specific startup file will be run under any conditions.
     */

#ifdef WITH_TK
    Tcl_SetVar(interp, "tcl_rcFileName", "~/tkpprc.tcl", TCL_GLOBAL_ONLY);
#else
    Tcl_SetVar(interp, "tcl_rcFileName", "~/tclshrc.tcl", TCL_GLOBAL_ONLY);
#endif
    return TCL_OK;
}


/*
 * ----------------------------------------------------------------------------
 *
 * WinMain --
 *
 *      Main entry point for Windows.
 *
 * Results:
 *     None. Tcl_Main never returns here, so this procedure never
 *     return either.
 *
 * Side effects:
 *     Whatever the application does.
 *
 * ----------------------------------------------------------------------------
 */

#ifdef WITH_TK

int APIENTRY
WinMain(hInstance, hPrevInstance, lpszCmdLine, nCmdShow)
    HINSTANCE hInstance;
    HINSTANCE hPrevInstance;
    LPSTR lpszCmdLine;
    int nCmdShow;
{
    char **argv;
    int argc;
    char buffer[MAX_PATH+1];
    char *p;

    consoleRequired = TRUE;

    /*
     * Set up the default locale to be standard 'C' so parsing is
     * performed correctly.
     */

    setlocale (LC_ALL, "C");
    setargv(&argc, &argv);

    /*
     * Replace argv[0] with full pathname of executable, and substitute
     * forward slashes for backslashed.
     */

    GetModuleFileName (NULL, buffer, sizeof(buffer));
    argv[0] = buffer;
    for (p = buffer; *p != '\0'; p++) {
	    if (*p == '\\') {
	        *p = '/';
	    }
    }

    Tk_Main(argc, argv, Tcl_AppInit);

    return 0;           /* Needed only to prevent compiler warning. */
}

#endif


/*
 * ----------------------------------------------------------------------------
 *
 * main --
 *
 *     This  the main program for the application.
 *
 * Results:
 *     None. Tcl_Main never returns here, so this procedure never
 *     return either.
 *
 * Side effects:
 *     Whatever the application does.
 *
 * ----------------------------------------------------------------------------
 */

int
main(argc, argv)
    int argc;                   /* Number of command-line arguments. */
    char **argv;                /* Values of command-line arguments. */
{
    char buffer[MAX_PATH + 1];
    char *p;

    /*
     * Set up the default locale to be standard 'C' so parsing is
     * performed correctly.
     */

    setlocale (LC_ALL, "C");
    setargv(&argc, &argv);

    /*
     * Replace argv[0] with full pathname of executable, and substitute
     * forward slashes for backslashed.
     */

    GetModuleFileName (NULL, buffer, sizeof(buffer));
    argv[0] = buffer;
    for (p = buffer; *p != '\0'; p++) {
	    if (*p == '\\') {
	        *p = '/';
	    }
    }

#ifdef WITH_TK
    consoleRequired = FALSE;
    Tk_Main(argc, argv, Tcl_AppInit);
#else
    Tcl_Main(argc, argv, Tcl_AppInit);
#endif
    return 0;           /* Needed only to prevent compiler warning. */
}

/*
 * vi: set ai expandtab ts=4:
 */
