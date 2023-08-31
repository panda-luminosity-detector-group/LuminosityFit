import os
import subprocess
import threading
from abc import abstractmethod
from time import sleep
from typing import Any, Dict, List, Optional, Tuple

import attr
from attrs import define


@define
class JobResourceRequest:
    walltime_in_minutes: int = 0
    number_of_nodes: int = 1
    processors_per_node: int = 1
    memory_in_mb: int = 2000
    virtual_memory_in_mb: int = 2000
    node_scratch_filesize_in_mb: int = 0
    is_dev_job: bool = False


def make_test_job_resource_request() -> JobResourceRequest:
    return JobResourceRequest(
        walltime_in_minutes=30,
        number_of_nodes=1,
        processors_per_node=1,
        memory_in_mb=500,
        virtual_memory_in_mb=500,
        node_scratch_filesize_in_mb=0,
        is_dev_job=False,
    )


# TODO: rewrite with define
@attr.s(hash=False)
class Job:
    def _validate_job_array_indices(self, attribute: Any, value: Any) -> None:
        if not isinstance(value, list):
            raise TypeError("Job array indices must be of type list")
        if not value:
            raise ValueError("No job array indices given!")

    resource_request: JobResourceRequest = attr.ib()
    application_url: str = attr.ib()
    name: str = attr.ib()
    logfile_url: str = attr.ib()
    array_indices: List[int] = attr.ib(validator=_validate_job_array_indices)
    exported_user_variables: Dict[str, Any] = attr.ib(factory=dict)
    additional_flags: str = attr.ib(default="")


class JobHandler:
    @abstractmethod
    def submit(self, job: Job) -> Tuple[int, int]:
        pass

    @abstractmethod
    def get_active_number_of_jobs(self, jobID: Optional[int] = None) -> int:
        pass


class DebugJobHandler(JobHandler):
    def submit(self, job: Job) -> Tuple[int, int]:
        """
        Debug handler executes locally, not via Slurm.
        Job Array ID is always -1.
        """
        print(job.exported_user_variables)
        my_env = os.environ.copy()
        my_env.update(job.exported_user_variables)
        print(job.application_url)

        # job array ID is always -1, because it's not a cluster
        return subprocess.call(job.application_url, env=my_env), -1

    def get_active_number_of_jobs(self) -> int:
        return 0


class ClusterJobManager:
    """
    Manages submission of jobs on a cluster environment.

    The manager blocks as soon as a job should be submitted (enqueued) and only returns once the submission
    was successful. No other error handling here, either the job was submitted successfully, or
    the other threads have to wait anyway.

    Manager should be thread safe, but do not create multiple instances.
    """

    def __init__(
        self,
        job_handler: JobHandler,
        total_job_threshold: int = 1000,
        resubmit_wait_time_in_seconds: int = 1800,
    ) -> None:
        if not isinstance(job_handler, JobHandler):
            raise TypeError(f"job_handler must be of type JobHandler, got {job_handler}!")
        self.__job_handler = job_handler
        self.__total_job_threshold = total_job_threshold
        self.__resubmit_wait_time_in_seconds = resubmit_wait_time_in_seconds
        self.__lock = threading.Lock()

    def get_active_number_of_jobs(self, jobID: int) -> int:
        """
        Calls the equivalent function in the job handler.
        """
        with self.__lock:
            return self.__job_handler.get_active_number_of_jobs(jobID)

    def enqueue(self, job: Job) -> int:
        """
        Enqueues a job to slurm.

        The manager submits the jobs as soon as there is capacity on the cluster,
        no need to manually submit jobs!

        returns job (array) ID
        """
        with self.__lock:
            while True:
                if self.__job_handler.get_active_number_of_jobs() < self.__total_job_threshold:
                    triesCounter = 0
                    while triesCounter < 3:
                        returncode, jobArrayID = self.__job_handler.submit(job)
                        if returncode > 0:
                            triesCounter += 1
                            print("Submit failed! Waiting 15 seconds and then trying again...")
                            sleep(15)

                        else:
                            print(f"Job submitted as id {jobArrayID}, waiting 3 seconds.")
                            # it seems SLURM takes a few seconds to update the queue, so we wait a bit
                            sleep(3)
                            return jobArrayID

                    raise RuntimeError("Job submission failed 3 times in a row, aborting...")

                else:
                    print("Yep, we have currently have too many jobs waiting in queue!")
                    # and sleep for some time
                    print("Waiting for " + str(self.__resubmit_wait_time_in_seconds / 60) + " min and then trying a resubmit...")
                    sleep(self.__resubmit_wait_time_in_seconds)
