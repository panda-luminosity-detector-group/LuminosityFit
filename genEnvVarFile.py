#!/usr/bin/env python3

doc = """
This is the script to generate the env var file.
It needs only be run once on a new cluster environment to check if all
neccessary environment variables are set and to generate the lmdEnvFile.env,
which is passed to singularity during job submission.

Run this file INSIDE of the singularity container.

The necessary env variables are:

- VMCWORKDIR
- LMDFIT_DATA_DIR
- PATH (this is complicated because of cvmfs/PandaRoot/KoalaSoft)
- LMDFIT_BUILD_PATH
- LMDFIT_SCRIPTPATH
- LMDFIT_MACROPATH
- LD_LIBRARY_PATH (also complicated because it needs to be modded after PandaRoot is loaded)

You'll be prompted for each one separately and asked for variables that can't
be detected automatically.
"""

import os
import sys

from pathlib import Path


def ask(
    varname: str,
    default: str = None,
    description: str = None,
) -> str:

    varnameVal: str = os.getenv(varname)
    if varnameVal is None:
        print(f"\n{varname} is not set.")
        if description is not None:
            print(description)

        if default is None:
            varnameVal = input(f"please enter the value for {varname}:\n")

        else:
            print("\nDefault value is:")
            varnameVal = str(Path(default).resolve())

    # only break free if user answers yes
    while True:
        print(f"\n{varname} is {varnameVal}.\n")
        if description != "":
            print(description)

        answer = input("is this correct? [y]/n\n").lower()
        if answer in ["", "y"]:
            return varnameVal

        elif answer in ["n"]:
            varnameVal = input(f"please enter the value for {varname}:\n")


def genEnvs():
    currentDir = Path.cwd()
    print(f"working dir is {currentDir}")

    vars = {}

    if (currentDir / Path("lmdEnvFile.env")).exists():
        if input("Warning! env file already exists! Overwrite? y/[n]") != "y":
            print(f"Exiting!")
            sys.exit()

    # * --------------
    vars["VMCWORKDIR"] = ask(
        "VMCWORKDIR", "", "This is PandaRoot / KoalaSoft installation dir."
    )

    # * --------------
    vars["LMDFIT_MACROPATH"] = ask(
        "LMDFIT_MACROPATH",
        f"{vars['VMCWORKDIR']}/macro/lmd",
        "This should be where the run macros for PandaRoot / KoalaSoft are.",
    )

    # * --------------
    vars["LMDFIT_BUILD_PATH"] = ask(
        "LMDFIT_BUILD_PATH",
        default=f'{Path.cwd() / Path("build")}',
        description="This is where the compiled Luminosity Fit executables will be.",
    )

    # * --------------
    vars["LMDFIT_SCRIPTPATH"] = ask(
        "LMDFIT_SCRIPTPATH",
        default=f"{vars['LMDFIT_BUILD_PATH']}/../python",
        description="This must be the python subdirectory of the Lmd Fit directory.",
    )

    # * --------------
    vars["LD_LIBRARY_PATH"] = ask(
        "LD_LIBRARY_PATH",
        default=f":{vars['LMDFIT_BUILD_PATH']}/lib",
        description="The compiled Lumi Fit libraries. Set this path correctly so that ROOT can find the libraries at run time. It must contain 'LuminosityFit/build/lib'",
    )

    # * --------------
    vars["PATH"] = ask(
        "PATH", description="It's probably safe to use this just as it is."
    )

    # * --------------

    vars["LMDFIT_DATA_DIR"] = ask(
        "LMDFIT_DATA_DIR",
        description="The base directory where all generated data is stored. This should NOT be inside your home directory and instead somewhere on the lustreFS.",
    )

    print(f"Summary:\n")
    print(vars)

    print(f"Writing to env var file...")

    with open("lmdEnvFile.env", "w") as file:
        for k, v in vars.items():
            file.write(f'{k}="{v}"\n')


if __name__ == "__main__":
    print(doc)
    genEnvs()
