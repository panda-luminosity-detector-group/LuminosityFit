# cSpell:ignore slurm,sbatch,squeue,CPUs,Popen

import os
import subprocess
import time
from typing import Callable, Optional

from .agent import Client, SlurmOrder
from .cluster import Job, JobHandler, JobResourceRequest


def _stringify(job_resource_request: JobResourceRequest):
    resource_request = (
        f" -N {job_resource_request.number_of_nodes}"
        + f" -n 1 -c {job_resource_request.processors_per_node}"
        + f" --mem-per-cpu={int(job_resource_request.memory_in_mb)}mb"
        + " --time="
        + f"{_format_walltime(job_resource_request.walltime_in_minutes)}"
    )
    # if job_resource_request.node_scratch_filesize_in_mb > 0:
    #    resource_request += ' --tmp=' + \
    #        str(job_resource_request.node_scratch_filesize_in_mb)
    return resource_request


def _format_walltime(walltime_in_minutes):
    day_in_minutes = 60 * 24
    days = int(int(walltime_in_minutes) / day_in_minutes)
    remaining_minutes = int(walltime_in_minutes) % day_in_minutes
    hours = int(remaining_minutes / 60)
    remaining_minutes = remaining_minutes % 60
    walltime_string = str(days) + "-"
    if hours < 10:
        walltime_string += "0"
    walltime_string += str(hours) + ":"
    if remaining_minutes < 10:
        walltime_string += "0"
    walltime_string += str(remaining_minutes)
    return walltime_string


def _create_array_string(job: Job) -> str:
    array_indices = job.array_indices
    if len(array_indices) > 1:
        indices_string = ""
        temp_range_start = -1
        previous_index = -1
        array_indices.sort()
        for index in array_indices:
            if temp_range_start == -1:
                temp_range_start = index
            elif index > previous_index + 1:
                if previous_index - temp_range_start > 1:
                    indices_string += (
                        str(temp_range_start) + "-" + str(previous_index) + ","
                    )
                else:
                    if temp_range_start != previous_index:
                        indices_string += str(temp_range_start) + ","
                    indices_string += str(previous_index) + ","
                temp_range_start = index
            previous_index = index
        if temp_range_start < index:
            indices_string += (
                str(temp_range_start) + "-" + str(array_indices[-1])
            )
        else:
            indices_string += str(array_indices[-1])
        return " --array=" + indices_string
    elif len(array_indices) == 1:
        job.exported_user_variables["SLURM_ARRAY_TASK_ID"] = array_indices[0]
        return ""
    else:
        raise ValueError("Number of jobs is zero!")


class SlurmJobHandler(JobHandler):
    def __init__(
        self,
        partition: str,
        account: Optional[str] = None,
        constraints: Optional[str] = None,
        job_preprocessor: Optional[Callable[[Job], Job]] = None,
    ) -> None:
        self.__partition = partition
        self.__account = account
        self.__constraints = constraints
        self.__job_preprocessor = job_preprocessor
        self.__useSlurmAgent__ = True

    def get_active_number_of_jobs(self) -> int:
        """Check users current number of running and queued jobs."""
        bashcommand = "squeue -u $USER -o %C | sed 's/CPUS/0/' | awk '{s+=$1} END {print s}'"

        # attention! sometimes this reads back the empty string, which shouldn't happen.
        # I don't know why, maybe because two orders came at the same time.
        # for now, just wait 30 seconds and try again, up to 3 times. Then fail.

        attemptCounter = 0
        while attemptCounter < 3:

            if self.__useSlurmAgent__:
                client = Client()
                thisOrder = SlurmOrder()
                thisOrder.cmd = bashcommand
                thisOrder.runShell = True
                # default output to zero, doesn't work otherwise
                thisOrder.stdout = "0"
                thisOrder.env = os.environ.copy()
                client.sendOrder(thisOrder, 30)  # this may take some time
                resultOrder = client.receiveOrder()
                resultOut = resultOrder.stdout

            else:
                returnvalue = subprocess.Popen(
                    bashcommand, shell=True, stdout=subprocess.PIPE, text=True
                )
                resultOut, _ = returnvalue.communicate()

            if resultOut == "":
                time.sleep(30)
                attemptCounter += 1
                continue

            return int(resultOut)

        # if we've reached this point, all is lost
        raise ValueError("Error getting number of running jobs!")

    def submit(self, job: Job) -> int:
        if self.__job_preprocessor:
            job = self.__job_preprocessor(job)

        bashcommand = (
            "sbatch"
            + (f" -A {self.__account}" if self.__account else "")
            + f" -p {self.__partition}"
            + (
                f" --constraint={self.__constraints}"
                if self.__constraints
                else ""
            )
        )

        bashcommand += _create_array_string(job)

        bashcommand += (
            f" --job-name={job.name}"
            + _stringify(job.resource_request)
            + f" --output={job.logfile_url}"
        )
        # export variables
        bashcommand += " --export=ALL,"
        for name, value in job.exported_user_variables.items():
            bashcommand += f"{name}={value},"

        bashcommand = (
            bashcommand[:-1]
            + " "
            + job.additional_flags
            + " "
            + job.application_url
        )
        if self.__useSlurmAgent__:
            client = Client()
            thisOrder = SlurmOrder()
            thisOrder.cmd = bashcommand
            thisOrder.runShell = True
            thisOrder.env = os.environ.copy()
            client.sendOrder(thisOrder)
            resultOrder = client.receiveOrder()
            returnCode = resultOrder.returnCode
            return int(returnCode)

        else:
            return subprocess.call(bashcommand, shell=True)
