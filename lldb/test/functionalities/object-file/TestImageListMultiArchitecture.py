"""
Test lldb 'image list' on object files across multiple architectures.
This exercises classes like ObjectFileELF and their support for opening
foreign-architecture object files.
"""

import os.path
import unittest2
import lldb
from lldbtest import *
import lldbutil
import re

class TestImageListMultiArchitecture(TestBase):

    mydir = TestBase.compute_mydir(__file__)

    def setUp(self):
        # Call super's setUp().
        TestBase.setUp(self)

    def test_image_list_shows_multiple_architectures(self):
        """Test that image list properly shows the correct architecture for a set of different architecture object files."""
        images = {
            "hello-freebsd-10.0-x86_64-clang-3.3": re.compile(r"x86_64-(unknown)?-freebsd10.0 x86_64"),
            "hello-freebsd-10.0-x86_64-gcc-4.7.3": re.compile(r"x86_64-(unknown)?-freebsd10.0 x86_64"),
            "hello-netbsd-6.1-x86_64-gcc-4.5.3": re.compile(r"x86_64-(unknown)?-netbsd x86_64"),
            "hello-ubuntu-14.04-x86_64-gcc-4.8.2": re.compile(r"x86_64-(unknown)?-linux x86_64"),
            "hello-ubuntu-14.04-x86_64-clang-3.5pre": re.compile(r"x86_64-(unknown)?-linux x86_64"),
        }

        for image_name in images:
            file_name = os.path.abspath(os.path.join(os.path.dirname(__file__), "bin", image_name))
            expected_triple_and_arch_regex = images[image_name]

            self.runCmd("file {}".format(file_name))
            self.match("image list -t -A", [expected_triple_and_arch_regex])

if __name__ == '__main__':
    import atexit
    lldb.SBDebugger.Initialize()
    atexit.register(lambda: lldb.SBDebugger.Terminate())
    unittest2.main()
