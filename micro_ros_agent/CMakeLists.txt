# Copyright 2018-present Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

cmake_minimum_required(VERSION 3.5)

option(UROSAGENT_GENERATE_PROFILE
  "Generates agent.refs according to the .msgs provided in the .repos" OFF
  )
set(CMAKE_C_CLANG_TIDY clang-tidy -checks=*)

option(MICROROSAGENT_SUPERBUILD "Enable superbuild compilation." ON)
option(UAGENT_USE_SYSTEM_LOGGER "Force use of system installed spdlog logger on superbuild." OFF)

if(NOT MICROROSAGENT_SUPERBUILD)
    project(micro_ros_agent LANGUAGES CXX)
else()
    project(uagent_superbuild NONE)
    include(${PROJECT_SOURCE_DIR}/cmake/SuperBuild.cmake)
    return()
endif()

find_package(ament_cmake REQUIRED)
find_package(microxrcedds_agent REQUIRED)
find_package(rosidl_cmake REQUIRED)
find_package(fastcdr REQUIRED)
find_package(fastrtps REQUIRED)
find_package(fastrtps_cmake_module REQUIRED)
find_package(rmw_dds_common REQUIRED)
find_package(rmw REQUIRED)
find_package(rcutils REQUIRED)
find_package(rmw_fastrtps_shared_cpp REQUIRED)

find_package(ament_lint_auto REQUIRED)

find_package(rosidl_typesupport_fastrtps_cpp REQUIRED)
find_package(rosidl_runtime_cpp REQUIRED)
find_package(rosidl_typesupport_cpp REQUIRED)
find_package(ament_cmake_gtest REQUIRED)

find_package(micro_ros_msgs REQUIRED)

add_executable(${PROJECT_NAME}
  src/main.cpp
  src/agent/Agent.cpp
  src/agent/graph_manager/graph_manager.cpp
  src/agent/graph_manager/graph_typesupport.cpp
  src/agent/utils/demangle.cpp
  )

target_include_directories(${PROJECT_NAME}
  PRIVATE
    include
  )

ament_target_dependencies(${PROJECT_NAME}
  rosidl_typesupport_fastrtps_cpp
  rosidl_runtime_cpp
  rosidl_typesupport_cpp
  fastcdr
  fastrtps
  rmw_dds_common
  rmw
  rmw_fastrtps_shared_cpp
  micro_ros_msgs
  )

target_link_libraries(${PROJECT_NAME}
  microxrcedds_agent
  fastcdr
  fastrtps
  $<$<BOOL:$<PLATFORM_ID:Linux>>:rt>
  $<$<BOOL:$<PLATFORM_ID:Linux>>:dl>
  )

target_compile_options(${PROJECT_NAME}
  PRIVATE
    $<$<C_COMPILER_ID:GNU>:-Wall>
    $<$<C_COMPILER_ID:GNU>:-Wextra>
    $<$<C_COMPILER_ID:GNU>:-pedantic>
  )

set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_STANDARD
    14
  CXX_STANDARD_REQUIRED
    YES
  )

set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_STANDARD
    14
  CXX_STANDARD_REQUIRED
    YES
  )

target_compile_options(${PROJECT_NAME}
  PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-Wall>
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-Wextra>
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-Wpedantic>
  )

ament_export_dependencies(microxrcedds_agent)

ament_package()

install(
  TARGETS
    ${PROJECT_NAME}
  DESTINATION
    lib/${PROJECT_NAME}
  )

install(
  DIRECTORY
    launch
  DESTINATION
    share/${PROJECT_NAME}
  )

if(UROSAGENT_GENERATE_PROFILE)
  set(_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/python")

  set(_XML_INTERFACE_GEN_BIN "${_OUTPUT_PATH}/bin/Xml_interface_gen.py")
  normalize_path(_XML_INTERFACE_GEN_BIN "${_XML_INTERFACE_GEN_BIN}")

  set(_XML_DEFAULT_READ_BIN "${_OUTPUT_PATH}/bin/Xml_read_default_profiles.py")
  normalize_path(_XML_DEFAULT_READ_BIN "${_XML_DEFAULT_READ_BIN}")

  set(_PYTHON_PKG_TOOL utils)

  set(_RESOURCE_DIR "${_OUTPUT_PATH}/resource")
  normalize_path(_RESOURCE_DIR "${_RESOURCE_DIR}")

  set(_DEFAULT_FASTRTPS_PROFILES_PATH "${_OUTPUT_PATH}/gen/DEFAULT_FASTRTPS_PROFILES.xml")
  normalize_path(_DEFAULT_FASTRTPS_PROFILES_PATH "${_DEFAULT_FASTRTPS_PROFILES_PATH}")

  get_filename_component(_COLCON_CALL_DIR "${PROJECT_BINARY_DIR}" DIRECTORY)
  get_filename_component(_COLCON_CALL_DIR "${_COLCON_CALL_DIR}" DIRECTORY)

  set(_PYTHON_SCRIPT_HEAD
"import os
import sys
sys.path.append('${_OUTPUT_PATH}')
from ${_PYTHON_PKG_TOOL} import *
"
    )

  file(
    COPY
      "bin" "${_PYTHON_PKG_TOOL}" "resource"
    DESTINATION
      "${CMAKE_CURRENT_BINARY_DIR}/python"
    )

  execute_process(
    COMMAND
      "${PYTHON_EXECUTABLE}"
      "-c"
      "${_PYTHON_SCRIPT_HEAD}for package in GetInterfacePackages(GetPackageList('${_COLCON_CALL_DIR}')): print ('%s,%s' % (package, GetPackageName(package)))"
    OUTPUT_VARIABLE
      _packages
    RESULT_VARIABLE
      _result
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

  if(NOT _result EQUAL 0)
    message(FATAL_ERROR "Error finding XML packages")
  endif()

  string(REPLACE "\n" ";" _packages ${_packages})

  set(ALL_MSG_FILES "")
  foreach(package ${_packages})

    string(REPLACE "," ";" package ${package})
    list(GET package 0 package_file)
    list(GET package 1 package)

    execute_process(
      COMMAND
        "${PYTHON_EXECUTABLE}"
        "-c"
        "${_PYTHON_SCRIPT_HEAD}for msg in GetInterfacePackageMsgs('${package_file}'): print ('%s' % msg)"
      OUTPUT_VARIABLE
        ${package}_MSG_FILES
      RESULT_VARIABLE
        _result
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )

    if(NOT _result EQUAL 0)
      message(FATAL_ERROR "Error finding .msg files")
    endif()

    if(${package}_MSG_FILES STREQUAL "")
      continue()
    endif()

    string(REPLACE "\n" ";" ${package}_MSG_FILES ${${package}_MSG_FILES})

  endforeach()

  execute_process(
    COMMAND
      "${PYTHON_EXECUTABLE}" "${_XML_DEFAULT_READ_BIN}" "--default-xml-path" "${_RESOURCE_DIR}"
    OUTPUT_VARIABLE
      _XmlDoc
    RESULT_VARIABLE
      _result
    )

  if(NOT _result EQUAL 0)
    message(FATAL_ERROR "Error in typesupport generation")
  endif()

  foreach(package ${_packages})
    string(REPLACE "," ";" package ${package})
    list(GET package 0 package_file)
    list(GET package 1 package)

    if(${package}_MSG_FILES STREQUAL "")
      continue()
    endif()

    set(generator_arguments_file "${_OUTPUT_PATH}/XML_ArgFiles/${package}_Args.json")
    rosidl_write_generator_arguments(
      "${generator_arguments_file}"
      PACKAGE_NAME "${package}"
      ROS_INTERFACE_FILES "${${package}_MSG_FILES}"
      ROS_INTERFACE_DEPENDENCIES "NULL"
      OUTPUT_DIR "NULL"
      TEMPLATE_DIR "NULL"
      TARGET_DEPENDENCIES "NULL"
      ADDITIONAL_FILES ""
    )

     execute_process(
      COMMAND
        "${PYTHON_EXECUTABLE}" "${_XML_INTERFACE_GEN_BIN}" --generator-arguments-file "${generator_arguments_file}"
      OUTPUT_VARIABLE
        _XmlGen
      RESULT_VARIABLE
        _result
      )

    if(NOT _result EQUAL 0)
      message(FATAL_ERROR "Error in typesupport generation")
    endif()

    set(_XmlDoc "${_XmlDoc}${_XmlGen}")

  endforeach()

  set (_XmlDoc "<profiles>\n${_XmlDoc}</profiles>\n")

  file(WRITE "${_DEFAULT_FASTRTPS_PROFILES_PATH}" "${_XmlDoc}")
endif()
