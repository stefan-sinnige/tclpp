/*
 * tclppInt.h --
 *
 *      This header file describes the internally-visible facilities
 *      of the 'Tcl Propellant' extension.
 *
 * RCS: $Id: tclppInt.h,v 1.2 2000/05/23 19:34:13 stefan Exp $
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

#ifndef _TCLPPINT
#define _TCLPPINT

#include <tcl.h>
#include <tclInt.h>
#include "tclpp.h"

/*
 * Tclpp internal structures
 */

extern Tcl_HashTable classTable;
extern Tcl_HashTable instanceTable;

/*
 * Data structure to maintain class definition information. This contains
 * both the public and private section. The public section is part of the
 * API and is known as Tclpp_Class. The first entries of the structure 
 * defined here must match with this public Tclpp_Class structure.
 */

typedef struct TclppClass {
    /* ---- PUBLIC ----- */
    char* name;                  /* The unqualified name of the class,
                                  * containing no ::'s */
    char* fullName;              /* The fully qualified name of the class,
                                  * starting with a :: */
    Tcl_HashTable baseTable;     /* The list of base classes. Each entry
                                  * is a TclppClass */
    Tcl_HashTable methodTable;   /* The list of methods for this class. Each
                                  * entry is a TclppMethod */
    Tcl_HashTable variableTable; /* The list of variables for this class. Each
                                  * entry is a TclppVariable */
    /* ---- PRIVATE ---- */
    Namespace* nsPtr;            /* The associated namespace */
} TclppClass;

/*
 * Data structure to maintain class instance information. This contains
 * both the public and private section. The public section is part of the
 * API and is known as Tclpp_Instance. The first entries of the structure 
 * defined here must match with this public Tclpp_Instance structure.
 */

typedef struct TclppInstance {
    /* ---- PUBLIC ----- */
    char          *name;         /* The unqualifed name of the instance, 
                                  * containing no ::'s */
    char          *fullName;     /* The fully qualifed name of the instance,
                                  * starting with :: */
    Tclpp_Class *classPtr;       /* The associated class of this instance */
    Tcl_HashTable variableTable; /* The list of variables for this instance.
                                  * Each entry is a TclppVariable */
    /* ---- PRIVATE ---- */
    Namespace *nsPtr;            /* The associated namespace */
    Command* cmdPtr;             /* The instance access command */
} TclppInstance;

/*
 * Data structure to maintain class method information. This contains
 * both the public and private section. The public section is part of the
 * API and is known as Tclpp_Method. The first entries of the structure 
 * defined here must match with this public Tclpp_Method structure.
 */

typedef struct TclppMethod {
    /* ---- PUBLIC ----- */
    char* name;                  /* The unqualified name of the class method,
                                  * containing no ::'s */
    int flags;                   /* Method attributes bitmask:
                                  *      0x01: TCLPP_METHOD_VIRTUAL
                                  *      0x02: TCLPP_METHOD_STATIC 
                                  *      0x10: TCLPP_METHOD_PUBLIC
                                  *      0x20: TCLPP_METHOD_PROTECTED
                                  *      0x40: TCLPP_METHOD_PRIVATE */
    /* ---- PRIVATE ---- */
    TclppClass* classPtr;        /* The class containing this method */
    Proc* procPtr;               /* The command procedure */
} TclppMethod;

#define TclppMethodSetVirtual(methPtr) \
    ((methPtr)->flags) &= 0xF0; \
    ((methPtr)->flags) != TCLPP_METHOD_VIRTUAL
#define TclppMethodSetStatic(methPtr) \
    ((methPtr)->flags) &= 0xF0; \
    ((methPtr)->flags) != TCLPP_METHOD_STATIC
#define TclppMethodSetPublic(methPtr) \
    ((methPtr)->flags) &= 0x0F; \
    ((methPtr)->flags) != TCLPP_METHOD_PUBLIC
#define TclppMethodSetProtected(methPtr) \
    ((methPtr)->flags) &= 0x0F; \
    ((methPtr)->flags) != TCLPP_METHOD_PROTECTED
#define TclppMethodSetPrivate(methPtr) \
    ((methPtr)->flags) &= 0x0F; \
    ((methPtr)->flags) != TCLPP_METHOD_PRIVATE

/*
 * Data structure to maintain class variable information. This contains
 * both the public and private section. The public section is part of the
 * API and is known as Tclpp_Variable. The first entries of the structure 
 * defined here must match with this public Tclpp_Variable structure.
 */

struct TclppVariable;

typedef int (TclppConstructorProc) _ANSI_ARGS_ ((Tcl_Interp *interp,
        struct TclppVariable *variablePtr, int objc, Tcl_Obj * CONST objv[]));
typedef void (TclppDestructorProc) _ANSI_ARGS_ ((Tcl_Interp *interp,
        struct TclppVariable *variablePtr));

typedef struct TclppVariable {
    /* ---- PUBLIC ----- */
    char* name;                      /* The unqualified name of the variable,
                                      * containing no ::'s */
    char* fullName;                  /* The fully qualifed name of the variable,
                                      * starting with ::'s */
    int type;                        /* Type of the variable:
                                      *      0x01: TCLPP_VARIABLE_SCALAR
                                      *      0x02: TCLPP_VARIABLE_ARRAY
                                      *      0x03: TCLPP_VARIABLE_CLASS */
    char* typeName;                  /* Name of the type, ie. 'scalar',
                                      * 'array' or a class name */
    int flags;                       /* Method attributes bitmask:
                                      *      0x01: TCLPP_VARIABLE_STATIC */
    /* ---- PRIVATE ---- */
    TclppClass* classPtr;            /* The class this variable belongs to.
                                      * This makes name resolving in cases
                                      * of inheritance much easier. */
    TclppInstance* instancePtr;      /* The instance this variable belongs 
                                      * to. */
    union {
        Var*    varPtr;              /* The actual variable. This is NULL for
                                      * all the variables defined in the class
                                      * hash table, and not NULL for those 
                                      * defined in the instance hash table. 
                                      * NOTE: Only valid if the variable is 
                                      * scalar or array. */
        TclppInstance *instancePtr;  /* The instance if the class variable is
                                      * defined in the instance hash table. It
                                      * is NULL if defined in the class hash
                                      * table. 
                                      * NOTE: Only valid if the variable is a
                                      * class variable.
                                      */
    } value;
    TclppConstructorProc *ctProc;    /* Constructor function. This is called
                                      * each time a new instance variable
                                      * is created. */
    TclppDestructorProc *dtProc;     /* Destructor function. This is called
                                      * each time an instance variable is
                                      * deleted. */

} TclppVariable;

#define TclppVariableSetScalar(varPtr) \
    ((varPtr)->type) = TCLPP_VARIABLE_SCALAR
#define TclppVariableSetArray(varPtr) \
    ((varPtr)->type) = TCLPP_VARIABLE_ARRAY
#define TclppVariableSetClass(varPtr) \
    ((varPtr)->type) = TCLPP_VARIABLE_CLASS

#define TclppVariableSetStatic(varPtr) \
    ((varPtr)->flags) |= TCLPP_VARIABLE_STATIC
#define TclppVariableSetPublic(varPtr) \
    ((varPtr)->flags) &= 0x0F; \
    ((varPtr)->flags) |= TCLPP_VARIABLE_PUBLIC
#define TclppVariableSetProtected(varPtr) \
    ((varPtr)->flags) &= 0x0F; \
    ((varPtr)->flags) |= TCLPP_VARIABLE_PROTECTED
#define TclppVariableSetPrivate(varPtr) \
    ((varPtr)->flags) &= 0x0F; \
    ((varPtr)->flags) |= TCLPP_VARIABLE_PRIVATE

/*
 * The following structure maintains the current adornment for a method
 * or variable while parsing the class definition. There is only one
 * such an adornment in the application.
 */

typedef struct TclppAdornment {
    int flags;                      /* The adornment flags:
                                     *      0x01: TCLPP_ADORNMENT_VIRTUAL
                                     *      0x02: TCLPP_ADORNMENT_STATIC
                                     *      0x10: TCLPP_ADORNMENT_PUBLIC
                                     *      0x20: TCLPP_ADORNMENT_PROTECTED
                                     *      0x40: TCLPP_ADORNMENT_PRIVATE 
                                     * These can be controlled by the 
                                     * functions below */
} TclppAdornment;

extern TclppAdornment tclppAdornment;

#define TCLPP_ADORNMENT_VIRTUAL     0x01
#define TCLPP_ADORNMENT_STATIC      0x02
#define TCLPP_ADORNMENT_PUBLIC      0x10
#define TCLPP_ADORNMENT_PROTECTED   0x20
#define TCLPP_ADORNMENT_PRIVATE     0x40

#define TclppResetVariableAdornment() \
    tclppAdornment.flags &= 0xF0 
#define TclppResetMethodAdornment() \
    tclppAdornment.flags &= 0xF0 
#define TclppResetProtectionAdornment() \
    tclppAdornment.flags &= 0x0F 
#define TclppResetAdornment() \
    tclppAdornment.flags = TCLPP_ADORNMENT_PUBLIC
#define TclppSetAdornmentStatic() \
    tclppAdornment.flags &= 0xF0; \
    tclppAdornment.flags |= TCLPP_ADORNMENT_STATIC
#define TclppSetAdornmentVirtual() \
    tclppAdornment.flags &= 0xF0; \
    tclppAdornment.flags |= TCLPP_ADORNMENT_VIRTUAL
#define TclppSetAdornmentPublic() \
    tclppAdornment.flags &= 0x0F; \
    tclppAdornment.flags = TCLPP_ADORNMENT_PUBLIC
#define TclppSetAdornmentProtected() \
    tclppAdornment.flags &= 0x0F; \
    tclppAdornment.flags = TCLPP_ADORNMENT_PROTECTED
#define TclppSetAdornmentPrivate() \
    tclppAdornment.flags &= 0x0F; \
    tclppAdornment.flags = TCLPP_ADORNMENT_PRIVATE
#define TclppGetAdornment() \
    tclppAdornment.flags

/*
 * The following structure is the item for the context stack, which runs
 * in parallel with the procedure call frame to keep track of the current
 * class and instance context.
 */

typedef struct TclppContext {
    TclppClass *classPtr;           /* The current class pointer to execute
                                     * the procedure in. This is the same as
                                     * the class of the procedure and is al-
                                     * ways the class or one of its base
                                     * classes of the instancePtr. */
    TclppInstance *instancePtr;     /* Current instance defining the data 
                                     * mapping for any class method. */
    struct TclppContext *prevPtr;   /* Previous context on the stack. */
} TclppContext;

/*
 * The following structure is an extension of the Tcl_ResolvedVarInfo which
 * is used to store additional information about a compiled variable.
 */

typedef struct TclppResolvedVarInfo {
    /* ---- Tcl_ResolvedVarInfo ---- */
    Tcl_ResolveRuntimeVarProc *fetchProc;
    Tcl_ResolveVarDeleteProc *deleteProc;
    /* ---- PRIVATE ---- */
    TclppVariable *variablePtr;     /* The instance variable to link to */
} TclppResolvedVarInfo;

/*
 * Tclpp internal functions
 */

/*
 * Tcl object commands 
 */

int TclppClassCmd _ANSI_ARGS_ ((ClientData clientData, Tcl_Interp* interp, 
                           int objc, Tcl_Obj* CONST objv[]));
int TclppClassInfoCmd _ANSI_ARGS_ ((ClientData clientData, Tcl_Interp* interp, 
                           int objc, Tcl_Obj* CONST objv[]));
int TclppInstanceCmd _ANSI_ARGS_ ((ClientData clientData, Tcl_Interp* interp, 
                           int objc, Tcl_Obj* CONST objv[]));
int TclppNewCmd _ANSI_ARGS_ ((ClientData clientData, Tcl_Interp* interp, 
                           int objc, Tcl_Obj* CONST objv[]));
int TclppDeleteCmd _ANSI_ARGS_ ((ClientData clientData, Tcl_Interp* interp, 
                           int objc, Tcl_Obj* CONST objv[]));
int TclppObjectInfoCmd _ANSI_ARGS_ ((ClientData clientData, Tcl_Interp* interp, 
                           int objc, Tcl_Obj* CONST objv[]));
int TclppProcCmd _ANSI_ARGS_ ((ClientData clientData, Tcl_Interp* interp, 
                           int objc, Tcl_Obj* CONST objv[]));
int TclppVariableCmd _ANSI_ARGS_ ((ClientData clientData, Tcl_Interp* interp, 
                           int objc, Tcl_Obj* CONST objv[]));
int TclppVirtualCmd _ANSI_ARGS_ ((ClientData clientData, Tcl_Interp* interp, 
                           int objc, Tcl_Obj* CONST objv[]));

/*
 * Namespace variable and command resolvers
 */

int TclppResolveClassVar _ANSI_ARGS_ ((Tcl_Interp* interp, char* name,
                           Tcl_Namespace *context, int flags, Tcl_Var *rPtr));
int TclppResolveClassCmd _ANSI_ARGS_ ((Tcl_Interp* interp, char* name,
                           Tcl_Namespace *context, int flags, 
                           Tcl_Command* rPtr));
int TclppResolveClassCompiledVar _ANSI_ARGS_ ((Tcl_Interp* interp, char* name,
                           int length, Tcl_Namespace *context, 
                           Tcl_ResolvedVarInfo **rPtrPtr));
Tcl_Var TclppResolveRuntimeClassVar _ANSI_ARGS_ ((Tcl_Interp* interp, 
                           Tcl_ResolvedVarInfo *rPtr));

/*
 * The variable constructors and destructors.
 */

int TclppScalarConstructor _ANSI_ARGS_ ((Tcl_Interp* interp, 
                           TclppVariable* variablePtr, int objc, 
                           Tcl_Obj * CONST objv[]));
int TclppArrayConstructor _ANSI_ARGS_ ((Tcl_Interp* interp, 
                           TclppVariable* variablePtr, int objc, 
                           Tcl_Obj * CONST objv[]));
int TclppClassConstructor _ANSI_ARGS_ ((Tcl_Interp* interp, 
                           TclppVariable* variablePtr, int objc, 
                           Tcl_Obj * CONST objv[]));

void TclppScalarDestructor _ANSI_ARGS_ ((Tcl_Interp* interp, 
                           TclppVariable* variablePtr));
void TclppArrayDestructor _ANSI_ARGS_ ((Tcl_Interp* interp, 
                           TclppVariable* variablePtr));
void TclppClassDestructor _ANSI_ARGS_ ((Tcl_Interp* interp, 
                           TclppVariable* variablePtr));

/*
 * Context Call Stack
 */

TclppContext* TclppGetCurrentContext _ANSI_ARGS_ ((void));
void TclppPushContext _ANSI_ARGS_ ((TclppContext *contextPtr));
TclppContext* TclppPopContext _ANSI_ARGS_ ((void));

/*
 * Miscellaneous
 */

void TclppGetQualVariableName _ANSI_ARGS_ ((TclppInstance* instancePtr,
                           TclppVariable *variablePtr, Tcl_DString* name));
int TclppLookupMethod _ANSI_ARGS_ ((Tcl_Interp *interp, 
                           TclppInstance* instancePtr, char *name,
                           TclppClass *classPtr, TclppMethod **methodPtr));

/*
 * vi: set ai expandtab ts=4: 
 */

#endif /* _TCLPPINT */

