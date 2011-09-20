/*
 * Copyright 2006-2009 The FLWOR Foundation.
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
#ifndef ZORBA_SERIALIZABLE_H
#define ZORBA_SERIALIZABLE_H

#include <store/api/item.h>

namespace zorba { namespace intern
{

  class Serializable {

    public:

      virtual ~Serializable() {}

      virtual bool nextSerializableItem(store::Item_t& item) = 0;

  };

} // namespace intern

} // namespace zorba

#endif
/* vim:set et sw=2 ts=2: */
