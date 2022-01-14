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

#include <agent/Agent.hpp>

#include <string>
#include <vector>

int main(int argc, char** argv)
{
    uros::agent::Agent micro_ros_agent;

    /** Bypass '--ros-args' flag, as we use our own CLI parser.
      * As a workaround for launch files, arguments will be passed from
      * ros2launch as a single token with spaces,
      * to preserve the correct argument order; split it here.
      * TODO(jamoralp): investigate ROS2 tools for properly parsing params from launch
      **/
    std::vector<std::string> params;
    for (int i = 0; i < argc; ++i)
    {
        if(strcmp("--ros-args", argv[i]) == 0)
        {
            argc = i;
            break;
        }
        params.emplace_back(std::string(argv[i]));
    }

    auto it = std::find(params.begin(), params.end(), "--ros-args");
    if (params.end() != it)
    {
        if ((it - params.begin()) != 2)
        {
            std::ostringstream ss;
            ss << "Error: when using ros2 launch, please specify ";
            ss << "your arguments in a single variable." << std::endl;
            ss << "Instead of 'arguments: {'udp4', '-p', '8888'}', do ";
            ss << "'arguments: {'udp4 -p 8888'}" << std::endl;

            std::cerr << ss.str();
            return 1;
        }
        const std::string& agent_args(params.at(1));
        std::istringstream iss(agent_args);
        std::vector<std::string> agent_args_split(
            std::istream_iterator<std::string>{iss},
            std::istream_iterator<std::string>());

        char** agent_argv = new char*[agent_args_split.size() + 1];
        agent_argv[0] = argv[0];
        char** agent_argv_it = &agent_argv[1];

        for (const auto& arg : agent_args_split)
        {
            *agent_argv_it = new char[arg.length() + 1];
            strcpy(*agent_argv_it, arg.c_str());
            agent_argv_it++;
        }

        bool success = micro_ros_agent.create(agent_args_split.size() + 1, agent_argv);

        for (size_t i = 1; i <= agent_args_split.size(); ++i)
        {
            delete [] agent_argv[i];
        }
        delete [] agent_argv;

        if (!success)
        {
            return 1;
        }
    }
    else if (!micro_ros_agent.create(argc, argv))
    {
        return 1;
    }

    micro_ros_agent.run();

    return 0;
}