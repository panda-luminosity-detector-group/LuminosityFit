#!/usr/bin/env python3
"""
Simple program that runs commands given as a json-fomratted object
in a named pipe and returns their outputs and return code in the same way.

the structure of the order and return object is governd by the SlurmOrder Class.
"""

import datetime
import json
import logging
import os
import random
import shlex
import stat
import string
import subprocess
import sys

# import multiprocessing as mp
import threading as th
from enum import IntEnum
from pathlib import Path
from typing import Dict

import attr


@attr.s(hash=True)
class orderType(IntEnum):
    # the actual values of these enums must be integers now!
    EXIT = -1
    META = 0
    REGULAR = 1
    MAKE_UNQUE = 2
    UNIQUE_CONFIRM = 4


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

    # TODO: I think there is a more modern way to do this, but cattrs is still too flakey
    @classmethod
    def fromDict(cls, jsonDict: dict) -> "SlurmOrder":
        temp = cls()
        for key, value in jsonDict.items():
            setattr(temp, key, value)
        return temp


class Agent:
    universalPipePath: Path = Path(f"{os.getenv('HOME')}/tmp/lmdfit")

    def SlurmOrderExamples(self) -> None:
        """
        This function currently doesn't work. I wanted to use cattrs to serialize and deserialize slurmOrders,
        but on Virgo only python 3.6 is available. So I'll leave this for now and will implement more cleanly
        once I get access to python 3.7 on Virgo.
        """
        # slurmOrder = SlurmOrder()
        # print(f"This is the Slurm Order:\n{slurmOrder}\n")
        # print(f"This is the Slurm Order as dict:\n{slurmOrder.toJson()}\n")
        # jsonString = slurmOrder.toJson()
        # jsonObj = json.loads(str(jsonString))
        # jsonObj["cmd"] = "whoami"
        # jsonObj["env"] = {"AWESOME": "hellYes"}
        # jsonString = json.dumps(jsonObj)
        # print(f"Let's change it up a bit:\n{jsonString}\n")
        # print(
        #     f"now, backwards from Json:\n{SlurmOrder.fromJson(jsonString)}\n"
        # )
        sys.exit(0)

    def sendOrder(self, thisOrder: SlurmOrder, timeout: float = 5.0) -> bool:
        self.sendOrderBlocking(thisOrder)
        return True
        # proc = mp.Process(target=self.sendOrderBlocking, args=(thisOrder,))
        proc = th.Thread(target=self.sendOrderBlocking, args=(thisOrder,))
        proc.start()
        # writing alone shouldn't take long
        proc.join(timeout)

        if proc.is_alive():
            proc.terminate()
            raise TimeoutError(
                "Could not write to pipe! Is the agent listening?"
            )
        return True

    def sendOrderBlocking(self, thisOrder: SlurmOrder) -> None:
        with open(
            self.universalPipePath, "w", encoding="utf-8"
        ) as universalPipe:
            payload = thisOrder.__dict__
            json.dump(payload, universalPipe)

    def receiveOrder(self) -> SlurmOrder:
        with open(
            self.universalPipePath, "r", encoding="utf-8"
        ) as universalPipe:
            try:
                payload = json.load(universalPipe)
            except Exception:
                print(f"error parsing json order from pipe!")
                return None

        # python json's are actually dicts
        return SlurmOrder.fromDict(payload)


class Server(Agent):
    """
    checks:
    - if pipe exists and is pipe. if yes, return. else:
    - if its regular file, delete it. then:
    - if parent path for pipe exists, if not, creates it. then make pipe
    """

    def getUniqueServer(self) -> Path:
        """
        If multiple clients connect to one server, the server may get too many requests at the same time and may confuse the orders and results. This method returns the path to a new named pipe that is to be used by only one client. This way the order -> result structure is preserved and only one command is expected and executed.

        This is made this way so that each client instance can choose to get a unique server and doensn't have to care if it gets resutls for someone elses order.
        """

        # generate random suffix for named pipe name, 8 should do just fine
        letters = string.ascii_lowercase
        newNamedPipeSuffix = "-" + "".join(
            random.choice(letters) for _ in range(8)
        )
        newPipe = self.universalPipePath.with_name(
            self.universalPipePath.stem + newNamedPipeSuffix
        )

        # create new server just for this caller
        pid = os.fork()

        # child
        if pid > 0:
            self.universalPipePath = newPipe
            self.preparePipe()
            # print(f"Im child {pid}, server at {self.universalPipePath}")
            logging.info(f"created new server with pipe at {newPipe}")

        # both parent and child
        # print(f"Im parent {pid}, server at {self.universalPipePath}")
        return newPipe

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

    def mainLoop(self) -> None:
        """
        continouosly read from pipe and execute, write output and return codes back.
        """
        while True:
            # read entire pipe contents and try to deserialize json from it (close pipe!)
            thisOrder = self.receiveOrder()

            if thisOrder is not None:
                logging.info(
                    f"{datetime.datetime.now().isoformat(timespec='seconds')}: Received Order:"
                )
                logging.info(f"cmd: {thisOrder.cmd}")

                logging.debug(
                    f"{datetime.datetime.now().isoformat(timespec='seconds')}: Received Order:\n{thisOrder}\n"
                )

                # execute command as ordered
                returnOrder = self.execute(thisOrder)

                # return result
                logging.info(
                    f"{datetime.datetime.now().isoformat(timespec='seconds')}: Sent back result:"
                )
                logging.info(f"stdout: {returnOrder.stdout}")

                logging.debug(
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

        elif thisOrder.orderType == orderType.MAKE_UNQUE:
            thisOrder.stdout = str(self.getUniqueServer())
            thisOrder.returnCode = 0
            thisOrder.orderType = orderType.UNIQUE_CONFIRM
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
        # print(f'Received "exit" command. Exiting now.')
        self.deletePipes()
        # print("Have a nice day.")
        sys.exit(0)

    def run(self, debug: bool = False) -> None:

        if not debug:
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

        if debug:
            print("DEBUG MODE")
            logLevel = logging.DEBUG
        else:
            logLevel = logging.INFO

        # preapare log
        logging.basicConfig(
            filename=f"agentLog-{datetime.datetime.now().isoformat()}.log",
            level=logLevel,
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
        assert resultOrder.stdout == "ok", "Unexpected answer!"
        return True

    def __init__(self) -> None:
        if not os.path.exists(self.universalPipePath):
            raise Exception(
                f"named pipe not found at {self.universalPipePath}. Please start the agent first!"
            )
        if not stat.S_ISFIFO(os.stat(self.universalPipePath).st_mode):
            raise Exception(
                f"\nERROR! Path {self.universalPipePath} is not a pipe!"
            )

        self.checkConnection()

    def __enter__(self) -> "Client":
        """
        when the "with Client() as client" method is used, each client get's their own pipe.

        workflow:
        - send UNIQE command to default pipe
        - agent there will fork
        - original will send back order with name of new pipe
        - fork will send copy of that same order into new pipe (and will block until someone receives)
        - client will reveive one order with path of new pipe (from original server)
        - client must also receive copy order from new pipe, else fork will block (useful as confirmation)
        """

        makeUniqueOrder = SlurmOrder(orderType=orderType.MAKE_UNQUE)
        self.sendOrder(makeUniqueOrder)
        returnOrder = self.receiveOrder()

        if returnOrder.orderType == orderType.UNIQUE_CONFIRM:
            self.universalPipePath = Path(returnOrder.stdout)
        # race condition has occured
        else:
            raise ValueError("race condition in 'makeUniqueOrder'")

        confirmationOrder = self.receiveOrder()
        if confirmationOrder.orderType == orderType.UNIQUE_CONFIRM:
            logging.info(
                f"new pipe confirmation received: {confirmationOrder}"
            )
        else:
            raise ValueError("returned confirmation is invalid!")
        return self

    def __exit__(self, type, value, traceback) -> None:
        self.sendOrder(SlurmOrder(orderType=orderType.EXIT))


if __name__ == "__main__":
    thisServer = Server()

    if "--debug" in sys.argv:
        thisServer.run(debug=True)
    else:
        thisServer.run()

    #! Uncomment this to see some basic examples
    # Agent.SlurmOrderExamples()
