#!/usr/bin/env python3

import argparse
import os
import sys

sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)),'..'))

from rosidl_cmake import read_generator_arguments
from rosidl_adapter.parser import UnknownMessageType
from micro_ros_agent import generate_XML


def main(argv=sys.argv[1:]):
    parser = argparse.ArgumentParser(
        description='Generate the C interfaces for Micro RTPS.',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument(
        '--generator-arguments-file',
        required=True,
        help='The location of the file containing the generator arguments')
    args = parser.parse_args(argv)

    generator_args = read_generator_arguments(args.generator_arguments_file)
    rc = generate_XML(generator_args)
    return rc


if __name__ == '__main__':
    sys.exit(main())
