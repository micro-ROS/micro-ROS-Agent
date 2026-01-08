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

#ifndef _UROS_AGENT_GRAPH_MANAGER_PLUGIN_HPP
#define _UROS_AGENT_GRAPH_MANAGER_PLUGIN_HPP

#include <agent/graph_manager/graph_manager.hpp>

#include <uxr/agent/AgentInstance.hpp>
#include <uxr/agent/middleware/Middleware.hpp>
#include <uxr/agent/middleware/utils/Callbacks.hpp>

#include <map>
#include <memory>
#include <mutex>
#include <atomic>
#include <functional>

namespace uros {
namespace agent {
namespace graph_manager {

/**
 * @brief   Helper class for integrating GraphManager into custom micro-ROS agents.
 */
class GraphManagerPlugin
{
public:
    /**
     * @brief   Default constructor.
     */
    GraphManagerPlugin();

    /**
     * @brief   Destructor. Calls shutdown() if not already called.
     */
    ~GraphManagerPlugin();

    // Non-copyable, non-movable (contains mutex)
    GraphManagerPlugin(const GraphManagerPlugin&) = delete;
    GraphManagerPlugin& operator=(const GraphManagerPlugin&) = delete;
    GraphManagerPlugin(GraphManagerPlugin&&) = delete;
    GraphManagerPlugin& operator=(GraphManagerPlugin&&) = delete;

    /**
     * @brief   Registers graph manager callbacks with the agent.
     * @param   agent Reference to the XRCE-DDS agent.
     * @return  true if callbacks were registered successfully.
     */
    template<typename AgentType>
    bool register_callbacks(AgentType& agent);

    /**
     * @brief   Shuts down all graph managers and cleans up resources.
     */
    void shutdown();

    /**
     * @brief   Gets or creates a GraphManager for the specified domain.
     * @param   domain_id The DDS domain ID.
     * @return  Shared pointer to the GraphManager.
     */
    std::shared_ptr<GraphManager> get_graph_manager(
        eprosima::fastdds::dds::DomainId_t domain_id);

private:
    std::map<eprosima::fastdds::dds::DomainId_t, std::shared_ptr<GraphManager>> graph_manager_map_;
    std::mutex mutex_;
    std::atomic<bool> shutdown_requested_{false};
};

template<typename AgentType>
bool GraphManagerPlugin::register_callbacks(AgentType& agent)
{
    std::function<void(const eprosima::fastdds::dds::DomainParticipant*)> on_create_participant =
        [this](const eprosima::fastdds::dds::DomainParticipant* participant) -> void
        {
            if (shutdown_requested_.load()) return;
            auto gm = get_graph_manager(participant->get_domain_id());
            gm->add_participant(participant);
        };
    agent.add_middleware_callback(
        eprosima::uxr::Middleware::Kind::FASTDDS,
        eprosima::uxr::middleware::CallbackKind::CREATE_PARTICIPANT,
        std::move(on_create_participant));

    std::function<void(const eprosima::fastdds::dds::DomainParticipant*)> on_delete_participant =
        [this](const eprosima::fastdds::dds::DomainParticipant* participant) -> void
        {
            if (shutdown_requested_.load()) return;
            auto gm = get_graph_manager(participant->get_domain_id());
            gm->remove_participant(participant);
        };
    agent.add_middleware_callback(
        eprosima::uxr::Middleware::Kind::FASTDDS,
        eprosima::uxr::middleware::CallbackKind::DELETE_PARTICIPANT,
        std::move(on_delete_participant));

    std::function<void(
        const eprosima::fastdds::dds::DomainParticipant*,
        const eprosima::fastdds::dds::DataWriter*)> on_create_datawriter =
        [this](
            const eprosima::fastdds::dds::DomainParticipant* participant,
            const eprosima::fastdds::dds::DataWriter* datawriter) -> void
        {
            if (shutdown_requested_.load()) return;
            auto gm = get_graph_manager(participant->get_domain_id());
            gm->add_datawriter(datawriter->guid(), participant, datawriter);
            gm->associate_entity(datawriter->guid(), participant, dds::xrce::OBJK_DATAWRITER);
        };
    agent.add_middleware_callback(
        eprosima::uxr::Middleware::Kind::FASTDDS,
        eprosima::uxr::middleware::CallbackKind::CREATE_DATAWRITER,
        std::move(on_create_datawriter));

    std::function<void(
        const eprosima::fastdds::dds::DomainParticipant*,
        const eprosima::fastdds::dds::DataWriter*)> on_delete_datawriter =
        [this](
            const eprosima::fastdds::dds::DomainParticipant* participant,
            const eprosima::fastdds::dds::DataWriter* datawriter) -> void
        {
            if (shutdown_requested_.load()) return;
            auto gm = get_graph_manager(participant->get_domain_id());
            gm->remove_datawriter(datawriter->guid());
        };
    agent.add_middleware_callback(
        eprosima::uxr::Middleware::Kind::FASTDDS,
        eprosima::uxr::middleware::CallbackKind::DELETE_DATAWRITER,
        std::move(on_delete_datawriter));

    std::function<void(
        const eprosima::fastdds::dds::DomainParticipant*,
        const eprosima::fastdds::dds::DataReader*)> on_create_datareader =
        [this](
            const eprosima::fastdds::dds::DomainParticipant* participant,
            const eprosima::fastdds::dds::DataReader* datareader) -> void
        {
            if (shutdown_requested_.load()) return;
            auto gm = get_graph_manager(participant->get_domain_id());
            // Workaround for Fast-DDS bug #9977
            const eprosima::fastrtps::rtps::InstanceHandle_t instance_handle =
                datareader->get_instance_handle();
            const eprosima::fastrtps::rtps::GUID_t datareader_guid =
                eprosima::fastrtps::rtps::iHandle2GUID(instance_handle);
            gm->add_datareader(datareader_guid, participant, datareader);
            gm->associate_entity(datareader_guid, participant, dds::xrce::OBJK_DATAREADER);
        };
    agent.add_middleware_callback(
        eprosima::uxr::Middleware::Kind::FASTDDS,
        eprosima::uxr::middleware::CallbackKind::CREATE_DATAREADER,
        std::move(on_create_datareader));

    std::function<void(
        const eprosima::fastdds::dds::DomainParticipant*,
        const eprosima::fastdds::dds::DataReader*)> on_delete_datareader =
        [this](
            const eprosima::fastdds::dds::DomainParticipant* participant,
            const eprosima::fastdds::dds::DataReader* datareader) -> void
        {
            if (shutdown_requested_.load()) return;
            auto gm = get_graph_manager(participant->get_domain_id());
            // Workaround for Fast-DDS bug #9977
            const eprosima::fastrtps::rtps::InstanceHandle_t instance_handle =
                datareader->get_instance_handle();
            const eprosima::fastrtps::rtps::GUID_t datareader_guid =
                eprosima::fastrtps::rtps::iHandle2GUID(instance_handle);
            gm->remove_datareader(datareader_guid);
        };
    agent.add_middleware_callback(
        eprosima::uxr::Middleware::Kind::FASTDDS,
        eprosima::uxr::middleware::CallbackKind::DELETE_DATAREADER,
        std::move(on_delete_datareader));

    std::function<void(
        const eprosima::fastdds::dds::DomainParticipant*,
        const eprosima::fastdds::dds::DataWriter*,
        const eprosima::fastdds::dds::DataReader*)> on_create_requester =
        [this](
            const eprosima::fastdds::dds::DomainParticipant* participant,
            const eprosima::fastdds::dds::DataWriter* datawriter,
            const eprosima::fastdds::dds::DataReader* datareader) -> void
        {
            if (shutdown_requested_.load()) return;
            auto gm = get_graph_manager(participant->get_domain_id());

            gm->add_datawriter(datawriter->guid(), participant, datawriter);
            gm->associate_entity(datawriter->guid(), participant, dds::xrce::OBJK_DATAWRITER);

            // Workaround for Fast-DDS bug #9977
            const eprosima::fastrtps::rtps::InstanceHandle_t instance_handle =
                datareader->get_instance_handle();
            const eprosima::fastrtps::rtps::GUID_t datareader_guid =
                eprosima::fastrtps::rtps::iHandle2GUID(instance_handle);
            gm->add_datareader(datareader_guid, participant, datareader);
            gm->associate_entity(datareader_guid, participant, dds::xrce::OBJK_DATAREADER);
        };
    agent.add_middleware_callback(
        eprosima::uxr::Middleware::Kind::FASTDDS,
        eprosima::uxr::middleware::CallbackKind::CREATE_REQUESTER,
        std::move(on_create_requester));

    std::function<void(
        const eprosima::fastdds::dds::DomainParticipant*,
        const eprosima::fastdds::dds::DataWriter*,
        const eprosima::fastdds::dds::DataReader*)> on_delete_requester =
        [this](
            const eprosima::fastdds::dds::DomainParticipant* participant,
            const eprosima::fastdds::dds::DataWriter* datawriter,
            const eprosima::fastdds::dds::DataReader* datareader) -> void
        {
            if (shutdown_requested_.load()) return;
            auto gm = get_graph_manager(participant->get_domain_id());

            gm->remove_datawriter(datawriter->guid());

            // Workaround for Fast-DDS bug #9977
            const eprosima::fastrtps::rtps::InstanceHandle_t instance_handle =
                datareader->get_instance_handle();
            const eprosima::fastrtps::rtps::GUID_t datareader_guid =
                eprosima::fastrtps::rtps::iHandle2GUID(instance_handle);
            gm->remove_datareader(datareader_guid);
        };
    agent.add_middleware_callback(
        eprosima::uxr::Middleware::Kind::FASTDDS,
        eprosima::uxr::middleware::CallbackKind::DELETE_REQUESTER,
        std::move(on_delete_requester));

    std::function<void(
        const eprosima::fastdds::dds::DomainParticipant*,
        const eprosima::fastdds::dds::DataWriter*,
        const eprosima::fastdds::dds::DataReader*)> on_create_replier =
        [this](
            const eprosima::fastdds::dds::DomainParticipant* participant,
            const eprosima::fastdds::dds::DataWriter* datawriter,
            const eprosima::fastdds::dds::DataReader* datareader) -> void
        {
            if (shutdown_requested_.load()) return;
            auto gm = get_graph_manager(participant->get_domain_id());

            gm->add_datawriter(datawriter->guid(), participant, datawriter);
            gm->associate_entity(datawriter->guid(), participant, dds::xrce::OBJK_DATAWRITER);

            // Workaround for Fast-DDS bug #9977
            const eprosima::fastrtps::rtps::InstanceHandle_t instance_handle =
                datareader->get_instance_handle();
            const eprosima::fastrtps::rtps::GUID_t datareader_guid =
                eprosima::fastrtps::rtps::iHandle2GUID(instance_handle);
            gm->add_datareader(datareader_guid, participant, datareader);
            gm->associate_entity(datareader_guid, participant, dds::xrce::OBJK_DATAREADER);
        };
    agent.add_middleware_callback(
        eprosima::uxr::Middleware::Kind::FASTDDS,
        eprosima::uxr::middleware::CallbackKind::CREATE_REPLIER,
        std::move(on_create_replier));

    std::function<void(
        const eprosima::fastdds::dds::DomainParticipant*,
        const eprosima::fastdds::dds::DataWriter*,
        const eprosima::fastdds::dds::DataReader*)> on_delete_replier =
        [this](
            const eprosima::fastdds::dds::DomainParticipant* participant,
            const eprosima::fastdds::dds::DataWriter* datawriter,
            const eprosima::fastdds::dds::DataReader* datareader) -> void
        {
            if (shutdown_requested_.load()) return;
            auto gm = get_graph_manager(participant->get_domain_id());

            gm->remove_datawriter(datawriter->guid());

            // Workaround for Fast-DDS bug #9977
            const eprosima::fastrtps::rtps::InstanceHandle_t instance_handle =
                datareader->get_instance_handle();
            const eprosima::fastrtps::rtps::GUID_t datareader_guid =
                eprosima::fastrtps::rtps::iHandle2GUID(instance_handle);
            gm->remove_datareader(datareader_guid);
        };
    agent.add_middleware_callback(
        eprosima::uxr::Middleware::Kind::FASTDDS,
        eprosima::uxr::middleware::CallbackKind::DELETE_REPLIER,
        std::move(on_delete_replier));

    return true;
}

}  // namespace graph_manager
}  // namespace agent
}  // namespace uros

#endif  // _UROS_AGENT_GRAPH_MANAGER_PLUGIN_HPP
