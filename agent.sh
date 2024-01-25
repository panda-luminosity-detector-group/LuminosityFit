#!/bin/bash

# Check if "python3 python/lumifit/agent.py" is running for the current user
if $(ps -u $USER x | grep -F 'python/lumifit/agent.py' | grep -v grep > /dev/null); then
    echo "Agent already running."
else
    # If the process isn't running, load the Python module and execute the agent.py script
    module load lang/Python/3.10.4-GCCcore-11.2.0
    $HOME/LuminosityFit/python/lumifit/agent.py
fi
