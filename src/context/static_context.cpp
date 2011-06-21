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

#include <assert.h>
#include <algorithm>
#include <memory>

#include <zorba/external_module.h>
#include <zorba/serialization_callback.h>

#include "zorbaserialization/serialization_engine.h"

#include "zorbamisc/ns_consts.h"
#include "util/string_util.h"

#define ZORBA_UTILS_HASHMAP_WITH_SERIALIZATION
#include "zorbautils/hashmap_itemp.h"
#undef ZORBA_UTILS_HASHMAP_WITH_SERIALIZATION

#include "context/static_context_consts.h"
#include "context/static_context.h"
#include "context/root_static_context.h"
#include "context/uri_resolver_wrapper.h"
#include "context/standard_uri_resolvers.h"
#include "context/dynamic_loader.h"
#include "context/decimal_format.h"
#include "context/sctx_map_iterator.h"

#include "compiler/expression/expr_base.h"
#include "compiler/expression/var_expr.h"
#ifndef ZORBA_NO_FULL_TEXT
#include "compiler/expression/ftnode.h"
#endif /* ZORBA_NO_FULL_TEXT */
#include "compiler/xqddf/collection_decl.h"
#include "compiler/xqddf/value_index.h"
#include "compiler/xqddf/value_ic.h"

#include "zorbatypes/collation_manager.h"
#include "zorbatypes/URI.h"

#include "api/unmarshaller.h"
#include "api/auditimpl.h"

#include "api/uri_resolver_wrappers.h"
#include "diagnostics/xquery_diagnostics.h"

#include "system/globalenv.h"

#include "types/typemanager.h"
#include "types/casting.h"
#include "types/typeops.h"
#include "types/schema/validate.h"

#include "functions/function.h"
#include "functions/library.h"
#include "functions/signature.h"

#include "store/api/store.h"
#include "store/api/item_factory.h"
#include "store/api/iterator.h"

#include "zorba/module_import_checker.h"


using namespace std;
using namespace zorba::locale;

namespace zorba
{

#define ITEM_FACTORY (GENV.getStore().getItemFactory())


SERIALIZABLE_CLASS_VERSIONS(BaseUriInfo)
END_SERIALIZABLE_CLASS_VERSIONS(BaseUriInfo)

SERIALIZABLE_CLASS_VERSIONS(FunctionInfo)
END_SERIALIZABLE_CLASS_VERSIONS(FunctionInfo)

SERIALIZABLE_CLASS_VERSIONS(PrologOption)
END_SERIALIZABLE_CLASS_VERSIONS(PrologOption)

SERIALIZABLE_CLASS_VERSIONS(static_context::ctx_module_t)
END_SERIALIZABLE_CLASS_VERSIONS(static_context::ctx_module_t)

SERIALIZABLE_CLASS_VERSIONS(static_context)
END_SERIALIZABLE_CLASS_VERSIONS(static_context)


/**************************************************************************//**

*******************************************************************************/
void BaseUriInfo::serialize(::zorba::serialization::Archiver& ar)
{
  ar & thePrologBaseUri;
  ar & theApplicationBaseUri;
  ar & theEntityRetrievalUri;
  ar & theEncapsulatingEntityUri;

  ar & theBaseUri;

  ar & theHavePrologBaseUri;
  ar & theHaveApplicationBaseUri;
  ar & theHaveEntityRetrievalUri;
  ar & theHaveEncapsulatingEntityUri;

  ar & theHaveBaseUri;
}


/**************************************************************************//**

*******************************************************************************/
FunctionInfo::FunctionInfo()
  :
  theIsDisabled(false)
{
}


FunctionInfo::FunctionInfo(const function_t& f, bool disabled)
  :
  theFunction(f),
  theIsDisabled(disabled)
{
}


FunctionInfo::FunctionInfo(::zorba::serialization::Archiver& ar)
  :
  ::zorba::serialization::SerializeBaseClass()
{
}


/**************************************************************************//**

*******************************************************************************/
FunctionInfo::~FunctionInfo()
{
}


/**************************************************************************//**

*******************************************************************************/
void FunctionInfo::serialize(::zorba::serialization::Archiver& ar)
{
  ar & theFunction;
  ar & theIsDisabled;
}


/**************************************************************************//**

*******************************************************************************/
void PrologOption::serialize(::zorba::serialization::Archiver& ar)
{
  ar & theName;
  ar & theValue;
}


/**************************************************************************//**

*******************************************************************************/
void static_context::ctx_module_t::serialize(serialization::Archiver& ar)
{
  if(ar.is_serializing_out())
  {
    // serialize out: the uri of the module that is used in this plan

    zstring lURI = Unmarshaller::getInternalString(module->getURI());
    ar.set_is_temp_field(true);
    ar.dont_allow_delay();
    ar & lURI;
    ar.set_is_temp_field(false);
    ar & dyn_loaded_module;
    ar & sctx;
  }
  else
  {
    // serialize in: load the serialized uri of the module and
    //               get the externalmodule from the user's
    //               registered serialization callback
    zstring lURI;
    ar.set_is_temp_field(true);
    ar & lURI;
    ar.set_is_temp_field(false);
    ar & dyn_loaded_module;
    ar & sctx;

    if (dyn_loaded_module)
    {
      ZORBA_ASSERT(sctx);
      module = DynamicLoader::getExternalModule(lURI, *sctx);

      // no way to get the module
      if (!module)
      {
        throw ZORBA_EXCEPTION(
          zerr::ZCSE0013_UNABLE_TO_LOAD_QUERY,
          ERROR_PARAMS( ZED( NoExternalModuleFromDLL ), lURI )
        );
      }
    }
    else
    {
      // class registered by the user
      SerializationCallback* lCallback = ar.getUserCallback();
      if (!lCallback)
      {
        throw ZORBA_EXCEPTION(
          zerr::ZCSE0013_UNABLE_TO_LOAD_QUERY,
          ERROR_PARAMS( ZED( NoSerializationCallbackForModule ), lURI )
        );
      }

      // the life-cycle of the module is managed by the user
      module = lCallback->getExternalModule(lURI.str());
      if (!module)
      {
        throw ZORBA_EXCEPTION(
          zerr::ZCSE0013_UNABLE_TO_LOAD_QUERY,
          ERROR_PARAMS( ZED( NoRegisteredSerializationCallback_2 ), lURI )
        );
      }
    }
  }
}


/***************************************************************************//**
  Target namespaces of zorba builtin modules
********************************************************************************/
#define NS_PRE static_context::ZORBA_NS_PREFIX

const zstring
static_context::DOT_VAR_NAME = "$$dot";

const zstring
static_context::DOT_POS_VAR_NAME = "$$pos";

const zstring
static_context::DOT_SIZE_VAR_NAME = "$$last-idx";

const zstring
static_context::W3C_NS_PREFIX = "http://www.w3.org/";

const zstring
static_context::ZORBA_NS_PREFIX = "http://www.zorba-xquery.com/";

const zstring
static_context::W3C_FN_NS = W3C_NS_PREFIX + "2005/xpath-functions";

const zstring
static_context::ZORBA_MATH_FN_NS = NS_PRE + "modules/math";

const zstring
static_context::ZORBA_BASE64_FN_NS = NS_PRE + "modules/converters/base64";

const zstring
static_context::ZORBA_NODEREF_FN_NS = NS_PRE + "modules/node-reference";

const zstring
static_context::ZORBA_STORE_DYNAMIC_COLLECTIONS_DDL_FN_NS
  = NS_PRE + "modules/store/dynamic/collections/ddl";

const zstring
static_context::ZORBA_STORE_DYNAMIC_COLLECTIONS_DML_FN_NS
  = NS_PRE + "modules/store/dynamic/collections/dml";

const zstring
static_context::ZORBA_STORE_STATIC_COLLECTIONS_DDL_FN_NS
  = NS_PRE + "modules/store/static/collections/ddl";

const zstring
static_context::ZORBA_STORE_STATIC_COLLECTIONS_DML_FN_NS
  = NS_PRE + "modules/store/static/collections/dml";

const zstring
static_context::ZORBA_STORE_STATIC_INDEXES_DDL_FN_NS
  = NS_PRE + "modules/store/static/indexes/ddl";

const zstring
static_context::ZORBA_STORE_STATIC_INDEXES_DML_FN_NS
  = NS_PRE + "modules/store/static/indexes/dml";

const zstring
static_context::ZORBA_STORE_STATIC_INTEGRITY_CONSTRAINTS_DDL_FN_NS
  = NS_PRE + "modules/store/static/integrity_constraints/ddl";

const zstring
static_context::ZORBA_STORE_STATIC_INTEGRITY_CONSTRAINTS_DML_FN_NS
  = NS_PRE + "modules/store/static/tegrity_constraints/dml";

const zstring
static_context::ZORBA_STORE_DYNAMIC_DOCUMENTS_FN_NS
  = static_context::ZORBA_NS_PREFIX + "modules/store/dynamic/documents";

const zstring
static_context::ZORBA_STORE_DYNAMIC_HASH_MAP_FN_NS
  = static_context::ZORBA_NS_PREFIX + "modules/store/data-structures/hash-map";

const zstring
static_context::ZORBA_SCHEMA_FN_NS = static_context::ZORBA_NS_PREFIX + "modules/schema";

const zstring
static_context::ZORBA_XQDOC_FN_NS = static_context::ZORBA_NS_PREFIX + "modules/xqdoc";

const zstring
static_context::ZORBA_RANDOM_FN_NS = static_context::ZORBA_NS_PREFIX + "modules/random";

const zstring
static_context::ZORBA_INTROSP_SCTX_FN_NS = static_context::ZORBA_NS_PREFIX + "modules/introspection/sctx";

const zstring
static_context::ZORBA_REFLECTION_FN_NS = NS_PRE + "modules/reflection";

const zstring
static_context::ZORBA_UTIL_FN_NS = NS_PRE + "zorba/util-functions";

const zstring
static_context::ZORBA_SCRIPTING_FN_NS = NS_PRE + "zorba/scripting";

const zstring
static_context::ZORBA_STRING_FN_NS = NS_PRE + "modules/string";

const zstring
static_context::ZORBA_FETCH_FN_NS = NS_PRE + "modules/fetch";

const zstring
static_context::ZORBA_NODE_FN_NS = NS_PRE + "modules/node";

/***************************************************************************//**
  Target namespaces of zorba reserved modules
********************************************************************************/
const zstring
static_context::XQUERY_OP_NS = ZORBA_NS_PREFIX + "internal/xquery-ops";

const zstring
static_context::ZORBA_OP_NS = ZORBA_NS_PREFIX + "internal/zorba-ops";


/***************************************************************************//**
  Static method to check if a given target namespace identifies a zorba
  builtin module.
********************************************************************************/
bool static_context::is_builtin_module(const zstring& ns)
{
  if (ns.compare(0, ZORBA_NS_PREFIX.size(), ZORBA_NS_PREFIX) == 0)
  {
    return (ns == ZORBA_MATH_FN_NS ||
            ns == ZORBA_BASE64_FN_NS ||
            ns == ZORBA_NODEREF_FN_NS ||
            ns == ZORBA_STORE_DYNAMIC_DOCUMENTS_FN_NS ||
            ns == ZORBA_STORE_DYNAMIC_HASH_MAP_FN_NS ||
            ns == ZORBA_STORE_DYNAMIC_COLLECTIONS_DDL_FN_NS ||
            ns == ZORBA_STORE_DYNAMIC_COLLECTIONS_DML_FN_NS ||
            ns == ZORBA_STORE_STATIC_COLLECTIONS_DDL_FN_NS ||
            ns == ZORBA_STORE_STATIC_COLLECTIONS_DML_FN_NS ||
            ns == ZORBA_STORE_STATIC_INDEXES_DDL_FN_NS ||
            ns == ZORBA_STORE_STATIC_INDEXES_DML_FN_NS ||
            ns == ZORBA_STORE_STATIC_INTEGRITY_CONSTRAINTS_DDL_FN_NS ||
            ns == ZORBA_STORE_STATIC_INTEGRITY_CONSTRAINTS_DML_FN_NS ||
            ns == ZORBA_SCHEMA_FN_NS ||
            ns == ZORBA_XQDOC_FN_NS ||
            ns == ZORBA_RANDOM_FN_NS ||
            ns == ZORBA_INTROSP_SCTX_FN_NS ||
            ns == ZORBA_REFLECTION_FN_NS ||
            ns == ZORBA_SCRIPTING_FN_NS ||
            ns == ZORBA_STRING_FN_NS ||
            ns == ZORBA_FETCH_FN_NS ||
            ns == ZORBA_NODE_FN_NS);
  }

  return false;
}


/***************************************************************************//**
  Static method to check if a given target namespace identifies a zorba
  builtin virtual module.
********************************************************************************/
bool static_context::is_builtin_virtual_module(const zstring& ns)
{
  if (ns.compare(0, ZORBA_NS_PREFIX.size(), ZORBA_NS_PREFIX) == 0)
  {
    return (ns == ZORBA_SCRIPTING_FN_NS ||
            ns == ZORBA_UTIL_FN_NS);
  }

  return false;
}


/***************************************************************************//**
  Static method to check if a given target namespace identifies a zorba non
  pure builtin module, i.e. a builtin module that, in addition to builtin
  external functions, contains variable declarations and/or udfs.
********************************************************************************/
bool static_context::is_non_pure_builtin_module(const zstring& ns)
{
  if (ns.compare(0, ZORBA_NS_PREFIX.size(), ZORBA_NS_PREFIX) == 0)
  {
    return (ns == ZORBA_MATH_FN_NS ||
            ns == ZORBA_INTROSP_SCTX_FN_NS ||
            ns == ZORBA_RANDOM_FN_NS);
  }

  return false;
}


/***************************************************************************//**
  Static method to check if a given target namespace identifies a zorba
  reserved module.
********************************************************************************/
bool static_context::is_reserved_module(const zstring& ns)
{
  if (ns.compare(0, ZORBA_NS_PREFIX.size(), ZORBA_NS_PREFIX) == 0)
  {
    return (ns == ZORBA_OP_NS || ns == XQUERY_OP_NS);
  }

  return false;
}


/***************************************************************************//**
  Default Constructor.
********************************************************************************/
static_context::static_context()
  :
  theParent(NULL),
  theTraceStream(NULL),
  theImportedBuiltinModules(NULL),
  theBaseUriInfo(NULL),
  theDocResolver(0),
  theExternalModulesMap(NULL),
  theNamespaceBindings(NULL),
  theHaveDefaultElementNamespace(false),
  theHaveDefaultFunctionNamespace(false),
  theVariablesMap(NULL),
  theFunctionMap(NULL),
  theFunctionArityMap(NULL),
  theAnnotationMap(NULL),
  theCollectionMap(NULL),
  theW3CCollectionMap(NULL),
  theIndexMap(NULL),
  theICMap(NULL),
  theDocumentMap(NULL),
  theCollationMap(NULL),
  theDefaultCollation(NULL),
  theOptionMap(NULL),
  theAuditEvent(&zorba::audit::NOP_EVENT_IMPL),
#ifndef ZORBA_NO_FULL_TEXT
  theFTMatchOptions(NULL),
#endif /* ZORBA_NO_FULL_TEXT */
  theXQueryVersion(StaticContextConsts::xquery_version_unknown),
  theXPathCompatibility(StaticContextConsts::xpath_unknown),
  theConstructionMode(StaticContextConsts::cons_unknown),
  theInheritMode(StaticContextConsts::inherit_unknown),
  thePreserveMode(StaticContextConsts::preserve_unknown),
  theOrderingMode(StaticContextConsts::ordering_unknown),
  theEmptyOrderMode(StaticContextConsts::empty_order_unknown),
  theBoundarySpaceMode(StaticContextConsts::boundary_space_unknown),
  theValidationMode(StaticContextConsts::validation_unknown),
  theDecimalFormats(NULL),
  theAllWarningsDisabled(false)
{
}


/*******************************************************************************

********************************************************************************/
static_context::static_context(static_context* parent)
  :
  theParent(parent),
  theTraceStream(NULL),
  theImportedBuiltinModules(NULL),
  theBaseUriInfo(NULL),
  theDocResolver(0),
  theExternalModulesMap(NULL),
  theNamespaceBindings(NULL),
  theHaveDefaultElementNamespace(false),
  theHaveDefaultFunctionNamespace(false),
  theVariablesMap(NULL),
  theFunctionMap(NULL),
  theFunctionArityMap(NULL),
  theAnnotationMap(NULL),
  theCollectionMap(0),
  theW3CCollectionMap(NULL),
  theIndexMap(NULL),
  theICMap(NULL),
  theDocumentMap(NULL),
  theCollationMap(NULL),
  theDefaultCollation(NULL),
  theOptionMap(NULL),
  theAuditEvent(&zorba::audit::NOP_EVENT_IMPL),
#ifndef ZORBA_NO_FULL_TEXT
  theFTMatchOptions(NULL),
#endif /* ZORBA_NO_FULL_TEXT */
  theXQueryVersion(StaticContextConsts::xquery_version_unknown),
  theXPathCompatibility(StaticContextConsts::xpath_unknown),
  theConstructionMode(StaticContextConsts::cons_unknown),
  theInheritMode(StaticContextConsts::inherit_unknown),
  thePreserveMode(StaticContextConsts::preserve_unknown),
  theOrderingMode(StaticContextConsts::ordering_unknown),
  theEmptyOrderMode(StaticContextConsts::empty_order_unknown),
  theBoundarySpaceMode(StaticContextConsts::boundary_space_unknown),
  theValidationMode(StaticContextConsts::validation_unknown),
  theDecimalFormats(NULL),
  theAllWarningsDisabled(false)
{
  if (theParent != NULL)
    RCHelper::addReference(theParent);
}


/*******************************************************************************

********************************************************************************/
static_context::static_context(::zorba::serialization::Archiver& ar)
  :
  SimpleRCObject(ar),
  theParent(NULL),
  theTraceStream(NULL),
  theImportedBuiltinModules(NULL),
  theBaseUriInfo(NULL),
  theDocResolver(0),
  theExternalModulesMap(NULL),
  theNamespaceBindings(NULL),
  theHaveDefaultElementNamespace(false),
  theHaveDefaultFunctionNamespace(false),
  theVariablesMap(NULL),
  theFunctionMap(NULL),
  theFunctionArityMap(NULL),
  theAnnotationMap(NULL),
  theCollectionMap(0),
  theW3CCollectionMap(NULL),
  theIndexMap(0),
  theICMap(0),
  theDocumentMap(NULL),
  theCollationMap(NULL),
  theDefaultCollation(NULL),
  theOptionMap(NULL),
  theAuditEvent(&zorba::audit::NOP_EVENT_IMPL),
#ifndef ZORBA_NO_FULL_TEXT
  theFTMatchOptions(NULL),
#endif /* ZORBA_NO_FULL_TEXT */
  theXQueryVersion(StaticContextConsts::xquery_version_unknown),
  theXPathCompatibility(StaticContextConsts::xpath_unknown),
  theConstructionMode(StaticContextConsts::cons_unknown),
  theInheritMode(StaticContextConsts::inherit_unknown),
  thePreserveMode(StaticContextConsts::preserve_unknown),
  theOrderingMode(StaticContextConsts::ordering_unknown),
  theEmptyOrderMode(StaticContextConsts::empty_order_unknown),
  theBoundarySpaceMode(StaticContextConsts::boundary_space_unknown),
  theValidationMode(StaticContextConsts::validation_unknown),
  theDecimalFormats(NULL),
  theAllWarningsDisabled(false)
{
}


/***************************************************************************//**
  Destructor.
********************************************************************************/
static_context::~static_context()
{
  if (theImportedBuiltinModules)
    delete theImportedBuiltinModules;

  if (theExternalModulesMap)
  {
    ExternalModuleMap::iterator ite = theExternalModulesMap->begin();
    ExternalModuleMap::iterator end = theExternalModulesMap->end();
    for(; ite != end; ++ite)
    {
      const ctx_module_t& val = ite.getValue();
      if (val.dyn_loaded_module)
      {
        val.module->destroy();
      }
    }

    delete theExternalModulesMap;
  }

  if (theVariablesMap)
    delete theVariablesMap;

  if (theFunctionMap)
    delete theFunctionMap;

  if (theFunctionArityMap)
  {
    FunctionArityMap::iterator ite = theFunctionArityMap->begin();
    FunctionArityMap::iterator end = theFunctionArityMap->end();
    for (; ite != end; ++ite)
    {
      delete (*ite).second;
    }

    delete theFunctionArityMap;
  }

  if (theAnnotationMap)
  {
    delete theAnnotationMap;
  }

  if (theW3CCollectionMap)
    delete theW3CCollectionMap;

  if (theCollectionMap)
    delete theCollectionMap;

  if (theIndexMap)
    delete theIndexMap;

  if (theICMap)
    delete theICMap;

  if (theNamespaceBindings)
    delete theNamespaceBindings;

  if (theDefaultCollation)
    delete theDefaultCollation;

  if (theCollationMap)
  {
    CollationMap::iterator ite = theCollationMap->begin();
    CollationMap::iterator end = theCollationMap->end();
    for ( ; ite != end ; ++ite)
    {
      delete ite->second;
    }

    delete theCollationMap;
    theCollationMap = 0;
  }

  if (theOptionMap)
    delete theOptionMap;

#ifndef ZORBA_NO_FULL_TEXT
  delete theFTMatchOptions;
#endif /* ZORBA_NO_FULL_TEXT */

  if (theDecimalFormats)
    delete theDecimalFormats;

  if (theBaseUriInfo)
    delete theBaseUriInfo;

  if (theParent)
    RCHelper::removeReference(theParent);

  {
    std::vector<ModuleImportChecker*>::iterator i =
        theModuleImportCheckers.begin();
    for (; i != theModuleImportCheckers.end(); ++i) {
      delete *i;
    }
  }
}


/*******************************************************************************

********************************************************************************/
void static_context::serialize_resolvers(serialization::Archiver& ar)
{
  bool lUserDocResolver;
  size_t lNumURIMappers, lNumURLResolvers;
  if (ar.is_serializing_out())
  {
    // serialize out: remember whether a doc and collection
    //                resolver was registered by the user
    lUserDocResolver = ((theDocResolver != NULL) && (dynamic_cast<StandardDocumentURIResolver*>(theDocResolver) == NULL));
    lNumURIMappers = theURIMappers.size();
    lNumURLResolvers = theURLResolvers.size();

    ar.set_is_temp_field(true);
    ar & lUserDocResolver;
    ar & lNumURIMappers;
    ar & lNumURLResolvers;
    ar.set_is_temp_field(false);
  }
  else
  {
    // serialize in: set the document and collection resolvers
    //               use one by the user or use the default
    //               if null is returned
    SerializationCallback* lCallback = ar.getUserCallback();

    ar.set_is_temp_field(true);
    ar & lUserDocResolver; // doc resolver passed by the user
    ar & lNumURIMappers;   // number of URIMappers passed by the user
    ar & lNumURLResolvers; // number of URLResolvers passed by the user
    ar.set_is_temp_field(false);

    // callback required but not available
    if ((lUserDocResolver || lNumURIMappers || lNumURLResolvers) && !lCallback)
    {
      throw ZORBA_EXCEPTION(
        zerr::ZCSE0013_UNABLE_TO_LOAD_QUERY,
        ERROR_PARAMS( ZED( NoSerializationCallbackForDocColMod ) )
      );
    }

    if (lUserDocResolver) {
      DocumentURIResolver* lDocResolver = lCallback->getDocumentURIResolver();
      if (!lDocResolver)
      {
        throw ZORBA_EXCEPTION(
          zerr::ZCSE0013_UNABLE_TO_LOAD_QUERY,
          ERROR_PARAMS( ZED( NoDocumentURIResolver ) )
        );
      }
      set_document_uri_resolver(new DocumentURIResolverWrapper(lDocResolver));
    }

    if (lNumURIMappers) {
      for (size_t i = 0; i < lNumURIMappers; ++i) {
        zorba::URIMapper* lURIMapper = lCallback->getURIMapper(i);
        if (!lURIMapper) {
          throw ZORBA_EXCEPTION(
            zerr::ZCSE0013_UNABLE_TO_LOAD_QUERY,
            ERROR_PARAMS( ZED( NoModuleURIResolver ) )
          );
        }
        // QQQ memory management?
        add_uri_mapper(new URIMapperWrapper(*lURIMapper));
      }
    }
    if (lNumURLResolvers) {
      for (size_t i = 0; i < lNumURLResolvers; ++i) {
        zorba::URLResolver* lURLResolver = lCallback->getURLResolver(i);
        if (!lURLResolver) {
          throw ZORBA_EXCEPTION(
            zerr::ZCSE0013_UNABLE_TO_LOAD_QUERY,
            ERROR_PARAMS( ZED( NoModuleURIResolver ) )
          );
        }
        // QQQ memory management?
        add_url_resolver(new URLResolverWrapper(*lURLResolver));
      }
    }
  }
}


/*******************************************************************************

********************************************************************************/
void static_context::serialize_tracestream(serialization::Archiver& ar)
{
  bool lUserTraceStream;
  if (ar.is_serializing_out())
  {
    // serialize out: remember whether the user registered a trace stream
    lUserTraceStream = (theTraceStream != 0);

    ar.set_is_temp_field(true);
    ar & lUserTraceStream;
    ar.set_is_temp_field(false);
  }
  else
  {
    // serialize in: set the trace stream from the user
    //               std::cerr is used if non was registered
    SerializationCallback* lCallback = ar.getUserCallback();

    ar.set_is_temp_field(true);
    ar & lUserTraceStream; // trace stream passed by the user
    ar.set_is_temp_field(false);

    // callback required but not available
    if (lUserTraceStream && !lCallback)
    {
      throw ZORBA_EXCEPTION(
        zerr::ZCSE0013_UNABLE_TO_LOAD_QUERY,
        ERROR_PARAMS( ZED( NoSerializationCallbackForTraceStream ) )
      );
    }

    if (lUserTraceStream) {
      bool lTraceStream =  lCallback->getTraceStream(theTraceStream);
      if (!lTraceStream) {
        throw ZORBA_EXCEPTION(
          zerr::ZCSE0013_UNABLE_TO_LOAD_QUERY,
          ERROR_PARAMS( ZED( BadTraceStream ) )
        );
      }
    }
  }
}


/*******************************************************************************

********************************************************************************/
void static_context::serialize(::zorba::serialization::Archiver& ar)
{
  if (ar.is_serializing_out())
  {
    ar.set_is_temp_field(true);
    bool  parent_is_root = check_parent_is_root();
    ar & parent_is_root;
    ar.set_is_temp_field(false);

    if(!parent_is_root)
    {
      ar.dont_allow_delay();
      ar & theParent;
    }
    else
    {
      //  context *fooctx = NULL;
      //  ar & fooctx;
    }
  }
  else
  {
    //in serialization
    ar.set_is_temp_field(true);
    bool  parent_is_root;
    ar & parent_is_root;
    ar.set_is_temp_field(false);

    if(parent_is_root)
    {
      set_parent_as_root();
    }
    else
      ar & theParent;

    if(theParent)
      theParent->addReference(theParent->getSharedRefCounter() SYNC_PARAM2(theParent->getRCLock()));
  }

  ar & theModuleNamespace;
  ar & theImportedBuiltinModules;

  ar & theBaseUriInfo;

  serialize_resolvers(ar);
  serialize_tracestream(ar);

  ar & theModulePaths;

  ar & theExternalModulesMap;

  SERIALIZE_TYPEMANAGER_RCHANDLE(TypeManager, theTypemgr);

  ar & theNamespaceBindings;
  ar & theDefaultElementNamespace;
  ar & theHaveDefaultElementNamespace;
  ar & theDefaultFunctionNamespace;
  ar & theHaveDefaultFunctionNamespace;

  ar & theCtxItemType;

  ar & theVariablesMap;

  ar.set_serialize_only_for_eval(true);
  ar & theFunctionMap;
  ar & theFunctionArityMap;
  ar.set_serialize_only_for_eval(false);

  ar & theAnnotationMap;

  ar & theCollectionMap;

  ar & theW3CCollectionMap;
  ar & theDefaultW3CCollectionType;

  ar & theIndexMap;

  ar & theICMap;

  ar & theDocumentMap;

  ar & theCollationMap;
  ar & theDefaultCollation;

  ar & theOptionMap;
#ifndef ZORBA_NO_FULL_TEXT
  ar & theFTMatchOptions;
#endif /* ZORBA_NO_FULL_TEXT */

  SERIALIZE_ENUM(StaticContextConsts::xquery_version_t, theXQueryVersion);
  SERIALIZE_ENUM(StaticContextConsts::xpath_compatibility_t, theXPathCompatibility);
  SERIALIZE_ENUM(StaticContextConsts::construction_mode_t, theConstructionMode);
  SERIALIZE_ENUM(StaticContextConsts::inherit_mode_t, theInheritMode);
  SERIALIZE_ENUM(StaticContextConsts::preserve_mode_t, thePreserveMode);
  SERIALIZE_ENUM(StaticContextConsts::ordering_mode_t, theOrderingMode);
  SERIALIZE_ENUM(StaticContextConsts::empty_order_mode_t, theEmptyOrderMode);
  SERIALIZE_ENUM(StaticContextConsts::boundary_space_mode_t, theBoundarySpaceMode);
  SERIALIZE_ENUM(StaticContextConsts::validation_mode_t, theValidationMode);

  ar & theDecimalFormats;

  ar & theDisabledWarnings;

  ar & theAllWarningsDisabled;

  ar & theWarningsAreErrors;
}


/***************************************************************************//**
  Create a new static_context obj and make a child of "this" static_context obj.
********************************************************************************/
static_context* static_context::create_child_context()
{
  return new static_context(this);
}


/***************************************************************************//**

********************************************************************************/
bool static_context::is_global_root_sctx() const
{
  return (this == &GENV_ROOT_STATIC_CONTEXT);
}


/***************************************************************************//**

********************************************************************************/
bool static_context::check_parent_is_root()
{
  return theParent == &GENV_ROOT_STATIC_CONTEXT;
}


/***************************************************************************//**

********************************************************************************/
void static_context::set_parent_as_root()
{
  theParent = &GENV_ROOT_STATIC_CONTEXT;
}


/***************************************************************************//**

********************************************************************************/
expr_t static_context::get_query_expr() const
{
  return theQueryExpr;
}


/***************************************************************************//**

********************************************************************************/
void static_context::set_query_expr(expr_t expr)
{
  theQueryExpr = expr;
}


/***************************************************************************//**

********************************************************************************/
void static_context::set_trace_stream(std::ostream& os)
{
  theTraceStream = &os;
}


/***************************************************************************//**

********************************************************************************/
std::ostream* static_context::get_trace_stream() const
{
  if (theTraceStream)
    return theTraceStream;

  return (theParent == NULL ?
          &std::cerr :
          dynamic_cast<static_context*>(theParent)->get_trace_stream());
}


/***************************************************************************//**

********************************************************************************/
void static_context::add_imported_builtin_module(const zstring& ns)
{
  if (theImportedBuiltinModules == NULL)
  {
    theImportedBuiltinModules = new std::vector<zstring>;
  }

  theImportedBuiltinModules->push_back(ns);
}


/***************************************************************************//**

********************************************************************************/
bool static_context::is_imported_builtin_module(const zstring& ns)
{
  static_context* sctx = this;

  while (sctx != NULL)
  {
    if (sctx->theImportedBuiltinModules)
    {
      std::vector<zstring>::const_iterator ite = sctx->theImportedBuiltinModules->begin();
      std::vector<zstring>::const_iterator end = sctx->theImportedBuiltinModules->end();
      for (; ite != end; ++ite)
      {
        if (*ite == ns)
          return true;
      }
    }

    sctx = sctx->theParent;
  }

  return false;
}


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  Base URI                                                                   //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


/***************************************************************************//**

********************************************************************************/
zstring static_context::get_implementation_baseuri() const
{
  return reinterpret_cast<root_static_context*>(&GENV_ROOT_STATIC_CONTEXT)->
         theImplementationBaseUri;
}


/***************************************************************************//**

********************************************************************************/
bool static_context::get_encapsulating_entity_uri(zstring& res) const
{
  const static_context* sctx = this;
  while (sctx != NULL)
  {
    if (sctx->theBaseUriInfo != NULL &&
        sctx->theBaseUriInfo->theHaveEncapsulatingEntityUri)
    {
      res = sctx->theBaseUriInfo->theEncapsulatingEntityUri;
      return true;
    }

    sctx = sctx->theParent;
  }

  return false;
}


/***************************************************************************//**

********************************************************************************/
void static_context::set_encapsulating_entity_uri(const zstring& uri)
{
  if (theBaseUriInfo == NULL)
  {
    theBaseUriInfo = new BaseUriInfo;
  }

  theBaseUriInfo->theEncapsulatingEntityUri = uri;
  theBaseUriInfo->theHaveEncapsulatingEntityUri = true;

  compute_base_uri();
}


/***************************************************************************//**

********************************************************************************/
bool static_context::get_entity_retrieval_uri(zstring& res) const
{
  const static_context* sctx = this;
  while (sctx != NULL)
  {
    if (sctx->theBaseUriInfo != NULL &&
        sctx->theBaseUriInfo->theHaveEntityRetrievalUri)
    {
      res = sctx->theBaseUriInfo->theEntityRetrievalUri;
      return true;
    }

    sctx = sctx->theParent;
  }

  return false;
}


/***************************************************************************//**

********************************************************************************/
void static_context::set_entity_retrieval_uri(const zstring& uri)
{
  if (theBaseUriInfo == NULL)
  {
    theBaseUriInfo = new BaseUriInfo;
  }

  theBaseUriInfo->theEntityRetrievalUri = uri;
  theBaseUriInfo->theHaveEntityRetrievalUri = true;

  compute_base_uri();
}


/***************************************************************************//**

********************************************************************************/
zstring static_context::get_base_uri() const
{
  const static_context* sctx = this;
  while (sctx != NULL)
  {
    if (sctx->theBaseUriInfo != NULL &&
        sctx->theBaseUriInfo->theHaveBaseUri)
    {
      return sctx->theBaseUriInfo->theBaseUri;
    }

    sctx = sctx->theParent;
  }

  ZORBA_ASSERT(false);
}


/***************************************************************************//**

********************************************************************************/
void static_context::set_base_uri(const zstring& uri, bool from_prolog)
{
  if (theBaseUriInfo == NULL)
  {
    theBaseUriInfo = new BaseUriInfo;
  }

  if (from_prolog)
  {
    if (theBaseUriInfo->theHavePrologBaseUri)
      throw XQUERY_EXCEPTION(err::XQST0032);

    theBaseUriInfo->thePrologBaseUri = uri;
    theBaseUriInfo->theHavePrologBaseUri = true;
  }
  else
  {
    // overwite existing value of application baseuri, if any
    theBaseUriInfo->theApplicationBaseUri = uri;
    theBaseUriInfo->theHaveApplicationBaseUri = true;
  }

  compute_base_uri();
}


/***************************************************************************//**
  Base Uri Computation

  The from_prolog_baseuri is the one declared in the prolog. The baseuri is set
  explicitly from the C++/C api. If both the from_prolog_baseuri and the baseuri
  are set, the from_prolog_baseuri hides the baseuri.

  For the main module, the entity_retrieval_url is set by default to the name
  of file containing the query we are running. For library modules, it is set
  to the location uri of each module component. It may also be set explicitly
  from the C++/C api.
********************************************************************************/
void static_context::compute_base_uri()
{
  if (theBaseUriInfo == NULL)
  {
    theBaseUriInfo = new BaseUriInfo;
  }

  bool found;

  bool foundUserBaseUri = false;
  zstring userBaseUri;
  zstring encapsulatingUri;
  zstring entityUri;

  const static_context* sctx = this;

  while (sctx != NULL)
  {
    if (sctx->theBaseUriInfo != NULL &&
        sctx->theBaseUriInfo->theHavePrologBaseUri)
    {
      userBaseUri = sctx->theBaseUriInfo->thePrologBaseUri;
      foundUserBaseUri = true;
      break;
    }

    sctx = sctx->theParent;
  }

  if (!foundUserBaseUri)
  {
    sctx = this;

    while (sctx != NULL)
    {
      if (sctx->theBaseUriInfo != NULL &&
          sctx->theBaseUriInfo->theHaveApplicationBaseUri)
      {
        userBaseUri = sctx->theBaseUriInfo->theApplicationBaseUri;
        foundUserBaseUri = true;
        break;
      }

      sctx = sctx->theParent;
    }
  }

  if (foundUserBaseUri)
  {
    try
    {
      URI lCheckValid(userBaseUri);
      if (lCheckValid.is_absolute())
      {
        theBaseUriInfo->theBaseUri = lCheckValid.toString();
        theBaseUriInfo->theHaveBaseUri = true;
        return; // valid (absolute) uri
      }
    }
    catch (ZorbaException const&)
    {
      // assume it's relative and go on
    }

    /// is relative, needs to be resolved
    found = get_encapsulating_entity_uri(encapsulatingUri);
    if (found)
    {
      URI base(encapsulatingUri);
      URI resolvedURI(base, userBaseUri);
      theBaseUriInfo->theBaseUri = resolvedURI.toString();
      theBaseUriInfo->theHaveBaseUri = true;
      return;
    }

    found = get_entity_retrieval_uri(entityUri);
    if (found)
    {
      URI base(entityUri);
      URI resolvedURI(base, userBaseUri);
      theBaseUriInfo->theBaseUri = resolvedURI.toString();
      theBaseUriInfo->theHaveBaseUri = true;
      return;
    }

    URI base(get_implementation_baseuri());
    URI resolvedURI(base, userBaseUri);
    theBaseUriInfo->theBaseUri = resolvedURI.toString();
    theBaseUriInfo->theHaveBaseUri = true;
    return;
  }

  found = get_encapsulating_entity_uri(encapsulatingUri);
  if (found)
  {
    theBaseUriInfo->theBaseUri = encapsulatingUri;
    theBaseUriInfo->theHaveBaseUri = true;
    return;
  }

  found = get_entity_retrieval_uri(entityUri);
  if (found)
  {
    theBaseUriInfo->theBaseUri = entityUri;
    theBaseUriInfo->theHaveBaseUri = true;
    return;
  }

  theBaseUriInfo->theBaseUri = get_implementation_baseuri();
  theBaseUriInfo->theHaveBaseUri = true;
  return;
}


/***************************************************************************/

zstring
static_context::resolve_relative_uri(
    const zstring& aUri,
    bool aValidate) const
{
  URI lBaseUri(get_base_uri());
  URI lResolvedUri(lBaseUri, aUri, aValidate);
  return lResolvedUri.toString();
}

zstring
static_context::resolve_relative_uri(
    const zstring& aRelativeUri,
    const zstring& aBaseUri,
    bool aValidate) const
{
  URI lBaseUri(aBaseUri);
  URI lResolvedUri(lBaseUri, aRelativeUri, aValidate);
  return lResolvedUri.toString();
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  URI Resolution                                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

void static_context::add_uri_mapper(impl::URIMapper* aMapper) throw ()
{
  theURIMappers.push_back(std::auto_ptr<impl::URIMapper>(aMapper));
}

void static_context::add_url_resolver(impl::URLResolver* aResolver) throw ()
{
  theURLResolvers.push_back(std::auto_ptr<impl::URLResolver>(aResolver));
}

std::auto_ptr<impl::Resource>
static_context::resolve_uri
(zstring const& aUri, impl::Resource::EntityType aEntityType, zstring& oErrorMessage) const
{
  std::vector<zstring> lUris;
  apply_uri_mappers(aUri, aEntityType, impl::URIMapper::CANDIDATE, lUris);

  std::auto_ptr<impl::Resource> lRetval;
  apply_url_resolvers(lUris, aEntityType, lRetval, oErrorMessage);

  return std::auto_ptr<impl::Resource>(lRetval.release());
}

void
static_context::get_component_uris
(zstring const& aUri, impl::Resource::EntityType aEntityType,
  std::vector<zstring>& oComponents) const
{
  apply_uri_mappers(aUri, aEntityType, impl::URIMapper::COMPONENT, oComponents);
  if (oComponents.size() == 0) {
    oComponents.push_back(aUri);
  }
}

void
static_context::apply_uri_mappers
(zstring const& aUri, impl::Resource::EntityType aEntityType,
  impl::URIMapper::Kind aMapperKind, std::vector<zstring>& oUris) const throw ()
{
  // Initialize list with the one input URI.
  oUris.push_back(aUri);

  // Iterate upwards through the static_context tree...
  for (static_context const* sctx = this;
       sctx != NULL; sctx = sctx->theParent)
  {
    // Iterate through all available mappers on this static_context...
    for (ztd::auto_vector<impl::URIMapper>::const_iterator mapper =
           sctx->theURIMappers.begin();
         mapper != sctx->theURIMappers.end(); mapper++)
    {
      // Only call mappers of the appropriate kind
      if ((*mapper)->mapperKind() != aMapperKind) {
        continue;
      }

      // Create new list (currently empty) for this mapper
      std::vector<zstring> lResultUris;

      // Iterate through all URIs on the current list...
      for (std::vector<zstring>::iterator uri = oUris.begin();
           uri != oUris.end(); uri++)
      {
        // And call the current mapper with the current URI.
        size_t lNumResultUris = lResultUris.size();
        (*mapper)->mapURI(*uri, aEntityType, *this, lResultUris);
        if (lNumResultUris == lResultUris.size()) {
          // Mapper didn't map this URI to anything new, therefore add
          // the original URI to the result list
          lResultUris.push_back(*uri);
        }
      }

      // Now repeat process with the next mapper using the new list of
      // URIs.
      oUris = lResultUris;
    }
  }
}

void
static_context::apply_url_resolvers
(std::vector<zstring>& aUrls, impl::Resource::EntityType aEntityType,
  std::auto_ptr<impl::Resource>& oResource, zstring& oErrorMessage) const
{
  oErrorMessage = "";

  // Iterate through all candidate URLs...
  for (std::vector<zstring>::iterator url = aUrls.begin();
       url != aUrls.end(); url++) {

    // Iterate upwards through the static_context tree...
    for (static_context const* sctx = this;
         sctx != NULL; sctx = sctx->theParent)
    {
      // Iterate through all available resolvers on this static_context...
      for (ztd::auto_vector<impl::URLResolver>::const_iterator resolver =
             sctx->theURLResolvers.begin();
           resolver != sctx->theURLResolvers.end(); resolver++)
      {
        try {
          // Take ownership of returned Resource (if any)
          oResource.reset((*resolver)->resolveURL(*url, aEntityType));
          if (oResource.get() != NULL) {
            // Populate the URL used to load this Resource
            oResource->setUrl(*url);
            return;
          }
        }
        catch (const std::exception& e) {
          if (oErrorMessage == "") {
            // Really no point in saving anything more than the first message
            oErrorMessage = e.what();
          }
        }
        catch (...) {
          // Not much we can do here except try the rest of the
          // candidate URIs
        }
      }
    }
  }
}

/***************************************************************************//**
QQQ delete all these empty comment headers
********************************************************************************/
void static_context::set_document_uri_resolver(InternalDocumentURIResolver* aDocResolver)
{
  delete theDocResolver;

  theDocResolver = aDocResolver;
}


/*******************************************************************************

********************************************************************************/
InternalDocumentURIResolver* static_context::get_document_uri_resolver() const
{
  if ( theDocResolver != 0 )
    return theDocResolver;

  return (theParent != NULL ?
          dynamic_cast<static_context*>(theParent)->get_document_uri_resolver() :
          0);
}


#ifndef ZORBA_NO_FULL_TEXT
/******************************************************************************

 ******************************************************************************/

void static_context::add_stop_words_uri_resolver(
    InternalFullTextURIResolver* aFullTextResolver)
{
  theStopWordsResolvers.push_back(aFullTextResolver);
}

void static_context::add_thesaurus_uri_resolver(
    InternalFullTextURIResolver* aFullTextResolver)
{
  theThesaurusResolvers.push_back(aFullTextResolver);
}

core::Stemmer const* static_context::get_stemmer( iso639_1::type lang ) const {
  FOR_EACH( stemmer_providers_t, provider, theStemmerProviders )
    if ( core::Stemmer const *const stemmer = (*provider)->get_stemmer( lang ) )
      return stemmer;
  return theParent ?
    theParent->get_stemmer( lang ) :
    core::StemmerProvider::get_default_provider().get_stemmer( lang );
}

void static_context::get_stop_words_uri_resolvers(
    std::vector<InternalFullTextURIResolver*>& aResolvers) const
{
  if (theParent)
    theParent->get_stop_words_uri_resolvers(aResolvers);

  aResolvers.insert(aResolvers.end(),
                    theStopWordsResolvers.begin(),
                    theStopWordsResolvers.end());
}

void static_context::get_thesaurus_uri_resolvers(
    std::vector<InternalFullTextURIResolver*>& aResolvers) const
{
  if (theParent)
    theParent->get_thesaurus_uri_resolvers(aResolvers);

  aResolvers.insert(aResolvers.end(),
                    theThesaurusResolvers.begin(),
                    theThesaurusResolvers.end());
}


void static_context::remove_stemmer_provider( core::StemmerProvider const *p ) {
  ztd::erase_1st_if(
    theStemmerProviders,
    std::bind2nd( std::equal_to<core::StemmerProvider const*>(), p )
  );
}

void static_context::remove_stop_words_uri_resolver(
    InternalFullTextURIResolver* aResolver)
{
  ztd::erase_1st_if(
    theStopWordsResolvers,
    std::bind2nd( std::equal_to<InternalFullTextURIResolver*>(), aResolver )
  );
}

void static_context::remove_thesaurus_uri_resolver(
    InternalFullTextURIResolver* aResolver)
{
  std::vector<InternalFullTextURIResolver*>::iterator ite;
  for (ite = theThesaurusResolvers.begin(); ite != theThesaurusResolvers.end(); ++ite)
  {
    if (aResolver == *ite)
    {
      theThesaurusResolvers.erase(ite);
      return; // no duplicates in the vector
    }
  }
}
#endif /* ZORBA_NO_FULL_TEXT */


/*******************************************************************************

********************************************************************************/
void static_context::set_module_paths(const std::vector<zstring>& paths)
{
  theModulePaths = paths;
}


/*******************************************************************************

********************************************************************************/
void static_context::get_module_paths(std::vector<zstring>& paths) const
{
  paths.insert(paths.end(), theModulePaths.begin(), theModulePaths.end());
}


/*******************************************************************************

********************************************************************************/
void static_context::get_full_module_paths(std::vector<zstring>& paths) const
{
  if (theParent != NULL)
  {
    theParent->get_full_module_paths(paths);
  }

  get_module_paths(paths);
}

/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  Module import chekcers                                                     //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////

void static_context::addModuleImportChecker(ModuleImportChecker* aChecker)
{
  theModuleImportCheckers.push_back(aChecker);
}

void static_context::removeModuleImportChecker(ModuleImportChecker* aChecker)
{
  std::vector<ModuleImportChecker*> lCheckers = getAllModuleImportCheckers();
  std::vector<ModuleImportChecker*>::iterator lIter =
      lCheckers.begin();
  for (; lIter != lCheckers.end(); ++lIter)
  {
    if (**lIter == *aChecker) {
      theModuleImportCheckers.erase(lIter);
    }
  }
}

std::vector<ModuleImportChecker*> static_context::getAllModuleImportCheckers() const
{
  std::vector<ModuleImportChecker*> lResult;
  lResult.insert(lResult.end(), theModuleImportCheckers.begin(),
                 theModuleImportCheckers.end());
  static_context* lParent = get_parent();
  if (lParent) {
    std::vector<ModuleImportChecker*> lC = lParent->getAllModuleImportCheckers();
    lResult.insert(lResult.end(), lC.begin(), lC.end());
  }
  return lResult;
}


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  Validating Items                                                           //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////

bool static_context::validate(
    store::Item* rootElement,
    store::Item* validatedResult,
    StaticContextConsts::validation_mode_t validationMode)
{
  zstring xsTns(XML_SCHEMA_NS);
  return validate(rootElement, validatedResult, xsTns, validationMode);
}


bool static_context::validate(
    store::Item* rootElement,
    store::Item* validatedResult,
    const zstring& targetNamespace,
    StaticContextConsts::validation_mode_t validationMode)
{
  if ( !rootElement->isNode() ||
       (rootElement->getNodeKind() != store::StoreConsts::documentNode &&
       rootElement->getNodeKind() != store::StoreConsts::elementNode))
    return false;

  if ( rootElement->isValidated() )
    return false;

#ifndef ZORBA_NO_XMLSCHEMA

  TypeManager* tm = this->get_typemanager();
  zstring docUri;
  rootElement->getDocumentURI(docUri);

  if (validationMode != StaticContextConsts::skip_validation)
  {
    store::Item_t validatedNode;
    store::Item_t typeName;
    QueryLoc loc;

    ParseConstants::validation_mode_t mode =
        (validationMode == StaticContextConsts::strict_validation ?
            ParseConstants::val_strict :
            ParseConstants::val_lax );

    store::Item_t tmp = validatedResult;
    return Validator::effectiveValidationValue(tmp,
                                               rootElement,
                                               typeName,
                                               tm,
                                               mode,
                                               this,
                                               loc);

  }
#endif //ZORBA_NO_XMLSCHEMA

  return false;
}

bool static_context::validateSimpleContent(
    zstring& stringValue,
    store::Item* typeQName,
    std::vector<store::Item_t>& resultList)
{
  store::NsBindings bindings;
  this->get_namespace_bindings(bindings);
  store::Item_t lTypeQName(typeQName);

  Validator::processTextValue(
      this,
      this->get_typemanager(),
      bindings,
      lTypeQName,
      stringValue,
      resultList,
      QueryLoc::null);

  return true;
}

/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  Type Manager                                                               //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************

********************************************************************************/
void static_context::set_typemanager(rchandle<TypeManager> typemgr)
{
  theTypemgr = typemgr;
}


TypeManager* static_context::get_typemanager() const
{
  TypeManager* tm = theTypemgr.getp();
  if (tm != NULL)
  {
    return tm;
  }
  return static_cast<static_context *>(theParent)->get_typemanager();
}


TypeManager* static_context::get_local_typemanager() const
{
  return theTypemgr.getp();
}


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  Namespace Bindings                                                         //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


/***************************************************************************//**
  Get the default namespace for element and type qnames.
********************************************************************************/
const zstring& static_context::default_elem_type_ns() const
{
  if (theHaveDefaultElementNamespace || theParent == NULL)
  {
    return theDefaultElementNamespace;
  }
  else
  {
    return theParent->default_elem_type_ns();
  }
}


/***************************************************************************//**
  Set the default namespace for element and type qnames to the given namespace.
********************************************************************************/
void static_context::set_default_elem_type_ns(
    const zstring& ns,
    const QueryLoc& loc)
{
  if (!theHaveDefaultElementNamespace)
  {
    theDefaultElementNamespace = ns;
    theHaveDefaultElementNamespace = true;
  }
  else
  {
    throw XQUERY_EXCEPTION(err::XQST0066, ERROR_LOC(loc));
  }
}


/***************************************************************************//**
  Get the default namespace for function qnames.
********************************************************************************/
const zstring& static_context::default_function_ns() const
{
  if (theHaveDefaultFunctionNamespace || theParent == NULL)
  {
    return theDefaultFunctionNamespace;
  }
  else
  {
    return theParent->default_function_ns();
  }
}


/***************************************************************************//**
 Set the default namespace for function qnames to the given namespace.
********************************************************************************/
void static_context::set_default_function_ns(
   const zstring& ns,
   const QueryLoc& loc)
{
  if (!theHaveDefaultFunctionNamespace)
  {
    theDefaultFunctionNamespace = ns;
    theHaveDefaultFunctionNamespace = true;
  }
  else
  {
    throw XQUERY_EXCEPTION(err::XQST0066, ERROR_LOC(loc));
  }
}


/***************************************************************************//**
  Bind the given prefix to the given namaspace uri. The binding is stored in
  "this". If there is already in "this" a binding for the prefix, raise error.
********************************************************************************/
void static_context::bind_ns(
    const zstring& prefix,
    const zstring& ns,
    const QueryLoc& loc,
    const Error& err)
{
  if (theNamespaceBindings == NULL)
  {
    theNamespaceBindings = new NamespaceBindings(16, false);
  }

  zstring temp(ns);

  if (!theNamespaceBindings->insert(prefix, temp))
  {
    throw XQUERY_EXCEPTION_VAR(
      err,
      ERROR_PARAMS(
				prefix, temp
  		),
      ERROR_LOC(loc));
  }
}


/***************************************************************************//**
  Search the static-context tree, starting from "this" and moving upwards,
  looking for the 1st namespace binding for the given prefix. If no such
  binding is found, either raise an error (if the given error code is not
  ZXQP0000_NO_ERROR) or return false. Otherwise, return true and the
  associated namespace uri.
********************************************************************************/
bool static_context::lookup_ns(
    zstring& ns,
    const zstring& prefix,
    const QueryLoc& loc,
    const Error& err) const
{
  if (theNamespaceBindings == NULL || !theNamespaceBindings->get(prefix, ns))
  {
    if (theParent != NULL)
    {
      return theParent->lookup_ns(ns, prefix, loc, err);
    }
    else if (err != zerr::ZXQP0000_NO_ERROR)
    {
      throw XQUERY_EXCEPTION_VAR(
        err, ERROR_PARAMS( prefix ), ERROR_LOC( loc )
      );
    }
    else
    {
      return false;
    }
  }
  else if (!prefix.empty() && ns.empty())
  {
    if (err != zerr::ZXQP0000_NO_ERROR)
    {
      throw XQUERY_EXCEPTION_VAR(
        err, ERROR_PARAMS( prefix ), ERROR_LOC( loc )
      );
    }
    else
    {
      return false;
    }
  }

  return true;
}


/***************************************************************************//**
  Convert a [prefix, localName] pair to an expanded QName item, using the given
  default namespace if the prefix is empty. Raise error if the prefix is non-
  empty and there is no associated namespace uri.
********************************************************************************/
void static_context::expand_qname(
    store::Item_t& qname,
    const zstring& default_ns,
    const zstring& prefix,
    const zstring& local,
    const QueryLoc& loc) const
{
  if (prefix.empty())
  {
    ITEM_FACTORY->createQName(qname, default_ns, prefix, local);
  }
  else
  {
    zstring ns;
    lookup_ns(ns, prefix, loc);
    ITEM_FACTORY->createQName(qname, ns, prefix, local);
  }
}


/***************************************************************************//**

********************************************************************************/
void static_context::get_namespace_bindings(store::NsBindings& bindings) const
{
  const static_context* sctx = this;

  while (sctx != NULL)
  {
    if (sctx->theNamespaceBindings != NULL)
    {
      NamespaceBindings::iterator ite = sctx->theNamespaceBindings->begin();
      NamespaceBindings::iterator end = sctx->theNamespaceBindings->end();

      for (; ite != end; ++ite)
      {
        std::pair<zstring, zstring> binding = (*ite);

        // Ignore duplicates
        const zstring& prefix = binding.first;
        ulong numBindings = (ulong)bindings.size();
        bool found = 0;
        for (unsigned int i = 0; i < numBindings; ++i)
        {
          if (bindings[i].first == prefix)
          {
            found = 1;
            break;
          }
        }

        if (!found)
          bindings.push_back(binding);
      }
    }

    sctx = sctx->theParent;
  }
}


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  Variables                                                                  //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


/***************************************************************************//**

********************************************************************************/
void static_context::bind_var(
    var_expr_t& varExpr,
    const QueryLoc& loc,
    const Error& err)
{
  if (theVariablesMap == NULL)
  {
    theVariablesMap = new VariableMap(0, NULL, 8, false);
  }

  store::Item* qname = varExpr->get_name();

  if (!theVariablesMap->insert(qname, varExpr))
  {
    throw XQUERY_EXCEPTION_VAR(
      err, ERROR_PARAMS( qname->getStringValue() ), ERROR_LOC( loc )
    );
  }
}


/***************************************************************************//**
  Lookup variable by expanded qname. Search starts from the "current" sctx and
  moves upwards the ancestor path until the first instance (if any) of the var
  is found.

  If var is not found, the method raises the given error, unless the given error
  is ZXQP0000_NO_ERROR, in which case it returns NULL.
********************************************************************************/
var_expr* static_context::lookup_var(
    const store::Item* qname,
    const QueryLoc& loc,
    const Error& error) const
{
  store::Item* qname2 = const_cast<store::Item*>(qname);

  const static_context* sctx = this;
  var_expr_t varExpr;

  while (sctx != NULL)
  {
    if (sctx->theVariablesMap != NULL &&
        sctx->theVariablesMap->get(qname2, varExpr))
    {
      return varExpr.getp();
    }

    sctx = sctx->theParent;
  }

  if (error != zerr::ZXQP0000_NO_ERROR)
  {
    zstring lVarName = var_name(qname);
    throw XQUERY_EXCEPTION_VAR(
      error, ERROR_PARAMS( lVarName ), ERROR_LOC( loc )
    );
  }

  return NULL;
}


/***************************************************************************//**
  This method is used by introspection and debugger
********************************************************************************/
void static_context::getVariables(
  std::vector<var_expr_t>& vars,
  bool aLocalsOnly) const
{
  const static_context* sctx = this;

  while (sctx != NULL)
  {
    if (sctx->theVariablesMap != NULL)
    {
      VariableMap::iterator ite = sctx->theVariablesMap->begin();
      VariableMap::iterator end = sctx->theVariablesMap->end();

      for (; ite != end; ++ite)
      {
        ulong numVars = (ulong)vars.size();
        ulong i = 0;
        for (; i < numVars; ++i)
        {
          if (vars[i]->get_name()->equals((*ite).first))
            break;
        }

        if (i == numVars)
          vars.push_back((*ite).second);
      }
    }

    if (aLocalsOnly)
    {
      break;
    }

    sctx = sctx->theParent;
  }
}


/***************************************************************************//**

********************************************************************************/
void static_context::set_context_item_type(xqtref_t& t)
{
  theCtxItemType = t;
}

/***************************************************************************//**

********************************************************************************/
zstring
static_context::var_name(const store::Item* aVarName)
{
  zstring lVarName = aVarName->getStringValue();
  if (lVarName == static_context::DOT_POS_VAR_NAME)
  {
    lVarName = "context position";
  } else if (lVarName == static_context::DOT_SIZE_VAR_NAME)
  {
    lVarName = "context size";
  } else if (lVarName == static_context::DOT_VAR_NAME)
  {
    lVarName = "context item";
  }
  return lVarName;
}


/***************************************************************************//**

********************************************************************************/
const XQType* static_context::get_context_item_type() const
{
  const static_context* sctx = this;
  while (sctx != NULL)
  {
    if (theCtxItemType != NULL)
      return theCtxItemType.getp();

    sctx = sctx->theParent;
  }

  return NULL;
}


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  Functions                                                                  //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


/***************************************************************************//**

********************************************************************************/
void static_context::bind_fn(
    function_t& f,
    ulong arity,
    const QueryLoc& loc)
{
  store::Item* qname = f->getName();

  if (!is_global_root_sctx() && lookup_local_fn(qname, arity) != NULL)
  {
    throw XQUERY_EXCEPTION(
      err::XQST0034, ERROR_PARAMS( qname->getStringValue() ), ERROR_LOC( loc )
    );
  }

  if (theFunctionMap == NULL)
  {
    ulong size = (is_global_root_sctx() ? 500 : 32);
    theFunctionMap = new FunctionMap(0, NULL, size, false);
  }

  FunctionInfo fi(f);

  if (!theFunctionMap->insert(qname, fi))
  {
    // There is already a function F with the given qname in theFunctionMap.
    // First, check if F is the same as f, which implies that f is disabled.
    // In this case, re-enable f. Otherwise, we have to use theFunctionArityMap.
    if (fi.theFunction == f)
    {
      ZORBA_ASSERT(fi.theIsDisabled);
      fi.theIsDisabled = false;
      return;
    }

    fi.theFunction = f;
    fi.theIsDisabled = false;

    ZORBA_ASSERT(!f->isVariadic());

    if (theFunctionArityMap == NULL)
    {
      theFunctionArityMap = new FunctionArityMap(0, NULL, 16, false);
    }

    std::vector<FunctionInfo>* fv = 0;

    if (theFunctionArityMap->get(qname, fv))
    {
      ulong numFunctions = (ulong)fv->size();
      for (ulong i = 0; i < numFunctions; ++i)
      {
        if ((*fv)[i].theFunction == f)
        {
          ZORBA_ASSERT((*fv)[i].theIsDisabled);
          (*fv)[i].theIsDisabled = false;
          return;
        }
      }

      fv->push_back(fi);
    }
    else
    {
      fv = new std::vector<FunctionInfo>(1);
      (*fv)[0] = fi;
      theFunctionArityMap->insert(qname, fv);
    }
  }
}


/***************************************************************************//**
  Remove the function with the given qname and arity from the in-scope functions
  of this sctx.
********************************************************************************/
void static_context::unbind_fn(
    const store::Item* qname,
    ulong arity)
{
  ZORBA_ASSERT(!is_global_root_sctx());

  function* f = lookup_fn(qname, arity);

  if (f == NULL)
    return;

  if (theFunctionMap == NULL)
  {
    theFunctionMap = new FunctionMap(0, NULL, 32, false);
  }

  FunctionInfo fi(f, true);
  store::Item* qname2 = const_cast<store::Item*>(f->getName());

  if (theFunctionMap->get(qname2, fi))
  {
    if (fi.theFunction.getp() == f)
    {
      fi.theIsDisabled = true;
      theFunctionMap->update(qname2, fi);
      return;
    }

    if (theFunctionArityMap == NULL)
    {
      theFunctionArityMap = new FunctionArityMap(0, NULL, 16, false);
    }

    std::vector<FunctionInfo>* fv = NULL;

    if (theFunctionArityMap->get(qname2, fv))
    {
      ulong numFunctions = (ulong)fv->size();
      for (ulong i = 0; i < numFunctions; ++i)
      {
        if ((*fv)[i].theFunction.getp() == f)
        {
          (*fv)[i].theIsDisabled = true;
          return;
        }
      }
    }

    fv = new std::vector<FunctionInfo>(1);
    fi.theIsDisabled = true;
    (*fv)[0] = fi;
    theFunctionArityMap->insert(qname2, fv);
  }
  else
  {
    theFunctionMap->insert(qname2, fi);
  }
}


/***************************************************************************//**
  Search the static-context tree, starting from "this" and moving upwards,
  looking for the 1st sctx obj that contains a binding for a function with
  the given qname and arity. If no such binding is found return NULL. Otherwise,
  return the associated function object (which may be NULL if the function
  was disabled).
********************************************************************************/
function* static_context::lookup_fn(
    const store::Item* qname,
    ulong arity)
{
  FunctionInfo fi;
  store::Item* qname2 = const_cast<store::Item*>(qname);

  static_context* sctx = this;

  while (sctx != NULL)
  {
    if (sctx->theFunctionMap != NULL && sctx->theFunctionMap->get(qname2, fi))
    {
      function* f = fi.theFunction.getp();

      if (f->getArity() == arity || f->isVariadic())
      {
        return (fi.theIsDisabled ? NULL : f);
      }

      std::vector<FunctionInfo>* fv = NULL;

      if (sctx->theFunctionArityMap != NULL &&
          sctx->theFunctionArityMap->get(qname2, fv))
      {
        ulong numFunctions = (ulong)fv->size();
        for (ulong i = 0; i < numFunctions; ++i)
        {
          if ((*fv)[i].theFunction->getArity() == arity)
            return ((*fv)[i].theIsDisabled ? NULL : (*fv)[i].theFunction.getp());
        }
      }
    }

    sctx = sctx->theParent;
  }

  return NULL;
}


/***************************************************************************//**
  Search "this" static context a function with the given qname and arity. If no
  such function binding is found return NULL. Otherwise, return the associated
  function object (which may be NULL if the function was disabled).
********************************************************************************/
function* static_context::lookup_local_fn(
    const store::Item* qname,
    ulong arity)
{
  FunctionInfo fi;
  store::Item* qname2 = const_cast<store::Item*>(qname);

  if (theFunctionMap != NULL && theFunctionMap->get(qname2, fi))
  {
    function* f = fi.theFunction.getp();

    if (f->getArity() == arity || f->isVariadic())
    {
      return (fi.theIsDisabled ? NULL : f);
    }

    std::vector<FunctionInfo>* fv = NULL;

    if (theFunctionArityMap != NULL && theFunctionArityMap->get(qname2, fv))
    {
      ulong numFunctions = (ulong)fv->size();
      for (ulong i = 0; i < numFunctions; ++i)
      {
        if ((*fv)[i].theFunction->getArity() == arity)
          return ((*fv)[i].theIsDisabled ? NULL : (*fv)[i].theFunction.getp());
      }
    }
  }

  return NULL;
}


/***************************************************************************//**
  Find all the in-scope and non-disabled functions in this sctx and its ancestors.
********************************************************************************/
void static_context::get_functions(
    std::vector<function *>& functions) const
{
  std::vector<function*> disabled;
  std::vector<zstring> importedBuiltinModules;

  const static_context* sctx = this;

  while (sctx != NULL)
  {
    if (sctx->theImportedBuiltinModules)
    {
      std::vector<zstring>::const_iterator ite = sctx->theImportedBuiltinModules->begin();
      std::vector<zstring>::const_iterator end = sctx->theImportedBuiltinModules->end();
      for (; ite != end; ++ite)
      {
        importedBuiltinModules.push_back(*ite);
      }
    }

    if (sctx->theFunctionMap != NULL)
    {
      FunctionMap::iterator ite = sctx->theFunctionMap->begin();
      FunctionMap::iterator end = sctx->theFunctionMap->end();

      for (; ite != end; ++ite)
      {
        function* f = (*ite).second.theFunction.getp();

        if (!(*ite).second.theIsDisabled)
        {
          if (std::find(disabled.begin(), disabled.end(), f) == disabled.end())
          {
            if (f->isBuiltin())
            {
              assert(sctx->is_global_root_sctx());

              const zstring& ns = f->getName()->getNamespace();

              if (ns != W3C_FN_NS)
              {
                if (ns == XQUERY_OP_NS || ns == ZORBA_OP_NS)
                  continue;

                std::vector<zstring>::const_iterator ite = importedBuiltinModules.begin();
                std::vector<zstring>::const_iterator end = importedBuiltinModules.end();
                for (; ite != end; ++ite)
                {
                  if (ns == *ite)
                    break;
                }

                if (ite == end)
                  continue;
              }
            }
            else
            {
              assert(!sctx->is_global_root_sctx());
            }

            functions.push_back(f);
          }
        }
        else
        {
          disabled.push_back(f);
        }
      }
    }

    if (sctx->theFunctionArityMap != NULL)
    {
      FunctionArityMap::iterator ite = sctx->theFunctionArityMap->begin();
      FunctionArityMap::iterator end = sctx->theFunctionArityMap->end();

      for (; ite != end; ++ite)
      {
        std::vector<FunctionInfo>* fv = (*ite).second;

        ulong numFunctions = (ulong)fv->size();
        for (ulong i = 0; i < numFunctions; ++i)
        {
          function* f = (*fv)[i].theFunction.getp();

          if (!(*fv)[i].theIsDisabled)
          {
            if (std::find(disabled.begin(), disabled.end(), f) == disabled.end())
            {
              if (f->isBuiltin())
              {
                assert(sctx->is_global_root_sctx());

                const zstring& ns = f->getName()->getNamespace();

                if (ns != W3C_FN_NS)
                {
                  if (ns == XQUERY_OP_NS || ns == ZORBA_OP_NS)
                    continue;

                  std::vector<zstring>::const_iterator ite = importedBuiltinModules.begin();
                  std::vector<zstring>::const_iterator end = importedBuiltinModules.end();
                  for (; ite != end; ++ite)
                  {
                    if (ns == *ite)
                      break;
                  }

                  if (ite == end)
                    continue;
                }
              }
              else
              {
                assert(!sctx->is_global_root_sctx());
              }

              functions.push_back(f);
            }
          }
          else
          {
            disabled.push_back(f);
          }
        }
      }
    }

    sctx = sctx->theParent;
  }
}


/***************************************************************************//**
  Find all the functions with the given qname.
********************************************************************************/
void static_context::find_functions(
    const store::Item* qname,
    std::vector<function *>& functions) const
{
  FunctionInfo fi;
  store::Item* qname2 = const_cast<store::Item*>(qname);

  if (theFunctionMap != NULL && theFunctionMap->get(qname2, fi))
  {
    if (!fi.theIsDisabled)
      functions.push_back(fi.theFunction.getp());
  }

  std::vector<FunctionInfo>* fv = NULL;

  if (theFunctionArityMap != NULL && theFunctionArityMap->get(qname2, fv))
  {
    ulong numFunctions = (ulong)fv->size();
    for (ulong i = 0; i < numFunctions; ++i)
    {
      if (!(*fv)[i].theIsDisabled)
        functions.push_back((*fv)[i].theFunction.getp());
    }
  }

  if (theParent != NULL)
    theParent->find_functions(qname2, functions);
}


/***************************************************************************//**
  Register an external module.  This module can be used to retrieve external
  functions defined in the target namespace of the module.

  If aDynamicallyLoaded is false, then the external module to register has been
  created and is provided directly by the application. Otherwise, it is an
  external module that is created and loaded dynamically by zorba from a lib
  file that is stored somewhere in the in-scope module paths (see
  DynamicLoader::getExternalModule method and how this method is
  invoked by the static_context::lookup_external_function method below).
********************************************************************************/
void static_context::bind_external_module(
    ExternalModule* aModule,
    bool aDynamicallyLoaded)
{
  if (theExternalModulesMap == NULL)
  {
    theExternalModulesMap = new ExternalModuleMap(8, false);
  }

  zstring uri = Unmarshaller::getInternalString(aModule->getURI());
  ctx_module_t modinfo;
  modinfo.module = aModule;
  modinfo.dyn_loaded_module = aDynamicallyLoaded;
  modinfo.sctx = this;

  if (!theExternalModulesMap->insert(uri, modinfo))
  {
    throw ZORBA_EXCEPTION(
      zerr::ZAPI0019_MODULE_ALREADY_REGISTERED, ERROR_PARAMS( uri )
    );
  }
}


/***************************************************************************//**
  Find and return the implementation of an external function, given its namespace
  URI and local name.
********************************************************************************/
ExternalFunction* static_context::lookup_external_function(
    const zstring& aURI,
    const zstring& aLocalName)
{
  // get the module for the given namespace
  bool found = false;
  ctx_module_t modinfo;
  const static_context* sctx = this;

  while (sctx != NULL)
  {
    if (sctx->theExternalModulesMap != NULL &&
        sctx->theExternalModulesMap->get(aURI, modinfo))
    {
      found = true;
      break;
    }

    sctx = sctx->theParent;
  }

  ExternalModule* lModule = 0;

  // If the module is not yet in the static context we try to get it from the
  // URI resolver
  if (!found)
  {
    lModule = DynamicLoader::getExternalModule(aURI, *this);

    // no way to get the module
    if (!lModule)
      return NULL;

    // remember the module for future use
    bind_external_module(lModule, true);
  }
  else
  {
    lModule = modinfo.module;
  }

  // get the function from this module.
  // return 0 if not found
  return lModule->getExternalFunction(aLocalName.str());
}

/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  Annotations                                                               //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////
void
static_context::add_ann(
    const std::string& aName,
    const store::Item_t& aQName)
{
  if (!theAnnotationMap) {
    theAnnotationMap = new AnnotationMap();
  }
  (*theAnnotationMap)[aName] = aQName;
}

store::Item_t
static_context::lookup_ann(const std::string& aName) const
{
  std::map<std::string, store::Item_t>::const_iterator lIter;
  if (!theAnnotationMap ||
      (lIter = theAnnotationMap->find(aName)) == theAnnotationMap->end()) {
    return theParent?theParent->lookup_ann(aName):NULL;
  }
  return lIter->second;
}

/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  Documents                                                                  //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


/***************************************************************************//**
  This method may be called only on an application-created sctx.
********************************************************************************/
void static_context::bind_document(const zstring& uri, xqtref_t& type)
{
  if (theDocumentMap == NULL)
  {
    theDocumentMap = new DocumentMap(16, false);
  }

  if (!theDocumentMap->update(uri, type))
  {
    theDocumentMap->insert(uri, type);
  }
}


/***************************************************************************//**

********************************************************************************/
const XQType* static_context::lookup_document(const zstring& uri)
{
  xqtref_t type;

  static_context* sctx = this;
  while (sctx != NULL)
  {
    if (sctx->theDocumentMap && sctx->theDocumentMap->get(uri, type))
      return type.getp();

    sctx = sctx->theParent;
  }

  return NULL;
}

void static_context::get_all_documents(std::vector<zstring>& documents)
{
  static_context* sctx = this;
  documents.clear();

  while (sctx != NULL)
  {
    if (sctx->theDocumentMap != NULL)
    {
      for(DocumentMap::iterator it = sctx->theDocumentMap->begin();
          it != sctx->theDocumentMap->end();
          ++it)
        documents.push_back(it.getKey());
    }

    sctx = sctx->theParent;
  }
}


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  W3C Collections                                                            //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


/***************************************************************************//**
  This method may be called only on an application-created sctx.
********************************************************************************/
void static_context::bind_w3c_collection(zstring& uri, xqtref_t& type)
{
  if (theW3CCollectionMap == NULL)
  {
    theW3CCollectionMap = new W3CCollectionMap(16, false);
  }

  if (!theW3CCollectionMap->update(uri, type))
  {
    theW3CCollectionMap->insert(uri, type);
  }
}


/***************************************************************************//**

********************************************************************************/
const XQType* static_context::lookup_w3c_collection(const zstring& uri)
{
  xqtref_t type;

  if (theW3CCollectionMap && theW3CCollectionMap->get(uri, type))
    return type.getp();
  else
    return (theParent == NULL ? 0 : theParent->lookup_w3c_collection(uri));
}


/***************************************************************************//**

********************************************************************************/
void static_context::set_default_w3c_collection_type(xqtref_t& t)
{
  theDefaultW3CCollectionType = t;
}


/***************************************************************************//**

********************************************************************************/
const XQType* static_context::get_default_w3c_collection_type()
{
  static_context* sctx = this;
  while (sctx != NULL)
  {
    if (sctx->theDefaultW3CCollectionType != NULL)
      return sctx->theDefaultW3CCollectionType.getp();

    sctx = sctx->theParent;
  }

  return NULL;
}


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  XQDDF Collections                                                          //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


/***************************************************************************//**

********************************************************************************/
void static_context::bind_collection(
    StaticallyKnownCollection_t& aCollection,
    const QueryLoc& aLoc)
{
  if (lookup_collection(aCollection->getName()) != NULL)
  {
    throw XQUERY_EXCEPTION(
      zerr::ZDST0001_COLLECTION_ALREADY_DECLARED,
      ERROR_PARAMS( aCollection->getName()->getStringValue() ),
      ERROR_LOC( aLoc )
    );
  }

  if (theCollectionMap == 0)
    theCollectionMap = new CollectionMap(0, NULL, 8, false);

  store::Item* qname = const_cast<store::Item*>(aCollection->getName());
  theCollectionMap->insert(qname, aCollection);
}


/***************************************************************************//**

********************************************************************************/
const StaticallyKnownCollection* static_context::lookup_collection(
    const store::Item* qname) const
{
  StaticallyKnownCollection_t lColl;
  store::Item* qname2 = const_cast<store::Item*>(qname);

  if (theCollectionMap && theCollectionMap->get(qname2, lColl))
    return lColl.getp();
  else
    return (theParent == NULL ? 0 : theParent->lookup_collection(qname));
}


/***************************************************************************//**

********************************************************************************/
store::Iterator_t static_context::collection_names() const
{
  return new SctxMapIterator<StaticallyKnownCollection>(
             this,
             &static_context::collection_map);
}


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  XQDDF Indexes                                                              //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


/***************************************************************************//**

********************************************************************************/
void static_context::bind_index(IndexDecl_t& index, const QueryLoc& loc)
{
  store::Item* qname = const_cast<store::Item*>(index->getName());

  if (lookup_index(qname) != NULL)
  {
    throw XQUERY_EXCEPTION(
      zerr::ZDST0021_INDEX_ALREADY_DECLARED,
      ERROR_PARAMS( qname->getStringValue() ),
      ERROR_LOC( loc )
    );
  }

  if (theIndexMap == NULL)
    theIndexMap = new IndexMap(0, NULL, 8, false);

  theIndexMap->insert(qname, index);
}


/***************************************************************************//**

********************************************************************************/
IndexDecl* static_context::lookup_index(const store::Item* qname) const
{
  IndexDecl_t index;
  store::Item* qname2 = const_cast<store::Item*>(qname);

  if (theIndexMap && theIndexMap->get(qname2, index))
    return index.getp();
  else
    return (theParent == NULL ? NULL : theParent->lookup_index(qname));
}


/***************************************************************************//**

********************************************************************************/
store::Iterator_t static_context::index_names() const
{
  return new SctxMapIterator<IndexDecl>(this, &static_context::index_map);
}


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  XQDDF Integrity Constraints                                                //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


/***************************************************************************//**

********************************************************************************/
void static_context::bind_ic(
    ValueIC_t& vic,
    const QueryLoc& loc)
{
  store::Item* qname = vic->getICName();

  if (lookup_ic(qname) != NULL)
  {
    throw XQUERY_EXCEPTION(
      zerr::ZDST0041_IC_ALREADY_DECLARED,
      ERROR_PARAMS( qname->getStringValue() ),
      ERROR_LOC( loc )
    );
  }

  if (theICMap == NULL)
    theICMap = new ICMap(0, NULL, 8, false);

  theICMap->insert(qname, vic);
}


/***************************************************************************//**

********************************************************************************/
ValueIC_t static_context::lookup_ic(const store::Item* qname) const
{
  ValueIC_t vic;
  store::Item* qname2 = const_cast<store::Item*>(qname);

  if (theICMap != NULL && theICMap->get(qname2, vic))
    return vic;
  else
    return (theParent == NULL ? NULL : theParent->lookup_ic(qname));
}


/***************************************************************************//**

********************************************************************************/
store::Iterator_t static_context::ic_names() const
{
  return new SctxMapIterator<ValueIC>(this, &static_context::ic_map);
}


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  Collations                                                                 //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


/***************************************************************************//**
  This method may be called only on the the zorba root sctx or an application-
  created sctx.
********************************************************************************/
void static_context::add_collation(const std::string& uri, const QueryLoc& loc)
{
  if (is_known_collation(uri))
    return;

  std::string resolvedURI = resolve_relative_uri(uri).str();

  XQPCollator* collator = CollationFactory::createCollator(resolvedURI);

  if (collator == NULL)
  {
    throw XQUERY_EXCEPTION(
      err::FOCH0002, ERROR_PARAMS( uri ), ERROR_LOC( loc )
    );
  }
  else
  {
    if (theCollationMap == NULL)
      theCollationMap = new CollationMap();

    (*theCollationMap)[resolvedURI] = collator;
  }
}


/***************************************************************************//**

********************************************************************************/
bool static_context::is_known_collation(const std::string& uri) const
{
  std::string resolvedURI = resolve_relative_uri(uri).str();

  const static_context* sctx = this;

  while (sctx != NULL)
  {
    if (sctx->theCollationMap != NULL &&
        sctx->theCollationMap->find(resolvedURI) != sctx->theCollationMap->end())
    {
      return true;
    }

    sctx = sctx->theParent;
  }

  return false;
}


/***************************************************************************//**

********************************************************************************/
XQPCollator* static_context::get_collator(
     const std::string& uri,
     const QueryLoc& loc) const
{
  std::string resolvedURI = resolve_relative_uri(uri).str();

  const static_context* sctx = this;

  while (sctx != NULL)
  {
    if (sctx->theCollationMap != NULL)
    {
      CollationMap::iterator ite = sctx->theCollationMap->find(resolvedURI);

      if (ite != sctx->theCollationMap->end())
      {
        return ite->second;
      }
    }

    sctx = sctx->theParent;
  }

  throw XQUERY_EXCEPTION(err::FOCH0002,
                         ERROR_PARAMS(uri, ZED(NotInStaticCtx)),
                         ERROR_LOC(loc));
}


/***************************************************************************//**

********************************************************************************/
void static_context::set_default_collation(
    const std::string& uri,
    const QueryLoc& loc)
{
  if (theDefaultCollation != NULL || !is_known_collation(uri))
    throw XQUERY_EXCEPTION(err::XQST0038, ERROR_LOC(loc));

  zstring resolvedUri = resolve_relative_uri(uri);

  theDefaultCollation = new std::string(resolvedUri.c_str());
}


/***************************************************************************//**

********************************************************************************/
const std::string& static_context::get_default_collation(const QueryLoc& loc) const
{
  const static_context* sctx = this;

  while (sctx != NULL)
  {
    if (sctx->theDefaultCollation != NULL)
    {
      return *sctx->theDefaultCollation;
    }

    sctx = sctx->theParent;
  }

  ZORBA_ASSERT(false);
  return *theDefaultCollation;
}


/***************************************************************************//**

********************************************************************************/
XQPCollator* static_context::get_default_collator(const QueryLoc& loc) const
{
  const std::string& default_collation = get_default_collation(loc);
  return get_collator(default_collation, loc);
}


/***************************************************************************//**

********************************************************************************/
void static_context::get_collations(std::vector<std::string>& collations) const
{
  const static_context* sctx = this;

  while (sctx != NULL)
  {
    if (theCollationMap != NULL)
    {
      CollationMap::const_iterator ite = theCollationMap->begin();
      CollationMap::const_iterator end = theCollationMap->end();
      for (; ite !=  end; ++ite)
      {
        collations.push_back(ite->first);
      }
    }

    sctx = sctx->theParent;
  }
}


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  Options                                                                    //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


/***************************************************************************//**

********************************************************************************/
void static_context::bind_option(
    const store::Item* qname,
    const zstring& value)
{
  if (theOptionMap == NULL)
  {
    theOptionMap = new OptionMap(0, NULL, 8, false);
  }

  PrologOption option(qname, value);

  store::Item* qname2 = option.theName.getp();

  if (!theOptionMap->update(qname2, option))
  {
    theOptionMap->insert(qname2, option);
  }
}


/***************************************************************************//**

********************************************************************************/
bool static_context::lookup_option(
    const store::Item* qname,
    zstring& value) const
{
  store::Item* qname2 = const_cast<store::Item*>(qname);
  PrologOption option;
  const static_context* sctx = this;
  while (sctx != NULL)
  {
    if (sctx->theOptionMap && sctx->theOptionMap->get(qname2, option))
    {
      value = option.theValue;
      return true;
    }
    sctx = sctx->theParent;
  }

  return false;
}


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  Auditing                                                                    //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


/***************************************************************************//**

********************************************************************************/
void static_context::set_audit_event(audit::Event* ae)
{
  theAuditEvent = ae;
}


/***************************************************************************//**

********************************************************************************/
audit::Event* static_context::get_audit_event()
{
  const static_context* sctx = this;
  audit::Event* res = sctx->theAuditEvent;
  while (res == &zorba::audit::NOP_EVENT_IMPL && sctx != NULL)
  {
    sctx = sctx->theParent;
    if (sctx != NULL) {
      res = sctx->theAuditEvent;
    }
  }
  return res;
}


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  Miscellaneous                                                              //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


/***************************************************************************//**

********************************************************************************/
StaticContextConsts::xquery_version_t static_context::xquery_version() const
{
  const static_context* sctx = this;

  while (sctx != NULL)
  {
    if (sctx->theXQueryVersion != StaticContextConsts::xquery_version_unknown)
      return sctx->theXQueryVersion;

    sctx = sctx->theParent;
  }

  ZORBA_ASSERT(false);
  return StaticContextConsts::xquery_version_unknown;
}


/***************************************************************************//**

********************************************************************************/
void static_context::set_xquery_version(StaticContextConsts::xquery_version_t v)
{
  theXQueryVersion = v;
}


/***************************************************************************//**

********************************************************************************/
StaticContextConsts::xpath_compatibility_t static_context::xpath_compatibility() const
{
  const static_context* sctx = this;

  while (sctx != NULL)
  {
    if (sctx->theXPathCompatibility != StaticContextConsts::xpath_unknown)
      return sctx->theXPathCompatibility;

    sctx = sctx->theParent;
  }

  ZORBA_ASSERT(false);
  return StaticContextConsts::xpath_unknown;
}


/***************************************************************************//**

********************************************************************************/
void static_context::set_xpath_compatibility(StaticContextConsts::xpath_compatibility_t v)
{
  theXPathCompatibility = v;
}


/***************************************************************************//**

********************************************************************************/
StaticContextConsts::construction_mode_t static_context::construction_mode() const
{
  const static_context* sctx = this;

  while (sctx != NULL)
  {
    if (sctx->theConstructionMode != StaticContextConsts::cons_unknown)
      return sctx->theConstructionMode;

    sctx = sctx->theParent;
  }

  ZORBA_ASSERT(false);
  return StaticContextConsts::cons_unknown;
}


/***************************************************************************//**

********************************************************************************/
void static_context::set_construction_mode(StaticContextConsts::construction_mode_t v)
{
  theConstructionMode = v;
}


/***************************************************************************//**

********************************************************************************/
StaticContextConsts::inherit_mode_t static_context::inherit_mode() const
{
  const static_context* sctx = this;

  while (sctx != NULL)
  {
    if (sctx->theInheritMode != StaticContextConsts::inherit_unknown)
      return sctx->theInheritMode;

    sctx = sctx->theParent;
  }

  ZORBA_ASSERT(false);
  return StaticContextConsts::inherit_unknown;
}


/***************************************************************************//**

********************************************************************************/
void static_context::set_inherit_mode(StaticContextConsts::inherit_mode_t v)
{
  theInheritMode = v;
}


/***************************************************************************//**

********************************************************************************/
StaticContextConsts::preserve_mode_t static_context::preserve_mode() const
{
  const static_context* sctx = this;

  while (sctx != NULL)
  {
    if (sctx->thePreserveMode != StaticContextConsts::preserve_unknown)
      return sctx->thePreserveMode;

    sctx = sctx->theParent;
  }

  ZORBA_ASSERT(false);
  return StaticContextConsts::preserve_unknown;
}


/***************************************************************************//**

********************************************************************************/
void static_context::set_preserve_mode(StaticContextConsts::preserve_mode_t v)
{
  thePreserveMode = v;
}


/***************************************************************************//**

********************************************************************************/
StaticContextConsts::ordering_mode_t static_context::ordering_mode() const
{
  const static_context* sctx = this;

  while (sctx != NULL)
  {
    if (sctx->theOrderingMode != StaticContextConsts::ordering_unknown)
      return sctx->theOrderingMode;

    sctx = sctx->theParent;
  }

  ZORBA_ASSERT(false);
  return StaticContextConsts::ordering_unknown;
}


/***************************************************************************//**

********************************************************************************/
bool static_context::is_in_ordered_mode() const
{
  return ordering_mode() == StaticContextConsts::ordered;
}


/***************************************************************************//**

********************************************************************************/
void static_context::set_ordering_mode(StaticContextConsts::ordering_mode_t v)
{
  theOrderingMode = v;
}


/***************************************************************************//**

********************************************************************************/
StaticContextConsts::empty_order_mode_t static_context::empty_order_mode() const
{
  const static_context* sctx = this;

  while (sctx != NULL)
  {
    if (sctx->theEmptyOrderMode != StaticContextConsts::empty_order_unknown)
      return sctx->theEmptyOrderMode;

    sctx = sctx->theParent;
  }

  ZORBA_ASSERT(false);
  return StaticContextConsts::empty_order_unknown;
}


/***************************************************************************//**

********************************************************************************/
void static_context::set_empty_order_mode(StaticContextConsts::empty_order_mode_t v)
{
  theEmptyOrderMode = v;
}


/***************************************************************************//**

********************************************************************************/
StaticContextConsts::boundary_space_mode_t static_context::boundary_space_mode() const
{
  const static_context* sctx = this;

  while (sctx != NULL)
  {
    if (sctx->theBoundarySpaceMode != StaticContextConsts::boundary_space_unknown)
      return sctx->theBoundarySpaceMode;

    sctx = sctx->theParent;
  }

  ZORBA_ASSERT(false);
  return StaticContextConsts::boundary_space_unknown;
}


/***************************************************************************//**

********************************************************************************/
void static_context::set_boundary_space_mode(StaticContextConsts::boundary_space_mode_t v)
{
  theBoundarySpaceMode = v;
}


/***************************************************************************//**

********************************************************************************/
StaticContextConsts::validation_mode_t static_context::validation_mode() const
{
  const static_context* sctx = this;

  while (sctx != NULL)
  {
    if (sctx->theValidationMode != StaticContextConsts::validation_unknown)
      return sctx->theValidationMode;

    sctx = sctx->theParent;
  }

  ZORBA_ASSERT(false);
  return StaticContextConsts::validation_unknown;
}


/***************************************************************************//**

********************************************************************************/
void static_context::set_validation_mode(StaticContextConsts::validation_mode_t v)
{
  theValidationMode = v;
}


/***************************************************************************//**

********************************************************************************/
void static_context::add_decimal_format(
    const DecimalFormat_t& decimalFormat,
    const QueryLoc& loc)
{
  if (theDecimalFormats == NULL)
    theDecimalFormats = new std::vector<DecimalFormat_t>;
  else {
    ulong num = (ulong)theDecimalFormats->size();
    for (ulong i = 0; i < num; ++i)
    {
      const DecimalFormat_t& format = (*theDecimalFormats)[i];

      if (decimalFormat->isDefault() && format->isDefault())
      {
        throw XQUERY_EXCEPTION(
          err::XQST0111,
          ERROR_PARAMS( ZED( TwoDefaultDecimalFormats ) ),
          ERROR_LOC( loc )
        );
      }

      if (!format->isDefault() &&
          !decimalFormat->isDefault() &&
          format->getName()->equals(decimalFormat->getName()))
      {
        throw XQUERY_EXCEPTION(
          err::XQST0111,
          ERROR_PARAMS(
            ZED( TwoDecimalFormatsSameName_2 ),
            format->getName()->getStringValue()
          ),
          ERROR_LOC( loc )
        );
      }
    }
  }

  theDecimalFormats->push_back(decimalFormat);
}


/***************************************************************************//**

********************************************************************************/
DecimalFormat_t static_context::get_decimal_format(const store::Item_t& qname)
{
  if (theDecimalFormats != NULL)
  {
    ulong num = (ulong)theDecimalFormats->size();

    for (ulong i = 0; i < num; ++i)
    {
      const DecimalFormat_t& format = (*theDecimalFormats)[i];

      if ((qname == NULL && format->isDefault()) ||
          (qname != NULL && !format->isDefault() && format->getName()->equals(qname)))
      {
        return format;
      }
    }
  }

  return (theParent == NULL ? NULL : theParent->get_decimal_format(qname));
}

/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  Warnings                                                                   //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////

/*******************************************************************************

********************************************************************************/
void static_context::disableWarning(store::Item_t qname)
{
  for (unsigned int i=0; i<theDisabledWarnings.size(); i++)
    if (qname->equals(theDisabledWarnings[i]))
      return;

  theDisabledWarnings.push_back(qname);
}

/*******************************************************************************

********************************************************************************/
void static_context::disableAllWarnings()
{
  theAllWarningsDisabled = true;
}

/*******************************************************************************

********************************************************************************/
void static_context::setWarningAsError(store::Item_t qname)
{
  for (unsigned int i=0; i<theWarningsAreErrors.size(); i++)
    if (theWarningsAreErrors[i] )
      return;

  theWarningsAreErrors.push_back(qname);
}

/*******************************************************************************

********************************************************************************/
bool static_context::isWarningDisabled(const char* ns, const char* localname)
{
  if (theAllWarningsDisabled)
    return true;

  for (unsigned int i=0; i<theDisabledWarnings.size(); i++)
    if (theDisabledWarnings[i]->getNamespace() == ns && theDisabledWarnings[i]->getLocalName() == localname)
      return true;

  return (theParent == NULL ? false : theParent->isWarningDisabled(ns, localname));
}

/*******************************************************************************

********************************************************************************/
bool static_context::isWarningAnError(const char* ns, const char* localname)
{
  for (unsigned int i=0; i<theWarningsAreErrors.size(); i++)
    if (theWarningsAreErrors[i]->getNamespace() == ns && theWarningsAreErrors[i]->getLocalName() == localname)
      return true;

  return (theParent == NULL ? false : theParent->isWarningAnError(ns, localname));
}


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  Module Import                                                              //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


/***************************************************************************//**
  Merge the static context of a module with this static context. Only functions
  and variables defined in the module are included in this static context.
********************************************************************************/
void static_context::import_module(const static_context* module, const QueryLoc& loc)
{
  if (module->theVariablesMap)
  {
    if (theVariablesMap == NULL)
    {
      theVariablesMap = new VariableMap(0,
                                        NULL,
                                        (ulong)module->theVariablesMap->capacity(),
                                        false);
    }

    VariableMap::iterator ite = module->theVariablesMap->begin();
    VariableMap::iterator end = module->theVariablesMap->end();
    for (; ite != end; ++ite)
    {
      var_expr_t ve = ite.getValue();

#if 0
      // This check has been moved to the translator (in the translation of
      // a VarRef).
      xqtref_t type = ve->get_return_type();
      if (!TypeOps::is_in_scope(get_typemanager(), *type))
      {
        ZORBA_ERROR_LOC_DESC_OSS(XQST0036, loc,
                                 "The type " << type->toString()
                                 << " is not among the in-scope types"
                                 << " of the importing module");
      }
#endif

      if (!ve->is_private())
        bind_var(ve, loc, err::XQST0049);
    }
  }

  if (module->theFunctionMap)
  {
    if (theFunctionMap == NULL)
    {
      theFunctionMap = new FunctionMap(0,
                                       NULL,
                                       (ulong)module->theFunctionMap->capacity(),
                                       false);
    }

    FunctionMap::iterator ite = module->theFunctionMap->begin();
    FunctionMap::iterator end = module->theFunctionMap->end();
    for (; ite != end; ++ite)
    {
      function_t f = (*ite).second.theFunction;
#if 0
      // This check has been moved to the translator (in the translation of
      // a FunctionCall).
      const signature& sign = f->getSignature();
      ulong numArgs = f->getArity();

      for (ulong i = 0; i < numArgs; ++i)
      {
        xqtref_t type = sign[i];
        if (!TypeOps::is_in_scope(get_typemanager(), *type))
        {
          ZORBA_ERROR_LOC_DESC_OSS(XQST0036, loc,
                                   "The type " << type->toString()
                                   << " is not among the in-scope types"
                                   << " of the importing module");
        }
      }

      xqtref_t type = sign.return_type();
      if (!TypeOps::is_in_scope(get_typemanager(), *type))
      {
        ZORBA_ERROR_LOC_DESC_OSS(XQST0036, loc,
                                 "The type " << type->toString()
                                 << " is not among the in-scope types"
                                 << " of the importing module");
      }
#endif
      if (!f->isPrivate())
        bind_fn(f, f->getArity(), loc);
    }
  }

  if (module->theFunctionArityMap)
  {
    if (theFunctionArityMap == NULL)
    {
      theFunctionArityMap = new FunctionArityMap(0,
                                                 NULL,
                                                 (ulong)module->theFunctionArityMap->capacity(),
                                                 false);
    }

    FunctionArityMap::iterator ite = module->theFunctionArityMap->begin();
    FunctionArityMap::iterator end = module->theFunctionArityMap->end();
    for (; ite != end; ++ite)
    {
      std::vector<FunctionInfo>* fv = (*ite).second;
      ulong num = (ulong)fv->size();
      for (ulong i = 0; i < num; ++i)
      {
        function_t f = (*fv)[i].theFunction;

#if 0
      // This check has been moved to the translator (in the translation of
      // a FunctionCall).
        const signature& sign = f->getSignature();
        ulong numArgs = f->getArity();

        for (ulong i = 0; i < numArgs; ++i)
        {
          xqtref_t type = sign[i];
          if (!TypeOps::is_in_scope(get_typemanager(), *type))
          {
            ZORBA_ERROR_LOC_DESC_OSS(XQST0036, loc,
                                     "The type " << type->toString()
                                     << " is not among the in-scope types"
                                     << " of the importing module");
          }
        }

        xqtref_t type = sign.return_type();
        if (!TypeOps::is_in_scope(get_typemanager(), *type))
        {
          ZORBA_ERROR_LOC_DESC_OSS(XQST0036, loc,
                                   "The type " << type->toString()
                                   << " is not among the in-scope types"
                                   << " of the importing module");
        }
#endif

        bind_fn(f, f->getArity(), loc);
      }
    }
  }

  if (module->theCollectionMap)
  {
    if (theCollectionMap == 0)
      theCollectionMap = new CollectionMap(0, 0, 8, false);

    CollectionMap::iterator coll_iter = module->theCollectionMap->begin();
    CollectionMap::iterator coll_end = module->theCollectionMap->end();
    for (; coll_iter != coll_end; ++ coll_iter)
    {
      std::pair<store::Item*, StaticallyKnownCollection_t > pair = (*coll_iter);

      if (!theCollectionMap->insert(pair.first, pair.second))
      {
        throw XQUERY_EXCEPTION(
          zerr::ZDST0002_COLLECTION_ALREADY_IMPORTED,
          ERROR_PARAMS(
            pair.second->getName()->getStringValue(),
            module->get_module_namespace()
          ),
          ERROR_LOC( loc )
        );
      }
    }
  }

  if (module->theIndexMap)
  {
    if (theIndexMap == NULL)
      theIndexMap = new IndexMap(0, NULL, 8, false);

    IndexMap::iterator idx_iter = module->theIndexMap->begin();
    IndexMap::iterator idx_end = module->theIndexMap->end();
    for (; idx_iter != idx_end; ++idx_iter)
    {
      std::pair<store::Item*, rchandle<IndexDecl> > pair = (*idx_iter);

      if (lookup_index(pair.first) != NULL)
      {
        throw XQUERY_EXCEPTION(
          zerr::ZDST0022_INDEX_ALREADY_IMPORTED,
          ERROR_PARAMS(
            pair.first->getStringValue(), module->get_module_namespace()
          ),
          ERROR_LOC( loc )
        );
      }

      if (!theIndexMap->insert(pair.first, pair.second))
      {
        ZORBA_ASSERT(false);
      }
    }
  }

  if (module->theICMap)
  {
    if (theICMap == NULL)
    {
      theICMap = new ICMap(0, NULL, 8, false);
    }

    ICMap::iterator ic_iter = module->theICMap->begin();
    ICMap::iterator ic_end = module->theICMap->end();
    for (; ic_iter != ic_end; ++ic_iter)
    {
      std::pair<store::Item*, rchandle<ValueIC> > pair = (*ic_iter);

      if (!theICMap->insert(pair.first, pair.second))
      {
        throw XQUERY_EXCEPTION(
          zerr::ZDST0041_IC_ALREADY_DECLARED,
          ERROR_PARAMS( pair.first->getStringValue() ),
          ERROR_LOC( loc )
        );
      }
    }
  }
}

} // namespace zorba
/* vim:set et sw=2 ts=2: */
