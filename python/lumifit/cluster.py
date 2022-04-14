import os
import subprocess
import threading
from abc import abstractmethod
from time import sleep, time
from typing import Any, Dict, List

import attr


@attr.s(hash=True)
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


@attr.s(hash=False)
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
    def submit(self, job: Job) -> int:
        pass

    @abstractmethod
    def get_active_number_of_jobs(self) -> int:
        pass


class DebugJobHandler(JobHandler):
    def submit(self, job: Job) -> int:
        print(job.exported_user_variables)
        my_env = os.environ.copy()
        my_env.update(job.exported_user_variables)
        print(job.application_url)
        return subprocess.call(job.application_url, env=my_env)

    def get_active_number_of_jobs(self) -> int:
        return 0


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
        self.__manage_thread = threading.Thread(target=self.__manage_jobs)
        self.__lock = threading.Lock()

    def __manage_jobs(self):
        failed_jobs: Dict[Job, float] = {}

        while self.__jobs:
            print("checking if total job threshold is reached...")
            if (
                self.__job_handler.get_active_number_of_jobs()
                < self.__total_job_threshold
            ):
                print("Nope, trying to submit job...")
                current_job = None
                with self.__lock:
                    current_job = self.__jobs.pop(0)

                print(current_job)
                returncode = self.__job_handler.submit(current_job)
                if returncode > 0:
                    resubmit = True
                    if current_job in failed_jobs:
                        if time() < (
                            failed_jobs[current_job]
                            + self.__resubmit_wait_time_in_seconds
                        ):
                            print(current_job)
                            print(
                                "something is wrong with this job. Skipping..."
                            )
                            resubmit = False
                    else:
                        print(
                            "Submit failed! Appending job to resubmit"
                            " list for later submission..."
                        )
                        failed_jobs[current_job] = time()

                    if resubmit:
                        # put the command back into the list
                        with self.__lock:
                            self.__jobs.insert(0, current_job)
                else:
                    # sleep a bit to make the queue changes active
                    sleep(self.__time_to_sleep_after_submission)
            else:
                with self.__lock:
                    print(
                        "Yep, we have currently have "
                        + str(len(self.__jobs))
                        + " jobs waiting in queue!"
                    )
                # and sleep for some time
                print(
                    "Waiting for "
                    + str(self.__resubmit_wait_time_in_seconds / 60)
                    + " min and then trying a resubmit..."
                )
                sleep(self.__resubmit_wait_time_in_seconds)
        print("\n\nAll jobs submitted!\n\n")

    def is_active(self) -> bool:
        return self.__manage_thread.is_alive()

    def append(self, job: Job):
        """Append Job to this managers list of pending jobs.

        The manager submits the jobs as soon as there is capacity on the cluster,
        no need to manually submit jobs!
        """
        with self.__lock:
            self.__jobs.append(job)

        if not self.__manage_thread.is_alive():
            self.__manage_thread = threading.Thread(target=self.__manage_jobs)
            self.__manage_thread.start()
