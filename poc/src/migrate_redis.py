import fileinput
import os
import signal
import sys

# replace the line
for line in fileinput.input('config.txt', inplace=True):
    print(line.replace('tcp://127.0.0.1:6380', 'tcp://127.0.0.1:6381'), end='')

# Get PID of process from command line argument
pid = int(sys.argv[1])

# Send SIGUSR1 signal to process
os.kill(pid, signal.SIGUSR1)

