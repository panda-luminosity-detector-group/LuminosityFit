# cSpell:ignore slurm, cpus

from lumifit.cluster import Job
from lumifit.slurm import SlurmJobHandler


def create_himster_job_handler(partition: str, account: str = "m2_him_exp") -> SlurmJobHandler:
    def job_preprocessor(job: Job) -> Job:
        # hyperthreading on himster2 virtually increases cpu count by a
        # factor of 2. we want to allocate only real cpus, hence the
        # factors of 2 in the code below
        job.resource_request.processors_per_node *= 2
        return job

    return SlurmJobHandler(
        partition=partition,
        account=account,
        constraints="skylake",
        job_preprocessor=job_preprocessor,
    )
