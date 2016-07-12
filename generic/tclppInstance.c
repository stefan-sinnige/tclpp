/*
 * tclppInstance.c --
 *
 *     This file implements the class instantiation procedure.
 *
 * RCS: $Id: tclppInstance.c,v 1.2 2000/05/23 19:34:13 stefan Exp $
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

static char rcsid[] = "$Id: tclppInstance.c,v 1.2 2000/05/23 19:34:13 stefan Exp $";

static instanceTableInitialized = 0;
Tcl_HashTable instanceTable;

/*
 * Forward declarations
 */

static void TclppDeleteInstance _ANSI_ARGS_ ((ClientData clientData));


/*
 * ----------------------------------------------------------------------------
 *
 * TclppInstanceCmd --
 *
 *     Used to process the generic class instance creation procedure.
 *
 * Results:
 *     Check the command line arguments and create an instance of the
 *     specified class. Returns TCL_OK on success and TCL_ERROR otherwise.
 *
 * Side effects:
 *     None.
 *
 * ----------------------------------------------------------------------------
 */

int
TclppInstanceCmd (clientData, interp, objc, objv)
    ClientData clientData;       /* The Tclpp_Class class */
    Tcl_Interp *interp;          /* The interpreter to create the instance in */
    int objc;                    /* Number of objects */
    Tcl_Obj * CONST objv[];      /* Array of objects:
                                  *     objv[0] = class
                                  *     objv[1] = instance
                                  *     objv[n] = constructor args */
{
    char *instanceName;          /* Instance name, without leading ':' */
    Tcl_Namespace *globalNS;     /* The global namespace */
    Tcl_Namespace *currentNS;    /* The current namespace */
    Tclpp_Class *classPtr;       /* The class definition (client-data) */
    Tclpp_Instance *instancePtr; /* The instance created */
    Tcl_DString name;            /* The fully qualified instance name */

    classPtr = (Tclpp_Class*) clientData;

    /*
     * Check the number of objects 
     */

    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "name ?constructor-args?");
        return TCL_ERROR;
    }

    /*
     * Construct the fully qualified instance name which will be used to
     * create the namespace and the command with. Basically we prepend 
     * the current namespace name to the instance name.
     */

    instanceName = Tcl_GetStringFromObj(objv[1], (int *) NULL);
    while (*instanceName == ':') {
        ++instanceName;
    }

    Tcl_DStringInit (&name);
    currentNS = Tcl_GetCurrentNamespace(interp);
    globalNS = Tcl_GetGlobalNamespace(interp);
    if (currentNS == globalNS) {
        Tcl_DStringAppend(&name, currentNS->fullName, -1);
        Tcl_DStringAppend(&name, instanceName, -1);
    } else {
        Tcl_DStringAppend(&name, currentNS->fullName, -1);
        Tcl_DStringAppend(&name, "::", 2);
        Tcl_DStringAppend(&name, instanceName, -1);
    }

    /*
     * Create the instance
     */

    instancePtr = Tclpp_CreateInstance(interp, Tcl_DStringValue(&name), 
            classPtr, objc-2, &(objv[2]));
    if (instancePtr == (Tclpp_Instance*) NULL) {
        Tcl_DStringFree(&name);
        return TCL_ERROR;
    }
    Tcl_DStringFree(&name);

    /*
     * Return the instance name
     */

    Tcl_ResetResult (interp);
    Tcl_SetStringObj (Tcl_GetObjResult(interp), instanceName, -1);

    return TCL_OK;
}


/*
 * ----------------------------------------------------------------------------
 *
 * TclppDeleteCmd --
 *
 *     Used to process the generic class instance creation procedure and
 *     produce a really unique instance name automatically.
 *
 * Results:
 *     Check the command line arguments and create an instance of the
 *     specified class. Returns TCL_OK on success and TCL_ERROR otherwise.
 *
 * Side effects:
 *     None.
 *
 * ----------------------------------------------------------------------------
 */

int
TclppDeleteCmd (clientData, interp, objc, objv)
    ClientData clientData;       /* Not used */
    Tcl_Interp *interp;          /* The interpreter to create the instance in */
    int objc;                    /* Number of objects */
    Tcl_Obj * CONST objv[];      /* Array of objects:
                                  *     objv[0] = delete
                                  *     objv[1] = object */
{
    Tclpp_Instance *instancePtr; /* The instance to delete */

    /*
     * Check the number of objects 
     */

    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "object");
        return TCL_ERROR;
    }

    /*
     * Get the instance pointer
     */

    instancePtr = Tclpp_GetInstanceByName(interp, Tcl_GetStringFromObj(objv[1],
            (int*) NULL));
    if (instancePtr == (Tclpp_Instance*) NULL) {
        Tcl_ResetResult(interp);
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "instance \"",
                Tcl_GetStringFromObj(objv[1],(int*)NULL), "\" unknown", 
                (char*) NULL);
        return TCL_ERROR;
    }

    /*
     * Delete it
     */

    return Tclpp_DeleteInstance(interp, instancePtr);
}


/*
 * ----------------------------------------------------------------------------
 *
 * TclppNewCmd --
 *
 *     Used to process the generic class instance creation procedure and
 *     produce a really unique instance name automatically.
 *
 * Results:
 *     Check the command line arguments and create an instance of the
 *     specified class. Returns TCL_OK on success and TCL_ERROR otherwise.
 *
 * Side effects:
 *     None.
 *
 * ----------------------------------------------------------------------------
 */

int
TclppNewCmd (clientData, interp, objc, objv)
    ClientData clientData;       /* Not used */
    Tcl_Interp *interp;          /* The interpreter to create the instance in */
    int objc;                    /* Number of objects */
    Tcl_Obj * CONST objv[];      /* Array of objects:
                                  *     objv[0] = new
                                  *     objv[1] = class
                                  *     objv[n] = constructor args */
{
    Tclpp_Class* classPtr;       /* The class pointer */
    int count = 0;               /* Count of equally names instances */

    /*
     * Check the number of objects 
     */

    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "class ?constructor-args?");
        return TCL_ERROR;
    }

    /*
     * Get the class pointer
     */

    classPtr = Tclpp_GetClassByName(interp, Tcl_GetStringFromObj(objv[1],
            (int*) NULL));
    if (classPtr == (Tclpp_Class*) NULL) {
        Tcl_ResetResult(interp);
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "class \"",
                Tcl_GetStringFromObj(objv[1],(int*)NULL), "\" unknown", 
                (char*) NULL);
        return TCL_ERROR;
    }

    /*
     * Construct the fully qualified instance name which will be used to
     * create the namespace and the command with. Basically we prepend 
     * the current namespace name to the instance name. The instance name
     * itself consists of the string "Obj", followed by the creation
     * time (system dependent number of seconds) and a count value. We
     * kepp on trying an object name until we get a unqiue one.
     */

    while (1) {
        char instanceName[256];      /* Generated instance name */
        Tcl_Namespace *globalNS;     /* The global namespace */
        Tcl_Namespace *currentNS;    /* The current namespace */
        Tcl_DString name;            /* The fully qualified instance name */

        /*
         * Construct the fully qualified name with the nr-secs and count values
         */

        (void) sprintf(instanceName, "obj_%lu_%u", TclpGetSeconds(), count);

        /*
         * Prepend the namespace to the instance
         */

        Tcl_DStringInit (&name);
        currentNS = Tcl_GetCurrentNamespace(interp);
        globalNS = Tcl_GetGlobalNamespace(interp);
        if (currentNS == globalNS) {
            Tcl_DStringAppend(&name, currentNS->fullName, -1);
            Tcl_DStringAppend(&name, instanceName, -1);
        } else {
            Tcl_DStringAppend(&name, currentNS->fullName, -1);
            Tcl_DStringAppend(&name, "::", 2);
            Tcl_DStringAppend(&name, instanceName, -1);
        }

        /*
         * If this one is a unique object name, create the instance and
         * return that instance. Otherwise continue with a new count.
         */
        
        if (!instanceTableInitialized) {
            Tcl_InitHashTable(&instanceTable, TCL_STRING_KEYS);
            instanceTableInitialized = 1;
        }
        if (Tcl_FindHashEntry(&instanceTable, Tcl_DStringValue(&name))
                == (Tcl_HashEntry*) NULL) {
            int result;             /* Result of the instance creation */
            int localObjc, i;       /* Local object counter */
            Tcl_Obj **localObjv;    /* Local object array */

            /*
             * Create a local object array for invoking the instance cmd
             * procedure.
             */

            localObjc = objc;
            localObjv = (Tcl_Obj**) Tcl_Alloc (sizeof(Tcl_Obj*) * localObjc);
            localObjv[0] = Tcl_NewStringObj (classPtr->name, -1);
            localObjv[1] = Tcl_NewStringObj (instanceName, -1);
            for (i = 2; i < objc; i++) {
                localObjv[i] = objv[i];
            }

            /*
             * Try to create the object
             */

            Tcl_DStringFree(&name);
            result = TclppInstanceCmd ((ClientData) classPtr, interp, localObjc,
                    localObjv);
            Tcl_DecrRefCount(localObjv[0]);
            Tcl_DecrRefCount(localObjv[1]);
            Tcl_Free((char*)localObjv);
            return result;
        }
        Tcl_DStringFree(&name);
        count++;
    }
    /* Never reached */
}


/*
 * ----------------------------------------------------------------------------
 *
 * Tclpp_GetInstanceByName --
 *
 *     Returns the instance pointer of an instance specified by its name in
 *     the current namespace.
 *
 * Results:
 *     Returns the Tclpp_Instance of the instance if one could be located
 *     and NULL otherwise. Note that this will be an O(n) search through
 *     all instances.
 *
 * Side effects:
 *     None.
 *
 * ----------------------------------------------------------------------------
 */

Tclpp_Instance*
Tclpp_GetInstanceByName (interp, instanceName)
    Tcl_Interp *interp;          /* The interpreter */
    char *instanceName;          /* The instance name */
{
    Tcl_DString fullName;        /* The fully qualified instance name */
    Tcl_HashEntry *entryPtr;     /* The hash table entry */
    Tcl_HashSearch search;       /* Keep track of hash table search */
    Tcl_Namespace *globalNS;     /* The global namespace */
    Tcl_Namespace *currentNS;    /* The current namespace */
    TclppInstance *instancePtr;  /* The associated instance */

    /*
     * Get the fully qualified name of the instance. Basically we prepend
     * the current namespace to the instance name.
     */

    while (*instanceName == ':') {
        ++instanceName;
    }

    Tcl_DStringInit (&fullName);
    currentNS = Tcl_GetCurrentNamespace(interp);
    globalNS = Tcl_GetGlobalNamespace(interp);
    if (currentNS == globalNS) {
        Tcl_DStringAppend(&fullName, currentNS->fullName, -1);
        Tcl_DStringAppend(&fullName, instanceName, -1);
    } else {
        Tcl_DStringAppend(&fullName, currentNS->fullName, -1);
        Tcl_DStringAppend(&fullName, "::", 2);
        Tcl_DStringAppend(&fullName, instanceName, -1);
    }

    /*
     * Search through all instances
     */

    instancePtr = (TclppInstance*) NULL;
    for (entryPtr = Tcl_FirstHashEntry(&instanceTable, &search);
            entryPtr != NULL; entryPtr = Tcl_NextHashEntry(&search)) {
        TclppInstance* entryInstPtr;
        entryInstPtr = (TclppInstance*)Tcl_GetHashValue(entryPtr);
        if (strcmp(entryInstPtr->fullName, Tcl_DStringValue(&fullName)) == 0) {
            instancePtr = entryInstPtr;
        }
    }

    return (Tclpp_Instance*) instancePtr;
}


/*
 * ----------------------------------------------------------------------------
 *
 * TclppInvokeMethod --
 *
 *     Used to invoke the class defined methods through an instance.
 *
 * Results:
 *     The results of the invoked method is returned or an error if something
 *     went wrong.
 *
 * Side effects:
 *     The side effects of the invoked method apply.
 *
 * ----------------------------------------------------------------------------
 */

int
TclppInvokeMethod (clientData, interp, objc, objv)
    ClientData clientData;      /* The Tclpp_Instance instance */
    Tcl_Interp *interp;         /* The interpreter to run the method in */
    int objc;                   /* Number of objects */
    Tcl_Obj * CONST objv[];     /* Array of objects:
                                 *      objv[0] = instance
                                 *      objv[1] = method
                                 *      objv[n] = arguments */
{
    char *methodName;           /* Name of the method */

    /*
     * Check number of arguments
     */

    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "method ?args ...?");
        return TCL_ERROR;
    }
    
    /*
     * Check for built-in commands. If the method name matches with one
     * of them, call it.
     */

    methodName = Tcl_GetStringFromObj(objv[1], (int *)NULL);
    if ((*methodName == 'd') && strcmp(methodName, "delete") == 0) {
        return Tclpp_DeleteInstance(interp, (Tclpp_Instance*)clientData);
    }
    
    /*
     * Invoke the command. Strip off the first object from the object
     * array, since that is the name of the instance.
     */

    return Tclpp_InvokeMethod(interp, (Tclpp_Instance*)clientData, objc-1,
            &(objv[1]));
}


/*
 * ----------------------------------------------------------------------------
 *
 * TclppCreateInstanceVariable --
 *
 *     Create an instance variable in the specified instance namespace. The
 *     variable can either be a 'scalar', 'array' or (existing) class type.
 *     Initializes all scalar types to an empty string, except for the 'this'
 *     variable, which gets initialized to the instance name. Note that the
 *     constructor of the variable is not invoked.
 *
 * Results:
 *     Return TCL_OK when the instance variable could be created successfully. 
 *     Returns TCL_ERROR otherwise.
 *
 * Side effects:
 *     None.
 *
 * ----------------------------------------------------------------------------
 */

int
TclppCreateInstanceVariable (interp, instancePtr, variablePtr, nsPtr)
    Tcl_Interp *interp;          /* The interpreter to create the variable in */
    TclppInstance *instancePtr;  /* The instance to create the variable in */
    TclppVariable *variablePtr;  /* The variable to create */
    Namespace *nsPtr;            /* The namespace to create the variable in */
{
    Tcl_DString fullName;        /* The fully qualified name of the variable */
    TclppVariable *instVarPtr;   /* The instance counterpart of the variable */
    Tcl_HashEntry *entryPtr;     /* Hash table entry for new variable */
    int isNew;                   /* Set if the variable did not exist */

    /*
     * Create the full name of the variable to create.
     */

    Tcl_DStringInit (&fullName);
    TclppGetQualVariableName (instancePtr, variablePtr, &fullName);

    /*
     * If the type is a scalar or array, create the variable using the Tcl
     * variable creation commands. Otherwise we have a class instance which
     * we use the instance creation command to create it.
     */

    if (TclppVariableIsClass(variablePtr)) {
        TclppClass* varClassPtr;    /* The class variable */
        TclppInstance* varInstPtr;  /* The class variable instance */

        /*
         * Get the class to instantiate by name. Abort if the class could
         * not be found.
         */
        
        varClassPtr = (TclppClass*) Tclpp_GetClassByName (interp, 
                variablePtr->typeName);
        if (varClassPtr == (TclppClass*) NULL) {
            Tcl_DStringFree(&fullName);
            Tcl_ResetResult(interp);
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "class \"",
                    variablePtr->typeName, "\" unknown", (char*) NULL);
            return TCL_ERROR;
        }

        /*
         * Create the instance. This will create an object of it recursively,
         * including all its variables, which might be classes as well. The
         * constructor is not invoked (number of constructor arguments is 
         * negative). That will be accomplished when the entire instance has
         * been created.
         */

        varInstPtr = (TclppInstance*) Tclpp_CreateInstance (interp, 
                Tcl_DStringValue(&fullName), (Tclpp_Class*)varClassPtr,
                -1, (Tcl_Obj **) NULL);
        if (instancePtr == (TclppInstance*) NULL) {
            Tcl_DStringFree(&fullName);
            return TCL_ERROR;
        }

        /* 
         * Create a copy of the class' Tclpp_Variable and insert it into the
         * TclppInstance variable hash table.
         */

        instVarPtr = (TclppVariable*) Tcl_Alloc (sizeof(TclppVariable));
        *instVarPtr = *variablePtr;
        instVarPtr->fullName = (char*)Tcl_Alloc(Tcl_DStringLength(&fullName)+1);        strcpy (instVarPtr->fullName, Tcl_DStringValue(&fullName));
        instVarPtr->instancePtr = instancePtr;
        instVarPtr->instancePtr = instancePtr;
        instVarPtr->value.instancePtr = varInstPtr ;

        entryPtr = Tcl_CreateHashEntry(&(instancePtr->variableTable), 
                instVarPtr->fullName, &isNew);
        if (!isNew) {
            Tcl_DStringFree(&fullName);
            Tcl_ResetResult(interp);
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "variable \"",
                    instVarPtr->name, "\" already exists", (char *) NULL);
            Tcl_Free((char*)instVarPtr);
            return TCL_ERROR;
        }
        Tcl_SetHashValue(entryPtr, (ClientData)instVarPtr);
    } else {
        Var* varPtr;    /* The newly created variable structure */

        /*
         * Create the variable in the namespace
         */

        varPtr = (Var*) Tcl_Alloc (sizeof(Var));
        varPtr->name = NULL;
        varPtr->refCount = 0;
        varPtr->tracePtr = NULL;
        varPtr->searchPtr = NULL;

        entryPtr = Tcl_CreateHashEntry(&(nsPtr->varTable), variablePtr->name, 
                &isNew);
        if (!isNew) {
            Tcl_ResetResult(interp);
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp),
                    "Tclpp internal error: variable already created", 
                    (char*) NULL);
            return TCL_ERROR;
        }
        Tcl_SetHashValue (entryPtr, varPtr);
        varPtr->hPtr = entryPtr;
        varPtr->nsPtr = nsPtr;

        /*
         * Ensure that the flags are correct by overwriting them. How do we
         * ensure that the variable is an array which cannot be used as a
         * scalar ? At this point, an array can be used as a scalar as well !
         */

        varPtr->flags = VAR_NAMESPACE_VAR | VAR_UNDEFINED;
        if (TclppVariableIsScalar(variablePtr)) {
            varPtr->flags |= VAR_SCALAR;
            varPtr->value.objPtr = NULL;
        } else {
            varPtr->flags |= VAR_ARRAY;
            varPtr->value.tablePtr = 
                    (Tcl_HashTable*) ckalloc(sizeof (Tcl_HashTable));
            Tcl_InitHashTable(varPtr->value.tablePtr, TCL_STRING_KEYS);
        }

        /* 
         * Create a copy of the class' Tclpp_Variable and insert it into the
         * TclppInstance variable hash table.
         */

        instVarPtr = (TclppVariable*) Tcl_Alloc (sizeof(TclppVariable));
        *instVarPtr = *variablePtr;
        instVarPtr->fullName = (char*)Tcl_Alloc(Tcl_DStringLength(&fullName)+1);        strcpy (instVarPtr->fullName, Tcl_DStringValue(&fullName));
        instVarPtr->instancePtr = instancePtr;
        instVarPtr->value.varPtr = varPtr;

        entryPtr = Tcl_CreateHashEntry(&(instancePtr->variableTable), 
                instVarPtr->fullName, &isNew);
        if (!isNew) {
            Tcl_DStringFree(&fullName);
            Tcl_ResetResult(interp);
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "variable \"",
                    instVarPtr->name, "\" already exists", (char *) NULL);
            Tcl_Free((char*)instVarPtr);
            return TCL_ERROR;
        }
        Tcl_SetHashValue(entryPtr, (ClientData)instVarPtr);
    }

    Tcl_DStringFree(&fullName);

    return TCL_OK;
}


/*
 * ----------------------------------------------------------------------------
 *
 * TclppCreateInstanceVariables --
 *
 *     Creates all variables of a certain class (and all its base classes)
 *     for a given instance. Therefore, the instance can be of different 
 *     type than the class itself.
 *
 * Results:
 *     Return TCL_OK when all instance variables could be created successfully. 
 *     Returns TCL_ERROR otherwise.
 *
 * Side effects:
 *     None.
 *
 * ----------------------------------------------------------------------------
 */

int
TclppCreateInstanceVariables (interp, instancePtr, classPtr)
    Tcl_Interp *interp;          /* The interpreter to create the variable in */
    TclppInstance *instancePtr;  /* The instance to create the variable in */
    TclppClass *classPtr;        /* The class which variables to create */
{
    Tcl_DString nsName;          /* The fully qualified namespace name */
    Namespace *nsPtr;            /* The namespace holding the variables */
    Tcl_HashEntry *entryPtr;     /* Hash table entry for a base class */
    Tcl_HashSearch search;       /* Keep track of hash table search */

    /*
     * Create the namespace ::<instance>::<class>
     */

    Tcl_DStringInit(&nsName);
    Tcl_DStringAppend(&nsName, instancePtr->fullName, -1);
    Tcl_DStringAppend(&nsName, classPtr->fullName, -1);
    nsPtr = (Namespace*) Tcl_CreateNamespace (interp, 
            Tcl_DStringValue(&nsName), (ClientData)instancePtr, 
            (Tcl_NamespaceDeleteProc*) NULL);
    if (nsPtr == (Namespace*) NULL) {
        Tcl_DStringFree(&nsName);
        return TCL_ERROR;
    }
    Tcl_DStringFree(&nsName);

    /*
     * Create the variables in the namespace just created. Except for
     * the 'this' variable. That one will be created in the instance
     * namespace in the Tclpp_CreateInstance procedure.
     */

    for (entryPtr = Tcl_FirstHashEntry(
            &(classPtr->variableTable), &search); 
            entryPtr != (Tcl_HashEntry*) NULL; 
            entryPtr = Tcl_NextHashEntry(&search)) {
        TclppVariable *variablePtr;
        variablePtr = (TclppVariable*)Tcl_GetHashValue(entryPtr);

        if (variablePtr->name[0] == 't' 
                && strcmp(variablePtr->name,"this") == 0) {
            continue;
        } else {
             if (TclppCreateInstanceVariable(interp, instancePtr, 
                     variablePtr, nsPtr) != TCL_OK) {
                 return TCL_ERROR;
             }
        }
    }

    /* 
     * Recursively do the same for all the base classes.
     */

    for (entryPtr = Tcl_FirstHashEntry(&(classPtr->baseTable), &search); 
            entryPtr != (Tcl_HashEntry*) NULL;
            entryPtr = Tcl_NextHashEntry(&search)) {
        TclppClass *baseClassPtr;   /* The base class */

        baseClassPtr = (TclppClass*)Tcl_GetHashValue(entryPtr);
        if (TclppCreateInstanceVariables (interp, instancePtr, baseClassPtr)
                != TCL_OK) {
            return TCL_ERROR;
        }
    }

    return TCL_OK;
}


/*
 * ----------------------------------------------------------------------------
 *
 * Tclpp_CreateInstance --
 *
 *     Create an instance of a certain class. This results in a new namespace
 *     for the instance, a command to access the class functions through the
 *     instance. The constructor is automatically invoked as well, unless
 *     the number of argument object is a negative number.
 *
 * Results:
 *     Returns the newly created instance if everything went ok. Returns 
 *     NULL otherwise and sets the result string in the interpreter.
 *
 * Side effects:
 *     A namespace and command are created whose name is equal to the name
 *     of the instance. On any error, a namespace and/or command might be
 *     created but not removed.
 *
 * ----------------------------------------------------------------------------
 */

Tclpp_Instance*
Tclpp_CreateInstance (interp, fullName, classPtr, objc, objv)
    Tcl_Interp *interp;         /* The interpreter to create the class in */
    char *fullName;             /* Fully qualified name of the instance. Must
                                 * start with a :: */
    Tclpp_Class *classPtr;      /* The class to create an instance of */
    int objc;                   /* Number of constructor arguments */
    Tcl_Obj * CONST objv[];     /* The constructor arguments */
{
    char *name;                 /* Unqualified instance name, containing
                                 * no ::'s at all */
    Tcl_Namespace *instanceNS;  /* The instance namespace */
    Tcl_HashEntry *entryPtr;    /* Hash table entry for new class */
    int isNew;                  /* Set if the class exists in hashtable */
    TclppInstance *instancePtr; /* The new instance structure */
    Tcl_CmdInfo cmdInfo;        /* Information about a certain command */
    Tcl_Command cmd;            /* The Tcl command procedure */
    TclppVariable variable;     /* A temporary variable structure to invoke
                                 * the constructor */

    /*
     * Get the simple instance name, which is from the last occurence of ':' 
     * of the full name.
     */

    name = strrchr (fullName, ':') + 1;

    /*
     * Check if the instance already exists in the instanceTable
     */

    if (!instanceTableInitialized) {
        Tcl_InitHashTable(&instanceTable, TCL_STRING_KEYS);
        instanceTableInitialized = 1;
    }
    entryPtr = Tcl_FindHashEntry(&instanceTable, fullName);
    if (entryPtr != (Tcl_HashEntry*) NULL) {
        Tcl_ResetResult(interp);
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "instance \"",
                fullName, "\" already exists", (char *) NULL);
        return (Tclpp_Instance*) NULL;
    }

    /*
     * Check if the instance exists as a command. Note that we must use the
     * fully qualified name (including namespace) for this.
     */

    if (Tcl_GetCommandInfo(interp, fullName, &cmdInfo)) {
        Tcl_ResetResult(interp);
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "command \"",
                fullName, "\" already exists", (char *) NULL);
        return (Tclpp_Instance*) NULL;
    }

    /*
     * Create the main namespace
     */

    instancePtr = (TclppInstance*) Tcl_Alloc(sizeof(TclppInstance));
    instanceNS = Tcl_CreateNamespace (interp, fullName, 
            (ClientData) instancePtr, TclppDeleteInstance);
    if (instanceNS == NULL) {
        Tcl_Free((char*) instancePtr);
        return (Tclpp_Instance*) NULL;
    }

    /*
     * Create an entry in the instanceTable.
     */

    entryPtr = Tcl_CreateHashEntry(&instanceTable, fullName, &isNew);
    if (!isNew) {
        Tcl_ResetResult(interp);
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "instance \"",
                fullName, "\" already exists", (char *) NULL);
        Tcl_DeleteNamespace(instanceNS);
        Tcl_Free((char*) instancePtr);
        return (Tclpp_Instance*) NULL;
    }

    instancePtr->name = (char *) Tcl_Alloc(strlen(name)+1);
    strcpy(instancePtr->name, name);
    instancePtr->fullName = (char *) Tcl_Alloc(strlen(fullName)+1);
    strcpy(instancePtr->fullName, fullName);
    instancePtr->classPtr = classPtr;
    instancePtr->nsPtr = (Namespace*)instanceNS;
    Tcl_InitHashTable(&(instancePtr->variableTable), TCL_STRING_KEYS);
    Tcl_SetHashValue(entryPtr, (ClientData)instancePtr);

    /* 
     * Search for the 'this' variable and create an instance of it.
     */

    entryPtr = Tcl_FindHashEntry(&(classPtr->variableTable), "this");
    if (entryPtr == (Tcl_HashEntry*) NULL) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "unable to locate ",
                "'this' variable", (char *)NULL);
        return (Tclpp_Instance*) NULL;
    }
    if (TclppCreateInstanceVariable (interp, instancePtr, (TclppVariable*)
            Tcl_GetHashValue(entryPtr), (Namespace*)instanceNS) != TCL_OK) {
        return (Tclpp_Instance*) NULL;
    }

    /*
     * Create the instance variables for this class and all its base
     * classes (recursively).
     */

    if (TclppCreateInstanceVariables (interp, instancePtr, (TclppClass*)
            classPtr) != TCL_OK) {
        return (Tclpp_Instance*) NULL;
    }

    /*
     * Create the command to invoke the class defined methods.
     */

    cmd = Tcl_CreateObjCommand(interp, fullName, 
            (Tcl_ObjCmdProc*) &TclppInvokeMethod, (ClientData) instancePtr, 
            (Tcl_CmdDeleteProc *) NULL);
    instancePtr->cmdPtr = (Command*) cmd;
   
    /*
     * Call the constructor if required using a temporary variable.
     */

    if (objc >= 0) {
        variable.instancePtr = instancePtr;
        variable.value.instancePtr = instancePtr;
        variable.ctProc = TclppClassConstructor;
        if ((*variable.ctProc)(interp, &variable, objc, objv) != TCL_OK) {
            (void) Tcl_DeleteCommand(interp, fullName);
            return (Tclpp_Instance*) NULL;
        }
    }

    /*
     * That's it. 
     */

    return (Tclpp_Instance*) instancePtr;
}


/*
 * ----------------------------------------------------------------------------
 *
 * TclppDeleteInstance --
 *
 *     Callback function called when the namespace is deleted in which the
 *     instance was created. It will remove the reference from the instance
 *     table.
 *
 * Results:
 *     Remove the instance properly.
 *
 * Side effects:
 *     None.
 *
 * ----------------------------------------------------------------------------
 */

static void
TclppDeleteInstance (clientData)
    ClientData clientData;       /* The Tclpp_Instance pointer */
{
    Tclpp_Instance* instancePtr; /* The instance to delete */
    Tcl_HashEntry* entryPtr;     /* The associated table entry */

    instancePtr = (Tclpp_Instance*) clientData;

    /*
     * Get the entry from the hash table and remove it.
     */

    entryPtr = Tcl_FindHashEntry (&instanceTable, instancePtr->fullName);
    if (entryPtr != (Tcl_HashEntry*) NULL) {
        Tcl_DeleteHashEntry(entryPtr);
    }

    /*
     * Free the instance pointer
     */

    Tcl_Free((char*) instancePtr);
}


/*
 * ----------------------------------------------------------------------------
 *
 * Tclpp_DeleteInstance --
 *
 *     Remove an instance by removing the namespace, command and the entry
 *     in the instance hash table. The instance-structure is deleted when
 *     the namespace is deleted by TclppDeleteInstance.
 *
 * Results:
 *     Return TCL_OK on success and TCL_ERROR otherwise.
 *
 * Side effects:
 *     None.
 *
 * ----------------------------------------------------------------------------
 */

int
Tclpp_DeleteInstance (interp, instancePtr)
    Tcl_Interp* interp;          /* Current interpreter */
    Tclpp_Instance* instancePtr; /* The instance to delete */
{
    Tcl_HashEntry* entryPtr;     /* The associated table entry */
    TclppVariable variable;      /* A temporary variable structure to invoke
                                  * the destructor */

    /*
     * Call the destructor for this instance. It will recusrively call
     * the destructor of all its variables as well.
     */

    variable.instancePtr = (TclppInstance*) instancePtr;
    variable.value.instancePtr = (TclppInstance*) instancePtr;
    variable.dtProc = TclppClassDestructor;
    (*variable.dtProc)(((TclppInstance*)instancePtr)->nsPtr->interp, &variable);

    /*
     * Get the entry from the hash table and remove it.
     */

    entryPtr = Tcl_FindHashEntry (&instanceTable, instancePtr->fullName);
    if (entryPtr != (Tcl_HashEntry*) NULL) {
        Tcl_DeleteHashEntry(entryPtr);
    }

    /* 
     * Delete the command and the namespace. This will invoke the function
     * TclppDeleteInstance automatically. 
     */

    Tcl_DeleteNamespace((Tcl_Namespace*)((TclppInstance*)instancePtr)->nsPtr);
    Tcl_DeleteCommandFromToken(interp, 
            (Tcl_Command)((TclppInstance*)instancePtr)->cmdPtr);

    return TCL_OK;
}

/*
 * vi: set ai expandtab ts=4: 
 */

