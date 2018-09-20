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

import struct
import fcntl
import os
import sys

from rosidl_cmake import convert_camel_case_to_lower_case_underscore
from rosidl_cmake import expand_template
from rosidl_cmake import extract_message_types
from rosidl_cmake import get_newest_modification_time
from rosidl_parser import parse_message_file
from rosidl_parser import parse_service_file
from rosidl_parser import validate_field_types
from shutil import copyfile
from pathlib import Path


def generate_micro_ros_agent_xml_support(args):    
    pkg_name = args['package_name']
    known_msg_types = extract_message_types(
        pkg_name, args['ros_interface_files'], args.get('ros_interface_dependencies', []))

    functions = {
        'get_header_filename_from_msg_name': convert_camel_case_to_lower_case_underscore,
    }


    # Set file format
    file_format = ".xml"
    ros2_prefix = "rt/"


    # Check destination dir
    dest_dir = args['output_dir']
    if not os.path.exists(dest_dir):
        os.makedirs(dest_dir)


    # Check source dir
    srcs_dir = os.path.join(dest_dir, "srcs")
    if not os.path.exists(srcs_dir):
        os.makedirs(srcs_dir)


    # Copy all included xml files
    for filename in os.listdir(args['template_dir']):
        if filename.endswith(file_format): 
            template_src_path = os.path.join(args['template_dir'], filename)
            template_dest_path = os.path.join(srcs_dir, filename)
            if not os.path.isfile(template_dest_path):
                copyfile(template_src_path, template_dest_path)
        
        
    # Iterate throw all msgs/srvs
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


            # Generate source file path
            src_file = os.path.join(srcs_dir, "%s_%s_%s.xml" % (spec.base_type.pkg_name, subfolder, spec.msg_name))


            # Publisher
            file_content  = "   <publisher profile_name=\"%s_%s_%s_p\">\n" % (spec.base_type.pkg_name, subfolder, spec.msg_name)
            file_content += "       <topic profile_name=\"%s_%s_%s_t\">\n" % (spec.base_type.pkg_name, subfolder, spec.msg_name)
            file_content += "           <kind>NO_KEY</kind>\n"
            file_content += "           <name>%s%s_%s_%s</name>\n" % (ros2_prefix, spec.base_type.pkg_name, subfolder, spec.msg_name)
            file_content += "           <dataType>%s::%s::dds_::%s_</dataType>\n" % (spec.base_type.pkg_name, subfolder, spec.msg_name)
            file_content += "           <historyQos>\n"
            file_content += "               <kind>KEEP_LAST</kind>\n"
            file_content += "               <depth>5</depth>\n"
            file_content += "           </historyQos>\n"
            file_content += "           <durability>\n"
            file_content += "               <kind>TRANSIENT_LOCAL</kind>\n"
            file_content += "           </durability>\n"
            file_content += "       </topic>\n"
            file_content += "   </publisher>\n"


            # Subscriber
            file_content += "   <subscriber profile_name=\"%s_%s_%s_s\">\n" % (spec.base_type.pkg_name, subfolder, spec.msg_name)
            file_content += "       <topic profile_name=\"%s_%s_%s_t\">\n" % (spec.base_type.pkg_name, subfolder, spec.msg_name)
            file_content += "           <kind>NO_KEY</kind>\n"
            file_content += "           <name>%s%s_%s_%s</name>\n" % (ros2_prefix, spec.base_type.pkg_name, subfolder, spec.msg_name)
            file_content += "           <dataType>%s::%s::dds_::%s_</dataType>\n" % (spec.base_type.pkg_name, subfolder, spec.msg_name)
            file_content += "           <historyQos>\n"
            file_content += "               <kind>KEEP_LAST</kind>\n"
            file_content += "               <depth>5</depth>\n"
            file_content += "           </historyQos>\n"
            file_content += "           <durability>\n"
            file_content += "               <kind>TRANSIENT_LOCAL</kind>\n"
            file_content += "           </durability>\n"
            file_content += "       </topic>\n"
            file_content += "   </subscriber>\n"


            # Topic
            file_content += "   <topic profile_name=\"%s_%s_%s_t\">\n" % (spec.base_type.pkg_name, subfolder, spec.msg_name)
            file_content += "       <name>%s%s_%s_%s</name>\n" % (ros2_prefix, spec.base_type.pkg_name, subfolder, spec.msg_name)
            file_content += "       <dataType>%s::%s::dds_::%s_</dataType>\n" % (spec.base_type.pkg_name, subfolder, spec.msg_name)
            file_content += "   </topic>\n"


            # Write file content
            fd1 = open(src_file, "w")
            fcntl.lockf(fd1, fcntl.LOCK_EX)
            fd1.write(file_content)
            fd1.close()


            # Open collect file
            collec_file = os.path.join(args['output_dir'], "DEFAULT_FASTRTPS_PROFILES.xml")
            fd2 = open(collec_file, "w")
            fcntl.lockf(fd2, fcntl.LOCK_EX)


            # Generate head
            fd2.write("<profiles>\n")


            # Append all files contents
            for filename in os.listdir(srcs_dir):
                if filename.endswith(".xml"): 
                    fd3 = open(os.path.join(srcs_dir, filename), "r+")
                    fcntl.lockf(fd3, fcntl.LOCK_EX)
                    fd2.write(fd3.read())
                    fd3.close()


            # Generate tail
            fd2.write("</profiles>\n")        


            # Close file
            fd2.close()


        #elif extension == '.srv':

            #data = {'spec': spec}
            #data.update(functions)

            #if not os.path.exists(args['output_dir']):
            #    os.makedirs(args['output_dir'])

            #f = open(os.path.join(args['output_dir'], 'demofile.txt'), 'w+')
            #f.write("%s\n" % spec)
            #f.close()
    return 0