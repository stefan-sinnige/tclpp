/* 
 * tclppWin32Dll.c --
 *
 *	This file contains the dll entry point.
 *
 * Copyright (c) 2000, Stefan Sinnige.
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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#if defined (_MSC_VER)
# define DLLEntryPoint DllMain
#endif


/*
 *----------------------------------------------------------------------
 *
 * DllEntryPoint --
 *
 *	This wrapper function is used by Windows to invoke the initialization
 *	code for the DLL. 
 *
 * Results:
 *	Returns True.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

BOOL APIENTRY
DllEntryPoint(hInst, reason, reserved)
    HINSTANCE hInst;		/* Library instance handle. */
    DWORD reason;	    	/* Reason this function is being called. */
    LPVOID reserved;		/* Not used. */
{
    return TRUE;
}
