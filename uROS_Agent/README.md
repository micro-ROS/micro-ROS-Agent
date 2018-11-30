# Micro ROS Agent

## Overview

This repository contains the Micro-ROS Agent package.
Micro-ROS Agent is a ROS 2 node that wraps the Micro XRCE-DDS Agent.
For further information about Micro XRCE-DDS Agent click [here](https://github.com/eProsima/Micro-XRCE-DDS-Agent)
This package is a part of the Micro-ROS project stack.
For more information about Micro-ROS project click [here](https://microros.github.io/micro-ROS/).

The node acts as a server between DDS Network and Micro-ROS nodes inside MCU.
It receives and send messages from Micro-ROS nodes, and keep track of the Micro-ROS nodes exposing them to the ROS 2 network.
The node interacts with DDS Global Data Space on behalf of the Micro-ROS nodes.

## Package features

### XML generation

During the build process, the package looks for all ROS 2 messages to generate an initial list of XML profiles.
These profiles can are referenced in the Agent-Client communication to avoid sending the full XML content.
This reference mechanism can be switched on and off from the Micro XRCE-DDS middleware layer.

### Agent-Client communication mechanism

Communication between the Micro-ROS Agent and the Micro-ROS nodes supports two types of transport:

- UDP.
- SerialPort.

All available configurations are supported directly by the Micro XRCE-DDS agent.
For further information on how to configure the agent click [here](TODO).
