# cSpell:ignore slurm,sbatch,squeue,CPUs,Popen

import os
import re
import subprocess
import time
from typing import Callable, Optional, Tuple

from lumifit.agent import Client, SlurmOrder
from lumifit.cluster import Job, JobHandler, JobResourceRequest


def _stringify(job_resource_request: JobResourceRequest):
    resource_request = (
        f" -N {job_resource_request.number_of_nodes}"
        + f" -n 1 -c {job_resource_request.processors_per_node}"
        + f" --mem-per-cpu={int(job_resource_request.memory_in_mb)}mb"
        + " --time="
        + f"{_format_walltime(job_resource_request.walltime_in_minutes)}"
    )
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
                    indices_string += str(temp_range_start) + "-" + str(previous_index) + ","
                else:
                    if temp_range_start != previous_index:
                        indices_string += str(temp_range_start) + ","
                    indices_string += str(previous_index) + ","
                temp_range_start = index
            previous_index = index
        if temp_range_start < index:
            indices_string += str(temp_range_start) + "-" + str(array_indices[-1])
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

    def get_active_number_of_jobs(self, jobID: Optional[int] = None) -> int:
        """
        Check users current number of running and queued jobs.
        If no jobID is given, this counts allocated cpus, not jobs.
        """

        if jobID is None:
            bashCommand = "squeue -u $USER -o %C | sed 's/CPUS/0/' | awk '{s+=$1} END {print s}'"
        else:
            bashCommand = f"squeue -u $USER -h --state R,PD --job {jobID}  | wc -l"

        # attention! sometimes this reads back the empty string, which shouldn't happen.
        # I don't know why, maybe because two orders came at the same time.
        # for now, just wait 30 seconds and try again, up to 3 times. Then fail.

        attemptCounter = 0
        while attemptCounter < 3:
            if self.__useSlurmAgent__:
                with Client() as client:
                    thisOrder = SlurmOrder()
                    thisOrder.cmd = bashCommand
                    thisOrder.runShell = True
                    # default output to zero, doesn't work otherwise
                    thisOrder.stdout = "0"
                    thisOrder.env = os.environ.copy()
                    client.sendOrder(thisOrder, 30)  # this may take some time
                    resultOrder = client.receiveOrder()
                    resultOut = resultOrder.stdout

            else:
                returnValue = subprocess.Popen(bashCommand, shell=True, stdout=subprocess.PIPE, text=True)
                resultOut, _ = returnValue.communicate()

            if resultOut == "":
                time.sleep(30)
                attemptCounter += 1
                continue

            return int(resultOut)

        # if we've reached this point, all is lost
        raise ValueError("Error getting number of running jobs!")

    def submit(self, job: Job) -> Tuple[int, int]:
        """
        Submits a job to the work manager (probably SLURM)
        and returns the returnCode from the submit command
        (mostly 0 if success or 1 if failed)
        and the job array ID, if successful
        """
        if self.__job_preprocessor:
            job = self.__job_preprocessor(job)

        bashCommand = (
            "sbatch"
            + (f" -A {self.__account}" if self.__account else "")
            + f" -p {self.__partition}"
            + (f" --constraint={self.__constraints}" if self.__constraints else "")
        )

        bashCommand += _create_array_string(job)

        bashCommand += f" --job-name={job.name}" + _stringify(job.resource_request) + f" --output={job.logfile_url}"
        # export variables
        bashCommand += " --export=ALL,"
        for name, value in job.exported_user_variables.items():
            bashCommand += f"{name}={value},"

        bashCommand = bashCommand[:-1] + " " + job.additional_flags + " " + job.application_url
        if self.__useSlurmAgent__:
            with Client() as client:
                # todo handle jobArray ID as well
                thisOrder = SlurmOrder()
                thisOrder.cmd = bashCommand
                thisOrder.runShell = True
                thisOrder.env = os.environ.copy()
                client.sendOrder(thisOrder)
                resultOrder = client.receiveOrder()
                returnCode = resultOrder.returnCode

                if returnCode != 0:
                    jobArrayID: int = 0
                    raise RuntimeError("Job submission failed!")
                else:
                    returnMessage = resultOrder.stdout
                    # will be something like
                    # Submitted batch job 14049737

                    # regex parse the job ID
                    match = re.search(r"Submitted batch job (\d+)", returnMessage)
                    if match is not None:
                        jobArrayID = int(match.group(1))
                    else:
                        raise RuntimeError("Job submission failed in a weird way! Return code was 0, but no job ID was found!")

                if not isinstance(jobArrayID, int):
                    raise RuntimeError(f"Job submission failed in a weird way! Return code was 0, but I could not parse the job ID {jobArrayID}!")
                return int(returnCode), jobArrayID

        else:
            # todo handle jobArray ID as well
            return subprocess.call(bashCommand, shell=True), 0
