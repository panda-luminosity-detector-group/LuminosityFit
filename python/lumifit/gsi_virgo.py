# cSpell:ignore slurm, cpus

from .cluster import Job
from .slurm import SlurmJobHandler


def create_virgo_job_handler(partition: str) -> SlurmJobHandler:
    def job_preprocessor(job: Job) -> Job:
        # at gsi virgo to run containerized jobs
        # the -- option has to be used
        job.additional_flags = "--"
        return job

    return SlurmJobHandler(
        partition=partition,
        job_preprocessor=job_preprocessor,
    )
