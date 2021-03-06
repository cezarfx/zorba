/*
 * Copyright 2006-2012 The FLWOR Foundation.
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


#include "stdafx.h"
#include "zorbatypes/rchandle.h"
#include "zorbatypes/zstring.h"
#include "runtime/visitors/planiter_visitor.h"
#include "runtime/uris/uris.h"
#include "system/globalenv.h"



namespace zorba {

// <DecodeURIIterator>
SERIALIZABLE_CLASS_VERSIONS(DecodeURIIterator)

void DecodeURIIterator::serialize(::zorba::serialization::Archiver& ar)
{
  serialize_baseclass(ar,
  (NaryBaseIterator<DecodeURIIterator, PlanIteratorState>*)this);
}


void DecodeURIIterator::accept(PlanIterVisitor& v) const
{
  v.beginVisit(*this);

  std::vector<PlanIter_t>::const_iterator lIter = theChildren.begin();
  std::vector<PlanIter_t>::const_iterator lEnd = theChildren.end();
  for ( ; lIter != lEnd; ++lIter ){
    (*lIter)->accept(v);
  }

  v.endVisit(*this);
}

DecodeURIIterator::~DecodeURIIterator() {}


zstring DecodeURIIterator::getNameAsString() const {
  return "fn-zorba-uri:decode";
}
// </DecodeURIIterator>


// <ParseURIIterator>
SERIALIZABLE_CLASS_VERSIONS(ParseURIIterator)

void ParseURIIterator::serialize(::zorba::serialization::Archiver& ar)
{
  serialize_baseclass(ar,
  (NaryBaseIterator<ParseURIIterator, PlanIteratorState>*)this);
}


void ParseURIIterator::accept(PlanIterVisitor& v) const
{
  v.beginVisit(*this);

  std::vector<PlanIter_t>::const_iterator lIter = theChildren.begin();
  std::vector<PlanIter_t>::const_iterator lEnd = theChildren.end();
  for ( ; lIter != lEnd; ++lIter ){
    (*lIter)->accept(v);
  }

  v.endVisit(*this);
}

ParseURIIterator::~ParseURIIterator() {}


zstring ParseURIIterator::getNameAsString() const {
  return "fn-zorba-uri:parse";
}
// </ParseURIIterator>


// <SerializeURIIterator>
SERIALIZABLE_CLASS_VERSIONS(SerializeURIIterator)

void SerializeURIIterator::serialize(::zorba::serialization::Archiver& ar)
{
  serialize_baseclass(ar,
  (NaryBaseIterator<SerializeURIIterator, PlanIteratorState>*)this);
}


void SerializeURIIterator::accept(PlanIterVisitor& v) const
{
  v.beginVisit(*this);

  std::vector<PlanIter_t>::const_iterator lIter = theChildren.begin();
  std::vector<PlanIter_t>::const_iterator lEnd = theChildren.end();
  for ( ; lIter != lEnd; ++lIter ){
    (*lIter)->accept(v);
  }

  v.endVisit(*this);
}

SerializeURIIterator::~SerializeURIIterator() {}


zstring SerializeURIIterator::getNameAsString() const {
  return "fn-zorba-uri:serialize";
}
// </SerializeURIIterator>



}


