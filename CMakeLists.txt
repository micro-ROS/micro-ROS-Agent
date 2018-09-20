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


# Find packages depencences
find_package(ament_cmake REQUIRED)

find_package(fastcdr REQUIRED CONFIG)
find_package(fastrtps REQUIRED CONFIG)
find_package(micrortps_agent REQUIRED CONFIG)

find_package(ament_cmake_python REQUIRED)


# Export dependencies to downstream packages
ament_export_dependencies(fastcdr)
ament_export_dependencies(fastrtps)
ament_export_dependencies(micrortps_agent)

ament_export_dependencies(rosidl_cmake)
ament_export_dependencies(rosidl_generator_c)
ament_export_dependencies(rosidl_generator_dds_idl)


#ament_export_include_directories(include)

# Install python files
ament_python_install_package(${PROJECT_NAME})


ament_index_register_resource("rosidl_typesupport_c")


# Install the package.xml file, and generate code for
# ``find_package`` so that other packages can get information about
# this package.
#
# :param CONFIG_EXTRAS: a list of CMake files containing extra stuff
#   that should be accessible to users of this package after
#   ``find_package``\ -ing it.
ament_package(
  CONFIG_EXTRAS "micro_ros_agent-extras.cmake.in"
)


install(
  PROGRAMS bin/micro_ros_agent
  DESTINATION lib/micro_ros_agent
)

install(
  DIRECTORY cmake resource
  DESTINATION share/${PROJECT_NAME}
)