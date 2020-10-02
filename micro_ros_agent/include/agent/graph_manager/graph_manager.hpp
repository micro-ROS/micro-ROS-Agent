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
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>

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
#include "micro_ros_msgs/msg/graph.hpp"
#include "micro_ros_msgs/msg/node.hpp"
#include "micro_ros_msgs/msg/entity.hpp"

#include <uxr/agent/types/XRCETypes.hpp>

#include <agent/graph_manager/graph_typesupport.hpp>
#include <agent/utils/demangle.hpp>

#ifndef _UROS_AGENT_GRAPH_MANAGER_HPP
#define _UROS_AGENT_GRAPH_MANAGER_HPP

namespace uros {
namespace agent {
namespace graph_manager {

class GraphManager
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
    void publish_microros_graph();

    /**
     * @brief   Adds a DDS participant to the graph tree.
     * @param   guid rtps::GUID_t of the participant to be added.
     * @param   participant Pointer to the participant to be added to the graph.
     */
    void add_participant(
            const eprosima::fastrtps::rtps::GUID_t& guid,
            const eprosima::fastdds::dds::DomainParticipant* participant);

    /**
     * @brief   TODO
     */
    void add_participant(
            const eprosima::fastrtps::rtps::GUID_t& guid,
            const std::string& node_name,
            const std::string& enclave);

    /**
     * @brief   Removes a DDS participant from the graph tree.
     * @param   guid rtps::GUID_t of the participant to be removed.
     */
    void remove_participant(
            const eprosima::fastrtps::rtps::GUID_t& guid);

    /**
     * @brief   TODO
     */
    void add_datawriter(
            const eprosima::fastrtps::rtps::GUID_t& datawriter_guid,
            const eprosima::fastdds::dds::DomainParticipant* participant,
            const eprosima::fastdds::dds::DataWriter* datawriter);

    /**
     * @brief   TODO
     */
    void add_datawriter(
            const eprosima::fastrtps::rtps::GUID_t& datawriter_guid,
            const std::string& topic_name,
            const std::string& type_name,
            const eprosima::fastrtps::rtps::GUID_t& participant_guid,
            const eprosima::fastdds::dds::DataWriterQos& writer_qos);

    void add_datawriter(
            const eprosima::fastrtps::rtps::GUID_t& datawriter_guid,
            const std::string& topic_name,
            const std::string& type_name,
            const eprosima::fastrtps::rtps::GUID_t& participant_guid,
            const eprosima::fastdds::dds::WriterQos& writer_qos);

    /**
     * @brief   TODO
     */
    void remove_datawriter(
            const eprosima::fastrtps::rtps::GUID_t& datawriter_guid);

    /**
     * @brief   TODO
     */
    void add_datareader(
            const eprosima::fastrtps::rtps::GUID_t& datareader_guid,
            const eprosima::fastdds::dds::DomainParticipant* participant,
            const eprosima::fastdds::dds::DataReader* datareader);

    /**
     * @brief   TODO
     */
    void add_datareader(
            const eprosima::fastrtps::rtps::GUID_t& datareader_guid,
            const std::string& topic_name,
            const std::string& type_name,
            const eprosima::fastrtps::rtps::GUID_t& participant_guid,
            const eprosima::fastdds::dds::DataReaderQos& reader_qos);

    void add_datareader(
            const eprosima::fastrtps::rtps::GUID_t& datareader_guid,
            const std::string& topic_name,
            const std::string& type_name,
            const eprosima::fastrtps::rtps::GUID_t& participant_guid,
            const eprosima::fastdds::dds::ReaderQos& reader_qos);

    /**
     * @brief   TODO
     */
    void remove_datareader(
            const  eprosima::fastrtps::rtps::GUID_t& datareader_guid);

    /**
     * @brief   Associates a certain DDS entity with a provided participant.
     * @param   guid rtps::GUID_t identifier of the entity.
     * @param   participant Participant to be associated with.
     * @param   entity_kind Kind of the DDS entity.
     */
    void associate_entity(
            const eprosima::fastrtps::rtps::GUID_t& entity_guid,
            const eprosima::fastdds::dds::DomainParticipant* participant,
            const dds::xrce::ObjectKind& entity_kind);

private:

    /**
     * @brief   TODO(jamoralp) docs
     */
    class ParticipantListener : public eprosima::fastdds::dds::DomainParticipantListener
    {
    public:

        ParticipantListener(
                GraphManager* graph_manager);
    private:

        template <typename Info>
        void process_discovery_info(
                const Info& proxyData);

        void on_participant_discovery(
                eprosima::fastdds::dds::DomainParticipant* participant,
                eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info) override;

        void on_subscriber_discovery(
                eprosima::fastdds::dds::DomainParticipant* participant,
                eprosima::fastrtps::rtps::ReaderDiscoveryInfo&& info) override;

        void on_publisher_discovery(
                eprosima::fastdds::dds::DomainParticipant* participant,
                eprosima::fastrtps::rtps::WriterDiscoveryInfo&& info) override;

        GraphManager* graphManager_from_;
    };

    template <typename FastDDSQos>
    const rmw_qos_profile_t fastdds_qos_to_rmw_qos(
            const FastDDSQos& fastdds_qos);

    bool graph_changed_;
    const char * enclave_;
    std::thread microros_graph_publisher_;
    std::mutex mtx_;
    std::condition_variable cv_;

    rmw_dds_common::GraphCache graphCache_;
    std::unique_ptr<ParticipantListener> participant_listener_;

    std::unique_ptr<eprosima::fastdds::dds::TypeSupport> participant_info_typesupport_;
    std::unique_ptr<eprosima::fastdds::dds::TypeSupport> microros_graph_info_typesupport_;
    std::unique_ptr<eprosima::fastdds::dds::DomainParticipant> participant_;
    std::unique_ptr<eprosima::fastdds::dds::Publisher> publisher_;
    std::unique_ptr<eprosima::fastdds::dds::Topic> ros_discovery_topic_;
    std::unique_ptr<eprosima::fastdds::dds::Topic> ros_to_microros_graph_topic_;
    std::unique_ptr<eprosima::fastdds::dds::DataWriter> ros_discovery_datawriter_;
    std::unique_ptr<eprosima::fastdds::dds::DataWriter> ros_to_microros_graph_datawriter_;

    /// TODO(jamoralp)
    /**
     * We need to come up with a way to identify which participant corresponds with its nodes,
     * to associates writers/readers with nodes.
     * This should be implemented by means of a subscriber to the "ros_discovery_info" topic.
     * This subscriber, which must have a DataWriterListener entity associated to it, should implement
     * the callback function for the DataWriterListener, filling a micro_ros_msgs::msg::Graph message instance
     * and sending it via a different datawriter (also to be implemented).
     *
     * We could keep the current DomainParticipantListener, or limit it to a SubscriberListener/PublisherListener
     * to gather information about the existing writers and readers and keep them in our local graphCache. As "ros_discovery_info"
     * topic gives us the writer/readers GID associated to each node, the could be looked up from our local graphCache entity,
     * to fill the micro_ros_msgs::msg::Node entities array and send it back to micro-ROS.
     *
     */
};

}  // namespace graph_manager
}  // namespace agent
}  // namespace uros

#endif  // _UROS_AGENT_GRAPH_MANAGER_HPP