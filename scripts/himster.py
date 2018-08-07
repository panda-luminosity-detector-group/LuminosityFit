import os
import subprocess
from time import time, sleep

batch_command = 'sbatch'


def get_num_jobs_on_himster():
    '''
    check number of jobs currently running or in queue on himster
    (only this user, so you!)
    '''
    bashcommand = 'squeue -u $USER -t RUNNING -o %C | sed \'s/CPUS/0/\' | awk \'{s+=$1} END {print s}\''
    returnvalue = subprocess.Popen(
        bashcommand, shell=True, stdout=subprocess.PIPE)
    out, err = returnvalue.communicate()
    return int(out)


def is_cluster_environment():
    return is_executable(batch_command)


def is_executable(exe_name):
    for path in os.environ["PATH"].split(os.pathsep):
        path = path.strip('"')
        exe_url = os.path.join(path, exe_name)
        if os.path.isfile(exe_url) and os.access(exe_url, os.X_OK):
            return True
    return False


def make_test_resource_request():
    test_rr = JobResourceRequest(30)
    test_rr.partition = 'devel'
    test_rr.number_of_nodes = 1
    test_rr.processors_per_node = 1
    test_rr.memory_in_mb = 500
    test_rr.virtual_memory_in_mb = 500
    test_rr.node_scratch_filesize_in_mb = 0
    return test_rr


class JobResourceRequest:
    def __init__(self, walltime_in_minutes):
        self.walltime_string = self.format_walltime(walltime_in_minutes)
        self.partition = 'himster2_exp'
        self.number_of_nodes = 1
        self.processors_per_node = 1
        self.memory_in_mb = 1000
        self.virtual_memory_in_mb = 1000
        self.node_scratch_filesize_in_mb = 0

    def format_walltime(self, walltime_in_minutes):
        day_in_minutes = 60 * 24
        days = int(int(walltime_in_minutes) / day_in_minutes)
        remaining_minutes = int(walltime_in_minutes) % day_in_minutes
        hours = int(remaining_minutes / 60)
        remaining_minutes = remaining_minutes % 60
        walltime_string = str(days) + '-'
        if hours < 10:
            walltime_string += '0'
        walltime_string += str(hours) + ':'
        if remaining_minutes < 10:
            walltime_string += '0'
        walltime_string += str(remaining_minutes)
        return walltime_string

    def get_submit_string(self):
        # hyperthreading on himster2 virtually increases cpu count by a
        # factor of 2. we want to allocate only real cpus, hence the
        # factors of 2 in the code below
        resource_request = ' -N ' + str(self.number_of_nodes) \
            + ' -n 1 -c ' + str(2 * self.processors_per_node) \
            + ' --mem-per-cpu=' + str(int(self.memory_in_mb / 2)) + 'mb' \
            + ' --time=' + self.walltime_string
        # if self.node_scratch_filesize_in_mb > 0:
        #    resource_request += ' --tmp=' + \
        #        str(self.node_scratch_filesize_in_mb)
        return resource_request


class Job:
    def __init__(self,
                 job_resource_request,
                 application_url,
                 jobname,
                 logfile_url):
        self.resource_request = job_resource_request
        self.application_url = str(application_url)
        self.jobname = str(jobname)
        self.logfile_url = str(logfile_url)
        self.job_array_index_bundles = []
        self.exported_user_variables = {}

    def set_job_array_indices(self, array_indices):
        if not isinstance(array_indices, list):
            raise TypeError("array_indices must be of type list")
        if not array_indices:
            raise ValueError("array_indices is empty! This can occur, when all"
                             " of the output files already exist and are above"
                             " the defined threshold filesize. Please use the"
                             " --force flag to overwrite the output.")
        self.job_array_index_bundles = [array_indices]

    def add_exported_user_variable(self, name, value):
        self.exported_user_variables[str(name)] = str(value)

    def create_array_string(self, array_indices):
        if len(array_indices) > 1:
            indices_string = ''
            temp_range_start = -1
            previous_index = -1
            array_indices.sort()
            for index in array_indices:
                if temp_range_start == -1:
                    temp_range_start = index
                elif index > previous_index + 1:
                    if previous_index - temp_range_start > 1:
                        indices_string += str(temp_range_start) + \
                            '-' + str(previous_index) + ','
                    else:
                        if temp_range_start != previous_index:
                            indices_string += str(temp_range_start) + ','
                        indices_string += str(previous_index) + ','
                    temp_range_start = index
                previous_index = index
            if temp_range_start < index:
                indices_string += str(temp_range_start) + \
                    '-' + str(array_indices[-1])
            else:
                indices_string += str(array_indices[-1])
            return ' --array=' + indices_string
        elif len(array_indices) == 1:
            self.add_exported_user_variable(
                'SLURM_ARRAY_TASK_ID', array_indices[0])
            return ''
        else:
            raise ValueError("Number of jobs is zero!")

    def create_bash_commands(self):
        bashcommand_list = []
        for array_indices in self.job_array_index_bundles:
            bashcommand = batch_command + ' -A m2_him_exp -p ' + \
                self.resource_request.partition + \
                ' --constraint=\"skylake,mhz-2101\"'

            bashcommand += self.create_array_string(array_indices)

            bashcommand += ' --job-name=' + self.jobname + \
                self.resource_request.get_submit_string() + ' --output=' \
                + self.logfile_url
            # export variables
            bashcommand += ' --export=ALL,'
            for name, value in self.exported_user_variables.items():
                bashcommand += name + '="' + value + '",'

            bashcommand = bashcommand[:-1] + ' ' + self.application_url
            # print(bashcommand)
            bashcommand_list.append(bashcommand)
        return bashcommand_list


class HimsterJobManager:
    def __init__(self, himster_total_job_threshold=1600,
                 resubmit_wait_time_in_seconds=1800):
        self.job_command_list = []
        # user total job threshold
        self.himster_total_job_threshold = himster_total_job_threshold
        # sleep time when total job threshold is reached in seconds
        self.resubmit_wait_time_in_seconds = resubmit_wait_time_in_seconds

    def manage_jobs(self):
        failed_submit_commands = {}
        while self.job_command_list:
            print("checking if total job threshold is reached...")
            bashcommand = self.job_command_list.pop(0)
            if get_num_jobs_on_himster() < self.himster_total_job_threshold:
                print("Nope, trying to submit job...")
                returncode = subprocess.call(bashcommand, shell=True)
                if returncode > 0:
                    resubmit = True
                    if bashcommand in failed_submit_commands:
                        if time() < (failed_submit_commands[bashcommand]
                                     + self.resubmit_wait_time_in_seconds):
                            print(bashcommand)
                            print(
                                "something is wrong with this submit command."
                                " Skipping...")
                            resubmit = False
                    else:
                        print(
                            "Submit failed! Appending job to resubmit list "
                            "for later submission...")
                        failed_submit_commands[bashcommand] = time()

                    if resubmit:
                        # put the command back into the list
                        self.job_command_list.insert(0, bashcommand)
                else:
                    # sleep 3 sec to make the queue changes active
                    sleep(3)
            else:
                # put the command back into the list
                self.job_command_list.insert(0, bashcommand)
                print('Yep, we have currently have ' +
                      str(len(self.job_command_list)) +
                      ' jobs waiting in queue!')
                # and sleep for some time
                print('Waiting for '
                      + str(self.resubmit_wait_time_in_seconds / 60)
                      + ' min and then trying a resubmit...')
                sleep(self.resubmit_wait_time_in_seconds)

    def submit_jobs_to_himster(self, job_list):
        if is_cluster_environment():
            print('This is a cluster environment. Adding jobs to queue list!')
            for job in job_list:
                for bashcommand in job.create_bash_commands():
                    self.job_command_list.append(bashcommand)

        else:
            print('This is not a cluster environment! Please make sure this '
                  'script is executed on a cluster environment!')
