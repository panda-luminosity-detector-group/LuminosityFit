#!/usr/bin/env python3
"""
Simple program that runs commands given as a json-fomratted object
in a named pipe and returns their outputs and return code in the same way.

the structure of the order and return object is governd by the SlurmOrder Class.
"""

import attr
import cattrs
import datetime
import json
import logging
import multiprocessing as mp
import os
import shlex
import subprocess
import stat
import sys
from enum import IntEnum
from pathlib import Path
from typing import Dict


@attr.s(hash=True)
class orderType(IntEnum):
    # the actual values of these enums must be integers now!
    EXIT = -1
    META = 0
    REGULAR = 1


@attr.s(hash=True)
class SlurmOrder:
    orderType: orderType = attr.ib(default=orderType.REGULAR)
    cmd: str = attr.ib(default="id")
    returnCode: int = attr.ib(default=-1)
    stdout: str = attr.ib(default="")
    stderr: str = attr.ib(default="")
    env: Dict = attr.ib(default={})
    runShell: bool = attr.ib(default=False)
    version: int = attr.ib(default=1)

    # I think there is no real constructor overloading in python?
    # TODO: I think there is a more modern way to do this
    @classmethod
    def fromJson(cls, jsonString: str):
        temp = cls()
        data = json.loads(jsonString)
        for key, value in data.items():
            setattr(temp, key, value)
        return temp

    # TODO: I think there is a more modern way to do this
    @classmethod
    def fromDict(cls, jsonDict: dict):
        temp = cls()
        for key, value in jsonDict.items():
            setattr(temp, key, value)
        return temp

    def toJson(self) -> str:
        return json.dumps(self.__dict__)


class Agent:
    universalPipePath: Path = Path(f"{os.getenv('HOME')}/tmp/lmdfit")

    def SlurmOrderExamples(self) -> None:
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
        proc = mp.Process(target=self.sendOrderSP, args=(thisOrder,))
        proc.start()
        # writing shouldn't take long, but command execution may take a second
        proc.join(10)

    def sendOrderSP(self, thisOrder: SlurmOrder) -> None:
        with open(
            self.universalPipePath, "w", encoding="utf-8"
        ) as universalPipe:
            json.dump(thisOrder.toJson(), universalPipe)

    def receiveOrder(self):
        with open(
            self.universalPipePath, "r", encoding="utf-8"
        ) as universalPipe:
            try:
                # this handles the entire pipe (encoding, newlines and all)
                payload = json.load(universalPipe)
            except Exception:
                print(f"error parsing json order from pipe!")
                return None

        if isinstance(payload, str):
            # wait there is an error here, the json is encapsulated with quotes? check the sendOrder function
            payload = json.loads(payload)

        # python json's are actually dicts
        return SlurmOrder.fromDict(payload)


class Server(Agent):
    """
    checks:
    - if pipe exists and is pipe. if yes, return. else:
    - if its regular file, delete it. then:
    - if parent path for pipe exists, if not, creates it. then make pipe
    """

    def preparePipe(self) -> None:
        if self.universalPipePath.exists():
            if not stat.S_ISFIFO(os.stat(self.universalPipePath).st_mode):
                print(
                    f"Warning! Path {self.universalPipePath} exists but is not a pipe. Deleting!"
                )
                self.universalPipePath.unlink()

        if not self.universalPipePath.parent.exists():
            self.universalPipePath.parent.mkdir()

        if not os.path.exists(self.universalPipePath):
            os.mkfifo(self.universalPipePath)

    def deletePipes(self) -> None:
        if self.universalPipePath.exists():
            self.universalPipePath.unlink()

    # continouosly read from pipe and execute, write output and return codes back
    def mainLoop(self) -> None:
        while True:
            # read entire pipe contents and try to deserialize json from it (close pipe!)
            thisOrder = self.receiveOrder()
            logging.info(
                f"{datetime.datetime.now().isoformat(timespec='seconds')}: Received Order:\n{thisOrder}\n"
            )

            if thisOrder is not None:

                # execute command as ordered
                returnOrder = self.execute(thisOrder)

                # return result
                logging.info(
                    f"{datetime.datetime.now().isoformat(timespec='seconds')}: Sent back result:\n{returnOrder}\n"
                )
                self.sendOrder(returnOrder)

    def execute(self, thisOrder: SlurmOrder) -> SlurmOrder:
        if thisOrder.orderType == orderType.EXIT:
            self.bail()

        elif thisOrder.orderType == orderType.META:
            if thisOrder.cmd == "test":
                thisOrder.stdout = "ok"
                thisOrder.returnCode = 0
                return thisOrder

        elif thisOrder.orderType == orderType.REGULAR:
            if not thisOrder.runShell:
                cmds = shlex.split(thisOrder.cmd)
                # this returns a CompletedProcess
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

        raise NotImplementedError(
            f"order type {thisOrder.orderType} is not implemented!"
        )

    def bail(self) -> None:
        print(f'Received "exit" command. Exiting now.')
        self.deletePipes()
        print("Have a nice day.")
        sys.exit(0)

    def run(self) -> None:

        # fork to background
        try:
            pid = os.fork()
            if pid > 0:
                # Exit parent process
                sys.exit(0)

        except OSError as e:
            print(f"fork failed: {e.errno} ({e.strerror})")
            sys.exit(1)

        welcome = f"""
        Agent starting and forking to background. Write: {{"orderType": -1}} to orderPipe to exit agent.
        Or, just copy this command:

        echo '{{"orderType":-1 }}' > {self.universalPipePath}
        """

        print(welcome)

        # preapare log
        logging.basicConfig(
            filename=f"agentLog-{datetime.datetime.now().isoformat()}.log",
            encoding="utf-8",
            level=logging.INFO,
        )
        logging.info(
            f"Starting Log at {datetime.datetime.now().isoformat()}\n"
        )

        self.preparePipe()
        self.mainLoop()


class Client(Agent):
    def checkConnection(self) -> bool:
        testOrder = SlurmOrder()
        testOrder.orderType = orderType.META
        testOrder.cmd = "test"
        self.sendOrder(testOrder)
        resultOrder = self.receiveOrder()
        assert (
            resultOrder.stdout == "ok"
        ), "Agent not running or not accepting commands!"
        return True

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
    thisServer = Server()
    thisServer.run()

    #! Uncomment this to see some basic examples
    # Agent.SlurmOrderExamples()
