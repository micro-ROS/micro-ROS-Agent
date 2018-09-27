# Copyright 2016-2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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


# list msg files 
set(_ros_idl_files "")
foreach(_idl_file ${rosidl_generate_interfaces_IDL_FILES})
  get_filename_component(_extension "${_idl_file}" EXT)
  # Skip .srv files
  if(_extension STREQUAL ".msg")
    list(APPEND _ros_idl_files "${_idl_file}")
  endif()
endforeach()


# Set output dir
set(_output_path "${CMAKE_CURRENT_BINARY_DIR}/../micro_ros_agent/xml_gen")


# check if all templates exits
set(target_dependencies
  "${micro_ros_agent_BIN}"
  "${micro_ros_agent_GENERATOR_FILES}"
  ${rosidl_generate_interfaces_IDL_FILES}
  ${_dependency_files})
foreach(dep ${target_dependencies})
  if(NOT EXISTS "${dep}")
    message(FATAL_ERROR "Target dependency '${dep}' does not exist")
  endif()
endforeach()


# generate script argument file 
set(generator_arguments_file "${CMAKE_CURRENT_BINARY_DIR}/micro_ros_agent__arguments.json")
rosidl_write_generator_arguments(
  "${generator_arguments_file}"
  PACKAGE_NAME "${PROJECT_NAME}"
  ROS_INTERFACE_FILES "${rosidl_generate_interfaces_IDL_FILES}"
  ROS_INTERFACE_DEPENDENCIES "${_dependencies}"
  OUTPUT_DIR "${_output_path}"
  TEMPLATE_DIR "${micro_ros_agent_DEFAULT_PROFILES_DIR}"
  TARGET_DEPENDENCIES ${target_dependencies}
  ADDITIONAL_FILES ${_dds_idl_files}
)


# Execute python script
execute_process(
                COMMAND ${PYTHON_EXECUTABLE} ${micro_ros_agent_BIN}
                --generator-arguments-file "${generator_arguments_file}"
                )


#Install
install(
  DIRECTORY "${_output_path}/"
  DESTINATION "../micrortps_agent/bin"
)