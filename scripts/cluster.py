import os
import subprocess
from time import time, sleep
from abc import abstractmethod
import attr
from typing import Any, Iterable, Tuple, Dict


@attr.s
class JobResourceRequest:
    walltime_in_minutes: int = attr.ib()
    number_of_nodes: int = attr.ib(default=1)
    processors_per_node: int = attr.ib(default=1)
    memory_in_mb: int = attr.ib(default=1000)
    virtual_memory_in_mb: int = attr.ib(default=1000)
    node_scratch_filesize_in_mb: int = attr.ib(default=0)
    is_dev_job: bool = attr.ib(default=False)


def make_test_job_resource_request() -> JobResourceRequest:
    return JobResourceRequest(
        walltime_in_minutes=30,
        number_of_nodes=1,
        processors_per_node=1,
        memory_in_mb=500,
        virtual_memory_in_mb=500,
        node_scratch_filesize_in_mb=0,
    )


@attr.s
class Job:
    def _validate_job_array_indices(instance, attribute, value):
        if not isinstance(value, list):
            raise TypeError("Job array indices must be of type list")
        if not value:
            raise ValueError("No job array indices given!")

    resource_request: JobResourceRequest = attr.ib()
    application_url: str = attr.ib()
    name: str = attr.ib()
    logfile_url: str = attr.ib()
    array_indices = attr.ib(validator=_validate_job_array_indices)
    exported_user_variables: Dict[str, Any] = attr.ib(factory=dict)


class JobHandler:
    @abstractmethod
    def create_submit_commands(
        self, job: Job, debug=False
    ) -> Iterable[Tuple[str, ...]]:
        pass

    @abstractmethod
    def get_active_number_of_jobs(self) -> int:
        pass


class ClusterJobManager:
    def __init__(
        self,
        job_handler: JobHandler,
        total_job_threshold: int = 30000,
        resubmit_wait_time_in_seconds: int = 1800,
        debug: bool = False,
    ) -> None:
        self.__job_handler = job_handler
        self.__jobs = []
        self.__total_job_threshold = total_job_threshold
        # sleep time when total job threshold is reached in seconds
        self.__resubmit_wait_time_in_seconds = resubmit_wait_time_in_seconds
        self.__debug = debug

    def manage_jobs(self):
        job_command_list = []
        for job in self.__jobs:
            job_command_list.extend(
                self.__job_handler.create_submit_commands(job=job)
            )
        failed_submit_commands = {}
        if self.__debug:
            for cmd in job_command_list:
                my_env = os.environ.copy()
                bashcommand = cmd
                if len(cmd) == 2:
                    my_env.update(cmd[1])
                    bashcommand = cmd[0]
                returncode = subprocess.call(bashcommand, env=my_env)
        else:
            while job_command_list:
                print("checking if total job threshold is reached...")
                bashcommand = job_command_list.pop(0)
                if (
                    self.__job_handler.get_active_number_of_jobs()
                    < self.__total_job_threshold
                ):
                    print("Nope, trying to submit job...")
                    returncode = subprocess.call(bashcommand, shell=True)
                    if returncode > 0:
                        resubmit = True
                        if bashcommand in failed_submit_commands:
                            if time() < (
                                failed_submit_commands[bashcommand]
                                + self.__resubmit_wait_time_in_seconds
                            ):
                                print(bashcommand)
                                print(
                                    "something is wrong with this submit"
                                    " command. Skipping..."
                                )
                                resubmit = False
                        else:
                            print(
                                "Submit failed! Appending job to resubmit"
                                " list for later submission..."
                            )
                            failed_submit_commands[bashcommand] = time()

                        if resubmit:
                            # put the command back into the list
                            job_command_list.insert(0, bashcommand)
                    else:
                        # sleep 3 sec to make the queue changes active
                        sleep(3)
                else:
                    # put the command back into the list
                    job_command_list.insert(0, bashcommand)
                    print(
                        "Yep, we have currently have "
                        + str(len(job_command_list))
                        + " jobs waiting in queue!"
                    )
                    # and sleep for some time
                    print(
                        "Waiting for "
                        + str(self.__resubmit_wait_time_in_seconds / 60)
                        + " min and then trying a resubmit..."
                    )
                    sleep(self.__resubmit_wait_time_in_seconds)

    def append(self, job: Job):
        self.__jobs.append(job)
