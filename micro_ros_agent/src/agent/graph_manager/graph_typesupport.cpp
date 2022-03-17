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

#ifndef UROS_AGENT_GRAPH_PARTICIPANTS_TYPESUPPORT_CPP_
#define UROS_AGENT_GRAPH_PARTICIPANTS_TYPESUPPORT_CPP_

#include <agent/graph_manager/graph_typesupport.hpp>

#include <string>

namespace uros {
namespace agent {
namespace graph_manager {

ParticipantEntitiesInfoTypeSupport::ParticipantEntitiesInfoTypeSupport()
    : TopicDataType()
{
    type_support_ = rosidl_typesupport_cpp::get_message_type_support_handle<
        rmw_dds_common::msg::ParticipantEntitiesInfo>();
    type_support_ = get_message_typesupport_handle(type_support_,
        "rosidl_typesupport_fastrtps_cpp");
    callbacks_ = static_cast<const message_type_support_callbacks_t *>(type_support_->data);

    std::ostringstream ss;
    const std::string message_namespace(callbacks_->message_namespace_);
    const std::string message_name(callbacks_->message_name_);

    if (!message_namespace.empty())
    {
        ss << message_namespace << "::";
    }
    ss << "dds_::" << message_name << "_";
    this->setName(ss.str().c_str());

    char full_bounded;
    m_typeSize = 4 + callbacks_->max_serialized_size(full_bounded);
}

bool ParticipantEntitiesInfoTypeSupport::serialize(
        void * data,
        eprosima::fastrtps::rtps::SerializedPayload_t * payload)
{
    eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char *>(payload->data),
        payload->max_size);
    eprosima::fastcdr::Cdr scdr(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
        eprosima::fastcdr::Cdr::DDS_CDR);

    scdr.serialize_encapsulation();
    if (callbacks_->cdr_serialize(data, scdr))
    {
        payload->encapsulation = (scdr.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS) ?
            CDR_BE : CDR_LE;
        payload->length = static_cast<uint32_t>(scdr.getSerializedDataLength());
        return true;
    }
    else
    {
        return false;
    }
}

bool ParticipantEntitiesInfoTypeSupport::deserialize(
        eprosima::fastrtps::rtps::SerializedPayload_t * payload,
        void * data)
{
    eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char *>(payload->data),
        payload->length);
    eprosima::fastcdr::Cdr dcdr(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
        eprosima::fastcdr::Cdr::DDS_CDR);

    dcdr.read_encapsulation();
    return callbacks_->cdr_deserialize(dcdr, data);
}

std::function<uint32_t()> ParticipantEntitiesInfoTypeSupport::getSerializedSizeProvider(
        void * data)
{
    return [data, this]() -> uint32_t
    {
        return static_cast<uint32_t>(4 + callbacks_->get_serialized_size(data));
    };
}

void * ParticipantEntitiesInfoTypeSupport::createData()
{
    return static_cast<void *>(nullptr);
}

void ParticipantEntitiesInfoTypeSupport::deleteData(
        void * data)
{
    (void) data;
}

bool ParticipantEntitiesInfoTypeSupport::getKey(
        void * data,
        eprosima::fastrtps::rtps::InstanceHandle_t * handle,
        bool force_md5)
{
    (void) data;
    (void) handle;
    (void) force_md5;
    return m_isGetKeyDefined;
}


MicrorosGraphInfoTypeSupport::MicrorosGraphInfoTypeSupport()
    : TopicDataType()
{
    type_support_ = rosidl_typesupport_cpp::get_message_type_support_handle<
        micro_ros_msgs::msg::Graph>();
    type_support_ = get_message_typesupport_handle(type_support_,
        "rosidl_typesupport_fastrtps_cpp");
    callbacks_ = static_cast<const message_type_support_callbacks_t *>(type_support_->data);

    std::ostringstream ss;
    const std::string message_namespace(callbacks_->message_namespace_);
    const std::string message_name(callbacks_->message_name_);

    if (!message_namespace.empty())
    {
        ss << message_namespace << "::";
    }
    ss << "dds_::" << message_name << "_";
    this->setName(ss.str().c_str());

    char full_bounded;
    m_typeSize = 4 + callbacks_->max_serialized_size(full_bounded);
}

bool MicrorosGraphInfoTypeSupport::serialize(
        void * data,
        eprosima::fastrtps::rtps::SerializedPayload_t * payload)
{
    eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char *>(payload->data),
        payload->max_size);
    eprosima::fastcdr::Cdr scdr(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
        eprosima::fastcdr::Cdr::DDS_CDR);

    scdr.serialize_encapsulation();
    if (callbacks_->cdr_serialize(data, scdr))
    {
        payload->encapsulation = (scdr.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS) ?
            CDR_BE : CDR_LE;
        payload->length = static_cast<uint32_t>(scdr.getSerializedDataLength());
        return true;
    }
    else
    {
        return false;
    }
}

bool MicrorosGraphInfoTypeSupport::deserialize(
        eprosima::fastrtps::rtps::SerializedPayload_t * payload,
        void * data)
{
    eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char *>(payload->data),
        payload->length);
    eprosima::fastcdr::Cdr dcdr(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
        eprosima::fastcdr::Cdr::DDS_CDR);

    dcdr.read_encapsulation();
    return callbacks_->cdr_deserialize(dcdr, data);
}

std::function<uint32_t()> MicrorosGraphInfoTypeSupport::getSerializedSizeProvider(
        void * data)
{
    return [data, this]() -> uint32_t
    {
        return static_cast<uint32_t>(4 + callbacks_->get_serialized_size(data));
    };
}

void * MicrorosGraphInfoTypeSupport::createData()
{
    return static_cast<void *>(nullptr);
}

void MicrorosGraphInfoTypeSupport::deleteData(
        void * data)
{
    (void) data;
}

bool MicrorosGraphInfoTypeSupport::getKey(
        void * data,
        eprosima::fastrtps::rtps::InstanceHandle_t * handle,
        bool force_md5)
{
    (void) data;
    (void) handle;
    (void) force_md5;
    return m_isGetKeyDefined;
}

}  // namespace graph_manager
}  // namespace agent
}  // namespace uros

#endif  // UROS_AGENT_GRAPH_PARTICIPANTS_TYPESUPPORT_CPP_