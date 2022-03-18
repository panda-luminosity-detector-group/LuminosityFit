#!/usr/bin/env python3

"""
Simple program that runs commands given in a named pipe and returns their 
output and return code in two other pipes.

TODO: use json objects for orders, enviromentment variables and so on,
and return json objects with stdout/stderr outputs and return code

json command structure:
order type (regular, management (i.e. for the agent itself))
order value
enviroment variables
shell=(true/false)

return json object structure:
stdout value
stderr value
return code
runtime maybe
"""

import os, shlex, subprocess, sys

orderPipeName = 'orderPipe'
outputPipeName = 'outputPipe'
returnCodePipeName = 'returncodePipe'

# TODO: only two pipes in the future, use json objects instead
def preparePipes():
    os.mkfifo(orderPipeName)
    os.mkfifo(outputPipeName)
    os.mkfifo(returnCodePipeName)

def deletePipes():
    os.unlink(orderPipeName)
    os.unlink(outputPipeName)
    os.unlink(returnCodePipeName)

# continouosly read from pipe and execute, write output and return codes back
def readAndExecute():
    while True:
        with open(orderPipeName, 'r') as orderPipe:
            for line in orderPipe:
                if line.startswith('exit'):
                    print('Got exit command.')
                    return
                else:
                    with open(outputPipeName, 'w') as pipeOut:
                        # args = shlex.split(line)
                        completedProcess = subprocess.run(line, shell=True, stdout=pipeOut)
                        returnCode = completedProcess.returncode

                        with open(returnCodePipeName, 'w') as returnPipe:
                            returnPipe.write(str(returnCode))

if __name__ == '__main__':
    welcome = """
    Agent starting and forking to background.
    Write "exit" to orderPipe to exit agent.
    """

    print(welcome)

    try:
        pid = os.fork()
        if pid > 0:
            # Exit parent process
            sys.exit(0)
    
    except OSError:
        print(f"fork failed: {e.errno} ({e.strerror})")
        sys.exit(1)
    
    preparePipes()
    
    try:
        readAndExecute()    
    except:
        print('unknown error.')
    
    deletePipes()
    
    print('Have a nice day.')
