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

option(ENABLE_RECOMMENDED_SECURITY_FLAGS "Compile all targets with recommended security hardening flags. Check https://best.openssf.org/Compiler-Hardening-Guides/Compiler-Options-Hardening-Guide-for-C-and-C++" OFF)

set(COMPILER_SECURE_FLAGS -Wconversion -Wtrampolines -Wimplicit-fallthrough
-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3
-D_GLIBCXX_ASSERTIONS
-fstrict-flex-arrays=3
-fstack-clash-protection -fstack-protector-strong
-Wl,-z,nodlopen -Wl,-z,noexecstack
-Wl,-z,relro -Wl,-z,now)

function(enable_security_flags_for_target)
    set(options NONE)
    set(oneValueArgs TARGET NAME)
    cmake_parse_arguments(SECURE "${options}" "${oneValueArgs}" ${ARGN})

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "13.0")
            list(APPEND DISABLED_OPTIONS "-fstrict-flex-arrays=3")
            if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "12.0")
                list(APPEND DISABLED_OPTIONS "-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3")
                if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
                    message(ERROR "GNU compiler version less than 9.0. Security flags are not supported for older versions with CompilerSecurityOptions.cmake. Please enable supported flags manually")
                else()
                    # All flags except strict-flex-arrays and fortify source
                    target_compile_options(SECURE_TARGET PRIVATE
                    -Wconversion -Wtrampolines -Wimplicit-fallthrough
                    -fstack-clash-protection -fstack-protector-strong
                    -Wl,-z,nodlopen -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now)
                    message(WARNING "GNU compiler version less than 12 following flags are not supported: ${DISABLED_OPTIONS}")
                endif()
            else()
                # All flags except strict-flex-arrays
                target_compile_options(SECURE_TARGET PRIVATE
                    -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3
                    -Wconversion -Wtrampolines -Wimplicit-fallthrough
                    -fstack-clash-protection -fstack-protector-strong
                    -Wl,-z,nodlopen -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now)
                message(WARNING "GNU compiler version less than 13 following flags are not supported: ${DISABLED_OPTIONS}")
            endif()
        else()
            # Enable all flags
        endif()
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "(Apple)?[Cc]lang")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "your.required.gcc.version")
        else()
        endif()
    endif()
endfunction(enable_security_flags_for_target)
