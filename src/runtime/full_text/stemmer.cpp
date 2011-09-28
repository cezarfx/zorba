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

#include "util/cxx_util.h"
#include "zorbautils/locale.h"
#include "zorbautils/mutex.h"

#include "stemmer.h"
#include "stemmer/sb_stemmer.h"

using namespace std;
using namespace zorba::locale;

namespace zorba {
namespace internal {

///////////////////////////////////////////////////////////////////////////////

Stemmer::~Stemmer() {
  // do nothing
}

StemmerProvider::~StemmerProvider() {
  // do nothing
}

StemmerProvider const& StemmerProvider::get_default() {
  static StemmerProvider default_provider;
  return default_provider;
}

Stemmer::ptr StemmerProvider::get_stemmer( iso639_1::type lang ) const {
  typedef unique_ptr<SnowballStemmer const> cache_ptr;

  static cache_ptr cached_stemmers[ iso639_1::NUM_ENTRIES ];
  static Mutex mutex;

  if ( !lang )
    lang = get_host_lang();

  AutoMutex const lock( &mutex );
  cache_ptr &ptr_ref = cached_stemmers[ lang ];
  if ( !ptr_ref )
    ptr_ref.reset( SnowballStemmer::create( lang ) );
  return Stemmer::ptr( ptr_ref.get() );
}

///////////////////////////////////////////////////////////////////////////////

} // namespace internal
} // namespace zorba
/* vim:set et sw=2 ts=2: */
