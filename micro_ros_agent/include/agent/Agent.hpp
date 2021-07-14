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

    bool create(
            int argc,
            char** argv);

    void run();

    void add_callbacks();

private:

    eprosima::uxr::AgentInstance& xrce_dds_agent_instance_;
    std::map<eprosima::fastdds::dds::DomainId_t, std::shared_ptr<graph_manager::GraphManager>> graph_manager_map_;

    std::shared_ptr<graph_manager::GraphManager> find_or_create_graph_manager(eprosima::fastdds::dds::DomainId_t domain_id);
    void remove_graph_manager(eprosima::fastdds::dds::DomainId_t domain_id);    // TODO: check grap_manager on agent closing
};

template<typename AgentType>
class AgentAPI : Agent
{
public:
    AgentAPI() : Agent(){};

    ~AgentAPI() = default;

    void create(uint16_t port)
	{
		xrce_dds_agent_instance_API.configure(port);
        add_callbacks();
	}

	void create(std::string dev, const std::string baudrate);
	void create(std::vector<std::string> devs, const std::string baudrate);
	void create(const std::string baudrate);

    void run()
    {
        xrce_dds_agent_instance_API.run();
    }
    
    void stop()
    {
        xrce_dds_agent_instance_API.stop();
    }

private:
    eprosima::uxr::AgentInstanceAPI<AgentType> xrce_dds_agent_instance_API;
};

template<> inline UXR_AGENT_EXPORT void AgentAPI<eprosima::uxr::TermiosAgent>::create(std::string dev, const std::string baudrate)
{
    xrce_dds_agent_instance_API.configure(dev, baudrate);
    add_callbacks();
}

template<> inline UXR_AGENT_EXPORT void AgentAPI<eprosima::uxr::MultiTermiosAgent>::create(std::vector<std::string> devs, const std::string baudrate)
{
    xrce_dds_agent_instance_API.configure(devs, baudrate);
    add_callbacks();
}

template<> inline UXR_AGENT_EXPORT void AgentAPI<eprosima::uxr::PseudoTerminalAgent>::create(const std::string baudrate)
{
    xrce_dds_agent_instance_API.configure(baudrate);
    add_callbacks();
}

}  // namespace agent
}  // namespace uros
#endif  // _UROS_AGENT_AGENT_HPP