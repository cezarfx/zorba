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

#include "diagnostics/assert.h"

#include "runtime/visitors/planiter_visitor.h"
#include "runtime/core/gflwor/let_iterator.h"
#include "runtime/core/var_iterators.h"
#include "runtime/core/gflwor/common.h"

namespace zorba 
{
namespace flwor 
{
SERIALIZABLE_CLASS_VERSIONS(LetIterator)



LetIterator::LetIterator (
    static_context* sctx,
    const QueryLoc& aLoc,
    store::Item* aVarName, 
    PlanIter_t aTupleIter,
    PlanIter_t aInput,
    const std::vector<PlanIter_t>& aLetVars,
    bool lazyEval,
    bool aNeedsMaterialization ) 
  :
  BinaryBaseIterator<LetIterator, PlanIteratorState>(sctx, aLoc, aTupleIter, aInput),
  theVarName(aVarName),
  theLetVars(aLetVars),
  theLazyEval(lazyEval), 
  theNeedsMat(aNeedsMaterialization),
  theSingleItemLETVar(false)
{
}


LetIterator::~LetIterator() 
{
}


void LetIterator::serialize(::zorba::serialization::Archiver& ar)
{
  serialize_baseclass(ar,
  (BinaryBaseIterator<LetIterator, PlanIteratorState>*)this);

  ar & theVarName;
  ar & theLetVars;
  ar & theLazyEval;
  ar & theNeedsMat;
  ar & theSingleItemLETVar;
}


void LetIterator::accept(PlanIterVisitor& v) const 
{ 
  v.beginVisit(*this); 

  v.beginVisitFlworLetVariable(theNeedsMat, 
                               theVarName->getStringValue(),
                               theLetVars);
  v.endVisitFlworLetVariable();

  theChild0->accept(v); 
  theChild1->accept(v); 

  v.endVisit(*this);
}


zstring LetIterator::getNameAsString() const {
  return "LetIterator";
}


bool LetIterator::nextImpl(store::Item_t& result, PlanState& planState) const 
{
  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  while (consumeNext(result, theChild0, planState)) 
  {
    bindVariables(theChild1,
                  theLetVars,
                  planState,
                  theLazyEval,
                  theNeedsMat,
                  theSingleItemLETVar);

    STACK_PUSH(true, state);

    theChild1->reset(planState);
  }
  STACK_END(state);
}


} //Namespace flwor
}//Namespace zorba
/* vim:set et sw=2 ts=2: */
