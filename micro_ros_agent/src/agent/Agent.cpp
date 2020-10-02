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

#ifndef _UROS_AGENT_AGENT_CPP
#define _UROS_AGENT_AGENT_CPP

#include <agent/Agent.hpp>

namespace uros {
namespace agent {

Agent::Agent()
    : xrce_dds_agent_instance_(xrce_dds_agent_instance_.getInstance())
    , graph_manager_(std::make_unique<graph_manager::GraphManager>())
{
    /**
     * Add CREATE_PARTICIPANT callback.
     */
    std::function<void (const eprosima::fastrtps::rtps::GUID_t &,
        const eprosima::fastdds::dds::DomainParticipant *)> on_create_participant
        ([&](
            const eprosima::fastrtps::rtps::GUID_t& guid,
            const eprosima::fastdds::dds::DomainParticipant* participant) -> void
        {
            graph_manager_->add_participant(guid, participant);
        });
    xrce_dds_agent_instance_.add_middleware_callback(
        eprosima::uxr::Middleware::Kind::FASTDDS,
        eprosima::uxr::middleware::CallbackKind::CREATE_PARTICIPANT,
        std::move(on_create_participant));

    /**
     * Add REMOVE_PARTICIPANT callback.
     */
    std::function<void (const eprosima::fastrtps::rtps::GUID_t &,
        const eprosima::fastdds::dds::DomainParticipant *)> on_delete_participant
        ([&](
            const eprosima::fastrtps::rtps::GUID_t& guid,
            const eprosima::fastdds::dds::DomainParticipant* /*participant*/) -> void
        {
            graph_manager_->remove_participant(guid);
        });
    xrce_dds_agent_instance_.add_middleware_callback(
        eprosima::uxr::Middleware::Kind::FASTDDS,
        eprosima::uxr::middleware::CallbackKind::DELETE_PARTICIPANT,
        std::move(on_delete_participant));

    /**
     * Add CREATE_DATAWRITER callback.
     */
    std::function<void (const eprosima::fastrtps::rtps::GUID_t &,
        const eprosima::fastdds::dds::DomainParticipant *,
        const eprosima::fastdds::dds::DataWriter *)> on_create_datawriter
        ([&](
            const eprosima::fastrtps::rtps::GUID_t& datawriter_guid,
            const eprosima::fastdds::dds::DomainParticipant* participant,
            const eprosima::fastdds::dds::DataWriter* datawriter) -> void
        {
            graph_manager_->add_datawriter(datawriter_guid, participant, datawriter);
            graph_manager_->associate_entity(datawriter_guid, participant, dds::xrce::OBJK_DATAWRITER);
        });
    xrce_dds_agent_instance_.add_middleware_callback(
        eprosima::uxr::Middleware::Kind::FASTDDS,
        eprosima::uxr::middleware::CallbackKind::CREATE_DATAWRITER,
        std::move(on_create_datawriter));

    /**
     * Add DELETE_DATAWRITER callback.
     */
    std::function<void (const eprosima::fastrtps::rtps::GUID_t &,
        const eprosima::fastdds::dds::DomainParticipant *,
        const eprosima::fastdds::dds::DataWriter *)> on_delete_datawriter
        ([&](
            const eprosima::fastrtps::rtps::GUID_t& datawriter_guid,
            const eprosima::fastdds::dds::DomainParticipant* /*participant*/,
            const eprosima::fastdds::dds::DataWriter* /*datawriter*/) -> void
        {
            graph_manager_->remove_datawriter(datawriter_guid);
        });

    xrce_dds_agent_instance_.add_middleware_callback(
        eprosima::uxr::Middleware::Kind::FASTDDS,
        eprosima::uxr::middleware::CallbackKind::DELETE_DATAWRITER,
        std::move(on_delete_datawriter));

    /**
     * Add CREATE_DATAREADER callback.
     */
    std::function<void (const eprosima::fastrtps::rtps::GUID_t &,
        const eprosima::fastdds::dds::DomainParticipant *,
        const eprosima::fastdds::dds::DataReader*)> on_create_datareader
        ([&](
            const eprosima::fastrtps::rtps::GUID_t& datareader_guid,
            const eprosima::fastdds::dds::DomainParticipant* participant,
            const eprosima::fastdds::dds::DataReader* datareader) -> void
        {
            graph_manager_->add_datareader(datareader_guid, participant, datareader);
            graph_manager_->associate_entity(datareader_guid, participant, dds::xrce::OBJK_DATAREADER);
        });
    xrce_dds_agent_instance_.add_middleware_callback(
        eprosima::uxr::Middleware::Kind::FASTDDS,
        eprosima::uxr::middleware::CallbackKind::CREATE_DATAREADER,
        std::move(on_create_datareader));

    /**
     * Add DELETE_DATAREADER callback.
     */
    std::function<void (const eprosima::fastrtps::rtps::GUID_t &,
        const eprosima::fastdds::dds::DomainParticipant *,
        const eprosima::fastdds::dds::DataReader *)> on_delete_datareader
        ([&](
            const eprosima::fastrtps::rtps::GUID_t& datareader_guid,
            const eprosima::fastdds::dds::DomainParticipant* /*participant*/,
            const eprosima::fastdds::dds::DataReader* /*datareader*/) -> void
        {
            graph_manager_->remove_datareader(datareader_guid);
        });

    xrce_dds_agent_instance_.add_middleware_callback(
        eprosima::uxr::Middleware::Kind::FASTDDS,
        eprosima::uxr::middleware::CallbackKind::DELETE_DATAREADER,
        std::move(on_delete_datareader));
}

bool Agent::create(
        int argc,
        char** argv)
{
    return xrce_dds_agent_instance_.create(argc, argv);
}

void Agent::run()
{
    return xrce_dds_agent_instance_.run();
}

}  // namespace agent
}  // namespace uros
#endif  // _UROS_AGENT_AGENT_CPP