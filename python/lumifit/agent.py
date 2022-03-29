#!/usr/bin/env python3
"""
Simple program that runs commands given as a json-fomratted object
in a named pipe and returns their outputs and return code in the same way.

the structure of the order and return object is governd by the SlurmOrder Class.
"""

import attr
import datetime
import json
import logging
import os
import shlex
import subprocess
import stat
import sys
from typing import Dict


@attr.s(hash=True)
class SlurmOrder:
    orderType: str = attr.ib(default="regular")
    cmd: str = attr.ib(default="id")
    returnCode: int = attr.ib(default=-1)
    stdout: str = attr.ib(default="")
    stderr: str = attr.ib(default="")
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


class Agent:
    # TODO: put this somewhere else, everybody can write to this!
    # this may even be accidental by some scripts
    universalPipePath = "/tmp/lmdfit"

    def SlurmOrderExamples() -> None:
        slurmOrder = SlurmOrder()
        print(f"This is the Slurm Order:\n{slurmOrder}\n")
        print(f"This is the Slurm Order as json:\n{slurmOrder.toJson()}\n")
        jsonString = slurmOrder.toJson()
        jsonObj = json.loads(jsonString)
        jsonObj["cmd"] = "whoami"
        jsonObj["env"] = {"AWESOME": "hellYes"}
        jsonString = json.dumps(jsonObj)
        print(f"Let's change it up a bit:\n{jsonString}\n")
        print(
            f"now, backwards from Json:\n{SlurmOrder.fromJson(jsonString)}\n"
        )
        sys.exit(0)

    def sendOrder(self, thisOrder: SlurmOrder) -> None:
        with open(
            self.universalPipePath, "w", encoding="utf-8"
        ) as universalPipe:
            json.dump(thisOrder.toJson(), universalPipe)

    def receiveOrder(self) -> SlurmOrder:
        # TODO: I don't know is this works for multi-line orders. check that!
        with open(
            self.universalPipePath, "r", encoding="utf-8"
        ) as universalPipe:

            # this handles the entire pipe (encoding, newlines and all)
            payload = json.load(universalPipe)

        if isinstance(payload, str):
            # apparently, this is by design! if the json thing is interpreted as string, then a string is returned (and not a dict)!
            # so just remove the output and parse again so that it becomes a dict
            # TODO: find out how you can force this dict-behaviour
            # could be https://stackoverflow.com/questions/71397342/how-to-use-pythons-jsondecoder
            payload = json.loads(payload)

        # python json's are actually dicts
        return SlurmOrder.fromDict(payload)


class Server(Agent):
    def preparePipes(self) -> None:
        if os.path.exists(self.universalPipePath):
            if not stat.S_ISFIFO(os.stat(self.universalPipePath).st_mode):
                print(
                    f"Warning! Path {self.universalPipePath} exists but is not a pipe. Deleting!"
                )
                os.unlink(self.universalPipePath)

        if not os.path.exists(self.universalPipePath):
            os.mkfifo(self.universalPipePath)

    def deletePipes(self) -> None:
        if os.path.exists(self.universalPipePath):
            os.unlink(self.universalPipePath)

    # continouosly read from pipe and execute, write output and return codes back
    def mainLoop(self) -> None:
        while True:
            # read entire pipe contents and try to deserialize json from it (close pipe!)
            thisOrder = self.receiveOrder()
            logging.info(f"Received Order:\n{thisOrder}\n")
            # execute command as ordered
            returnOrder = self.execute(thisOrder)

            # return result
            logging.info(f"Sent back result:\n{returnOrder}\n")
            self.sendOrder(returnOrder)

    def execute(self, thisOrder: SlurmOrder) -> SlurmOrder:
        if thisOrder.orderType == "meta":
            if thisOrder.cmd == "exit":
                self.bail()
            elif thisOrder.cmd == "test":
                thisOrder.stdout = "ok"
                thisOrder.returnCode = 0
                return thisOrder

        elif thisOrder.orderType == "regular":
            # this returns a CompletedProcess

            if not thisOrder.runShell:
                cmds = shlex.split(thisOrder.cmd)
                process = subprocess.run(
                    cmds,
                    env=thisOrder.env,
                    capture_output=True,  # this is available from Python 3.7 onwards, but NOT 3.6 (which is on himster)
                    shell=False,
                    encoding="utf-8",
                )

            else:
                process = subprocess.run(
                    thisOrder.cmd,
                    env=thisOrder.env,
                    capture_output=True,
                    shell=True,
                    encoding="utf-8",
                )

            # write stdout, stderr and return code to returned SlurmOrder
            thisOrder.returnCode = process.returncode
            thisOrder.stdout = process.stdout
            thisOrder.stderr = process.stderr
            return thisOrder

    def bail(self) -> None:
        print(f'Received "exit" command. Exiting now.')
        self.deletePipes()
        print("Have a nice day.")
        sys.exit(0)

    def run() -> None:
        welcome = """
        Agent starting and forking to background.
        Write "exit" to orderPipe to exit agent.
        """

        print(welcome)

        try:
            pid = os.fork()
            if pid > 0:
                # Exit parent process
                sys.exit(0)

        except OSError as e:
            print(f"fork failed: {e.errno} ({e.strerror})")
            sys.exit(1)

        # preapare log
        logging.basicConfig(
            filename=f"agentLog-{datetime.datetime.now().isoformat()}.log",
            encoding="utf-8",
            level=logging.INFO,
        )
        logging.info(
            f"Starting Log at {datetime.datetime.now().isoformat()}\n"
        )

        thisServer = Server()
        thisServer.preparePipes()
        thisServer.mainLoop()


class Client(Agent):
    def checkConnection(self) -> None:
        testOrder = SlurmOrder()
        testOrder.orderType = "meta"
        testOrder.cmd = "test"
        self.sendOrder(testOrder)
        resultOrder = self.receiveOrder()
        assert resultOrder.stdout == "ok"

    def __init__(self) -> None:
        super().__init__()
        if not os.path.exists(self.universalPipePath):
            raise Exception(
                f"named pipe not found at {self.universalPipePath}. Please start the agent first!"
            )
        if not stat.S_ISFIFO(os.stat(self.universalPipePath).st_mode):
            raise Exception(
                f"\nERROR! Path {self.universalPipePath} is not a pipe!"
            )


if __name__ == "__main__":
    Server.run()

    #! Uncomment this to see some basic examples
    # Agent.SlurmOrderExamples()
