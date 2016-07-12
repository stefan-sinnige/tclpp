/*
 * tclAppInit.c --
 *
 *     Provides a default version of the main program and Tcl_AppInit
 *     procedure for Tclpp applications.
 *
 * RCS: $Id: tclAppInit.c,v 1.3 2000/06/29 21:14:26 stefan Exp $
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
#include <tk.h>
#endif
#include "tclpp.h"

static char rcsid[] = "$Id: tclAppInit.c,v 1.3 2000/06/29 21:14:26 stefan Exp $";


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

int
Tcl_AppInit(interp)
    Tcl_Interp *interp;         /* Interpreter for application. */
{
    if (Tcl_Init(interp) == TCL_ERROR) {
        return TCL_ERROR;
    }
#ifdef WITH_TK
    if (Tk_Init(interp) == TCL_ERROR) {
        return TCL_ERROR;
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
        return TCL_ERROR;
    }

    /*
     * Specify a user-specific startup file to invoke if the application
     * is run interactively.  Typically the startup file is "~/.apprc"
     * where "app" is the name of the application.  If this line is deleted
     * then no user-specific startup file will be run under any conditions.
     */

#ifdef WITH_TK
    Tcl_SetVar(interp, "tcl_rcFileName", "~/.wishrc", TCL_GLOBAL_ONLY);
#else
    Tcl_SetVar(interp, "tcl_rcFileName", "~/.tclshrc", TCL_GLOBAL_ONLY);
#endif
    return TCL_OK;
}

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
#ifdef WITH_TK
    Tk_Main(argc, argv, Tcl_AppInit);
#else
    Tcl_Main(argc, argv, Tcl_AppInit);
#endif
    return 0;                   /* Needed only to prevent compiler warning. */
}

/*
 * vi: set ai expandtab ts=4:
 */
