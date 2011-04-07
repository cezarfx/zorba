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

#ifndef WIN32
# include <cstdio>
# include <cstring>
#else
# include <windows.h>
# include <strsafe.h>
#endif /* WIN32 */

#include <zorba/err.h>

#include "zorbaerrors/dict.h"

#include "error_util.h"
#include "stl_util.h"

namespace zorba {
namespace error {

using namespace std;

///////////////////////////////////////////////////////////////////////////////

zstring get_os_err_string( char const *what, os_code code ) {
  internal::err::parameters params;
  internal::err::parameters::value_type result;
#ifndef WIN32
  char const *const err_string = ::strerror( code );
#else
  LPWSTR werr_string;
  FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER
    | FORMAT_MESSAGE_FROM_SYSTEM
    | FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL, code, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
    reinterpret_cast<LPWSTR>( &werr_string ), 0, NULL
  );
  int const err_size = ::wcslen( werr_string ) * 3;
  ztd::auto_vec<char> const err_buf( new char[ err_size ] );
  char *const err_string = err_buf.get();
  WideCharToMultiByte(
    CP_UTF8, 0, werr_string, -1, err_string, err_size, NULL, NULL
  );
  LocalFree( werr_string );
#endif /* WIN32 */
  if ( what && *what ) {
    result = err::dict::lookup( ZED( OSWhatFailedError ) );
    params = ERROR_PARAMS( what, code, err_string );
  } else {
    result = err::dict::lookup( ZED( OSFailedError ) );
    params = ERROR_PARAMS( code, err_string );
  }
  params.substitute( &result );
  return zstring( result );
}

///////////////////////////////////////////////////////////////////////////////

} // namespace error
} // namespace zorba

/*
 * Local variables:
 * mode: c++
 * End:
 */
/* vim:set et sw=2 ts=2: */
