# Time Machine 

Tool for manipulating time in running processes.

## Goals

1. Run programs with different time than on the operating system.
2. Manipulate time in running processes.

## Usage

Load `timemachine.so` into dynamically linked program with LD_PRELOAD. Set time offset in seconds in TIME_MACHINE_OFFSET environment variable.

### Example

LD_PRELOAD=./timemachine.so TIME_MACHINE_OFFSET=3153600000 date
