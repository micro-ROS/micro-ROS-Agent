# Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

include(ExternalProject)

unset(_deps)

enable_language(C)
enable_language(CXX)

unset(xrceagent_DIR CACHE)
find_package(xrceagent 2 EXACT QUIET)
if(NOT xrceagent_FOUND)
    ExternalProject_Add(xrceagent
            GIT_REPOSITORY
                https://github.com/eProsima/Micro-XRCE-DDS-Agent.git
            GIT_TAG
                v2.4.2
            PREFIX
                ${PROJECT_BINARY_DIR}/agent
            INSTALL_DIR
                ${CMAKE_INSTALL_PREFIX}
            CMAKE_CACHE_ARGS
                -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
                -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
                -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
                -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
                -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
                -DCMAKE_EXE_LINKER_FLAGS:STRING=${CMAKE_EXE_LINKER_FLAGS}
                -DCMAKE_SHARED_LINKER_FLAGS:STRING=${CMAKE_SHARED_LINKER_FLAGS}
                -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
                -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
                -DCMAKE_PREFIX_PATH:PATH=<INSTALL_DIR>
                -DCMAKE_SYSTEM_NAME:STRING=${CMAKE_SYSTEM_NAME}
                -DUAGENT_USE_SYSTEM_FASTDDS:BOOL=ON
                -DUAGENT_USE_SYSTEM_FASTCDR:BOOL=ON
                -DUAGENT_USE_SYSTEM_LOGGER:BOOL=${UAGENT_USE_SYSTEM_LOGGER}
                -DUAGENT_CED_PROFILE:BOOL=OFF
                -DUAGENT_P2P_PROFILE:BOOL=OFF
                -DUAGENT_BUILD_EXECUTABLE:BOOL=OFF
                -DUAGENT_ISOLATED_INSTALL:BOOL=OFF
            )
endif()

# Main project.
ExternalProject_Add(micro_ros_agent
    SOURCE_DIR
        ${PROJECT_SOURCE_DIR}
    BINARY_DIR
        ${CMAKE_CURRENT_BINARY_DIR}
    CMAKE_CACHE_ARGS
        -DMICROROSAGENT_SUPERBUILD:BOOL=OFF
    INSTALL_COMMAND
        ""
    DEPENDS
        xrceagent
    )
