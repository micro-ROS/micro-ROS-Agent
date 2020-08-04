// Copyright 2017-present Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <uxr/agent/AgentInstance.hpp>

int main(int argc, char** argv)
{
    eprosima::uxr::AgentInstance& xrce_dds_agent_instance =
        xrce_dds_agent_instance.getInstance();

    if (!xrce_dds_agent_instance.create(argc, argv))
    {
        return 1;
    }
    xrce_dds_agent_instance.run();

    return 0;
}
