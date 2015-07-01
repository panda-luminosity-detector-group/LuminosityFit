import os
import subprocess
import time

# check number of jobs currently running or in queue on himster from my side
def getNumJobsOnHimster():    
    bashcommand = 'qstat -t | wc -l'
    returnvalue = subprocess.Popen(bashcommand, shell=True, stdout=subprocess.PIPE)
    out, err = returnvalue.communicate()
    return int(out)

def isExecutable(fpath):
  return os.path.isfile(fpath) and os.access(fpath, os.X_OK)

def isClusterEnvironment():
  program = 'qsub'
  is_cluster = False
  for path in os.environ["PATH"].split(os.pathsep):
    path = path.strip('"')
    exe_file = os.path.join(path, program)
    if isExecutable(exe_file):
      is_cluster = True
      break
  return is_cluster

class JobResourceRequest:
  number_of_nodes=1
  processors_per_node=1
  memory_in_mb=1000
  virtual_memory_in_mb=1000
  node_scratch_filesize_in_mb=0
  
  def __init__(self, walltime_in_minutes):
    self.walltime_string = self.formatWalltime(int(walltime_in_minutes))
  
  def formatWalltime(self, walltime_in_minutes):
    walltime_string = str(int(walltime_in_minutes)/60) + ':'
    walltime_string = walltime_string + str(int(walltime_in_minutes)%60)
    if int(walltime_in_minutes)%60 < 10:
       walltime_string = walltime_string + '0'
    walltime_string = walltime_string + ':00'
    return walltime_string

  def getSubmitString(self):
    resource_request = ' -l nodes=' + str(self.number_of_nodes) + ':ppn=' + str(self.processors_per_node) \
        + ',mem=' + str(self.memory_in_mb) + 'mb,vmem=' + str(self.virtual_memory_in_mb) + 'mb'\
        + ',walltime=' + self.walltime_string
    if self.node_scratch_filesize_in_mb > 0:
      resource_request = resource_request + ',file=' + str(self.node_scratch_filesize_in_mb) + 'mb'
    return resource_request
    
class Job:
  job_array_index_low=1
  job_array_index_high=1
  exported_user_variables = {}
  def __init__(self, job_resource_request, application_url, jobname, logfile_url):
    self.resource_request = job_resource_request
    self.application_url = str(application_url)
    self.jobname = str(jobname)
    self.logfile_url = str(logfile_url)

  def setJobArraySize(self, job_array_index_low, job_array_index_high):
    self.job_array_index_low = int(job_array_index_low)
    self.job_array_index_high = int(job_array_index_high)
  
  def addExportedUserVariable(self, name, value):
    self.exported_user_variables[str(name)] = str(value)
  
  def createBashCommands(self, max_jobarray_size):
    bashcommand_list = []
    for job_index in range(self.job_array_index_low, self.job_array_index_high + 1, max_jobarray_size):    
      bashcommand = 'qsub'
      if self.job_array_index_high > self.job_array_index_low:
        bashcommand = bashcommand + ' -t ' + str(job_index) + '-' + str(min(job_index + max_jobarray_size - 1, self.job_array_index_high))
      bashcommand = bashcommand + ' -N ' + self.jobname + self.resource_request.getSubmitString() + ' -j oe -o ' + self.logfile_url
      if len(self.exported_user_variables) > 0:
        bashcommand = bashcommand + ' -v '
        counter = 0
        for name, value in self.exported_user_variables.iteritems():
          counter = counter + 1
          bashcommand = bashcommand + name + '="' + value + '"'
          if counter < len(self.exported_user_variables):
            bashcommand = bashcommand + ','
      bashcommand = bashcommand + ' -V ' + self.application_url
      bashcommand_list.append(bashcommand)
      
    return bashcommand_list

class HimsterJobManager:
  job_command_list=[]
    
  def __init__(self, himster_total_job_threshold=1600, max_jobarray_size=100, job_resubmit_sleep_time_in_seconds=1800):
    # user total job threshold
    self.himster_total_job_threshold = himster_total_job_threshold
    # max number of jobs within a job array on himster (atm 100)
    self.max_jobarray_size = max_jobarray_size
    # sleep time when total job threshold is reached in seconds
    self.job_resubmit_sleep_time_in_seconds = job_resubmit_sleep_time_in_seconds

  def manageJobs(self):
    while self.job_command_list:
      print "checking if total job threshold is reached..."
      bashcommand = self.job_command_list.pop(0)
      if getNumJobsOnHimster() < self.himster_total_job_threshold:
        print "Nope, trying to submit job..."
        returnvalue = subprocess.call(bashcommand.split())
        if returnvalue > 0:
          print "Submit failed! Appending job to resubmit list for later submission..."
          # put the command back into the list 
          self.job_command_list.insert(0, bashcommand)
        else:
          time.sleep(5)  # sleep 5 sec to make the queue changes active
      else:
        # put the command back into the list 
        self.job_command_list.insert(0, bashcommand)  
        print 'Yep, we have currently have ' + str(len(self.job_command_list)) + ' jobs waiting in queue!'
        # and sleep for some time
        print 'Waiting for ' + str(self.job_resubmit_sleep_time_in_seconds / 60) + ' min and then trying a resubmit...'
        time.sleep(self.job_resubmit_sleep_time_in_seconds)  
        
  def submitJobsToHimster(self, job_list):
    if isClusterEnvironment():
      submit_commands = []  
      print 'This is a cluster environment... adding jobs to queue list!'  
      for job in job_list:
        for bashcommand in job.createBashCommands(self.max_jobarray_size):
          self.job_command_list.append(bashcommand)
          
    else:
      print 'This is not a cluster environment! Please make sure this script is executed on a cluster environment!' 
