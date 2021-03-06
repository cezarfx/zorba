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
#ifndef ZORBA_API_STATIC_CONTEXT_IMPL_H
#define ZORBA_API_STATIC_CONTEXT_IMPL_H

#include <zorba/static_context.h>
#include <zorba/api_shared_types.h>
#include <map>

#include "common/shared_types.h"


namespace zorba {

class DiagnosticHandler;
class StaticCollectionManagerImpl;
class static_context;
class function;


/*******************************************************************************

  theCtx :
  --------
  rchandle to the internal static_context obj that is wrapped by "this".

  theMaxVarId :
  -------------
  If loadProlog() is called on "this", the compiler will store in theMaxVarId
  the max variable id generated by the compilation of the given prolog. This 
  is needed by the "outer" query, where the prolog will be loaded to.

  theDiagnosticHandler :
  ----------------------
  Pointer to an error handle to handle any errors raised by the methods of 
  this class.

  theUserDiagnosticHandler :
  --------------------------
  If true, theDiagnosticHandler is owned by "this", and it should be deleted
  when "this" is destroyed. Otherwise, theDiagnosticHandler is not owned by
  "this".
********************************************************************************/
class StaticContextImpl : public StaticContext
{
  friend class Unmarshaller; // needs to get the context out of this class
  friend class XQueryImpl;   // needed for accessing theSctxMap

protected:
  static_context_t                      theCtx;

  CompilerCB                          * theCompilerCB;

  ulong                                 theMaxVarId;

  DiagnosticHandler                   * theDiagnosticHandler;
  bool                                  theUserDiagnosticHandler;

  // allow for lazy creation
  mutable StaticCollectionManagerImpl * theCollectionMgr;

protected:
  static std::string createInvokeQuery(const function*, size_t aArity);

  static std::string createHOFQuery(size_t aArity);

private:
  StaticContextImpl(const StaticContextImpl&);

  ItemSequence_t
  invokeFunc(
      const Item& aItem,
      const store::Item_t& qname,
      const std::vector<ItemSequence_t>& aArgs) const;

  ItemSequence_t
  invokeFuncItem(
      const Item& aItem,
      const store::Item_t& funItem,
      const std::vector<ItemSequence_t>& aArgs) const;

public:
  StaticContextImpl(DiagnosticHandler* = 0);

  StaticContextImpl(static_context*, DiagnosticHandler*);

  virtual ~StaticContextImpl();

  void loadProlog(const String&, const Zorba_CompilerHints_t& hints);

  ulong getMaxVarId() const { return theMaxVarId; }

  void setMaxVarId(ulong v) { if (v > theMaxVarId) theMaxVarId = v; }

  StaticContext_t createChildContext() const;

  bool addNamespace(const String& prefix, const String& URI);

  String getNamespaceURIByPrefix(const String& prefix) const;

  void getNamespaceBindings(NsBindings& aBindings) const;

  bool setDefaultElementAndTypeNamespace(const String& URI);

  String getDefaultElementAndTypeNamespace() const;

  bool setDefaultFunctionNamespace(const String& URI);

  virtual String getDefaultFunctionNamespace() const;

  virtual bool
  setDefaultFunctionNamespaces( const std::vector<String>& aURIs );

  virtual void
  addCollation( const String& URI );

  virtual void
  setDefaultCollation( const String& URI );

  virtual String
  getDefaultCollation() const;

  virtual bool
  setXQueryVersion(xquery_version_t mode);

  virtual bool
  setJSONiqVersion(jsoniq_version_t aMode);

  virtual xquery_version_t
  getXQueryVersion() const;

  virtual jsoniq_version_t
  getJSONiqVersion( ) const;

  virtual bool
  setXPath1_0CompatibMode( xpath1_0compatib_mode_t mode );

  virtual xpath1_0compatib_mode_t
  getXPath1_0CompatibMode( ) const;

  virtual bool
  setConstructionMode( construction_mode_t );

  virtual construction_mode_t
  getConstructionMode( ) const;

  virtual bool
  setOrderingMode( ordering_mode_t );

  virtual ordering_mode_t
  getOrderingMode( ) const;

  virtual bool
  setDefaultOrderForEmptySequences( order_empty_mode_t );

  virtual order_empty_mode_t
  getDefaultOrderForEmptySequences( ) const;

  virtual bool
  setBoundarySpacePolicy( boundary_space_mode_t );

  virtual boundary_space_mode_t
  getBoundarySpacePolicy( ) const;

  virtual bool
  setCopyNamespacesMode( preserve_mode_t preserve,
                         inherit_mode_t inherit );

  virtual void
  getCopyNamespacesMode( preserve_mode_t& preserve,
                         inherit_mode_t& inherit ) const;

  virtual bool
  setBaseURI( const String& baseURI );

  virtual String
  getBaseURI( ) const;

  virtual validation_mode_t
  getRevalidationMode() const;

  virtual void
  setRevalidationMode(validation_mode_t aMode);

  virtual bool
  registerModule(ExternalModule* aModule);

  virtual void
  setDocumentType(const String& aDocUri, const SequenceType& type);

  virtual SequenceType
  getDocumentType(const String& aDocUri) const;

  virtual void
  registerURIMapper(URIMapper* aMapper);

  virtual void
  registerURLResolver(URLResolver* aResolver);

  virtual void
  setCollectionType(const String& aCollectionUri, const SequenceType& type);

  virtual SequenceType
  getCollectionType(const String& aCollectionUri) const;

  virtual bool
  containsFunction(const String& aFnNameUri, const String& aFnNameLocal, int arity) const;

  virtual void
  findFunctions(const Item& aQName, std::vector<Function_t>& aFunctions) const;

  virtual void
  disableFunction(const Function_t& aFunction);

  virtual void
  disableFunction(const Item& aQName, int arity);

  virtual void
  getFunctionAnnotations(const Item& aQName, int arity, std::vector<Annotation_t>& aAnnotations) const;

  virtual void
  getFunctions(std::vector<Function_t>& aFunctions) const;

  virtual void
  getFunctions(
      const String& aFnNameUri,
      uint32_t arity,
      std::vector<Function_t>& aFunctions) const;

  virtual void
  setContextItemStaticType(const SequenceType& type);

  virtual SequenceType
  getContextItemStaticType() const;

  virtual void
  setTraceStream(std::ostream&);

  virtual void
  resetTraceStream();

  virtual bool
  getOption( const Item& aQName, String& aOptionValue) const;

  virtual void
  declareOption( const Item& aQName, const String& aOptionValue);

  virtual void
  setModulePaths( const std::vector<String>& aModulePaths );

  virtual void
  getModulePaths( std::vector<String>& aModulePaths ) const;

  virtual void
  getFullModulePaths( std::vector<String>& aFullModulePaths ) const;

  virtual String
  resolve(const String& aRelativeUri) const;

  virtual String
  resolve(const String& aRelativeUri, const String& aBaseUri) const;

  virtual bool
  validate(
      const Item& rootElement,
      Item& validatedResult,
      validation_mode_t validationMode = validate_strict) const;

  virtual bool
  validate(
      const Item& rootElement, Item& validatedResult,
      const String& targetNamespace,
      validation_mode_t validationMode = validate_strict) const;

  virtual bool
  validateSimpleContent(
      const String& stringValue,
      const Item& typeQName,
      std::vector<Item>& resultList) const;

  ItemSequence_t
  invoke(const Item& item, const std::vector<ItemSequence_t>& aArgs) const;

  virtual StaticCollectionManager*
  getStaticCollectionManager() const;

  virtual void
  setAuditEvent(audit::Event* anEvent);
  
  virtual audit::Event*
  getAuditEvent() const;

  virtual void
  getExternalVariables(Iterator_t& aVarsIter) const;  

  virtual bool
  getExternalVariableAnnotations( Item const&,
                                  std::vector<Annotation_t>& ) const;

  virtual void
  setURIPath(const std::vector<String>& aURIPath);

  virtual void
  getURIPath(std::vector<String>& aURIPath) const;

  virtual void
  getFullURIPath(std::vector<String>& aURIPath) const;

  virtual void
  setLibPath(const std::vector<String>& aLibPath);

  virtual void
  getLibPath(std::vector<String>& aLibPath) const;

  virtual void
  getFullLibPath(std::vector<String>& aLibPath) const;

  virtual Item
  fetch(const String& aURI) const;

  virtual Item
  fetch(const String& aURI, const String& aEntityKind) const;

  virtual Item
  fetch(
      const String& aURI,
      const String& aEntityKind,
      const String& aEncoding) const;

  virtual Item
  fetchBinary(const String& aURI) const;

  virtual Item
  fetchBinary(const String& aURI, const String& aEntityKind) const;

  virtual void
  clearBaseURI();
};

} // namespace zorba
#endif
/*
 * Local variables:
 * mode: c++
 * End:
 */
/* vim:set et sw=2 ts=2: */
