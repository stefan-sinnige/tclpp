/*
 * tclppInfo.c --
 *
 *     This file implements the Tclpp information procedures.
 *
 * RCS: $Id: tclppInfo.c,v 1.2 2000/05/23 19:34:13 stefan Exp $
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

#include "tclppInt.h"

static char rcsid[] = "$Id";

/*
 * Forward declarations
 */

static int InfoClassesCmd _ANSI_ARGS_ ((ClientData clientData, 
            Tcl_Interp* interp, int objc, Tcl_Obj * CONST objv[]));
static int InfoHeritageCmd _ANSI_ARGS_ ((ClientData clientData, 
            Tcl_Interp* interp, int objc, Tcl_Obj * CONST objv[]));
static int InfoMethodsCmd _ANSI_ARGS_ ((ClientData clientData, 
            Tcl_Interp* interp, int objc, Tcl_Obj * CONST objv[]));
static int InfoVariablesCmd _ANSI_ARGS_ ((ClientData clientData, 
            Tcl_Interp* interp, int objc, Tcl_Obj * CONST objv[]));
static int InfoIsACmd _ANSI_ARGS_ ((ClientData clientData, 
            Tcl_Interp* interp, int objc, Tcl_Obj * CONST objv[]));
static int InfoIsKindOfCmd _ANSI_ARGS_ ((ClientData clientData, 
            Tcl_Interp* interp, int objc, Tcl_Obj * CONST objv[]));


/*
 * ----------------------------------------------------------------------------
 *
 * TclppClassInfoCmd --
 *
 *     This procedure is invoked to process the 'classinfo' Tclpp command.
 *
 * Results:
 *     Returns TCL_OK on success and TCL_ERROR otherwise. The result is
 *     depending on the sub-command.
 *
 * Side effects:
 *     None.
 *
 * ----------------------------------------------------------------------------
 */

int
TclppClassInfoCmd (clientData, interp, objc, objv)
    ClientData clientData;      /* Dummy */
    Tcl_Interp *interp;         /* The interpreter */
    int objc;                   /* Number of objects */
    Tcl_Obj * CONST objv[];     /* Array of objects */
{
    static char* subCmds[] = {  /* List of sub command strings */
            "classes", "heritage", "methods", "variables",
            (char*) NULL
    };

    enum subCmdIdx {            /* List of associated sub command indices */
            classesIdx, heritageIdx, methodsIdx, variablesIdx
    } index;
   
    /*
     * Check the number of objects
     */

    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "option ?arg arg ...?");
        return TCL_ERROR;
    }

    /*
     * Select the detailed information procedure
     */

    if (Tcl_GetIndexFromObj(interp, objv[1], subCmds, "option", 0, 
            (int*) &index) != TCL_OK) {
        return TCL_ERROR;
    }

    switch (index) {
        case classesIdx:
            return InfoClassesCmd (clientData, interp, objc, objv);
        case heritageIdx:
            return InfoHeritageCmd (clientData, interp, objc, objv);
        case methodsIdx:
            return InfoMethodsCmd (clientData, interp, objc, objv);
        case variablesIdx:
            return InfoVariablesCmd (clientData, interp, objc, objv);
    };
    
    return TCL_OK;
}


/*
 * ----------------------------------------------------------------------------
 *
 * static InfoClassesCmd --
 *
 *     Called to implement the classinfo command syntax:
 *
 *          classinfo classes ?pattern?
 *
 *     to return a list of all fully qualified class names. A pattern
 *     may be specified to restrict the output.
 *
 * Results:
 *     Returns TCL_OK on success and TCL_ERROR otherwise.
 *
 * Side effects:
 *     None.
 *
 * ----------------------------------------------------------------------------
 */

static int
InfoClassesCmd (clientData, interp, objc, objv)
    ClientData clientData;      /* Dummy */
    Tcl_Interp *interp;         /* The interpreter */
    int objc;                   /* Number of objects */
    Tcl_Obj * CONST objv[];     /* Array of objects */
{
    char* pattern;              /* The pattern to match against */
    Tcl_Obj* listPtr;           /* List of class names */
    Tcl_Obj* elemObjPtr;        /* List element object pointer */
    Tcl_HashEntry *entryPtr;    /* The hash entry */
    Tcl_HashSearch search;      /* Keep track of hash table search */

    /*
     * Check the number of objects
     */

    if (objc == 2) {
        pattern = (char *) NULL;
    } else if (objc == 3) {
        pattern = Tcl_GetStringFromObj(objv[2], (int *) NULL);
    } else {
        Tcl_WrongNumArgs(interp, 2, objv, "?pattern?");
        return TCL_ERROR;
    }

    /*
     * Step through the entire class table and create a list of all
     * the class names.
     */

    listPtr = Tcl_NewListObj(0, (Tcl_Obj **) NULL);
    for (entryPtr = Tcl_FirstHashEntry(&classTable, &search); entryPtr != NULL;
            entryPtr = Tcl_NextHashEntry(&search)) {
        TclppClass* classPtr = (TclppClass*)Tcl_GetHashValue(entryPtr);
        if ((pattern == (char*) NULL) 
                || Tcl_StringMatch(classPtr->fullName, pattern)) {
            elemObjPtr = Tcl_NewStringObj (classPtr->fullName, -1);
            Tcl_ListObjAppendElement(interp, listPtr, elemObjPtr);
        }
    }

    Tcl_SetObjResult(interp, listPtr);
    return TCL_OK;
}


/*
 * ----------------------------------------------------------------------------
 *
 * static InfoHeritageCmd --
 *
 *     Called to implement the classinfo command syntax:
 *
 *          classinfo heritage <class>
 *
 *     to return a list of all the base classes of a certain class.
 *     Note that only the direct base classes are returned, not the
 *     entire tree.
 *
 * Results:
 *     Returns TCL_OK on success and TCL_ERROR otherwise.
 *
 * Side effects:
 *     None.
 *
 * ----------------------------------------------------------------------------
 */

static int
InfoHeritageCmd (clientData, interp, objc, objv)
    ClientData clientData;      /* Dummy */
    Tcl_Interp *interp;         /* The interpreter */
    int objc;                   /* Number of objects */
    Tcl_Obj * CONST objv[];     /* Array of objects */
{
    TclppClass *classPtr;       /* The class pointer */
    Tcl_Obj* listPtr;           /* List of base classes */
    Tcl_HashEntry *entryPtr;    /* The hash entry */
    Tcl_HashSearch search;      /* Keep track of hash table search */

    /*
     * Check the number of objects
     */

    if (objc != 3) {
        Tcl_WrongNumArgs(interp, 2, objv, "class");
        return TCL_ERROR;
    }

    /* 
     * Get the class associated with the class name.
     */

    classPtr = (TclppClass*) Tclpp_GetClassByName (interp, 
            Tcl_GetStringFromObj(objv[2], (int*) NULL));
    if (classPtr == NULL) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "class \"",
                Tcl_GetStringFromObj(objv[2], (int*) NULL), "\" unknown",
                (char*) NULL);
        return TCL_ERROR;
    }

    /*
     * Step through the entire class bases table and create a list of all
     * the base classes.
     */

    listPtr = Tcl_NewListObj(0, (Tcl_Obj **) NULL);
    for (entryPtr = Tcl_FirstHashEntry(&(classPtr->baseTable), &search); 
            entryPtr != NULL; entryPtr = Tcl_NextHashEntry(&search)) {
        TclppClass* baseClassPtr;   /* The base class */
        Tcl_Obj* elemObjPtr;    /* List element object pointer */

        baseClassPtr = (TclppClass*)Tcl_GetHashValue(entryPtr);
        elemObjPtr = Tcl_NewStringObj (baseClassPtr->name, -1);
        Tcl_ListObjAppendElement(interp, listPtr, elemObjPtr);
    }

    Tcl_SetObjResult(interp, listPtr);
    return TCL_OK;
}


/*
 * ----------------------------------------------------------------------------
 *
 * static InfoMethodsCmd --
 *
 *     Called to implement the classinfo command syntax:
 *
 *          classinfo methods <class>
 *
 *     to return a list of all methods of a certain class.
 *
 * Results:
 *     Returns TCL_OK on success and TCL_ERROR otherwise.
 *
 * Side effects:
 *     None.
 *
 * ----------------------------------------------------------------------------
 */

static int
InfoMethodsCmd (clientData, interp, objc, objv)
    ClientData clientData;      /* Dummy */
    Tcl_Interp *interp;         /* The interpreter */
    int objc;                   /* Number of objects */
    Tcl_Obj * CONST objv[];     /* Array of objects */
{
    TclppClass *classPtr;       /* The class pointer */
    Tcl_Obj* listPtr;           /* List of class methods */
    Tcl_HashEntry *entryPtr;    /* The hash entry */
    Tcl_HashSearch search;      /* Keep track of hash table search */

    /*
     * Check the number of objects
     */

    if (objc != 3) {
        Tcl_WrongNumArgs(interp, 2, objv, "class");
        return TCL_ERROR;
    }

    /* 
     * Get the class associated with the class name.
     */

    classPtr = (TclppClass*) Tclpp_GetClassByName (interp, 
            Tcl_GetStringFromObj(objv[2], (int*) NULL));
    if (classPtr == NULL) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "class \"",
                Tcl_GetStringFromObj(objv[2], (int*) NULL), "\" unknown",
                (char*) NULL);
        return TCL_ERROR;
    }

    /*
     * Step through the entire class method table and create a list of all
     * the methods.
     */

    listPtr = Tcl_NewListObj(0, (Tcl_Obj **) NULL);
    for (entryPtr = Tcl_FirstHashEntry(&(classPtr->methodTable), &search); 
            entryPtr != NULL; entryPtr = Tcl_NextHashEntry(&search)) {
        Tcl_Obj* elemObjPtr;    /* List element object pointer */
        TclppMethod* methodPtr; /* The class method */
        
        methodPtr = (TclppMethod*)Tcl_GetHashValue(entryPtr);
        elemObjPtr = Tcl_NewStringObj (methodPtr->name, -1);
        Tcl_ListObjAppendElement(interp, listPtr, elemObjPtr);
    }

    Tcl_SetObjResult(interp, listPtr);
    return TCL_OK;
}


/*
 * ----------------------------------------------------------------------------
 *
 * static InfoVariablesCmd --
 *
 *     Called to implement the classinfo command syntax:
 *
 *          classinfo variables <class>
 *
 *     to return a list of all variables and their types of a certain class.
 *
 * Results:
 *     Returns TCL_OK on success and TCL_ERROR otherwise.
 *
 * Side effects:
 *     None.
 *
 * ----------------------------------------------------------------------------
 */

static int
InfoVariablesCmd (clientData, interp, objc, objv)
    ClientData clientData;      /* Dummy */
    Tcl_Interp *interp;         /* The interpreter */
    int objc;                   /* Number of objects */
    Tcl_Obj * CONST objv[];     /* Array of objects */
{
    TclppClass *classPtr;       /* The class pointer */
    Tcl_Obj* listPtr;           /* List of class variables */
    Tcl_HashEntry *entryPtr;    /* The hash entry */
    Tcl_HashSearch search;      /* Keep track of hash table search */

    /*
     * Check the number of objects
     */

    if (objc != 3) {
        Tcl_WrongNumArgs(interp, 2, objv, "class");
        return TCL_ERROR;
    }

    /* 
     * Get the class associated with the class name.
     */

    classPtr = (TclppClass*) Tclpp_GetClassByName (interp, 
            Tcl_GetStringFromObj(objv[2], (int*) NULL));
    if (classPtr == NULL) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "class \"",
                Tcl_GetStringFromObj(objv[2], (int*) NULL), "\" unknown",
                (char*) NULL);
        return TCL_ERROR;
    }

    /*
     * Step through the entire class variable table and create a list of all
     * the variables.
     */

    listPtr = Tcl_NewListObj(0, (Tcl_Obj **) NULL);
    for (entryPtr = Tcl_FirstHashEntry(&(classPtr->variableTable), &search); 
            entryPtr != NULL; entryPtr = Tcl_NextHashEntry(&search)) {
        Tcl_Obj* elemObjPtr;    /* List element object pointer */
        TclppVariable* varPtr;  /* The variable structure */

        varPtr = (TclppVariable*)Tcl_GetHashValue(entryPtr);
        elemObjPtr = Tcl_NewListObj(0, (Tcl_Obj **) NULL);
        Tcl_ListObjAppendElement(interp, elemObjPtr, 
                Tcl_NewStringObj (varPtr->name, -1));
        Tcl_ListObjAppendElement(interp, elemObjPtr, 
                Tcl_NewStringObj (varPtr->typeName, -1));
        Tcl_ListObjAppendElement(interp, listPtr, elemObjPtr);
    }

    Tcl_SetObjResult(interp, listPtr);
    return TCL_OK;
}


/*
 * ----------------------------------------------------------------------------
 *
 * TclppObjectInfoCmd --
 *
 *     This procedure is invoked to process the 'objectinfo' Tclpp command.
 *
 * Results:
 *     Returns TCL_OK on success and TCL_ERROR otherwise. The result is
 *     depending on the sub-command.
 *
 * Side effects:
 *     None.
 *
 * ----------------------------------------------------------------------------
 */

int
TclppObjectInfoCmd (clientData, interp, objc, objv)
    ClientData clientData;      /* Dummy */
    Tcl_Interp *interp;         /* The interpreter */
    int objc;                   /* Number of objects */
    Tcl_Obj * CONST objv[];     /* Array of objects */
{
    static char* subCmds[] = {  /* List of sub command strings */
            "isA", "isKindOf",
            (char*) NULL
    };

    enum subCmdIdx {            /* List of associated sub command indices */
            isAIdx, isKindOfIdx
    } index;
   
    /*
     * Check the number of objects
     */

    if (objc < 3) {
        Tcl_WrongNumArgs(interp, 1, objv, "object option ?arg arg ...?");
        return TCL_ERROR;
    }

    /*
     * Select the detailed information procedure
     */

    if (Tcl_GetIndexFromObj(interp, objv[2], subCmds, "option", 0, 
            (int*) &index) != TCL_OK) {
        return TCL_ERROR;
    }

    switch (index) {
        case isAIdx:
            return InfoIsACmd (clientData, interp, objc, objv);
        case isKindOfIdx:
            return InfoIsKindOfCmd (clientData, interp, objc, objv);
    };
    
    return TCL_OK;
}


/*
 * ----------------------------------------------------------------------------
 *
 * static InfoIsACmd --
 *
 *     Called to implement the objectinfo command syntax:
 *
 *          objectinfo <object> isA
 *
 *     to return the fully qualified class name of the object.
 *
 * Results:
 *     Returns TCL_OK on success and TCL_ERROR otherwise.
 *
 * Side effects:
 *     None.
 *
 * ----------------------------------------------------------------------------
 */

static int
InfoIsACmd (clientData, interp, objc, objv)
    ClientData clientData;      /* Dummy */
    Tcl_Interp *interp;         /* The interpreter */
    int objc;                   /* Number of objects */
    Tcl_Obj * CONST objv[];     /* Array of objects */
{
    TclppInstance* instancePtr; /* The instance */

    /*
     * Check the number of objects
     */

    if (objc != 3) {
        Tcl_WrongNumArgs(interp, 1, objv, "object isA");
        return TCL_ERROR;
    }

    /*
     * Get the object by its name and return the class full name.
     */

    instancePtr = (TclppInstance*) Tclpp_GetInstanceByName (interp,
            Tcl_GetStringFromObj(objv[1], (int *) NULL));
    if (instancePtr == (TclppInstance*)NULL) {
        Tcl_ResetResult(interp);
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "instance \"",
                Tcl_GetStringFromObj(objv[1], (int *) NULL), "\" unknown",
                (char*) NULL);
        return TCL_ERROR;
    }

    Tcl_SetObjResult(interp, 
            Tcl_NewStringObj(instancePtr->classPtr->fullName,-1));
    return TCL_OK;
}


/*
 * ----------------------------------------------------------------------------
 *
 * static InfoIsKindOfCmd --
 *
 *     Called to implement the objectinfo command syntax:
 *
 *          objectinfo <object> isKindOf <class>
 *
 *     which returns 1 if the <object> is (or is derived from) <class>.
 *
 * Results:
 *     Returns TCL_OK on success and TCL_ERROR otherwise.
 *
 * Side effects:
 *     None.
 *
 * ----------------------------------------------------------------------------
 */

static int
ObjectIsKindOfClass (instancePtr, classPtr, searchClassPtr)
Tclpp_Instance* instancePtr;    /* The instance */
Tclpp_Class* classPtr;          /* The class (or its base classes) to check */
Tclpp_Class* searchClassPtr;    /* The class to search for */
{
    Tcl_HashEntry *entryPtr;    /* The hash entry */
    Tcl_HashSearch search;      /* Keep track of hash table search */
    
    if (classPtr == searchClassPtr) {
        return 1;
    } else {
        for (entryPtr = Tcl_FirstHashEntry(&(classPtr->baseTable), &search); 
                entryPtr != NULL; 
                entryPtr = Tcl_NextHashEntry(&search)) {
            Tclpp_Class *baseClassPtr;
            baseClassPtr = (Tclpp_Class*) Tcl_GetHashValue(entryPtr);
            if (ObjectIsKindOfClass(instancePtr,baseClassPtr,searchClassPtr)) {
                return 1;
            }
        }
    }
    return 0;
}

static int
InfoIsKindOfCmd (clientData, interp, objc, objv)
    ClientData clientData;      /* Dummy */
    Tcl_Interp *interp;         /* The interpreter */
    int objc;                   /* Number of objects */
    Tcl_Obj * CONST objv[];     /* Array of objects */
{
    Tclpp_Instance* instancePtr;/* The instance pointer */
    Tclpp_Class* classPtr;      /* The class pointer */
    Tcl_Obj* result;            /* The result */

    /*
     * Check the number of objects
     */

    if (objc != 4) {
        Tcl_WrongNumArgs(interp, 1, objv, "object isKindOf class");
        return TCL_ERROR;
    }

    /*
     * Get the object by its name
     */

    instancePtr = Tclpp_GetInstanceByName (interp,
            Tcl_GetStringFromObj(objv[1], (int *) NULL));
    if (instancePtr == (Tclpp_Instance*)NULL) {
        Tcl_ResetResult(interp);
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "instance \"",
                Tcl_GetStringFromObj(objv[1], (int *) NULL), "\" unknown",
                (char*) NULL);
        return TCL_ERROR;
    }

    /*
     * Get the class by its name
     */

    classPtr = Tclpp_GetClassByName (interp,
            Tcl_GetStringFromObj(objv[3], (int *) NULL));
    if (classPtr == (Tclpp_Class*)NULL) {
        Tcl_ResetResult(interp);
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "class \"",
                Tcl_GetStringFromObj(objv[3], (int *) NULL), "\" unknown",
                (char*) NULL);
        return TCL_ERROR;
    }

    /*
     * Check if the class is the objects class or one of its base classes.
     */

    if (ObjectIsKindOfClass(instancePtr, instancePtr->classPtr, classPtr)) {
        result = Tcl_NewBooleanObj(1);
    } else {
        result = Tcl_NewBooleanObj(0);
    }
        
    Tcl_SetObjResult(interp, result);

    return TCL_OK;
}

/*
 * vi: set ai expandtab ts=4: 
 */

