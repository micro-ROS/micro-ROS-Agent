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
#include "rmw/names_and_types.h"
#include "rmw/impl/cpp/key_value.hpp"
#include "rmw_dds_common/graph_cache.hpp"
#include "rmw_fastrtps_shared_cpp/create_rmw_gid.hpp"
#include "rmw_fastrtps_shared_cpp/qos.hpp"

#include "rcutils/types.h"
#include "rcutils/types/string_array.h"

#include "rosidl_typesupport_cpp/message_type_support.hpp"
#include "rosidl_typesupport_fastrtps_cpp/message_type_support.h"

#include "rmw_dds_common/msg/participant_entities_info.hpp"
#include "micro_ros_agent_msgs/msg/graph.hpp"
#include "micro_ros_agent_msgs/msg/node.hpp"
#include "micro_ros_agent_msgs/msg/entity.hpp"

#include "demangle.hpp"

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

        graphCache.set_on_change_callback([this](){ this->updated_graph_callback(); });
        std::cout << "Graph manager init\n"; 
        microros_graph_publisher = std::thread(&GraphManager::publish_microros_graph, this);

        graph_changed = false;
    }

    void updated_graph_callback()
    {
        std::cout << "Updated Graph\n";
        std::unique_lock<std::mutex> lock(mtx);
        graph_changed = true;
        cv.notify_one();
    }

    void publish_microros_graph()
    {   
        while(true) {
          {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this]{ return this->graph_changed; });
          }
          if(graph_changed){
            std::cout << "Updated uros Graph\n";
            std::cout << graphCache;
            graph_changed = false;

            micro_ros_agent_msgs::msg::Graph graph_message;


            rcutils_string_array_t node_names = rcutils_get_zero_initialized_string_array();
            rcutils_string_array_t node_namespaces = rcutils_get_zero_initialized_string_array();
            rcutils_allocator_t allocator = rcutils_get_default_allocator();
            graphCache.get_node_names(&node_names, &node_namespaces, nullptr, &allocator);

            for (size_t i = 0; i < node_names.size; i++)
            {
              rmw_names_and_types_t rmw_names_and_types = rmw_get_zero_initialized_names_and_types();
              const std::string node_name(node_names.data[i]);
              const std::string node_namespace(node_namespaces.data[i]);
              std::cout << node_name << "\n";

              using DemangleFunction = std::string (*)(const std::string &);

              DemangleFunction demangle_topic = _demangle_ros_topic_from_topic;
              DemangleFunction demangle_type = _demangle_if_ros_type;

              rmw_ret_t ret = graphCache.get_writer_names_and_types_by_node(  node_name, 
                                                              node_namespace,
                                                              demangle_topic,
                                                              demangle_type,
                                                              &allocator,
                                                              &rmw_names_and_types
                                                            );

              std::cout << rmw_names_and_types.names.size << " reeet \n";


              for (size_t i = 0; i < rmw_names_and_types.names.size; i++)
              {
                std::cout << rmw_names_and_types.names.data[i] << "\n";
              }
            }
          }
        }
    }

    void add_participant(const eprosima::fastrtps::rtps::GUID_t& guid, void* participant)
    {
        const rmw_gid_t gid = rmw_fastrtps_shared_cpp::create_rmw_gid("rmw_fastrtps_cpp", guid);
        const eprosima::fastdds::dds::DomainParticipant * _participant = (eprosima::fastdds::dds::DomainParticipant *) participant;

        eprosima::fastdds::dds::DomainParticipantQos qos = _participant->get_qos();

        graphCache.add_participant(gid, enclave);
        ParticipantEntitiesInfo info = graphCache.add_node(gid, qos.name().c_str(), "/");

        publisher->write((void*)&info);
    }

    void associate_entity(const eprosima::fastrtps::rtps::GUID_t& guid, void* participant, bool is_reader)
    {
        const rmw_gid_t gid = rmw_fastrtps_shared_cpp::create_rmw_gid("rmw_fastrtps_cpp", guid);
        const eprosima::fastdds::dds::DomainParticipant * _participant = (eprosima::fastdds::dds::DomainParticipant *) participant;
        const rmw_gid_t participant_gid = rmw_fastrtps_shared_cpp::create_rmw_gid("rmw_fastrtps_cpp", _participant->guid());

        eprosima::fastdds::dds::DomainParticipantQos qos = _participant->get_qos();

        ParticipantEntitiesInfo info;
        if (is_reader) {
            info = graphCache.associate_reader(gid, participant_gid,  qos.name().c_str(), "/");
        } else {
            info = graphCache.associate_writer(gid, participant_gid,  qos.name().c_str(), "/");
        }
        
        publisher->write((void*)&info);
    }

    private:
      const char * enclave;
      ParticipantEntitiesInfoTypeSupport * m_type;
      Participant * participant;
      Publisher * publisher;
      const message_type_support_callbacks_t * callbacks;
      rmw_dds_common::GraphCache graphCache;

      bool graph_changed;
      std::thread microros_graph_publisher;
      std::mutex mtx;
      std::condition_variable cv;
};

class ParticipantListener : public eprosima::fastrtps::ParticipantListener
{
public:
    ParticipantListener(rmw_dds_common::GraphCache& graphCache) 
    : _graphCache(graphCache)
    {
    }
  void onParticipantDiscovery(
    eprosima::fastrtps::Participant *,
    eprosima::fastrtps::rtps::ParticipantDiscoveryInfo && info) override
  {
    switch (info.status) {
      case eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT:
        {
          auto map = rmw::impl::cpp::parse_key_value(info.info.m_userData);
          auto name_found = map.find("enclave");

          if (name_found == map.end()) {
            return;
          }
          auto enclave =
            std::string(name_found->second.begin(), name_found->second.end());

          _graphCache.add_participant(
            rmw_fastrtps_shared_cpp::create_rmw_gid(
              "rmw_fastrtps_cpp", info.info.m_guid),
            enclave);
          break;
        }
      case eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::REMOVED_PARTICIPANT:
      // fall through
      case eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DROPPED_PARTICIPANT:
        _graphCache.remove_participant(
          rmw_fastrtps_shared_cpp::create_rmw_gid(
            "rmw_fastrtps_cpp", info.info.m_guid));
        break;
      default:
        return;
    }
  }

  void onSubscriberDiscovery(
    eprosima::fastrtps::Participant *,
    eprosima::fastrtps::rtps::ReaderDiscoveryInfo && info) override
  {
    if (eprosima::fastrtps::rtps::ReaderDiscoveryInfo::CHANGED_QOS_READER != info.status) {
      bool is_alive =
        eprosima::fastrtps::rtps::ReaderDiscoveryInfo::DISCOVERED_READER == info.status;
      process_discovery_info(info.info, is_alive, true);
    }
  }

  void onPublisherDiscovery(
    eprosima::fastrtps::Participant *,
    eprosima::fastrtps::rtps::WriterDiscoveryInfo && info) override
  {
    if (eprosima::fastrtps::rtps::WriterDiscoveryInfo::CHANGED_QOS_WRITER != info.status) {
      bool is_alive =
        eprosima::fastrtps::rtps::WriterDiscoveryInfo::DISCOVERED_WRITER == info.status;
      process_discovery_info(info.info, is_alive, false);
    }
  }

private:
  template<class T>
  void
  process_discovery_info(T & proxyData, bool is_alive, bool is_reader)
  {
    {
      if (is_alive) {
        rmw_qos_profile_t qos_profile = rmw_qos_profile_unknown;
        dds_qos_to_rmw_qos(proxyData.m_qos, &qos_profile);

        _graphCache.add_entity(
          rmw_fastrtps_shared_cpp::create_rmw_gid(
            "rmw_fastrtps_cpp",
            proxyData.guid()),
          proxyData.topicName().to_string(),
          proxyData.typeName().to_string(),
          rmw_fastrtps_shared_cpp::create_rmw_gid(
            "rmw_fastrtps_cpp",
            iHandle2GUID(proxyData.RTPSParticipantKey())),
          qos_profile,
          is_reader);
      } else {
        _graphCache.remove_entity(
          rmw_fastrtps_shared_cpp::create_rmw_gid(
            "rmw_fastrtps_cpp",
            proxyData.guid()),
          is_reader);
      }
    }
  }

  rmw_dds_common::GraphCache& _graphCache;
};
}
