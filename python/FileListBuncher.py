#!/usr/bin/env python3

import argparse
import re
from pathlib import Path
from typing import List

import attr
from lumifit.general import getGoodFiles


@attr.s
class FileListBuncher:
    dirname: Path = attr.ib(default=Path("."))
    pattern: str = attr.ib(default="")
    filename_prefix: str = attr.ib(default="")
    maximum_number_of_files: int = attr.ib(default=-1)
    force: bool = attr.ib(default=False)
    files_per_bunch: int = attr.ib(default=1000)
    _dirs: List[Path] = attr.ib(factory=list)

    def collect_list_of_directories(self, path: Path) -> None:
        bunch_pattern = re.compile("bunches.*")
        file_pattern = re.compile(f"{self.filename_prefix}\d*\.root")
        dir_pattern = re.compile(self.pattern)

        directories = [path]

        while directories:
            current_path = directories.pop()
            
            if current_path.is_dir():
                for stub in current_path.iterdir():
                    dirpath = current_path / stub
                    if dirpath.is_dir():
                        if not self.force:
                            match = bunch_pattern.search(str(dirpath))
                            if match:
                                print(f"skipping bunch creation for directory {current_path} as it already was bunched. Please used the --force option to bunch this directory anyway!")                            
                                continue
                        directories.append(dirpath)
                    else:
                        match = file_pattern.search(str(stub))
                        if match:
                            match_dir_pattern = dir_pattern.search(str(current_path))
                            if match_dir_pattern:
                                self._dirs.append(current_path)

        

    @staticmethod
    def create_file_list_file(output_url: Path, list_of_files: List[str]) -> None:
        
        # make directory if necessary
        output_url.parent.mkdir(parents=True, exist_ok=True)

        with open(output_url, "w") as f:
            for file_url in list_of_files:
                f.write(file_url)
                f.write("\n")

    def make_file_list_bunches(self, directory: Path) -> None:
        good_files, _ = getGoodFiles(directory, self.filename_prefix + "*", 2000)
        
        print("creating file lists...")
        
        num_of_files = len(good_files)
        
        if 0 < self.maximum_number_of_files < num_of_files:
            good_files = good_files[:self.maximum_number_of_files]
            num_of_files = self.maximum_number_of_files
    
        max_bundles = (num_of_files + self.files_per_bunch - 1) // self.files_per_bunch
        output_bunch_dir = directory / "bunches_" / str(max_bundles)
        output_bunch_dir.mkdir(parents=True, exist_ok=True)
    
        for file_list_index in range(max_bundles):
            start_index = file_list_index * self.files_per_bunch
            chunk_of_good_files = good_files[start_index : start_index + self.files_per_bunch]  # can only be too long in the last chunk, and then it just takes the rest
            self.create_file_list_file(output_bunch_dir / f"filelist_{file_list_index+1}.txt", chunk_of_good_files)
    

    def run(self) -> None:
        self.get_list_of_directories(self.dirname)
        for directory in self._dirs:
            self.make_file_list_bunches(directory)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Module for going through whole directory trees and generating filelist bunches for faster lmd data creation.",
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
    parser.add_argument(
        "--filenamePrefix",
        type=str,
        help="Either Lumi_TrksQA_ or Koala_IP_",
        required=True,
    )

    parser.add_argument("--force", action="store_true", help="force recreation")

    args = parser.parse_args()

    buncher = FileListBuncher(
        dirname=Path(args.dirname[0]),
        pattern=args.directory_pattern,
        filename_prefix=args.filenamePrefix,
        files_per_bunch=args.files_per_bunch,
        maximum_number_of_files=args.maximum_number_of_files,
        force=args.force
    )
  
    buncher.run()
