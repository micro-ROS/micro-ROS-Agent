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
#include "rmw/names_and_types.h"
#include "rmw/impl/cpp/key_value.hpp"
#include "rmw_dds_common/graph_cache.hpp"
#include "rmw_fastrtps_shared_cpp/create_rmw_gid.hpp"
#include "rmw_fastrtps_shared_cpp/qos.hpp"

#include "rcutils/types.h"
#include "rcutils/types/string_array.h"

#include "rosidl_typesupport_cpp/message_type_support.hpp"
#include "rosidl_typesupport_fastrtps_cpp/message_type_support.h"

#include "rmw_dds_common/msg/participant_entities_info.hpp"
#include "micro_ros_agent_msgs/msg/graph.hpp"
#include "micro_ros_agent_msgs/msg/node.hpp"
#include "micro_ros_agent_msgs/msg/entity.hpp"

#include <uxr/agent/types/XRCETypes.hpp>

#include <agent/graph_manager/graph_participants_typesupport.hpp>
#include <agent/utils/demangle.hpp>

#ifndef _UROS_AGENT_GRAPH_MANAGER_HPP
#define _UROS_AGENT_GRAPH_MANAGER_HPP

namespace uros {
namespace agent {
namespace graph_manager {

/**
 * TODO(jamoralp): class documentation
 */
class GraphManager : public eprosima::fastrtps::ParticipantListener
{
public:
    /**
     * @brief   Default constructor.
     */
    GraphManager();

    /**
     * @brief   Default destructor.
     */
    ~GraphManager() = default;

    /**
     * @brief   Implementation of the notification logic that updates the micro-ROS graph.
     */
    inline void publish_microros_graph();

    /**
     * @brief   Adds a DDS participant to the graph tree.
     * @param   guid rtps::GUID_t of the participant to be added.
     * @param   participant Pointer to the participant to be added to the graph.
     */
    void add_participant(
            const eprosima::fastrtps::rtps::GUID_t& guid,
            const eprosima::fastdds::dds::DomainParticipant* participant);

    /**
     * @brief   Associates a certain DDS entity with a provided participant.
     * @param   guid rtps::GUID_t identifier of the entity.
     * @param   participant Participant to be associated with.
     * @param   entity_kind Kind of the DDS entity.
     */
    void associate_entity(
            const eprosima::fastrtps::rtps::GUID_t& guid,
            const eprosima::fastdds::dds::DomainParticipant* participant,
            const dds::xrce::ObjectKind& entity_kind);

private:

    /**
     * @brief TODO
     */
    template <typename Info>
    void process_discovery_info(
            const Info& proxyData);

    /**
     * @brief   Override eprosima::fastrtps::ParticipantListener behaviour over certain events.
     */
    void onParticipantDiscovery(
            eprosima::fastrtps::Participant* /*participant*/,
            eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info) override;

    void onSubscriberDiscovery(
            eprosima::fastrtps::Participant* /*participant*/,
            eprosima::fastrtps::rtps::ReaderDiscoveryInfo&& info) override;

    void onPublisherDiscovery(
            eprosima::fastrtps::Participant* /*participant*/,
            eprosima::fastrtps::rtps::WriterDiscoveryInfo&& info) override;

    bool graph_changed_;
    const char * enclave_;
    std::thread microros_graph_publisher_;
    std::mutex mtx_;
    std::condition_variable cv_;

    rmw_dds_common::GraphCache graphCache_;
    const message_type_support_callbacks_t * callbacks_;

    // TODO (jamoralp): migration to FastDDS API
    std::unique_ptr<ParticipantEntitiesInfoTypeSupport> participant_info_typesupport_;
    std::shared_ptr<eprosima::fastrtps::Participant> participant_;
    std::shared_ptr<eprosima::fastrtps::Publisher> publisher_;
};

}  // namespace graph_manager
}  // namespace agent
}  // namespace uros

#endif  // _UROS_AGENT_GRAPH_MANAGER_HPP