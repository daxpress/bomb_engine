import difflib
import multiprocessing
import os
from pathlib import Path


class HeaderTool:
    '''Generates code to bind the C++ API to Python using Pybind11 syntax'''

    generated_headers_dir = "generated"

    def __init__(self, add, remove, change):
        # setup headers list and the corresponding operaton
        self.headers = []
        for header in add:
            self.headers.append((header, "add"))
        for header in remove:
            self.headers.append((header, "remove"))
        for header in change:
            self.headers.append((header, "update"))
        self.operation = {
            "add": self.__add_header,
            "remove": self.__remove_header,
            "update": self.__update_header
        }

        if not os.path.exists(self.generated_headers_dir):
            os.mkdir(self.generated_headers_dir)

    def run(self):
        cpu_count = multiprocessing.cpu_count()
        for (header, op) in self.headers:
            self.operation[op](header)

    def __remove_header(self, header):
        '''Deletes the generated header (pass in the original header path)'''
        path = self.generated_headers_dir + "/" + self.__as_generated(header)
        if os.path.exists(path):
            os.remove(path)

    def __add_header(self, header):
        path = self.generated_headers_dir + "/" + self.__as_generated(header)
        with open(path, "x") as f:
            pass

    def __update_header(self, header):
        path = self.generated_headers_dir + "/" + self.__as_generated(header)
        with open(path, "w+") as f:
            pass

    def __as_generated(self, header: str):
        '''Returns the generated header's relative path.
           Supported extensions are .h, .hpp, .ixx'''
        return Path(header).stem + ".pybound.h"