Author          : Peng Zheng, Pin Lyu
Created         : Oct 16th, 2019
Last Modified   : Nov 6th, 2019
Affiliation     : Georgia Institute of Technology

Description
-------------
The whole program is built to simulate a queuing network. It includes three modules which are documented in a .h file. 
Functions consist of reading the configuration file, controlling the whole simulation process (FEL) and specific application with detailed functions.


To compile and run:
--------------------

Installation
------------

To install, first make sure ReadEngineApplication.h, readfile.c, Engine.c and Application.c are in the same directory
Then simply run:

$ module load gcc/7.3.0
$ gcc readfile.c engine.c application.c -o desim -lm

As a result, the executable file-name is desim.

Execution
-----------
Assuming your executable is called "desim", run it using

$ ./desim total_time config outfile

e.g.
$ ./desim 240 Configuration_Sample.txt output.txt

Notice: The statistics will be printed on the screen as well as written into the output file.
If there are some errors with configuration files, the program will stop running and give hints to the user, and
the user can modify configuration files and rerun the executable file.
For infomation about grammar of configuration files, please refer to the report.