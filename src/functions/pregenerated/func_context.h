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
 
// ******************************************
// *                                        *
// * THIS IS A GENERATED FILE. DO NOT EDIT! *
// * SEE .xml FILE WITH SAME NAME           *
// *                                        *
// ******************************************


#ifndef ZORBA_FUNCTIONS_CONTEXT_H
#define ZORBA_FUNCTIONS_CONTEXT_H


#include "common/shared_types.h"
#include "functions/function_impl.h"


namespace zorba {


void populate_context_context(static_context* sctx);




//fn:current-dateTime
class fn_current_dateTime : public function
{
public:
  fn_current_dateTime(const signature& sig)
    :
    function(sig, FunctionConsts::FN_CURRENT_DATETIME_0)
  {
  }

  bool accessesDynCtx() const { return true; }

  CODEGEN_DECL();
};


//fn:current-date
class fn_current_date : public function
{
public:
  fn_current_date(const signature& sig)
    :
    function(sig, FunctionConsts::FN_CURRENT_DATE_0)
  {
  }

  bool accessesDynCtx() const { return true; }

  CODEGEN_DECL();
};


//fn:current-time
class fn_current_time : public function
{
public:
  fn_current_time(const signature& sig)
    :
    function(sig, FunctionConsts::FN_CURRENT_TIME_0)
  {
  }

  bool accessesDynCtx() const { return true; }

  CODEGEN_DECL();
};


//fn:implicit-timezone
class fn_implicit_timezone : public function
{
public:
  fn_implicit_timezone(const signature& sig)
    :
    function(sig, FunctionConsts::FN_IMPLICIT_TIMEZONE_0)
  {
  }

  bool accessesDynCtx() const { return true; }

  CODEGEN_DECL();
};


//fn:default-collation
class fn_default_collation : public function
{
public:
  fn_default_collation(const signature& sig)
    :
    function(sig, FunctionConsts::FN_DEFAULT_COLLATION_0)
  {
  }

  bool accessesDynCtx() const { return true; }

  CODEGEN_DECL();
};


} //namespace zorba


#endif
/*
 * Local variables:
 * mode: c++
 * End:
 */ 