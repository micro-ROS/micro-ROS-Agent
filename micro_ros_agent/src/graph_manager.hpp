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

#include "rmw_dds_common/graph_cache.hpp"

#include "rosidl_typesupport_cpp/message_type_support.hpp"
#include "rosidl_typesupport_fastrtps_cpp/message_type_support.h"
#include "rmw_dds_common/msg/participant_entities_info.hpp"


namespace graphmanager{
  
  using namespace eprosima::fastrtps;

  class GraphType: public TopicDataType {
    public:

      GraphType() 
      : TopicDataType(){
        m_typeSize = 1024 + 4 /*encapsulation*/;
        m_isGetKeyDefined = false;
      }

      bool serialize(void *data, rtps::SerializedPayload_t *payload) override
      {
          bool rv = false;
          std::vector<unsigned char>* buffer = reinterpret_cast<std::vector<unsigned char>*>(data);
          payload->data[0] = 0;
          payload->data[1] = 1;
          payload->data[2] = 0;
          payload->data[3] = 0;
          if (buffer->size() <= (payload->max_size - 4))
          {
              memcpy(&payload->data[4], buffer->data(), buffer->size());
              payload->length = uint32_t(buffer->size() + 4); //Get the serialized length
              rv = true;
          }
          return rv;
      }

      bool deserialize(rtps::SerializedPayload_t* payload, void* data) override
      {
          std::vector<unsigned char>* buffer = reinterpret_cast<std::vector<unsigned char>*>(data);
          buffer->assign(payload->data + 4, payload->data + payload->length);

          return true;
      }

      std::function<uint32_t()> getSerializedSizeProvider(void* data) override
      {
          return [data]() -> uint32_t
          {
              return (uint32_t)reinterpret_cast<std::vector<unsigned char>*>(data)->size() + 4 /*encapsulation*/;
          };
      }

      void* createData() override
      {
          return (void*)new std::vector<unsigned char>;
      }

      void deleteData(void* data) override
      {
          delete((std::vector<unsigned char>*)data);
      }

      bool getKey(void *data, rtps::InstanceHandle_t* handle, bool force_md5) override
      {
          // TODO.
          (void) data;
          (void) handle;
          (void) force_md5;
          return m_isGetKeyDefined;
      }
    private:
      message_type_support_callbacks_t _callbacks;
    };

    class GraphManager{
    public:
      GraphManager()
      {     
        const char * enclave = "/";
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
        type_support = get_message_type_support_handle(type_support, "rosidl_typesupport_fastrtps_cpp");
        callbacks = static_cast<const message_type_support_callbacks_t *>(type_support->data);

        std::ostringstream ss;
        std::string message_namespace(callbacks->message_namespace_);
        std::string message_name(callbacks->message_name_);
        if (!message_namespace.empty()) {
          ss << message_namespace << "::";
        }
        ss << "dds_::" << message_name << "_";

        m_type = new GraphType();

        eprosima::fastrtps::Domain::registerType(participant,  m_type);

        //CREATE THE PUBLISHER
        PublisherAttributes publisherAttrs;
        publisherAttrs.topic.topicKind = eprosima::fastrtps::rtps::NO_KEY;
        publisherAttrs.topic.topicDataType = ss.str();
        publisherAttrs.topic.topicName = "ros_discovery_info";
        publisherAttrs.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
        publisherAttrs.topic.historyQos.depth = 30;
        publisherAttrs.topic.resourceLimitsQos.max_samples = 50;
        publisherAttrs.topic.resourceLimitsQos.allocated_samples = 20;
        publisherAttrs.times.heartbeatPeriod.seconds = 2;
        publisherAttrs.times.heartbeatPeriod.nanosec = 200 * 1000 * 1000;
        publisherAttrs.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
        publisher = Domain::createPublisher(participant, publisherAttrs, (PublisherListener*)&m_listener);

        std::cout << "Graph manager init\n"; 
      }

      void start(){

      }

    private:
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

    //   void timer_callback()
    //   {
    //     auto message = std_msgs::msg::String();
    //     message.data = "Hello, world! " + std::to_string(count_++);
    //     RCLCPP_INFO(this->get_logger(), "Publishing: '%s'", message.data.c_str());
    //     publisher_->publish(message);
    //   }

    //   rclcpp::TimerBase::SharedPtr timer_;
    //   rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
    //   size_t count_;

  };
}
