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

    std::function<void (const eprosima::fastrtps::rtps::GUID_t &,
        const eprosima::fastdds::dds::DomainParticipant *)> on_create_datawriter
        ([&](
            const eprosima::fastrtps::rtps::GUID_t& guid,
            const eprosima::fastdds::dds::DomainParticipant* participant) -> void
        {
            graph_manager_->associate_entity(guid, participant, dds::xrce::OBJK_DATAWRITER);
        });
    xrce_dds_agent_instance_.add_middleware_callback(
        eprosima::uxr::Middleware::Kind::FASTDDS,
        eprosima::uxr::middleware::CallbackKind::CREATE_DATAWRITER,
        std::move(on_create_datawriter));

    std::function<void (const eprosima::fastrtps::rtps::GUID_t &,
        const eprosima::fastdds::dds::DomainParticipant *)> on_create_datareader
        ([&](
            const eprosima::fastrtps::rtps::GUID_t& guid,
            const eprosima::fastdds::dds::DomainParticipant* participant) -> void
        {
            graph_manager_->associate_entity(guid, participant, dds::xrce::OBJK_DATAREADER);
        });
    xrce_dds_agent_instance_.add_middleware_callback(
        eprosima::uxr::Middleware::Kind::FASTDDS,
        eprosima::uxr::middleware::CallbackKind::CREATE_DATAREADER,
        std::move(on_create_datareader));
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