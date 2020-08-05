// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "fastrtps/Domain.h"
#include <fastrtps/TopicDataType.h>
#include "fastrtps/attributes/ParticipantAttributes.h"
#include "fastrtps/participant/Participant.h"
#include "fastrtps/participant/ParticipantListener.h"
#include "fastrtps/attributes/PublisherAttributes.h"
#include "fastrtps/publisher/Publisher.h"
#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/rtps/common/MatchingInfo.h>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>

#include "rmw/types.h"
#include "rmw_dds_common/graph_cache.hpp"
#include "rmw_fastrtps_shared_cpp/create_rmw_gid.hpp"

#include "rosidl_typesupport_cpp/message_type_support.hpp"
#include "rosidl_typesupport_fastrtps_cpp/message_type_support.h"
#include "rmw_dds_common/msg/participant_entities_info.hpp"

namespace GraphManager{
  
  using namespace eprosima::fastrtps;
  using namespace eprosima::fastcdr;
  using namespace rmw_dds_common::msg;

  class ParticipantEntitiesInfoTypeSupport: public TopicDataType {
    public:

      ParticipantEntitiesInfoTypeSupport() 
      : TopicDataType()
      {
        m_isGetKeyDefined = false;

        type_support = rosidl_typesupport_cpp::get_message_type_support_handle<ParticipantEntitiesInfo>();
        type_support = get_message_typesupport_handle(type_support, "rosidl_typesupport_fastrtps_cpp");
        callbacks = static_cast<const message_type_support_callbacks_t *>(type_support->data);

        std::ostringstream ss;
        std::string message_namespace(callbacks->message_namespace_);
        std::string message_name(callbacks->message_name_);
        if (!message_namespace.empty()) {
          ss << message_namespace << "::";
        }
        ss << "dds_::" << message_name << "_";
        setName(ss.str().c_str());

        bool max_size_bound_ = true;
        m_typeSize = 4 + callbacks->max_serialized_size(max_size_bound_);
      }

      bool serialize(void *data, rtps::SerializedPayload_t *payload) override
      {   
          FastBuffer fastbuffer(reinterpret_cast<char *>(payload->data), payload->max_size);
          Cdr ser(fastbuffer, Cdr::DEFAULT_ENDIAN, Cdr::DDS_CDR);

          ser.serialize_encapsulation();

          if (callbacks->cdr_serialize(data, ser)) {
            payload->encapsulation = ser.endianness() == Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
            payload->length = (uint32_t) ser.getSerializedDataLength();
            return true;
          }
          return false;
      }

      bool deserialize(rtps::SerializedPayload_t* payload, void* data) override
      {
        FastBuffer fastbuffer(reinterpret_cast<char *>(payload->data), payload->length);
        Cdr deser(fastbuffer, Cdr::DEFAULT_ENDIAN, Cdr::DDS_CDR);

        deser.read_encapsulation();

        return callbacks->cdr_deserialize(deser, data);
      }

      std::function<uint32_t()> getSerializedSizeProvider(void* data) override
      {
          return [data, this]() -> uint32_t
          {
              return (uint32_t) 4 + this->callbacks->get_serialized_size(data);
          };
      }

      void* createData() override
      {
        return (void*)nullptr;
      }

      void deleteData(void* data) override
      {
        (void) data;
      }

      bool getKey(void *data, rtps::InstanceHandle_t* handle, bool force_md5) override
      {
          (void) data;
          (void) handle;
          (void) force_md5;
          return m_isGetKeyDefined;
      }
    
    private:
      const message_type_support_callbacks_t * callbacks;
      const rosidl_message_type_support_t * type_support;
    };

  class GraphManager{
    public:
      GraphManager()
      : enclave("/")
      {     
        uint32_t domain_id = 0;

        ParticipantAttributes participantAttrs;
        Domain::getDefaultParticipantAttributes(participantAttrs);

        size_t length = snprintf(nullptr, 0, "enclave=%s;", enclave) + 1;
        participantAttrs.rtps.userData.resize(length);
        snprintf(reinterpret_cast<char *>(participantAttrs.rtps.userData.data()), length, "enclave=%s;", enclave);
        participantAttrs.rtps.setName(enclave);
        participantAttrs.rtps.builtin.readerHistoryMemoryPolicy = rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
        participantAttrs.rtps.builtin.writerHistoryMemoryPolicy = rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
        participantAttrs.domainId = static_cast<uint32_t>(domain_id);

        participant = Domain::createParticipant(participantAttrs);

        const rosidl_message_type_support_t * type_support = rosidl_typesupport_cpp::get_message_type_support_handle<ParticipantEntitiesInfo>();
        type_support = get_message_typesupport_handle(type_support, "rosidl_typesupport_fastrtps_cpp");
        callbacks = static_cast<const message_type_support_callbacks_t *>(type_support->data);


        m_type = new ParticipantEntitiesInfoTypeSupport();
        eprosima::fastrtps::Domain::registerType(participant,  m_type);

        //CREATE THE PUBLISHER
        PublisherAttributes publisherAttrs;
        Domain::getDefaultPublisherAttributes(publisherAttrs);
        publisherAttrs.topic.topicKind = eprosima::fastrtps::rtps::NO_KEY;
        publisherAttrs.topic.topicDataType = m_type->getName();
        publisherAttrs.topic.topicName = "ros_discovery_info";
        publisherAttrs.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
        publisherAttrs.topic.historyQos.depth = 1;
        publisherAttrs.historyMemoryPolicy = rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
        publisherAttrs.qos.m_publishMode.kind = ASYNCHRONOUS_PUBLISH_MODE;
        publisherAttrs.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
        publisherAttrs.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
        publisher = Domain::createPublisher(participant, publisherAttrs);

        std::cout << "Graph manager init\n"; 

        // eprosima::fastrtps::rtps::GUID_t guid;
        // guid.entityId.value[0] = 12;

        // const rmw_gid_t gid = rmw_fastrtps_shared_cpp::create_rmw_gid("rmw_fastrtps_cpp", guid);
        // graphCache.add_participant(gid, enclave);

        // ParticipantEntitiesInfo info = graphCache.add_node(gid, "test_node_name", "/");
        

        // publisher->write((void*)&info);

        // eprosima::fastrtps::rtps::GUID_t reader_guid;
        // reader_guid.entityId.value[0] = 12;
        // reader_guid.guidPrefix.value[2] = 32;
        // rmw_gid_t reader_gid = rmw_fastrtps_shared_cpp::create_rmw_gid("rmw_fastrtps_cpp", reader_guid);

        // info = graphCache.associate_reader(reader_gid, gid, "test_node_name", "/");

        // reader_guid.guidPrefix.value[2] = 2;
        // reader_gid = rmw_fastrtps_shared_cpp::create_rmw_gid("rmw_fastrtps_cpp", reader_guid);
        // info = graphCache.associate_writer(reader_gid, gid, "test_node_name", "/");

        // publisher->write((void*)&info);


      }

      void add_participant(const eprosima::fastrtps::rtps::GUID_t& guid, void* data)
      {
        const rmw_gid_t gid = rmw_fastrtps_shared_cpp::create_rmw_gid("rmw_fastrtps_cpp", guid);
        graphCache.add_participant(gid, enclave);

        eprosima::fastdds::dds::DomainParticipant* participant = (eprosima::fastdds::dds::DomainParticipant*) data;
        eprosima::fastdds::dds::DomainParticipantQos qos = participant->get_qos();

        ParticipantEntitiesInfo info = graphCache.add_node(gid, qos.name().c_str(), "/");
        
        std::cout << "adding_participant " << qos.name().c_str() << "in graph manager\n";

        publisher->write((void*)&info);

        std::cout << graphCache;
      }

      void add_datawriter(const eprosima::fastrtps::rtps::GUID_t& guid, void* data)
      {
        const rmw_gid_t reader_gid = rmw_fastrtps_shared_cpp::create_rmw_gid("rmw_fastrtps_cpp", guid);


        eprosima::fastdds::dds::DomainParticipant* participant = (eprosima::fastdds::dds::DomainParticipant*) data;
        eprosima::fastdds::dds::DomainParticipantQos qos = participant->get_qos();

        const rmw_gid_t gid = rmw_fastrtps_shared_cpp::create_rmw_gid("rmw_fastrtps_cpp", participant->guid());

        ParticipantEntitiesInfo info = graphCache.associate_writer(reader_gid, gid,  qos.name().c_str(), "/");
        
        std::cout << "add_datawriter " << qos.name().c_str() << "in graph manager\n";

        publisher->write((void*)&info);

        std::cout << graphCache;
      }

    private:
      const char * enclave;
      ParticipantEntitiesInfoTypeSupport * m_type;
      Participant * participant;
      Publisher * publisher;
      const message_type_support_callbacks_t * callbacks;
      rmw_dds_common::GraphCache graphCache;
};
}
