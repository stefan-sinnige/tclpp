/*
 * tclpp.h --
 *
 *      This header file describes the externally-visible facilities
 *      of the 'Tcl Propellant' extension.
 *
 * RCSID: $Id: tclpp.h,v 1.3 2000/05/23 21:31:39 stefan Exp $
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

#ifndef _TCLPP
#define _TCLPP

#include <tcl.h>

/*
 * When version numbers change here, they must be changed in the following
 * files as well:
 *
 *      configure.in
 *
 * The release level should be  0 for alpha, 1 for beta, and 2 for
 * final/patch.  The release serial value is the number that follows the
 * "a", "b", or "p" in the patch level; for example, if the patch level
 * is 2.3b2, TCLPP_RELEASE_SERIAL is 2.  It restarts at 1 whenever the
 * release level is changed, except for the final release which is 0
 * (the first patch will start at 1).
 */

#define TCLPP_VERSION       "2.0"
#define TCLPP_PATCH_LEVEL   "2.0.0"
#define TCLPP_MAJOR_VERSION  2
#define TCLPP_MINOR_VERSION  0
#define TCLPP_RELEASE_LEVEL  0
#define TCLPP_RELEASE_SERIAL 0

/*
 *  If a function is declared which should be included in a shared library,
 *  then it should have the DLLEXPORT storage class.
 */

#ifdef BUILD_tclpp
# undef TCL_STORAGE_CLASS
# define TCL_STORAGE_CLASS DLLEXPORT
#endif

/*
 * Data structure to maintain class definition information. This is the
 * public section of the internal TclppClass. The entries must match
 * with the first entries of the TclppClass structure.
 */

typedef struct Tclpp_Class {
    char* name;                  /* The unqualified name of the class. This 
                                  * name does no ::'s */
    char* fullName;              /* The fully qualified name of the class,
                                  * starting with a :: */
    Tcl_HashTable baseTable;     /* The list of base classes for this class. 
                                  * Each entry is a Tclpp_Class */
    Tcl_HashTable methodTable;   /* The list of methods for this class. Each
                                  * entry is a Tclpp_Method */
    Tcl_HashTable variableTable; /* The list of variables for this class, Each
                                  * entry is a Tclpp_Variable */
} Tclpp_Class;

/*
 * Data structure to maintain class method information. This is the
 * public section of the internal TclppMethod. The entries must match
 * with the first entries of the TclppMethod structure.
 */

typedef struct Tclpp_Method {
    char* name;                 /* The unqualified name of the class method,
                                 * containing no ::'s */
    int flags;                  /* Method attributes bitmask:
                                 *      0x01: TCLPP_METHOD_VIRTUAL
                                 *      0x02: TCLPP_METHOD_STATIC
                                 *      0x10: TCLPP_METHOD_PUBLIC
                                 *      0x20: TCLPP_METHOD_PROTECTED
                                 *      0x40: TCLPP_METHOD_PRIVATE
                                 * These flags can be checked through the 
                                 * macros defined below. */ 
} Tclpp_Method;

#define TCLPP_METHOD_VIRTUAL    0x01
#define TCLPP_METHOD_STATIC     0x02
#define TCLPP_METHOD_PUBLIC     0x10
#define TCLPP_METHOD_PROTECTED  0x20
#define TCLPP_METHOD_PRIVATE    0x40

#define TclppMethodIsVirtual(methPtr) \
    (((methPtr)->flags) & TCLPP_METHOD_VIRTUAL)
#define TclppMethodIsStatic(methPtr) \
    (((methPtr)->flags) & TCLPP_METHOD_STATIC)

/*
 * Data structure to maintain class variable information. This is the
 * public section of the internal TclppVariable. The entries must match
 * with the first entries of the TclppVariable structure.
 */

typedef struct Tclpp_Variable {
    char* name;                 /* The unqualified name of the class variable,
                                 * containing no ::'s */
    char* fullName;             /* The fully qualifed name of the variable,
                                 * starting with ::'s */
    int type;                   /* Type of the variable:
                                 *      0x01: TCLPP_VARIABLE_SCALAR
                                 *      0x02: TCLPP_VARIABLE_ARRAY
                                 *      0x03: TCLPP_VARIABLE_CLASS
                                 * These flags can be checked through the
                                 * macros defined below. */
    char* typeName;             /* The name of the type, ie. 'scalar', 'array'
                                 * or the name of a class. */
    int flags;                  /* Variable attributes bitmask:
                                 *      0x01: TCLPP_VARIABLE_STATIC
                                 *      0x10: TCLPP_VARIABLE_PUBLIC
                                 *      0x20: TCLPP_VARIABLE_PROTECTED
                                 *      0x40: TCLPP_VARIABLE_PRIVATE
                                 * These flags can be checked through the 
                                 * macros defined below. */ 
} Tclpp_Variable;

#define TCLPP_VARIABLE_SCALAR       0x01
#define TCLPP_VARIABLE_ARRAY        0x02
#define TCLPP_VARIABLE_CLASS        0x03

#define TclppVariableIsScalar(varPtr) \
    (((varPtr)->type) == TCLPP_VARIABLE_SCALAR)
#define TclppVariableIsArray(varPtr) \
    (((varPtr)->type) == TCLPP_VARIABLE_ARRAY)
#define TclppVariableIsClass(varPtr) \
    (((varPtr)->type) == TCLPP_VARIABLE_CLASS)

#define TCLPP_VARIABLE_STATIC       0x01
#define TCLPP_VARIABLE_PUBLIC       0x10
#define TCLPP_VARIABLE_PROTECTED    0x20
#define TCLPP_VARIABLE_PRIVATE      0x40

#define TclppVariableIsStatic(varPtr) \
    (((varPtr)->flags) & TCLPP_VARIABLE_STATIC)
#define TclppVariableIsPublic(varPtr) \
    (((varPtr)->flags) & TCLPP_VARIABLE_PUBLIC)
#define TclppVariableIsProtected(varPtr) \
    (((varPtr)->flags) & TCLPP_VARIABLE_PROTECTED)
#define TclppVariableIsPrivate(varPtr) \
    (((varPtr)->flags) & TCLPP_VARIABLE_PRIVATE)

/*
 * Data structure to maintain class instance information. This is the
 * public section of the internal TclppInstance. The entries must match
 * with the first entries of the TclppInstance structure.
 */

typedef struct Tclpp_Instance {
    char          *name;         /* The unqualifed name of the instance,
                                  * containing no ::'s */
    char          *fullName;     /* The fully qualifed name of the instance,
                                  * starting with :: */
    Tclpp_Class   *classPtr;     /* The associated class of this instance */
    Tcl_HashTable variableTable; /* The list of variables for this instance. 
                                  * Each entry is a Tclpp_Variable */
} Tclpp_Instance;

/*
 * API Procedures defined by this package.
 */

EXTERN int                  Tclpp_AddBaseClass _ANSI_ARGS_ ((
                                Tcl_Interp* interp, Tclpp_Class* dervClassPtr,
                                Tclpp_Class* baseClassPtr));
EXTERN Tclpp_Class*         Tclpp_CreateClass _ANSI_ARGS_ ((Tcl_Interp* interp, 
                                char* name, Tcl_Obj* definition));
EXTERN Tclpp_Instance*      Tclpp_CreateInstance _ANSI_ARGS_ ((
                                Tcl_Interp* interp, char* name, 
                                Tclpp_Class* classPtr, int objc,
                                Tcl_Obj * CONST objv[]));
EXTERN Tclpp_Method*        Tclpp_CreateMethod _ANSI_ARGS_ ((
                                Tcl_Interp* interp, Tclpp_Class* classPtr, 
                                char* name, Tcl_Obj* args, Tcl_Obj* body));
EXTERN Tclpp_Variable*      Tclpp_CreateVariable _ANSI_ARGS_ ((
                                Tcl_Interp* interp, Tclpp_Class* classPtr, 
                                char* type, char* name));
EXTERN int                  Tclpp_DeleteInstance _ANSI_ARGS_ ((
                                Tcl_Interp* interp, 
                                Tclpp_Instance* instancePtr));
EXTERN Tclpp_Class*         Tclpp_GetClassByName _ANSI_ARGS_ ((
                                Tcl_Interp* interp, char *className));
EXTERN Tclpp_Class*         Tclpp_GetCurrentClass _ANSI_ARGS_ ((
                                Tcl_Interp* interp));
EXTERN Tclpp_Instance*      Tclpp_GetInstanceByName _ANSI_ARGS_ ((
                                Tcl_Interp* interp, char *instanceName));
EXTERN int                  Tclpp_Init _ANSI_ARGS_ ((Tcl_Interp* interp));
EXTERN int                  Tclpp_InvokeMethod _ANSI_ARGS_ ((
                                Tcl_Interp* interp, Tclpp_Instance*, int objc,
                                Tcl_Obj * CONST objv[]));

#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLIMPORT

#endif

/*
 * vi: set ai expandtab ts=4: 
 */

