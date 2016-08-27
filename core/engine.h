   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*             CLIPS Version 6.50  08/25/16            */
   /*                                                     */
   /*                 ENGINE HEADER FILE                  */
   /*******************************************************/

/*************************************************************/
/* Purpose: Provides functionality primarily associated with */
/*   the run and focus commands.                             */
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
/*            Corrected compilation errors for files         */
/*            generated by constructs-to-c. DR0861           */
/*                                                           */
/*      6.24: Removed DYNAMIC_SALIENCE, INCREMENTAL_RESET,   */
/*            and LOGICAL_DEPENDENCIES compilation flags.    */
/*                                                           */
/*            Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*            Added access functions to the HaltRules flag.  */
/*                                                           */
/*            Added EnvGetNextFocus, EnvGetFocusChanged, and */
/*            EnvSetFocusChanged functions.                  */
/*                                                           */
/*      6.30: Added additional developer statistics to help  */
/*            analyze join network performance.              */
/*                                                           */
/*            Removed pseudo-facts used in not CEs.          */
/*                                                           */
/*            Added context information for run functions.   */
/*                                                           */
/*            Added before rule firing callback function.    */ 
/*                                                           */
/*            Changed garbage collection algorithm.          */
/*                                                           */
/*            Changed integer type/precision.                */
/*                                                           */
/*            Added EnvHalt function.                        */
/*                                                           */
/*            Used gensprintf instead of sprintf.            */
/*                                                           */
/*            Removed conditional code for unsupported       */
/*            compilers/operating systems (IBM_MCW,          */
/*            MAC_MCW, and IBM_TBC).                         */
/*            Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*            Converted API macros to function calls.        */
/*                                                           */
/*      6.40: Removed LOCALE definition.                     */
/*                                                           */
/*            Pragma once and other inclusion changes.       */
/*                                                           */
/*            Added support for booleans with <stdbool.h>.   */
/*                                                           */
/*            Removed use of void pointers for specific      */
/*            data structures.                               */
/*                                                           */
/*            ALLOW_ENVIRONMENT_GLOBALS no longer supported. */
/*                                                           */
/*            Incremental reset is always enabled.           */
/*                                                           */
/*            UDF redesign.                                  */
/*                                                           */
/*************************************************************/

#ifndef _H_engine

#pragma once

#define _H_engine

#include "lgcldpnd.h"
#include "ruledef.h"
#include "network.h"
#include "moduldef.h"
#include "retract.h"

struct focus
  {
   Defmodule *theModule;
   struct defruleModule *theDefruleModule;
   struct focus *next;
  };
  
#define ENGINE_DATA 18

struct engineData
  { 
   Defrule *ExecutingRule;
   bool HaltRules;
   struct joinNode *TheLogicalJoin;
   struct partialMatch *TheLogicalBind;
   struct dependency *UnsupportedDataEntities;
   bool alreadyEntered;
   struct callFunctionItem *ListOfRunFunctions;
   struct callFunctionItemWithArg *ListOfBeforeRunFunctions;
   struct focus *CurrentFocus;
   bool FocusChanged;
#if DEBUGGING_FUNCTIONS
   bool WatchStatistics;
   bool WatchFocus;
#endif
   bool IncrementalResetInProgress;
   bool JoinOperationInProgress;
   struct partialMatch *GlobalLHSBinds;
   struct partialMatch *GlobalRHSBinds;
   struct joinNode *GlobalJoin;
   struct partialMatch *GarbagePartialMatches;
   struct alphaMatch *GarbageAlphaMatches;
   bool AlreadyRunning;
#if DEVELOPER
   long leftToRightComparisons;
   long rightToLeftComparisons;
   long leftToRightSucceeds;
   long rightToLeftSucceeds;
   long leftToRightLoops;
   long rightToLeftLoops;
   long findNextConflictingComparisons;
   long betaHashHTSkips;
   long betaHashListSkips;
   long unneededMarkerCompare;
#endif
  };

#define EngineData(theEnv) ((struct engineData *) GetEnvironmentData(theEnv,ENGINE_DATA))

#define MAX_PATTERNS_CHECKED 64

   long long               EnvRun(Environment *,long long);
   bool                    EnvAddRunFunction(Environment *,const char *,
                                             void (*)(Environment *),int);
   bool                    EnvAddRunFunctionWithContext(Environment *,const char *,
                                                        void (*)(Environment *),int,void *);
   bool                    EnvRemoveRunFunction(Environment *,const char *);
   bool                    EnvAddBeforeRunFunction(Environment *,const char *,
                                                   void (*)(Environment *,void *),int);
   bool                    EnvAddBeforeRunFunctionWithContext(Environment *,const char *,
                                                              void (*)(Environment *, void *),int,void *);
   bool                    EnvRemoveBeforeRunFunction(Environment *,const char *);
   void                    InitializeEngine(Environment *);
   void                    EnvSetBreak(Environment *,Defrule *);
   void                    EnvHalt(Environment *);
   bool                    EnvRemoveBreak(Environment *,Defrule *);
   void                    RemoveAllBreakpoints(Environment *);
   void                    EnvShowBreaks(Environment *,const char *,Defmodule *);
   bool                    EnvDefruleHasBreakpoint(Environment *,Defrule *);
   void                    RunCommand(Environment *,UDFContext *,CLIPSValue *);
   void                    SetBreakCommand(Environment *,UDFContext *,CLIPSValue *);
   void                    RemoveBreakCommand(Environment *,UDFContext *,CLIPSValue *);
   void                    ShowBreaksCommand(Environment *,UDFContext *,CLIPSValue *);
   void                    HaltCommand(Environment *,UDFContext *,CLIPSValue *);
   void                    FocusCommand(Environment *,UDFContext *,CLIPSValue *);
   void                    ClearFocusStackCommand(Environment *,UDFContext *,CLIPSValue *);
   void                    EnvClearFocusStack(Environment *);
   struct focus           *EnvGetNextFocus(Environment *,struct focus *);
   void                    EnvFocus(Environment *,Defmodule *);
   bool                    EnvGetFocusChanged(Environment *);
   void                    EnvSetFocusChanged(Environment *,bool);
   void                    ListFocusStackCommand(Environment *,UDFContext *,CLIPSValue *);
   void                    EnvListFocusStack(Environment *,const char *);
   void                    GetFocusStackFunction(Environment *,UDFContext *,CLIPSValue *);
   void                    EnvGetFocusStack(Environment *,CLIPSValue *);
   void                    PopFocusFunction(Environment *,UDFContext *,CLIPSValue *);
   void                    GetFocusFunction(Environment *,UDFContext *,CLIPSValue *);
   Defmodule              *EnvPopFocus(Environment *);
   Defmodule              *EnvGetFocus(Environment *);
   bool                    EnvGetHaltRules(Environment *);
   void                    EnvSetHaltRules(Environment *,bool);
   Activation             *NextActivationToFire(Environment *);

#endif /* _H_engine */






