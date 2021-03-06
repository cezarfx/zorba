/*
 * Copyright 2006-2011 The FLWOR Foundation.
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

#ifndef ZORBA_RUNTIME_JSON_JSONML_OBJECT_H
#define ZORBA_RUNTIME_JSON_JSONML_OBJECT_H

#include "store/api/item.h"

namespace zorba {
namespace jsonml_object {

///////////////////////////////////////////////////////////////////////////////

void json_to_xml( store::Item_t const &json_item, store::Item_t *xml_item );
void xml_to_json( store::Item_t const &xml_item, store::Item_t *json_item );

///////////////////////////////////////////////////////////////////////////////

} // namespace jsonml_object
} // namespace zorba
#endif /* ZORBA_RUNTIME_JSON_JSONML_OBJECT_H */
/* vim:set et sw=2 ts=2: */
