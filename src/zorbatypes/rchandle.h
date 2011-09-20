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
#pragma once
#ifndef ZORBA_RCHANDLE_H
#define ZORBA_RCHANDLE_H

#include <sstream>
#include <errno.h>

#include <zorba/config.h>

#include "common/common.h"

#include "zorbautils/fatal.h"

#include "zorbatypes/rclock.h"

#include "zorbaserialization/class_serializer.h"
#include "zorbaserialization/archiver.h"


namespace zorba
{

/*******************************************************************************
  
  Base class for reference counted objects

  Any class wishing to take advantage of automatic reference counting
  must inherit from this class.

  rcobject encapsulates the reference count, as well as functions
  for incrementing and decrementing the count. It also contains code
  for destroying a value when it is no longer in use, i.e., when its
  reference count becomes 0.

********************************************************************************/
class ZORBA_DLL_PUBLIC RCObject : public ::zorba::serialization::SerializeBaseClass
{
protected:
  mutable long  theRefCount;

public:
  SERIALIZABLE_ABSTRACT_CLASS(RCObject);

  RCObject(::zorba::serialization::Archiver& ar)  
    :
    ::zorba::serialization::SerializeBaseClass(),
    theRefCount(0)
  {
  }

  void serialize(::zorba::serialization::Archiver& ar)
  {
    if(!ar.is_serializing_out())
      theRefCount = 0;
  }

public:
  RCObject()
    :
    ::zorba::serialization::SerializeBaseClass(),
    theRefCount(0)
  {
  }

  RCObject(const RCObject&) 
    :
    ::zorba::serialization::SerializeBaseClass(),
    theRefCount(0) 
  {
  }

  virtual ~RCObject() { }

  RCObject& operator=(const RCObject&) { return *this; }

  virtual void free() { delete this; }

  long getRefCount() const { return theRefCount; }

  long* getSharedRefCounter() const { ZORBA_FATAL(0, ""); return NULL; } 
 
  SYNC_CODE(RCLock* getRCLock() const { ZORBA_FATAL(0, ""); return NULL; });

  void addReference(long* sharedCounter SYNC_PARAM2(RCLock* lock)) const;

  void removeReference(long* sharedCounter SYNC_PARAM2(RCLock* lock));
};


/*******************************************************************************

********************************************************************************/
class ZORBA_DLL_PUBLIC SimpleRCObject : public RCObject
{
public:
  SERIALIZABLE_ABSTRACT_CLASS(SimpleRCObject)
  SERIALIZABLE_CLASS_CONSTRUCTOR2(SimpleRCObject, RCObject)
  void serialize(::zorba::serialization::Archiver &ar)
  {
    if(!ar.is_serializing_out())
      theRefCount = 0;
  }

public:
  SimpleRCObject() : RCObject() { }

  SimpleRCObject(const SimpleRCObject& rhs) : RCObject(rhs) { }

  void free() { delete this; }

  long* getSharedRefCounter() const  { return NULL; } 

  SYNC_CODE(RCLock* getRCLock() const { return NULL; })

  SimpleRCObject& operator=(const SimpleRCObject&) { return *this; }
};



/*******************************************************************************

  rchandle = r(ef)c(ount)handle

  Template class for smart pointers-to-T objects. T must support the RCObject
  interface, typically by inheriting from RCObject

********************************************************************************/
template<class T> class rchandle
{
protected:
  T  * p;

public:
  union union_T
  {
    T** t;
    void** v;
  };

public:
  rchandle(T* ptr = 0) : p(ptr)
  {
    init(); 
  }

  rchandle(rchandle const& rhs) : p(rhs.getp())
  {
    init();
  }

  ~rchandle()
  {
    if (p)
      p->removeReference(p->getSharedRefCounter() SYNC_PARAM2(p->getRCLock()));
    p = 0;
  }

  bool isNull () const { return p == NULL; }

  void setNull() { p = NULL;}

  T* getp() const { return p; }

  union_T getp_ref() { union_T u_t; u_t.t = &p; return u_t;}

  // rchandle const-ness is unclear. The implicit operators are more
  // restrictive than the explicit cast() and getp() methods.
  operator T* ()              { return getp(); }

  operator const T * () const { return getp(); }

  T* operator->() const       { return p; } 

  T& operator*() const        { return *p; } 

  bool operator!() const      { return !p; }

  bool operator==(rchandle const& h) const  { return p == h.p; }
  bool operator!=(rchandle const& h) const  { return p != h.p; }
  bool operator==(T const* pp) const        { return p == pp; } 
  bool operator!=(T const* pp) const        { return p != pp; }
  bool operator<(const rchandle& h) const   { return p < h.p; }


  template <class otherT> rchandle<otherT> cast() const 
  {
    return rchandle<otherT> (static_cast<otherT *> (p));
  }

  template <class otherT> rchandle<otherT> dyn_cast() const 
  {
    return rchandle<otherT> (dynamic_cast<otherT *> (p));
  }  

  template <class otherT> operator rchandle<otherT> () 
  {
    return cast<otherT> ();
  }

  template <class otherT> operator const rchandle<otherT> () const 
  {
    return cast<otherT> ();
  }

  rchandle& operator=(const T* rhs) 
  {
    if (p != rhs)
    {
      if (p) p->removeReference(p->getSharedRefCounter()
                                SYNC_PARAM2(p->getRCLock()));
      p = const_cast<T*>(rhs);
      init();
    }
    return *this;
  }

  template <class otherT> rchandle& operator=(const otherT* rhs)
  {
    if (p != rhs)
    {
      if (p) p->removeReference(p->getSharedRefCounter()
                                SYNC_PARAM2(p->getRCLock()));
      p = static_cast<T*>(const_cast<otherT*>(rhs));
      init();
    }
    return *this;
  }

  rchandle& operator=(rchandle const& rhs) 
  {
    return assign (rhs);
  }

  template <class otherT> rchandle& operator=(rchandle<otherT> const& rhs) 
  {
    return assign (rhs);
  }

  template <class otherT> rchandle& transfer(rchandle<otherT>& rhs)
  {
    if (p != rhs.getp())
    {
      if (p) p->removeReference(p->getSharedRefCounter()
                                SYNC_PARAM2(p->getRCLock()));
      p = static_cast<T*>(rhs.getp());
      rhs.setNull();
    }
    return *this;
  }

  rchandle& transfer(rchandle& rhs)
  {
    if (p != rhs.p)
    {
      if (p) p->removeReference(p->getSharedRefCounter()
                                SYNC_PARAM2(p->getRCLock()));
      p = rhs.p;
      rhs.p = NULL;
    }
    return *this;
  }

  T* release()
  {
    T* tmp = p;
    p = NULL;
    return tmp;
  }

public:
  std::string debug() const
  {
    std::ostringstream oss;
    oss << "rchandle[refcount=" << p->getRefCount() << ']';
    return oss.str();
  }

protected:
  void init()
  {
    if (p == 0) return;
    p->addReference(p->getSharedRefCounter() SYNC_PARAM2(p->getRCLock()));
  }


  template <class otherT> rchandle& assign(const rchandle<otherT>& rhs)
  {
    if (p != rhs.getp())
    {
      if (p) p->removeReference(p->getSharedRefCounter()
                                SYNC_PARAM2(p->getRCLock()));
      p = static_cast<T*>(rhs.getp());
      init();
    }
    return *this;
  }

};

namespace ztd {

template<typename T> inline
std::string to_string( rchandle<T> const &r ) {
  return !r ? "<null>" : to_string( *r );
}

template<typename T,class OutputStringType> inline
void to_string( rchandle<T> const &r, OutputStringType *out ) {
  if ( !r )
    *out = "<null>";
  else
    to_string( *r, out );
}

} // namespace ztd

/*******************************************************************************

********************************************************************************/
template<class T> class const_rchandle : protected rchandle<T> 
{
public:
  const_rchandle (const T *_p = 0) : rchandle<T> (const_cast<T *> (_p)) {}

  const_rchandle (const const_rchandle &rhs) : rchandle<T> (rhs) {}

  const_rchandle (rchandle<T> &rhs) : rchandle<T> (rhs) {}

  const_rchandle(::zorba::serialization::Archiver &ar) {}

  const_rchandle& operator= (const const_rchandle &rhs) {
    this->assign (rhs);
    return *this;
  }

public:
  bool isNull () const        { return rchandle<T>::isNull(); }
  void setNull()              { rchandle<T>::setNull();}

  const T* getp () const { return rchandle<T>::getp (); }
  typename rchandle<T>::union_T getp_ref()             { return rchandle<T>::getp_ref(); }
  operator const T * () const { return rchandle<T>::getp (); }

  const T* operator->() const { return getp(); } 
  const T& operator*() const  { return *getp(); }

  bool operator== (const_rchandle h) const  { return rchandle<T>::operator== (h); }
  bool operator!= (const_rchandle h) const  { return rchandle<T>::operator!= (h); }
  bool operator== (const T * pp) const      { return rchandle<T>::operator== (pp); } 
  bool operator!= (const T * pp) const      { return rchandle<T>::operator!= (pp); } 
  bool operator< (const_rchandle h) const   { return rchandle<T>::operator<  (h); }
};



/*******************************************************************************

********************************************************************************/
namespace RCHelper 
{
  template<class T> 
  static void addReference(T *t)
  {
    t->addReference(t->getSharedRefCounter()
                    SYNC_PARAM2(t->getRCLock()));
  }

  template<class T> 
  static void removeReference(T *t)
  {
    t->removeReference(t->getSharedRefCounter()
                       SYNC_PARAM2(t->getRCLock()));
  }

  template<class T> 
  static void addReference(rchandle<T> &t)
  {
    addReference(t.getp());
  }
  
  template<class T> 
  static void removeReference(rchandle<T> &t)
  {
    removeReference(t.getp());
  }
};

} // namespace zorba

#endif
/*
 * Local variables:
 * mode: c++
 * End:
 */
/* vim:set et sw=2 ts=2: */
