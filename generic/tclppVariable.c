/*
 * tclppVariable.c --
 *
 *     This file implements functionality to use class members. This
 *     includes the registration of variables, handle variable adornments
 *     such as 'static' and the binding of class variables to instances
 *     inside class methods.
 *
 * RCS: $Id: tclppVariable.c,v 1.2 2000/05/23 19:34:13 stefan Exp $
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

static char rcsid[] = "$Id: tclppVariable.c,v 1.2 2000/05/23 19:34:13 stefan Exp $";


/*
 * ----------------------------------------------------------------------------
 *     
 * TclppVariableCmd --
 * 
 *     Command to use instead of the original Tcl variable command such
 *     that we can do registration of this class member.
 *
 * Results:
 *     See 'Tclpp_CreateVariable'.
 *  
 * Side effects:
 *     See 'Tclpp_CreateVariable'.
 *  
 * ----------------------------------------------------------------------------
 */ 

int
TclppVariableCmd (clientData, interp, objc, objv)
    ClientData clientData;      /* Not used */
    Tcl_Interp *interp;         /* The interpreter to create the command in */
    int objc;                   /* Number of objects */
    Tcl_Obj * CONST objv[];     /* Array of objects */
{
    Tclpp_Class *classPtr;      /* The current class */
    Tclpp_Variable *variablePtr;/* The created variable */

    /*
     * Check the number of objects
     */

    if (objc != 3) {
        Tcl_WrongNumArgs(interp, 1, objv, "type name");
        return TCL_ERROR;
    }

    /*
     * Get the current class definition.
     */

    classPtr = Tclpp_GetCurrentClass (interp);
    if (classPtr == (Tclpp_Class*) NULL) {
        Tcl_ResetResult(interp);
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), 
                "Tclpp internal error: not in class definition context", 
                (char *)NULL);
        return TCL_ERROR;
    }

    /* 
     * Create the class variable
     */

    variablePtr = Tclpp_CreateVariable (interp, classPtr, 
		    Tcl_GetStringFromObj(objv[1], (int*)NULL),
		    Tcl_GetStringFromObj(objv[2], (int*)NULL));
    return ((variablePtr == (Tclpp_Variable*)NULL) ? TCL_ERROR : TCL_OK);
}


/*
 * ----------------------------------------------------------------------------
 *     
 * TclppGetQualVariableName --
 *
 *     Returns the fully qualified variable name, given the instance and
 *     the unqualified variable name. Takes into account the special case
 *     for the 'this' variable name.
 *
 * Result:
 *     Returns the fully qualified name in the dynamic string.
 *
 * Side effects:
 *     None
 *
 * ----------------------------------------------------------------------------
 */

void
TclppGetQualVariableName (instancePtr, variablePtr, qualName)
    TclppInstance *instancePtr;     /* The instance */
    TclppVariable *variablePtr;     /* The variable */
    Tcl_DString *qualName;          /* Returns the fully qualified name */
{
    Tcl_DStringFree(qualName);
    Tcl_DStringAppend(qualName, instancePtr->fullName, -1);
    if (variablePtr->name[0] == 't' && strcmp(variablePtr->name, "this") == 0) {
        Tcl_DStringAppend(qualName, "::this", 6);
    } else {
        Tcl_DStringAppend(qualName, variablePtr->classPtr->fullName, -1);
        Tcl_DStringAppend(qualName, "::", 2);
        Tcl_DStringAppend(qualName, variablePtr->name, -1);
    }
}


/*
 * ----------------------------------------------------------------------------
 *     
 * TclppGetVariable --
 *
 *     Returns the variable structure for a certain instance, based on the
 *     name. The variable is either located in the instance class or one of
 *     its base classes. 
 *
 * Result:
 *     Returns the variable structure or NULL if such a variable could not
 *     be located.
 *
 * Side effects:
 *     None
 *
 * ----------------------------------------------------------------------------
 */

TclppVariable*
TclppGetVariable (classPtr, name)
    TclppClass *classPtr;           /* The class to start the search from */
    char *name;                     /* The variable to search for */
{
    Tcl_HashEntry *entryPtr;        /* The variable hash table entry */
    Tcl_HashSearch search;          /* Keep track of hash table search */

    /*
     * First try to locate it in the current class's variable list.
     */

    entryPtr = Tcl_FindHashEntry(&(classPtr->variableTable), name);
    if (entryPtr != (Tcl_HashEntry*) NULL) {
        return (TclppVariable*) Tcl_GetHashValue(entryPtr);
    }

    /*
     * Variable could not be located in the current class, try the base
     * classes. Note that this is probably not the best order to use since
     * it does a depth-first search approach, instead of a breadth-first 
     * search.
     * This need to be fixed!
     */

    for (entryPtr = Tcl_FirstHashEntry(&(classPtr->baseTable), &search);
            entryPtr != (Tcl_HashEntry*) NULL;
            entryPtr = Tcl_NextHashEntry(&search)) {
        TclppClass    *baseClassPtr;    /* The base class */
        TclppVariable *variablePtr;     /* The variable pointer */

        baseClassPtr = (TclppClass*)Tcl_GetHashValue(entryPtr);
        variablePtr = TclppGetVariable (baseClassPtr, name);
        if (variablePtr != (TclppVariable*) NULL) {
             return variablePtr;
        }
    }

    return (TclppVariable*) NULL;
}


/*
 * ----------------------------------------------------------------------------
 *     
 * Tclpp_CreateVariable --
 * 
 *     Create the variable in the current class definition. The variable is
 *     only registered. When an instance is made, it is actually created.
 *     That is, unless the variable is adorned with the 'static' keyword.
 *     In that case, the variable is created inside the class definition and
 *     not in any instance.
 *
 * Results:
 *     The variable is registered in the class definition. Returns the 
 *     created variable or NULL on any error. In that case the reason is
 *     appended to the interpreter.
 *  
 * Side effects:
 *     None.
 *  
 * ----------------------------------------------------------------------------
 */ 

Tclpp_Variable*
Tclpp_CreateVariable (interp, classPtr, type, name)
    Tcl_Interp *interp;         /* The interpreter to create the variable in */
    Tclpp_Class *classPtr;      /* The class for this variable */
    char* type;			        /* The type of the variable */
    char* name;			        /* The name of the variable */
{
    Tcl_DString fullName;       /* The fully qualified variable name */
    Tcl_HashEntry *entryPtr;    /* Hash table entry for new variable */
    int isNew;                  /* Set if the variable exists in the table */
    TclppVariable *variablePtr; /* The new class variable structure */

    /*
     * Check if variable already exists in this class.
     */

    entryPtr = Tcl_FindHashEntry(&(classPtr->variableTable), name);
    if (entryPtr != (Tcl_HashEntry*) NULL) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "class variable \"",
            name, "\" already exists", (char *)NULL);
        return (Tclpp_Variable*) NULL;
    }

    /*
     * Register the class by adding the TclppVariable to the class's 
     * variable hash-table.
     */

    entryPtr = Tcl_CreateHashEntry (&(classPtr->variableTable),
        name, &isNew);
    if (!isNew) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "class variable \"",
            name, "\" already exists", (char *)NULL);
        return (Tclpp_Variable*) NULL;
    }

    variablePtr = (TclppVariable*) Tcl_Alloc(sizeof(TclppVariable));
    variablePtr->name = (char *) Tcl_Alloc(strlen(name)+1);
    strcpy(variablePtr->name, name);
    if (strcmp(type, "scalar") == 0) {
        TclppVariableSetScalar(variablePtr);
        variablePtr->ctProc = TclppScalarConstructor;
        variablePtr->dtProc = TclppScalarDestructor;
    } else if (strcmp(type, "array") == 0) {
        TclppVariableSetArray(variablePtr);
        variablePtr->ctProc = TclppArrayConstructor;
        variablePtr->dtProc = TclppArrayDestructor;
    } else {
        TclppVariableSetClass(variablePtr);
        variablePtr->ctProc = TclppClassConstructor;
        variablePtr->dtProc = TclppClassDestructor;
    }

    Tcl_DStringInit (&fullName);
    Tcl_DStringAppend(&fullName, classPtr->fullName, -1);
    Tcl_DStringAppend(&fullName, "::", 2);
    Tcl_DStringAppend(&fullName, name, -1);
    variablePtr->fullName = (char *) Tcl_Alloc(Tcl_DStringLength(&fullName)+1);
    strcpy(variablePtr->fullName, Tcl_DStringValue(&fullName));
    Tcl_DStringFree (&fullName);

    variablePtr->typeName = (char *) Tcl_Alloc(strlen(type)+1);
    strcpy(variablePtr->typeName, type);
    variablePtr->flags = 0;
    variablePtr->classPtr = (TclppClass*) classPtr;
    Tcl_SetHashValue(entryPtr, (ClientData)variablePtr);

    /*
     * In case the variable is a 'static' one, it will be created here
     * as well. It will not be created in any instance.
     */

    TclppResetVariableAdornment();

    return (Tclpp_Variable*)variablePtr;
}


/*
 * ----------------------------------------------------------------------------
 *     
 * TclppResolveClassVar --
 *     Variable name resolver for the class namespaces when a class procedure
 *     is invoked. It will lookup in the current instance for a matching
 *     instance variable. If one is found, the variable is linked to the
 *     associated instance variable.
 *
 * Results:
 *     Returns the link to the instance variable if a variable with the
 *     matching name could be located. Returns TCL_OK if one could be 
 *     retrieved and TCL_CONTINUE otherwise.
 *  
 * Side effects:
 *     None.
 *  
 * ----------------------------------------------------------------------------
 */ 

int
TclppResolveClassVar (interp, name, context, flags, rPtr)
    Tcl_Interp *interp;          /* The interpreter to lookup the variable */
    char *name;                  /* Name of the variable */
    Tcl_Namespace *context;      /* The class namespace */
    int flags;                   /* Variable flags */
    Tcl_Var *rPtr;               /* The variable if one is found */
{
    Tcl_DString varName;         /* The fully qualified variable name */
    TclppContext *contextPtr;    /* The current context */
    TclppInstance *instancePtr;  /* The current instance */
    TclppClass *classPtr;        /* The current class */
    TclppVariable *variablePtr;  /* The class/instance variable pointer */
    Tcl_HashEntry *entryPtr;     /* Hash table entry to find the variable */

    /*
     * Get the current instance. If there is none, there should be an error
     * because we could not enter this function then ! 
     */

    contextPtr = TclppGetCurrentContext();
    if (contextPtr == (TclppContext*) NULL) {
        panic ("resolver called but empty context stack");
    }
    instancePtr = contextPtr->instancePtr;
    classPtr = contextPtr->classPtr;

    /*
     * Try to get the variable from the class variables or its base class
     * variables.
     */

    variablePtr = TclppGetVariable (classPtr, name);
    if (variablePtr == (TclppVariable*) NULL) {
        return TCL_CONTINUE;
    }

    /*
     * If we could find the variable as one of the instance variables, setup 
     * the variable pointer to link it to the instance variable.
     */

    Tcl_DStringInit(&varName);
    TclppGetQualVariableName(instancePtr, variablePtr, &varName);
    entryPtr = Tcl_FindHashEntry(&(instancePtr->variableTable), 
            Tcl_DStringValue(&varName));
    Tcl_DStringFree(&varName);
    if (entryPtr != (Tcl_HashEntry*) NULL) {
        variablePtr = (TclppVariable*) Tcl_GetHashValue(entryPtr);
        *rPtr = (Tcl_Var) variablePtr->value.varPtr;
        return TCL_OK;
    }
   
    /*
     * Variable could not be found, so continue to search for local variables.
     */

    return TCL_CONTINUE;
}


/*
 * ----------------------------------------------------------------------------
 *     
 * TclppResolveRuntimeClassVar --
 *     This function gets invoked for each compiled local which needs to
 *     resolve its name. This means that we have to link this variable to
 *     the corresponding instance variable.
 *
 * Results:
 *     The static variable information block is changed.
 *  
 * Side effects:
 *     None.
 *  
 * ----------------------------------------------------------------------------
 */ 

Tcl_Var
TclppResolveRuntimeClassVar (interp, rPtr)
    Tcl_Interp *interp;             /* The interpreter */
    Tcl_ResolvedVarInfo *rPtr;      /* The resolving info block */
{
    TclppVariable *variablePtr;     /* The instance variable pointer */

    variablePtr = ((TclppResolvedVarInfo*)rPtr)->variablePtr;
    return (Tcl_Var) variablePtr->value.varPtr;
}


/*
 * ----------------------------------------------------------------------------
 *     
 * static TclppResolveDeleteClassVar --
 *     This function gets invoked for each compiled local which gets deleted
 *     because of recompilation or because the procedure is deleted.
 *
 * Results:
 *     Remove the info block allocated by TclppResolveClassCompiledVar.
 *  
 * Side effects:
 *     None.
 *  
 * ----------------------------------------------------------------------------
 */ 

static void
TclppResolveDeleteClassVar (rPtr)
    Tcl_ResolvedVarInfo *rPtr;          /* The (static) info block. */
{
    Tcl_Free((char*)rPtr);
}


/*
 * ----------------------------------------------------------------------------
 *     
 * TclppResolveClassCompiledVar --
 *     Variable name resolver for the class namespaces when a class procedure
 *     is invoked for compiled variables. If the variable is a known instance
 *     variable, it will setup a new 'Tcl_ResolvedVarInfo' linked to the
 *     instance variable. This block of info gets used by the runtime resolver
 *     and deleted desolveDeleteClassVar function when appropriate.
 *     If the variable is not located, it should be a local variable.
 *
 * Results:
 *     Returns the link to the instance variable if a variable with the
 *     matching name could be located. Returns TCL_OK if one could be 
 *     retrieved and TCL_CONTINUE otherwise.
 *  
 * Side effects:
 *     The returned Tcl_ResolvedVarInfo pointer point to the static resolve
 *     info block. This means that Tcl may not delete this block such that
 *     we have to specoify the deleteProc as well.
 *  
 * ----------------------------------------------------------------------------
 */ 

int
TclppResolveClassCompiledVar (interp, name, length, context, rPtrPtr)
    Tcl_Interp *interp;             /* The interpreter */
    char *name;                     /* Name of the variable */
    int length;                     /* Length of the variable name */
    Tcl_Namespace *context;         /* The class namespace */
    Tcl_ResolvedVarInfo **rPtrPtr;  /* The resolving info block */
{
    Tcl_DString varName;            /* The fully qualified variable name */
    TclppContext *contextPtr;       /* The current context */
    TclppInstance *instancePtr;     /* The current instance */
    TclppClass *classPtr;           /* The current class */
    TclppVariable *variablePtr;     /* The class/instance variable pointer */
    Tcl_HashEntry *entryPtr;        /* Hash table entry to find the variable */

    /*
     * Get the current instance. If there is none, there should be an error
     * because we could not enter this function then ! 
     */

    contextPtr = TclppGetCurrentContext();
    if (contextPtr == (TclppContext*) NULL) {
        panic ("compiled resolver called but empty context stack");
    }
    instancePtr = contextPtr->instancePtr;
    classPtr = contextPtr->classPtr;

    /*
     * Try to get the variable from the class variables or its base class
     * variables.
     */

    variablePtr = TclppGetVariable (classPtr, name);
    if (variablePtr == (TclppVariable*) NULL) {
        return TCL_CONTINUE;
    }

    /*
     * If we could find the variable as one of the instance variables, setup 
     * the resolved variable structure and return TCL_OK. 
     */

    Tcl_DStringInit(&varName);
    TclppGetQualVariableName(instancePtr, variablePtr, &varName);
    entryPtr = Tcl_FindHashEntry(&(instancePtr->variableTable), 
            Tcl_DStringValue(&varName));
    Tcl_DStringFree(&varName);
    if (entryPtr != (Tcl_HashEntry*) NULL) {
        variablePtr = (TclppVariable*) Tcl_GetHashValue(entryPtr);
        *rPtrPtr = (Tcl_ResolvedVarInfo *) Tcl_Alloc(
                sizeof(TclppResolvedVarInfo));
        (*rPtrPtr)->fetchProc  = (Tcl_ResolveRuntimeVarProc*)
                TclppResolveRuntimeClassVar;
        (*rPtrPtr)->deleteProc = (Tcl_ResolveVarDeleteProc*)
                TclppResolveDeleteClassVar;
        ((TclppResolvedVarInfo*)(*rPtrPtr))->variablePtr = variablePtr;
        return TCL_OK;
    }

    /*
     * The variable is not an instance variable, so continue
     */

    return TCL_CONTINUE;
}


/*
 * ----------------------------------------------------------------------------
 *     
 * TclppScalarConstructor
 *     Constructor for a scalar variable. The scalar gets initialized to
 *     an empty value, unles it is the 'this' variable. In that case we
 *     define it as the name of the instance.
 *
 * Results:
 *     Returns TCL_OK.
 *  
 * Side effects:
 *     None.
 *  
 * ----------------------------------------------------------------------------
 */ 

int
TclppScalarConstructor (interp, variablePtr, objc, objv)
    Tcl_Interp *interp;         /* Current interpreter */
    TclppVariable *variablePtr; /* The variable to call the constructor for */
    int objc;                   /* Not used. */
    Tcl_Obj * CONST objv[];     /* Not used. */
{
    TclppInstance *instancePtr; /* The instance the variable is created in */
    Tcl_DString name;           /* The fully qualified name of the variable */
    char *value;                /* The value of the variable */

    /*
     * Get the full name of the variable
     */

    instancePtr = variablePtr->instancePtr;
    Tcl_DStringInit (&name);
    TclppGetQualVariableName (instancePtr, variablePtr, &name);

    /*
     * Get the value and initialize it.
     */

    if ((*variablePtr->name == 't') && (strcmp(variablePtr->name, "this")) 
            == 0) {
        value = instancePtr->fullName;
    } else {
        value = (char*) NULL;
    }

    (void) Tcl_SetVar(interp,Tcl_DStringValue(&name),value,TCL_NAMESPACE_ONLY);

    Tcl_DStringFree(&name);

    return TCL_OK;
}


/*
 * ----------------------------------------------------------------------------
 *     
 * TclppArrayConstructor
 *     Constructor for an array variable. Basically nothing.
 *
 * Results:
 *     Returns TCL_OK.
 *  
 * Side effects:
 *     None.
 *  
 * ----------------------------------------------------------------------------
 */ 

int
TclppArrayConstructor (interp, variablePtr, objc, objv)
    Tcl_Interp *interp;         /* Current interpreter */
    TclppVariable *variablePtr; /* The variable to call the constructor for */
    int objc;                   /* Not used. */
    Tcl_Obj * CONST objv[];     /* Not used. */
{
    return TCL_OK;
}


/*
 * ----------------------------------------------------------------------------
 *     
 * TclppInvokeClassConstructor
 *     Check if these is a constructor to invoke. If there is, invoke it
 *     and do the same for any base class (recursively). Only the first
 *     constructor, that is, the constructor of the actual type of the
 *     variable will be passed the given arguments. All others (like the
 *     base classes) are passed without any arguments.
 *
 * Results:
 *     Returns TCL_OK on success and TCL_ERROR on error. The absence of
 *     a constructor function is not an error.
 *  
 * Side effects:
 *     None.
 *  
 * ----------------------------------------------------------------------------
 */ 

int
TclppInvokeClassConstructor (interp, variablePtr, classPtr, objc, objv)
    Tcl_Interp *interp;         /* Current interpreter */
    TclppVariable *variablePtr; /* The variable to call the constructor for */
    TclppClass *classPtr;       /* The (base) class of the variable */
    int objc;                   /* Number of constructor arguments */
    Tcl_Obj * CONST objv[];     /* Array of constructor arguments */
{
    TclppInstance *instancePtr; /* The instance of the class variable */
    Tcl_HashEntry* entryPtr;    /* Hash table entry */
    Tcl_HashSearch search;      /* Keep track of hash table search */
    char *ctName;               /* Name of the constructor function, equal to
                                 * the name of the class */
    int result = TCL_OK;        /* Result of the coistructor call */

    /*
     * Do it recursively for all the base classes.
     */

    for (entryPtr = Tcl_FirstHashEntry(&(classPtr->baseTable), &search);
            result == TCL_OK && entryPtr != (Tcl_HashEntry*) NULL;
            entryPtr = Tcl_NextHashEntry(&search)) {
        TclppClass *baseClassPtr;   /* The base class */

        baseClassPtr = (TclppClass*)Tcl_GetHashValue(entryPtr);
        result = TclppInvokeClassConstructor(interp, variablePtr, baseClassPtr,
                0, (Tcl_Obj **)NULL);
    }

    /*
     * Get the name of the constructor (without any preceeding ':' or other
     * subclass/namespace specification) method
     * from the associated class.
     */

    instancePtr = variablePtr->value.instancePtr;
    ctName = classPtr->name;

    /*
     * If there is a constructor method call it. We use the Tclpp_InvokeMethod
     * to resolve any variable linkage. But for that, we need to add an
     * extra parameter (the name of the method) to the list of arguments.
     */

    entryPtr = Tcl_FindHashEntry (&(classPtr->methodTable), ctName);
    if (result == TCL_OK && entryPtr != (Tcl_HashEntry*) NULL) {
        int localObjc, i;
        Tcl_Obj ** localObjv;

        /*
         * Create a new block of arguments, which is the ones specified
         * prepended with the constructor name.
         */

        localObjc = objc + 1;
        localObjv = (Tcl_Obj**) Tcl_Alloc (sizeof(Tcl_Obj*) * localObjc);
        localObjv[0] = Tcl_NewStringObj (ctName, -1);
        for (i = 0; i < objc; i++) {
            localObjv[i+1] = objv[i];
        }

        /*
         * Call the constructor. 
         */

        result = Tclpp_InvokeMethod (interp, (Tclpp_Instance*) instancePtr, 
                localObjc, localObjv);
        Tcl_DecrRefCount(localObjv[0]);
        Tcl_Free((char*)localObjv);
    } else {
        /*
         * There is no constructor defined 
         */

        result = TCL_OK;
    }

    return result;
}


/*
 * ----------------------------------------------------------------------------
 *     
 * TclppClassConstructor
 *     Constructor for a class variable. It will invoke the constructor
 *     of all its variables (recursively) and base classes. It will invoke
 *     any user-defined constructor method if present.
 *
 * Results:
 *     Returns TCL_OK on success and TCL_ERROR on error. The absence of
 *     a constructor function is not an error.
 *  
 * Side effects:
 *     None.
 *  
 * ----------------------------------------------------------------------------
 */ 

int
TclppClassConstructor (interp, variablePtr, objc, objv)
    Tcl_Interp *interp;         /* Current interpreter */
    TclppVariable *variablePtr; /* The variable to call the constructor for */
    int objc;                   /* Number of constructor arguments */
    Tcl_Obj * CONST objv[];     /* Array of constructor arguments */
{
    TclppInstance *instancePtr; /* The instance of the class variable */
    TclppClass* classPtr;       /* The class type of the variable */
    Tcl_HashEntry* entryPtr;    /* Hash table entry */
    Tcl_HashSearch search;      /* Keep track of hash table search */

    /*
     * For each variable of this class instance, invoke the constructor of 
     * these variable first. Note that it will be the default constructor 
     * without any arguments ! Note: these already include all the base
     * class variables as well.
     */

    instancePtr = variablePtr->value.instancePtr;
    for (entryPtr = Tcl_FirstHashEntry(&(instancePtr->variableTable), &search);
            entryPtr != NULL; entryPtr = Tcl_NextHashEntry(&search)) {
        TclppVariable* variablePtr;
        variablePtr = (TclppVariable*)Tcl_GetHashValue(entryPtr);
        if ((*variablePtr->ctProc)(interp, variablePtr, 0, (Tcl_Obj**) NULL) 
                != TCL_OK) {
            return TCL_ERROR;
        }
    }

    /*
     * Invoke the constructor of this class and all its base classes.
     */

    classPtr = (TclppClass*) instancePtr->classPtr;
    if (TclppInvokeClassConstructor(interp, variablePtr, classPtr, objc, objv)
            != TCL_OK) {
        return TCL_ERROR;
    }

    return TCL_OK;
}


/*
 * ----------------------------------------------------------------------------
 *     
 * TclppScalarDestructor
 *     Constructor for a scalar variable. Does nothing. 
 *
 * Results:
 *     None.
 *  
 * Side effects:
 *     None.
 *  
 * ----------------------------------------------------------------------------
 */ 

void
TclppScalarDestructor (interp, variablePtr)
    Tcl_Interp *interp;         /* Current interpreter */
    TclppVariable *variablePtr; /* The variable to call the destructor for */
{
}


/*
 * ----------------------------------------------------------------------------
 *     
 * TclppArrayDestructor
 *     Destructor for an array variable. Does nothing.
 *
 * Results:
 *     None
 *  
 * Side effects:
 *     None.
 *  
 * ----------------------------------------------------------------------------
 */ 

void
TclppArrayDestructor (interp, variablePtr)
    Tcl_Interp *interp;         /* Current interpreter */
    TclppVariable *variablePtr; /* The variable to call the destructor for */
{
}


/*
 * ----------------------------------------------------------------------------
 *     
 * TclppInvokeClassDestructor
 *     Check if these is a destructor to invoke. If there is, invoke it
 *     and do the same for any base class (recursively). 
 *
 * Results:
 *     Returns TCL_OK on success and TCL_ERROR on error. The absence of
 *     a destructor function is not an error.
 *  
 * Side effects:
 *     None.
 *  
 * ----------------------------------------------------------------------------
 */ 

void
TclppInvokeClassDestructor (interp, instancePtr, classPtr)
    Tcl_Interp *interp;         /* Current interpreter */
    TclppInstance *instancePtr; /* The instance to call the destructor for */
    TclppClass *classPtr;       /* The (base) class of the variable */
{
    Tcl_HashEntry* entryPtr;    /* Hash table entry */
    Tcl_HashSearch search;      /* Keep track of hash table search */
    Tcl_DString dtName;         /* Name of the destructor function, equal to
                                 * the name of the class, prepended with '~' */

    /*
     * Get the name of the destructor (without any preceeding ':' or other
     * subclass/namespace specification) of the associated class.
     */

    Tcl_DStringInit(&dtName);
    Tcl_DStringAppend (&dtName, "~", 1);
    Tcl_DStringAppend(&dtName, classPtr->name, -1);

    /*
     * If there is a destructor method call it. We use the Tclpp_InvokeMethod
     * to resolve any variable linkage. But for that, we need to add an
     * extra parameter (the name of the method) to the list of arguments.
     */

    entryPtr = Tcl_FindHashEntry (&(classPtr->methodTable), 
            Tcl_DStringValue(&dtName));
    if (entryPtr != (Tcl_HashEntry*) NULL) {
        int localObjc;
        Tcl_Obj ** localObjv;

        localObjc = 1;
        localObjv = (Tcl_Obj**) Tcl_Alloc (sizeof(Tcl_Obj*) * localObjc);
        localObjv[0] = Tcl_NewStringObj (Tcl_DStringValue(&dtName), -1);

        (void) Tclpp_InvokeMethod (interp, (Tclpp_Instance*) instancePtr, 
                localObjc, localObjv);
        Tcl_ResetResult(interp);
    } 
    Tcl_DStringFree(&dtName);

    /*
     * Do it recursively for all the base classes.
     */

    for (entryPtr = Tcl_FirstHashEntry(&(classPtr->baseTable), &search);
            entryPtr != (Tcl_HashEntry*) NULL;
            entryPtr = Tcl_NextHashEntry(&search)) {
        TclppClass *baseClassPtr;   /* The base class */

        baseClassPtr = (TclppClass*)Tcl_GetHashValue(entryPtr);
        TclppInvokeClassDestructor(interp, instancePtr, baseClassPtr);
    }
}


/*
 * ----------------------------------------------------------------------------
 *     
 * TclppClassDestructor
 *     Destructor for a class variable. It will invoke the destructor
 *     of all its variables (recursively) and base classes. It will invoke
 *     any user-defined destructor method if present.
 *
 * Results:
 *     Returns TCL_OK on success and TCL_ERROR on error. The absence of
 *     a constructor function is not an error.
 *  
 * Side effects:
 *     None.
 *  
 * ----------------------------------------------------------------------------
 */ 

void
TclppClassDestructor (interp, variablePtr)
    Tcl_Interp *interp;         /* Current interpreter */
    TclppVariable *variablePtr; /* The variable to call the destructor for */
{
    TclppInstance *instancePtr; /* The instance of the class variable */
    TclppClass* classPtr;       /* The class type of the variable */
    Tcl_HashEntry* entryPtr;    /* Hash table entry */
    Tcl_HashSearch search;      /* Keep track of hash table search */

    /*
     * Invoke the destructor of this class and all its base classes.
     */

    instancePtr = variablePtr->value.instancePtr;
    classPtr = (TclppClass*) instancePtr->classPtr;
    TclppInvokeClassDestructor(interp, instancePtr, classPtr);

    /*
     * For each variable of this class instance, invoke the destructor of 
     * these variable.
     */

    for (entryPtr = Tcl_FirstHashEntry(&(instancePtr->variableTable), &search);
            entryPtr != NULL; entryPtr = Tcl_NextHashEntry(&search)) {
        TclppVariable* variablePtr;
        variablePtr = (TclppVariable*)Tcl_GetHashValue(entryPtr);
        (*variablePtr->dtProc)(interp, variablePtr);
    }

    return;
}

/*
 * vi: set ai expandtab ts=4: 
 */

