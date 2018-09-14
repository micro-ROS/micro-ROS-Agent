# Copyright 2016 Open Source Robotics Foundation, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os

from rosidl_cmake import convert_camel_case_to_lower_case_underscore
from rosidl_cmake import expand_template
from rosidl_cmake import extract_message_types
from rosidl_cmake import get_newest_modification_time
from rosidl_parser import parse_message_file
from rosidl_parser import parse_service_file
from rosidl_parser import validate_field_types
from pathlib import Path


def generate_micro_ros_agent_xml_support(args):    
    pkg_name = args['package_name']
    known_msg_types = extract_message_types(
        pkg_name, args['ros_interface_files'], args.get('ros_interface_dependencies', []))

    functions = {
        'get_header_filename_from_msg_name': convert_camel_case_to_lower_case_underscore,
    }

    for idl_file in args['ros_interface_files']:
        extension = os.path.splitext(idl_file)[1]
        if extension == '.msg':
            spec = parse_message_file(pkg_name, idl_file)
            validate_field_types(spec, known_msg_types)
            subfolder = os.path.basename(os.path.dirname(idl_file))

            data = {
                'spec': spec,
                'pkg': spec.base_type.pkg_name,
                'msg': spec.msg_name,
                'type': spec.base_type.type,
                'subfolder': subfolder,
            }
            data.update(functions)


            # Make destinatino dir
            if not os.path.exists(args['output_dir']):
                os.makedirs(args['output_dir'])

            # Check if publixher exists
            pub_file_path = os.path.join(args['output_dir'], 'publisher.xml')
            pub_file = Path(pub_file_path)
            if not pub_file.is_file():
                publ = open(pub_file_path, 'a')
                publ.write("<profiles>\n")
            else:
                publ = open(pub_file_path)
                lines = publ.readlines()
                publ.close()
                publ = open(pub_file_path,'w')
                publ.writelines([item for item in lines[:-1]])
                #publ = open(pub_file_path, "w")
                #publ.seek(publ.seek(-1,2) - len("</profiles>\n"))



            publ.write("   <publisher profile_name=\"default_xrce_publisher_profile\">\n")
            publ.write("       <topic>\n")
            publ.write("           <kind>NO_KEY</kind>\n")
            publ.write("           <name>%sPubSubTopic</name>\n" % (spec.msg_name))
            publ.write("           <dataType>%s::%s::dds_::%s_</dataType>\n" % (spec.base_type.pkg_name, subfolder, spec.msg_name))
            publ.write("           <historyQos>\n")
            publ.write("               <kind>KEEP_LAST</kind>\n")
            publ.write("               <depth>5</depth>\n")
            publ.write("           </historyQos>\n")
            publ.write("           <durability>\n")
            publ.write("               <kind>TRANSIENT_LOCAL</kind>\n")
            publ.write("           </durability>\n")
            publ.write("       </topic>\n")
            publ.write("   </publisher>\n")
            publ.write("</profiles>\n")
            publ.close()



            # Check if subcriber exists
            subs_file_path = os.path.join(args['output_dir'], 'subscriber.xml')
            subs_file = Path(subs_file_path)
            if not subs_file.is_file():
                subs = open(subs_file_path, 'a')
                subs.write("<profiles>\n")
            else:
                subs = open(subs_file_path)
                lines = subs.readlines()
                subs.close()
                subs = open(subs_file_path,'w')
                subs.writelines([item for item in lines[:-1]])


            subs.write("   <subscriber profile_name=\"default_xrce_subscriber_profile\">\n")
            subs.write("       <topic>\n")
            subs.write("           <kind>NO_KEY</kind>\n")
            subs.write("           <name>%sPubSubTopic</name>\n" % (spec.msg_name))
            subs.write("           <dataType>%s::%s::dds_::%s_</dataType>\n" % (spec.base_type.pkg_name, subfolder, spec.msg_name))
            subs.write("           <historyQos>\n")
            subs.write("               <kind>KEEP_LAST</kind>\n")
            subs.write("               <depth>5</depth>\n")
            subs.write("           </historyQos>\n")
            subs.write("           <durability>\n")
            subs.write("               <kind>TRANSIENT_LOCAL</kind>\n")
            subs.write("           </durability>\n")
            subs.write("       </topic>\n")
            subs.write("   </subscriber>\n")
            subs.write("</profiles>\n")
            subs.close()


            # Check if topic exists
            topi_file_path = os.path.join(args['output_dir'], 'topic.xml')
            topi_file = Path(topi_file_path)
            if not topi_file.is_file():
                topi = open(topi_file_path, 'a')
                topi.write("<dds>\n")
            else:
                topi = open(topi_file_path)
                lines = topi.readlines()
                topi.close()
                topi = open(topi_file_path,'w')
                topi.writelines([item for item in lines[:-1]])

            topi.write("   <topic>\n")
            topi.write("       <name>%sPubSubTopic</name>\n" % (spec.msg_name))
            topi.write("       <dataType>%s::%s::dds_::%s_</dataType>\n" % (spec.base_type.pkg_name, subfolder, spec.msg_name))
            topi.write("   </topic>\n")
            topi.write("</dds>\n")
            topi.close() 

        #elif extension == '.srv':

            #data = {'spec': spec}
            #data.update(functions)

            #if not os.path.exists(args['output_dir']):
            #    os.makedirs(args['output_dir'])

            #f = open(os.path.join(args['output_dir'], 'demofile.txt'), 'w+')
            #f.write("%s\n" % spec)
            #f.close()
    return 0
