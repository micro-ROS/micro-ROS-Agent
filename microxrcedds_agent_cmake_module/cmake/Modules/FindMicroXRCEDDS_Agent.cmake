# Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

###############################################################################
#
# CMake module for finding eProsima MICROXRCEDDS_AGENT.
#
# Output variables:
#
# - MicroXRCEDDS_Agent_FOUND: flag indicating if the package was found
# - MicroXRCEDDS_Agent_INCLUDE_DIR: Paths to the header files
#
# Example usage:
#
#   find_package(uros_agent_cmake_module REQUIRED)
#   find_package(MicroXRCEDDS_Agent MODULE)
#   # use MicroXRCEDDS_Agent_* variables
#
###############################################################################

# lint_cmake: -convention/filename, -package/stdargs

set(MicroXRCEDDS_Agent_FOUND FALSE)

find_package(microxrcedds_agent REQUIRED CONFIG)
find_package(fastcdr REQUIRED CONFIG)
find_package(fastrtps REQUIRED CONFIG)

find_path(MicroXRCEDDS_Agent_INCLUDE_DIR NAMES uxr)

string(REGEX MATCH "^[0-9]+\\.[0-9]+" microxrcedds_agent_MAJOR_MINOR_VERSION "${microxrcedds_agent_VERSION}")
string(REGEX MATCH "^[0-9]+\\.[0-9]+" fastcdr_MAJOR_MINOR_VERSION "${fastcdr_VERSION}")
string(REGEX MATCH "^[0-9]+\\.[0-9]+" fastrtps_MAJOR_MINOR_VERSION "${fastrtps_VERSION}")

##
find_library(MicroXRCEDDS_Agent_LIBRARY_RELEASE
  NAMES microxrcedds_agent-${microxrcedds_agent_MAJOR_MINOR_VERSION} microxrcedds_agent)

find_library(MicroXRCEDDS_Agent_LIBRARY_DEBUG
  NAMES microxrcedds_agentd-${microxrcedds_agent_MAJOR_MINOR_VERSION} microxrcedds_agentd)

if(MicroXRCEDDS_Agent_LIBRARY_RELEASE AND MicroXRCEDDS_Agent_LIBRARY_DEBUG)
    set(MicroXRCEDDS_Agent_LIBRARIES
        optimized ${MicroXRCEDDS_Agent_LIBRARY_RELEASE}
        debug ${MicroXRCEDDS_Agent_LIBRARY_DEBUG}
    )
elseif(MicroXRCEDDS_Agent_LIBRARY_RELEASE)
    set(MicroXRCEDDS_Agent_LIBRARIES
        ${MicroXRCEDDS_Agent_LIBRARY_RELEASE}
    )
elseif(MicroXRCEDDS_Agent_LIBRARY_DEBUG)
    set(MicroXRCEDDS_Agent_LIBRARIES
        ${MicroXRCEDDS_Agent_LIBRARY_DEBUG}
    )
else()
    set(MicroXRCEDDS_Agent_LIBRARIES "")
endif()

##
find_library(FastRTPS_LIBRARY_RELEASE
  NAMES fastrtps-${fastrtps_MAJOR_MINOR_VERSION} fastrtps)

find_library(FastRTPS_LIBRARY_DEBUG
  NAMES fastrtpsd-${fastrtps_MAJOR_MINOR_VERSION} fastrtpsd)

if(FastRTPS_LIBRARY_RELEASE AND FastRTPS_LIBRARY_DEBUG)
    set(FastRTPS_LIBRARIES
        optimized ${FastRTPS_LIBRARY_RELEASE}
        debug ${FastRTPS_LIBRARY_DEBUG}
    )
elseif(FastRTPS_LIBRARY_RELEASE)
    set(FastRTPS_LIBRARIES
        ${MicroXRCEDDS_Agent_LIBRARY_RELEASE}
    )
elseif(FastRTPS_LIBRARY_DEBUG)
    set(FastRTPS_LIBRARIES
        ${MicroXRCEDDS_Agent_LIBRARY_DEBUG}
    )
else()
    set(FastRTPS_LIBRARIES "")
endif()

##
find_library(FastCDR_LIBRARY_RELEASE
  NAMES fastcdr-${fastcdr_MAJOR_MINOR_VERSION} fastcdr)

find_library(FastCDR_LIBRARY_DEBUG
  NAMES fastcdrd-${fastcdr_MAJOR_MINOR_VERSION} fastcdrd)

if(FastCDR_LIBRARY_RELEASE AND FastCDR_LIBRARY_DEBUG)
    set(FastCDR_LIBRARIES
        optimized ${FastCDR_LIBRARY_RELEASE}
        debug ${FastCDR_LIBRARY_DEBUG}
    )
elseif(FastCDR_LIBRARY_RELEASE)
    set(FastCDR_LIBRARIES
        ${FastCDR_LIBRARY_RELEASE}
    )
elseif(FastCDR_LIBRARY_DEBUG)
    set(FastCDR_LIBRARIES
        ${FastCDR_LIBRARY_DEBUG}
    )
else()
    set(FastCDR_LIBRARIES "")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MicroXRCEDDS_Agent
        FOUND_VAR MicroXRCEDDS_Agent_FOUND
  REQUIRED_VARS
        MicroXRCEDDS_Agent_INCLUDE_DIR
        MicroXRCEDDS_Agent_LIBRARIES
)