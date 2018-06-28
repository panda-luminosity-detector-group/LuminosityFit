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


def get_exe_path(exe_name):
    exe_url = ''
    found = False
    for path in os.environ["PATH"].split(os.pathsep):
        path = path.strip('"')
        exe_url = os.path.join(path, exe_name)
        if os.path.isfile(exe_url):
            found = True
            break
    if not found:
        exe_url = os.path.join(os.getcwd(), exe_name)
        if not os.path.isfile(exe_url):
            raise FileNotFoundError(
                'Could not find executable ' + str(exe_name))
    if not os.access(exe_url, os.X_OK):
        raise PermissionError(
            'Please give ' + str(exe_url) + ' execute permission!')

    return exe_url


def is_cluster_environment():
    return is_executable(batch_command)


def is_executable(exe_name):
    for path in os.environ["PATH"].split(os.pathsep):
        path = path.strip('"')
        exe_url = os.path.join(path, exe_name)
        if os.path.isfile(exe_url) and os.access(exe_url, os.X_OK):
            return True
    return False


class JobResourceRequest:
    def __init__(self, walltime_in_minutes):
        self.walltime_string = self.format_walltime(walltime_in_minutes)
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
            + ' -n 1 -c ' + str(2*self.processors_per_node) \
            + ' --mem-per-cpu=' + str(int(self.memory_in_mb/2)) + 'mb' \
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
        self.job_array_index_low = 1
        self.job_array_index_high = 1
        self.exported_user_variables = {}

    def set_job_array_size(self, job_array_index_low, job_array_index_high):
        self.job_array_index_low = int(job_array_index_low)
        self.job_array_index_high = int(job_array_index_high)
        if self.job_array_index_low == self.job_array_index_high:
            self.add_exported_user_variable(
                'SLURM_ARRAY_TASK_ID', self.job_array_index_low)

    def add_exported_user_variable(self, name, value):
        self.exported_user_variables[str(name)] = str(value)

    def create_bash_commands(self, max_jobarray_size):
        bashcommand_list = []
        for job_index in range(self.job_array_index_low,
                               self.job_array_index_high + 1,
                               max_jobarray_size):
            bashcommand = batch_command + ' -A m2_him_exp -p himster2_exp'
            bashcommand += ' --constraint=\"skylake,mhz-2101\"'
            if self.job_array_index_high > self.job_array_index_low:
                bashcommand += ' --array=' + str(job_index) + '-' \
                    + str(min(job_index + max_jobarray_size - 1,
                              self.job_array_index_high))
            bashcommand += ' --job-name=' + self.jobname + \
                self.resource_request.get_submit_string() + ' --output=' \
                + self.logfile_url
            # export variables
            bashcommand += ' --export=ALL,'
            for name, value in self.exported_user_variables.items():
                bashcommand += name + '="' + value + '",'
            bashcommand = bashcommand[:-1] + ' ' + self.application_url
            bashcommand_list.append(bashcommand)
        return bashcommand_list


class HimsterJobManager:
    def __init__(self, himster_total_job_threshold=1600,
                 resubmit_wait_time_in_seconds=1800,
                 max_jobarray_size=100):
        self.job_command_list = []
        # user total job threshold
        self.himster_total_job_threshold = himster_total_job_threshold
        # max number of jobs within a job array on himster (atm 100)
        self.max_jobarray_size = max_jobarray_size
        # sleep time when total job threshold is reached in seconds
        self.resubmit_wait_time_in_seconds = resubmit_wait_time_in_seconds

    def manage_jobs(self):
        failed_submit_commands = {}
        while self.job_command_list:
            print("checking if total job threshold is reached...")
            bashcommand = self.job_command_list.pop(0)
            if get_num_jobs_on_himster() < self.himster_total_job_threshold:
                print("Nope, trying to submit job...")
                returnvalue = subprocess.call(bashcommand, shell=True)
                if returnvalue > 0:
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
                    # sleep 5 sec to make the queue changes active
                    sleep(5)
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
                for bashcommand in job.create_bash_commands(
                        self.max_jobarray_size):
                    self.job_command_list.append(bashcommand)

        else:
            print('This is not a cluster environment! Please make sure this '
                  'script is executed on a cluster environment!')
