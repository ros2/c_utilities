
# Copyright 2017 Open Source Robotics Foundation, Inc.
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

from launch import LaunchDescription
from launch import LaunchService
from launch.actions import ExecuteProcess
from launch_testing.legacy import LaunchTestService
from launch_testing.legacy.output import create_output_test_from_file


def test_logging_output_format():
    ld = LaunchDescription()
    launch_test = LaunchTestService()
    # Re-use the test_logging_long_messages test binary and modify the output format from an
    # environment variable.
    executable = os.path.join(os.getcwd(), 'test_logging_long_messages')
    if os.name == 'nt':
        executable += '.exe'
    env_long = dict(os.environ)
    # In this custom output, the long message is output twice, to test both dynamic allocation and
    # re-allocation.
    env_long['RCUTILS_CONSOLE_OUTPUT_FORMAT'] = \
        '[{{name}}].({severity}) output: {file_name}:{line_number} {message}, again: {message} ({function_name}()){'  # noqa
    name = 'test_logging_output_format_long'
    action = launch_test.add_fixture_action(ld, ExecuteProcess(
        cmd=[executable], env=env_long, name=name, output='screen'
    ))
    output_file = os.path.join(os.path.dirname(__file__), name)
    launch_test.add_output_test(
        ld, action, create_output_test_from_file(output_file)
    )

    env_edge_cases = dict(os.environ)
    # This custom output is to check different edge cases of the output format string parsing.
    env_edge_cases['RCUTILS_CONSOLE_OUTPUT_FORMAT'] = '{}}].({unknown_token}) {{{{'
    name = 'test_logging_output_format_edge_cases'
    action = launch_test.add_fixture_action(ld, ExecuteProcess(
        cmd=[executable], env=env_edge_cases, name=name, output='screen'
    ))
    output_file = os.path.join(os.path.dirname(__file__), name)
    launch_test.add_output_test(
        ld, action, create_output_test_from_file(output_file)
    )

    env_no_tokens = dict(os.environ)
    # This custom output is to check that there are no issues when no tokens are used.
    env_no_tokens['RCUTILS_CONSOLE_OUTPUT_FORMAT'] = 'no_tokens'
    name = 'test_logging_output_format_no_tokens'
    action = launch_test.add_fixture_action(ld, ExecuteProcess(
        cmd=[executable], env=env_no_tokens, name=name, output='screen'
    ))
    output_file = os.path.join(os.path.dirname(__file__), name)
    launch_test.add_output_test(
        ld, action, create_output_test_from_file(output_file)
    )

    env_time_tokens = dict(os.environ)
    # This custom output is to check that time stamps work correctly
    env_time_tokens['RCUTILS_CONSOLE_OUTPUT_FORMAT'] = "'{time}' '{time_as_nanoseconds}'"
    name = 'test_logging_output_timestamps'
    action = launch_test.add_fixture_action(ld, ExecuteProcess(
        cmd=[executable], env=env_time_tokens, name=name, output='screen'
    ))
    output_file = os.path.join(os.path.dirname(__file__), name)
    launch_test.add_output_test(
        ld, action, create_output_test_from_file(output_file)
    )

    launch_service = LaunchService()
    launch_service.include_launch_description(ld)
    return_code = launch_test.run(launch_service)
    assert return_code == 0, 'Launch failed with exit code %r' % (return_code,)


if __name__ == '__main__':
    test_logging_output_format()
