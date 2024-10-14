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

#include "fastdds/dds/publisher/Publisher.hpp"
#include "fastdds/dds/topic/TopicDataType.hpp"
#include "fastdds/rtps/attributes/RTPSParticipantAttributes.hpp"
#include "fastdds/rtps/participant/RTPSParticipant.hpp"
#include "fastdds/rtps/participant/RTPSParticipantListener.hpp"
#include "fastdds/rtps/RTPSDomain.hpp"
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/PublisherListener.hpp>
#include <fastdds/rtps/common/MatchingInfo.hpp>

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
            const void* const data,
            eprosima::fastdds::rtps::SerializedPayload_t& payload,
            eprosima::fastdds::dds::DataRepresentationId_t data_representation) override;

    virtual bool deserialize(
            eprosima::fastdds::rtps::SerializedPayload_t& payload,
            void* data) override;

    virtual uint32_t calculate_serialized_size(
            const void* const data,
            eprosima::fastdds::dds::DataRepresentationId_t data_representation) override;

    virtual void* create_data() override;

    virtual void delete_data(void* data) override;

    virtual bool compute_key(
            eprosima::fastdds::rtps::SerializedPayload_t& payload,
            eprosima::fastdds::rtps::InstanceHandle_t& ihandle,
            bool force_md5 = false) override;

    virtual bool compute_key(
            const void* const data,
            eprosima::fastdds::rtps::InstanceHandle_t& ihandle,
            bool force_md5 = false) override;

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
            const void* const data,
            eprosima::fastdds::rtps::SerializedPayload_t& payload,
            eprosima::fastdds::dds::DataRepresentationId_t data_representation) override;

    virtual bool deserialize(
            eprosima::fastdds::rtps::SerializedPayload_t& payload,
            void* data) override;

    virtual uint32_t calculate_serialized_size(
            const void* const data,
            eprosima::fastdds::dds::DataRepresentationId_t data_representation) override;

    virtual void* create_data() override;

    virtual void delete_data(void* data) override;

    virtual bool compute_key(
            eprosima::fastdds::rtps::SerializedPayload_t& payload,
            eprosima::fastdds::rtps::InstanceHandle_t& ihandle,
            bool force_md5 = false) override;

    virtual bool compute_key(
            const void* const data,
            eprosima::fastdds::rtps::InstanceHandle_t& ihandle,
            bool force_md5 = false) override;

private:

    const message_type_support_callbacks_t* callbacks_;
    const rosidl_message_type_support_t* type_support_;
};

}  // namespace graph_manager
}  // namespace agent
}  // namespace uros

#endif  // UROS_AGENT_GRAPH_PARTICIPANTS_TYPESUPPORT_HPP_