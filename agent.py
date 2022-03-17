#!/usr/bin/env python3

import os, shlex, subprocess

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
    Agent starting.
    Press ctrl + C or write "exit" to orderPipe to exit.
    """

    print(welcome)
    
    preparePipes()
    
    try:
        readAndExecute()
    except KeyboardInterrupt:
        print('Caught ctrl+c. Exiting.')
    
    except:
        pass
    
    deletePipes()
    
    print('Have a nice day.')
