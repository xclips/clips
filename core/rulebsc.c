   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*             CLIPS Version 6.30  02/04/15            */
   /*                                                     */
   /*          DEFRULE BASIC COMMANDS HEADER FILE         */
   /*******************************************************/

/*************************************************************/
/* Purpose: Implements core commands for the defrule         */
/*   construct such as clear, reset, save, undefrule,        */
/*   ppdefrule, list-defrules, and                           */
/*   get-defrule-list.                                       */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*                                                           */
/* Contributing Programmer(s):                               */
/*      Brian L. Dantes                                      */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*      6.23: Corrected compilation errors for files         */
/*            generated by constructs-to-c. DR0861           */
/*                                                           */
/*            Changed name of variable log to logName        */
/*            because of Unix compiler warnings of shadowed  */
/*            definitions.                                   */
/*                                                           */
/*      6.24: Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*      6.30: Removed conditional code for unsupported       */
/*            compilers/operating systems (IBM_MCW,          */
/*            MAC_MCW, and IBM_TBC).                         */
/*                                                           */
/*            Support for join network changes.              */
/*                                                           */
/*            Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*            Converted API macros to function calls.        */
/*                                                           */
/*            Added code to prevent a clear command from     */
/*            being executed during fact assertions via      */
/*            JoinOperationInProgress mechanism.             */
/*                                                           */
/*************************************************************/

#define _RULEBSC_SOURCE_

#include "setup.h"

#if DEFRULE_CONSTRUCT

#include <stdio.h>
#define _STDIO_INCLUDED_

#include "argacces.h"
#include "constrct.h"
#include "envrnmnt.h"
#include "router.h"
#include "watch.h"
#include "extnfunc.h"
#include "ruledef.h"
#include "engine.h"
#include "drive.h"
#include "reteutil.h"
#if BLOAD || BLOAD_ONLY || BLOAD_AND_BSAVE
#include "rulebin.h"
#endif
#if CONSTRUCT_COMPILER && (! RUN_TIME)
#include "rulecmp.h"
#endif

#include "rulebsc.h"

/***************************************/
/* LOCAL INTERNAL FUNCTION DEFINITIONS */
/***************************************/

   static void                    ResetDefrules(void *);
   static void                    ResetDefrulesPrime(void *);
   static void                    SaveDefrules(void *,void *,const char *);
#if (! RUN_TIME)
   static int                     ClearDefrulesReady(void *);
   static void                    ClearDefrules(void *);
#endif

/*************************************************************/
/* DefruleBasicCommands: Initializes basic defrule commands. */
/*************************************************************/
globle void DefruleBasicCommands(
  void *theEnv)
  {
   EnvAddResetFunction(theEnv,"defrule",ResetDefrules,70);
   EnvAddResetFunction(theEnv,"defrule",ResetDefrulesPrime,10);
   AddSaveFunction(theEnv,"defrule",SaveDefrules,0);
#if (! RUN_TIME)
   AddClearReadyFunction(theEnv,"defrule",ClearDefrulesReady,0);
   EnvAddClearFunction(theEnv,"defrule",ClearDefrules,0);
#endif
   
#if DEBUGGING_FUNCTIONS
   AddWatchItem(theEnv,"rules",0,&DefruleData(theEnv)->WatchRules,70,DefruleWatchAccess,DefruleWatchPrint);
#endif

#if ! RUN_TIME
   EnvDefineFunction2(theEnv,"get-defrule-list",'m',PTIEF GetDefruleListFunction,"GetDefruleListFunction","01w");
   EnvDefineFunction2(theEnv,"undefrule",'v',PTIEF UndefruleCommand,"UndefruleCommand","11w");
   EnvDefineFunction2(theEnv,"defrule-module",'w',PTIEF DefruleModuleFunction,"DefruleModuleFunction","11w");

#if DEBUGGING_FUNCTIONS
   EnvDefineFunction2(theEnv,"rules",'v', PTIEF ListDefrulesCommand,"ListDefrulesCommand","01w");
   EnvDefineFunction2(theEnv,"list-defrules",'v', PTIEF ListDefrulesCommand,"ListDefrulesCommand","01w");
   EnvDefineFunction2(theEnv,"ppdefrule",'v',PTIEF PPDefruleCommand,"PPDefruleCommand","11w");
#endif

#if (BLOAD || BLOAD_ONLY || BLOAD_AND_BSAVE)
   DefruleBinarySetup(theEnv);
#endif

#if CONSTRUCT_COMPILER && (! RUN_TIME)
   DefruleCompilerSetup(theEnv);
#endif

#endif
  }

/*****************************************************/
/* ResetDefrules: Defrule reset routine for use with */
/*   the reset command. Sets the current entity time */
/*   tag (used by the conflict resolution strategies */
/*   for recency) to zero. The focus stack is also   */
/*   cleared.                                        */
/*****************************************************/
static void ResetDefrules(
  void *theEnv)
  {
   struct defmodule *theModule;
   struct joinLink *theLink;
   struct partialMatch *notParent;
  
   DefruleData(theEnv)->CurrentEntityTimeTag = 1L;
   EnvClearFocusStack(theEnv);
   theModule = (struct defmodule *) EnvFindDefmodule(theEnv,"MAIN");
   EnvFocus(theEnv,(void *) theModule);
   
   for (theLink = DefruleData(theEnv)->RightPrimeJoins;
        theLink != NULL;
        theLink = theLink->next)
     { PosEntryRetractAlpha(theEnv,theLink->join->rightMemory->beta[0]); }

   for (theLink = DefruleData(theEnv)->LeftPrimeJoins;
        theLink != NULL;
        theLink = theLink->next)
     { 
      if ((theLink->join->patternIsNegated || theLink->join->joinFromTheRight) && 
          (! theLink->join->patternIsExists))
        {
         notParent = theLink->join->leftMemory->beta[0];
         
         if (notParent->marker)
           { RemoveBlockedLink(notParent); }
           
         /*==========================================================*/
         /* Prevent any retractions from generating partial matches. */
         /*==========================================================*/
           
         notParent->marker = notParent;
         
         if (notParent->children != NULL)
           { PosEntryRetractBeta(theEnv,notParent,notParent->children); }
           /*
         if (notParent->dependents != NULL) 
           { RemoveLogicalSupport(theEnv,notParent); } */
        }
     }
  }

/*****************************************************/
/* ResetDefrulesPrime:                      */
/*****************************************************/
static void ResetDefrulesPrime(
  void *theEnv)
  {
   struct joinLink *theLink;
   struct partialMatch *notParent;
      
   for (theLink = DefruleData(theEnv)->RightPrimeJoins;
        theLink != NULL;
        theLink = theLink->next)
     { NetworkAssert(theEnv,theLink->join->rightMemory->beta[0],theLink->join); }

   for (theLink = DefruleData(theEnv)->LeftPrimeJoins;
        theLink != NULL;
        theLink = theLink->next)
     { 
      if ((theLink->join->patternIsNegated || theLink->join->joinFromTheRight) && 
          (! theLink->join->patternIsExists))
        {
         notParent = theLink->join->leftMemory->beta[0];

         if (theLink->join->secondaryNetworkTest != NULL)
           {
            if (EvaluateSecondaryNetworkTest(theEnv,notParent,theLink->join) == FALSE)
              { continue; }
           }

         notParent->marker = NULL;

         EPMDrive(theEnv,notParent,theLink->join);
        }
     }

  }

#if (! RUN_TIME)

/******************************************************************/
/* ClearDefrulesReady: Indicates whether defrules can be cleared. */
/******************************************************************/
static int ClearDefrulesReady(
  void *theEnv)
  {
   if (EngineData(theEnv)->ExecutingRule != NULL) return(FALSE);

   if (EngineData(theEnv)->JoinOperationInProgress) return(FALSE);

   EnvClearFocusStack(theEnv);
   if (EnvGetCurrentModule(theEnv) == NULL) return(FALSE);

   DefruleData(theEnv)->CurrentEntityTimeTag = 1L;

   return(TRUE);
  }

/***************************************************************/
/* ClearDefrules: Pushes the MAIN module as the current focus. */
/***************************************************************/
static void ClearDefrules(
  void *theEnv)
  {
   struct defmodule *theModule;

   theModule = (struct defmodule *) EnvFindDefmodule(theEnv,"MAIN");
   EnvFocus(theEnv,(void *) theModule);
  }

#endif

/**************************************/
/* SaveDefrules: Defrule save routine */
/*   for use with the save command.   */
/**************************************/
static void SaveDefrules(
  void *theEnv,
  void *theModule,
  const char *logicalName)
  {
   SaveConstruct(theEnv,theModule,logicalName,DefruleData(theEnv)->DefruleConstruct); 
  }

/******************************************/
/* UndefruleCommand: H/L access routine   */
/*   for the undefrule command.           */
/******************************************/
globle void UndefruleCommand(
  void *theEnv)
  { 
   UndefconstructCommand(theEnv,"undefrule",DefruleData(theEnv)->DefruleConstruct); 
  }

/**********************************/
/* EnvUndefrule: C access routine */
/*   for the undefrule command.   */
/**********************************/
globle intBool EnvUndefrule(
  void *theEnv,
  void *theDefrule)
  {
   return(Undefconstruct(theEnv,theDefrule,DefruleData(theEnv)->DefruleConstruct)); 
  }

/************************************************/
/* GetDefruleListFunction: H/L access routine   */
/*   for the get-defrule-list function.         */
/************************************************/
globle void GetDefruleListFunction(
  void *theEnv,
  DATA_OBJECT_PTR returnValue)
  {
   GetConstructListFunction(theEnv,"get-defrule-list",returnValue,DefruleData(theEnv)->DefruleConstruct); 
  }

/****************************************/
/* EnvGetDefruleList: C access routine  */
/*   for the get-defrule-list function. */
/****************************************/
globle void EnvGetDefruleList(
  void *theEnv,
  DATA_OBJECT_PTR returnValue,
  void *theModule)
  {
   GetConstructList(theEnv,returnValue,DefruleData(theEnv)->DefruleConstruct,(struct defmodule *) theModule);
  }

/*********************************************/
/* DefruleModuleFunction: H/L access routine */
/*   for the defrule-module function.        */
/*********************************************/
globle void *DefruleModuleFunction(
  void *theEnv)
  {
   return(GetConstructModuleCommand(theEnv,"defrule-module",DefruleData(theEnv)->DefruleConstruct)); 
  }

#if DEBUGGING_FUNCTIONS

/******************************************/
/* PPDefruleCommand: H/L access routine   */
/*   for the ppdefrule command.           */
/******************************************/
globle void PPDefruleCommand(
  void *theEnv)
  {
   PPConstructCommand(theEnv,"ppdefrule",DefruleData(theEnv)->DefruleConstruct);
  }

/***********************************/
/* PPDefrule: C access routine for */
/*   the ppdefrule command.        */
/***********************************/
globle int PPDefrule(
  void *theEnv,
  const char *defruleName,
  const char *logicalName)
  {
   return(PPConstruct(theEnv,defruleName,logicalName,DefruleData(theEnv)->DefruleConstruct));
  }

/*********************************************/
/* ListDefrulesCommand: H/L access routine   */
/*   for the list-defrules command.          */
/*********************************************/
globle void ListDefrulesCommand(
  void *theEnv)
  {
   ListConstructCommand(theEnv,"list-defrules",DefruleData(theEnv)->DefruleConstruct); 
  }

/*************************************/
/* EnvListDefrules: C access routine */
/*   for the list-defrules command.  */
/*************************************/
globle void EnvListDefrules(
  void *theEnv,
  const char *logicalName,
  void *theModule)
  {
   ListConstruct(theEnv,DefruleData(theEnv)->DefruleConstruct,logicalName,(struct defmodule *) theModule); 
  }

/*******************************************************/
/* EnvGetDefruleWatchActivations: C access routine for */
/*   retrieving the current watch value of a defrule's */
/*   activations.                                      */
/*******************************************************/
globle unsigned EnvGetDefruleWatchActivations(
  void *theEnv,
  void *rulePtr)
  {
   struct defrule *thePtr;
#if MAC_XCD
#pragma unused(theEnv)
#endif

   for (thePtr = (struct defrule *) rulePtr;
        thePtr != NULL;
        thePtr = thePtr->disjunct)
     { if (thePtr->watchActivation) return(TRUE); }

   return(FALSE);
  }

/***********************************************/
/* EnvGetDefruleWatchFirings: C access routine */
/*   for retrieving the current watch value of */
/*   a defrule's firings.                      */
/***********************************************/
globle unsigned EnvGetDefruleWatchFirings(
  void *theEnv,
  void *rulePtr)
  {
   struct defrule *thePtr;
#if MAC_XCD
#pragma unused(theEnv)
#endif

   for (thePtr = (struct defrule *) rulePtr;
        thePtr != NULL;
        thePtr = thePtr->disjunct)
     { if (thePtr->watchFiring) return(TRUE); }

   return(FALSE);
  }

/***************************************************/
/* EnvSetDefruleWatchActivations: C access routine */
/*   for setting the current watch value of a      */
/*   defrule's activations.                        */
/***************************************************/
globle void EnvSetDefruleWatchActivations(
  void *theEnv,
  unsigned newState,
  void *rulePtr)
  {
   struct defrule *thePtr;
#if MAC_XCD
#pragma unused(theEnv)
#endif

   for (thePtr = (struct defrule *) rulePtr;
        thePtr != NULL;
        thePtr = thePtr->disjunct)
     { thePtr->watchActivation = newState; }
  }

/****************************************************/
/* EnvSetDefruleWatchFirings: C access routine for  */
/*   setting the current watch value of a defrule's */ 
/*   firings.                                       */
/****************************************************/
globle void EnvSetDefruleWatchFirings(
  void *theEnv,
  unsigned newState,
  void *rulePtr)
  {
   struct defrule *thePtr;
#if MAC_XCD
#pragma unused(theEnv)
#endif

   for (thePtr = (struct defrule *) rulePtr;
        thePtr != NULL;
        thePtr = thePtr->disjunct)
     { thePtr->watchFiring = newState; }
  }

/*******************************************************************/
/* DefruleWatchAccess: Access function for setting the watch flags */
/*   associated with rules (activations and rule firings).         */
/*******************************************************************/
globle unsigned DefruleWatchAccess(
  void *theEnv,
  int code,
  unsigned newState,
  struct expr *argExprs)
  {
   if (code)
     return(ConstructSetWatchAccess(theEnv,DefruleData(theEnv)->DefruleConstruct,newState,argExprs,
                                    EnvGetDefruleWatchActivations,EnvSetDefruleWatchActivations));
   else
     return(ConstructSetWatchAccess(theEnv,DefruleData(theEnv)->DefruleConstruct,newState,argExprs,
                                    EnvGetDefruleWatchFirings,EnvSetDefruleWatchFirings));
  }

/*****************************************************************/
/* DefruleWatchPrint: Access routine for printing which defrules */
/*   have their watch flag set via the list-watch-items command. */
/*****************************************************************/
globle unsigned DefruleWatchPrint(
  void *theEnv,
  const char *logName,
  int code,
  struct expr *argExprs)
  {   
   if (code)
     return(ConstructPrintWatchAccess(theEnv,DefruleData(theEnv)->DefruleConstruct,logName,argExprs,
                                      EnvGetDefruleWatchActivations,EnvSetDefruleWatchActivations));
   else
     return(ConstructPrintWatchAccess(theEnv,DefruleData(theEnv)->DefruleConstruct,logName,argExprs,
                                      EnvGetDefruleWatchActivations,EnvSetDefruleWatchActivations));
  }

#endif /* DEBUGGING_FUNCTIONS */

/*#####################################*/
/* ALLOW_ENVIRONMENT_GLOBALS Functions */
/*#####################################*/

#if ALLOW_ENVIRONMENT_GLOBALS

globle void GetDefruleList(
  DATA_OBJECT_PTR returnValue,
  void *theModule)
  {
   EnvGetDefruleList(GetCurrentEnvironment(),returnValue,theModule);
  }

#if DEBUGGING_FUNCTIONS

globle unsigned GetDefruleWatchActivations(
  void *rulePtr)
  {
   return EnvGetDefruleWatchActivations(GetCurrentEnvironment(),rulePtr);
  }

globle unsigned GetDefruleWatchFirings(
  void *rulePtr)
  {
   return EnvGetDefruleWatchFirings(GetCurrentEnvironment(),rulePtr);
  }

globle void ListDefrules(
  const char *logicalName,
  void *theModule)
  {
   EnvListDefrules(GetCurrentEnvironment(),logicalName,theModule);
  }

globle void SetDefruleWatchActivations(
  unsigned newState,
  void *rulePtr)
  {
   EnvSetDefruleWatchActivations(GetCurrentEnvironment(),newState,rulePtr);
  }

globle void SetDefruleWatchFirings(
  unsigned newState,
  void *rulePtr)
  {
   EnvSetDefruleWatchFirings(GetCurrentEnvironment(),newState,rulePtr);
  }

#endif

globle intBool Undefrule(
  void *theDefrule)
  {
   return EnvUndefrule(GetCurrentEnvironment(),theDefrule);
  }

#endif /* ALLOW_ENVIRONMENT_GLOBALS */

#endif /* DEFTEMPLATE_CONSTRUCT */


