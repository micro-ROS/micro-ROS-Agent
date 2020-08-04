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

#include "rmw/types.h"
#include "rmw_dds_common/graph_cache.hpp"

#include "rosidl_typesupport_cpp/message_type_support.hpp"
#include "rosidl_typesupport_fastrtps_cpp/message_type_support.h"
#include "rmw_dds_common/msg/participant_entities_info.hpp"

#define RMW_GID_STORAGE_SIZE 24u

namespace graphmanager{
  
  using namespace eprosima::fastrtps;
  using namespace eprosima::fastcdr;

  class GraphType: public TopicDataType {
    public:

      GraphType() 
      : TopicDataType(){
        m_isGetKeyDefined = false;

        type_support = rosidl_typesupport_cpp::get_message_type_support_handle<rmw_dds_common::msg::ParticipantEntitiesInfo>();
        type_support = get_message_typesupport_handle(type_support, "rosidl_typesupport_fastrtps_cpp");
        callbacks = static_cast<const message_type_support_callbacks_t *>(type_support->data);

        bool max_size_bound_ = false;
        m_typeSize = 4 + callbacks->max_serialized_size(max_size_bound_);
        m_typeSize=2000;
        std::cout << "max size: " << m_typeSize << "\n";
      }

      bool serialize(void *data, rtps::SerializedPayload_t *payload) override
      {   

          std::cout << "Call to serialize:\n";

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
        std::cout << "Call to deserialize:\n";

        FastBuffer fastbuffer(reinterpret_cast<char *>(payload->data), payload->length);
        Cdr deser(fastbuffer, Cdr::DEFAULT_ENDIAN, Cdr::DDS_CDR);

        deser.read_encapsulation();

        return callbacks->cdr_deserialize(deser, data);
      }

      std::function<uint32_t()> getSerializedSizeProvider(void* data) override
      {
        std::cout << "Call to getSerializedSizeProvider:\n";
          return [data, this]() -> uint32_t
          {
              std::cout << "Call to getSerializedSizeProvider inside:"<< (4 + this->callbacks->get_serialized_size(data)) << "\n";
              return (uint32_t) 4 + this->callbacks->get_serialized_size(data);
;
          };
      }

      void* createData() override
      {
                std::cout << "Call to createData:\n";

          return (void*)new std::vector<unsigned char>;
      }

      void deleteData(void* data) override
      {
                        std::cout << "Call to deleteData:\n";

          delete((std::vector<unsigned char>*)data);
      }

      bool getKey(void *data, rtps::InstanceHandle_t* handle, bool force_md5) override
      {
                                std::cout << "Call to getKey:\n";

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
      GraphManager(): enclave("/")
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

        const rosidl_message_type_support_t * type_support = rosidl_typesupport_cpp::get_message_type_support_handle<rmw_dds_common::msg::ParticipantEntitiesInfo>();
        type_support = get_message_typesupport_handle(type_support, "rosidl_typesupport_fastrtps_cpp");
        callbacks = static_cast<const message_type_support_callbacks_t *>(type_support->data);

        std::ostringstream ss;
        std::string message_namespace(callbacks->message_namespace_);
        std::string message_name(callbacks->message_name_);
        if (!message_namespace.empty()) {
          ss << message_namespace << "::";
        }
        ss << "dds_::" << message_name << "_";

        m_type = new GraphType();
        m_type->setName(ss.str().c_str());
        eprosima::fastrtps::Domain::registerType(participant,  m_type);

        //CREATE THE PUBLISHER
        PublisherAttributes publisherAttrs;
        Domain::getDefaultPublisherAttributes(publisherAttrs);
        publisherAttrs.topic.topicKind = eprosima::fastrtps::rtps::NO_KEY;
        publisherAttrs.topic.topicDataType = ss.str();
        publisherAttrs.topic.topicName = "ros_discovery_info";
        publisherAttrs.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
        publisherAttrs.topic.historyQos.depth = 1;
        publisherAttrs.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
        publisherAttrs.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
        publisher = Domain::createPublisher(participant, publisherAttrs, (PublisherListener*)&m_listener);

        std::cout << "Graph manager init\n"; 
      }

      void add_participant(const dds::GUID_t& guid)
      {
        const rmw_gid_t gid = create_rmw_gid("rmw_fastrtps_cpp", guid);
        graphCache.add_participant(gid, enclave);
        rmw_dds_common::msg::ParticipantEntitiesInfo info = graphCache.add_node(gid, "testnodeeeeeeee", "/");
        std::cout << "adding_participant in graph manager\n";

        publisher->write((void*)&info);

      }

      rmw_gid_t create_rmw_gid(
        const char * identifier, const dds::GUID_t& guid)
      {
        rmw_gid_t rmw_gid = {};
        rmw_gid.implementation_identifier = identifier;
        static_assert(
          sizeof(eprosima::fastrtps::rtps::GUID_t) <= RMW_GID_STORAGE_SIZE,
          "RMW_GID_STORAGE_SIZE insufficient to store the fastrtps GUID_t."
        );
        copy_from_fastrtps_guid_to_byte_array(
          guid,
          rmw_gid.data);
        return rmw_gid;
      }

      template<typename ByteT>
      void copy_from_fastrtps_guid_to_byte_array(
        const dds::GUID_t& guid,
        ByteT * guid_byte_array)
      {
        static_assert(
          std::is_same<uint8_t, ByteT>::value || std::is_same<int8_t, ByteT>::value,
          "ByteT should be either int8_t or uint8_t");
        assert(guid_byte_array);
        constexpr auto prefix_size = sizeof(guid.guidPrefix());
        memcpy(guid_byte_array, &guid.guidPrefix(), prefix_size);
        // memcpy(&guid_byte_array[prefix_size], &guid.entityId(), guid.entityId().size);
        memcpy(&guid_byte_array[prefix_size], &guid.entityId(), 4);
      }

    private:
      const char * enclave;
      GraphType * m_type;
      Participant * participant;
      Publisher * publisher;
      const message_type_support_callbacks_t * callbacks;
      rmw_dds_common::GraphCache graphCache;

      class PubListener:public eprosima::fastrtps::PublisherListener
      {
      public:
        PubListener():n_matched(0),firstConnected(false){};
        ~PubListener(){};
        int n_matched;
        bool firstConnected;

        void onPublicationMatched(
            Publisher* /*pub*/,
            rtps::MatchingInfo& info)
        {
            if (info.status == rtps::MATCHED_MATCHING)
            {
                n_matched++;
                firstConnected = true;
                std::cout << "Publisher matched" << std::endl;
            }
            else
            {
                n_matched--;
                std::cout << "Publisher unmatched" << std::endl;
            }
        }
      }m_listener;
  };
}
