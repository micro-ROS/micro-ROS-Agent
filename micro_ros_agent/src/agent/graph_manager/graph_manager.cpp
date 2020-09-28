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

#include <agent/graph_manager/graph_manager.hpp>

#ifndef _UROS_AGENT_GRAPH_MANAGER_CPP
#define _UROS_AGENT_GRAPH_MANAGER_CPP

namespace uros {
namespace agent {
namespace graph_manager {

GraphManager::GraphManager()
    : eprosima::fastrtps::ParticipantListener()
    , graph_changed_(false)
    , enclave_("/")
    , mtx_()
    , cv_()
    , graphCache_()
    , participant_info_typesupport_(std::make_unique<ParticipantEntitiesInfoTypeSupport>())
{
    using namespace eprosima::fastrtps;

    // Create participant
    uint32_t domain_id = 0;
    ParticipantAttributes participantAttrs;
    Domain::getDefaultParticipantAttributes(participantAttrs);

    const std::string participant_name = std::string("enclave=") + enclave_;
    participantAttrs.rtps.userData.resize(participant_name.length());
    snprintf(reinterpret_cast<char *>(participantAttrs.rtps.userData.data()),
        participant_name.length(), "enclave=%s;", enclave_);

    participantAttrs.rtps.setName(enclave_);
    participantAttrs.rtps.builtin.readerHistoryMemoryPolicy =
        rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    participantAttrs.rtps.builtin.writerHistoryMemoryPolicy =
        rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    participantAttrs.domainId = domain_id;

    participant_.reset(Domain::createParticipant(participantAttrs), Domain::removeParticipant);

    // Register new participant within participant typesupport
    Domain::registerType(participant_.get(), participant_info_typesupport_.get());

    // Create publisher
    PublisherAttributes publisherAttrs;
    Domain::getDefaultPublisherAttributes(publisherAttrs);

    publisherAttrs.topic.topicKind = rtps::NO_KEY;
    publisherAttrs.topic.topicDataType = participant_info_typesupport_->getName();
    publisherAttrs.topic.topicName = "ros_discovery_info";
    publisherAttrs.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    publisherAttrs.topic.historyQos.depth = 1;
    publisherAttrs.historyMemoryPolicy = rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    publisherAttrs.qos.m_publishMode.kind = ASYNCHRONOUS_PUBLISH_MODE;
    publisherAttrs.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    publisherAttrs.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;

    publisher_.reset(Domain::createPublisher(participant_.get(), publisherAttrs),
        Domain::removePublisher);

    // Set graph cache on change callback function
    graphCache_.set_on_change_callback([this]()
    {
        std::unique_lock<std::mutex> lock(this->mtx_);
        this->graph_changed_ = true;
        this->cv_.notify_one();
    });

    microros_graph_publisher_ = std::thread(&GraphManager::publish_microros_graph, this);
}

inline void GraphManager::publish_microros_graph()
{
    while (true)
    {
        {
            std::unique_lock<std::mutex> lock(mtx_);
            cv_.wait(lock, [this]()
            {
                return this->graph_changed_;
            });
        }

        // micro_ros_agent_msgs::msg::Graph graph_message;

        rcutils_string_array_t node_names = rcutils_get_zero_initialized_string_array();
        rcutils_string_array_t node_namespaces = rcutils_get_zero_initialized_string_array();
        rcutils_allocator_t allocator = rcutils_get_default_allocator();

        graphCache_.get_node_names(&node_names, &node_namespaces, nullptr, &allocator);

        for (size_t i = 0; i < node_names.size; ++i)
        {
            rmw_names_and_types_t rmw_names_and_types =
                rmw_get_zero_initialized_names_and_types();
            const std::string node_name(node_names.data[i]);
            const std::string node_namespace(node_namespaces.data[i]);

            graphCache_.get_writer_names_and_types_by_node(node_name, node_namespace,
                uros::agent::utils::Demangle::demangle_ros_topic_from_topic,
                uros::agent::utils::Demangle::demangle_if_ros_type,
                &allocator, &rmw_names_and_types);

            // Publish: print to screen (TODO(jamoralp): is this really the required behaviour?)
            for (size_t i = 0; i < rmw_names_and_types.names.size; ++i)
            {
                std::cout << rmw_names_and_types.names.data[i] << std::endl;
            }
        }

        if (RCUTILS_RET_OK != rcutils_string_array_fini(&node_names) ||
            RCUTILS_RET_OK != rcutils_string_array_fini(&node_namespaces))
        {
            std::cerr << "Problem while freeing resources in Micro-ROS Graph Manager"
                      << ", file: '" << __FILE__ << "', line: '" << __LINE__ << "'." << std::endl;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void GraphManager::add_participant(
        const eprosima::fastrtps::rtps::GUID_t& guid,
        const eprosima::fastdds::dds::DomainParticipant* participant)
{
    const rmw_gid_t gid = rmw_fastrtps_shared_cpp::create_rmw_gid("rmw_fastrtps_cpp", guid);
    const eprosima::fastdds::dds::DomainParticipantQos qos = participant->get_qos();

    graphCache_.add_participant(gid, enclave_);

    rmw_dds_common::msg::ParticipantEntitiesInfo info =
        graphCache_.add_node(gid, qos.name().c_str(), "/");
    publisher_->write(static_cast<void *>(&info));
}

void GraphManager::associate_entity(
        const eprosima::fastrtps::rtps::GUID_t& guid,
        const eprosima::fastdds::dds::DomainParticipant* participant,
        const dds::xrce::ObjectKind& entity_kind)
{
    const rmw_gid_t gid = rmw_fastrtps_shared_cpp::create_rmw_gid(
        "rmw_fastrtps_cpp", guid);
    const rmw_gid_t participant_gid = rmw_fastrtps_shared_cpp::create_rmw_gid(
        "rmw_fastrtps_cpp", participant->guid());

    eprosima::fastdds::dds::DomainParticipantQos qos = participant->get_qos();
    rmw_dds_common::msg::ParticipantEntitiesInfo info;

    switch (entity_kind)
    {
        case dds::xrce::OBJK_DATAWRITER:
        {
            info = graphCache_.associate_writer(gid, participant_gid, qos.name().c_str(), "/");
            break;
        }
        case dds::xrce::OBJK_DATAREADER:
        {
            info = graphCache_.associate_reader(gid, participant_gid, qos.name().c_str(), "/");
            break;
        }
        default:
        {
            break;
        }
    }
    publisher_->write(static_cast<void *>(&info));
}

void GraphManager::onParticipantDiscovery(
        eprosima::fastrtps::Participant* /*participant*/,
        eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info)
{
    const rmw_gid_t guid =
        rmw_fastrtps_shared_cpp::create_rmw_gid("rmw_fastrtps_cpp", info.info.m_guid);

    switch (info.status)
    {
        case eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT:
        {
            auto map = rmw::impl::cpp::parse_key_value(info.info.m_userData);
            auto name_found = map.find("enclave");

            if (map.end() == name_found)
            {
                return;
            }
            const std::string enclave =
                std::string(name_found->second.begin(), name_found->second.end());

            graphCache_.add_participant(guid, enclave);
            break;
        }
        case eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::REMOVED_PARTICIPANT:
        case eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DROPPED_PARTICIPANT:
        {
            graphCache_.remove_participant(guid);
            break;
        }
        default:
        {
            break;
        }
    }
}

template <>
inline void GraphManager::process_discovery_info<eprosima::fastrtps::rtps::ReaderDiscoveryInfo>(
        const eprosima::fastrtps::rtps::ReaderDiscoveryInfo& reader_info)
{
    const rmw_gid_t guid =
        rmw_fastrtps_shared_cpp::create_rmw_gid("rmw_fastrtps_cpp", reader_info.info.guid());

    switch (reader_info.status)
    {
        case eprosima::fastrtps::rtps::ReaderDiscoveryInfo::CHANGED_QOS_READER:
        {
            return;
        }
        case eprosima::fastrtps::rtps::ReaderDiscoveryInfo::DISCOVERED_READER:
        {
            rmw_qos_profile_t qos_profile = rmw_qos_profile_unknown;
            dds_qos_to_rmw_qos(reader_info.info.m_qos, &qos_profile);

            graphCache_.add_entity(guid, reader_info.info.topicName().to_string(),
                reader_info.info.typeName().to_string(),
                rmw_fastrtps_shared_cpp::create_rmw_gid("rmw_fastrtps_cpp",
                    iHandle2GUID(reader_info.info.RTPSParticipantKey())),
                qos_profile, true);
            break;
        }
        default:
        {
            graphCache_.remove_entity(guid, true);
            break;
        }
    }
}

template <>
inline void GraphManager::process_discovery_info<eprosima::fastrtps::rtps::WriterDiscoveryInfo>(
        const eprosima::fastrtps::rtps::WriterDiscoveryInfo& writer_info)
{
    const rmw_gid_t guid =
        rmw_fastrtps_shared_cpp::create_rmw_gid("rmw_fastrtps_cpp", writer_info.info.guid());

    switch (writer_info.status)
    {
        case eprosima::fastrtps::rtps::WriterDiscoveryInfo::CHANGED_QOS_WRITER:
        {
            return;
        }
        case eprosima::fastrtps::rtps::WriterDiscoveryInfo::DISCOVERED_WRITER:
        {
            rmw_qos_profile_t qos_profile = rmw_qos_profile_unknown;
            dds_qos_to_rmw_qos(writer_info.info.m_qos, &qos_profile);

            graphCache_.add_entity(guid, writer_info.info.topicName().to_string(),
                writer_info.info.typeName().to_string(),
                rmw_fastrtps_shared_cpp::create_rmw_gid("rmw_fastrtps_cpp",
                    iHandle2GUID(writer_info.info.RTPSParticipantKey())),
                qos_profile, false);
            break;
        }
        default:
        {
            graphCache_.remove_entity(guid, false);
            break;
        }
    }
}

void GraphManager::onSubscriberDiscovery(
        eprosima::fastrtps::Participant* /*participant*/,
        eprosima::fastrtps::rtps::ReaderDiscoveryInfo&& info)
{
    process_discovery_info<eprosima::fastrtps::rtps::ReaderDiscoveryInfo>(info);
}

void GraphManager::onPublisherDiscovery(
        eprosima::fastrtps::Participant* /*participant*/,
        eprosima::fastrtps::rtps::WriterDiscoveryInfo&& info)
{
    process_discovery_info<eprosima::fastrtps::rtps::WriterDiscoveryInfo>(info);
}

}  // namespace graph_manager
}  // namespace agent
}  // namespace uros

#endif  // _UROS_AGENT_GRAPH_MANAGER_CPP