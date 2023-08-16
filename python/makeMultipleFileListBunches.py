#!/usr/bin/env python3

"""
This thing searches root files.
It then separates them into bunches of 10 files each (other sizes can be specified).
It then creates a text file for each bunch (so 10 files if there were 100 ROOT files)
and writes the absolute paths to the files into the text files.

TODO: add type hints and make this a module
"""

import argparse
from pathlib import Path
from typing import List

from lumifit.general import getGoodFiles
from lumifit.paths import generateRelativeBunchesDir


def createFileListFile(output_url: Path, list_of_files: List[Path]) -> None:
    if output_url.exists() and not args.force:
        print("file already exists, skipping: " + str(output_url))
        return

    with open(output_url, "w") as f:
        for file_url in list_of_files:
            f.write(str(file_url))
            f.write("\n")


def makeFileListBunches(directory: Path) -> None:
    good_files, _ = getGoodFiles(directory, filename_prefix + "*", 2000)

    print("creating file lists...")

    if args.maximum_number_of_files > 0 and args.maximum_number_of_files < len(good_files):
        good_files = good_files[: args.maximum_number_of_files]

    max_bundles = len(good_files) / args.files_per_bunch
    if len(good_files) % args.files_per_bunch > 0:
        max_bundles += 1
    max_bundles = int(max_bundles)

    # right now, this only generates "bunches"
    output_bunch_dir = directory / generateRelativeBunchesDir()

    output_bunch_dir.mkdir(parents=True, exist_ok=True)

    file_list_index = 1
    while len(good_files) > 0:
        # get next chunk of good files
        chunk_size = args.files_per_bunch
        if len(good_files) < args.files_per_bunch:
            chunk_size = len(good_files)

        chunk_of_good_files: List[Path] = []
        for i in range(1, chunk_size + 1):
            chunk_of_good_files.append(good_files.pop())

        createFileListFile(
            output_bunch_dir / Path("filelist_" + str(file_list_index) + ".txt"),
            chunk_of_good_files,
        )
        file_list_index += 1


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

pattern = args.directory_pattern
filename_prefix = args.filenamePrefix

pathToRootFiles = Path(args.dirname[0])
makeFileListBunches(pathToRootFiles)
