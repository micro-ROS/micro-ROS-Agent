#!/usr/bin/env python3

import os
import sys
import argparse
import xml.etree.ElementTree

sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)),'..'))

from micro_ros_agent import *

def main(argv=sys.argv[1:]):
    parser = argparse.ArgumentParser(
        description='Import default xml files',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument(
        '--default-xml-path',
        required=True,
        help='The location of the default profiles')
    args = parser.parse_args(argv)
    ReadDefaultXMLs(args.default_xml_path)

    return 0

if __name__ == '__main__':
    sys.exit(main())
