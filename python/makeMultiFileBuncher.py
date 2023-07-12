#!/usr/bin/env python3

import argparse
import errno
import multiprocessing
import os
import re
from typing import List, Tuple

import attr
from lumifit.general import getGoodFiles


@attr.s
class FileListBunches:
    cpu_cores: int = attr.ib(default=multiprocessing.cpu_count())
    dirs: List[str] = attr.ib(factory=list)
    pattern: str = attr.ib(default="")
    filename_prefix: str = attr.ib(init=False)

    def get_list_of_directories(self, path: str) -> None:
        if os.path.isdir(path):
            for dir in os.listdir(path):
                dirpath = path + "/" + dir
                if os.path.isdir(dirpath):
                    if not args.force:
                        match = re.search("bunches.*", dirpath)
                        if match:
                            print(
                                "skipping bunch creation for directory",
                                path,
                                "as it already was bunched. Please used the --force option to bunch this directory anyway!",
                            )
                            return
                    self.get_list_of_directories(dirpath)
                else:
                    match = re.search(f"{self.filename_prefix}\d*\.root", dir)
                    if match:
                        match_dir_pattern = re.search(self.pattern, path)
                        if match_dir_pattern:
                            self.dirs.append(path)
                        return

    @staticmethod
    def create_file_list_file(output_url: str, list_of_files: List[str]) -> None:
        f = open(output_url, "w")
        for file_url in list_of_files:
            f.write(file_url)
            f.write("\n")
        f.close()

    def make_file_list_bunches(self, directory: str) -> None:
        [good_files, percentage] = getGoodFiles(directory, self.filename_prefix + "*", 2000)

        print("creating file lists...")

        if args.maximum_number_of_files > 0 and args.maximum_number_of_files < len(good_files):
            good_files = good_files[: args.maximum_number_of_files]

        max_bundles = len(good_files) / args.files_per_bunch
        if len(good_files) % args.files_per_bunch > 0:
            max_bundles += 1
        max_bundles = int(max_bundles)
        output_bunch_dir = directory + "/bunches_" + str(max_bundles)

        try:
            os.makedirs(output_bunch_dir)
        except OSError as exception:
            if exception.errno != errno.EEXIST:
                print("error: thought dir does not exists but it does...")

        file_list_index = 1
        while len(good_files) > 0:
            chunk_size = args.files_per_bunch
            if len(good_files) < args.files_per_bunch:
                chunk_size = len(good_files)

            chunk_of_good_files = []
            for i in range(1, chunk_size + 1):
                chunk_of_good_files.append(good_files.pop())

            self.create_file_list_file(
                output_bunch_dir + "/filelist_" + str(file_list_index) + ".txt",
                chunk_of_good_files,
            )
            file_list_index += 1

    def main(self) -> None:
        self.pattern = args.directory_pattern
        self.filename_prefix = args.filenamePrefix

        self.get_list_of_directories(args.dirname[0])

        for dir in self.dirs:
            self.make_file_list_bunches(dir)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Script for going through whole directory trees and generating filelist bunches for faster lmd data creation.",
        formatter_class=argparse.RawTextHelpFormatter,
    )

    parser.add_argument(
        "dirname",
        metavar="dirname_to_scan",
        type=str,
        nargs=1,
        help="Name of directory to scan recursively for qa files and create bunches",
    )
    parser.add_argument(
        "--files_per_bunch",
        metavar="files_per_bunch",
        type=int,
        default=4,
        help="number of root files used for a bunch",
    )
    parser.add_argument(
        "--maximum_number_of_files",
        metavar="maximum_number_of_files",
        type=int,
        default=-1,
        help="total number of root files to use",
    )
    parser.add_argument(
        "--directory_pattern",
        metavar="directory_pattern",
        type=str,
        default=".*",
        help="Only directories with according to this pattern will be used.",
    )
    parser.add_argument("--force", action="store_true", help="force recreation")

    parser.add_argument(
        "--filenamePrefix",
        type=str,
        help="Either Lumi_TrksQA_ or Koala_IP_",
        required=True,
    )

    args = parser.parse_args()

    file_list_bunches = FileListBunches()
    file_list_bunches.main()
