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

#include <map>
#include <string>
#include <iostream>

#include <zorba/internal/unit_tests.h>

#include "unit_test_list.h"

typedef int (*libunittestfunc)(int,char*[]);

using namespace std;

namespace zorba {
namespace UnitTests {

///////////////////////////////////////////////////////////////////////////////

map<string,libunittestfunc> libunittests;

/**
 * ADD NEW UNIT TESTS HERE
 */
void initializeTestList() 
{
  libunittests["ato"] = test_ato_;
  libunittests["base64"] = test_base64;
  libunittests["base64_streambuf"] = test_base64_streambuf;
  libunittests["fs_util"] = test_fs_util;
  libunittests["hashmaps"] = test_hashmaps;
  libunittests["hexbinary"] = test_hexbinary;
  libunittests["hexbinary_streambuf"] = test_hexbinary_streambuf;

#ifndef ZORBA_NO_ICU
  libunittests["icu_streambuf"] = test_icu_streambuf;
#endif /* ZORBA_NO_ICU */

  libunittests["mem_sizeof"] = test_mem_sizeof;

  libunittests["json_parser"] = test_json_parser;
  libunittests["parameters"] = test_parameters;
  libunittests["string"] = test_string;
  libunittests["time"] = test_time;
  libunittests["time_parse"] = test_time_parse;

#ifndef ZORBA_NO_FULL_TEXT
  libunittests["stemmer"] = test_stemmer;
  libunittests["thesaurus"] = test_thesaurus;
  libunittests["tokenizer"] = test_tokenizer;
#endif /* ZORBA_NO_FULL_TEXT */

#ifndef ZORBA_HAVE_UNIQUE_PTR
  libunittests["unique_ptr"] = test_unique_ptr;
#endif /* ZORBA_HAVE_UNIQUE_PTR */

  libunittests["utf8_streambuf"] = test_utf8_streambuf;
  libunittests["uuid"] = test_uuid;

#ifndef ZORBA_HAVE_UNORDERED_MAP
  libunittests["unordered_map"] = test_unordered_map;
#endif /* ZORBA_HAVE_UNORDERED_MAP */

#ifndef ZORBA_HAVE_UNORDERED_SET
  libunittests["unordered_set"] = test_unordered_set;
#endif /* ZORBA_HAVE_UNORDERED_SET */

  libunittests["uri"] = runUriTest;

#ifdef ZORBA_WITH_DEBUGGER
  // libunittests["debugger_protocol"] = runDebuggerProtocolTest;
#endif /* ZORBA_WITH_DEBUGGER */
}


int runUnitTest(int argc, char* argv[]) 
{
  // Look up the function pointer for the test based on the first arg
  if (argc < 1)
  {
    cerr << "No test name passed to UnitTests::runUnitTest!" << '\n';
    return 240;
  }

  initializeTestList();

  map<string,libunittestfunc>::iterator iter = libunittests.find(argv[0]);
  if (iter == libunittests.end())
  {
    cerr << "No registered test matches '" << argv[0] << "'" << '\n';
    return 241;
  }
  return (* iter->second)(argc, argv);
}

///////////////////////////////////////////////////////////////////////////////

} // namespace UnitTests
} // namespace zorba
/* vim:set et sw=2 ts=2: */
