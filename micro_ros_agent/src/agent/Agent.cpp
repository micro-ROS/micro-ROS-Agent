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

#include <utility>
#include <memory>

namespace uros {
namespace agent {

Agent::Agent()
    : xrce_dds_agent_instance_(xrce_dds_agent_instance_.getInstance())
{
}

bool Agent::create(
        int argc,
        char** argv)
{
    bool result = xrce_dds_agent_instance_.create(argc, argv);
    if (result)
    {
        /**
         * Add CREATE_PARTICIPANT callback.
         */
        std::function<void (
            const eprosima::fastdds::dds::DomainParticipant *)> on_create_participant
            ([&](
                const eprosima::fastdds::dds::DomainParticipant* participant) -> void
            {
                auto graph_manager_ = find_or_create_graph_manager(participant->get_domain_id());
                graph_manager_->add_participant(participant);
            });
        xrce_dds_agent_instance_.add_middleware_callback(
            eprosima::uxr::Middleware::Kind::FASTDDS,
            eprosima::uxr::middleware::CallbackKind::CREATE_PARTICIPANT,
            std::move(on_create_participant));

        /**
         * Add REMOVE_PARTICIPANT callback.
         */
        std::function<void (
            const eprosima::fastdds::dds::DomainParticipant *)> on_delete_participant
            ([&](
                const eprosima::fastdds::dds::DomainParticipant* participant) -> void
            {
                auto graph_manager_ = find_or_create_graph_manager(participant->get_domain_id());
                graph_manager_->remove_participant(participant);
            });
        xrce_dds_agent_instance_.add_middleware_callback(
            eprosima::uxr::Middleware::Kind::FASTDDS,
            eprosima::uxr::middleware::CallbackKind::DELETE_PARTICIPANT,
            std::move(on_delete_participant));

        /**
         * Add CREATE_DATAWRITER callback.
         */
        std::function<void (
            const eprosima::fastdds::dds::DomainParticipant *,
            const eprosima::fastdds::dds::DataWriter *)> on_create_datawriter
            ([&](
                const eprosima::fastdds::dds::DomainParticipant* participant,
                const eprosima::fastdds::dds::DataWriter* datawriter) -> void
            {
                auto graph_manager_ = find_or_create_graph_manager(participant->get_domain_id());

                // TODO(jamoralp): Workaround for Fast-DDS bug #9977. Remove when fixed
                const eprosima::fastrtps::rtps::InstanceHandle_t instance_handle =
                    datawriter->get_instance_handle();
                const eprosima::fastrtps::rtps::GUID_t datawriter_guid =
                    iHandle2GUID(instance_handle);
                graph_manager_->add_datawriter(datawriter_guid, participant, datawriter);
                graph_manager_->associate_entity(
                    datawriter_guid, participant, dds::xrce::OBJK_DATAWRITER);
            });
        xrce_dds_agent_instance_.add_middleware_callback(
            eprosima::uxr::Middleware::Kind::FASTDDS,
            eprosima::uxr::middleware::CallbackKind::CREATE_DATAWRITER,
            std::move(on_create_datawriter));

        /**
         * Add DELETE_DATAWRITER callback.
         */
        std::function<void (
            const eprosima::fastdds::dds::DomainParticipant *,
            const eprosima::fastdds::dds::DataWriter *)> on_delete_datawriter
            ([&](
                const eprosima::fastdds::dds::DomainParticipant* participant,
                const eprosima::fastdds::dds::DataWriter* datawriter) -> void
            {

                auto graph_manager_ = find_or_create_graph_manager(participant->get_domain_id());

                // TODO(jamoralp): Workaround for Fast-DDS bug #9977. Remove when fixed
                const eprosima::fastrtps::rtps::InstanceHandle_t instance_handle =
                    datawriter->get_instance_handle();
                const eprosima::fastrtps::rtps::GUID_t datawriter_guid =
                    eprosima::fastrtps::rtps::iHandle2GUID(instance_handle);
                graph_manager_->remove_datawriter(datawriter_guid);
            });

        xrce_dds_agent_instance_.add_middleware_callback(
            eprosima::uxr::Middleware::Kind::FASTDDS,
            eprosima::uxr::middleware::CallbackKind::DELETE_DATAWRITER,
            std::move(on_delete_datawriter));

        /**
         * Add CREATE_DATAREADER callback.
         */
        std::function<void (
            const eprosima::fastdds::dds::DomainParticipant *,
            const eprosima::fastdds::dds::DataReader*)> on_create_datareader
            ([&](
                const eprosima::fastdds::dds::DomainParticipant* participant,
                const eprosima::fastdds::dds::DataReader* datareader) -> void
            {
                auto graph_manager_ = find_or_create_graph_manager(participant->get_domain_id());

                // TODO(jamoralp): Workaround for Fast-DDS bug #9977. Remove when fixed
                const eprosima::fastrtps::rtps::InstanceHandle_t instance_handle =
                    datareader->get_instance_handle();
                const eprosima::fastrtps::rtps::GUID_t datareader_guid =
                    eprosima::fastrtps::rtps::iHandle2GUID(instance_handle);
                graph_manager_->add_datareader(datareader_guid, participant, datareader);
                graph_manager_->associate_entity(
                    datareader_guid, participant, dds::xrce::OBJK_DATAREADER);
            });
        xrce_dds_agent_instance_.add_middleware_callback(
            eprosima::uxr::Middleware::Kind::FASTDDS,
            eprosima::uxr::middleware::CallbackKind::CREATE_DATAREADER,
            std::move(on_create_datareader));

        /**
         * Add DELETE_DATAREADER callback.
         */
        std::function<void (
            const eprosima::fastdds::dds::DomainParticipant *,
            const eprosima::fastdds::dds::DataReader *)> on_delete_datareader
            ([&](
                const eprosima::fastdds::dds::DomainParticipant* participant,
                const eprosima::fastdds::dds::DataReader* datareader) -> void
            {
                auto graph_manager_ = find_or_create_graph_manager(participant->get_domain_id());

                // TODO(jamoralp): Workaround for Fast-DDS bug #9977. Remove when fixed
                const eprosima::fastrtps::rtps::InstanceHandle_t instance_handle =
                    datareader->get_instance_handle();
                const eprosima::fastrtps::rtps::GUID_t datareader_guid =
                    eprosima::fastrtps::rtps::iHandle2GUID(instance_handle);
                graph_manager_->remove_datareader(datareader_guid);
            });

        xrce_dds_agent_instance_.add_middleware_callback(
            eprosima::uxr::Middleware::Kind::FASTDDS,
            eprosima::uxr::middleware::CallbackKind::DELETE_DATAREADER,
            std::move(on_delete_datareader));

        /**
         * Add CREATE_REQUESTER callback.
         */
        std::function<void (
            const eprosima::fastdds::dds::DomainParticipant *,
            const eprosima::fastdds::dds::DataWriter *,
            const eprosima::fastdds::dds::DataReader *)> on_create_requester
            ([&](
                const eprosima::fastdds::dds::DomainParticipant* participant,
                const eprosima::fastdds::dds::DataWriter* datawriter,
                const eprosima::fastdds::dds::DataReader * datareader) -> void
            {
                auto graph_manager_ = find_or_create_graph_manager(participant->get_domain_id());

                // TODO(pablogs): Workaround for Fast-DDS bug #9977. Remove when fixed
                const eprosima::fastrtps::rtps::InstanceHandle_t instance_handle_dw =
                    datawriter->get_instance_handle();
                const eprosima::fastrtps::rtps::GUID_t datawriter_guid =
                    eprosima::fastrtps::rtps::iHandle2GUID(instance_handle_dw);

                graph_manager_->add_datawriter(datawriter_guid, participant, datawriter);
                graph_manager_->associate_entity(
                    datawriter_guid, participant, dds::xrce::OBJK_DATAWRITER);

                // TODO(pablogs): Workaround for Fast-DDS bug #9977. Remove when fixed
                const eprosima::fastrtps::rtps::InstanceHandle_t instance_handle_dr =
                    datareader->get_instance_handle();
                const eprosima::fastrtps::rtps::GUID_t datareader_guid =
                    eprosima::fastrtps::rtps::iHandle2GUID(instance_handle_dr);
                graph_manager_->add_datareader(datareader_guid, participant, datareader);
                graph_manager_->associate_entity(
                    datareader_guid, participant, dds::xrce::OBJK_DATAREADER);
            });
        xrce_dds_agent_instance_.add_middleware_callback(
            eprosima::uxr::Middleware::Kind::FASTDDS,
            eprosima::uxr::middleware::CallbackKind::CREATE_REQUESTER,
            std::move(on_create_requester));

        /**
         * Add DELETE_REQUESTER callback.
         */
        std::function<void (
            const eprosima::fastdds::dds::DomainParticipant *,
            const eprosima::fastdds::dds::DataWriter *,
            const eprosima::fastdds::dds::DataReader *)> on_delete_requester
            ([&](
                const eprosima::fastdds::dds::DomainParticipant* participant,
                const eprosima::fastdds::dds::DataWriter* datawriter,
                const eprosima::fastdds::dds::DataReader * datareader) -> void
            {
                auto graph_manager_ = find_or_create_graph_manager(participant->get_domain_id());

                // TODO(pablogs): Workaround for Fast-DDS bug #9977. Remove when fixed
                const eprosima::fastrtps::rtps::InstanceHandle_t instance_handle_dw =
                    datawriter->get_instance_handle();
                const eprosima::fastrtps::rtps::GUID_t datawriter_guid =
                    eprosima::fastrtps::rtps::iHandle2GUID(instance_handle_dw);
                graph_manager_->remove_datawriter(datawriter_guid);

                // TODO(pablogs): Workaround for Fast-DDS bug #9977. Remove when fixed
                const eprosima::fastrtps::rtps::InstanceHandle_t instance_handle_dr =
                    datareader->get_instance_handle();
                const eprosima::fastrtps::rtps::GUID_t datareader_guid =
                    eprosima::fastrtps::rtps::iHandle2GUID(instance_handle_dr);
                graph_manager_->remove_datareader(datareader_guid);
            });

        xrce_dds_agent_instance_.add_middleware_callback(
            eprosima::uxr::Middleware::Kind::FASTDDS,
            eprosima::uxr::middleware::CallbackKind::DELETE_REQUESTER,
            std::move(on_delete_requester));

        /**
         * Add CREATE_REPLIER callback.
         */
        std::function<void (
            const eprosima::fastdds::dds::DomainParticipant *,
            const eprosima::fastdds::dds::DataWriter *,
            const eprosima::fastdds::dds::DataReader *)> on_create_replier
            ([&](
                const eprosima::fastdds::dds::DomainParticipant* participant,
                const eprosima::fastdds::dds::DataWriter* datawriter,
                const eprosima::fastdds::dds::DataReader * datareader) -> void
            {
                auto graph_manager_ = find_or_create_graph_manager(participant->get_domain_id());

                // TODO(pablogs): Workaround for Fast-DDS bug #9977. Remove when fixed
                const eprosima::fastrtps::rtps::InstanceHandle_t instance_handle_dw =
                    datawriter->get_instance_handle();
                const eprosima::fastrtps::rtps::GUID_t datawriter_guid =
                    eprosima::fastrtps::rtps::iHandle2GUID(instance_handle_dw);
                graph_manager_->add_datawriter(datawriter_guid, participant, datawriter);
                graph_manager_->associate_entity(
                    datawriter_guid, participant, dds::xrce::OBJK_DATAWRITER);

                // TODO(pablogs): Workaround for Fast-DDS bug #9977. Remove when fixed
                const eprosima::fastrtps::rtps::InstanceHandle_t instance_handle_dr =
                    datareader->get_instance_handle();
                const eprosima::fastrtps::rtps::GUID_t datareader_guid =
                    eprosima::fastrtps::rtps::iHandle2GUID(instance_handle_dr);
                graph_manager_->add_datareader(datareader_guid, participant, datareader);
                graph_manager_->associate_entity(
                    datareader_guid, participant, dds::xrce::OBJK_DATAREADER);
            });
        xrce_dds_agent_instance_.add_middleware_callback(
            eprosima::uxr::Middleware::Kind::FASTDDS,
            eprosima::uxr::middleware::CallbackKind::CREATE_REPLIER,
            std::move(on_create_replier));

        /**
         * Add DELETE_REPLIER callback.
         */
        std::function<void (
            const eprosima::fastdds::dds::DomainParticipant *,
            const eprosima::fastdds::dds::DataWriter *,
            const eprosima::fastdds::dds::DataReader *)> on_delete_replier
            ([&](
                const eprosima::fastdds::dds::DomainParticipant* participant,
                const eprosima::fastdds::dds::DataWriter* datawriter,
                const eprosima::fastdds::dds::DataReader * datareader) -> void
            {
                auto graph_manager_ = find_or_create_graph_manager(participant->get_domain_id());

                // TODO(pablogs): Workaround for Fast-DDS bug #9977. Remove when fixed
                const eprosima::fastrtps::rtps::InstanceHandle_t instance_handle_dw =
                    datawriter->get_instance_handle();
                const eprosima::fastrtps::rtps::GUID_t datawriter_guid =
                    eprosima::fastrtps::rtps::iHandle2GUID(instance_handle_dw);
                graph_manager_->remove_datawriter(datawriter_guid);

                // TODO(pablogs): Workaround for Fast-DDS bug #9977. Remove when fixed
                const eprosima::fastrtps::rtps::InstanceHandle_t instance_handle_dr =
                    datareader->get_instance_handle();
                const eprosima::fastrtps::rtps::GUID_t datareader_guid =
                    eprosima::fastrtps::rtps::iHandle2GUID(instance_handle_dr);
                graph_manager_->remove_datareader(datareader_guid);
            });

        xrce_dds_agent_instance_.add_middleware_callback(
            eprosima::uxr::Middleware::Kind::FASTDDS,
            eprosima::uxr::middleware::CallbackKind::DELETE_REPLIER,
            std::move(on_delete_replier));
    }

    return result;
}

void Agent::run()
{
    return xrce_dds_agent_instance_.run();
}

std::shared_ptr<graph_manager::GraphManager> Agent::find_or_create_graph_manager(eprosima::fastdds::dds::DomainId_t domain_id)
{

auto it = graph_manager_map_.find(domain_id);

    if (it != graph_manager_map_.end()) {
        return it->second;
    }else{
        return graph_manager_map_.insert(
            std::make_pair(
                domain_id,
                std::make_shared<graph_manager::GraphManager>(domain_id)
            )
        ).first->second;
    }
}

}  // namespace agent
}  // namespace uros
#endif  // _UROS_AGENT_AGENT_CPP