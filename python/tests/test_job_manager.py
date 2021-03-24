from time import sleep
from typing import Iterable, List

import pytest
from lumifit.cluster import (
    ClusterJobManager,
    Job,
    JobHandler,
    JobResourceRequest,
)


class JobHandlerMock(JobHandler):
    def __init__(self) -> None:
        self.__number_of_active_jobs = 0
        self.__submit_calls: List[Job] = []

    def create_submit_commands(self, job: Job) -> Iterable[str]:
        self.__submit_calls.append(job)
        return ['echo "hello world"']

    def get_active_number_of_jobs(self) -> int:
        return self.__number_of_active_jobs

    def set_number_of_active_jobs(self, active_jobs_count: int) -> None:
        self.__number_of_active_jobs = active_jobs_count

    @property
    def submit_calls(self) -> List[Job]:
        return self.__submit_calls


@pytest.mark.timeout(20)
def test_job_manager():
    job_handler_mock = JobHandlerMock()
    job_manager = ClusterJobManager(
        job_handler=job_handler_mock,
        time_to_sleep_after_submission=0,
        resubmit_wait_time_in_seconds=0,
    )
    resource_request = JobResourceRequest(1)

    dummy_job = Job(
        resource_request,
        application_url="dummy.exe",
        name="dummy exe",
        logfile_url="dummy.log",
        array_indices=[1],
    )
    job_manager.append(dummy_job)
    job_manager.append(dummy_job)
    job_manager.append(dummy_job)
    while job_manager.is_active():
        sleep(1)
    assert job_handler_mock.submit_calls == 3 * [dummy_job]
    job_manager.append(dummy_job)
    job_manager.append(dummy_job)
    while job_manager.is_active():
        sleep(1)
    assert job_handler_mock.submit_calls == 5 * [dummy_job]
