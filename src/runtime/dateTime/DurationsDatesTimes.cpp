/*
 * Copyright 2006-2008 The FLWOR Foundation.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "store/api/item.h"

#include "zorbaerrors/Assert.h"

#include "types/casting.h"
#include "context/dynamic_context.h"

#include "runtime/dateTime/DurationsDatesTimes.h"
#include "runtime/core/arithmetic_impl.h"
#include "runtime/api/runtimecb.h"
#include "zorbaerrors/error_manager.h"
#include "system/globalenv.h"
#include "store/api/item_factory.h"
#include "store/api/store.h"
#include "zorbatypes/datetime.h"
#include "zorbatypes/duration.h"

using namespace std;

namespace zorba
{

bool FnDateTimeConstructorIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  store::Item_t item0;
  store::Item_t item1;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  if (!consumeNext(item0, theChild0.getp(), planState) || !consumeNext(item1, theChild1.getp(), planState))
    STACK_PUSH(false, state);
  else
    STACK_PUSH(GENV_ITEMFACTORY->createDateTime(result, item0, item1), state);
     
  STACK_END (state);
}

/**
 *______________________________________________________________________
 *
 * 10.5.1 fn:years-from-duration
 *
 * fn:years-from-duration($arg as xs:duration?) as xs:integer?
 *_______________________________________________________________________*/

/*begin class FnYearsFromDurationIterator */
bool
FnYearsFromDurationIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  store::Item_t itemArg;
  int32_t lYears;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  if (consumeNext(itemArg, theChildren[0].getp(), planState))
  {
    itemArg = itemArg->getAtomizationValue();
    lYears = itemArg->getDurationValue()->getYears();
    STACK_PUSH( GENV_ITEMFACTORY->createInteger(
      result,
      Integer::parseInt(lYears)), 
      state 
    );
  }
  STACK_END (state);
}
/*end class FnYearsFromDurationIterator */

/**
 *______________________________________________________________________
 *
 * 10.5.2 fn:months-from-duration
 *
 * fn:months-from-duration($arg as xs:duration?) as xs:integer?
 *_______________________________________________________________________*/

/*begin class FnMonthsFromDurationIterator */
bool
FnMonthsFromDurationIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  store::Item_t itemArg;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  if (consumeNext(itemArg, theChildren[0].getp(), planState))
  {
    itemArg = itemArg->getAtomizationValue();
    STACK_PUSH( GENV_ITEMFACTORY->createInteger(
      result,
      Integer::parseInt(itemArg->getDurationValue()->getMonths())), 
      state 
    );
  }
  STACK_END (state);
}
/*end class FnMonthsFromDurationIterator */

/**
 *______________________________________________________________________
 *
 * 10.5.3 fn:days-from-duration
 *
 * fn:days-from-duration($arg as xs:duration?) as xs:integer?
 *_______________________________________________________________________*/

/*begin class FnDaysFromDurationIterator */
bool
FnDaysFromDurationIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  store::Item_t itemArg;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  if (consumeNext(itemArg, theChildren[0].getp(), planState))
  {
    itemArg = itemArg->getAtomizationValue();
    STACK_PUSH( GENV_ITEMFACTORY->createInteger(
      result,
      Integer::parseInt(itemArg->getDurationValue()->getDays())), 
      state 
    );
  }
  STACK_END (state);
}
/*end class FnDaysFromDurationIterator */

/**
 *______________________________________________________________________
 *
 * 10.5.4 fn:hours-from-duration
 *
 * fn:hours-from-duration($arg as xs:duration?) as xs:integer?
 *_______________________________________________________________________*/

/*begin class FnHoursFromDurationIterator */
bool
FnHoursFromDurationIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  store::Item_t itemArg;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  if (consumeNext(itemArg, theChildren[0].getp(), planState))
  {
    itemArg = itemArg->getAtomizationValue();
    STACK_PUSH( GENV_ITEMFACTORY->createInteger(
      result,
      Integer::parseInt(itemArg->getDurationValue()->getHours())), 
      state 
    );
  }
  STACK_END (state);
}
/*end class FnHoursFromDurationIterator */

/**
 *______________________________________________________________________
 *
 * 10.5.5 fn:minutes-from-duration
 *
 * fn:minutes-from-duration($arg as xs:duration?) as xs:integer?
 *_______________________________________________________________________*/

/*begin class FnMinutesFromDurationIterator */
bool
FnMinutesFromDurationIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  store::Item_t itemArg;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  if (consumeNext(itemArg, theChildren[0].getp(), planState))
  {
    itemArg = itemArg->getAtomizationValue();
    STACK_PUSH( GENV_ITEMFACTORY->createInteger(
      result,
      Integer::parseInt(itemArg->getDurationValue()->getMinutes())), 
      state 
    );
  }
  STACK_END (state);
}
/*end class FnMinutesFromDurationIterator */

/**
 *______________________________________________________________________
 *
 * 10.5.6 fn:seconds-from-duration
 *
 * fn:seconds-from-duration($arg as xs:duration?) as xs:decimal?
 *_______________________________________________________________________*/

/*begin class FnSecondsFromDurationIterator */
bool
FnSecondsFromDurationIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  store::Item_t itemArg;
  xqp_decimal lDecimal;
  bool lBool;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  if (consumeNext(itemArg, theChildren[0].getp(), planState))
  {
    itemArg = itemArg->getAtomizationValue();
    lBool = Decimal::parseNativeDouble(itemArg->getDurationValue()->getSeconds(), lDecimal);
    ZORBA_ASSERT(lBool);
    STACK_PUSH( GENV_ITEMFACTORY->createDecimal(result, lDecimal), state );
  }
  STACK_END (state);
}
/*end class FnSecondsFromDurationIterator */

/**
 *______________________________________________________________________
 *
 * 10.5.7 fn:year-from-dateTime
 *
 * fn:year-from-dateTime($arg as xs:dateTime?) as xs:integer?
 *_______________________________________________________________________*/

/*begin class FnYearFromDatetimeIterator */
bool
FnYearFromDatetimeIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  store::Item_t itemArg;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  if (consumeNext(itemArg, theChildren[0].getp(), planState))
  {
    itemArg = itemArg->getAtomizationValue();
    STACK_PUSH(
        GENV_ITEMFACTORY->createInteger(
          result,
          Integer::parseInt(itemArg->getDateTimeValue()->getYear())), state );
  }
  STACK_END (state);
}
/*end class FnYearFromDatetimeIterator */

/**
 *______________________________________________________________________
 *
 * 10.5.8 fn:month-from-dateTime
 *
 * fn:month-from-dateTime($arg as xs:dateTime?) as xs:integer?
 *_______________________________________________________________________*/

/*begin class FnMonthFromDatetimeIterator */
bool
FnMonthFromDatetimeIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  store::Item_t itemArg;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  if (consumeNext(itemArg, theChildren[0].getp(), planState))
  {
    itemArg = itemArg->getAtomizationValue();
    STACK_PUSH( GENV_ITEMFACTORY->createInteger(
      result,
      Integer::parseInt(itemArg->getDateTimeValue()->getMonth())), 
      state 
    );
  }
  STACK_END (state);
}
/*end class FnMonthFromDatetimeIterator */

/**
 *______________________________________________________________________
 *
 * 10.5.9 fn:day-from-dateTime
 *
 * fn:day-from-dateTime($arg as xs:dateTime?) as xs:integer?
 *_______________________________________________________________________*/

/*begin class FnDayFromDatetimeIterator */
bool
FnDayFromDatetimeIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  store::Item_t itemArg;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  if (consumeNext(itemArg, theChildren[0].getp(), planState))
  {
    itemArg = itemArg->getAtomizationValue();
    STACK_PUSH( GENV_ITEMFACTORY->createInteger(
      result,
      Integer::parseInt(itemArg->getDateTimeValue()->getDay())), 
      state 
    );
  }
  STACK_END (state);
}
/*end class FnDayFromDatetimeIterator */

/**
 *______________________________________________________________________
 *
 * 10.5.10 fn:hours-from-dateTime
 *
 * fn:hours-from-dateTime($arg as xs:dateTime?) as xs:integer?
 *_______________________________________________________________________*/

/*begin class FnHoursFromDatetimeIterator */
bool
FnHoursFromDatetimeIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  store::Item_t itemArg;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  if (consumeNext(itemArg, theChildren[0].getp(), planState))
  {
    itemArg = itemArg->getAtomizationValue();
    STACK_PUSH( GENV_ITEMFACTORY->createInteger(
      result,
      Integer::parseInt(itemArg->getDateTimeValue()->getHours())), 
      state 
    );
  }
  STACK_END (state);
}
/*end class FnHoursFromDatetimeIterator */

/**
 *______________________________________________________________________
 *
 * 10.5.11 fn:minutes-from-dateTime
 *
 * fn:minutes-from-dateTime($arg as xs:dateTime?) as xs:integer?
 *_______________________________________________________________________*/

/*begin class FnMinutesFromDatetimeIterator */
bool
FnMinutesFromDatetimeIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  store::Item_t itemArg;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  if (consumeNext(itemArg, theChildren[0].getp(), planState))
  {
    itemArg = itemArg->getAtomizationValue();
    STACK_PUSH( GENV_ITEMFACTORY->createInteger(
      result,
      Integer::parseInt(itemArg->getDateTimeValue()->getMinutes())), 
      state 
    );
  }
  STACK_END (state);
}
/*end class FnMinutesFromDatetimeIterator */

/**
 *______________________________________________________________________
 *
 * 10.5.12 fn:seconds-from-dateTime
 *
 * fn:seconds-from-dateTime($arg as xs:dateTime?) as xs:decimal?
 *_______________________________________________________________________*/

/*begin class FnSecondsFromDatetimeIterator */
bool
FnSecondsFromDatetimeIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  store::Item_t itemArg;
  xqp_decimal lDecimal;
  bool lBool;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  if (consumeNext(itemArg, theChildren[0].getp(), planState))
  {
    itemArg = itemArg->getAtomizationValue();
    lBool = Decimal::parseNativeDouble(itemArg->getDateTimeValue()->getSeconds(), lDecimal);
    ZORBA_ASSERT(lBool);
    STACK_PUSH( GENV_ITEMFACTORY->createDecimal(result, lDecimal), state );
  }
  STACK_END (state);
}
/*end class FnSecondsFromDatetimeIterator */

/**
 *______________________________________________________________________
 *
 * 10.5.13 fn:timezone-from-dateTime
 *
 * fn:timezone-from-dateTime($arg as xs:dateTime?) as xs:dayTimeDuration?
 *_______________________________________________________________________*/

/*begin class FnTimezoneFromDatetimeIterator */
bool
FnTimezoneFromDatetimeIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  store::Item_t itemArg;
  xqp_duration tmpDuration;
  bool res = false;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  if (consumeNext(itemArg, theChildren[0].getp(), planState))
  {
    itemArg = itemArg->getAtomizationValue();
    res = DayTimeDuration::from_Timezone(itemArg->getDateTimeValue()->getTimezone(), tmpDuration);
    if(res)
      STACK_PUSH( GENV_ITEMFACTORY->createDuration(result, tmpDuration), state );
  }
  STACK_END (state);
}
/*end class FnTimezoneFromDatetimeIterator */

/**
 *______________________________________________________________________
 *
 * 10.5.14 fn:year-from-date
 *
 * fn:year-from-date($arg as xs:date?) as xs:integer?
 *_______________________________________________________________________*/

/*begin class FnYearFromDateIterator */
bool
FnYearFromDateIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  store::Item_t itemArg;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  if (consumeNext(itemArg, theChildren[0].getp(), planState))
  {
    itemArg = itemArg->getAtomizationValue();
    STACK_PUSH( GENV_ITEMFACTORY->createInteger(
      result,
      Integer::parseInt(itemArg->getDateValue()->getYear())), 
      state 
    );
  }
  STACK_END (state);
}
/*end class FnYearFromDateIterator */

/**
 *______________________________________________________________________
 *
 * 10.5.15 fn:month-from-date
 *
 * fn:month-from-date($arg as xs:date?) as xs:integer?
 *_______________________________________________________________________*/

/*begin class FnMonthFromDateIterator */
bool
FnMonthFromDateIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  store::Item_t itemArg;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  if (consumeNext(itemArg, theChildren[0].getp(), planState))
  {
    itemArg = itemArg->getAtomizationValue();
    STACK_PUSH( GENV_ITEMFACTORY->createInteger(
      result,
      Integer::parseInt(itemArg->getDateValue()->getMonth())), 
      state 
    );
  }
  STACK_END (state);
}
/*end class FnMonthFromDateIterator */

/**
 *______________________________________________________________________
 *
 * 10.5.16 fn:day-from-date
 *
 * fn:day-from-date($arg as xs:date?) as xs:integer?
 *_______________________________________________________________________*/

/*begin class FnDayFromDateIterator */
bool
FnDayFromDateIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  store::Item_t itemArg;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  if (consumeNext(itemArg, theChildren[0].getp(), planState))
  {
    itemArg = itemArg->getAtomizationValue();
    STACK_PUSH( GENV_ITEMFACTORY->createInteger(
      result,
      Integer::parseInt(itemArg->getDateValue()->getDay())), 
      state 
    );
  }
  STACK_END (state);
}
/*end class FnDayFromDateIterator */

/**
 *______________________________________________________________________
 *
 * 10.5.17 fn:timezone-from-date
 *
 * fn:timezone-from-date($arg as xs:date?) as xs:dayTimeDuration?
 *_______________________________________________________________________*/

/*begin class FnTimezoneFromDateIterator */
bool
FnTimezoneFromDateIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  store::Item_t itemArg;
  xqp_duration tmpDuration;
  bool res = false;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  if (consumeNext(itemArg, theChildren[0].getp(), planState))
  {
    itemArg = itemArg->getAtomizationValue();
    res = DayTimeDuration::from_Timezone(itemArg->getDateValue()->getTimezone(), tmpDuration);
    if(res)
      STACK_PUSH( GENV_ITEMFACTORY->createDuration(result, tmpDuration), state );
  }
  STACK_END (state);
}
/*end class FnTimezoneFromDateIterator */

/**
 *______________________________________________________________________
 *
 * 10.5.18 fn:hours-from-time
 *
 * fn:hours-from-time($arg as xs:time?) as xs:integer?
 *_______________________________________________________________________*/

/*begin class FnHoursFromTimeIterator */
bool
FnHoursFromTimeIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  store::Item_t itemArg;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  if (consumeNext(itemArg, theChildren[0].getp(), planState))
  {
    itemArg = itemArg->getAtomizationValue();
    STACK_PUSH( GENV_ITEMFACTORY->createInteger(
      result,
      Integer::parseInt(itemArg->getTimeValue()->getHours())), 
      state 
    );
  }
  STACK_END (state);
}
/*end class FnHoursFromTimeIterator */

/**
 *______________________________________________________________________
 *
 * 10.5.19 fn:minutes-from-time
 *
 * fn:minutes-from-time($arg as xs:time?) as xs:integer?
 *_______________________________________________________________________*/

/*begin class FnMinutesFromTimeIterator */
bool
FnMinutesFromTimeIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  store::Item_t itemArg;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  if (consumeNext(itemArg, theChildren[0].getp(), planState))
  {
    itemArg = itemArg->getAtomizationValue();
    STACK_PUSH( GENV_ITEMFACTORY->createInteger(
      result,
      Integer::parseInt(itemArg->getTimeValue()->getMinutes())), 
      state 
    );
  }
  STACK_END (state);
}
/*end class FnMinutesFromTimeIterator */


/**
 *______________________________________________________________________
 *
 * 10.5.20 fn:seconds-from-time
 *
 * fn:seconds-from-time($arg as xs:time?) as xs:decimal?
 *_______________________________________________________________________*/

/*begin class FnSecondsFromTimeIterator */
bool
FnSecondsFromTimeIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  store::Item_t itemArg;
  xqp_decimal lDecimal;
  bool lBool;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  if (consumeNext(itemArg, theChildren[0].getp(), planState))
  {
    itemArg = itemArg->getAtomizationValue();
    lBool = Decimal::parseNativeDouble(itemArg->getTimeValue()->getSeconds(), lDecimal);
    ZORBA_ASSERT(lBool);
    STACK_PUSH( GENV_ITEMFACTORY->createDecimal(result, lDecimal), state );
  }
  STACK_END (state);
}
/*end class FnSecondsFromTimeIterator */



/**
 *______________________________________________________________________
 *
 * 10.5.21 fn:timezone-from-time
 *
 * fn:timezone-from-time($arg as xs:time?) as xs:dayTimeDuration?
 *_______________________________________________________________________*/

/*begin class FnTimezoneFromTimeIterator */
bool
FnTimezoneFromTimeIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  store::Item_t itemArg;
  xqp_duration tmpDuration;
  bool res = false;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  if (consumeNext(itemArg, theChildren[0].getp(), planState))
  if ( itemArg != NULL )
  {
    itemArg = itemArg->getAtomizationValue();
    res = DayTimeDuration::from_Timezone(itemArg->getTimeValue()->getTimezone(), tmpDuration);
    if(res)
      STACK_PUSH( GENV_ITEMFACTORY->createDuration(result, tmpDuration), state );
  }
  STACK_END (state);
}
/*end class FnTimezoneFromTimeIterator */

/**
 *______________________________________________________________________
 *
 * 10.6.1 op:add-yearMonthDurations
 *
 * op:add-yearMonthDurations(   $arg1    as xs:yearMonthDuration,
 *                                             $arg2    as xs:yearMonthDuration) as xs:yearMonthDuration
 *_______________________________________________________________________*/
 /**
 *______________________________________________________________________
 *
 * 10.6.6 op:add-dayTimeDurations
 *
 * op:add-dayTimeDurations(  $arg1    as xs:dayTimeDuration,
 *                                         $arg2    as xs:dayTimeDuration) as xs:dayTimeDuration
 *_______________________________________________________________________*/

 /**
 *______________________________________________________________________
 *
 * 10.6.2 op:subtract-yearMonthDurations
 *
 * op:subtract-yearMonthDurations(  $arg1    as xs:yearMonthDuration,
 *                                                  $arg2    as xs:yearMonthDuration) as xs:yearMonthDuration
 *_______________________________________________________________________*/
 /**
 *______________________________________________________________________
 *
 * 10.6.7 op:subtract-dayTimeDurations
 *
 * op:subtract-dayTimeDurations(  $arg1    as xs:dayTimeDuration,
 *                                               $arg2    as xs:dayTimeDuration) as xs:dayTimeDuration
 *_______________________________________________________________________*/

 /**
 *______________________________________________________________________
 *
 * 10.6.3 op:multiply-yearMonthDuration
 *
 * op:multiply-yearMonthDuration(   $arg1    as xs:yearMonthDuration,
 *                                                 $arg2    as xs:double) as xs:yearMonthDuration
 *_______________________________________________________________________*/
 /**
 *______________________________________________________________________
 *
 * 10.6.8 op:multiply-dayTimeDuration
 *
 * op:multiply-dayTimeDuration(   $arg1    as xs:dayTimeDuration,
 *                                              $arg2    as xs:double) as xs:dayTimeDuration
 *_______________________________________________________________________*/

  /**
 *______________________________________________________________________
 *
 * 10.6.4 op:divide-yearMonthDuration
 *
 * op:divide-yearMonthDuration(   $arg1    as xs:yearMonthDuration,
 *                                              $arg2    as xs:double) as xs:yearMonthDuration
 *_______________________________________________________________________*/
  /**
 *______________________________________________________________________
 *
 * 10.6.9 op:divide-dayTimeDuration
 *
 * op:divide-dayTimeDuration(   $arg1    as xs:dayTimeDuration,
 *                                            $arg2    as xs:double) as xs:dayTimeDuration
 *_______________________________________________________________________*/

 /**
 *______________________________________________________________________
 *
 * 10.6.5 op:divide-yearMonthDuration-by-yearMonthDuration
 *
 * op:divide-yearMonthDuration-by-yearMonthDuration(  $arg1    as xs:yearMonthDuration,
 *                                                                              $arg2    as xs:yearMonthDuration) as xs:decimal
 *_______________________________________________________________________*/

/**
 *______________________________________________________________________
 *
 * 10.6.10 op:divide-dayTimeDuration-by-dayTimeDuration
 *
 * op:divide-dayTimeDuration-by-dayTimeDuration(  $arg1    as xs:dayTimeDuration,
 *                                                                         $arg2    as xs:dayTimeDuration) as xs:decimal
 *_______________________________________________________________________*/
/* end class DivideOperations */

/**
 *______________________________________________________________________
 *
 * 10.7.1 fn:adjust-dateTime-to-timezone
 * 10.7.2 fn:adjust-date-to-timezone
 * 10.7.3 fn:adjust-time-to-timezone
 *
 * fn:adjust-dateTime-to-timezone($arg as xs:dateTime?, $timezone as xs:dayTimeDuration?) as xs:dateTime?
 *
 *_______________________________________________________________________*/
bool
FnAdjustToTimeZoneIterator_1::nextImpl(store::Item_t& result, PlanState& planState) const
{
  store::Item_t item0;
  store::Item_t item1;
  DateTime* dt = NULL;
  
  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  // If $arg is the empty sequence, then the result is the empty sequence.
  if (!consumeNext(item0, theChild.getp(), planState))
    STACK_PUSH(false, state);
  else
  {
    try 
    {
      dt = item0->getDateTimeValue()->adjustToTimeZone(
        planState.theRuntimeCB->theDynamicContext->get_implicit_timezone());
    }
    catch (InvalidTimezoneException)
    {
      ZORBA_ERROR(FODT0003);
    }
    STACK_PUSH(GENV_ITEMFACTORY->createDateTime(result, dt), state);
  }
    
  STACK_END (state);
}

bool
FnAdjustToTimeZoneIterator_2::nextImpl(store::Item_t& result, PlanState& planState) const
{
  store::Item_t item0;
  store::Item_t item1;
  bool s1;
  DateTime* dt = NULL;
  
  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  if (!consumeNext(item0, theChild0.getp(), planState))
    STACK_PUSH(false, state);
  else
  {
    s1 = consumeNext(item1, theChild1.getp(), planState);
    try 
    {
      dt = item0->getDateTimeValue()->adjustToTimeZone(!s1 ? NULL : item1->getDurationValue());
    }
    catch (InvalidTimezoneException)
    {
      ZORBA_ERROR(FODT0003);
    }
    STACK_PUSH(GENV_ITEMFACTORY->createDateTime(result, dt), state);
  }
     
  STACK_END (state);
}


}
