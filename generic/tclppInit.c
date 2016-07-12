/*
 * tclppInit.c --
 *
 *      This file implements the initialization for the PP extension. It
 *      registers all the commands in the current interpreter.
 *
 * RCS: $Id: tclppInit.c,v 1.2 2000/05/23 19:34:13 stefan Exp $
 *
 * Copyright (C) 1998-2000, Stefan Sinnige.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * the Free Software Foundation; either version 2.1 of the License, or
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

#include "tclppInt.h"

static char rcsid[] = "$Id: tclppInit.c,v 1.2 2000/05/23 19:34:13 stefan Exp $";

/*
 * The following structure defines the commands in the Tclpp core. 
 */

typedef struct {
    char *name;                 /* The name of the command, NULL marks end */
    Tcl_ObjCmdProc *objProcPtr; /* The object-based command procedure */
} TclppCmdInfo;

static TclppCmdInfo globalCmds[] = {
    {"class", TclppClassCmd},
    {"classinfo", TclppClassInfoCmd},
    {"delete", TclppDeleteCmd},
    {"new", TclppNewCmd},
    {"objectinfo", TclppObjectInfoCmd},
    {NULL, (Tcl_ObjCmdProc*) NULL}
};

static TclppCmdInfo tclppCmds[] = {
    {"tclpp::proc", TclppProcCmd},
    {"tclpp::variable", TclppVariableCmd},
    {"tclpp::virtual", TclppVirtualCmd},
    {NULL, (Tcl_ObjCmdProc*) NULL}
};


/*
 * ----------------------------------------------------------------------------
 *
 * Tclpp_Init --
 *
 *     Initialization routine for the Tclpp extension package. First it checks
 *     if the correct Tcl (8.x) is currently running, since it requires com-
 *     mands from this Tcl version (like the objects and namespace features).
 *     Then it will register all the commands defined in this package.
 *     Finally it will provide package information to the interpreter.
 *
 * Results:
 *     If the package could be initialized properly, it returns TCL_OK. If it
 *     fails, it returns TCL_ERROR and an error message is left at the inter-
 *     preters result string.
 * 
 * Side effects:
 *     None.
 *
 * ----------------------------------------------------------------------------
 */

int
Tclpp_Init (interp)
    Tcl_Interp* interp;         /* The interpreter to add the commands to */
{
    Tcl_CmdInfo    cmdInfo;     /* Information about a certain command */
    Tcl_Namespace *tclppNSPtr;  /* The Tclpp namespace to create */
    TclppCmdInfo *cmdInfoPtr;   /* Information about a Tclpp defined command */
 
    /*
     * Check the package dependency of Tcl 8.x.
     */
    
    if (Tcl_PkgRequire(interp, "Tcl", "8.0", 0) == (char *) NULL) {
        return TCL_ERROR;
    }

    /*
     * Register all the global commands
     */
  
    for (cmdInfoPtr = globalCmds; cmdInfoPtr->name != NULL; cmdInfoPtr++) {
        if (Tcl_GetCommandInfo(interp, cmdInfoPtr->name, &cmdInfo)) {
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "command \"", 
                    cmdInfoPtr->name, "\" already exists", (char *) NULL); 
            return TCL_ERROR;
        }
        Tcl_CreateObjCommand(interp, cmdInfoPtr->name, cmdInfoPtr->objProcPtr,
            (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
    }

    /* 
     * Create the Tclpp namespace and add all the commands. 
     */
    
    tclppNSPtr = Tcl_CreateNamespace(interp, "tclpp", (ClientData) NULL,
            (Tcl_NamespaceDeleteProc *) NULL);
    if (tclppNSPtr == (Tcl_Namespace*) NULL) {
        return TCL_ERROR;
    }

    /*
     * Register all commands in the Tclpp namespace
     */
 
    for (cmdInfoPtr = tclppCmds; cmdInfoPtr->name != NULL; cmdInfoPtr++) {
        if (Tcl_GetCommandInfo(interp, cmdInfoPtr->name, &cmdInfo)) {
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "command \"", 
                    cmdInfoPtr->name, "\" already exists", (char *) NULL); 
            return TCL_ERROR;
        }
        Tcl_CreateObjCommand(interp, cmdInfoPtr->name, cmdInfoPtr->objProcPtr,
                (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
    }

    /* 
     * Export all the commands from the Tclpp namespace. They will be
     * imported by all the namespaces created in the 'class' command.
     */

    if (Tcl_Export(interp, tclppNSPtr, "*", 0) != TCL_OK) {
        return TCL_ERROR;
    }
    
    /*
     * Specify the Tclpp version variables 'tclpp_version' and
     * 'tclpp_patchLevel'.
     */

    Tcl_SetVar (interp, "tclpp_version", TCLPP_VERSION, TCL_GLOBAL_ONLY);
    Tcl_SetVar (interp, "tclpp_patchLevel", TCLPP_PATCH_LEVEL, TCL_GLOBAL_ONLY);    
    /*
     * All initialized properly, so provide the package information to the
     * interpreter.
     */
    
    if (Tcl_PkgProvide (interp, "Tclpp", TCLPP_VERSION) != TCL_OK) {
        return TCL_ERROR;
    }

    return TCL_OK;
}

/*
 * vi: set expandtab ts=4: 
 */

