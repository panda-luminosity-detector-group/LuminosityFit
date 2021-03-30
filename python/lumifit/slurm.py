# cSpell:ignore slurm,sbatch,squeue,CPUs,Popen

import subprocess
from typing import Callable, List, Optional

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

    def get_active_number_of_jobs(self) -> int:
        """Check users current number of running and queued jobs."""
        bashcommand = (
            "squeue -u $USER -o %C | sed 's/CPUS/0/'"
            + " | awk '{s+=$1} END {print s}'"
        )
        returnvalue = subprocess.Popen(
            bashcommand, shell=True, stdout=subprocess.PIPE
        )
        out, err = returnvalue.communicate()
        return int(out)

    def create_submit_commands(self, job: Job) -> List[str]:
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
        return [bashcommand]
