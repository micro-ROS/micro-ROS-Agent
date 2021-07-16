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

#ifndef _UROS_AGENT_AGENT_HPP
#define _UROS_AGENT_AGENT_HPP

#include <uxr/agent/AgentInstance.hpp>
#include <uxr/agent/middleware/Middleware.hpp>
#include <uxr/agent/middleware/utils/Callbacks.hpp>

#include <agent/graph_manager/graph_manager.hpp>
// TODO(jamoralp): class Documentation
namespace uros {
namespace agent {

class Agent
{
public:

    Agent();

    ~Agent() = default;

    /**
     * @brief   Agent class shall not be copy constructible.
     */
    UXR_AGENT_EXPORT Agent(
            const Agent &) = delete;

    UXR_AGENT_EXPORT Agent(
            Agent &&) = delete;

    /**
     * @brief   Agent class shall not be copy assignable.
     */
    UXR_AGENT_EXPORT Agent& operator =(
            const Agent &) = delete;

    UXR_AGENT_EXPORT Agent& operator =(
            Agent &&) = delete;

    static Agent& getInstance()
    {
        static Agent instance;
        return instance;
    }

    bool create(
            int argc,
            char** argv);

    void run();

    void add_callbacks();

private:
    eprosima::uxr::AgentInstance& xrce_dds_agent_instance_;
    std::map<eprosima::fastdds::dds::DomainId_t, std::shared_ptr<graph_manager::GraphManager>> graph_manager_map_;
    std::shared_ptr<graph_manager::GraphManager> find_or_create_graph_manager(eprosima::fastdds::dds::DomainId_t domain_id);
    bool started = false;
};

template<typename AgentType>
class AgentAPI
{
public:
    AgentAPI()
    : agent_instance_(agent_instance_.getInstance())
    {};

    ~AgentAPI() = default;

    void create(uint16_t port)
	{
		xrce_dds_agent_instance_API.configure(port);
	}

	void create(std::string dev, const std::string baudrate);
	void create(std::vector<std::string> devs, const std::string baudrate);
	void create(const std::string baudrate);

    void set_verbose_level(uint8_t verbose_level)
    {
        xrce_dds_agent_instance_API.set_verbose_level(verbose_level);
    }

    void run()
    {
        agent_instance_.add_callbacks();
        xrce_dds_agent_instance_API.run();
    }

    void stop()
    {
        xrce_dds_agent_instance_API.stop();
    }

private:
    Agent& agent_instance_;
    eprosima::uxr::AgentInstanceAPI<AgentType> xrce_dds_agent_instance_API;
};

template<> inline UXR_AGENT_EXPORT void AgentAPI<eprosima::uxr::TermiosAgent>::create(std::string dev, const std::string baudrate)
{
    xrce_dds_agent_instance_API.configure(dev, baudrate);
}

template<> inline UXR_AGENT_EXPORT void AgentAPI<eprosima::uxr::MultiTermiosAgent>::create(std::vector<std::string> devs, const std::string baudrate)
{
    xrce_dds_agent_instance_API.configure(devs, baudrate);

}

template<> inline UXR_AGENT_EXPORT void AgentAPI<eprosima::uxr::PseudoTerminalAgent>::create(const std::string baudrate)
{
    xrce_dds_agent_instance_API.configure(baudrate);
}

}  // namespace agent
}  // namespace uros
#endif  // _UROS_AGENT_AGENT_HPP