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

#ifndef _UROS_AGENT_GRAPH_MANAGER_CPP
#define _UROS_AGENT_GRAPH_MANAGER_CPP

#include <agent/graph_manager/graph_manager.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace uros {
namespace agent {
namespace graph_manager {

GraphManager::GraphManager(eprosima::fastdds::dds::DomainId_t domain_id)
    : domain_id_(domain_id)
    , graph_changed_(false)
    , display_on_change_(false)
    , mtx_()
    , cv_()
    , graphCache_()
    , participant_listener_(std::make_unique<ParticipantListener>(this))
    , datareader_listener_(std::make_unique<DatareaderListener>(this))
    , participant_info_typesupport_(std::make_unique<
        eprosima::fastdds::dds::TypeSupport>(new graph_manager::ParticipantEntitiesInfoTypeSupport()))
    , microros_graph_info_typesupport_(std::make_unique<
        eprosima::fastdds::dds::TypeSupport>(new graph_manager::MicrorosGraphInfoTypeSupport()))
{
    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->load_profiles();

    // Create DomainParticipant
    eprosima::fastdds::dds::DomainParticipantQos participant_qos =
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->get_default_participant_qos();

    const char * enclave = "/";
    size_t length = snprintf(nullptr, 0, "enclave=%s;", enclave) + 1;
    participant_qos.user_data().resize(length);
    snprintf(reinterpret_cast<char *>(participant_qos.user_data().data_vec().data()),
        length, "enclave=%s;", enclave);

    participant_qos.name(enclave);
    participant_qos.wire_protocol().builtin.readerHistoryMemoryPolicy =
        eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    participant_qos.wire_protocol().builtin.writerHistoryMemoryPolicy =
        eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;

    eprosima::fastdds::dds::StatusMask par_mask = eprosima::fastdds::dds::StatusMask::none();
    participant_.reset(eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
        create_participant(domain_id_, participant_qos, participant_listener_.get(), par_mask));

    // Register participant within typesupport
    participant_->register_type(*participant_info_typesupport_);
    participant_->register_type(*microros_graph_info_typesupport_);

    // Create publisher
    publisher_.reset(participant_->create_publisher(
        eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT));

    // Create subscriber
    subscriber_.reset(participant_->create_subscriber(
        eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT));

    // Create topics
    ros_discovery_topic_.reset(participant_->create_topic("ros_discovery_info",
        participant_info_typesupport_->get_type_name(),
        eprosima::fastdds::dds::TOPIC_QOS_DEFAULT));

    ros_to_microros_graph_topic_.reset(participant_->create_topic("ros_to_microros_graph",
        microros_graph_info_typesupport_->get_type_name(),
        eprosima::fastdds::dds::TOPIC_QOS_DEFAULT));

    // Create datawriters
    datawriter_qos_ =
        eprosima::fastdds::dds::DATAWRITER_QOS_DEFAULT;

    datawriter_qos_.history().kind =
        eprosima::fastdds::dds::HistoryQosPolicyKind::KEEP_LAST_HISTORY_QOS;
    datawriter_qos_.history().depth = 1;
    datawriter_qos_.endpoint().history_memory_policy =
        eprosima::fastrtps::rtps::MemoryManagementPolicy::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    datawriter_qos_.publish_mode().kind =
        eprosima::fastdds::dds::PublishModeQosPolicyKind::ASYNCHRONOUS_PUBLISH_MODE;
    datawriter_qos_.reliability().kind =
        eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
    datawriter_qos_.durability().kind =
        eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;

    eprosima::fastdds::dds::DataWriterQos ros_to_microros_datawriter_qos_ = datawriter_qos_;
    ros_to_microros_datawriter_qos_.history().kind =
        eprosima::fastdds::dds::HistoryQosPolicyKind::KEEP_LAST_HISTORY_QOS;
    ros_to_microros_graph_datawriter_.reset(
        publisher_->create_datawriter(ros_to_microros_graph_topic_.get(), ros_to_microros_datawriter_qos_));

    // Create datareaders

    eprosima::fastdds::dds::DataReaderQos datareader_qos =
        eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT;
    datareader_qos.history().kind =
        eprosima::fastdds::dds::HistoryQosPolicyKind::KEEP_LAST_HISTORY_QOS;
    datareader_qos.history().depth = 1;
    datareader_qos.endpoint().history_memory_policy =
        eprosima::fastrtps::rtps::MemoryManagementPolicy::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    datareader_qos.reliability().kind =
        eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
    datareader_qos.durability().kind =
        eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;

    ros_discovery_datareader_.reset(
        subscriber_->create_datareader(ros_discovery_topic_.get(),
            datareader_qos, datareader_listener_.get()));

    // Set graph cache on change callback function
    graphCache_.set_on_change_callback([this]()
    {
        {
            std::unique_lock<std::mutex> lock(this->mtx_);
            this->graph_changed_ = true;
        }
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
            graph_changed_ = false;
        }

        if (display_on_change_)
        {
            std::cout << "Updated uros Graph: graph changed" << std::endl;
            std::cout << graphCache_ << std::endl;
        }

        micro_ros_msgs::msg::Graph graph_message;

        rcutils_string_array_t node_names = rcutils_get_zero_initialized_string_array();
        rcutils_string_array_t node_namespaces = rcutils_get_zero_initialized_string_array();
        rcutils_allocator_t allocator = rcutils_get_default_allocator();

        graphCache_.get_node_names(&node_names, &node_namespaces, nullptr, &allocator);

        for (size_t i = 0; i < node_names.size; ++i)
        {
            const std::string node_name(node_names.data[i]);
            const std::string node_namespace(node_namespaces.data[i]);

            micro_ros_msgs::msg::Node node_message;
            node_message.node_namespace = std::move(node_namespace);
            node_message.node_name = std::move(node_name);

            // Get publishers info
            rmw_names_and_types_t writer_names_and_types =
                rmw_get_zero_initialized_names_and_types();
            if (RMW_RET_OK != graphCache_.get_writer_names_and_types_by_node(node_name, node_namespace,
                uros::agent::utils::Demangle::demangle_ros_topic_from_topic,
                uros::agent::utils::Demangle::demangle_if_ros_type,
                &allocator, &writer_names_and_types))
            {
                break;
            }

            for (size_t i = 0; i < writer_names_and_types.names.size; ++i)
            {
                micro_ros_msgs::msg::Entity entity_message;
                entity_message.entity_type = micro_ros_msgs::msg::Entity::PUBLISHER;
                entity_message.name = std::move(std::string(writer_names_and_types.names.data[i]));

                for (size_t j = 0; j < writer_names_and_types.types[i].size; ++j)
                {
                    entity_message.types.emplace_back(writer_names_and_types.types[i].data[j]);
                }

                node_message.entities.emplace_back(std::move(entity_message));
            }

            // Get subscribers info
            rmw_names_and_types_t reader_names_and_types =
                rmw_get_zero_initialized_names_and_types();
            if (RMW_RET_OK != graphCache_.get_reader_names_and_types_by_node(node_name, node_namespace,
                uros::agent::utils::Demangle::demangle_ros_topic_from_topic,
                uros::agent::utils::Demangle::demangle_if_ros_type,
                &allocator, &reader_names_and_types))
            {
                break;
            }

            for (size_t i = 0; i < reader_names_and_types.names.size; ++i)
            {
                micro_ros_msgs::msg::Entity entity_message;
                entity_message.entity_type = micro_ros_msgs::msg::Entity::SUBSCRIBER;
                entity_message.name = std::move(std::string(reader_names_and_types.names.data[i]));

                for (size_t j = 0; j < reader_names_and_types.types[i].size; ++j)
                {
                    entity_message.types.emplace_back(reader_names_and_types.types[i].data[j]);
                }

                node_message.entities.emplace_back(std::move(entity_message));
            }

            // Get services
            //// Get servers
            rmw_names_and_types_t service_server_names_and_types =
              rmw_get_zero_initialized_names_and_types();
            if (RMW_RET_OK != graphCache_.get_names_and_types(
                uros::agent::utils::Demangle::demangle_service_request_from_topic,
                uros::agent::utils::Demangle::demangle_service_type_only,
                &allocator, &service_server_names_and_types))
            {
                break;
            }

            for (size_t i = 0; i < service_server_names_and_types.names.size; ++i)
            {
                micro_ros_msgs::msg::Entity entity_message;
                entity_message.entity_type = micro_ros_msgs::msg::Entity::SERVICE_SERVER;
                entity_message.name = std::move(std::string(service_server_names_and_types.names.data[i]));

                for (size_t j = 0; j < service_server_names_and_types.types[i].size; ++j)
                {
                    entity_message.types.emplace_back(service_server_names_and_types.types[i].data[j]);
                }

                node_message.entities.emplace_back(std::move(entity_message));
            }

            //// Get clients
            rmw_names_and_types_t service_client_names_and_types =
              rmw_get_zero_initialized_names_and_types();
            if (RMW_RET_OK != graphCache_.get_names_and_types(
                uros::agent::utils::Demangle::demangle_service_reply_from_topic,
                uros::agent::utils::Demangle::demangle_service_type_only,
                &allocator, &service_client_names_and_types))
            {
              break;
            }

            for (size_t i = 0; i < service_client_names_and_types.names.size; ++i)
            {
                micro_ros_msgs::msg::Entity entity_message;
                entity_message.entity_type = micro_ros_msgs::msg::Entity::SERVICE_CLIENT;
                entity_message.name = std::move(std::string(service_client_names_and_types.names.data[i]));

                for (size_t j = 0; j < service_client_names_and_types.types[i].size; ++j)
                {
                    entity_message.types.emplace_back(service_client_names_and_types.types[i].data[j]);
                }

                node_message.entities.emplace_back(std::move(entity_message));
            }

            graph_message.nodes.emplace_back(std::move(node_message));

            if (RMW_RET_OK != rmw_names_and_types_fini(&writer_names_and_types) ||
                RMW_RET_OK != rmw_names_and_types_fini(&reader_names_and_types) ||
                RMW_RET_OK != rmw_names_and_types_fini(&service_server_names_and_types) ||
                RMW_RET_OK != rmw_names_and_types_fini(&service_client_names_and_types))
            {
                std::cerr << "Problem while freeing resources in Micro-ROS Graph Manager"
                          << ", file: '" << __FILE__ << "', line: '" << __LINE__ << "'." << std::endl;
                return;
            }
        }

        ros_to_microros_graph_datawriter_->write(static_cast<void *>(&graph_message));

        if (RCUTILS_RET_OK != rcutils_string_array_fini(&node_names) ||
            RCUTILS_RET_OK != rcutils_string_array_fini(&node_namespaces))
        {
            std::cerr << "Problem while freeing resources in Micro-ROS Graph Manager"
                      << ", file: '" << __FILE__ << "', line: '" << __LINE__ << "'." << std::endl;
            break;
        }
    }
}

void GraphManager::add_participant(
        const eprosima::fastdds::dds::DomainParticipant* participant,
        bool from_microros,
        const std::string& enclave)
{
    const eprosima::fastdds::dds::DomainParticipantQos qos = participant->get_qos();
    const rmw_gid_t gid = rmw_fastrtps_shared_cpp::create_rmw_gid("rmw_fastrtps_cpp", participant->guid());

    graphCache_.add_participant(gid, enclave);

    // Do not add root node and
    // do not announce non-micro-ROS participants
    if (qos.name().to_string() != "/" && from_microros)
    {
        std::string isolated_node_name, isolated_namespace;
        get_name_and_namespace(qos.name().to_string(), isolated_node_name, isolated_namespace);

        rmw_dds_common::msg::ParticipantEntitiesInfo info =
            graphCache_.add_node(gid, isolated_node_name, isolated_namespace);

        auto it = micro_ros_graph_datawriters_.find(participant);
        if (it == micro_ros_graph_datawriters_.end())
        {
            // Create datawriter
            eprosima::fastdds::dds::DataWriter * datawriter = publisher_->create_datawriter(ros_discovery_topic_.get(), datawriter_qos_);

            it = micro_ros_graph_datawriters_.insert(
                std::make_pair(participant, datawriter)).first;
        }

        it->second->write(static_cast<void *>(&info));
    }
}

void GraphManager::remove_participant(
        const eprosima::fastdds::dds::DomainParticipant* participant,
        bool from_microros)
{
    const rmw_gid_t gid = rmw_fastrtps_shared_cpp::create_rmw_gid("rmw_fastrtps_cpp", participant->guid());
    graphCache_.remove_participant(gid);

    if (from_microros)
    {
        rmw_dds_common::msg::ParticipantEntitiesInfo info;
        rmw_dds_common::convert_gid_to_msg(&gid, &info.gid);
        auto it = micro_ros_graph_datawriters_.find(participant);
        it->second->write(static_cast<void *>(&info));
        publisher_->delete_datawriter(it->second);
        micro_ros_graph_datawriters_.erase(participant);
    }
}

void GraphManager::add_datawriter(
        const eprosima::fastrtps::rtps::GUID_t& datawriter_guid,
        const eprosima::fastdds::dds::DomainParticipant* participant,
        const eprosima::fastdds::dds::DataWriter* datawriter)
{
    const std::string& topic_name = datawriter->get_topic()->get_name();
    const std::string& type_name = datawriter->get_topic()->get_type_name();
    this->add_datawriter(datawriter_guid, topic_name, type_name,
        participant->guid(), datawriter->get_qos());
}

void GraphManager::add_datawriter(
        const eprosima::fastrtps::rtps::GUID_t& datawriter_guid,
        const std::string& topic_name,
        const std::string& type_name,
        const eprosima::fastrtps::rtps::GUID_t& participant_guid,
        const eprosima::fastdds::dds::DataWriterQos& writer_qos)
{
    const rmw_gid_t datawriter_gid = rmw_fastrtps_shared_cpp::create_rmw_gid(
        "rmw_fastrtps_cpp", datawriter_guid);
    const rmw_gid_t participant_gid = rmw_fastrtps_shared_cpp::create_rmw_gid(
        "rmw_fastrtps_cpp", participant_guid);
    const rmw_qos_profile_t qos_profile = fastdds_qos_to_rmw_qos(writer_qos);

    graphCache_.add_entity(datawriter_gid, topic_name,
        type_name, participant_gid, qos_profile, false);
}

void GraphManager::remove_datawriter(
        const eprosima::fastrtps::rtps::GUID_t& datawriter_guid)
{
    const rmw_gid_t datawriter_gid = rmw_fastrtps_shared_cpp::create_rmw_gid(
        "rmw_fastrtps_cpp", datawriter_guid);

    graphCache_.remove_entity(datawriter_gid, false);
}

void GraphManager::add_datareader(
        const eprosima::fastrtps::rtps::GUID_t& datareader_guid,
        const eprosima::fastdds::dds::DomainParticipant* participant,
        const eprosima::fastdds::dds::DataReader* datareader)
{
    const std::string& topic_name = datareader->get_topicdescription()->get_name();
    const std::string& type_name = datareader->get_topicdescription()->get_type_name();
    this->add_datareader(datareader_guid, topic_name, type_name,
        participant->guid(), datareader->get_qos());
}

void GraphManager::add_datareader(
        const eprosima::fastrtps::rtps::GUID_t& datareader_guid,
        const std::string& topic_name,
        const std::string& type_name,
        const eprosima::fastrtps::rtps::GUID_t& participant_guid,
        const eprosima::fastdds::dds::DataReaderQos& reader_qos)
{
    const rmw_gid_t datareader_gid = rmw_fastrtps_shared_cpp::create_rmw_gid(
        "rmw_fastrtps_cpp", datareader_guid);
    const rmw_gid_t participant_gid = rmw_fastrtps_shared_cpp::create_rmw_gid(
        "rmw_fastrtps_cpp", participant_guid);
    const rmw_qos_profile_t qos_profile = fastdds_qos_to_rmw_qos(reader_qos);

    graphCache_.add_entity(datareader_gid, topic_name,
        type_name, participant_gid, qos_profile, true);
}

void GraphManager::remove_datareader(
        const eprosima::fastrtps::rtps::GUID_t& datareader_guid)
{
    const rmw_gid_t datareader_gid = rmw_fastrtps_shared_cpp::create_rmw_gid(
        "rmw_fastrtps_cpp", datareader_guid);

    graphCache_.remove_entity(datareader_gid, true);
}

void GraphManager::associate_entity(
        const eprosima::fastrtps::rtps::GUID_t& entity_guid,
        const eprosima::fastdds::dds::DomainParticipant* participant,
        const dds::xrce::ObjectKind& entity_kind)
{
    const rmw_gid_t entity_gid = rmw_fastrtps_shared_cpp::create_rmw_gid(
        "rmw_fastrtps_cpp", entity_guid);
    const rmw_gid_t participant_gid = rmw_fastrtps_shared_cpp::create_rmw_gid(
        "rmw_fastrtps_cpp", participant->guid());

    eprosima::fastdds::dds::DomainParticipantQos qos = participant->get_qos();
    rmw_dds_common::msg::ParticipantEntitiesInfo info;

    switch (entity_kind)
    {
        case dds::xrce::OBJK_DATAWRITER:
        {
            std::string isolated_node_name, isolated_namespace;
            get_name_and_namespace(qos.name().c_str(), isolated_node_name, isolated_namespace);
            info = graphCache_.associate_writer(
                entity_gid, participant_gid, isolated_node_name, isolated_namespace);
            break;
        }
        case dds::xrce::OBJK_DATAREADER:
        {
            std::string isolated_node_name, isolated_namespace;
            get_name_and_namespace(qos.name().c_str(), isolated_node_name, isolated_namespace);
            info = graphCache_.associate_reader(
                entity_gid, participant_gid, isolated_node_name, isolated_namespace);
            break;
        }
        default:
        {
            break;
        }
    }

    auto it = micro_ros_graph_datawriters_.find(participant);
    it->second->write(static_cast<void *>(&info));
}


template <typename FastDDSQos>
const rmw_qos_profile_t GraphManager::fastdds_qos_to_rmw_qos(
        const FastDDSQos& fastdds_qos)
{
    rmw_qos_profile_t rmw_qos = rmw_qos_profile_unknown;
    switch (fastdds_qos.reliability().kind)
    {
        case eprosima::fastdds::dds::ReliabilityQosPolicyKind::BEST_EFFORT_RELIABILITY_QOS:
        {
            rmw_qos.reliability = RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT;
            break;
        }
        case eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS:
        {
            rmw_qos.reliability = RMW_QOS_POLICY_RELIABILITY_RELIABLE;
            break;
        }
        default:
        {
            rmw_qos.reliability = RMW_QOS_POLICY_RELIABILITY_UNKNOWN;
            break;
        }
    }

    switch (fastdds_qos.durability().kind)
    {
        case eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS:
        {
            rmw_qos.durability = RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL;
            break;
        }
        case eprosima::fastdds::dds::DurabilityQosPolicyKind::VOLATILE_DURABILITY_QOS:
        {
            rmw_qos.durability = RMW_QOS_POLICY_DURABILITY_VOLATILE;
            break;
        }
        default:
        {
            rmw_qos.durability = RMW_QOS_POLICY_DURABILITY_UNKNOWN;
            break;
        }
    }

    rmw_qos.deadline.sec = fastdds_qos.deadline().period.seconds;
    rmw_qos.deadline.nsec = fastdds_qos.deadline().period.nanosec;

    rmw_qos.lifespan.sec = fastdds_qos.lifespan().duration.seconds;
    rmw_qos.lifespan.nsec = fastdds_qos.lifespan().duration.nanosec;

    switch (fastdds_qos.liveliness().kind)
    {
        case eprosima::fastdds::dds::LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS:
        {
            rmw_qos.liveliness = RMW_QOS_POLICY_LIVELINESS_AUTOMATIC;
            break;
        }
        case eprosima::fastdds::dds::LivelinessQosPolicyKind::MANUAL_BY_TOPIC_LIVELINESS_QOS:
        {
            rmw_qos.liveliness = RMW_QOS_POLICY_LIVELINESS_MANUAL_BY_TOPIC;
            break;
        }
        default:
        {
            rmw_qos.liveliness = RMW_QOS_POLICY_LIVELINESS_UNKNOWN;
            break;
        }
    }

    rmw_qos.liveliness_lease_duration.sec = fastdds_qos.liveliness().lease_duration.seconds;
    rmw_qos.liveliness_lease_duration.nsec = fastdds_qos.liveliness().lease_duration.nanosec;

    return rmw_qos;
}

void GraphManager::update_node_entities_info()
{
    rmw_dds_common::msg::ParticipantEntitiesInfo entities_info;
    eprosima::fastdds::dds::SampleInfo sample_info;
    if (ros_discovery_datareader_->take_next_sample(&entities_info, &sample_info) ==
        eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK)
    {
        if (sample_info.instance_state == eprosima::fastdds::dds::InstanceStateKind::ALIVE_INSTANCE_STATE)
        {
            graphCache_.update_participant_entities(entities_info);
        }
    }
}

void GraphManager::get_name_and_namespace(
    std::string participant_name,
    std::string& node_name,
    std::string& node_namespace)
{
    // Remove first / if exists
    if (participant_name.rfind("/", 0) == 0)
    {
        participant_name.erase(participant_name.begin());
    }

    // Split node name in domain and node name
    std::istringstream iss(participant_name);
    std::vector<std::string> result;
    std::string token;

    while(std::getline(iss, token, '/'))
    {
        result.push_back(token);
    }

    if (result.size() > 1)
    {
        node_namespace = "/" + result[0];
        for (size_t i = 1; i < result.size(); i++)
        {
            node_name.append(result[i] + "/");
        }
        node_name.pop_back();
    }
    else
    {
        node_name = participant_name;
        node_namespace = "/";
    }
}

GraphManager::ParticipantListener::ParticipantListener(
        GraphManager* graph_manager)
    : eprosima::fastdds::dds::DomainParticipantListener()
    , graphManager_from_(graph_manager)
{
}

void GraphManager::ParticipantListener::on_participant_discovery(
        eprosima::fastdds::dds::DomainParticipant* /* participant */,
        eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info)
{
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

            const rmw_gid_t gid = rmw_fastrtps_shared_cpp::create_rmw_gid("rmw_fastrtps_cpp", info.info.m_guid);
            graphManager_from_->get_graph_cache().add_participant(gid, enclave);
            break;
        }
        case eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::REMOVED_PARTICIPANT:
        case eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DROPPED_PARTICIPANT:
        {
            const rmw_gid_t gid = rmw_fastrtps_shared_cpp::create_rmw_gid("rmw_fastrtps_cpp", info.info.m_guid);
            graphManager_from_->get_graph_cache().remove_participant(gid);
            break;
        }
        default:
        {
            break;
        }
    }
}

static eprosima::fastdds::dds::DataWriterQos writer_qos_conversion(
    const eprosima::fastdds::dds::WriterQos& writer_qos)
{
    eprosima::fastdds::dds::DataWriterQos datawriter_qos;
    datawriter_qos.durability(writer_qos.m_durability);
    datawriter_qos.durability_service(writer_qos.m_durabilityService);
    datawriter_qos.deadline(writer_qos.m_deadline);
    datawriter_qos.latency_budget(writer_qos.m_latencyBudget);
    datawriter_qos.liveliness(writer_qos.m_liveliness);
    datawriter_qos.reliability(writer_qos.m_reliability);
    datawriter_qos.destination_order(writer_qos.m_destinationOrder);
    datawriter_qos.lifespan(writer_qos.m_lifespan);
    datawriter_qos.user_data(writer_qos.m_userData);
    datawriter_qos.ownership(writer_qos.m_ownership);
    datawriter_qos.ownership_strength(writer_qos.m_ownershipStrength);
    datawriter_qos.publish_mode(writer_qos.m_publishMode);
    datawriter_qos.representation(writer_qos.representation);
    datawriter_qos.data_sharing(writer_qos.data_sharing);

    return datawriter_qos;
}

static eprosima::fastdds::dds::DataReaderQos reader_qos_conversion(
    const eprosima::fastdds::dds::ReaderQos& reader_qos)
{
    eprosima::fastdds::dds::DataReaderQos datareader_qos;

    datareader_qos.durability(reader_qos.m_durability);
    datareader_qos.deadline(reader_qos.m_deadline);
    datareader_qos.latency_budget(reader_qos.m_latencyBudget);
    datareader_qos.liveliness(reader_qos.m_liveliness);
    datareader_qos.reliability(reader_qos.m_reliability);
    datareader_qos.destination_order(reader_qos.m_destinationOrder);
    datareader_qos.user_data(reader_qos.m_userData);
    datareader_qos.ownership(reader_qos.m_ownership);
    datareader_qos.time_based_filter(reader_qos.m_timeBasedFilter);
    datareader_qos.lifespan(reader_qos.m_lifespan);
    datareader_qos.durability_service(reader_qos.m_durabilityService);
    eprosima::fastdds::dds::TypeConsistencyQos consistency;
    consistency.type_consistency = reader_qos.type_consistency;
    datareader_qos.type_consistency(consistency);
    datareader_qos.data_sharing(reader_qos.data_sharing);

    return datareader_qos;
}

template <>
void GraphManager::ParticipantListener::process_discovery_info<eprosima::fastrtps::rtps::ReaderDiscoveryInfo>(
        const eprosima::fastrtps::rtps::ReaderDiscoveryInfo& reader_info)
{
    switch (reader_info.status)
    {
        case eprosima::fastrtps::rtps::ReaderDiscoveryInfo::CHANGED_QOS_READER:
        {
            return;
        }
        case eprosima::fastrtps::rtps::ReaderDiscoveryInfo::DISCOVERED_READER:
        {
            const std::string topic_name = reader_info.info.topicName().to_string();
            const std::string type_name = reader_info.info.typeName().to_string();

            graphManager_from_->add_datareader(reader_info.info.guid(), topic_name, type_name,
                iHandle2GUID(reader_info.info.RTPSParticipantKey()), reader_qos_conversion(reader_info.info.m_qos));
            break;
        }
        default:
        {
            graphManager_from_->remove_datareader(reader_info.info.guid());
            break;
        }
    }
}

template <>
void GraphManager::ParticipantListener::process_discovery_info<eprosima::fastrtps::rtps::WriterDiscoveryInfo>(
        const eprosima::fastrtps::rtps::WriterDiscoveryInfo& writer_info)
{
    switch (writer_info.status)
    {
        case eprosima::fastrtps::rtps::WriterDiscoveryInfo::CHANGED_QOS_WRITER:
        {
            return;
        }
        case eprosima::fastrtps::rtps::WriterDiscoveryInfo::DISCOVERED_WRITER:
        {
            const std::string topic_name = writer_info.info.topicName().to_string();
            const std::string type_name = writer_info.info.typeName().to_string();

            graphManager_from_->add_datawriter(writer_info.info.guid(), topic_name, type_name,
                iHandle2GUID(writer_info.info.RTPSParticipantKey()), writer_qos_conversion(writer_info.info.m_qos));
            break;
        }
        default:
        {
            graphManager_from_->remove_datawriter(writer_info.info.guid());
            break;
        }
    }
}

void GraphManager::ParticipantListener::on_subscriber_discovery(
        eprosima::fastdds::dds::DomainParticipant* /*participant*/,
        eprosima::fastrtps::rtps::ReaderDiscoveryInfo&& info)
{
    process_discovery_info<eprosima::fastrtps::rtps::ReaderDiscoveryInfo>(info);
}

void GraphManager::ParticipantListener::on_publisher_discovery(
        eprosima::fastdds::dds::DomainParticipant* /*participant*/,
        eprosima::fastrtps::rtps::WriterDiscoveryInfo&& info)
{
    process_discovery_info<eprosima::fastrtps::rtps::WriterDiscoveryInfo>(info);
}

GraphManager::DatareaderListener::DatareaderListener(
        GraphManager* graph_manager)
    : eprosima::fastdds::dds::DataReaderListener()
    , graphManager_from_(graph_manager)
{
}

void GraphManager::DatareaderListener::on_data_available(
        eprosima::fastdds::dds::DataReader* /*sub*/)
{
    graphManager_from_->update_node_entities_info();
}

}  // namespace graph_manager
}  // namespace agent
}  // namespace uros

#endif  // _UROS_AGENT_GRAPH_MANAGER_CPP