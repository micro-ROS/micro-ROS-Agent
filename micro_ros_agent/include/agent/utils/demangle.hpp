// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef UROS_AGENT_UTILS_DEMANGLE_HPP_
#define UROS_AGENT_UTILS_DEMANGLE_HPP_

#include <string>

namespace uros {
namespace agent {
namespace utils {

class Demangle
{
private:
    /**
     * @brief   Default constructor. Creating instances of this class is not allowed.
     */
    Demangle() = default;

    /**
     * @brief   Default destructor.
     */
    ~Demangle() = default;

    /**
     * @brief   Demangle service name for a given topic, prefix and service suffix,
     *          if the topic is part of a service; otherwise, return blank.
     * @param   prefix The ROS service prefix.
     * @param   topic_name Topic to be demangled.
     * @param   suffix The ROS service suffix.
     * @returns The demangled service name.
     */
    static std::string _demangle_service_from_topic(
            const std::string& prefix,
            const std::string& topic_name,
            const std::string& suffix);

public:
    /**
     * @brief   Demangle if passed topic is a ROS topic; otherwise, keep it intact.
     * @param   topic_name Topic to be demangled.
     * @returns The demangled topic.
     */
    static std::string demangle_if_ros_topic(
            const std::string& topic_name);

    /**
     * @brief   Demangle if passed type is a ROS type; otherwise, keep it intact.
     * @param   dds_type_string Type to be demangled.
     * @returns The demangled type.
     */
    static std::string demangle_if_ros_type(
            const std::string& dds_type_string);

    /**
     * @brief   Demangle topic name for a given topic if it is part of one;
     *          otherwise, return empty.
     * @param   topic_name Topic to be demangled.
     * @returns The demangled topic name.
     */
    static std::string demangle_ros_topic_from_topic(
            const std::string& topic_name);

    /**
     * @brief   Demangle the service name for a given topic if it is part of a service;
     *          otherwise, return empty.
     * @param   topic_name Topic to be demangled.
     * @returns The demangled service name.
     */

    static std::string demangle_service_from_topic(
            const std::string& topic_name);

    /**
     * @brief   Demangle the service name for a given topic if it is part
     *          of a service request; otherwise, return empty.
     * @param   topic_name Topic to be demangled.
     * @returns The demangled service request name.
     */
    static std::string demangle_service_request_from_topic(
            const std::string& topic_name);

    /**
     * @brief   Demangle the service name for a given topic if it is part
     *          of a service reply; otherwise, return empty.
     * @param   topic_name Topic to be demangled.
     * @returns The demangled service reply name.
     */
    static std::string demangle_service_reply_from_topic(
            const std::string& topic_name);

    /**
     * @brief   Demangle the service type name if it is a ROS srv type; otherwise, return empty.
     * @param   dds_type_name Type to be demangled.
     * @returns The demangled service type.
     */
    static std::string demangle_service_type_only(
            const std::string& dds_type_name);

    /**
     * @brief   Generic demangle function, used when ROS names are not mangled.
     * @param   name Generic name to be demangled.
     * @returns The demangled name.
     */
    static std::string identity_demangle(
            const std::string& name);
};

}  // namespace utils
}  // namespace agent
}  // namespace uros

#endif  // UROS_AGENT_UTILS_DEMANGLE_HPP_