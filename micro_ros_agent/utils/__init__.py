# Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
import sys
import xml.etree.ElementTree

from rosidl_cmake import convert_camel_case_to_lower_case_underscore
from rosidl_cmake import expand_template
from rosidl_cmake import get_newest_modification_time
from rosidl_adapter.parser import parse_message_file
from rosidl_adapter.parser import parse_service_file
from rosidl_adapter.parser import validate_field_types

from rosidl_cmake import generate_files

def GetPackage(Dir):
    
    # ignore?
    for f in os.listdir(Dir):
        full_path = os.path.join(Dir, f)
        if os.path.isfile(full_path):
            if f == "COLCON_IGNORE":
                return "COLCON_IGNORE"

    # found package
    found_package_path = ""
    for f in os.listdir(Dir):
        if f == "package.xml":
            found_package_path = os.path.join(Dir, f)
            found_package_path = os.path.abspath(found_package_path)
            break


    return found_package_path.replace("\\", "/")


def GetPackageList(Dir):
    package_list = []
    package_path = GetPackage(Dir)

    if package_path == "":
        for f in os.listdir(Dir):
            full_path = os.path.join(Dir, f)
            if os.path.isdir(full_path):
                for l in GetPackageList(full_path):
                    package_list.append(l)
    elif package_path != "COLCON_IGNORE":
        package_list.append(package_path)

    return package_list


def GetInterfacePackages(packages_list):
    package_interface_list = []
    for package in packages_list:
        xml_root = xml.etree.ElementTree.parse(package).getroot()
        for element in xml_root.findall('member_of_group'):
            if element.text == "rosidl_interface_packages":
                package_interface_list.append(package)
    return package_interface_list


def GetPackageName(package_path):
    xml_root = xml.etree.ElementTree.parse(package_path).getroot()
    xml_name_elements = xml_root.findall('name')
    if len(xml_name_elements) != 1:
        return ""
    else:
        return xml_name_elements[0].text


def GetInterfacePackageMsgs(package_path):
    msg_list = []
    package_dir = os.path.dirname(package_path)
    for root, dirs, files in os.walk(package_dir):
        for file in files:
            if file.endswith(".msg"):
                full_path = os.path.join(root, file)
                msg_list.append(full_path.replace("\\", "/"))

    return msg_list

def GetInterfacePackageSrvs(package_path):
    msg_list = []
    package_dir = os.path.dirname(package_path)
    for root, dirs, files in os.walk(package_dir):
        for file in files:
            if file.endswith(".srv"):
                full_path = os.path.join(root, file)
                msg_list.append(full_path.replace("\\", "/"))

    return msg_list


def ReadDefaultXMLs(Path):
    for f in os.listdir(Path):
        full_path = os.path.join(Path, f)
        if os.path.isfile(full_path) and f.endswith(".xml"):
            #xml_root = xml.etree.ElementTree.parse(full_path).getroot()
            #if xml_root.iselement():
            fd = open(full_path)
            print ("%s" % fd.read())
            fd.close()



def generate_XML(args):    
    pkg_name = args['package_name']
    #known_msg_types = extract_message_types(
    #    pkg_name, args['ros_interface_files'], args.get('ros_interface_dependencies', []))

    functions = {
        'get_header_filename_from_msg_name': convert_camel_case_to_lower_case_underscore,
    }


    # Set file format
    ros2_prefix = "rt/"


    # Check destination dir
    dest_dir = args['output_dir']
    if not os.path.exists(dest_dir):
        os.makedirs(dest_dir)


    # Check source dir
    srcs_dir = os.path.join(dest_dir, "srcs")
    if not os.path.exists(srcs_dir):
        os.makedirs(srcs_dir)

        
        
    # Iterate throw all msgs/srvs
    for idl_file in args['ros_interface_files']:
        extension = os.path.splitext(idl_file)[1]
        if extension == '.msg':
            spec = parse_message_file(pkg_name, idl_file)
            #validate_field_types(spec, known_msg_types)
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

            # Data writer
            file_content  = "   <dds>\n"
            file_content += "       <data_writer profile_name=\"%s_%s_%s_p\">\n" % (spec.base_type.pkg_name, subfolder, spec.msg_name)
            file_content += "           <topic profile_name=\"%s_%s_%s_t\">\n" % (spec.base_type.pkg_name, subfolder, spec.msg_name)
            file_content += "               <kind>NO_KEY</kind>\n"
            file_content += "               <name>%s%s_%s_%s</name>\n" % (ros2_prefix, spec.base_type.pkg_name, subfolder, spec.msg_name)
            file_content += "               <dataType>%s::%s::dds_::%s_</dataType>\n" % (spec.base_type.pkg_name, subfolder, spec.msg_name)
            file_content += "           </topic>\n"
            file_content += "       </data_writer>\n"
            file_content += "   </dds>\n"


            # Data reader
            file_content += "   <dds>\n"
            file_content += "       <data_reader profile_name=\"%s_%s_%s_s\">\n" % (spec.base_type.pkg_name, subfolder, spec.msg_name)
            file_content += "           <topic profile_name=\"%s_%s_%s_t\">\n" % (spec.base_type.pkg_name, subfolder, spec.msg_name)
            file_content += "               <kind>NO_KEY</kind>\n"
            file_content += "               <name>%s%s_%s_%s</name>\n" % (ros2_prefix, spec.base_type.pkg_name, subfolder, spec.msg_name)
            file_content += "               <dataType>%s::%s::dds_::%s_</dataType>\n" % (spec.base_type.pkg_name, subfolder, spec.msg_name)
            file_content += "           </topic>\n"
            file_content += "       </data_reader>\n"
            file_content += "   </dds>\n"


            # Topic
            file_content += "   <dds>\n"
            file_content += "       <topic profile_name=\"%s_%s_%s_t\">\n" % (spec.base_type.pkg_name, subfolder, spec.msg_name)
            file_content += "           <name>%s%s_%s_%s</name>\n" % (ros2_prefix, spec.base_type.pkg_name, subfolder, spec.msg_name)
            file_content += "           <dataType>%s::%s::dds_::%s_</dataType>\n" % (spec.base_type.pkg_name, subfolder, spec.msg_name)
            file_content += "       </topic>\n"
            file_content += "   </dds>\n"


            # Write file content
            print ("%s" % file_content)



        #elif extension == '.srv':

            #data = {'spec': spec}
            #data.update(functions)

            #if not os.path.exists(args['output_dir']):
            #    os.makedirs(args['output_dir'])

            #f = open(os.path.join(args['output_dir'], 'demofile.txt'), 'w+')
            #f.write("%s\n" % spec)
            #f.close()
    return 0
