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

#ifndef UROS_AGENT_GRAPH_PARTICIPANTS_TYPESUPPORT_HPP_
#define UROS_AGENT_GRAPH_PARTICIPANTS_TYPESUPPORT_HPP_

#include "fastrtps/Domain.h"
#include <fastrtps/TopicDataType.h>
#include "fastrtps/attributes/ParticipantAttributes.h"
#include "fastrtps/participant/Participant.h"
#include "fastrtps/participant/ParticipantListener.h"
#include "fastrtps/attributes/PublisherAttributes.h"
#include "fastrtps/publisher/Publisher.h"
#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/rtps/common/MatchingInfo.h>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>

#include "rmw/types.h"
#include "rmw/impl/cpp/key_value.hpp"
#include "rmw_dds_common/graph_cache.hpp"
#include "rmw_fastrtps_shared_cpp/create_rmw_gid.hpp"
#include "rmw_fastrtps_shared_cpp/qos.hpp"

#include "rcutils/types.h"

#include "rosidl_typesupport_cpp/message_type_support.hpp"
#include "rosidl_typesupport_fastrtps_cpp/message_type_support.h"

#include "rmw_dds_common/msg/participant_entities_info.hpp"

#include "micro_ros_msgs/msg/graph.hpp"

namespace uros {
namespace agent {
namespace graph_manager {

/**
 * @brief   Implementation of virtual class eprosima::fastdds::dds::TopicDataType.
 *          Is used to gather and send information about the entities present within a DDS domain,
 *          in the Agent's context.
 */
class ParticipantEntitiesInfoTypeSupport : public eprosima::fastdds::dds::TopicDataType
{
public:

    ParticipantEntitiesInfoTypeSupport();

    virtual bool serialize(
            void* data,
            eprosima::fastrtps::rtps::SerializedPayload_t* payload) override;

    virtual bool deserialize(
            eprosima::fastrtps::rtps::SerializedPayload_t* payload,
            void* data) override;

    virtual std::function<uint32_t()> getSerializedSizeProvider(
            void* data) override;

    virtual void* createData() override;

    virtual void deleteData(void* data) override;

    virtual bool getKey(
            void* data,
            eprosima::fastrtps::rtps::InstanceHandle_t* handle,
            bool force_md5) override;

private:

    const message_type_support_callbacks_t* callbacks_;
    const rosidl_message_type_support_t* type_support_;
};

/**
 * @brief   Implementation of virtual class eprosima::fastdds::dds::TopicDataType.
 *          Is used to send graph information to Micro-ROS.
 */
class MicrorosGraphInfoTypeSupport : public eprosima::fastdds::dds::TopicDataType
{
public:

    MicrorosGraphInfoTypeSupport();

    virtual bool serialize(
            void* data,
            eprosima::fastrtps::rtps::SerializedPayload_t* payload) override;

    virtual bool deserialize(
            eprosima::fastrtps::rtps::SerializedPayload_t* payload,
            void* data) override;

    virtual std::function<uint32_t()> getSerializedSizeProvider(
            void* data) override;

    virtual void* createData() override;

    virtual void deleteData(void* data) override;

    virtual bool getKey(
            void* data,
            eprosima::fastrtps::rtps::InstanceHandle_t* handle,
            bool force_md5) override;

private:

    const message_type_support_callbacks_t* callbacks_;
    const rosidl_message_type_support_t* type_support_;
};

}  // namespace graph_manager
}  // namespace agent
}  // namespace uros

#endif  // UROS_AGENT_GRAPH_PARTICIPANTS_TYPESUPPORT_HPP_