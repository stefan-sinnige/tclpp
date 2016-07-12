/*
 * tclppQueue.c --
 *
 *     Implementation of in-memory queues for Tclpp and Tclpp-based 
 *     applications.
 *
 * RCS: $Id: tclppQueue.c,v 1.1 2000/04/30 14:07:28 stefan Exp $
 *
 * Copyright (C) 2000, Stefan Sinnige.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * For the full GNU General Public License, see the 'LICENSE' file.
 *
 */

#include "tclppInt.h"

static char rcsid[] = "$Id: tclppQueue.c,v 1.1 2000/04/30 14:07:28 stefan Exp $";


/*
 * ----------------------------------------------------------------------------
 *     
 * Tclpp_InitQueue --
 * 
 *     Given storage for a queue, setup the field to prepare the queue
 *     for use.
 *
 * Results:
 *     None.
 *  
 * Side effects:
 *     The Queue is now ready for use. 
 *  
 * ----------------------------------------------------------------------------
 */ 

int
Tclpp_InitQueue (queuePtr)
    Tclpp_Queue *queuePtr;
{
    queuePtr->head = (Tclpp_QueueEntry*) NULL;
    queuePtr->tail = (Tclpp_QueueEntry*) NULL;
    queuePtr->numEntries = 0;
    return TCL_OK;
}


/*
 * ----------------------------------------------------------------------------
 *     
 * Tclpp_DeleteQueue --
 * 
 *     Free up everything, except for the client-data and the queue itself.
 *
 * Results:
 *     None.
 *  
 * Side effects:
 *     The queue is no longer usable.
 *  
 * ----------------------------------------------------------------------------
 */ 

void
Tclpp_DeleteQueue (queuePtr)
    Tclpp_Queue *queuePtr;
{
    register Tclpp_QueueEntry* entryPtr;

    /*
     * Remove all the queue entries
     */

    entryPtr = queuePtr->head;
    while (entryPtr != (Tclpp_QueueEntry*) NULL) {
        queuePtr->head = entryPtr->nextPtr;
        Tcl_Free ((char*)entryPtr);
        entryPtr = queuePtr->head;
    }

    /*
     * Mark it as an empty queue
     */

    queuePtr->head = (Tclpp_QueueEntry*) NULL;
    queuePtr->tail = (Tclpp_QueueEntry*) NULL;
    queuePtr->numEntries = 0;
}


/*
 * ----------------------------------------------------------------------------
 *     
 * Tclpp_CreateQueueEntry --
 * 
 *     Create a queue-entry.
 *
 * Results:
 *     Returns the created entry.
 *  
 * Side effects:
 *     None.
 *  
 * ----------------------------------------------------------------------------
 */ 

Tclpp_QueueEntry*
Tclpp_CreateQueueEntry (clientData)
    ClientData clientData;                  /* The client data */
{
    register Tclpp_QueueEntry* entryPtr;    /* The entry created */

    entryPtr = (Tclpp_QueueEntry*) Tcl_Alloc(sizeof(Tclpp_QueueEntry));
    entryPtr->clientData = clientData;
    entryPtr->nextPtr = (Tclpp_QueueEntry*) NULL;

    return entryPtr;
}


/*
 * ----------------------------------------------------------------------------
 *     
 * Tclpp_DeleteQueueEntry --
 * 
 *     Delete a queue-entry and free up its memory (except for the client
 *     data).
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
Tclpp_DeleteQueueEntry (entryPtr)
    Tclpp_QueueEntry *entryPtr;
{
    Tcl_Free ((char*)entryPtr);
}


/*
 * ----------------------------------------------------------------------------
 *     
 * Tclpp_AppendQueueEntry --
 * 
 *     Create a queue-entry and append it to the queue. 
 *
 * Results:
 *     Returns the appended entry or NULL on any error.
 *  
 * Side effects:
 *     None.
 *  
 * ----------------------------------------------------------------------------
 */ 

int
Tclpp_AppendQueueEntry (queuePtr, entryPtr)
    Tclpp_Queue *queuePtr;      /* The queue at append the entry to */
    Tclpp_QueueEntry* entryPtr; /* The entry to append */
{
    /*
     * Link the entry at the end of the queue. There are two special cases:
     * one where the queue is empty and one where the queue is not empty.
     */

    entryPtr->queuePtr = queuePtr;
    if (queuePtr->tail == (Tclpp_QueueEntry*) NULL) {
        queuePtr->head = entryPtr;
        queuePtr->tail = entryPtr;
    } else {
        queuePtr->tail->nextPtr = entryPtr;
    }
    queuePtr->numEntries ++;

    return TCL_OK;
}


/*
 * ----------------------------------------------------------------------------
 *     
 * Tclpp_ServeQueueEntry --
 * 
 *     Get the entry at the head of the queue and remove it from the
 *     queue.
 *
 * Results:
 *     Returns the entry or NULL if the queue was empty.
 *  
 * Side effects:
 *     The caller is responsible to delete the returned entry.
 *  
 * ----------------------------------------------------------------------------
 */ 

Tclpp_QueueEntry*
Tclpp_ServeQueueEntry (queuePtr)
    Tclpp_Queue *queuePtr;      /* The queue to get the first entry from */
{
    register Tclpp_QueueEntry *entryPtr;    /* The head of the queue */

    /* 
     * If the queue is empty, there is nothing to return.
     */

    if (queuePtr->head == (Tclpp_QueueEntry*) NULL) {
        return (Tclpp_QueueEntry*) NULL;
    }

    /*
     * Get the head of the queue and remove it from the queue. 
     */

    entryPtr = queuePtr->head;
    queuePtr->head = entryPtr->nextPtr;
    if (queuePtr->head == (Tclpp_QueueEntry*) NULL) {
        queuePtr->tail = (Tclpp_QueueEntry*) NULL;
    }
    queuePtr->numEntries --;
    entryPtr->queuePtr = (Tclpp_Queue*) NULL;

    return entryPtr;
}

/*
 * vi: set ai expandtab ts=4: 
 */

