#!/usr/bin/env python3
"""
Simple program that runs commands given as a json-fomratted object
in a named pipe and returns their outputs and return code in the same way.

the structure of the order and return object is governd by the SlurmOrder Class.
"""

import attr, json, os, shlex, subprocess, sys
from typing import Dict


@attr.s(hash=True)
class SlurmOrder:
    orderType: str = attr.ib(default='regular')
    cmd: str = attr.ib(default='id')
    returnCode: int = attr.ib(default=255)
    stdout: str = attr.ib(default='')
    stderr: str = attr.ib(default='')
    env: Dict = attr.ib(default={})
    runShell: bool = attr.ib(default=False)
    version: int = attr.ib(default=1)

    # I think there is no real constructor overloading in python?
    @classmethod
    def fromJson(cls, jsonString):
        temp = cls()
        data = json.loads(jsonString)
        for key, value in data.items():
            setattr(temp, key, value)
        return temp

    @classmethod
    def fromDict(cls, jsonDict):
        temp = cls()
        for key, value in jsonDict.items():
            setattr(temp, key, value)
        return temp

    def toJson(self) -> str:
        return json.dumps(self.__dict__)


def preparePipes(universalPipeName) -> None:
    if not os.path.exists(universalPipeName):
        os.mkfifo(universalPipeName)


def deletePipes(universalPipeName) -> None:
    if os.path.exists(universalPipeName):
        os.unlink(universalPipeName)


# continouosly read from pipe and execute, write output and return codes back
def pipeIO(universalPipeName) -> None:
    while True:
        # read entire pipe contents and try to deserialize json from it (close pipe!)
        with open(universalPipeName, 'r', encoding='utf-8') as universalPipe:
            payload = json.load(
                universalPipe
            )  # this handles the entire pipe (encoding, newlines and all)
            thisOrder = SlurmOrder.fromDict(
                payload)  # python json's are actually dicts

        # execute command as ordered
        returnOrder = execute(thisOrder)

        with open(universalPipeName, 'w', encoding='utf-8') as universalPipe:
            json.dump(returnOrder.toJson(), universalPipe)


def execute(thisOrder: SlurmOrder) -> SlurmOrder:

    if (thisOrder.orderType == 'meta'):
        if (thisOrder.cmd == 'exit'):
            bail()

    elif (thisOrder.orderType == 'regular'):
        # this returns a CompletedProcess

        if not thisOrder.runShell:
            cmds = shlex.split(thisOrder.cmd)
            process = subprocess.run(cmds,
                                     env=thisOrder.env,
                                     capture_output=True,
                                     shell=False,
                                     encoding='utf-8')

        else:
            process = subprocess.run(thisOrder.cmd,
                                     env=thisOrder.env,
                                     capture_output=True,
                                     shell=True,
                                     encoding='utf-8')

        # write stdout, stderr and return code to returned SlurmOrder
        thisOrder.returnCode = process.returncode
        thisOrder.stdout = process.stdout
        thisOrder.stderr = process.stderr
        return thisOrder


def bail() -> None:
    print(f'Received "exit" command. Exiting now.')
    deletePipes(universalPipe)
    print('Have a nice day.')
    sys.exit(0)


def SlurmOrderExamples():
    slurmOrder = SlurmOrder()
    print(f'This is the Slurm Order:\n{slurmOrder}\n')

    print(f'This is the Slurm Order as json:\n{slurmOrder.toJson()}\n')

    jsonString = slurmOrder.toJson()
    jsonObj = json.loads(jsonString)
    jsonObj['cmd'] = 'whoami'
    jsonObj['env'] = {'AWESOME': 'hellYes'}
    jsonString = json.dumps(jsonObj)
    print(f'Let\'s change it up a bit:\n{jsonString}\n')

    print(f'now, backwards from Json:\n{SlurmOrder.fromJson(jsonString)}\n')
    sys.exit(0)


if __name__ == '__main__':
    welcome = """
    Agent starting and forking to background.
    Write "exit" to orderPipe to exit agent.
    """

    print(welcome)

    #! Uncomment this to see some basic examples
    # SlurmOrderExamples()

    try:
        pid = os.fork()
        if pid > 0:
            # Exit parent process
            sys.exit(0)

    except OSError:
        print(f"fork failed: {e.errno} ({e.strerror})")
        sys.exit(1)

    universalPipe = '/tmp/lmdfit'
    preparePipes(universalPipe)

    pipeIO(universalPipe)
