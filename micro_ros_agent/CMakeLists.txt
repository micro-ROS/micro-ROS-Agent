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


# Set CMake version 
cmake_minimum_required(VERSION 3.5)

# Set proyect name
project(micro_ros_agent)


set(CMAKE_C_CLANG_TIDY clang-tidy -checks=*)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()


# Find packages depencences
find_package(rosidl_cmake REQUIRED)
find_package(ament_cmake REQUIRED)
find_package(fastcdr REQUIRED CONFIG)
find_package(fastrtps REQUIRED CONFIG)
find_package(microxrcedds_agent REQUIRED CONFIG)
find_package(ament_cmake_python REQUIRED)


# Export dependencies to downstream packages
ament_export_dependencies(fastcdr)
ament_export_dependencies(fastrtps)
ament_export_dependencies(microxrcedds_agent)


# Set variables
set(_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/python")

set(_XML_INTERFACE_GEN_BIN "${_OUTPUT_PATH}/bin/Xml_interface_gen.py")
normalize_path(_XML_INTERFACE_GEN_BIN "${_XML_INTERFACE_GEN_BIN}")

set(_XML_DEFAULT_READ_BIN "${_OUTPUT_PATH}/bin/Xml_read_default_profiles.py")
normalize_path(_XML_DEFAULT_READ_BIN "${_XML_DEFAULT_READ_BIN}")

set(_PYTHON_PKG_TOOL ${PROJECT_NAME})

set(_RESOURCE_DIR "${_OUTPUT_PATH}/resource")
normalize_path(_RESOURCE_DIR "${_RESOURCE_DIR}")


set(_DEFAULT_FASTRTPS_PROFILES_PATH "${_OUTPUT_PATH}/gen/DEFAULT_FASTRTPS_PROFILES.xml")
normalize_path(_DEFAULT_FASTRTPS_PROFILES_PATH "${_DEFAULT_FASTRTPS_PROFILES_PATH}")


# Get colcon call dir
get_filename_component(_COLCON_CALL_DIR "${PROJECT_BINARY_DIR}" DIRECTORY)
get_filename_component(_COLCON_CALL_DIR "${_COLCON_CALL_DIR}" DIRECTORY)


# Generate python header
set(
_PYTHON_SCRIPT_HEAD
"
import os\n
import sys\n
sys.path.append('${_OUTPUT_PATH}')\n
from ${_PYTHON_PKG_TOOL} import *\n
"
)


# Copy pyton files  
file(COPY "bin" "${_PYTHON_PKG_TOOL}" "resource" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/python")


# Extract all packages and manifiest paths
execute_process(
  COMMAND
  "${PYTHON_EXECUTABLE}"
  "-c"
  "${_PYTHON_SCRIPT_HEAD}for package in GetInterfacePackages(GetPackageList('${_COLCON_CALL_DIR}')): print ('%s,%s' % (package, GetPackageName(package)))"
  OUTPUT_VARIABLE _packages
  RESULT_VARIABLE _result
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(NOT _result EQUAL 0)
  message(FATAL_ERROR "Error finding xml packages")
endif()


# Convert output to list
string(REPLACE "\n" ";" _packages ${_packages})


# Extract all msg from each package (stored in ${package}_MSG_FILES)
set(ALL_MSG_FILES "")
foreach(package ${_packages})
  
  # Extract info
  string(REPLACE "," ";" package ${package})
  list(GET package 0 package_file)
  list(GET package 1 package)


  # Get all msg files
  execute_process(
  COMMAND
  "${PYTHON_EXECUTABLE}"
  "-c"
  "${_PYTHON_SCRIPT_HEAD}for msg in GetInterfacePackageMsgs('${package_file}'): print ('%s' % msg)"
  OUTPUT_VARIABLE ${package}_MSG_FILES
  RESULT_VARIABLE _result
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if(NOT _result EQUAL 0)
    message(FATAL_ERROR "Error finding .msg files")
  endif()

  # Skip if there are no msgs to create
  if(${package}_MSG_FILES STREQUAL "")
    continue()
  endif()

  # Generate list
  string(REPLACE "\n" ";" ${package}_MSG_FILES ${${package}_MSG_FILES})

endforeach()


# Append defaul xml profiles
execute_process(
  COMMAND "${PYTHON_EXECUTABLE}" "${_XML_DEFAULT_READ_BIN}" "--default-xml-path" "${_RESOURCE_DIR}"
  OUTPUT_VARIABLE _XmlDoc
  RESULT_VARIABLE _result
  )
if(NOT _result EQUAL 0)
  message(FATAL_ERROR "Error in typesuppor generation") 
endif()


# Create one xml for each message
foreach(package ${_packages})
  string(REPLACE "," ";" package ${package})
  list(GET package 0 package_file)
  list(GET package 1 package)


  # Skip this generation if there are no msgs to process
  if(${package}_MSG_FILES STREQUAL "")
    continue()
  endif()



  # generate script argument file 
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


   # execute python script
   execute_process(
    COMMAND "${PYTHON_EXECUTABLE}" "${_XML_INTERFACE_GEN_BIN}" --generator-arguments-file "${generator_arguments_file}"
    OUTPUT_VARIABLE _XmlGen
    RESULT_VARIABLE _result
    )
  if(NOT _result EQUAL 0)
    message(FATAL_ERROR "Error in typesuppor generation") 
  endif()


  # Strore xml
  set(_XmlDoc "${_XmlDoc}${_XmlGen}")
  
endforeach()


# Close profile 
set (_XmlDoc "<profiles>\n${_XmlDoc}</profiles>\n")


# Save 
file(WRITE "${_DEFAULT_FASTRTPS_PROFILES_PATH}" "${_XmlDoc}")


# Only compile uROS agent if rclcpp is found
find_package(rclcpp QUIET)
if (rclcpp_FOUND)
  find_package(microxrcedds_agent REQUIRED)

  add_executable(${PROJECT_NAME} src/main.cpp)
  ament_target_dependencies(
    ${PROJECT_NAME} 
    rclcpp
    microxrcedds_agent
  )

  # TODO(Javier) Temporal until thread error is solver
  target_link_libraries(
    ${PROJECT_NAME}
    microxrcedds_agent
  )
  
else()
  message("uROS agent node will be not build")
endif()




# Install the package.xml file, and generate code for ``find_package`` so that other packages can get information about this package.
ament_package()


# Only install if uROS_Agent is comiled
if (rclcpp_FOUND)
  install(TARGETS
    ${PROJECT_NAME}
    DESTINATION lib/${PROJECT_NAME}
  )
endif()


#Install
install(
  FILES "${_DEFAULT_FASTRTPS_PROFILES_PATH}"
  DESTINATION lib/${PROJECT_NAME}
)


# Install package
#install(
#  DIRECTORY cmake resource
#  DESTINATION share/${PROJECT_NAME}
#)
