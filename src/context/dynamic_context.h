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
#ifndef ZORBA_DYNAMIC_CONTEXT_H
#define ZORBA_DYNAMIC_CONTEXT_H

#include <zorba/external_function_parameter.h>
#include <zorba/util/calendar.h>

#include "util/locale.h"
#include "zorbautils/hashmap_zstring.h"
#include "zorbautils/hashmap_itemp.h"

#include "common/shared_types.h"

#include "store/api/shared_types.h"


namespace zorba {


template <class V> class ItemPointerHashMap;


/*******************************************************************************
  The dynamic context stores the following info:

  - The current date and time, i.e the date and time when the dynamic context
    was created.
  - The current time in mili-secs.
  - The local timezone (obtained from the machine where zorba is running)
  - The default collection uri (the one to use when fn:collection is invoked
    with no argument)
  - The context item for the whole query, if any.
  - The position of the context item, if any.
  - A hasmap mapping the name of each prolog or block-local variable to its
    current value, which is either a single item or a temp sequence.
  - A map mapping the uri of each index to the store object representing the
    index (store::Index_t)
********************************************************************************/
class dynamic_context
{
  friend class DebugIterator;

public:

  static enum ID_VARS
  {
    IDVAR_CONTEXT_ITEM = 1,
    IDVAR_CONTEXT_ITEM_POSITION,
    IDVAR_CONTEXT_ITEM_SIZE,
    MAX_IDVARS_RESERVED
  } IDVARS_RESERVED;

  struct VarValue
  {
    typedef enum
    {
      undeclared,
      declared,
      item,
      temp_seq,
    } ValueState;

    union
    {
      store::Item     * item;
      store::TempSeq  * temp_seq;
    }           theValue;

    ValueState  theState;
    bool        theIsExternalOrLocal;

    VarValue() : theState(undeclared), theIsExternalOrLocal(false)
    {
      theValue.item = NULL;
    }

    VarValue(const VarValue& other);

    ~VarValue();

    bool isSet() const { return (theState == item || theState == temp_seq); }

    bool hasItemValue() const { return (theState == item); }

    bool hasSeqValue() const { return (theState == temp_seq); }
  };

protected:

  struct dctx_value_t
  {
    typedef enum
    {
      no_val,
      ext_func_param, // params that can be used by ext. functions
      ext_func_param_typed
    } val_type_t;

    val_type_t    type;
    void        * func_param;
  };

  ZSTRING_HASH_MAP(dctx_value_t, ValueMap);

  ITEM_PTR_HASH_MAP(store::Index_t, IndexMap);

  typedef std::map<const zstring, const zstring> EnvVarMap;

protected:
  dynamic_context            * theParent;

  store::Item_t                theCurrentDateTimeStamp;

  long                         theTimezone;

  store::Item_t                theDefaultCollectionUri;

  std::vector<VarValue>        theVarValues;

  mutable ValueMap           * keymap;

  IndexMap                   * theAvailableIndices;

  IndexMap                   * theAvailableMaps;

  //MODIFY
  EnvVarMap                  * theEnvironmentVariables;

  locale::iso639_1::type       theLang;
  locale::iso3166_1::type      theCountry;
  time::calendar::type         theCalendar;

  uint64_t                     theSnapshotID;

public:
  double                       theDocLoadingUserTime;
  double                       theDocLoadingTime;

public:
  dynamic_context(dynamic_context* parent = NULL);

  ~dynamic_context();

  dynamic_context* getParent() { return theParent; }

  const dynamic_context* getParent() const { return theParent; }

  store::Item_t get_default_collection() const;

  void set_default_collection(const store::Item_t& default_collection_uri);

  void set_current_date_time(const store::Item_t&);

  void reset_current_date_time();

  store::Item_t get_current_date_time() const;

  store::Item_t get_current_time_millis() const;

  void set_environment_variables();

  store::Item_t get_environment_variable(const zstring& varname);

  store::Iterator_t available_environment_variables();

  void set_implicit_timezone(long tzone_seconds);

  long get_implicit_timezone() const;

  void set_locale(
      locale::iso639_1::type lang,
      locale::iso3166_1::type country)
  {
    theLang = lang;
    theCountry = country;
  }

  void get_locale(
      locale::iso639_1::type* lang,
      locale::iso3166_1::type* country)
  {
    if ( lang )
      *lang = theLang;
    if ( country )
      *country = theCountry;
  }

  void set_calendar(time::calendar::type calendar) { theCalendar = calendar; }

  time::calendar::type get_calendar() const { return theCalendar; }

  const std::vector<VarValue>& get_variables() const { return theVarValues; }

  void add_variable(ulong varid, store::Item_t& value);

  void add_variable(ulong varid, store::Iterator_t& value);

  void declare_variable(ulong varid, bool external);

  void set_variable(
      ulong varid,
      const store::Item_t& varname,
      const QueryLoc& loc,
      store::Item_t& value);

  void set_variable(
      ulong varid,
      const store::Item_t& varname,
      const QueryLoc& loc,
      store::Iterator_t& value);

  void unset_variable(
      ulong varid,
      const store::Item_t& varname,
      const QueryLoc& loc);

  void get_variable(
        ulong varid,
        const store::Item_t& varname,
        const QueryLoc& loc,
        store::Item_t& itemValue,
        store::TempSeq_t& seqValue) const;

  bool is_set_variable(ulong varid) const;

  bool exists_variable(ulong varid) const;

  ulong get_next_var_id() const;

  store::Index* getIndex(store::Item* qname) const;

  void bindIndex(store::Item* qname, store::Index_t& index);

  void unbindIndex(store::Item* qname);

  store::Index* getMap(store::Item* qname, bool lookupParent = true) const;

  void bindMap(store::Item* qname, store::Index_t& index);

  void unbindMap(store::Item* qname);

  void getMapNames(std::vector<store::Item_t>& names) const;

  /**
   * Lists all active integrity constraints.
   */
  store::Iterator_t listActiveICNames();

  /**
   * Returns IC object representing the integriti constraint.
   * NULL if IC is not active.
   */
  store::IC* getIC(const store::Item* qname);

  bool addExternalFunctionParam(const std::string& aName, void* aValue);

  bool getExternalFunctionParam(const std::string& aName, void*& aValue) const;

  bool addExternalFunctionParameter(
     const std::string& aName,
     ExternalFunctionParameter* aValue) const;

  ExternalFunctionParameter* getExternalFunctionParameter(
      const std::string& aName) const;
  
#ifndef NDEBUG
  /**
   * Prints debugging information regarding this dynamic context. Currently
   * shows the list of declared variables.
   */ 
  std::string toString();
#endif  

  uint64_t getSnapshotID() const;
  void changeSnapshot();

protected:
  bool lookup_once(const std::string& key, dctx_value_t& val) const
  {
    if (keymap)
    {
      ValueMap::iterator lIter = keymap->find(key);
      if (lIter != keymap->end())
      {
        val = lIter.getValue();
        return true;
      }
    }
    return false;
  }

  bool context_value(const std::string& key, dctx_value_t& val) const
  {
    if (lookup_once(key, val))
    {
      return true;
    }
    return theParent == NULL ? false : theParent->context_value(key, val);
  }

  bool context_value(const std::string& key, dctx_value_t& val, ValueMap** map)
  {
    if (lookup_once (key, val))
    {
      if (map != NULL) *map = keymap;
      return true;
    }
    return theParent == NULL ? false : theParent->context_value(key, val, map);
  }

  void destroy_dctx_value(dctx_value_t *);
};


} /* namespace zorba */
#endif /* ZORBA_DYNAMIC_CONTEXT_H */

/*
 * Local variables:
 * mode: c++
 * End:
 */
/* vim:set et sw=2 ts=2: */
