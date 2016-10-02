   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  10/01/16             */
   /*                                                     */
   /*          FACT RETE ACCESS FUNCTIONS MODULE          */
   /*******************************************************/

/*************************************************************/
/* Purpose: Rete access functions for fact pattern matching. */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*                                                           */
/* Contributing Programmer(s):                               */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*      6.23: Correction for FalseSymbol/TrueSymbol. DR0859  */
/*                                                           */
/*      6.24: Removed INCREMENTAL_RESET compilation flag.    */
/*                                                           */
/*            Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*      6.30: Removed conditional code for unsupported       */
/*            compilers/operating systems (IBM_MCW,          */
/*            MAC_MCW, and IBM_TBC).                         */
/*                                                           */
/*            Support for hashing optimizations.             */
/*                                                           */
/*      6.40: Pragma once and other inclusion changes.       */
/*                                                           */
/*            Added support for booleans with <stdbool.h>.   */
/*                                                           */
/*            Removed use of void pointers for specific      */
/*            data structures.                               */
/*                                                           */
/*            UDF redesign.                                  */
/*                                                           */
/*      6.50: Modify command preserves fact address.         */
/*                                                           */
/*************************************************************/

#include <stdio.h>

#include "setup.h"

#if DEFTEMPLATE_CONSTRUCT && DEFRULE_CONSTRUCT

#include "drive.h"
#include "engine.h"
#include "envrnmnt.h"
#include "extnfunc.h"
#include "factgen.h"
#include "factmch.h"
#include "incrrset.h"
#include "memalloc.h"
#include "reteutil.h"
#include "router.h"

#include "factrete.h"

/***************************************************************/
/* FactPNGetVar1: Fact pattern network function for extracting */
/*   a variable's value. This is the most generalized routine. */
/***************************************************************/
bool FactPNGetVar1(
  Environment *theEnv,
  void *theValue,
  CLIPSValue *returnValue)
  {
   unsigned short theField, theSlot;
   struct fact *factPtr;
   struct field *fieldPtr;
   struct multifieldMarker *marks;
   struct multifield *segmentPtr;
   int extent;
   struct factGetVarPN1Call *hack;

   /*==========================================*/
   /* Retrieve the arguments for the function. */
   /*==========================================*/

   hack = (struct factGetVarPN1Call *) ((CLIPSBitMap *) theValue)->contents;

   /*=====================================================*/
   /* Get the pointer to the fact from the partial match. */
   /*=====================================================*/

   factPtr = FactData(theEnv)->CurrentPatternFact;
   marks = FactData(theEnv)->CurrentPatternMarks;

   /*==========================================================*/
   /* Determine if we want to retrieve the fact address of the */
   /* fact, rather than retrieving a field from the fact.      */
   /*==========================================================*/

   if (hack->factAddress)
     {
      returnValue->value = factPtr;
      return true;
     }

   /*=========================================================*/
   /* Determine if we want to retrieve the entire slot value. */
   /*=========================================================*/

   if (hack->allFields)
     {
      theSlot = hack->whichSlot;
      fieldPtr = &factPtr->theProposition.theFields[theSlot];
      returnValue->value = fieldPtr->value;
      if (returnValue->header->type == MULTIFIELD)
        {
         returnValue->begin = 0;
         returnValue->end = ((Multifield *) fieldPtr->value)->multifieldLength - 1;
        }

      return true;
     }

   /*====================================================*/
   /* If the slot being accessed is a single field slot, */
   /* then just return the single value found in that    */
   /* slot. The multifieldMarker data structures do not  */
   /* have to be considered since access to a single     */
   /* field slot is not affected by variable bindings    */
   /* from multifield slots.                             */
   /*====================================================*/

   theField = hack->whichField;
   theSlot = hack->whichSlot;
   fieldPtr = &factPtr->theProposition.theFields[theSlot];

   /*==========================================================*/
   /* Retrieve a value from a multifield slot. First determine */
   /* the range of fields for the variable being retrieved.    */
   /*==========================================================*/

   extent = -1;
   theField = AdjustFieldPosition(theEnv,marks,theField,theSlot,&extent);

   /*=============================================================*/
   /* If a range of values are being retrieved (i.e. a multifield */
   /* variable), then return the values as a multifield.          */
   /*=============================================================*/

   if (extent != -1)
     {
      returnValue->value = fieldPtr->value;
      returnValue->begin = theField;
      returnValue->end = theField + extent - 1;
      return true;
     }

   /*========================================================*/
   /* Otherwise a single field value is being retrieved from */
   /* a multifield slot. Just return the type and value.     */
   /*========================================================*/

   segmentPtr = (Multifield *) fieldPtr->value;
   fieldPtr = &segmentPtr->theFields[theField];

   returnValue->value = fieldPtr->value;

   return true;
  }

/**************************************************/
/* FactPNGetVar2: Fact pattern network function   */
/*   for extracting a variable's value. The value */
/*   extracted is from a single field slot.       */
/**************************************************/
bool FactPNGetVar2(
  Environment *theEnv,
  void *theValue,
  CLIPSValue *returnValue)
  {
   struct fact *factPtr;
   struct factGetVarPN2Call *hack;
   struct field *fieldPtr;

   /*==========================================*/
   /* Retrieve the arguments for the function. */
   /*==========================================*/

   hack = (struct factGetVarPN2Call *) ((CLIPSBitMap *) theValue)->contents;

   /*==============================*/
   /* Get the pointer to the fact. */
   /*==============================*/

   factPtr = FactData(theEnv)->CurrentPatternFact;

   /*============================================*/
   /* Extract the value from the specified slot. */
   /*============================================*/

   fieldPtr = &factPtr->theProposition.theFields[hack->whichSlot];

   returnValue->value = fieldPtr->value;

   return true;
  }

/*****************************************************************/
/* FactPNGetVar3: Fact pattern network function for extracting a */
/*   variable's value. The value extracted is from a multifield  */
/*   slot that contains at most one multifield variable.         */
/*****************************************************************/
bool FactPNGetVar3(
  Environment *theEnv,
  void *theValue,
  CLIPSValue *returnValue)
  {
   struct fact *factPtr;
   struct multifield *segmentPtr;
   struct field *fieldPtr;
   struct factGetVarPN3Call *hack;

   /*==========================================*/
   /* Retrieve the arguments for the function. */
   /*==========================================*/

   hack = (struct factGetVarPN3Call *) ((CLIPSBitMap *) theValue)->contents;

   /*==============================*/
   /* Get the pointer to the fact. */
   /*==============================*/

   factPtr = FactData(theEnv)->CurrentPatternFact;

   /*============================================================*/
   /* Get the multifield value from which the data is retrieved. */
   /*============================================================*/

   segmentPtr = (Multifield *) factPtr->theProposition.theFields[hack->whichSlot].value;

   /*=========================================*/
   /* If the beginning and end flags are set, */
   /* then retrieve a multifield value.       */
   /*=========================================*/

   if (hack->fromBeginning && hack->fromEnd)
     {
      returnValue->value = segmentPtr;
      returnValue->begin = (long) hack->beginOffset;
      returnValue->end = (long) (segmentPtr->multifieldLength - (hack->endOffset + 1));
      return true;
     }

   /*=====================================================*/
   /* Return a single field value from a multifield slot. */
   /*=====================================================*/

   if (hack->fromBeginning)
     { fieldPtr = &segmentPtr->theFields[hack->beginOffset]; }
   else
     { fieldPtr = &segmentPtr->theFields[segmentPtr->multifieldLength - (hack->endOffset + 1)]; }

   returnValue->value = fieldPtr->value;

   return true;
  }

/******************************************************/
/* FactPNConstant1: Fact pattern network function for */
/*   comparing a value stored in a single field slot  */
/*   to a constant for either equality or inequality. */
/******************************************************/
bool FactPNConstant1(
  Environment *theEnv,
  void *theValue,
  CLIPSValue *returnValue)
  {
#if MAC_XCD
#pragma unused(returnValue)
#endif
   struct factConstantPN1Call *hack;
   struct field *fieldPtr;
   struct expr *theConstant;

   /*==========================================*/
   /* Retrieve the arguments for the function. */
   /*==========================================*/

   hack = (struct factConstantPN1Call *) ((CLIPSBitMap *) theValue)->contents;

   /*============================================*/
   /* Extract the value from the specified slot. */
   /*============================================*/

   fieldPtr = &FactData(theEnv)->CurrentPatternFact->theProposition.theFields[hack->whichSlot];

   /*====================================*/
   /* Compare the value to the constant. */
   /*====================================*/

   theConstant = GetFirstArgument();
   if (theConstant->value != fieldPtr->value) return(1 - hack->testForEquality);
   return(hack->testForEquality);
  }

/****************************************************************/
/* FactPNConstant2: Fact pattern network function for comparing */
/*   a value stored in a slot to a constant for either equality */
/*   or inequality. The value being retrieved from the slot has */
/*   no multifields to its right (thus it can be retrieved      */
/*   relative to the beginning).                                */
/****************************************************************/
bool FactPNConstant2(
  Environment *theEnv,
  void *theValue,
  CLIPSValue *returnValue)
  {
#if MAC_XCD
#pragma unused(returnValue)
#endif
   struct factConstantPN2Call *hack;
   struct field *fieldPtr;
   struct expr *theConstant;
   struct multifield *segmentPtr;

   /*==========================================*/
   /* Retrieve the arguments for the function. */
   /*==========================================*/

   hack = (struct factConstantPN2Call *) ((CLIPSBitMap *) theValue)->contents;

   /*==========================================================*/
   /* Extract the value from the specified slot. Note that the */
   /* test to determine the slot's type (multifield) should be */
   /* unnecessary since this routine should only be used for   */
   /* multifield slots.                                        */
   /*==========================================================*/

   fieldPtr = &FactData(theEnv)->CurrentPatternFact->theProposition.theFields[hack->whichSlot];

   if (fieldPtr->header->type == MULTIFIELD)
     {
      segmentPtr = (Multifield *) fieldPtr->value;

      if (hack->fromBeginning)
        { fieldPtr = &segmentPtr->theFields[hack->offset]; }
      else
        {
         fieldPtr = &segmentPtr->theFields[segmentPtr->multifieldLength -
                    (hack->offset + 1)];
        }
     }

   /*====================================*/
   /* Compare the value to the constant. */
   /*====================================*/

   theConstant = GetFirstArgument();
   if (theConstant->value != fieldPtr->value) return(1 - hack->testForEquality);
   return(hack->testForEquality);
  }

/**************************************************************/
/* FactJNGetVar1: Fact join network function for extracting a */
/*   variable's value. This is the most generalized routine.  */
/**************************************************************/
bool FactJNGetVar1(
  Environment *theEnv,
  void *theValue,
  CLIPSValue *returnValue)
  {
   unsigned short theField, theSlot;
   struct fact *factPtr;
   struct field *fieldPtr;
   struct multifieldMarker *marks;
   Multifield *segmentPtr;
   int extent;
   struct factGetVarJN1Call *hack;
   struct multifield *theSlots = NULL;

   /*==========================================*/
   /* Retrieve the arguments for the function. */
   /*==========================================*/

   hack = (struct factGetVarJN1Call *) ((CLIPSBitMap *) theValue)->contents;

   /*=====================================================*/
   /* Get the pointer to the fact from the partial match. */
   /*=====================================================*/

   if (hack->lhs)
     {
      factPtr = (Fact *) get_nth_pm_match(EngineData(theEnv)->GlobalLHSBinds,hack->whichPattern)->matchingItem;
      marks = get_nth_pm_match(EngineData(theEnv)->GlobalLHSBinds,hack->whichPattern)->markers;
     }
   else if (hack->rhs)
     {
      factPtr = (Fact *) get_nth_pm_match(EngineData(theEnv)->GlobalRHSBinds,hack->whichPattern)->matchingItem;
      marks = get_nth_pm_match(EngineData(theEnv)->GlobalRHSBinds,hack->whichPattern)->markers;
     }
   else if (EngineData(theEnv)->GlobalRHSBinds == NULL)
     {
      factPtr = (Fact *) get_nth_pm_match(EngineData(theEnv)->GlobalLHSBinds,hack->whichPattern)->matchingItem;
      marks = get_nth_pm_match(EngineData(theEnv)->GlobalLHSBinds,hack->whichPattern)->markers;
     }
   else if ((((unsigned short) (EngineData(theEnv)->GlobalJoin->depth - 1))) == hack->whichPattern)
     {
      factPtr = (Fact *) get_nth_pm_match(EngineData(theEnv)->GlobalRHSBinds,0)->matchingItem;
      marks = get_nth_pm_match(EngineData(theEnv)->GlobalRHSBinds,0)->markers;
     }
   else
     {
      factPtr = (Fact *) get_nth_pm_match(EngineData(theEnv)->GlobalLHSBinds,hack->whichPattern)->matchingItem;
      marks = get_nth_pm_match(EngineData(theEnv)->GlobalLHSBinds,hack->whichPattern)->markers;
     }

   /*==========================================================*/
   /* Determine if we want to retrieve the fact address of the */
   /* fact, rather than retrieving a field from the fact.      */
   /*==========================================================*/

   if (hack->factAddress)
     {
      returnValue->value = factPtr;
      return true;
     }

   if ((factPtr->basisSlots != NULL) &&
       (! EngineData(theEnv)->JoinOperationInProgress))
     { theSlots = factPtr->basisSlots; }
   else
     { theSlots = &factPtr->theProposition; }

   /*=========================================================*/
   /* Determine if we want to retrieve the entire slot value. */
   /*=========================================================*/

   if (hack->allFields)
     {
      theSlot = hack->whichSlot;
      fieldPtr = &theSlots->theFields[theSlot];
      returnValue->value = fieldPtr->value;
      if (returnValue->header->type == MULTIFIELD)
        {
         returnValue->begin = 0;
         returnValue->end = ((Multifield *) fieldPtr->value)->multifieldLength - 1;
        }

      return true;
     }

   /*====================================================*/
   /* If the slot being accessed is a single field slot, */
   /* then just return the single value found in that    */
   /* slot. The multifieldMarker data structures do not  */
   /* have to be considered since access to a single     */
   /* field slot is not affected by variable bindings    */
   /* from multifield slots.                             */
   /*====================================================*/

   theField = hack->whichField;
   theSlot = hack->whichSlot;
   fieldPtr = &theSlots->theFields[theSlot];

   if (fieldPtr->header->type != MULTIFIELD)
     {
      returnValue->value = fieldPtr->value;
      return true;
     }

   /*==========================================================*/
   /* Retrieve a value from a multifield slot. First determine */
   /* the range of fields for the variable being retrieved.    */
   /*==========================================================*/

   extent = -1;
   theField = AdjustFieldPosition(theEnv,marks,theField,theSlot,&extent);

   /*=============================================================*/
   /* If a range of values are being retrieved (i.e. a multifield */
   /* variable), then return the values as a multifield.          */
   /*=============================================================*/

   if (extent != -1)
     {
      returnValue->value = fieldPtr->value;
      returnValue->begin = theField;
      returnValue->end = theField + extent - 1;
      return true;
     }

   /*========================================================*/
   /* Otherwise a single field value is being retrieved from */
   /* a multifield slot. Just return the type and value.     */
   /*========================================================*/

   segmentPtr = (Multifield *) theSlots->theFields[theSlot].value;
   fieldPtr = &segmentPtr->theFields[theField];

   returnValue->value = fieldPtr->value;

   return true;
  }

/*************************************************/
/* FactJNGetVar2: Fact join network function for */
/*   extracting a variable's value. The value    */
/*   extracted is from a single field slot.      */
/*************************************************/
bool FactJNGetVar2(
  Environment *theEnv,
  void *theValue,
  CLIPSValue *returnValue)
  {
   struct fact *factPtr;
   struct factGetVarJN2Call *hack;
   struct field *fieldPtr;

   /*==========================================*/
   /* Retrieve the arguments for the function. */
   /*==========================================*/

   hack = (struct factGetVarJN2Call *) ((CLIPSBitMap *) theValue)->contents;

   /*=====================================================*/
   /* Get the pointer to the fact from the partial match. */
   /*=====================================================*/

   if (hack->lhs)
     { factPtr = (Fact *) get_nth_pm_match(EngineData(theEnv)->GlobalLHSBinds,hack->whichPattern)->matchingItem; }
   else if (hack->rhs)
     { factPtr = (Fact *) get_nth_pm_match(EngineData(theEnv)->GlobalRHSBinds,hack->whichPattern)->matchingItem; }
   else if (EngineData(theEnv)->GlobalRHSBinds == NULL)
     { factPtr = (Fact *) get_nth_pm_match(EngineData(theEnv)->GlobalLHSBinds,hack->whichPattern)->matchingItem; }
   else if (((unsigned short) (EngineData(theEnv)->GlobalJoin->depth - 1)) == hack->whichPattern)
	 { factPtr = (Fact *) get_nth_pm_match(EngineData(theEnv)->GlobalRHSBinds,0)->matchingItem; }
   else
     { factPtr = (Fact *) get_nth_pm_match(EngineData(theEnv)->GlobalLHSBinds,hack->whichPattern)->matchingItem; }

   /*============================================*/
   /* Extract the value from the specified slot. */
   /*============================================*/

   if ((factPtr->basisSlots != NULL) &&
       (! EngineData(theEnv)->JoinOperationInProgress))
     { fieldPtr = &factPtr->basisSlots->theFields[hack->whichSlot]; }
   else
     { fieldPtr = &factPtr->theProposition.theFields[hack->whichSlot]; }

   returnValue->value = fieldPtr->value;

   return true;
  }

/****************************************************************/
/* FactJNGetVar3: Fact join network function for extracting a   */
/*   variable's value. The value extracted is from a multifield */
/*   slot that contains at most one multifield variable.        */
/****************************************************************/
bool FactJNGetVar3(
  Environment *theEnv,
  void *theValue,
  CLIPSValue *returnValue)
  {
   struct fact *factPtr;
   struct multifield *segmentPtr;
   struct field *fieldPtr;
   struct factGetVarJN3Call *hack;

   /*==========================================*/
   /* Retrieve the arguments for the function. */
   /*==========================================*/

   hack = (struct factGetVarJN3Call *) ((CLIPSBitMap *) theValue)->contents;

   /*=====================================================*/
   /* Get the pointer to the fact from the partial match. */
   /*=====================================================*/

   if (hack->lhs)
     { factPtr = (Fact *) get_nth_pm_match(EngineData(theEnv)->GlobalLHSBinds,hack->whichPattern)->matchingItem; }
   else if (hack->rhs)
     { factPtr = (Fact *) get_nth_pm_match(EngineData(theEnv)->GlobalRHSBinds,hack->whichPattern)->matchingItem; }
   else if (EngineData(theEnv)->GlobalRHSBinds == NULL)
     { factPtr = (Fact *) get_nth_pm_match(EngineData(theEnv)->GlobalLHSBinds,hack->whichPattern)->matchingItem; }
   else if (((unsigned short) (EngineData(theEnv)->GlobalJoin->depth - 1)) == hack->whichPattern)
     { factPtr = (Fact *) get_nth_pm_match(EngineData(theEnv)->GlobalRHSBinds,0)->matchingItem; }
   else
     { factPtr = (Fact *) get_nth_pm_match(EngineData(theEnv)->GlobalLHSBinds,hack->whichPattern)->matchingItem; }

   /*============================================================*/
   /* Get the multifield value from which the data is retrieved. */
   /*============================================================*/

   if ((factPtr->basisSlots != NULL) &&
       (! EngineData(theEnv)->JoinOperationInProgress))
     { segmentPtr = (Multifield *) factPtr->basisSlots->theFields[hack->whichSlot].value; }
   else
     { segmentPtr = (Multifield *) factPtr->theProposition.theFields[hack->whichSlot].value; }

   /*=========================================*/
   /* If the beginning and end flags are set, */
   /* then retrieve a multifield value.       */
   /*=========================================*/

   if (hack->fromBeginning && hack->fromEnd)
     {
      returnValue->value = segmentPtr;
      returnValue->begin = hack->beginOffset;
      returnValue->end = (long) (segmentPtr->multifieldLength - (hack->endOffset + 1));
      return true;
     }

   /*=====================================================*/
   /* Return a single field value from a multifield slot. */
   /*=====================================================*/

   if (hack->fromBeginning)
     { fieldPtr = &segmentPtr->theFields[hack->beginOffset]; }
   else
     { fieldPtr = &segmentPtr->theFields[segmentPtr->multifieldLength - (hack->endOffset + 1)]; }

   returnValue->value = fieldPtr->value;

   return true;
  }

/****************************************************/
/* FactSlotLength: Determines if the length of a    */
/*  multifield slot falls within a specified range. */
/****************************************************/
bool FactSlotLength(
  Environment *theEnv,
  void *theValue,
  CLIPSValue *returnValue)
  {
   struct factCheckLengthPNCall *hack;
   struct multifield *segmentPtr;
   long extraOffset = 0;
   struct multifieldMarker *tempMark;

   returnValue->value = theEnv->FalseSymbol;

   hack = (struct factCheckLengthPNCall *) ((CLIPSBitMap *) theValue)->contents;

   for (tempMark = FactData(theEnv)->CurrentPatternMarks;
        tempMark != NULL;
        tempMark = tempMark->next)
     {
      if (tempMark->where.whichSlotNumber != hack->whichSlot) continue;
      extraOffset += ((tempMark->endPosition - tempMark->startPosition) + 1);
     }

   segmentPtr = (Multifield *) FactData(theEnv)->CurrentPatternFact->theProposition.theFields[hack->whichSlot].value;

   if (segmentPtr->multifieldLength < (hack->minLength + extraOffset))
     { return false; }

   if (hack->exactly && (segmentPtr->multifieldLength > (hack->minLength + extraOffset)))
     { return false; }

   returnValue->value = theEnv->TrueSymbol;
   return true;
  }

/************************************************************/
/* FactJNCompVars1: Fact join network routine for comparing */
/*   the values of two single field slots.                  */
/************************************************************/
bool FactJNCompVars1(
  Environment *theEnv,
  void *theValue,
  CLIPSValue *theResult)
  {
#if MAC_XCD
#pragma unused(theResult)
#endif
   int p1, e1, p2, e2;
   struct fact *fact1, *fact2;
   struct factCompVarsJN1Call *hack;

   /*=========================================*/
   /* Retrieve the arguments to the function. */
   /*=========================================*/

   hack = (struct factCompVarsJN1Call *) ((CLIPSBitMap *) theValue)->contents;

   /*=================================================*/
   /* Extract the fact pointers for the two patterns. */
   /*=================================================*/

   p1 = (int) hack->pattern1;
   p2 = (int) hack->pattern2;

   fact1 = (Fact *) EngineData(theEnv)->GlobalRHSBinds->binds[p1].gm.theMatch->matchingItem;

   if (hack->p2rhs)
     { fact2 = (Fact *) EngineData(theEnv)->GlobalRHSBinds->binds[p2].gm.theMatch->matchingItem; }
   else
     { fact2 = (Fact *) EngineData(theEnv)->GlobalLHSBinds->binds[p2].gm.theMatch->matchingItem; }

   /*=====================*/
   /* Compare the values. */
   /*=====================*/

   e1 = (int) hack->slot1;
   e2 = (int) hack->slot2;

   if (fact1->theProposition.theFields[e1].value !=
       fact2->theProposition.theFields[e2].value)
     { return((bool) hack->fail); }

   return((bool) hack->pass);
  }

/*****************************************************************/
/* FactJNCompVars2:  Fact join network routine for comparing the */
/*   two single field value that are found in the first slot     */
/*   (which must also be a multifield slot) of a deftemplate.    */
/*   This function is provided so that variable comparisons of   */
/*   implied deftemplates will be faster.                        */
/*****************************************************************/
bool FactJNCompVars2(
  Environment *theEnv,
  void *theValue,
  CLIPSValue *theResult)
  {
#if MAC_XCD
#pragma unused(theResult)
#endif
   int p1, s1, p2, s2;
   struct fact *fact1, *fact2;
   struct factCompVarsJN2Call *hack;
   struct multifield *segment;
   struct field *fieldPtr1, *fieldPtr2;

   /*=========================================*/
   /* Retrieve the arguments to the function. */
   /*=========================================*/

   hack = (struct factCompVarsJN2Call *) ((CLIPSBitMap *) theValue)->contents;

   /*=================================================*/
   /* Extract the fact pointers for the two patterns. */
   /*=================================================*/

   p1 = (int) hack->pattern1;
   p2 = (int) hack->pattern2;
   s1 = (int) hack->slot1;
   s2 = (int) hack->slot2;

   fact1 = (Fact *) EngineData(theEnv)->GlobalRHSBinds->binds[p1].gm.theMatch->matchingItem;

   if (hack->p2rhs)
     { fact2 = (Fact *) EngineData(theEnv)->GlobalRHSBinds->binds[p2].gm.theMatch->matchingItem; }
   else
     { fact2 = (Fact *) EngineData(theEnv)->GlobalLHSBinds->binds[p2].gm.theMatch->matchingItem; }

   /*======================*/
   /* Retrieve the values. */
   /*======================*/

   if (fact1->theProposition.theFields[s1].header->type != MULTIFIELD)
     { fieldPtr1 = &fact1->theProposition.theFields[s1]; }
   else
     {
      segment = (Multifield *) fact1->theProposition.theFields[s1].value;

      if (hack->fromBeginning1)
        { fieldPtr1 = &segment->theFields[hack->offset1]; }
      else
        { fieldPtr1 = &segment->theFields[segment->multifieldLength - (hack->offset1 + 1)]; }
     }

   if (fact2->theProposition.theFields[s2].header->type != MULTIFIELD)
     { fieldPtr2 = &fact2->theProposition.theFields[s2]; }
   else
     {
      segment = (Multifield *) fact2->theProposition.theFields[s2].value;

      if (hack->fromBeginning2)
        { fieldPtr2 = &segment->theFields[hack->offset2]; }
      else
        { fieldPtr2 = &segment->theFields[segment->multifieldLength - (hack->offset2 + 1)]; }
     }

   /*=====================*/
   /* Compare the values. */
   /*=====================*/

   if (fieldPtr1->value != fieldPtr2->value)
     { return((bool) hack->fail); }

   return((bool) hack->pass);
  }

/*****************************************************/
/* FactPNCompVars1: Fact pattern network routine for */
/*   comparing the values of two single field slots. */
/*****************************************************/
bool FactPNCompVars1(
  Environment *theEnv,
  void *theValue,
  CLIPSValue *theResult)
  {
   int rv;
   struct field *fieldPtr1, *fieldPtr2;
   struct factCompVarsPN1Call *hack;

   /*========================================*/
   /* Extract the arguments to the function. */
   /*========================================*/

   hack = (struct factCompVarsPN1Call *) ((CLIPSBitMap *) theValue)->contents;
   fieldPtr1 = &FactData(theEnv)->CurrentPatternFact->theProposition.theFields[hack->field1];
   fieldPtr2 = &FactData(theEnv)->CurrentPatternFact->theProposition.theFields[hack->field2];

   /*=====================*/
   /* Compare the values. */
   /*=====================*/

   if (fieldPtr1->value != fieldPtr2->value) rv = (int) hack->fail;
   else rv = (int) hack->pass;

   if (rv) theResult->value = theEnv->TrueSymbol;
   else theResult->value = theEnv->FalseSymbol;

   return(rv);
  }

/*************************************************************************/
/* AdjustFieldPosition: Given a list of multifield markers and the index */
/*   to a variable in a slot, this function computes the index to the    */
/*   field in the slot where the variable begins. In the case of         */
/*   multifield variables, it also computes the extent (or length) of    */
/*   the multifield. Note that the extent should be given a default      */
/*   value of either -1 or 1 for variables other than multifield         */
/*   variables before calling this routine.  An extent of -1 for these   */
/*   variables will distinguish their extent as being different when it  */
/*   is necessary to note their difference from a multifield variable    */
/*   with an extent of 1. For example, given the slot pattern            */
/*   (data $?x c $?y ?z) and the slot value (data a b c d e f x), the    */
/*   actual index in the fact for the 5th item in the pattern (the       */
/*   variable ?z) would be 8 since $?x binds to 2 fields and $?y binds   */
/*   to 3 fields.                                                        */
/*************************************************************************/
unsigned short AdjustFieldPosition(
  Environment *theEnv,
  struct multifieldMarker *markList,
  unsigned short whichField,
  unsigned short whichSlot,
  int *extent)
  {
   unsigned short actualIndex;
#if MAC_XCD
#pragma unused(theEnv)
#endif

   actualIndex = whichField;
   for (;
        markList != NULL;
        markList = markList->next)
     {
      /*===============================================*/
      /* Skip over multifield markers for other slots. */
      /*===============================================*/

      if (markList->where.whichSlotNumber != whichSlot) continue;

      /*=========================================================*/
      /* If the multifield marker occurs exactly at the field in */
      /* question, then the actual index needs to be adjusted    */
      /* and the extent needs to be computed since the value is  */
      /* a multifield value.                                     */
      /*=========================================================*/

      if (markList->whichField == whichField)
        {
         *extent = (markList->endPosition - markList->startPosition) + 1;
         return(actualIndex);
        }

      /*=====================================================*/
      /* Otherwise if the multifield marker occurs after the */
      /* field in question, then the actual index has been   */
      /* completely computed and can be returned.            */
      /*=====================================================*/

      else if (markList->whichField > whichField)
        { return(actualIndex); }

      /*==========================================================*/
      /* Adjust the actual index to the field based on the number */
      /* of fields taken up by the preceding multifield variable. */
      /*==========================================================*/

      actualIndex += (unsigned short) (markList->endPosition - markList->startPosition);
     }

   /*=======================================*/
   /* Return the actual index to the field. */
   /*=======================================*/

   return(actualIndex);
  }

/*****************************************************/
/* FactStoreMultifield: This primitive is used by a  */
/*   number of multifield functions for grouping a   */
/*   series of valuesinto a single multifield value. */
/*****************************************************/
bool FactStoreMultifield(
  Environment *theEnv,
  void *theValue,
  CLIPSValue *theResult)
  {
#if MAC_XCD
#pragma unused(theValue)
#endif

   StoreInMultifield(theEnv,theResult,GetFirstArgument(),false);
   return true;
  }

#endif /* DEFTEMPLATE_CONSTRUCT && DEFRULE_CONSTRUCT */

