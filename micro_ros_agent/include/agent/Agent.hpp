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

#include <map>
#include <memory>

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

private:

    eprosima::uxr::AgentInstance& xrce_dds_agent_instance_;
    std::map<eprosima::fastdds::dds::DomainId_t, std::shared_ptr<graph_manager::GraphManager>> graph_manager_map_;

    std::shared_ptr<graph_manager::GraphManager> find_or_create_graph_manager(eprosima::fastdds::dds::DomainId_t domain_id);
};

}  // namespace agent
}  // namespace uros
#endif  // _UROS_AGENT_AGENT_HPP