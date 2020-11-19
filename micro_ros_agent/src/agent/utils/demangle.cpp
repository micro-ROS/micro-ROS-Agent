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

#ifndef UROS_AGENT_UTILS_DEMANGLE_CPP_
#define UROS_AGENT_UTILS_DEMANGLE_CPP_

#include <algorithm>
#include <string>
#include <vector>

#include <rcpputils/find_and_replace.hpp>
#include <rcutils/logging_macros.h>
#include <rcutils/types.h>

#include <rmw_fastrtps_shared_cpp/namespace_prefix.hpp>

#include <agent/utils/demangle.hpp>

namespace uros {
namespace agent {
namespace utils {

std::string Demangle::demangle_if_ros_topic(
        const std::string& topic_name)
{
    return _strip_ros_prefix_if_exists(topic_name);
}

std::string Demangle::demangle_if_ros_type(
        const std::string& dds_type_string)
{
    if ('_' != dds_type_string[dds_type_string.size() - 1])
    {
        // not a ROS type
        return dds_type_string;
    }

    const std::string dds_prefix("dds_::");
    size_t dds_prefix_pos = dds_type_string.find(dds_prefix);
    if (std::string::npos == dds_prefix_pos)
    {
        // not a ROS type
        return dds_type_string;
    }

    std::string type_namespace = dds_type_string.substr(0, dds_prefix_pos);
    type_namespace = rcpputils::find_and_replace(type_namespace, "::", "/");
    size_t start = dds_prefix_pos + dds_prefix.size();
    const std::string type_name =
        dds_type_string.substr(start, dds_type_string.length() - start - 1);
    return type_namespace + type_name;
}

std::string Demangle::demangle_ros_topic_from_topic(
        const std::string& topic_name)
{
    return _resolve_prefix(topic_name, ros_topic_prefix);
}

std::string Demangle::_demangle_service_from_topic(
        const std::string& prefix,
        const std::string& topic_name,
        const std::string& suffix)
{
    const std::string service_name = _resolve_prefix(topic_name, prefix);
    if (service_name.empty())
    {
        return std::string();
    }

    size_t suffix_position = service_name.rfind(suffix);
    if (std::string::npos == suffix_position)
    {
        RCUTILS_LOG_WARN_NAMED(
            "rmw_fastrtps_shared_cpp",
            "service topic has prefix but no suffix; report this: '%s'",
            topic_name.c_str());
        return std::string();
    }
    else
    {
        if (0 != (service_name.length() - suffix_position - suffix.length()))
        {
            RCUTILS_LOG_WARN_NAMED(
                "rmw_fastrtps_shared_cpp",
                "service topic has service prefix and a suffix"
                ", but not at the end; report this: '%s'",
                topic_name.c_str());
            return std::string();
        }
    }
    return service_name.substr(0, suffix_position);
}

std::string Demangle::demangle_service_from_topic(
        const std::string& topic_name)
{
    const std::string demangled_topic = demangle_service_reply_from_topic(topic_name);

    if (!demangled_topic.empty())
    {
        return demangled_topic;
    }
    return demangle_service_request_from_topic(topic_name);
}

std::string Demangle::demangle_service_request_from_topic(
        const std::string& topic_name)
{
    return _demangle_service_from_topic(ros_service_requester_prefix, topic_name, "Request");
}

std::string Demangle::demangle_service_reply_from_topic(
        const std::string& topic_name)
{
    return _demangle_service_from_topic(ros_service_response_prefix, topic_name, "Reply");
}

std::string Demangle::demangle_service_type_only(
        const std::string& dds_type_name)
{
    const std::string dds_prefix("dds_::");
    auto suffixes = {std::string("_Response_"), std::string("_Request_")};
    size_t dds_prefix_pos = dds_type_name.find(dds_prefix);

    // Perform checks
    if (std::string::npos == dds_prefix_pos)
    {
        // not a ROS service type
        return std::string();
    }

    size_t suffix_position = std::string::npos;
    for (const auto& suffix : suffixes)
    {
        suffix_position = dds_type_name.rfind(suffix);
        if (std::string::npos != suffix_position)
        {
            if (0 != (dds_type_name.length() - suffix_position - suffix.length()))
            {
                RCUTILS_LOG_WARN_NAMED(
                    "rmw_fastrtps_shared_cpp",
                    "service type contains 'dds_::' and a suffix"
                    ", but not at the end; repor this: '%s'",
                    dds_type_name.c_str());
                continue;
            }
            break;
        }
    }
    if (std::string::npos == suffix_position)
    {
        RCUTILS_LOG_WARN_NAMED(
            "rmw_fastrtps_shared_cpp",
            "service type contains 'dds_::' but"
            " does not have a suffix; report this: '%s'",
            dds_type_name.c_str());
        return std::string();
    }

    // Everything is OK. Reformat it from '[type_namespace::]dds_::<type><suffix>'
    // to '[type_namespace/]<type>'
    std::string type_namespace = dds_type_name.substr(0, dds_prefix_pos);
    type_namespace = rcpputils::find_and_replace(type_namespace, "::", "/");
    size_t start = dds_prefix_pos + dds_prefix.length();
    const std::string type_name = dds_type_name.substr(start, suffix_position - start);
    return type_namespace + type_name;
}

std::string Demangle::identity_demangle(
        const std::string& name)
{
    return name;
}

}  // namespace utils
}  // namespace agent
}  // namespace uros

#endif  // UROS_AGENT_UTILS_DEMANGLE_CPP_