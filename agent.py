#!/usr/bin/env python3

import os, shlex, subprocess, sys

orderPipeName = 'orderPipe'
outputPipeName = 'outputPipe'
returnCodePipeName = 'returncodePipe'

def preparePipes():
    os.mkfifo(orderPipeName)
    os.mkfifo(outputPipeName)
    os.mkfifo(returnCodePipeName)

def deletePipes():
    os.unlink(orderPipeName)
    os.unlink(outputPipeName)
    os.unlink(returnCodePipeName)

# continously read from pipe and execute, write output and return codes back
def readAndExecute():
    while True:
        with open(orderPipeName, 'r') as orderPipe:
            for line in orderPipe:
                if line.startswith('exit'):
                    print('Got exit command.')
                    return
                else:
                    with open(outputPipeName, 'w') as pipeOut:
                        args = shlex.split(line)
                        completedProcess = subprocess.run(args, stdout=pipeOut)
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
    except KeyboardInterrupt:
        print('Caught ctrl+c. Exiting.')
    
    except:
        pass
    
    deletePipes()
    
    print('Have a nice day.')
