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
#include "stdafx.h"

#include <vector>

#include <zorba/internal/unique_ptr.h>

#include "compiler/expression/fo_expr.h"
#include "compiler/expression/expr_visitor.h"
#include "compiler/expression/expr_manager.h"

#include "compiler/api/compilercb.h"

#include "context/static_context.h"

#include "functions/library.h"
#include "functions/function.h"
#include "functions/udf.h"

#include "diagnostics/assert.h"
#include "diagnostics/util_macros.h"
#include "diagnostics/xquery_diagnostics.h"

namespace zorba
{


void fo_expr::accept(expr_visitor& v)
{
  if (v.begin_visit(*this))
  {
    accept_children(v);
  }

  v.end_visit(*this);
}



/*******************************************************************************
  first-order expressions. Represents function invocations as well as:

  OrExpr, AndExpr, ComparisonExpr, RangeExpr, AdditiveExpr, MultiplicativeExpr,
  UnionExpr, and IntersectExceptExpr.
********************************************************************************/

fo_expr* fo_expr::create_seq(
    CompilerCB* ccb,
    static_context* sctx,
    user_function* udf,
    const QueryLoc& loc)
{
  function* f = GENV_FUNC_LIB->getFunction(FunctionConsts::OP_CONCATENATE_N);

  std::unique_ptr<fo_expr> fo(ccb->theEM->create_fo_expr(sctx, udf, loc, f));

  return fo.release();
}


fo_expr::fo_expr(
    CompilerCB* ccb,
    static_context* sctx,
    user_function* udf,
    const QueryLoc& loc,
    const function* f)
  :
  expr(ccb, sctx, udf, loc, fo_expr_kind),
  theFunction(const_cast<function*>(f))
{
  // This method is private and it is to be used only by the clone method
  assert(f != NULL);
  theScriptingKind = VACUOUS_EXPR;
}


fo_expr::fo_expr(
    CompilerCB* ccb,
    static_context* sctx,
    user_function* udf,
    const QueryLoc& loc,
    const function* f,
    expr* arg)
  :
  expr(ccb, sctx, udf, loc, fo_expr_kind),
  theFunction(const_cast<function*>(f))
{
  assert(f != NULL);
  theArgs.resize(1);
  theArgs[0] = arg;

  compute_scripting_kind();
}


fo_expr::fo_expr(
    CompilerCB* ccb,
    static_context* sctx,
    user_function* udf,
    const QueryLoc& loc,
    const function* f,
    expr* arg1,
    expr* arg2)
  :
  expr(ccb, sctx, udf, loc, fo_expr_kind),
  theFunction(const_cast<function*>(f))
{
  assert(f != NULL);
  theArgs.resize(2);
  theArgs[0] = arg1;
  theArgs[1] = arg2;

  compute_scripting_kind();
}


fo_expr::fo_expr(
    CompilerCB* ccb,
    static_context* sctx,
    user_function* udf,
    const QueryLoc& loc,
    const function* f,
    const std::vector<expr*>& args)
  :
  expr(ccb, sctx, udf, loc, fo_expr_kind),
  theArgs(args),
  theFunction(const_cast<function*>(f))
{
  assert(f != NULL);
  compute_scripting_kind();
}


const signature& fo_expr::get_signature() const
{
  return theFunction->getSignature();
}


const store::Item* fo_expr::get_fname() const
{
  return theFunction->getName();
}


void fo_expr::add_arg(expr* e)
{
  theArgs.push_back(e);
  compute_scripting_kind();
}


void fo_expr::add_args(const std::vector<expr*>& args)
{
  theArgs.insert(theArgs.end(), args.begin(), args.end());
  compute_scripting_kind();
}


void fo_expr::remove_arg(csize i)
{
  theArgs.erase(theArgs.begin() + i);
  compute_scripting_kind();
}


void fo_expr::compute_scripting_kind()
{
  const function* func = get_func();
  csize numArgs = num_args();

  switch (func->getKind())
  {
  case FunctionConsts::OP_CONCATENATE_N:
  {
    bool vacuous = true;

    theScriptingKind = VACUOUS_EXPR;

    for (csize i = 0; i < numArgs; ++i)
    {
      if (theArgs[i] == NULL)
        continue;

      short argKind = theArgs[i]->get_scripting_detail();

      if (argKind == VACUOUS_EXPR)
        continue;

      vacuous = false;

      if (!theSctx->is_feature_set(feature::scripting))
      {
        if (is_updating() && !(argKind & UPDATING_EXPR) && argKind != VACUOUS_EXPR)
        {
          RAISE_ERROR(err::XUST0001, theArgs[i]->get_loc(),
          ERROR_PARAMS(ZED(XUST0001_CONCAT)));
        }

        if (i > 0 && !is_updating() && !is_vacuous() && (argKind & UPDATING_EXPR))
        {
          RAISE_ERROR(err::XUST0001, theArgs[i]->get_loc(),
          ERROR_PARAMS(ZED(XUST0001_CONCAT)));
        }
      }

      theScriptingKind |= argKind;
    }

    if (!vacuous)
      theScriptingKind &= ~VACUOUS_EXPR;

    if (theScriptingKind & UPDATING_EXPR)
      theScriptingKind &= ~SIMPLE_EXPR;

    if (is_sequential(theScriptingKind))
      theScriptingKind &= ~SIMPLE_EXPR;

    checkScriptingKind();

    break;
  }
  case FunctionConsts::OP_HOIST_1:
  case FunctionConsts::OP_UNHOIST_1:
  {
    theScriptingKind = theArgs[0]->get_scripting_detail();
    break;
  }
  default:
  {
    theScriptingKind = func->getScriptingKind();

    bool vacuous = (theScriptingKind == VACUOUS_EXPR);

    for (csize i = 0; i < numArgs; ++i)
    {
      if (theArgs[i] == NULL)
        continue;

      expr* arg = theArgs[i];

      if (arg->is_updating())
      {
        RAISE_ERROR(err::XUST0001, theArgs[i]->get_loc(),
        ERROR_PARAMS(ZED(XUST0001_Generic)));
      }

      short argKind = arg->get_scripting_detail();

      if (arg->is_sequential())
      {
        vacuous = false;

        theScriptingKind |= argKind;
      }
    }

    if (!vacuous)
      theScriptingKind &= ~VACUOUS_EXPR;

    if (theScriptingKind & UPDATING_EXPR)
      theScriptingKind &= ~SIMPLE_EXPR;

    if (is_sequential(theScriptingKind))
      theScriptingKind &= ~SIMPLE_EXPR;

    checkScriptingKind();
  }
  }
}


}

/* vim:set et sw=2 ts=2: */
