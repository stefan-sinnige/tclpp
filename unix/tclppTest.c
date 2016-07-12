/*
 * tclppTest.c --
 *
 *     Provides a set of tests to test the Tclpp C API and provide the
 *     'cleanup' and generic 'test' procedure.
 *
 * RCS: $Id: tclppTest.c,v 1.1 2000/05/23 19:33:32 stefan Exp $
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

#include <limits.h>
#include <sys/times.h>
#include <sys/time.h>
#include "tclpp.h"
#include "tclppInt.h"

static char rcsid[] = "$Id: tclppTest.c,v 1.1 2000/05/23 19:33:32 stefan Exp $";


/*
 * ----------------------------------------------------------------------------
 *
 * TestObjCmd --
 *
 *     Generic test command. The command has the following syntax:
 *
 *          test <name> <description> <script> <answer>
 *
 * Results:
 *     Execute a script and check its result for correctness. If the
 *     result of the script matches the answer, the function outputs
 *     nothing. Otherwise it outputs the test name, description, the
 *     result string, the expected string and the script itself.
 *     It returns TCL_ERROR only when the arguments of the test com-
 *     mand do not match. Return TCL_OK otherwise.
 *
 * Side Effects:
 *     The script is run in a separate interpreter which gets deleted
 *     afterwards. So, no side effects should exist.
 *
 * ----------------------------------------------------------------------------
 */

int
TestObjCmd(clientData, interp, objc, objv)
    ClientData clientData;        /* Not used */
    Tcl_Interp *interp;           /* The interpreter to create the class in */
    int objc;                     /* Number of objects */
    Tcl_Obj * CONST objv[];       /* Array of objects */
{
    Tcl_Interp *testInterp;       /* The interpeter to run the test in */
    Tcl_HashSearch search;        /* Keep track of hash table search */
    Tcl_HashEntry *entryPtr;      /* The entry of the hash table */
    int result;                   /* Result of evaluation */

    /*
     * Check the number of objects.
     */

    if (objc != 5) {
        Tcl_WrongNumArgs(interp, 1, objv, "name description script answer");
        return TCL_ERROR;
    }

    /* 
     * Create and initialize a new interpreter to run the test in
     */

    testInterp = Tcl_CreateInterp();
    if (Tclpp_Init(testInterp) == TCL_ERROR) {
        return TCL_ERROR;
    }
    if (Tclpp_TestInit(testInterp) == TCL_ERROR) {
        return TCL_ERROR;
    }

    /*
     * Execute the script and set the result either to TCL_OK or TCL_ERROR
     * based on the result.
     */

    (void) Tcl_GlobalEvalObj (testInterp, objv[3]);
    if (strcmp(Tcl_GetStringFromObj(Tcl_GetObjResult(testInterp), (int *)NULL),
            Tcl_GetStringFromObj(objv[4], (int *)NULL)) != 0) {
        result = TCL_ERROR;
    } else {
        result = TCL_OK;
    }

    /*
     * If the result is TCL_ERROR, create a result string and output it.
     */
  
    if (result != TCL_OK) {
        Tcl_Channel stdoutChannelId;   /* Channel ID of stdout */
        Tcl_Obj    *outputPtr;         /* The result object to output */
        char       *outputBuf;         /* The result as a (char *) */
        int         outputLength;      /* The result character length */

        stdoutChannelId = Tcl_GetChannel(interp, "stdout", (int*)NULL);
        if (stdoutChannelId == (Tcl_Channel) NULL) {
            return TCL_ERROR;
        }

        outputPtr = Tcl_NewObj();
        Tcl_AppendStringsToObj(outputPtr, "\n**** TEST ", 
                Tcl_GetStringFromObj(objv[1], (int*)NULL), " FAILED:\n",
                Tcl_GetStringFromObj(objv[2], (int*)NULL), 
                "\n**** Test Case:\n",
                Tcl_GetStringFromObj(objv[3], (int*)NULL), 
                "\n**** Result was:\n",
                Tcl_GetStringFromObj(Tcl_GetObjResult(testInterp), (int*)NULL),
                "\n**** but should have been:\n", 
                Tcl_GetStringFromObj(objv[4], (int*)NULL),
                "\n**** TEST ", Tcl_GetStringFromObj(objv[1], (int*)NULL), 
                "\n", (char *) NULL);
        outputBuf = Tcl_GetStringFromObj(outputPtr, &outputLength);
        (void) Tcl_Write(stdoutChannelId, outputBuf, outputLength);
        Tcl_DecrRefCount(outputPtr);
    }

    /*
     * Delete the test interpreter and all associated commands and namespaces.
     */

    Tcl_DeleteInterp (testInterp);
    
    /*
     * Remove any stale class, method and instance references
     */

    for (entryPtr = Tcl_FirstHashEntry(&classTable, &search);
            entryPtr != NULL; 
            entryPtr = Tcl_FirstHashEntry(&classTable, &search)) {
        Tcl_DeleteHashEntry (entryPtr);
    }
    for (entryPtr = Tcl_FirstHashEntry(&instanceTable, &search);
            entryPtr != NULL; 
            entryPtr = Tcl_FirstHashEntry(&instanceTable, &search)) {
        Tcl_DeleteHashEntry (entryPtr);
    }

    /*
     * Exit
     */

    Tcl_ResetResult(interp);
    return TCL_OK;
}


/*
 * ----------------------------------------------------------------------------
 *
 * TestClassCmd
 *      This procedure implements the "testclass" command. It is used to
 *      test the Tclpp_CreateClass command.
 *
 * Results:
 *      A standard Tcl result.
 *
 * Side Effects:
 *      Creates a class.
 *
 */

static int
TestClassCmd (clientData, interp, objc, objv)
    ClientData clientData;      /* Not used. */
    Tcl_Interp *interp;         /* Current interpreter. */
    int objc;                   /* Number of objects. */
    Tcl_Obj *CONST objv[];      /* Argument objects. */
{
    Tclpp_Class* classPtr;      /* The class created. */

    /*
     * Check the command line arguments
     */

    if (objc != 3) {
        Tcl_WrongNumArgs(interp, 1, objv, "name definition");
        return TCL_ERROR;
    }
    
    /*
     * Create the class.
     */

    classPtr = Tclpp_CreateClass (interp, 
            Tcl_GetStringFromObj(objv[1], (int *)NULL), objv[2]);

    if (classPtr == (Tclpp_Class*) NULL) {
        return TCL_ERROR;
    } 

    return TCL_OK;
}


/*
 * ----------------------------------------------------------------------------
 *
 * TestInstanceCmd
 *      This procedure implements the "testinstance" command. It is used to
 *      test the followng commands:
 *
 *          Tclpp_CreateInstance
 *          Tclpp_DeleteInstance
 *
 * Results:
 *      A standard Tcl result.
 *
 * Side Effects:
 *      Creates a class.
 *
 */

static int
TestInstanceCmd (clientData, interp, objc, objv)
    ClientData clientData;      /* Not used. */
    Tcl_Interp *interp;         /* Current interpreter. */
    int objc;                   /* Number of objects. */
    Tcl_Obj *CONST objv[];      /* Argument objects. */
{
    char *subCmd;               /* The sub-command  */
    char *instanceName;         /* The instance name */ 

    /*
     * Check the command line arguments
     */

    if (objc < 3) {
        Tcl_WrongNumArgs(interp, 1, objv, "option instance ?args?");
        return TCL_ERROR;
    }
    
    subCmd = Tcl_GetStringFromObj(objv[1], (int*) NULL);
    instanceName = Tcl_GetStringFromObj(objv[2], (int*) NULL);

    /*
     * Execute the sub-command
     */

    if (strcmp(subCmd, "create") == 0) {
        char* className;             /* The name of the class */
        Tclpp_Class* classPtr;       /* The class to instantiate from */
        Tclpp_Instance* instancePtr; /* The instance created */

        /*
         * Check the number of arguments
         */

        if (objc < 4) {
            Tcl_WrongNumArgs(interp, 2, objv, "instance class ?ct-args?");
            return TCL_ERROR;
        }

        /*
         * Get the class object pointer 
         */

        className = Tcl_GetStringFromObj(objv[3], (int*) NULL);
        classPtr = Tclpp_GetClassByName (interp, className);
        if (classPtr == (Tclpp_Class*) NULL) {
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                    "class \"", className, "\" unknown", (char*) NULL);
            return TCL_ERROR;
        }

        /*
         * Create the instance
         */

        instancePtr = Tclpp_CreateInstance (interp, instanceName, classPtr,
                0, (Tcl_Obj **) NULL);
        if (instancePtr == (Tclpp_Instance*) NULL) {
            return TCL_ERROR;
        }
    }
    else
    if (strcmp(subCmd, "delete") == 0) {
        Tclpp_Instance* instancePtr; /* The instance to delete */

        /*
         * Check the number of arguments
         */

        if (objc != 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "instance");
            return TCL_ERROR;
        }

        /*
         * Get the instance object pointer 
         */

        instancePtr = Tclpp_GetInstanceByName (interp, instanceName);
        if (instancePtr == (Tclpp_Instance*) NULL) {
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                    "instance \"", instanceName, "\" unknown", (char*) NULL);
            return TCL_ERROR;
        }

        /*
         * Delete it
         */

        if (Tclpp_DeleteInstance (interp, instancePtr) != TCL_OK) {
            return TCL_ERROR;
        }
    }
    else {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                "bad option \"", Tcl_GetStringFromObj(objv[1], (int *) NULL),
                "\": must be create or delete", (char *) NULL);
        return TCL_ERROR;
    }

    return TCL_OK;
}


/*
 * ----------------------------------------------------------------------------
 *
 * Tclpp_TestInit
 *
 *     Initialization routine for the Tclpp test suite. It will register
 *     all the commands defined in this test suite.
 *     all the commands defined in
 *
 * Results:
 *     If the test suite could be initialized properly, returns TCL_OK.
 *     Otherwise it will return TCL_ERROR and an error message is left
 *     at the interpreter's result string.
 *
 * Side Effects:
 *     None.
 *
 * ----------------------------------------------------------------------------
 */

int
Tclpp_TestInit(interp)
    Tcl_Interp *interp;       /* The interpreter to add the commands to */
{
    Tcl_CmdInfo cmdInfo;      /* Information about a certain command */

    /*
     * Register the generic 'test' command.
     */

    if (Tcl_GetCommandInfo(interp, "test", &cmdInfo)) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                "command \"test\" already exists", (char *)NULL);
        return TCL_ERROR;
    }
    Tcl_CreateObjCommand(interp, "test", &TestObjCmd, (ClientData) NULL,
            (Tcl_CmdDeleteProc *) NULL);

    /*
     * Register all test commands
     */

    Tcl_CreateObjCommand(interp, "testclass", &TestClassCmd, 
            (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateObjCommand(interp, "testinstance", &TestInstanceCmd, 
            (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

    return TCL_OK;
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

int
Tcl_AppInit(interp)
    Tcl_Interp *interp;         /* Interpreter for application. */
{
    if (Tcl_Init(interp) == TCL_ERROR) {
        return TCL_ERROR;
    }

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
    if (Tclpp_TestInit(interp) == TCL_ERROR) {
        return TCL_ERROR;
    }

    /*
     * Specify a user-specific startup file to invoke if the application
     * is run interactively.  Typically the startup file is "~/.apprc"
     * where "app" is the name of the application.  If this line is deleted
     * then no user-specific startup file will be run under any conditions.
     */

    Tcl_SetVar(interp, "tcl_rcFileName", "~/.tclshrc", TCL_GLOBAL_ONLY);
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
    Tcl_Main(argc, argv, Tcl_AppInit);
    return 0;                   /* Needed only to prevent compiler warning. */
}

/*
 * vi: set ai expandtab ts=4:
 */
