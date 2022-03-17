import os
import subprocess
import threading
from abc import abstractmethod
from time import sleep, time
from typing import Any, Dict, Iterable, List

import attr


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
    additional_flags: str = attr.ib(default="")


class JobHandler:
    @abstractmethod
    def create_submit_commands(self, job: Job) -> Iterable[str]:
        pass

    @abstractmethod
    def get_active_number_of_jobs(self) -> int:
        pass


class ClusterJobManager:
    """Manages submission of jobs on a cluster environment.

    The jobs are pending in a queue until the number of jobs running on the
    cluster is below a certain threshold.
    """

    def __init__(
        self,
        job_handler: JobHandler,
        total_job_threshold: int = 1000,
        resubmit_wait_time_in_seconds: int = 1800,
        debug: bool = False,
        time_to_sleep_after_submission: int = 3,
    ) -> None:
        if not isinstance(job_handler, JobHandler):
            raise TypeError(
                f"job_handler must be of type JobHandler, got {job_handler}!"
            )
        self.__job_handler = job_handler
        self.__jobs: List[Job] = []
        self.__total_job_threshold = total_job_threshold
        # sleep time when total job threshold is reached in seconds
        self.__resubmit_wait_time_in_seconds = resubmit_wait_time_in_seconds
        self.__time_to_sleep_after_submission = time_to_sleep_after_submission
        self.__debug = debug
        self.__manage_thread = threading.Thread(target=self.__manage_jobs)
        self.__lock = threading.Lock()

    def __manage_jobs(self):
        if self.__debug:
            bashcommand_list = [
                (job.application_url, job.exported_user_variables)
                for job in self.__jobs
            ]
            for cmd in bashcommand_list:
                my_env = os.environ.copy()
                my_env.update(cmd[1])
                bashcommand = cmd[0]

                # TODO: use some sort of switch here instead                    
                if False:
                    returncode = subprocess.call(bashcommand, env=my_env)
                else:
                    with open('orderPipe') as orderPipe:
                        # open this pipe before committing orders, else deadlock!
                        with open('returncodePipe', 'r') as returnPipe:

                            # TODO: also handle env variables!
                            orderPipe.write(bashcommand)
                            returncode = int(returnPipe.readline())                

        else:
            job_command_queue: List[str] = []
            failed_submit_commands: Dict[str, float] = {}

            while True:
                with self.__lock:
                    for job in self.__jobs:
                        job_command_queue.extend(
                            self.__job_handler.create_submit_commands(job=job)
                        )

                    self.__jobs.clear()

                if not job_command_queue:
                    break  # finished and stop thread

                print("checking if total job threshold is reached...")
                if (
                    self.__job_handler.get_active_number_of_jobs()
                    < self.__total_job_threshold
                ):
                    print("Nope, trying to submit job...")
                    bashcommand = job_command_queue.pop(0)

                    # TODO: use some sort of switch here instead                    
                    if False:
                        returncode = subprocess.call(bashcommand, shell=True)
                    else:
                        with open('orderPipe') as orderPipe:

                            # open this pipe before committing orders, else deadlock!
                            with open('returncodePipe', 'r') as returnPipe:
                                orderPipe.write(bashcommand)
                                returncode = int(returnPipe.readline())
                    
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
                            job_command_queue.insert(0, bashcommand)
                    else:
                        # sleep a bit to make the queue changes active
                        sleep(self.__time_to_sleep_after_submission)
                else:
                    print(
                        "Yep, we have currently have "
                        + str(len(job_command_queue))
                        + " jobs waiting in queue!"
                    )
                    # and sleep for some time
                    print(
                        "Waiting for "
                        + str(self.__resubmit_wait_time_in_seconds / 60)
                        + " min and then trying a resubmit..."
                    )
                    sleep(self.__resubmit_wait_time_in_seconds)

    def is_active(self) -> bool:
        return self.__manage_thread.is_alive()

    def append(self, job: Job):
        with self.__lock:
            self.__jobs.append(job)

        if not self.__manage_thread.is_alive():
            self.__manage_thread = threading.Thread(target=self.__manage_jobs)
            self.__manage_thread.start()
