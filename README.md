<a href="http://www.eprosima.com"><img src="http://www.eprosima.com/images/logos/eprosima/logo.png" align="top" hspace="8" vspace="2" width="650" height="200" ></a>

# Overview

This repo contains the Micro-Ros Agent package.
Micro-Ros Agent is a ROS2 node that wrapps the Micro XRCE-DDS Agent.
For farther information about Micro XRCE-DDA Agent click [here](https://github.com/eProsima/Micro-XRCE-DDS-Agent)
This package is a part of the Micro-ROS poject stack. 
For more information about Micro-ROS project click [here]().

The node acts as a server between DDS Network and Micro-ROS nodes.
It will receive messages containing operations from Micro-ROS nodes, keep track of the Micro-ROS nodes. 
The node will interact with DDS Global Data Space on behalf of the Micro-ROS nodes.

While the node is running, it attends any received request from the Micro-ROS nodes and will answers back with the result of a request each time a request is attended.


# Package features

## XML generation

During the build proces, the package will look for all ROS2 / Micro-ROS messages in order to generare XML profiles.
This XML file can be referenced in the Agent-Client comunication in order to avoid sending the full XML content.


## Agent-Client comunication mechanims

Communication between the Micro-ROS Agent and the Micro-ROS nodes supports two kind transports: 
- UDP 
- SerialPort. 


All available configurations are supported direcly by the Micro XRCE-DDS agent.
For farther information how configure the agent click [here]().