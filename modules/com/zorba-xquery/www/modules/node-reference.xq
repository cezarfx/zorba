xquery version "1.0";

(:
 : Copyright 2006-2009 The FLWOR Foundation.
 :
 : Licensed under the Apache License, Version 2.0 (the "License");
 : you may not use this file except in compliance with the License.
 : You may obtain a copy of the License at
 :
 : http://www.apache.org/licenses/LICENSE-2.0
 :
 : Unless required by applicable law or agreed to in writing, software
 : distributed under the License is distributed on an "AS IS" BASIS,
 : WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 : See the License for the specific language governing permissions and
 : limitations under the License.
:)

(:~
 : The module provides functions to compute a stable reference (URI) for
 : a node that is stored in a collection and vice versa.
 :
 : <p>Please see the <a href="../../html/data_lifecycle.html">data lifecycle
 : documentation</a> about details on management and manipulation of collections.</p>
 : 
 : @see <a href="../../html/data_lifecycle.html">Data Lifecycle</a>
 :
 : @author Matthias Brantner
 :
 : @project XDM/node
 :
 :)
module namespace ref = "http://www.zorba-xquery.com/modules/node-reference";

declare namespace zerr = "http://www.zorba-xquery.com/errors";

declare namespace ver = "http://www.zorba-xquery.com/options/versioning";
declare option ver:module-version "2.0";

(:~
 : Compute a stable and opaque node reference (with type xs:anyURI) for
 : a given node.
 :
 : <p>The function can only compute reference for nodes that a stored
 : in a collection [zerr:ZAPI0080].</p>
 :
 : <p>The returned URI is stable, i.e. it still can be dereferenced if
 : the node or the containing collection is modified.</p>
 :
 : @param $arg the node for which the URI should be computed
 :
 : @return the opaque URI of the node.
 :)
declare function ref:node-reference(
  $arg as node()
) as xs:anyURI external;

(:~
 : Returns the node identified by the given node reference.
 :
 : <p>The function may return the empty sequence if the node
 : that is referenced was deleted.</p>
 :
 : @param $arg the URI of the node to retrieve.
 :
 : @return the node identified by the URI passed as parameter
 :         or the empty-sequence if no node with that URI is found.
 :
 : @error zerr::ZAPI0028 if the given URI is not a valid node reference
 :  computed by the <tt>ref:node-reference</tt> function.
 :)
declare function ref:node-by-reference(
  $arg as xs:anyURI
) as node()? external;
