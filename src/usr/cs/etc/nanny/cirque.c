/*****************************************************************
 *
 *	cirque - circular queue package
 *
 *****************************************************************
 * HISTORY
 * 15-Jan-87  Rudy Nedved (ern) at Carnegie-Mellon University
 *	Added missing semi-colon after struct decl. I created this 
 *	package a year or so ago.
 *
 */

#include <stdio.h>

struct DumRec {
    struct DumRec *Next,*Prev;
    int foo[1];
};

QueueTail(Base,NewOne)
struct DumRec **Base,*NewOne;
{
    if (*Base == NULL) {
	NewOne->Next = NewOne;
	NewOne->Prev = NewOne;
	*Base = NewOne;
    }
    else {
	NewOne->Next = *Base;
	NewOne->Prev = (*Base)->Prev;
	((*Base)->Prev)->Next = NewOne;
	(*Base)->Prev = NewOne;
    }
}

QueueHead(Base,NewOne)
struct DumRec **Base,*NewOne;
{
    QueueTail(Base,NewOne);
    *Base = NewOne;
}

QueueBefore(Base,NewOne,Referent)
struct DumRec **Base,*NewOne,*Referent;
{
    if (*Base == NULL) {
	NewOne->Next = NewOne;
	NewOne->Prev = NewOne;
	*Base = NewOne;
    }
    else {
	NewOne->Next = Referent;
	NewOne->Prev = Referent->Prev;
	(Referent->Prev)->Next = NewOne;
	Referent->Prev = NewOne;
    }
}

QueueAfter(Base,NewOne,Referent)
struct DumRec **Base,*NewOne,*Referent;
{
    if (*Base == NULL) {
	NewOne->Next = NewOne;
	NewOne->Prev = NewOne;
	*Base = NewOne;
    }
    else {
	NewOne->Prev = Referent;
	NewOne->Next = Referent->Next;
	(Referent->Next)->Prev = NewOne;
	Referent->Next = NewOne;
    }
}

DeQueue(Base,TheOne)
struct DumRec **Base,*TheOne;
{
    if (*Base == (*Base)->Next) {
	if (*Base != TheOne) {
	    fprintf(stderr,"DeQueue: given not in list\n");
	    abort();
	}
	*Base = NULL;
    }
    else {
	if (*Base == TheOne)
	    *Base = TheOne->Next;
	(TheOne->Prev)->Next = TheOne->Next;
	(TheOne->Next)->Prev = TheOne->Prev;
    }
    TheOne->Next = NULL;
    TheOne->Prev = NULL;
}

struct DumRec *DeQHead(Base)
struct DumRec **Base;
{
    struct DumRec *TheOne;
    if (*Base == NULL) {
	fprintf(stderr,"DeQHead: null list\n");
	abort();
    }
    TheOne = *Base;
    DeQueue(Base,TheOne);
    return (TheOne);
}

struct DumRec *DeQTail(Base)
struct DumRec **Base;
{
    struct DumRec *TheOne;
    if (*Base == NULL) {
	fprintf(stderr,"DeQTail: null list\n");
	abort();
    }
    TheOne = (*Base)->Prev;
    DeQueue(Base,TheOne);
    return (TheOne);
}
