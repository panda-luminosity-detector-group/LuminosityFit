#!/usr/bin/env python3

import os, shlex, subprocess

orderPipeName = 'orderPipe'
outputPipeName = 'outputPipe'

def preparePipes():
    os.mkfifo(orderPipeName)
    os.mkfifo(outputPipeName)

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
                        subprocess.run(args, stdout=pipeOut)

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
    
    os.unlink(orderPipeName)
    os.unlink(outputPipeName)
    
    print('Have a nice day.')
