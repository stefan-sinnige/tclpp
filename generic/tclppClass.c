/*
 * tclppClass.c --
 *
 *     This file implements the 'class' command. 
 *
 * RCS: $Id: tclppClass.c,v 1.2 2000/05/23 19:34:13 stefan Exp $
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

#include <string.h>
#include "tclppInt.h"

static char rcsid[] = "$Id: tclppClass.c,v 1.2 2000/05/23 19:34:13 stefan Exp $";

/*
 * The following hash-table lists all the user defined classes.
 */

static classTableInitialized = 0;
Tcl_HashTable classTable;

/*
 * The following is the adornment structure used during class definition
 */

TclppAdornment tclppAdornment;

/*
 * Forward declarations
 */

static void TclppDeleteClass _ANSI_ARGS_ ((ClientData clientData));


/*
 * ----------------------------------------------------------------------------
 *
 * TclppClassCmd --
 *
 *     This procedure is invoked to process the 'class' Tclpp command.
 *
 * Results:
 *     Returns TCL_OK when a class could be constructed and TCL_ERROR
 *     otehrwise.
 *
 * Side effects:
 *     None.
 *
 * ----------------------------------------------------------------------------
 */

int
TclppClassCmd (clientData, interp, objc, objv)
    ClientData clientData;      /* Not used */
    Tcl_Interp *interp;         /* The interpreter to create the class in */
    int objc;                   /* Number of objects */
    Tcl_Obj * CONST objv[];     /* Array of objects */
{
    char *className;            /* Class name, without leading ':' */
    Tcl_Namespace *globalNS;    /* The global namespace */
    Tcl_Namespace *currentNS;   /* The current namespace */
    Tcl_DString name;           /* The fully qualified class namespace */
    int i;                      /* General counter */
    Tclpp_Class *classPtr;      /* The class created */

    /*
     * Check the number of objects.
     */
    
    if (objc < 3) {
        Tcl_WrongNumArgs(interp, 1, objv, 
                "name ?: baseclasslist ...? definition");
        return TCL_ERROR;
    }

    /*
     * Construct the fully qualified class name which will be used to 
     * create the namespace and the command with. Basically we prepend
     * the current namespace name to the class name. 
     */
    
    className = Tcl_GetStringFromObj(objv[1], (int *)NULL);
    while (*className == ':') {
        ++className;
    }

    Tcl_DStringInit(&name);
    currentNS = Tcl_GetCurrentNamespace(interp);
    globalNS = Tcl_GetGlobalNamespace(interp);
    if (currentNS == globalNS) {
        Tcl_DStringAppend(&name, currentNS->fullName, -1);
        Tcl_DStringAppend(&name, className, -1);
    } else {
        Tcl_DStringAppend(&name, currentNS->fullName, -1);
        Tcl_DStringAppend(&name, "::", 2);
        Tcl_DStringAppend(&name, className, -1);
    }

    /*
     * Create the class definition.
     */

    classPtr = Tclpp_CreateClass (interp, Tcl_DStringValue(&name), 
            objv[objc-1]);
    if (classPtr == (Tclpp_Class*) NULL) {
        Tcl_DStringFree(&name);
        return TCL_ERROR;
    }

    /*
     * Add the base classes
     */

    for (i = 2; i < (objc-1); i++) {
        char *baseClassName;        /* The base class as name */
        Tclpp_Class *baseClassPtr;  /* The base class as Tclpp_Class object */

        baseClassName = Tcl_GetStringFromObj(objv[i], (int*)NULL);

        /*
         * Check we start off with a ':', followed by an list of classes
         */

        if (i == 2) {
            if (strcmp(baseClassName, ":") != 0) {
                Tcl_DStringFree(&name);
                Tcl_ResetResult(interp);
                Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), 
                        "expected ':' to start base class list", (char*) NULL);
                return TCL_ERROR;
            }
            continue;
        }

        /*
         * Get the base class pointer and add it to the newly created class
         */

        baseClassPtr = Tclpp_GetClassByName(interp, baseClassName);
        if (baseClassPtr == (Tclpp_Class*) NULL) {
            Tcl_DStringFree(&name);
            Tcl_ResetResult(interp);
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "base class \"",
                    baseClassName, "\" unknown", (char*)NULL);
            return TCL_ERROR;
        }
        if (Tclpp_AddBaseClass (interp, classPtr, baseClassPtr) != TCL_OK) {
            Tcl_DStringFree(&name);
            return TCL_ERROR;
        }
    }

    Tcl_DStringFree(&name);
    return TCL_OK;
}


/*
 * ----------------------------------------------------------------------------
 *
 * Tclpp_CreateClass --
 *
 *     Used to construct and initialize a new class definition.
 *
 * Results:
 *     A namespace is created with the same name as the class name in the
 *     current namespace. Then the class-namespace is entered, the class
 *     defined functions are loaded and the code is evaluated. 
 *     If all went according to plan, it will return a pointer to the
 *     newly created class. Otherwise it will return NULL and sets the
 *     interpreter result string.
 *
 * Side effects:
 *     None.
 *
 * ----------------------------------------------------------------------------
 */

Tclpp_Class*
Tclpp_CreateClass (interp, fullName, classDefinition)
    Tcl_Interp *interp;         /* Current Interpreter */
    char *fullName;             /* Fully qualified name of the class, must
                                 * start with a :: */
    Tcl_Obj *classDefinition;   /* The class definition code */
{
    char *name;                 /* Unqualified class name, containing no
                                 * ::'s at all */
    Tcl_Namespace *classNSPtr;  /* The namespace representing the class */
    Tcl_HashEntry *entryPtr;    /* Hash table entry for new class */
    int isNew;                  /* Set if the class exists in hashtable */ 
    TclppClass *classPtr;       /* The new class structure */
    Tcl_CmdInfo cmdInfo;        /* Information about a certain command */
    Tcl_CallFrame frame;        /* The call frame for 'eval' of definition */

    TclppResetAdornment();

    /*
     * Get the simple class name, which is from the last occurence of ':' of
     * the full name.
     */

    name = strrchr (fullName, ':') + 1;

    /*
     * Check if class already exists in the classTable
     */
     
    if (!classTableInitialized) {
        Tcl_InitHashTable(&classTable, TCL_STRING_KEYS);
        classTableInitialized = 1;
    }
    entryPtr = Tcl_FindHashEntry(&classTable, fullName);
    if (entryPtr != (Tcl_HashEntry*) NULL) {
        Tcl_ResetResult(interp);
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "class \"",
                fullName, "\" already exists", (char *) NULL);
        return (Tclpp_Class*) NULL;
    }

    /*
     * Check if the class exists as a command. Note that we must use the
     * fully qualified name (including namespace) for this.
     */
    
    if (Tcl_GetCommandInfo(interp, fullName, &cmdInfo)) {
        Tcl_ResetResult(interp);
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "command \"",
                fullName, "\" already exists", (char *) NULL);
        return (Tclpp_Class*) NULL;
    }

    /*
     * Create the namespace
     */

    classPtr = (TclppClass*) Tcl_Alloc(sizeof(TclppClass));
    classNSPtr = Tcl_CreateNamespace (interp, fullName, (ClientData) classPtr, 
            TclppDeleteClass);
    if (classNSPtr == NULL) {
        Tcl_Free((char*)classPtr);
        return (Tclpp_Class*) NULL;
    }

    /*
     * Create an entry in the classTable.
     */
    
    entryPtr = Tcl_CreateHashEntry (&classTable, fullName, &isNew);
    if (!isNew) {
        Tcl_ResetResult(interp);
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "class \"",
                fullName, "\" already exists", (char *) NULL);
        Tcl_DeleteNamespace(classNSPtr);
        Tcl_Free((char*)classPtr);
        return (Tclpp_Class*) NULL;
    }

    classPtr->name = (char *) Tcl_Alloc(strlen(name)+1);
    strcpy(classPtr->name, name);
    classPtr->fullName = (char *) Tcl_Alloc(strlen(fullName)+1);
    strcpy(classPtr->fullName, fullName);
    classPtr->nsPtr = (Namespace*)classNSPtr;
    Tcl_InitHashTable(&(classPtr->baseTable), TCL_STRING_KEYS);
    Tcl_InitHashTable(&(classPtr->methodTable), TCL_STRING_KEYS);
    Tcl_InitHashTable(&(classPtr->variableTable), TCL_STRING_KEYS);
    Tcl_SetHashValue(entryPtr, (ClientData)classPtr);

    /* 
     * Create the command to create instances of this class using the
     * complete namespace.
     */ 

    Tcl_CreateObjCommand(interp, fullName, &TclppInstanceCmd,
            (ClientData) classPtr, (Tcl_CmdDeleteProc *) NULL);

    /*
     * Enter the newly created class-namespace and load up all the
     * class internal functions by importing them from the ::Tclpp
     * namespace.
     */

    if (Tcl_PushCallFrame(interp, &frame, classNSPtr, 0) != TCL_OK) {
        return (Tclpp_Class*) NULL;
    }
    if (Tcl_Import(interp, classNSPtr, "::tclpp::*" , 1) != TCL_OK) {
        Tcl_PopCallFrame(interp);
        return (Tclpp_Class*) NULL;
    }

    /*
     * Add the 'this' variable to the class variable list. This prevents 
     * the user to add a variable 'this' as well.
     */

    if (Tclpp_CreateVariable (interp, (Tclpp_Class*) classPtr, "scalar", 
            "this") == (Tclpp_Variable*) NULL) {
        Tcl_PopCallFrame(interp);
        return (Tclpp_Class*) NULL;
    }

    /*
     * Setup the variable name resolver to our own method such that we can
     * link our class variables to the corresponding instance variables.
     */

    Tcl_SetNamespaceResolvers(classNSPtr, TclppResolveClassCmd,
            TclppResolveClassVar, TclppResolveClassCompiledVar);

    /*
     * Execute the class definition code.
     */

    if (Tcl_EvalObj(interp, classDefinition) == TCL_ERROR) {
        Tcl_PopCallFrame(interp);
        return (Tclpp_Class*) NULL;
    }

    /*
     * Leave the class namespace.
     */

    Tcl_PopCallFrame(interp);

    return (Tclpp_Class*) classPtr;
}


/*
 * ----------------------------------------------------------------------------
 *
 * Tclpp_GetCurrentClass --
 *
 *     Returns the current class definition pointer, based on the current
 *     namespace. 
 *
 * Results:
 *     Returns the Tclpp_Class of the current class context based on the
 *     current namespace. If such an entry does not exist (eg. the thread
 *     of execution is outside any class definition), it will return NULL.
 *     Note that this will be an O(n) search through all classes.
 *
 * Side effects:
 *     None.
 *
 * ----------------------------------------------------------------------------
 */

Tclpp_Class*
Tclpp_GetCurrentClass (interp)
    Tcl_Interp *interp;           /* The interpreter */
{
    Tcl_Namespace *currentNsPtr;  /* The current namespace */
    Tcl_HashEntry *entryPtr;      /* The hash entry */
    Tcl_HashSearch search;        /* Keep track of hash table search */

    /*
     * Get the current namespace.
     */

    currentNsPtr = Tcl_GetCurrentNamespace (interp);

    /*
     * Search through the entire classTable until an entry could be
     * located with matching namespace.
     */

    for (entryPtr = Tcl_FirstHashEntry(&classTable, &search); entryPtr != NULL;
            entryPtr = Tcl_NextHashEntry(&search)) {
        TclppClass* classPtr = (TclppClass*)Tcl_GetHashValue(entryPtr);
        if (((Tcl_Namespace *)classPtr->nsPtr) == currentNsPtr) {
            return (Tclpp_Class*) classPtr;
        }
    }
    return (Tclpp_Class*) NULL;
}


/*
 * ----------------------------------------------------------------------------
 *
 * Tclpp_GetClassByName --
 *
 *     Returns the class pointer of a class specified by its name.
 *
 * Results:
 *     Returns the Tclpp_Class of the class if one could be located and
 *     NULL otherwise. Note that this will be an O(n) search through all 
 *     classes.
 *
 * Side effects:
 *     None.
 *
 * ----------------------------------------------------------------------------
 */

Tclpp_Class*
Tclpp_GetClassByName (interp, className)
    Tcl_Interp *interp;           /* The interpreter */
    char *className;              /* The class name */
{
    Tcl_DString fullName;         /* The fully qualified class name */
    Tcl_HashEntry *entryPtr;      /* The hash table entry */
    Tcl_HashSearch search;        /* Keep track of hash table search */
    TclppClass *classPtr;         /* The associated class */

    /*
     * Construct a fully qualified class name.
     */

    Tcl_DStringInit(&fullName);
    while (*className == ':') {
        ++className;
    }
    Tcl_DStringAppend(&fullName, "::", 2);
    Tcl_DStringAppend(&fullName, className, -1);

    /*
     * Search through all classes to see if we have a matching class name
     */

    classPtr = (TclppClass*) NULL;
    for (entryPtr = Tcl_FirstHashEntry(&classTable, &search); entryPtr != NULL;
            entryPtr = Tcl_NextHashEntry(&search)) {
        TclppClass* entryClassPtr = (TclppClass*)Tcl_GetHashValue(entryPtr);
        if (strcmp(entryClassPtr->fullName, Tcl_DStringValue(&fullName)) == 0) {
            classPtr = entryClassPtr;
        }
    }

    return (Tclpp_Class*) classPtr;
}


/*
 * ----------------------------------------------------------------------------
 *
 * TclppDeleteClass --
 *
 *     Callback function called when the namespace is deleted in which the
 *     class was created. This is an unwanted situation. Do nothing and
 *     hope for the best ....
 *
 * Results:
 *     None.
 *
 * Side effects:
 *     This is clearly an unwanted situation. 
 *
 * ----------------------------------------------------------------------------
 */

static void
TclppDeleteClass (clientData)
    ClientData clientData;      /* The Tclpp_Class pointer */
{
}


/*
 * ----------------------------------------------------------------------------
 *
 * Tclpp_AddBaseClass --
 *     Add a base class to a class definition. The base class must exist at
 *     this point. 
 *
 * Results:
 *     Returns TCL_OK on success and TCL_ERROR otherwise. If it returns
 *     TCL_ERROR, the result string is appended in the interpreter result.
 *
 * Side effects:
 *     When an error occurs, the derived class becomes in an unknown state.
 *
 * ----------------------------------------------------------------------------
 */

static int
TclppCheckBaseClassAddition (interp, dervClassPtr, baseClassPtr)
    Tcl_Interp *interp;         /* Current interpreter */
    Tclpp_Class *dervClassPtr;  /* The derived class */
    Tclpp_Class *baseClassPtr;  /* Base class to check. */
{
    Tcl_HashEntry *entryPtr;    /* The current hash entry */
    Tcl_HashSearch search;      /* Keep track of hash table search */

    /* 
     * For each method specified by the base class, check if it overrides
     * another function which might result in ambiguity or strange behaviour.
     */

    for (entryPtr = Tcl_FirstHashEntry(&(baseClassPtr->methodTable), &search);
            entryPtr != (Tcl_HashEntry*) NULL; 
            entryPtr = Tcl_NextHashEntry(&search)) {
        TclppMethod *methodPtr1;
        TclppMethod *methodPtr2;
        methodPtr1 = (TclppMethod*) Tcl_GetHashValue(entryPtr);
        if (TclppLookupMethod(interp, (TclppInstance*) NULL, methodPtr1->name, 
                (TclppClass*) dervClassPtr, &methodPtr2) == TCL_OK) {
            /*
             * If method1 is virtual and class of method2 is the derived class,
             * all is Ok. The derived class merily overrides a virtual method.
             */

            if (TclppMethodIsVirtual(methodPtr1) 
                    && methodPtr2->classPtr == (TclppClass*) dervClassPtr) {
                continue;
            }

            /*
             * If method1 is not virtual and method2 is the derived class, we
             * cannot override.
             */

            if (!TclppMethodIsVirtual(methodPtr1)
                    && methodPtr2->classPtr == (TclppClass*) dervClassPtr) {
                Tcl_ResetResult(interp);
                Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "method \"",
                        methodPtr2->classPtr->fullName, "::", methodPtr2->name,
                        "\" cannot override non-virtual \"", 
                        methodPtr1->classPtr->fullName, "::", methodPtr1->name,
                        "\"", (char*) NULL);
                return TCL_ERROR;
            }

            /*
             * Everything else is an ambiguity
             */

            Tcl_ResetResult(interp);
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "method \"",
                    methodPtr2->classPtr->fullName, "::", methodPtr2->name,
                    "\" hides \"", 
                    methodPtr1->classPtr->fullName, "::", methodPtr1->name,
                    "\" (Ambiguity)", (char*) NULL);
            return TCL_ERROR;
        }
    }
    Tcl_ResetResult(interp);
    return TCL_OK;
}

int
Tclpp_AddBaseClass (interp, dervClassPtr, baseClassPtr)
    Tcl_Interp *interp;         /* Current interpreter */
    Tclpp_Class *dervClassPtr;  /* The derived class */
    Tclpp_Class *baseClassPtr;  /* Base class to add to the other class. */
{
    Tcl_HashEntry *entryPtr;    /* The current hash entry */
    Tcl_HashSearch search;      /* Keep track of hash table search */
    int isNew;                  /* Set if the hash entry did not exist */

    /*
     * Check if the base class can be added, that is, no ambiguities.
     */

    if (TclppCheckBaseClassAddition(interp, dervClassPtr, baseClassPtr)
            == TCL_ERROR) {
        return TCL_ERROR;
    }

    /* 
     * Check if all the base classes of the base class do not cause any
     * ambiguities as well.
     */

    for (entryPtr = Tcl_FirstHashEntry(&(baseClassPtr->baseTable), &search);
            entryPtr != (Tcl_HashEntry*) NULL; 
            entryPtr = Tcl_NextHashEntry(&search)) {
        TclppClass *baseBaseClassPtr;
        baseBaseClassPtr = (TclppClass*) Tcl_GetHashValue(entryPtr);
        if (TclppCheckBaseClassAddition(interp, dervClassPtr, baseBaseClassPtr)
                == TCL_ERROR) {
            return TCL_ERROR;
        }
    }

    /*
     * Add the base class to the base class list. If the base class already
     * exists, we consider it as a virtual base class and will not be
     * added to the hash-entry. This enables the 'diamond shaped' multiple
     * inheritance.
     */

    entryPtr = Tcl_CreateHashEntry(&(dervClassPtr->baseTable),
            baseClassPtr->name, &isNew);
    if (!isNew) {
        return TCL_OK;
    }
    Tcl_SetHashValue(entryPtr, (ClientData) baseClassPtr);

    return TCL_OK;
}

/*
 * vi: set ai expandtab ts=4: 
 */

