# Time Machine 

Tool for manipulating time in running processes.

## Goals

1. Run programs with different time than on the operating system.
2. Manipulate time in running processes.

## Usage

Load `timemachine.so` into dynamically linked program with LD_PRELOAD. Set time offset in seconds in TIME_MACHINE_OFFSET environment variable.

	LD_PRELOAD=./timemachine.so TIME_MACHINE_OFFSET=3153600000 date

Manipulate time by writing either positive or negative offset in secconds to `/tmp/timemachine/PID/set`.

	echo 123 > /tmp/timemachine/9876/set

Check effective offset by reading file `/tmp/timemachine/PID/get`.

	cat /tmp/timemachine/9876/get
