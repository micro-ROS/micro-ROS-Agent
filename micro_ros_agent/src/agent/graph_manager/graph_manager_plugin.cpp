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

#include <agent/graph_manager/graph_manager_plugin.hpp>

namespace uros {
namespace agent {
namespace graph_manager {

GraphManagerPlugin::GraphManagerPlugin()
{
}

GraphManagerPlugin::~GraphManagerPlugin()
{
    if (!shutdown_requested_.load()) {
        shutdown();
    }
}

std::shared_ptr<GraphManager> GraphManagerPlugin::get_graph_manager(
    eprosima::fastdds::dds::DomainId_t domain_id)
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = graph_manager_map_.find(domain_id);
    if (it != graph_manager_map_.end()) {
        return it->second;
    }

    auto gm = std::make_shared<GraphManager>(domain_id);
    graph_manager_map_.insert(std::make_pair(domain_id, gm));
    return gm;
}

void GraphManagerPlugin::shutdown()
{
    if (shutdown_requested_.exchange(true)) {
        return;  // Already shut down
    }

    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& [id, gm] : graph_manager_map_) {
        if (gm) {
            gm->shutdown();
        }
    }
    graph_manager_map_.clear();
}

}  // namespace graph_manager
}  // namespace agent
}  // namespace uros
