# Copyright 2006-2008 The FLWOR Foundation.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
# http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

SET(DEBUGGER_SRCS
  debugger_clientimpl.h
  debugger_clientimpl.cpp
  socket_streambuf.h
  socket_streambuf.cpp

  query_locationimpl.cpp
  socket.cpp
  socket_streambuf.cpp
  debugger_exception.cpp
  profiler.cpp

  debugger_server.cpp
  debugger_communicator.cpp
  debugger_runtime.cpp
  debugger_protocol.cpp
  debugger_commons.cpp
)
