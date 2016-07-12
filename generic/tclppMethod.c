/*
 * tclppMethod.c --
 *
 *     This file implements functionality to use class methods. This
 *     includes the registration of methods, handle method adornments
 *     like 'virtual' and 'static', specification of default class
 *     methods and invocation of methods.
 *
 * RCS: $Id: tclppMethod.c,v 1.2 2000/05/23 19:34:13 stefan Exp $
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

static char rcsid[] = "$Id: tclppMethod.c,v 1.2 2000/05/23 19:34:13 stefan Exp $";


/*
 * ----------------------------------------------------------------------------
 *     
 * TclppVirtualCmd --
 * 
 *     Used to annotate a procedure as virtual.
 *
 * Results:
 *     TBA.
 *  
 * Side effects:
 *     TBA.
 *  
 * ----------------------------------------------------------------------------
 */ 

int
TclppVirtualCmd (clientData, interp, objc, objv)
    ClientData clientData;      /* Not used */
    Tcl_Interp *interp;         /* The interpreter to create the command in */
    int objc;                   /* Number of objects */
    Tcl_Obj * CONST objv[];     /* Array of objects */
{
    /*
     * Check the number of objects. 
     */

    if (objc != 5) {
        Tcl_WrongNumArgs(interp, 1, objv, "proc name args body");
        return TCL_ERROR;
    }

    /*
     * Check if the second string matches 'proc'
     */

    if (strcmp(Tcl_GetStringFromObj(objv[1], (int*)NULL), "proc") != 0) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), 
                "'virtual' keyword not followed by 'proc'", (char *)NULL);
        return TCL_ERROR;
    }

    /*
     * Set the adornment
     */

    TclppSetAdornmentVirtual();

    /*
     * Continue with the 'proc' command.
     */

    return TclppProcCmd(clientData, interp, objc-1, objv+1);
}


/*
 * ----------------------------------------------------------------------------
 *     
 * TclppProcCmd --
 * 
 *     Command to use instead of the original Tcl procedure command such
 *     that we can do registration of this class method.
 *
 * Results:
 *     See 'Tclpp_CreateMethod'.
 *  
 * Side effects:
 *     See 'Tclpp_CreateMethod'.
 *  
 * ----------------------------------------------------------------------------
 */ 

int
TclppProcCmd (clientData, interp, objc, objv)
    ClientData clientData;      /* Not used */
    Tcl_Interp *interp;         /* The interpreter to create the command in */
    int objc;                   /* Number of objects */
    Tcl_Obj * CONST objv[];     /* Array of objects */
{
    Tclpp_Class *classPtr;      /* The current class */
    Tclpp_Method *methodPtr;    /* The created method */

    /*
     * Check the number of objects
     */

    if (objc != 4) {
        Tcl_WrongNumArgs(interp, 1, objv, "name args body");
        return TCL_ERROR;
    }

    /*
     * Get the current class definition.
     */

    classPtr = Tclpp_GetCurrentClass (interp);
    if (classPtr == (Tclpp_Class*) NULL) {
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), 
                "Tclpp internal error: not in class definition context", 
                (char *)NULL);
        return TCL_ERROR;
    }

    /* 
     * Create the class method
     */

    methodPtr = Tclpp_CreateMethod (interp, classPtr, 
            Tcl_GetStringFromObj(objv[1], (int*)NULL), objv[2], objv[3]);
    return ((methodPtr == (Tclpp_Method*) NULL) ? TCL_ERROR : TCL_OK);
}


/*
 * ----------------------------------------------------------------------------
 *
 * TclppLookupMethod --
 *
 *     Lookup a method for a certain instance when a name is known. The
 *     method must be specified in the class or one of its base classes
 *     of the classPtr specified. In case of a virtual function, the search
 *     will be started at the class of the instance instead.
 *     The 'instancePtr' may be NULL to do a search on class level only. In
 *     that case the repeated search in case of a virtual method will be
 *     skipped.
 *
 * Results:
 *     A pointer to the appropriate method will be returned. The function
 *     return TCL_OK on success and TCL_ERROR otherwise.
 *
 * Side Effects:
 *     None.
 *
 * ----------------------------------------------------------------------------
 */ 

static int
TclppLookupMethodFromClass (interp, instancePtr, name, classPtr, methodPtrPtr)
    Tcl_Interp *interp;             /* The current interpreter */
    TclppInstance *instancePtr;     /* The current instance. */
    char *name;                     /* Name of the method to look for. */
    TclppClass *classPtr;           /* The class to start the search for. */
    TclppMethod **methodPtrPtr;     /* The method found. */
{
    Tcl_HashEntry *entryPtr;        /* The hash table entry */
    Tcl_HashSearch search;          /* Keep track of hash table search */

    *methodPtrPtr = (TclppMethod*) NULL;

    /*
     * Search the class and all its base classes for the first method with
     * the same name.
     */

    entryPtr = Tcl_FindHashEntry(&(classPtr->methodTable), name);
    if (entryPtr != (Tcl_HashEntry*) NULL) {
        *methodPtrPtr = (TclppMethod*) Tcl_GetHashValue(entryPtr);
    } else {
        entryPtr = Tcl_FirstHashEntry(&(classPtr->baseTable), &search);       
        while ((entryPtr != (Tcl_HashEntry*) NULL)
                && (*methodPtrPtr == (TclppMethod*) NULL)) {
            TclppClass *baseClassPtr;   /* The base class */
            baseClassPtr = (TclppClass*) Tcl_GetHashValue(entryPtr);
            (void) TclppLookupMethodFromClass(interp, instancePtr, name, 
                    baseClassPtr, methodPtrPtr);
            if (*methodPtrPtr == (TclppMethod*) NULL) {
                entryPtr = Tcl_NextHashEntry(&search);
            }
        }
    }

    if (*methodPtrPtr == (TclppMethod*) NULL) {
        return TCL_ERROR;
    }

    return TCL_OK;
}

int
TclppLookupMethod (interp, instancePtr, name, classPtr, methodPtrPtr)
    Tcl_Interp *interp;             /* The current interpreter */
    TclppInstance *instancePtr;     /* The current instance. */
    char *name;                     /* Name of the method to look for. */
    TclppClass *classPtr;           /* The class to start the search for. If
                                     * the method found is a virtual one, the
                                     * search will restart at the class of
                                     * the instance instead. */
    TclppMethod **methodPtrPtr;     /* The method found. */
{
    *methodPtrPtr = (TclppMethod*) NULL;

    /*
     * Search the method from the specified class upwards.
     */

    if (TclppLookupMethodFromClass(interp, instancePtr, name, classPtr, 
            methodPtrPtr) == TCL_ERROR) {
        Tcl_ResetResult(interp);
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "method \"", name, 
                "\" not a member of class \"", classPtr->fullName, "\"",
                (char*) NULL);
        return TCL_ERROR;
    }

    /*
     * If the method found is a virtual one, repeat the search, but with the
     * actual instance class pointer to ensure that the latest overridden
     * function will be invoked.
     */

    if (TclppMethodIsVirtual(*methodPtrPtr) 
            && (instancePtr != (TclppInstance*) NULL)) {
        if (TclppLookupMethodFromClass(interp, instancePtr, name, (TclppClass*)
                instancePtr->classPtr, methodPtrPtr) == TCL_ERROR) {
            Tcl_ResetResult(interp);
            Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "method \"", name,
                    "\" not a member of class \"", 
                    instancePtr->classPtr->fullName, "\"", (char*) NULL);
            return TCL_ERROR;
        }
    }
    
    return TCL_OK;
}


/*
 * ----------------------------------------------------------------------------
 *
 * TclppMethodStub --
 *
 *     Procedure invoked just before any class method is invoked. It will
 *     keep track of the current class context by keeping track of a class
 *     context stack and runs in parallel with the procedure call frame. 
 *
 * Results:
 *     The result of the execution of the procedure.
 *
 * Side Effects:
 *     Depends of the commands in the actual procedure.
 *
 * ----------------------------------------------------------------------------
 */ 

int
TclppMethodStub (clientData, interp, objc, objv)
    ClientData clientData;      /* Pointer to the procedure TclppMethod info */
    Tcl_Interp *interp;         /* Current interpreter */
    int objc;                   /* Number of arguments to the actual command */
    Tcl_Obj * CONST objv[];     /* Arguments to the actual command */
{
    TclppMethod *methodPtr;     /* The method being invoked */
    TclppContext *contextPtr;   /* The current context */
    TclppInstance *instancePtr; /* The current instance */
    int result;                 /* Result of the actual command */

    methodPtr = (TclppMethod*) clientData;

    /*
     * Get the current instance. The instance to use is at the top of the
     * context stack. This is either from any previous procedure on the
     * context stack or from a new invocation.
     */

    contextPtr = TclppGetCurrentContext();
    if (contextPtr == (TclppContext*) NULL) {
        Tcl_ResetResult(interp);
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), 
            "no context stack", (char*) NULL);
        return TCL_ERROR;
    }
    instancePtr = contextPtr->instancePtr;

    /*
     * Construct and push a new context stack item.
     */

    contextPtr = (TclppContext*) Tcl_Alloc(sizeof(TclppContext));
    contextPtr->classPtr = methodPtr->classPtr;
    contextPtr->instancePtr = instancePtr;
    TclppPushContext (contextPtr);

    /*
     * Execute the actual procedure and save its result. 
     */

    result = TclObjInterpProc((ClientData)methodPtr->procPtr, interp, objc, 
            objv);

    /*
     * Pop the context stack item and free its memory.
     */

    if (contextPtr != TclppPopContext ()) {
        panic ("context call stack no longer consistent");
    }
    Tcl_Free ((char*) contextPtr);

    return result;
}


/*
 * ----------------------------------------------------------------------------
 *     
 * Tclpp_CreateMethod --
 * 
 *     Create the method in the current class definition. The procedure is
 *     created by the Tcl creation procedure and registered as a class
 *     method. The basics of this procedure is derived from Tcl_ProcObjCmd.
 *
 * Results:
 *     The procedure is created in the current class namespace and 
 *     registered as a class method. It returns the created method or
 *     NULL on any error. On error, the reason is appended to the 
 *     interpreter.
 *  
 * Side effects:
 *     The class method should never be deleted.
 *  
 * ----------------------------------------------------------------------------
 */ 

Tclpp_Method*
Tclpp_CreateMethod (interp, classPtr, name, args, body)
    Tcl_Interp *interp;         /* The interpreter to create the command in */
    Tclpp_Class *classPtr;      /* The class for this method */
    char *name;                 /* The name of the procedure */
    Tcl_Obj *args;              /* The procedure argumens */ 
    Tcl_Obj *body;              /* The procedure body */
{
    Tcl_DString fullName;       /* The fully qualified procedure name. */
    char *argsStr;              /* The arguments as a string */
    int argsLen;                /* The length of the argument string */
    Tcl_Command cmd;            /* The Tcl command procedure */
    Proc *procPtr;              /* The Tcl procedure */
    Tcl_HashEntry *entryPtr;    /* Hash table entry for new method */
    int isNew;                  /* Set if the method exists in methodTable */
    TclppMethod *methodPtr;     /* The new class method structure */

    /*
     * Get the fully qualified name of the procedure. This is the namespace
     * of the class appended with the function name.
     */

    Tcl_DStringInit(&fullName);
    Tcl_DStringAppend(&fullName, classPtr->fullName, -1);
    Tcl_DStringAppend(&fullName, "::", 2);
    Tcl_DStringAppend(&fullName, name, -1);

    /*
     * If the name is a destructor, check that it does not have any
     * arguments
     */

    argsStr = Tcl_GetStringFromObj (args, &argsLen);
    if ((*name == '~') && (argsLen != 0)) {
        Tcl_ResetResult(interp);
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "destructor \"",
                Tcl_DStringValue(&fullName), "\" does not allow arguments",
                (char *) NULL);
        return (Tclpp_Method*) NULL;
    }

    /* 
     * Check if procedure already exists in method table.
     */

    entryPtr = Tcl_CreateHashEntry (&(classPtr->methodTable), name, &isNew);
    if (!isNew) {
        Tcl_ResetResult(interp);
        Tcl_AppendStringsToObj(Tcl_GetObjResult(interp), "class method \"",
                Tcl_DStringValue(&fullName), "\" already exists", 
                (char *) NULL);
        return (Tclpp_Method*) NULL;
    }

    /*
     * Create the data structure to represent the procedure.
     */

    if (TclCreateProc(interp, ((TclppClass*)classPtr)->nsPtr, 
            Tcl_DStringValue(&fullName), args, body, &procPtr) 
            != TCL_OK) {
        return (Tclpp_Method*) NULL;
    }

    /*
     * Create an entry in the methodTable. The hash-key is the fully
     * qualified procedure name.
     */

    methodPtr = (TclppMethod*) Tcl_Alloc(sizeof(TclppMethod));
    methodPtr->name = (char *) Tcl_Alloc(strlen(name)+1);
    strcpy(methodPtr->name, name);
    methodPtr->flags = TclppGetAdornment();
    methodPtr->classPtr = (TclppClass*) classPtr;
    methodPtr->procPtr = procPtr;
    Tcl_SetHashValue(entryPtr, (ClientData)methodPtr);

    /*
     * Create the command for this procedure.
     */

    cmd = Tcl_CreateObjCommand(interp, Tcl_DStringValue(&fullName), 
            (Tcl_ObjCmdProc*) &TclppMethodStub, (ClientData) methodPtr, 
            (Tcl_CmdDeleteProc*) NULL);
    procPtr->cmdPtr = (Command *) cmd;

    TclppResetMethodAdornment();

    return (Tclpp_Method*) methodPtr;
}


/*
 * ----------------------------------------------------------------------------
 *
 * Tclpp_InvokeMethod --
 *
 *     Used to invoke a class method from an instance
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
Tclpp_InvokeMethod (interp, instancePtr, objc, objv)
    Tcl_Interp *interp;          /* The interpreter to run the method in */
    Tclpp_Instance *instancePtr; /* The instance */
    int objc;                    /* Number of objects */
    Tcl_Obj * CONST objv[];      /* Array of objects:
                                  *      objv[0] = method
                                  *      objv[n] = arguments */
{
    char *procName;              /* The class method procedure name */
    TclppClass *classPtr;        /* The instance associated class */
    TclppMethod *methodPtr;      /* The method to invoke */
    Command *cmdPtr;             /* The actual Tcl command to invoke */
    int result;                  /* Result of the executed command */
    TclppContext *contextPtr;    /* The current context */

    /*
     * Get the class method to invoke. The procedure name (objv[0]) will
     * be used to lookup the Tclpp_Method from the class' method table.
     */
    
    procName = Tcl_GetStringFromObj(objv[0], (int *)NULL);
    classPtr = (TclppClass*) instancePtr->classPtr;
    if (TclppLookupMethod(interp, (TclppInstance*)instancePtr, procName,
            classPtr, &methodPtr) == TCL_ERROR) {
        return TCL_ERROR;
    }

    /*
     * For now, always recompile the body to ensure that the internal
     * variables are linked to the correct instance variables and that
     * the variables representing a class (eg. 'variable A a') are
     * linked to the correct instance as well. At a later stage we might
     * enhance it by compiling only when the context (ie. the instance)
     * has changed. For that we can use a 'last-instance' field in the
     * tclppMethod.
     * Recompilation is enforced by incrementing the namespace resolver
     * epoch to signal that name resolution scheme has changed. This
     * happens to be the case, since we are evaluation with a different
     * instance namespace, so names are resolved differently.
     */

    methodPtr->procPtr->cmdPtr->nsPtr->resolverEpoch++;

    /*
     * Construct and push a new context stack item. Although it is done
     * by the procedure stub as well, it is essential to do it here as
     * well to get the instance pointer available to the stub in a nice
     * and consistent way.
     */

    contextPtr = (TclppContext*) Tcl_Alloc(sizeof(TclppContext));
    contextPtr->classPtr = methodPtr->classPtr;
    contextPtr->instancePtr = (TclppInstance*) instancePtr;
    TclppPushContext (contextPtr);

    /*
     * Execute the procedure.
     */

    cmdPtr = methodPtr->procPtr->cmdPtr;
    result = (*cmdPtr->objProc)(cmdPtr->objClientData, interp, objc, objv);

    /*
     * Pop the context stack item and free its memory.
     */

    if (contextPtr != TclppPopContext ()) {
        panic ("context call stack no longer consistent");
    }
    Tcl_Free ((char*) contextPtr);

    return result;
}


/*
 * ----------------------------------------------------------------------------
 *
 * TclppResolveClassCmd --
 *
 *     Resolve a command called from within a byte code execution of another
 *     class method. First try to get the command as a class of base-class
 *     method. If that fails, try to get the command as a class variable
 *     invocation. If that fails as well, fall back to the standard Tcl
 *     command procedure.
 *
 * Result:
 *     Returns the Tcl_Command if the command was a class method or a class
 *     variable invocation. Otherwise let Tcl continue with the standard
 *     command resolution method.
 *
 * Side Effects:
 *     None.
 *
 * ----------------------------------------------------------------------------
 */

int
TclppResolveClassCmd (interp, name, nsPtr, flags, rPtr)
    Tcl_Interp* interp;             /* Current Interpreter */
    char* name;                     /* Command name */
    Tcl_Namespace* nsPtr;           /* Class Namespace */
    int flags;                      /* Status flags */
    Tcl_Command* rPtr;              /* Returns resolved command */
{
    Tcl_DString varName;            /* Fully qualified variable name */
    TclppContext* contextPtr;       /* Current context */
    TclppInstance* instancePtr;     /* Current instance */
    TclppClass* classPtr;           /* Current class */
    TclppMethod* methodPtr;         /* The method */
    TclppVariable *variablePtr;     /* The class/instance variable pointer */
    Tcl_HashEntry* entryPtr;        /* Hash table entry */

    /*
     * Get the current instance pointer. If one cannot be found, resume with
     * the normal Tcl command resolution scheme. It might be a static
     * function.
     */

    contextPtr = TclppGetCurrentContext();
    if (contextPtr == (TclppContext*) NULL) {
        return TCL_CONTINUE;
    }
    instancePtr = contextPtr->instancePtr;
    classPtr = contextPtr->classPtr;

    /*
     * Check if we can find a method with the matching name first. If one
     * can be located, return it.
     */

    if (TclppLookupMethod(interp, instancePtr, name, classPtr, &methodPtr)
            == TCL_OK) {
        *rPtr = (Tcl_Command) methodPtr->procPtr->cmdPtr;
        return TCL_OK;
    }

    /*
     * Check if there is a class variable with the same name. If there is
     * one, we get the class variable access procedure from that class
     * instance and return that as the resolved command.
     */

    entryPtr = Tcl_FindHashEntry(&(classPtr->variableTable), name);
    if (entryPtr == (Tcl_HashEntry*) NULL) {
        return TCL_CONTINUE;
    }
    variablePtr = (TclppVariable*) Tcl_GetHashValue(entryPtr);

    Tcl_DStringInit(&varName);
    TclppGetQualVariableName(instancePtr, variablePtr, &varName);
    entryPtr = Tcl_FindHashEntry(&(instancePtr->variableTable), 
            Tcl_DStringValue(&varName));
    if (entryPtr != (Tcl_HashEntry*) NULL) {
        variablePtr = (TclppVariable*) Tcl_GetHashValue(entryPtr);
        if (TclppVariableIsClass(variablePtr)) {
            *rPtr = (Tcl_Command) variablePtr->value.instancePtr->cmdPtr;
            return TCL_OK;
        }
    }

    /*
     * Command is not a class-variable command, resolve it using the
     * normal Tcl resolving scheme.
     */

    return TCL_CONTINUE;
}


/*
 * ----------------------------------------------------------------------------
 *
 * TclppGetCurrentContext --
 *
 *     Returns the context on the top of the stack (the current context).
 *
 * Results:
 *     Returns the top of the context stack or NULL if stack is empty.
 *
 * Side effects:
 *     None.
 *
 * ----------------------------------------------------------------------------
 */

static TclppContext* contextCallStack = (TclppContext*) NULL;

TclppContext*
TclppGetCurrentContext (void)
{
    return contextCallStack;
}


/*
 * ----------------------------------------------------------------------------
 *
 * TclppPushContext --
 *
 *     Pushes a new context on the context call stack. This should ONLY occur
 *     when a new class method is invoked !
 *
 * Results:
 *     The new context is pushed on top of the context call stack.
 *
 * Side effects:
 *     None.
 *
 * ----------------------------------------------------------------------------
 */

void
TclppPushContext (contextPtr)
    TclppContext *contextPtr;       /* The context to push onto the stack */
{
    contextPtr->prevPtr = contextCallStack;
    contextCallStack = contextPtr;
}


/*
 * ----------------------------------------------------------------------------
 *
 * TclppPopContext --
 *
 *     Pops the top of the call context stack and returns the popped item.
 *
 * Results:
 *     The top of the context stack is popped.
 *
 * Side effects:
 *     None.
 *
 * ----------------------------------------------------------------------------
 */

TclppContext*
TclppPopContext ()
{
    if (contextCallStack != (TclppContext*) NULL) {
        TclppContext *contextPtr = contextCallStack;
        contextCallStack = contextPtr->prevPtr;
        return contextPtr;
    }
    return (TclppContext*) NULL;
}


/*
 * vi: set ai expandtab ts=4: 
 */

