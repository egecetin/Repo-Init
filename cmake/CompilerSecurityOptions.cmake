#
# Copyright (C) 2024 by Ege Cetin - egecetin@hotmail.com.tr
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
# specific language governing permissions and limitations under the License.

option(ENABLE_RECOMMENDED_SECURITY_FLAGS "Compile all targets with recommended security hardening flags. Check \
https://best.openssf.org/Compiler-Hardening-Guides/Compiler-Options-Hardening-Guide-for-C-and-C++ \
for information" OFF)

set(COMPILER_SECURE_FLAGS_ENABLED -Wconversion -Wtrampolines -Wimplicit-fallthrough
  -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3 -D_GLIBCXX_ASSERTIONS -fstrict-flex-arrays=3 
  -fstack-clash-protection -fstack-protector-strong -Wl,-z,nodlopen -Wl,-z,noexecstack 
  -Wl,-z,relro -Wl,-z,now)
set(COMPILER_SECURE_FLAGS_DISABLED "")

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "13.0")
    list(APPEND COMPILER_SECURE_FLAGS_DISABLED -fstrict-flex-arrays=3)
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "12.0")
      list(APPEND COMPILER_SECURE_FLAGS_DISABLED -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3)
      if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "8.0")
        message(ERROR "GNU compiler version less than 8.0. Security flags are not supported")
      endif() # Version 8.0
    endif() # Version 12.0
  endif() # Version 13.0
elseif(CMAKE_CXX_COMPILER_ID MATCHES "(Apple)?[Cc]lang")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "16.0")
    list(APPEND COMPILER_SECURE_FLAGS_DISABLED -fstrict-flex-arrays=3)
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "12.0")
      list(APPEND COMPILER_SECURE_FLAGS_DISABLED -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3)
      if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "11.0")
        message(ERROR "Clang compiler version less than 11.0. Security flags are not supported")
      endif() # Version 11.0
    endif() # Version 12.0
  endif() # Version 16.0
else()
  message(ERROR "Compiler ${CMAKE_CXX_COMPILER_ID} not supported for hardening flags")
endif()

list(REMOVE_ITEM COMPILER_SECURE_FLAGS_ENABLED ${COMPILER_SECURE_FLAGS_DISABLED})

# Enable flags
message(STATUS "Supported hardening flags ${COMPILER_SECURE_FLAGS_ENABLED}")
message(WARNING "Not supported hardening flags ${COMPILER_SECURE_FLAGS_DISABLED}")

macro(enable_security_flags_for_target name)
  message(STATUS "Enable hardening flags for target ${name}")
  target_compile_options(${name} PRIVATE ${COMPILER_SECURE_FLAGS_ENABLED})
endmacro()
