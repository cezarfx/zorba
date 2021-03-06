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

#include <algorithm>

#include "diagnostics/assert.h"
#include "diagnostics/util_macros.h"
#include "diagnostics/xquery_diagnostics.h"
#include <zorba/internal/unique_ptr.h>

#include "zorbatypes/URI.h"
#include "zorbatypes/numconversions.h"

#include "system/globalenv.h"

#include "context/static_context.h"
#include "context/dynamic_context.h"
#include "context/uri_resolver.h"
#include "context/static_context_consts.h"

#include "compiler/xqddf/value_index.h"
#include "compiler/xqddf/value_ic.h"

#include "runtime/collections/collections.h"
#include "runtime/core/apply_updates.h"
#include "runtime/base/plan_iterator.h"
#include "runtime/visitors/planiter_visitor.h"

#include "store/api/pul.h"
#include "store/api/item.h"
#include "store/api/item_factory.h"
#include "store/api/iterator.h"
#include "store/api/ic.h"
#include "store/api/index.h"
#include "store/api/annotation.h"
#include "store/api/shared_types.h"
#include "types/typeops.h"
#include "types/root_typemanager.h"


namespace zorba {


/*******************************************************************************

********************************************************************************/
const StaticallyKnownCollection* getCollection(
    const static_context* sctx,
    const store::Item_t& name,
    const QueryLoc& loc,
    bool dynamic,
    store::Collection_t& coll)
{
  const StaticallyKnownCollection* collectionDecl = sctx->lookup_collection(name);

  if (collectionDecl == 0 && !dynamic)
  {
    RAISE_ERROR(zerr::ZDDY0001_COLLECTION_NOT_DECLARED, loc,
    ERROR_PARAMS(name->getStringValue()));
  }

  coll = GENV_STORE.getCollection(name, dynamic);

  if (coll == NULL)
  {
    RAISE_ERROR(zerr::ZDDY0003_COLLECTION_DOES_NOT_EXIST, loc,
    ERROR_PARAMS(name->getStringValue()));
  }

  return collectionDecl;
}


/*******************************************************************************
  fn:collection() as node()*
  fn:collection($uri as xs:string?) as node()*
********************************************************************************/
FnCollectionIteratorState::~FnCollectionIteratorState()
{
  if ( theIterator != NULL )
  {
    // closing the iterator is necessary here if an exception occurs
    // in the producer or if the iterator is not fully consumed
    if (theIteratorOpened)
    {
      theIterator->close();
      theIteratorOpened = false;
    }
    theIterator = NULL;
  }
}


void FnCollectionIteratorState::init(PlanState& planState)
{
  PlanIteratorState::init(planState);
  theIterator = NULL;
}


void FnCollectionIteratorState::reset(PlanState& planState)
{
  PlanIteratorState::reset(planState);

  if ( theIterator != NULL )
  {
    // closing the iterator is necessary here if an exception occurs
    // in the producer or if the iterator is not fully consumed
    if (theIteratorOpened)
    {
      theIterator->close();
      theIteratorOpened = false;
    }
    theIterator = NULL;
  }
}


bool FnCollectionIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  store::Collection_t coll;
  zstring lErrorMessage;

  FnCollectionIteratorState* state;
  DEFAULT_STACK_INIT(FnCollectionIteratorState, state, planState);

  coll = getCollection(planState);

  state->theIterator = coll->getIterator();
  ZORBA_ASSERT(state->theIterator!=NULL);
  state->theIterator->open();
  state->theIteratorOpened = true;

  while(state->theIterator->next(result))
  {
    STACK_PUSH(true, state);
  }

  // close as early as possible
  state->theIterator->close();
  state->theIteratorOpened = false;

  STACK_END(state);
}


bool FnCollectionIterator::count(store::Item_t& result, PlanState& planState) const
{
  store::Collection_t collection;
  xs_integer count;

  FnCollectionIteratorState* state;
  DEFAULT_STACK_INIT(FnCollectionIteratorState, state, planState);

  collection = getCollection(planState);

  count = collection->size();

  STACK_PUSH(GENV_ITEMFACTORY->createInteger(result, count), state);
  STACK_END(state);
}


store::Collection_t FnCollectionIterator::getCollection(PlanState& planState) const
{
  store::Item_t uriItem;
  store::Collection_t coll;
  std::unique_ptr<internal::Resource> resource;
  internal::CollectionResource* collResource;
  zstring resolvedURI;
  zstring errorMessage;

  if (!theChildren.empty() && 
      consumeNext(uriItem, theChildren[0].getp(), planState))
  {
    try
    {
      resolvedURI = theSctx->resolve_relative_uri(uriItem->getStringValue());
    }
    catch (ZorbaException const&)
    {
      RAISE_ERROR(err::FODC0004, loc,
      ERROR_PARAMS(uriItem->getStringValue(), ZED(BadAnyURI)));
    }
  }
  else
  {
    uriItem = planState.theGlobalDynCtx->get_default_collection();

    if (NULL == uriItem)
    {
      RAISE_ERROR(err::FODC0002, loc,
      ERROR_PARAMS(ZED(DefaultCollation), ZED(NotDefInDynamicCtx)));
    }

    resolvedURI = theSctx->resolve_relative_uri(uriItem->getStringValue());
  }

  resource = theSctx->resolve_uri(resolvedURI, 
                                  internal::EntityData::COLLECTION,
                                  errorMessage);

  collResource = dynamic_cast<internal::CollectionResource*>(resource.get());

  if (collResource == 0 || !(coll = collResource->getCollection()))
  {
    RAISE_ERROR(err::FODC0002, loc, ERROR_PARAMS(resolvedURI, errorMessage));
  }

  return coll;
}


/*******************************************************************************

********************************************************************************/
ZorbaCollectionIteratorState::~ZorbaCollectionIteratorState()
{
  if (theIterator != NULL)
  {
    // closing the iterator is necessary here if an exception occurs
    // in the producer or if the iterator is not fully consumed
    if (theIteratorOpened)
    {
      theIterator->close();
      theIteratorOpened = false;
    }
    theIterator = NULL;
  }
}


void ZorbaCollectionIteratorState::init(PlanState& planState)
{
  PlanIteratorState::init(planState);
  theIteratorOpened = false;
  theIterator = NULL;
}


void ZorbaCollectionIteratorState::reset(PlanState& planState)
{
  PlanIteratorState::reset(planState);

  if (theIterator != NULL)
  {
    // closing the iterator is necessary here if an exception occurs
    // in the producer or if the iterator is not fully consumed
    if (theIteratorOpened)
    {
      theIterator->close();
      theIteratorOpened = false;
    }
    theIterator = NULL;
  }
}


bool ZorbaCollectionIterator::isCountOptimizable() const
{
  // if ref is passed to the collections function, count cannot be 
  // optimized anymore.
  return theChildren.size() <= 2;
}


void ZorbaCollectionIterator::initCollection(PlanState& planState, int64_t skipCount) const
{
  store::Item_t name;
  xs_integer lSkip;
  store::Collection_t collection;

  ZorbaCollectionIteratorState* state = StateTraitsImpl<ZorbaCollectionIteratorState>::getState(planState, theStateOffset);

  consumeNext(name, theChildren[0].getp(), planState);

  (void)getCollection(theSctx, name, loc, theIsDynamic, collection);

  if (theChildren.size() == 1)
  {
    if (skipCount < 0)
      skipCount = 0;
    lSkip = skipCount;
    state->theIterator = collection->getIterator(lSkip);
  }
  else
  {
    zstring lStart;
    bool lRefPassed = theChildren.size() >= 3;

    // read positional skip parameter
    store::Item_t lSkipItem;
    consumeNext(lSkipItem, theChildren[(lRefPassed ? 2 : 1)].getp(), planState);
    lSkip = lSkipItem->getIntegerValue() + skipCount;

    // negative skip is not allowed
    if (lSkip.sign() < 0)
    {
      lSkip = numeric_consts<xs_integer>::zero();
    }

    if (!lRefPassed)
    {
      state->theIterator = collection->getIterator(lSkip);
    }
    else
    {
      store::Item_t lRefItem;
      consumeNext(lRefItem, theChildren[1].getp(), planState);
      lStart = lRefItem->getString();
      try
      {
        state->theIterator = collection->getIterator(lSkip, lStart);
      }
      catch (ZorbaException& e)
      {
        set_source(e, loc);
        throw;
      }
    }
  }

  ZORBA_ASSERT(state->theIterator != NULL);

  try
  {
    state->theIterator->open();
  }
  catch (std::range_error const&)
  {
    RAISE_ERROR(zerr::ZXQD0004_INVALID_PARAMETER, loc,
    ERROR_PARAMS(ZED(ZXQD0004_NOT_WITHIN_RANGE), lSkip));
  }

  state->theIteratorOpened = true;
}


bool ZorbaCollectionIterator::nextImpl(
    store::Item_t& result,
    PlanState& planState) const
{
  store::Item_t name;
  xs_integer lSkip;
  store::Collection_t collection;

  ZorbaCollectionIteratorState* state;
  DEFAULT_STACK_INIT(ZorbaCollectionIteratorState, state, planState);

  if (state->theIterator.getp() != NULL && state->theIteratorOpened == false)
  {
    ZORBA_ASSERT (false && "nextImpl() called past iterator end");
    return false;
  }
  else if ( ! state->theIteratorOpened)
  {
    initCollection(planState, 0);
  }

  while (state->theIterator->next(result))
    STACK_PUSH(true, state);

  // close as early as possible
  state->theIterator->close();
  state->theIteratorOpened = false;

  STACK_END(state);
}


bool ZorbaCollectionIterator::skip(int64_t count, PlanState& planState) const
{  
  ZorbaCollectionIteratorState* state = StateTraitsImpl<ZorbaCollectionIteratorState>::getState(planState, theStateOffset);

  if (state->theIterator.getp() != NULL && state->theIteratorOpened == false)
  {
    ZORBA_ASSERT (false && "nextImpl() called past iterator end");
    return false;
  }
  else if ( ! state->theIteratorOpened)
  {
    initCollection(planState, count);
    return true;
  }
  else
    return false;
}


bool ZorbaCollectionIterator::count(store::Item_t& result, PlanState& planState) const
{
  if (!isCountOptimizable())
    return PlanIterator::count(result, planState);

  store::Item_t name;
  store::Collection_t collection;
  xs_integer count;

  ZorbaCollectionIteratorState* state;
  DEFAULT_STACK_INIT(ZorbaCollectionIteratorState, state, planState);

  consumeNext(name, theChildren[0].getp(), planState);

  (void)getCollection(theSctx, name, loc, theIsDynamic, collection);

  count = collection->size();

  if (theChildren.size() == 2) 
  {
    // skip parameter passed
    store::Item_t skipItem;
    consumeNext(skipItem, theChildren[1].getp(), planState);
    xs_integer skip( skipItem->getIntegerValue() );

    count -= (skip.sign() <= 0 ? numeric_consts<xs_integer>::zero() : skip);
    count = (count.sign() < 0 ? numeric_consts<xs_integer>::zero() : count);
  }

  STACK_PUSH(GENV_ITEMFACTORY->createInteger(result, count), state);

  STACK_END(state);
}


/*******************************************************************************
  declare function index-of($name as xs:QName,
                            $target as node()) as xs:integer

  The function will return the index of the $target node within the collection
  identified by $name. Note: the function uses index numbers starting at 1.
********************************************************************************/
bool ZorbaIndexOfIterator::nextImpl(
    store::Item_t& result,
    PlanState& planState) const
{
  store::Collection_t collection;
  store::Item_t node;
  xs_integer pos( 1 );
  bool found;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  if (consumeNext(node, theChildren[0].getp(), planState))
  {
    collection = node->getCollection();

    if (!collection)
      throw XQUERY_EXCEPTION(zerr::ZDDY0017_NODE_IS_ORPHAN, ERROR_LOC(loc));

    found = collection->findNode(node, pos);
    ZORBA_ASSERT(found);
    STACK_PUSH(GENV_ITEMFACTORY->createInteger(result, pos+1), state);
  }

  STACK_END(state);
}


/*******************************************************************************
  declare updating function
  create($name as xs:QName) as empty-sequence()

  declare updating function
  create($name as xs:QName, $nodes as node()*) as empty-sequence()

  The first function will create a new, empty, collection. The second function
  will create a new collection and will add the given nodes to it.
********************************************************************************/
bool ZorbaCreateCollectionIterator::nextImpl(
    store::Item_t& result,
    PlanState& aPlanState) const
{
  store::Item_t name;
  store::Collection_t collection;
  const StaticallyKnownCollection* collectionDecl;
  store::Item_t node;
  store::Item_t copyNode;
  std::unique_ptr<store::PUL> pul;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, aPlanState);

  consumeNext(name, theChildren[0].getp(), aPlanState);

  collectionDecl = getCollection(name, collection);

  pul.reset(GENV_ITEMFACTORY->createPendingUpdateList());

  if (theIsDynamic)
  {
    std::vector<store::Annotation_t> lAnnotations;
    store::Annotation_t lAnn = 0;

    // dynamic collections have some default properties
    lAnn = new store::Annotation();
    lAnn->theName = AnnotationInternal::lookup(AnnotationInternal::zann_mutable);
    lAnnotations.push_back(lAnn);

    lAnn = new store::Annotation();
    lAnn->theName = AnnotationInternal::lookup(AnnotationInternal::zann_ordered);
    lAnnotations.push_back(lAnn);

    lAnn = new store::Annotation();
    lAnn->theName = AnnotationInternal::lookup(AnnotationInternal::zann_mutable_nodes);
    lAnnotations.push_back(lAnn);

    pul->addCreateCollection(&loc, name, lAnnotations, true);
  }
  else
  {
    std::vector<store::Annotation_t> lAnnotations;
    AnnotationList* lDeclaredAnnotations = collectionDecl->getAnnotations();

    for (csize i = 0; i < lDeclaredAnnotations->size(); ++i)
    {
      store::Annotation_t lAnn = new store::Annotation();
      AnnotationInternal* lTmp = lDeclaredAnnotations->get(i);

      lAnn->theName = lTmp->getQName();

      for (csize j = 0; j < lTmp->getNumLiterals(); ++j)
      {
        lAnn->theLiterals.push_back(lTmp->getLiteral(j));
      }
      lAnnotations.push_back(lAnn);
    }

    pul->addCreateCollection(&loc, name, lAnnotations, false);
  }

  // also add some optional nodes to the collection
  if (theChildren.size() == 2)
  {
    store::CopyMode copymode;
    getCopyMode(copymode, theSctx);
    std::vector<store::Item_t> nodes;
    csize numNodes = 0;

    while (consumeNext(node, theChildren[1].getp(), aPlanState))
    {
      checkNodeType(theSctx, node, collectionDecl, loc, theIsDynamic);

      copyNode = node->copy(NULL, copymode);

      nodes.resize(numNodes+1);
      nodes[numNodes++].transfer(copyNode);
    }

    pul->addInsertIntoCollection(&loc, name, nodes, theIsDynamic);
  }

  result = pul.release();
  STACK_PUSH(result != NULL, state);

  STACK_END(state);
}


const StaticallyKnownCollection*
ZorbaCreateCollectionIterator::getCollection(
    const store::Item_t& name,
    store::Collection_t& coll) const
{
  const StaticallyKnownCollection* collectionDecl = theSctx->lookup_collection(name);
  if (collectionDecl == 0  && !theIsDynamic)
  {
    RAISE_ERROR(zerr::ZDDY0001_COLLECTION_NOT_DECLARED, loc,
    ERROR_PARAMS(name->getStringValue()));
  }

  coll = GENV_STORE.getCollection(name, theIsDynamic);

  if (coll != NULL)
  {
    RAISE_ERROR(zerr::ZDDY0002_COLLECTION_ALREADY_EXISTS, loc,
    ERROR_PARAMS(name->getStringValue()));
  }

  return collectionDecl;
}


/*******************************************************************************
  declare updating function delete($name as xs:QName)

  The function will delete the collection identified by the given URI.
********************************************************************************/
bool ZorbaDeleteCollectionIterator::nextImpl(
    store::Item_t& result,
    PlanState& aPlanState) const
{
  store::Item_t name;
  store::Collection_t collection;
  std::unique_ptr<store::PUL> pul;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, aPlanState);

  consumeNext(name, theChildren[0].getp(), aPlanState);

  (void)getCollection(theSctx, name, loc, theIsDynamic, collection);

  pul.reset(GENV_ITEMFACTORY->createPendingUpdateList());

  pul->addDeleteCollection(&loc, name, theIsDynamic);

  result = pul.release();
  STACK_PUSH(result != NULL, state);

  STACK_END(state);
}


/*******************************************************************************
  declare updating function
  insert($name as xs:QName, $newnode as node()*)

  The function will insert the given nodes to the given collection.
********************************************************************************/
bool ZorbaInsertIterator::nextImpl(
    store::Item_t& result,
    PlanState& planState) const
{
  std::vector<store::Item_t>       nodes;
  std::unique_ptr<store::PUL>        pul;
  store::Item_t                    name;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  checkCollectionAndCopyNodes(planState, name, nodes);

  // create the pul and add the primitive
  pul.reset(GENV_ITEMFACTORY->createPendingUpdateList());

  if (nodes.size() > 0)
    pul->addInsertIntoCollection(&loc, name, nodes, theIsDynamic);

  result = pul.release();
  STACK_PUSH(result != NULL, state);

  STACK_END(state);
}


const StaticallyKnownCollection*
ZorbaInsertIterator::getCollection(
    const store::Item_t& name,
    store::Collection_t& coll) const
{
  const StaticallyKnownCollection* collectionDecl =
  zorba::getCollection(theSctx, name, loc, theIsDynamic, coll);

  if (!theIsDynamic) 
  {
    // checking collection update mode
    switch(collectionDecl->getUpdateProperty())
    {
      case StaticContextConsts::decl_const:
        RAISE_ERROR(zerr::ZDDY0004_COLLECTION_CONST_UPDATE, loc,
        ERROR_PARAMS(name->getStringValue()));

      case StaticContextConsts::decl_append_only:
        RAISE_ERROR(zerr::ZDDY0005_COLLECTION_APPEND_BAD_INSERT, loc,
        ERROR_PARAMS(name->getStringValue()));

      case StaticContextConsts::decl_queue:
        RAISE_ERROR(zerr::ZDDY0006_COLLECTION_QUEUE_BAD_INSERT, loc,
        ERROR_PARAMS(name->getStringValue()));

      case StaticContextConsts::decl_mutable:
        // good to go
        break;

      default:
        ZORBA_ASSERT(false);
    }
  }

  return collectionDecl;
}


/*******************************************************************************
  declare updating function
  insert-first($name as xs:QName, $newnode as node()*) as none

  The function will insert the given node(s) as the first node(s) of the given
  collection. If multiple nodes are inserted, the nodes remain adjacent and
  their order preserves the node ordering of the source expression.
********************************************************************************/
bool ZorbaInsertFirstIterator::nextImpl(
    store::Item_t& result,
    PlanState& planState) const
{
  std::vector<store::Item_t> nodes;
  std::unique_ptr<store::PUL> pul;
  store::Item_t name;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  checkCollectionAndCopyNodes(planState, name, nodes);

  // create the pul and add the primitive
  pul.reset(GENV_ITEMFACTORY->createPendingUpdateList());

  pul->addInsertFirstIntoCollection(&loc, name, nodes, theIsDynamic);

  // this should not be necessary. we reset everything in the sequential iterator
  theChildren[theChildren.size()-1]->reset(planState);

  result = pul.release();
  STACK_PUSH(result != NULL, state);

  STACK_END(state);
}


const StaticallyKnownCollection*
ZorbaInsertFirstIterator::getCollection(
    const store::Item_t& name,
    store::Collection_t& coll) const
{
  const StaticallyKnownCollection* collectionDecl =
  zorba::getCollection(theSctx, name, loc, theIsDynamic, coll);

  if (!theIsDynamic) 
  {
    switch(collectionDecl->getUpdateProperty())
    {
      case StaticContextConsts::decl_const:
        RAISE_ERROR(zerr::ZDDY0004_COLLECTION_CONST_UPDATE, loc,
        ERROR_PARAMS(name->getStringValue()));

      case StaticContextConsts::decl_append_only:
        RAISE_ERROR(zerr::ZDDY0005_COLLECTION_APPEND_BAD_INSERT, loc,
        ERROR_PARAMS(name->getStringValue()));

      case StaticContextConsts::decl_queue:
        RAISE_ERROR(zerr::ZDDY0006_COLLECTION_QUEUE_BAD_INSERT, loc,
        ERROR_PARAMS(name->getStringValue()));

      case StaticContextConsts::decl_mutable:
        // good to go
        break;

      default:
        ZORBA_ASSERT(false);
    }

    if (collectionDecl && !collectionDecl->isOrdered())
    {
      RAISE_ERROR(zerr::ZDDY0012_COLLECTION_UNORDERED_BAD_OPERATION, loc,
      ERROR_PARAMS(name->getStringValue(), "insert" ));
    }
  }
  return collectionDecl;
}


/*******************************************************************************
  declare updating function
  insert-last($name as xs:QName, $newnode as node()*) as none

  The function will insert the given node(s) as the last node(s) of the given
  collection. If multiple nodes are inserted, the nodes remain adjacent and
  their order preserves the node ordering of the source expression.

  Error conditions:
  - If the collection URI is empty and the default collection
    is not defined in the dynamic context, FODC0002 is raised
  - If the specified collection does not exist, an error is raised
    (ZAPI0006_COLLECTION_NOT_FOUND - collection does not exist).
  - If the node is already in the collection, an error is raised
    (ZAPI0031_NODE_ALREADY_IN_COLLECTION)
********************************************************************************/
bool ZorbaInsertLastIterator::nextImpl(
    store::Item_t& result,
    PlanState& planState) const
{
  std::vector<store::Item_t>       nodes;
  std::unique_ptr<store::PUL>        pul;
  store::Item_t                    name;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  checkCollectionAndCopyNodes(planState, name, nodes);

  // create the pul and add the primitive
  pul.reset(GENV_ITEMFACTORY->createPendingUpdateList());

  pul->addInsertLastIntoCollection(&loc, name, nodes, theIsDynamic);

  // this should not be necessary. we reset everything in the sequential iterator
  theChildren[theChildren.size()-1]->reset(planState);

  result = pul.release();
  STACK_PUSH(result != NULL, state);

  STACK_END (state);
}


const StaticallyKnownCollection*
ZorbaInsertLastIterator::getCollection(
    const store::Item_t& name,
    store::Collection_t& coll) const
{
  const StaticallyKnownCollection* collectionDecl =
  zorba::getCollection(theSctx, name, loc, theIsDynamic, coll);

  if (!theIsDynamic) 
  {
    switch(collectionDecl->getUpdateProperty())
    {
      case StaticContextConsts::decl_const:
        RAISE_ERROR(zerr::ZDDY0004_COLLECTION_CONST_UPDATE, loc,
        ERROR_PARAMS(name->getStringValue()));

      case StaticContextConsts::decl_append_only:
      case StaticContextConsts::decl_queue:
      case StaticContextConsts::decl_mutable:
        // good to go
        break;

      default:
        ZORBA_ASSERT(false);
    }

    if (collectionDecl && !collectionDecl->isOrdered())
    {
      RAISE_ERROR(zerr::ZDDY0012_COLLECTION_UNORDERED_BAD_OPERATION, loc,
      ERROR_PARAMS(name->getStringValue(), "insert"));
    }
  }
  return collectionDecl;
}


/*******************************************************************************
  declare updating function
  insert-before($name as xs:QName,
                $target as node(),
                $newnodes as node()*)

  The inserted nodes become the preceding (or following) nodes of the target.
  The $target should be a non-updating expression (e.g. an XPath expression)
  identifying a node that is part of the given collection at the top-level.
  If multiple nodes are inserted by a single insert expression, the nodes remain
  adjacent and their order preserves the node ordering of the source expression.
********************************************************************************/
bool ZorbaInsertBeforeIterator::nextImpl(
    store::Item_t& result,
    PlanState& planState) const
{
  store::Item_t                    name;
  store::Item_t                    targetNode;
  store::Item_t                    node;
  std::vector<store::Item_t>       nodes;
  std::unique_ptr<store::PUL>        pul;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  checkCollectionAndCopyNodes(planState, name, nodes, targetNode, true);
  // create the pul and add the primitive
  pul.reset(GENV_ITEMFACTORY->createPendingUpdateList());

  pul->addInsertBeforeIntoCollection(&loc,
                                     name, 
                                     targetNode,
                                     nodes,
                                     theIsDynamic);

  // this should not be necessary. we reset everything in the sequential iterator
  theChildren[theChildren.size()-2]->reset(planState);
  theChildren[theChildren.size()-1]->reset(planState);

  result = pul.release();
  STACK_PUSH(result != NULL, state);

  STACK_END(state);
}


const StaticallyKnownCollection*
ZorbaInsertBeforeIterator::getCollection(
    const store::Item_t& name,
    store::Collection_t& coll) const
{
  const StaticallyKnownCollection* collectionDecl =
  zorba::getCollection(theSctx, name, loc, theIsDynamic, coll);

  if (!theIsDynamic) 
  {
    switch(collectionDecl->getUpdateProperty())
    {
      case StaticContextConsts::decl_const:
        RAISE_ERROR(zerr::ZDDY0004_COLLECTION_CONST_UPDATE, loc,
        ERROR_PARAMS(name->getStringValue()));

      case StaticContextConsts::decl_append_only:
        RAISE_ERROR(zerr::ZDDY0005_COLLECTION_APPEND_BAD_INSERT, loc,
        ERROR_PARAMS(name->getStringValue()));

      case StaticContextConsts::decl_queue:
        RAISE_ERROR(zerr::ZDDY0006_COLLECTION_QUEUE_BAD_INSERT, loc,
        ERROR_PARAMS(name->getStringValue()));

      case StaticContextConsts::decl_mutable:
        // good to go
        break;

      default:
        ZORBA_ASSERT(false);
    }

    if (collectionDecl && !collectionDecl->isOrdered())
    {
      RAISE_ERROR(zerr::ZDDY0012_COLLECTION_UNORDERED_BAD_OPERATION, loc,
      ERROR_PARAMS(name->getStringValue(), "insert" ));
    }
  }
  return collectionDecl;
}


/*******************************************************************************
  declare updating function
  insert-after($name as xs:QName,
               $target as node(),
               $newnode as node()*)

  The inserted nodes become the following nodes of the $target. The $target
  should be a non-updating expression (e.g. an XPath expression) identifying a
  node that is part of the given collection at the top-level. If multiple nodes
  are inserted by a single insert expression, the nodes remain adjacent and
  their order preserves the node ordering of the source expression.
********************************************************************************/
bool ZorbaInsertAfterIterator::nextImpl(
    store::Item_t& result,
    PlanState& planState) const
{
  store::Item_t               name;
  std::vector<store::Item_t>  nodes;
  std::unique_ptr<store::PUL>   pul;
  store::Item_t               targetNode;

  store::CopyMode lCopyMode;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  checkCollectionAndCopyNodes(planState, name, nodes, targetNode, true);

  // create the pul and add the primitive
  pul.reset(GENV_ITEMFACTORY->createPendingUpdateList());

  pul->addInsertAfterIntoCollection(&loc,
                                    name,
                                    targetNode,
                                    nodes, 
                                    theIsDynamic);

  // this should not be necessary. we reset everything in the sequential iterator
  theChildren[theChildren.size()-2]->reset(planState);
  theChildren[theChildren.size()-1]->reset(planState);

  result = pul.release();
  STACK_PUSH(result != NULL, state);

  STACK_END(state);
}


const StaticallyKnownCollection*
ZorbaInsertAfterIterator::getCollection(
    const store::Item_t& name,
    store::Collection_t& coll) const
{
  const StaticallyKnownCollection* collectionDecl =
  zorba::getCollection(theSctx, name, loc, theIsDynamic, coll);

  if (!theIsDynamic) 
  {
    // checking collection modifiers
    switch (collectionDecl->getUpdateProperty())
    {
      case StaticContextConsts::decl_const:
        RAISE_ERROR(zerr::ZDDY0004_COLLECTION_CONST_UPDATE, loc,
        ERROR_PARAMS(name->getStringValue()));

      case StaticContextConsts::decl_append_only:
        RAISE_ERROR(zerr::ZDDY0005_COLLECTION_APPEND_BAD_INSERT, loc,
        ERROR_PARAMS(name->getStringValue()));

      case StaticContextConsts::decl_queue:
        RAISE_ERROR(zerr::ZDDY0006_COLLECTION_QUEUE_BAD_INSERT, loc,
        ERROR_PARAMS(name->getStringValue()));

      case StaticContextConsts::decl_mutable:
        // good to go
        break;

      default:
        ZORBA_ASSERT(false);
    }

    if (collectionDecl && !collectionDecl->isOrdered())
    {
      RAISE_ERROR(zerr::ZDDY0011_COLLECTION_NODE_NOT_FOUND, loc,
      ERROR_PARAMS(name->getStringValue()));
    }
  }
  return collectionDecl;
}


/*******************************************************************************
  declare sequential function
  apply-insert($name as xs:QName, $newnode as node()*)

********************************************************************************/
ZorbaApplyInsertIteratorState::~ZorbaApplyInsertIteratorState()
{
  nodes.clear();
}


void ZorbaApplyInsertIteratorState::init(PlanState& planState)
{
  PlanIteratorState::init(planState);
  nodes.clear();
}


void ZorbaApplyInsertIteratorState::reset(PlanState& planState)
{
  PlanIteratorState::reset(planState);
  nodes.clear();
}


bool ZorbaApplyInsertIterator::nextImpl(
    store::Item_t& result,
    PlanState& planState) const
{
  std::unique_ptr<store::PUL>        pul;
  std::vector<store::Item_t>       nodes;
  store::Item_t                    name;

  ZorbaApplyInsertIteratorState* state;
  DEFAULT_STACK_INIT(ZorbaApplyInsertIteratorState, state, planState);

  checkCollectionAndCopyNodes(planState, name, nodes);

  // create the pul and add the primitive
  pul.reset(GENV_ITEMFACTORY->createPendingUpdateList());

  if (nodes.size() > 0) 
  {
    // addInsertIntoCollection removes the nodes from the vector.
    // hence, we also store them in a separate vector in the state
    state->nodes.resize(nodes.size());
    std::copy(nodes.begin(), nodes.end(), state->nodes.begin());

    pul->addInsertIntoCollection(&loc, name, nodes, theIsDynamic);
  }

  if (pul.get()) 
  {
    apply_updates(planState.theCompilerCB, 
                  planState.theGlobalDynCtx,
                  theSctx,
                  pul.get(),
                  loc);
    pul.reset();
  }

  state->iterator = state->nodes.begin();

  while (state->iterator != state->nodes.end()) 
  {
    result = *state->iterator;
    ++(state->iterator);
    STACK_PUSH(true, state);
  }

  STACK_PUSH(false, state);

  STACK_END(state);
}


const StaticallyKnownCollection*
ZorbaApplyInsertIterator::getCollection(
    const store::Item_t& name,
    store::Collection_t& coll) const
{
  return zorba::getCollection(theSctx, name, loc, theIsDynamic, coll);
}

/*******************************************************************************
  declare sequential function
  apply-insert-first($name as xs:QName, $newnode as node()*)

********************************************************************************/
ZorbaApplyInsertFirstIteratorState::~ZorbaApplyInsertFirstIteratorState()
{
  nodes.clear();
}


void ZorbaApplyInsertFirstIteratorState::init(PlanState& planState)
{
  PlanIteratorState::init(planState);
  nodes.clear();
}


void ZorbaApplyInsertFirstIteratorState::reset(PlanState& planState)
{
  PlanIteratorState::reset(planState);
  nodes.clear();
}


bool ZorbaApplyInsertFirstIterator::nextImpl(
    store::Item_t& result,
    PlanState& planState) const
{
  std::unique_ptr<store::PUL>        pul;
  std::vector<store::Item_t>       nodes;
  store::Item_t                    name;

  ZorbaApplyInsertFirstIteratorState* state;
  DEFAULT_STACK_INIT(ZorbaApplyInsertFirstIteratorState, state, planState);

  checkCollectionAndCopyNodes(planState, name, nodes);

  // create the pul and add the primitive
  pul.reset(GENV_ITEMFACTORY->createPendingUpdateList());

  if (nodes.size() > 0)
  {
    state->nodes.resize(nodes.size());
    std::copy(nodes.begin(), nodes.end(), state->nodes.begin());

    pul->addInsertFirstIntoCollection(&loc, name, nodes, theIsDynamic);
  }

  if (pul.get())
  {
    apply_updates(planState.theCompilerCB,
                  planState.theGlobalDynCtx,
                  theSctx,
                  pul.get(),
                  loc);
    pul.reset();
  }

  state->iterator = state->nodes.begin();

  while (state->iterator != state->nodes.end())
  {
    result = *state->iterator;
    ++(state->iterator);
    STACK_PUSH(true, state);
  }

  STACK_PUSH(false, state);

  STACK_END(state);
}

const StaticallyKnownCollection*
ZorbaApplyInsertFirstIterator::getCollection(
    const store::Item_t& name,
    store::Collection_t& coll) const
{
  return zorba::getCollection(theSctx, name, loc, theIsDynamic, coll);
}


/*******************************************************************************
  declare sequential function
  apply-last($name as xs:QName, $newnode as node()*)

********************************************************************************/
ZorbaApplyInsertLastIteratorState::~ZorbaApplyInsertLastIteratorState()
{
  nodes.clear();
}


void ZorbaApplyInsertLastIteratorState::init(PlanState& planState)
{
  PlanIteratorState::init(planState);
  nodes.clear();
}


void ZorbaApplyInsertLastIteratorState::reset(PlanState& planState)
{
  PlanIteratorState::reset(planState);
  nodes.clear();
}


bool
ZorbaApplyInsertLastIterator::nextImpl(
    store::Item_t& result,
    PlanState& planState) const
{
  std::unique_ptr<store::PUL>        pul;
  std::vector<store::Item_t>       nodes;
  store::Item_t                    name;

  ZorbaApplyInsertLastIteratorState* state;
  DEFAULT_STACK_INIT(ZorbaApplyInsertLastIteratorState, state, planState);

  checkCollectionAndCopyNodes(planState, name, nodes);

  // create the pul and add the primitive
  pul.reset(GENV_ITEMFACTORY->createPendingUpdateList());

  if (nodes.size() > 0) 
  {
    state->nodes.resize(nodes.size());
    std::copy(nodes.begin(), nodes.end(), state->nodes.begin());

    pul->addInsertLastIntoCollection(&loc, name, nodes, theIsDynamic);
  }

  if (pul.get()) 
  {
    apply_updates(planState.theCompilerCB,
                  planState.theGlobalDynCtx,
                  theSctx,
                  pul.get(),
                  loc);
    pul.reset();
  }

  state->iterator = state->nodes.begin();

  while (state->iterator != state->nodes.end()) 
  {
    result = *state->iterator;
    ++(state->iterator);
    STACK_PUSH(true, state);
  }

  STACK_PUSH(false, state);

  STACK_END(state);
}


const StaticallyKnownCollection*
ZorbaApplyInsertLastIterator::getCollection(
    const store::Item_t& name,
    store::Collection_t& coll) const
{
  return zorba::getCollection(theSctx, name, loc, theIsDynamic, coll);
}


/*******************************************************************************
  declare sequential function
  apply-before($name as xs:QName, $newnode as node()*)

********************************************************************************/
ZorbaApplyInsertBeforeIteratorState::~ZorbaApplyInsertBeforeIteratorState()
{
  nodes.clear();
}


void ZorbaApplyInsertBeforeIteratorState::init(PlanState& planState)
{
  PlanIteratorState::init(planState);
  nodes.clear();
}


void ZorbaApplyInsertBeforeIteratorState::reset(PlanState& planState)
{
  PlanIteratorState::reset(planState);
  nodes.clear();
}


bool
ZorbaApplyInsertBeforeIterator::nextImpl(
    store::Item_t& result,
    PlanState& planState) const
{
  std::unique_ptr<store::PUL>        pul;
  std::vector<store::Item_t>       nodes;
  store::Item_t                    name;
  store::Item_t                    targetNode;

  ZorbaApplyInsertIteratorState* state;
  DEFAULT_STACK_INIT(ZorbaApplyInsertIteratorState, state, planState);

  checkCollectionAndCopyNodes(planState, name, nodes, targetNode, true);

  // create the pul and add the primitive
  pul.reset(GENV_ITEMFACTORY->createPendingUpdateList());

  if (nodes.size() > 0) 
  {
    state->nodes.resize(nodes.size());
    std::copy(nodes.begin(), nodes.end(), state->nodes.begin());

    pul->addInsertBeforeIntoCollection(&loc,
                                       name,
                                       targetNode,
                                       nodes,
                                       theIsDynamic);
  }

  if (pul.get()) 
  {
    apply_updates(planState.theCompilerCB,
                  planState.theGlobalDynCtx,
                  theSctx,
                  pul.get(),
                  loc);
    pul.reset();
  }

  state->iterator = state->nodes.begin();

  while (state->iterator != state->nodes.end()) 
  {
    result = *state->iterator;
    ++(state->iterator);
    STACK_PUSH(true, state);
  }

  STACK_PUSH(false, state);

  STACK_END(state);
}


const StaticallyKnownCollection*
ZorbaApplyInsertBeforeIterator::getCollection(
    const store::Item_t& name,
    store::Collection_t& coll) const
{
  return zorba::getCollection(theSctx, name, loc, theIsDynamic, coll);
}


/*******************************************************************************
  declare sequential function
  apply-after($name as xs:QName, $newnode as node()*)

********************************************************************************/
ZorbaApplyInsertAfterIteratorState::~ZorbaApplyInsertAfterIteratorState()
{
  nodes.clear();
}


void ZorbaApplyInsertAfterIteratorState::init(PlanState& planState)
{
  PlanIteratorState::init(planState);
  nodes.clear();
}


void ZorbaApplyInsertAfterIteratorState::reset(PlanState& planState)
{
  PlanIteratorState::reset(planState);
  nodes.clear();
}


bool
ZorbaApplyInsertAfterIterator::nextImpl(
    store::Item_t& result,
    PlanState& planState) const
{
  std::unique_ptr<store::PUL>        pul;
  std::vector<store::Item_t>       nodes;
  store::Item_t                    name;
  store::Item_t                    targetNode;

  ZorbaApplyInsertIteratorState* state;
  DEFAULT_STACK_INIT(ZorbaApplyInsertIteratorState, state, planState);

  checkCollectionAndCopyNodes(planState, name, nodes, targetNode, true);

  // create the pul and add the primitive
  pul.reset(GENV_ITEMFACTORY->createPendingUpdateList());

  if (nodes.size() > 0) 
  {
    state->nodes.resize(nodes.size());
    std::copy(nodes.begin(), nodes.end(), state->nodes.begin());

    pul->addInsertAfterIntoCollection(&loc,
                                      name,
                                      targetNode,
                                      nodes,
                                      theIsDynamic);
  }

  if (pul.get()) 
  {
    apply_updates(planState.theCompilerCB,
                  planState.theGlobalDynCtx,
                  theSctx,
                  pul.get(),
                  loc);
    pul.reset();
  }

  state->iterator = state->nodes.begin();

  while (state->iterator != state->nodes.end())
  {
    result = *state->iterator;
    ++(state->iterator);
    STACK_PUSH(true, state);
  }

  STACK_PUSH(false, state);


  STACK_END(state);
}


const StaticallyKnownCollection*
ZorbaApplyInsertAfterIterator::getCollection(
    const store::Item_t& name,
    store::Collection_t& coll) const
{
  return zorba::getCollection(theSctx, name, loc, theIsDynamic, coll);
}


/*******************************************************************************
  declare updating function
  delete($name as xs:QName, $target as node()*)

  The function will remove the node(s) identified by the $target expression
  from the given collection. The nodes themselves will not be deleted until all
  references to them have been removed.
********************************************************************************/
bool ZorbaDeleteIterator::nextImpl(
    store::Item_t& result,
    PlanState& planState) const
{
  store::Collection_t              collection;
  store::Item_t                    name;
  store::Item_t                    node;
  std::vector<store::Item_t>       nodes;
  std::unique_ptr<store::PUL>        pul;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  while (consumeNext(node, theChildren[theChildren.size()-1].getp(), planState))
  {
    if (! node->getCollection())
    {
      throw XQUERY_EXCEPTION(zerr::ZDDY0017_NODE_IS_ORPHAN, ERROR_LOC(loc));
    }
    if (node->isNode() && node->getParent())
    {
      throw XQUERY_EXCEPTION(
        zerr::ZDDY0036_NON_ROOT_NODE_DELETION,
        ERROR_PARAMS(node->getCollection()->getName()->getStringValue()),
        ERROR_LOC( loc )
      );
    }
    if (collection && collection != node->getCollection()) 
    {
      throw XQUERY_EXCEPTION(zerr::ZDDY0018_NODES_NOT_IN_SAME_COLLECTION, ERROR_LOC(loc));
    }

    collection = node->getCollection();

    nodes.push_back(node);
  }

  if (!nodes.empty())
  {
    name = collection->getName();

    (void)getCollection(name, collection);

    // create the pul and add the primitive
    pul.reset(GENV_ITEMFACTORY->createPendingUpdateList());

    pul->addDeleteFromCollection(&loc,
                                 name,
                                 nodes,
                                 false,
                                 theIsDynamic);

    // this should not be necessary. we reset everything in the sequential iterator
    theChildren[theChildren.size()-1]->reset(planState);

    result = pul.release();
    STACK_PUSH(result != NULL, state);
  }

  STACK_END(state);
}


const StaticallyKnownCollection*
ZorbaDeleteIterator::getCollection(
    const store::Item_t& name,
    store::Collection_t& coll) const
{
  const StaticallyKnownCollection* collectionDecl = 
  zorba::getCollection(theSctx, name, loc, theIsDynamic, coll);

  if (!theIsDynamic) 
  {
    switch(collectionDecl->getUpdateProperty())
    {
      case StaticContextConsts::decl_const:
        RAISE_ERROR(zerr::ZDDY0004_COLLECTION_CONST_UPDATE, loc,
        ERROR_PARAMS(name->getStringValue()));

      case StaticContextConsts::decl_append_only:
        RAISE_ERROR(zerr::ZDDY0007_COLLECTION_APPEND_BAD_DELETE, loc,
        ERROR_PARAMS(name->getStringValue()));

      case StaticContextConsts::decl_queue:
        RAISE_ERROR(zerr::ZDDY0009_COLLECTION_QUEUE_BAD_DELETE, loc,
        ERROR_PARAMS(name->getStringValue()));

      case StaticContextConsts::decl_mutable:
        // good to go
        break;

      default:
        ZORBA_ASSERT(false);
    }
  }

  return collectionDecl;
}


/*******************************************************************************

********************************************************************************/
bool ZorbaDeleteFirstIterator::nextImpl(
    store::Item_t& result,
    PlanState& planState) const
{
  store::Collection_t              collection;
  store::Item_t                    name;
  store::Item_t                    numNodesItem;
  xs_integer                       numNodes( 1 );
  std::vector<store::Item_t>       nodes;
  std::unique_ptr<store::PUL>        pul;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  if (!consumeNext(name, theChildren[0].getp(), planState))
    ZORBA_ASSERT(false);

  (void)getCollection(name, collection);

  if (theChildren.size() > 1)
  {
    if (!consumeNext(numNodesItem, theChildren[1].getp(), planState))
      ZORBA_ASSERT(false);

    numNodes = numNodesItem->getIntegerValue();
  }

  if (collection->size() < numNodes)
  {
    RAISE_ERROR(zerr::ZDDY0011_COLLECTION_NODE_NOT_FOUND, loc,
    ERROR_PARAMS(name->getStringValue()));
  }

  // create the pul and add the primitive
  pul.reset(GENV_ITEMFACTORY->createPendingUpdateList());

  for (xs_integer i( 0 ); i < numNodes; ++i)
    nodes.push_back(collection->nodeAt(i));

  pul->addDeleteFromCollection(&loc, name, nodes, false, theIsDynamic);

  result = pul.release();
  STACK_PUSH(result != NULL, state);

  STACK_END(state);
}


const StaticallyKnownCollection*
ZorbaDeleteFirstIterator::getCollection(
    const store::Item_t& name,
    store::Collection_t& coll) const
{
  const StaticallyKnownCollection* collectionDecl = 
  zorba::getCollection(theSctx, name, loc, theIsDynamic, coll);

  if (!theIsDynamic) 
  {
    switch(collectionDecl->getUpdateProperty())
    {
      case StaticContextConsts::decl_const:
        RAISE_ERROR(zerr::ZDDY0004_COLLECTION_CONST_UPDATE, loc,
        ERROR_PARAMS(name->getStringValue()));

      case StaticContextConsts::decl_append_only:
        RAISE_ERROR(zerr::ZDDY0007_COLLECTION_APPEND_BAD_DELETE, loc,
        ERROR_PARAMS(name->getStringValue()));

      case StaticContextConsts::decl_queue:
      case StaticContextConsts::decl_mutable:
        // good to go
        break;

      default:
        ZORBA_ASSERT(false);
    }
  }

  if (collectionDecl && !collectionDecl->isOrdered())
  {
    RAISE_ERROR(zerr::ZDDY0012_COLLECTION_UNORDERED_BAD_OPERATION, loc,
    ERROR_PARAMS(name->getStringValue(), "delete" ));
  }

  return collectionDecl;
}


/*******************************************************************************

********************************************************************************/
bool ZorbaDeleteLastIterator::nextImpl(
    store::Item_t& result,
    PlanState& planState) const
{
  store::Collection_t              collection;
  store::Item_t                    name;
  store::Item_t                    numNodesItem;
  xs_integer                       numNodes( 1 );
  std::vector<store::Item_t>       nodes;
  std::unique_ptr<store::PUL>        pul;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  consumeNext(name, theChildren[0].getp(), planState);

  (void)getCollection(name, collection);

  if (theChildren.size() > 1)
  {
    if (!consumeNext(numNodesItem, theChildren[1].getp(), planState))
      ZORBA_ASSERT(false);

    numNodes = numNodesItem->getIntegerValue();
  }

  if (collection->size() < numNodes)
  {
    RAISE_ERROR(zerr::ZDDY0011_COLLECTION_NODE_NOT_FOUND, loc,
    ERROR_PARAMS(name->getStringValue()));
  }

  // create the pul and add the primitive
  pul.reset(GENV_ITEMFACTORY->createPendingUpdateList());

  for (xs_integer i = numNodes; i > 0; --i)
    nodes.push_back(collection->nodeAt(collection->size() - i));

  pul->addDeleteFromCollection(&loc, name, nodes, true, theIsDynamic);

  result = pul.release();
  STACK_PUSH( result != NULL, state);

  STACK_END (state);
}


/*******************************************************************************

********************************************************************************/
const StaticallyKnownCollection*
ZorbaDeleteLastIterator::getCollection(
    const store::Item_t& name,
    store::Collection_t& coll) const
{
  const StaticallyKnownCollection* collectionDecl = 
  zorba::getCollection(theSctx, name, loc, theIsDynamic, coll);

  if (!theIsDynamic) 
  {
    switch(collectionDecl->getUpdateProperty())
    {
      case StaticContextConsts::decl_const:
        RAISE_ERROR(zerr::ZDDY0004_COLLECTION_CONST_UPDATE, loc,
        ERROR_PARAMS(name->getStringValue()));

      case StaticContextConsts::decl_append_only:
        RAISE_ERROR(zerr::ZDDY0007_COLLECTION_APPEND_BAD_DELETE, loc,
        ERROR_PARAMS(name->getStringValue()));

      case StaticContextConsts::decl_queue:
        RAISE_ERROR(zerr::ZDDY0008_COLLECTION_QUEUE_BAD_DELETE, loc,
        ERROR_PARAMS(name->getStringValue()));

      case StaticContextConsts::decl_mutable:
        // good to go
        break;

      default:
        ZORBA_ASSERT(false);
    }
  }

  if (collectionDecl && !collectionDecl->isOrdered())
  {
    RAISE_ERROR(zerr::ZDDY0012_COLLECTION_UNORDERED_BAD_OPERATION, loc,
    ERROR_PARAMS(name->getStringValue(), "delete"));
  }

  return collectionDecl;
}


/*******************************************************************************

********************************************************************************/
bool ZorbaEditIterator::nextImpl(
    store::Item_t& result,
    PlanState& planState) const
{
  store::Collection_t              collection;
  store::Item_t                    collectionName;
  const StaticallyKnownCollection* collectionDecl;

  store::Item_t                    target;

  store::Item_t                    content;
  store::CopyMode lCopyMode;

  std::unique_ptr<store::PUL>        pul;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);
  consumeNext(target, theChildren[0].getp(), planState);
  consumeNext(content, theChildren[1].getp(), planState);

  // Target check.
  if (!target->getCollection() || !target->isStructuredItem())
  {
    throw XQUERY_EXCEPTION(zerr::ZDDY0017_NODE_IS_ORPHAN, ERROR_LOC(loc));
  }

  if (!target->isCollectionRoot())
  {
    throw XQUERY_EXCEPTION(
      zerr::ZDDY0039_NON_ROOT_NODE_EDIT,
      ERROR_PARAMS(target->getCollection()->getName()->getStringValue()),
      ERROR_LOC( loc )
    );
  }
  if (target->getKind() != content->getKind())
  {
    throw XQUERY_EXCEPTION(
      zerr::ZDDY0040_INCONSISTENT_EDIT,
      ERROR_LOC( loc )
    );
  }
  if (target->isNode() && (target->getNodeKind() != content->getNodeKind()))
  {
    throw XQUERY_EXCEPTION(
      zerr::ZDDY0040_INCONSISTENT_EDIT,
      ERROR_LOC( loc )
    );
  }
  collection = target->getCollection();
  collectionName = collection->getName();
  collectionDecl = getCollection(collectionName, collection);

  // Content check & copy.
  getCopyMode(lCopyMode, this->theSctx);
  lCopyMode.theDoCopy &= theNeedToCopy;
  lCopyMode.theDoCopy &= !this->theChildren[1]->isConstructor();
  checkNodeType(this->theSctx,
                content,
                collectionDecl,
                this->loc,
                theIsDynamic);
  content = content->copy(NULL, lCopyMode);

  // create the pul and add the primitive
  pul.reset(GENV_ITEMFACTORY->createPendingUpdateList());

  pul->addEditInCollection(
      &loc,
      collectionName,
      target,
      content,
      theIsDynamic);

  result = pul.release();
  STACK_PUSH( result != NULL, state);

  STACK_END (state);
}


/*******************************************************************************

********************************************************************************/
const StaticallyKnownCollection*
ZorbaEditIterator::getCollection(
    const store::Item_t& name,
    store::Collection_t& coll) const
{
  const StaticallyKnownCollection* collectionDecl = 
  zorba::getCollection(theSctx, name, loc, theIsDynamic, coll);

  if (!theIsDynamic) 
  {
    switch(collectionDecl->getUpdateProperty())
    {
      case StaticContextConsts::decl_const:
        RAISE_ERROR(zerr::ZDDY0004_COLLECTION_CONST_UPDATE, loc,
        ERROR_PARAMS(name->getStringValue()));

      case StaticContextConsts::decl_append_only:
        RAISE_ERROR(zerr::ZDDY0037_COLLECTION_APPEND_BAD_EDIT, loc,
        ERROR_PARAMS(name->getStringValue()));

      case StaticContextConsts::decl_queue:
        RAISE_ERROR(zerr::ZDDY0038_COLLECTION_QUEUE_BAD_EDIT, loc,
        ERROR_PARAMS(name->getStringValue()));

      case StaticContextConsts::decl_mutable:
        // good to go
        break;

      default:
        ZORBA_ASSERT(false);
    }
  }

  return collectionDecl;
}


/*******************************************************************************

********************************************************************************/
bool ZorbaTruncateCollectionIterator::nextImpl(
    store::Item_t& result,
    PlanState& planState) const
{
  store::Collection_t              collection;
  store::Item_t                    collectionName;
  std::unique_ptr<store::PUL>        pul;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  consumeNext(collectionName, theChildren[0].getp(), planState);

  (void)getCollection(collectionName, collection);

  // create the pul and add the primitive
  pul.reset(GENV_ITEMFACTORY->createPendingUpdateList());

  pul->addTruncateCollection(&loc, collectionName, theIsDynamic);

  result = pul.release();
  STACK_PUSH( result != NULL, state);

  STACK_END (state);
}


/*******************************************************************************

********************************************************************************/
const StaticallyKnownCollection*
ZorbaTruncateCollectionIterator::getCollection(
    const store::Item_t& name,
    store::Collection_t& coll) const
{
  const StaticallyKnownCollection* collectionDecl = 
  zorba::getCollection(theSctx, name, loc, theIsDynamic, coll);

  if (!theIsDynamic)
  {
    // checking collection update mode
    switch(collectionDecl->getUpdateProperty())
    {
      case StaticContextConsts::decl_const:
        RAISE_ERROR(zerr::ZDDY0004_COLLECTION_CONST_UPDATE, loc,
        ERROR_PARAMS(name->getStringValue()));

      case StaticContextConsts::decl_append_only:
        RAISE_ERROR(zerr::ZDDY0005_COLLECTION_APPEND_BAD_INSERT, loc,
        ERROR_PARAMS(name->getStringValue()));

      default:
        break;
    }
  }

  return collectionDecl;
}


/*******************************************************************************

********************************************************************************/
bool ZorbaCollectionNameIterator::nextImpl(
    store::Item_t& result,
    PlanState& planState) const
{
  store::Item_t item;
  store::Collection_t collection;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  consumeNext(item, theChildren[0].getp(), planState);

  collection = item->getCollection();

  if (!collection)
  {
    throw XQUERY_EXCEPTION(zerr::ZDDY0017_NODE_IS_ORPHAN, ERROR_LOC(loc));
  }

  result = collection->getName();

  STACK_PUSH( true, state);

  STACK_END (state);
}


/*******************************************************************************

********************************************************************************/
bool
IsAvailableCollectionIterator::nextImpl(
    store::Item_t& result,
    PlanState& planState) const
{
  store::Item_t       name;
  store::Collection_t collection;
  bool                res = true;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  consumeNext(name, theChildren[0].getp(), planState);

  try
  {
    (void)getCollection(theSctx, name, loc, theIsDynamic, collection);
  }
  catch (ZorbaException const& e)
  {
    if (e.diagnostic() != zerr::ZDDY0003_COLLECTION_DOES_NOT_EXIST)
    {
      throw;
    }

    // collection is not available if the getCollection helper function throws
    // error ZDDY0003_COLLECTION_DOES_NOT_EXIST
    res = false;
  }

  GENV_ITEMFACTORY->createBoolean(result, res);
  STACK_PUSH(true, state);

  STACK_END(state);
}


/*******************************************************************************

********************************************************************************/
AvailableCollectionsIteratorState::~AvailableCollectionsIteratorState()
{
  if ( nameItState != NULL )
  {
    nameItState->close();
    nameItState = NULL;
  }
}


void AvailableCollectionsIteratorState::init(PlanState& planState)
{
  PlanIteratorState::init(planState);
  nameItState = NULL;
}


void AvailableCollectionsIteratorState::reset(PlanState& planState)
{
  PlanIteratorState::reset(planState);
  if ( nameItState != NULL ) 
  {
    nameItState->close();
    nameItState = NULL;
  }
}


bool
AvailableCollectionsIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  store::Item_t nameItem;

  AvailableCollectionsIteratorState* state;
  DEFAULT_STACK_INIT(AvailableCollectionsIteratorState, state, planState);

  if (theIsDynamic)
  {
    state->nameItState = GENV_STORE.listCollectionNames(theIsDynamic);

    state->nameItState->open();
    while (state->nameItState->next(nameItem))
    {
      result = nameItem;
      STACK_PUSH(true, state);
    }
  }
  else
  {
    // static collections have to exist and need to be declared
    for ((state->nameItState = theSctx->collection_names())->open();
         state->nameItState->next(nameItem);)
    {
      if (!GENV_STORE.getCollection(nameItem.getp(), false))
      {
        continue;
      }
      else
      {
        result = nameItem;
        STACK_PUSH(true, state);
      }
    }
  }

  state->nameItState->close();

  STACK_END(state);
}


/*******************************************************************************

********************************************************************************/
bool IsAvailableIndexIterator::nextImpl(
    store::Item_t& result,
    PlanState& planState) const
{
  store::Item_t lName;
  bool res = false;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  consumeNext(lName, theChildren[0].getp(), planState);

  {
    store::Index_t lIndex = GENV_STORE.getIndex(lName);
    res = (lIndex != 0);
  }

  GENV_ITEMFACTORY->createBoolean(result, res);
  STACK_PUSH(true, state);

  STACK_END (state);
}


/*******************************************************************************

********************************************************************************/
AvailableIndexesIteratorState::~AvailableIndexesIteratorState()
{
  if ( nameItState != NULL ) 
  {
    nameItState->close();
    nameItState = NULL;
  }
}


void
AvailableIndexesIteratorState::init(PlanState& planState)
{
  PlanIteratorState::init(planState);
  nameItState = NULL;
}


void
AvailableIndexesIteratorState::reset(PlanState& planState)
{
  PlanIteratorState::reset(planState);
  if ( nameItState != NULL ) 
  {
    nameItState->close();
    nameItState = NULL;
  }
}


bool
AvailableIndexesIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  store::Item_t              nameItem;

  AvailableIndexesIteratorState * state;
  DEFAULT_STACK_INIT(AvailableIndexesIteratorState, state, planState);

  for ((state->nameItState = GENV_STORE.listIndexNames())->open ();
       state->nameItState->next(nameItem); )
  {
    result = nameItem;
    STACK_PUSH( true, state);
  }

  state->nameItState->close();

  STACK_END (state);
}


/*******************************************************************************

*******************************************************************************/
bool IsActivatedICIterator::nextImpl(store::Item_t& result,
                                     PlanState& planState) const
{
  PlanIteratorState  *state;
  store::Item_t       lName;
  bool                res = false;

  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  consumeNext(lName, theChildren[0].getp(), planState);

  {
    store::IC_t lIc = GENV_STORE.getIC(lName);
    res = (lIc != 0);
  }

  GENV_ITEMFACTORY->createBoolean(result, res);
  STACK_PUSH(true, state );

  STACK_END (state);
}


/*******************************************************************************

*******************************************************************************/
ActivatedICsIteratorState::~ActivatedICsIteratorState()
{
  if ( nameItState != NULL ) 
  {
    nameItState->close();
    nameItState = NULL;
  }
}


void
ActivatedICsIteratorState::init(PlanState& planState)
{
  PlanIteratorState::init(planState);
  nameItState = NULL;
}


void
ActivatedICsIteratorState::reset(PlanState& planState)
{
  PlanIteratorState::reset(planState);
  if ( nameItState != NULL ) 
  {
    nameItState->close();
    nameItState = NULL;
  }
}

bool
ActivatedICsIterator::nextImpl(store::Item_t& result,
                               PlanState& planState) const
{
  store::Item_t              nameItem;

  ActivatedICsIteratorState * state;
  DEFAULT_STACK_INIT(ActivatedICsIteratorState, state, planState);

  for ((state->nameItState = GENV_STORE.listActiveICNames())->open ();
       state->nameItState->next(nameItem); )
  {
    result = nameItem;
    STACK_PUSH( true, state);
  }

  state->nameItState->close();

  STACK_END (state);
}


/*******************************************************************************

********************************************************************************/
bool IsDeclaredCollectionIterator::nextImpl(
    store::Item_t& aResult,
    PlanState& aPlanState) const
{
  store::Item_t      lName;

  PlanIteratorState* lState;
  DEFAULT_STACK_INIT(PlanIteratorState, lState, aPlanState);

  consumeNext(lName, theChildren[0].getp(), aPlanState);

  if (theSctx->lookup_collection(lName.getp()) == 0)
  {
    STACK_PUSH(GENV_ITEMFACTORY->createBoolean(aResult, false), lState);
  }
  else
  {
    STACK_PUSH(GENV_ITEMFACTORY->createBoolean(aResult, true), lState);
  }
  STACK_END (lState);
}


/*******************************************************************************

********************************************************************************/
DeclaredCollectionsIteratorState::~DeclaredCollectionsIteratorState()
{
  if ( nameItState != NULL )
  {
    nameItState->close();
    nameItState = NULL;
  }
}


void DeclaredCollectionsIteratorState::init(PlanState& planState)
{
  PlanIteratorState::init(planState);
  nameItState = NULL;
}


void DeclaredCollectionsIteratorState::reset(PlanState& planState)
{
  PlanIteratorState::reset(planState);
  if ( nameItState != NULL )
  {
    nameItState->close();
    nameItState = NULL;
  }
}


bool DeclaredCollectionsIterator::nextImpl(
    store::Item_t& aResult,
    PlanState& aPlanState) const
{
  store::Item_t lName;

  DeclaredCollectionsIteratorState* lState;
  DEFAULT_STACK_INIT(DeclaredCollectionsIteratorState, lState, aPlanState);

  for ((lState->nameItState = theSctx->collection_names())->open();
       lState->nameItState->next(lName); )
  {
    aResult = lName;
    STACK_PUSH(true, lState);
  }

  lState->nameItState->close();

  STACK_END(lState);
}


/*******************************************************************************

*******************************************************************************/
bool
IsDeclaredICIterator::nextImpl(store::Item_t& aResult, PlanState& aPlanState)
  const
{
  store::Item_t      lName;

  PlanIteratorState* lState;
  DEFAULT_STACK_INIT(PlanIteratorState, lState, aPlanState);

  consumeNext(lName, theChildren[0].getp(), aPlanState);
  if (theSctx->lookup_ic(lName.getp()) == 0) 
  {
    STACK_PUSH (GENV_ITEMFACTORY->createBoolean ( aResult, false ), lState);
  }
  else
  {
    STACK_PUSH (GENV_ITEMFACTORY->createBoolean ( aResult, true ), lState);
  }
  STACK_END (lState);
}


/*******************************************************************************

********************************************************************************/
bool
DeclaredICsIterator::nextImpl(store::Item_t& aResult, PlanState& aPlanState)
  const
{
  DeclaredICsIteratorState * lState;
  store::Item_t              lName;

  DEFAULT_STACK_INIT(DeclaredICsIteratorState, lState, aPlanState);

  for ((lState->nameItState = theSctx->ic_names())->open ();
       lState->nameItState->next(lName); )
  {
    aResult = lName;
    STACK_PUSH( true, lState);
  }

  lState->nameItState->close();

  STACK_END (lState);
}

DeclaredICsIteratorState::~DeclaredICsIteratorState()
{
  if ( nameItState != NULL ) {
    nameItState->close();
    nameItState = NULL;
  }
}

void DeclaredICsIteratorState::reset(PlanState& planState)
{
  PlanIteratorState::reset(planState);
  if ( nameItState != NULL ) {
    nameItState->close();
    nameItState = NULL;
  }
}


/*******************************************************************************

********************************************************************************/
bool IsDeclaredIndexIterator::nextImpl(
    store::Item_t& aResult,
    PlanState& aPlanState) const
{
  store::Item_t      lName;
  zorba::IndexDecl*  lDecl;

  PlanIteratorState* lState;
  DEFAULT_STACK_INIT(PlanIteratorState, lState, aPlanState);

  consumeNext(lName, theChildren[0].getp(), aPlanState);

  if ((lDecl = theSctx->lookup_index(lName.getp())) && !lDecl->isTemp())
  {
    STACK_PUSH(GENV_ITEMFACTORY->createBoolean(aResult, true), lState);
  }
  else
  {
    STACK_PUSH(GENV_ITEMFACTORY->createBoolean(aResult, false), lState);
  }

  STACK_END(lState);
}


/*******************************************************************************

********************************************************************************/
DeclaredIndexesIteratorState::~DeclaredIndexesIteratorState()
{
  if (nameItState != NULL)
  {
    nameItState->close();
    nameItState = NULL;
  }
}


void DeclaredIndexesIteratorState::reset(PlanState& planState)
{
  PlanIteratorState::reset(planState);
  if (nameItState != NULL)
  {
    nameItState->close();
    nameItState = NULL;
  }
}


bool DeclaredIndexesIterator::nextImpl(
    store::Item_t& aResult,
    PlanState& aPlanState) const
{
  store::Item_t lName;

  DeclaredIndexesIteratorState* lState;
  DEFAULT_STACK_INIT(DeclaredIndexesIteratorState, lState, aPlanState);

  for ((lState->nameItState = theSctx->index_names())->open();
       lState->nameItState->next(lName); )
  {
    if (theSctx->lookup_index(lName.getp())->isTemp()) 
    {
      continue;
    }
    aResult = lName;
    STACK_PUSH(true, lState);
  }

  lState->nameItState->close();

  STACK_END(lState);
}


/*******************************************************************************
  14.8.5 fn:uri-collection
********************************************************************************/
FnURICollectionIteratorState::~FnURICollectionIteratorState()
{
  if(theIterator != NULL)
  {
    if(theIteratorOpened)
    {
      theIterator->close();
      theIteratorOpened = false;
    }
    theIterator = NULL;
  }
}

void FnURICollectionIteratorState::init(PlanState& planState)
{
  PlanIteratorState::init(planState);
  theIterator = NULL;
}

void FnURICollectionIteratorState::reset(PlanState& planState)
{
  PlanIteratorState::reset(planState);

  if(theIterator != NULL)
  {
    if(theIteratorOpened)
    {
      theIterator->close();
      theIteratorOpened = false;
    }
    theIterator = NULL;
  }
}

bool FnURICollectionIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  store::Item_t lURI, resolvedURIItem, lIte;
  store::Collection_t coll;
  std::unique_ptr<internal::Resource> lResource;
  internal::CollectionResource* lCollResource;
  zstring resolvedURIString;
  zstring lErrorMessage;
  zstring docuri;

  FnURICollectionIteratorState* state;
  DEFAULT_STACK_INIT(FnURICollectionIteratorState, state, planState);

  if(theChildren.size() == 1 &&
    consumeNext(lURI, theChildren[0].getp(),planState))
  {
    try
    {
      resolvedURIString= theSctx->resolve_relative_uri(lURI->getStringValue());
    }
    catch (ZorbaException const&)
    {
      throw XQUERY_EXCEPTION(
        err::FODC0004,
        ERROR_PARAMS(lURI->getStringValue(), ZED( BadAnyURI ) ),
        ERROR_LOC( loc )
      );
    }
  }
  else
  {
    resolvedURIItem = planState.theGlobalDynCtx->get_default_collection();

    if ( NULL == resolvedURIItem )
      throw XQUERY_EXCEPTION(
      err::FODC0002,
      ERROR_PARAMS( ZED( DefaultCollection), ZED( NotDefInDynamicCtx ) ),
      ERROR_LOC( loc )
    );

    resolvedURIString = theSctx->resolve_relative_uri(resolvedURIItem->getStringValue());
  }

  lResource = theSctx->resolve_uri(resolvedURIString,
                                  internal::EntityData::COLLECTION,
                                  lErrorMessage);

  lCollResource = dynamic_cast<internal::CollectionResource*>(lResource.get());

  if( lCollResource == 0 || !(coll = lCollResource->getCollection()) )
  {
    throw XQUERY_EXCEPTION(
      err::FODC0002,
      ERROR_PARAMS( resolvedURIString, lErrorMessage ),
      ERROR_LOC( loc )
    );
  }

  // return collection nodes
  state->theIterator = coll->getIterator();
  ZORBA_ASSERT(state->theIterator != NULL);
  state->theIterator->open();
  state->theIteratorOpened = true;

  //return the DocumentURI of the Collection
  while(state->theIterator->next(lIte))
  {
    lIte->getDocumentURI(docuri);
    if(!docuri.empty())
    {
      STACK_PUSH(GENV_ITEMFACTORY->createAnyURI(result, docuri), state);
    }
  }

  //close iterator
  state->theIterator->close();
  state->theIteratorOpened = false;

  STACK_PUSH(false, state);
  STACK_END(state);

}


} // namespace zorba
/* vim:set et sw=2 ts=2: */
