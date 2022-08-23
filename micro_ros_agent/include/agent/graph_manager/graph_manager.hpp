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

#ifndef _UROS_AGENT_GRAPH_MANAGER_HPP
#define _UROS_AGENT_GRAPH_MANAGER_HPP

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
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>

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

#include <condition_variable>
#include <string>
#include <memory>
#include <map>

namespace uros {
namespace agent {
namespace graph_manager {

/**
 * @brief   Class that keeps track of the existing entities in the ROS 2 world,
 *          both coming from micro-ROS or from external ROS 2 applications.
 */
class GraphManager
{
public:
    /**
     * @brief   Default constructor.
     */
    GraphManager(eprosima::fastdds::dds::DomainId_t domain_id);

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
     * @param   participant eprosima::fastdds::dds::DomainParticipant to be added.
     * @param   from_microros if this participant has been added from micro-ROS.
     * @param   enclave ROS 2 enclave.
     */
    void add_participant(
            const eprosima::fastdds::dds::DomainParticipant* participant,
            bool from_microros = true,
            const std::string& enclave = "/");

    /**
     * @brief   Removes a DDS participant from the graph tree.
     * @param   participant eprosima::fastdds::dds::DomainParticipant to be removed.
     */
    void remove_participant(
        const eprosima::fastdds::dds::DomainParticipant* participant,
        bool from_microros = true);

    /**
     * @brief   Getter for the graph cache.
     * @return  Reference to inner graph cache
     */
    rmw_dds_common::GraphCache& get_graph_cache() { return graphCache_; }

    /**
     * @brief   Adds a DDS datawriter to the graph tree.
     * @param   datawriter_guid rtps::GUID_t of the datawriter to be added.
     * @param   participant Pointer to the participant which owns this datawriter.
     * @param   datawriter Pointer to the datawriter to be added.
     */
    void add_datawriter(
            const eprosima::fastrtps::rtps::GUID_t& datawriter_guid,
            const eprosima::fastdds::dds::DomainParticipant* participant,
            const eprosima::fastdds::dds::DataWriter* datawriter);

    /**
     * @brief   Adds a DDS datawriter to the graph tree.
     * @param   datawriter_guid rtps::GUID_t of the datawriter to be added.
     * @param   topic_name Name of the topic to which the datawriter sends information to.
     * @param   type_name Type name of the sent topic.
     * @param   participant_guid rtps::GUID_t of the participant which owns this datawriter.
     * @param   writer_qos QOS of the datawriter to be included into the graph tree.
     */
    void add_datawriter(
            const eprosima::fastrtps::rtps::GUID_t& datawriter_guid,
            const std::string& topic_name,
            const std::string& type_name,
            const eprosima::fastrtps::rtps::GUID_t& participant_guid,
            const eprosima::fastdds::dds::DataWriterQos& writer_qos);

    /**
     * @brief   Adds a DDS datawriter to the graph tree.
     * @param   datawriter_guid rtps::GUID_t of the datawriter to be added.
     * @param   topic_name Name of the topic to which the datawriter sends information to.
     * @param   type_name Type name of the sent topic.
     * @param   participant_guid rtps::GUID_t of the participant which owns this datawriter.
     * @param   writer_qos QOS of the datawriter to be included into the graph tree.
     */
    void add_datawriter(
            const eprosima::fastrtps::rtps::GUID_t& datawriter_guid,
            const std::string& topic_name,
            const std::string& type_name,
            const eprosima::fastrtps::rtps::GUID_t& participant_guid,
            const eprosima::fastdds::dds::WriterQos& writer_qos);

    /**
     * @brief   Removes a DDS datawriter from the graph tree.
     * @param   datawriter_guid rtps::GUID_t of the datawriter to be removed.
     */
    void remove_datawriter(
            const eprosima::fastrtps::rtps::GUID_t& datawriter_guid);

    /**
     * @brief   Adds a DDS datareader to the graph tree.
     * @param   datareader_guid rtps::GUID_t of the datareader to be added.
     * @param   participant Pointer to the participant which owns this datareader.
     * @param   datareader Pointer to the datareader to be added.
     */
    void add_datareader(
            const eprosima::fastrtps::rtps::GUID_t& datareader_guid,
            const eprosima::fastdds::dds::DomainParticipant* participant,
            const eprosima::fastdds::dds::DataReader* datareader);

    /**
     * @brief   Adds a DDS datareader to the graph tree.
     * @param   datareader_guid rtps::GUID_t of the datareader to be added.
     * @param   topic_name Name of the topic to which the datareader sends information to.
     * @param   type_name Type name of the sent topic.
     * @param   participant_guid rtps::GUID_t of the participant which owns this datareader.
     * @param   writer_qos QOS of the datareader to be included into the graph tree.
     */
    void add_datareader(
            const eprosima::fastrtps::rtps::GUID_t& datareader_guid,
            const std::string& topic_name,
            const std::string& type_name,
            const eprosima::fastrtps::rtps::GUID_t& participant_guid,
            const eprosima::fastdds::dds::DataReaderQos& reader_qos);

    /**
     * @brief   Adds a DDS datareader to the graph tree.
     * @param   datareader_guid rtps::GUID_t of the datareader to be added.
     * @param   topic_name Name of the topic to which the datareader sends information to.
     * @param   type_name Type name of the sent topic.
     * @param   participant_guid rtps::GUID_t of the participant which owns this datareader.
     * @param   writer_qos QOS of the datareader to be included into the graph tree.
     */
    void add_datareader(
            const eprosima::fastrtps::rtps::GUID_t& datareader_guid,
            const std::string& topic_name,
            const std::string& type_name,
            const eprosima::fastrtps::rtps::GUID_t& participant_guid,
            const eprosima::fastdds::dds::ReaderQos& reader_qos);

    /**
     * @brief   Removes a DDS datareader from the graph tree.
     * @param   datareader_guid rtps::GUID_t of the datareader to be removed.
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
     * @brief   Implementation of FastDDS' DomainParticipantListener abstract class.
     */
    class ParticipantListener : public eprosima::fastdds::dds::DomainParticipantListener
    {
    public:

        /**
         * @brief   Constructor.
         * @param   graph_manager Pointer to the GraphManager object which owns this ParticipantListener.
         */
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
                eprosima::fastdds::dds::DomainParticipant* /*participant*/,
                eprosima::fastrtps::rtps::ReaderDiscoveryInfo&& info) override;

        void on_publisher_discovery(
                eprosima::fastdds::dds::DomainParticipant* /*participant*/,
                eprosima::fastrtps::rtps::WriterDiscoveryInfo&& info) override;

        GraphManager* graphManager_from_;
    };

    /**
     * @brief   Implementation of FastDDS' DomainReaderListener abstract class.
     */
    class DatareaderListener : public eprosima::fastdds::dds::DataReaderListener
    {
    public:

        /**
         * @brief   Constructor.
         * @param   graph_manager Pointer to the GraphManager object which owns this DataReaderListener.
         */
        DatareaderListener(
                GraphManager* graph_manager);

    private:

        void on_data_available(
                eprosima::fastdds::dds::DataReader* /*sub*/) override;

        GraphManager* graphManager_from_;
    };

    /**
     * @brief   Convert FastDDS QOS object instance to RMW instance.
     * @param   fastdds_qos QOS instance to be converted.
     * @returns RMW object representation of the given FastDDS QOS.
     */
    template <typename FastDDSQos>
    const rmw_qos_profile_t fastdds_qos_to_rmw_qos(
            const FastDDSQos& fastdds_qos);

    /**
     * @brief   Update micro-ROS graph information upon new data
     *          received in the 'ros_discovery_info' topic.
     */
    void update_node_entities_info();

    /**
     * @brief   Retrieves node name and namespace from
     *          participant.
     * @param   participant_name DDS participant name.
     * @param   node_name ROS2 Node name.
     * @param   namespace ROS2 Node namespace.
     */
    void get_name_and_namespace(
            std::string participant_name,
            std::string& node_name,
            std::string& node_namespace);

    eprosima::fastdds::dds::DomainId_t domain_id_;
    bool graph_changed_;
    bool display_on_change_;
    std::thread microros_graph_publisher_;
    std::mutex mtx_;
    std::condition_variable cv_;

    eprosima::fastdds::dds::DataWriterQos datawriter_qos_;

    rmw_dds_common::GraphCache graphCache_;
    std::unique_ptr<ParticipantListener> participant_listener_;
    std::unique_ptr<DatareaderListener> datareader_listener_;

    std::unique_ptr<eprosima::fastdds::dds::TypeSupport> participant_info_typesupport_;
    std::unique_ptr<eprosima::fastdds::dds::TypeSupport> microros_graph_info_typesupport_;
    std::unique_ptr<eprosima::fastdds::dds::DomainParticipant> participant_;
    std::unique_ptr<eprosima::fastdds::dds::Publisher> publisher_;
    std::unique_ptr<eprosima::fastdds::dds::Subscriber> subscriber_;
    std::unique_ptr<eprosima::fastdds::dds::Topic> ros_discovery_topic_;
    std::unique_ptr<eprosima::fastdds::dds::Topic> ros_to_microros_graph_topic_;
    std::unique_ptr<eprosima::fastdds::dds::DataWriter> ros_to_microros_graph_datawriter_;
    std::unique_ptr<eprosima::fastdds::dds::DataReader> ros_discovery_datareader_;

    // Store a auxiliary publishers and datawriter for each participant created in micro-ROS
    std::map<
        const eprosima::fastdds::dds::DomainParticipant*,
        eprosima::fastdds::dds::DataWriter*
    > micro_ros_graph_datawriters_;
};

}  // namespace graph_manager
}  // namespace agent
}  // namespace uros

#endif  // _UROS_AGENT_GRAPH_MANAGER_HPP
