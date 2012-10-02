mld - Matchlist Analyzer Daemon
===============================

The mld (Matchlist Analyzer Daemon) is a background process which check the 'matchlist' table in the database and fills the 'caller\_blacklist' to be able to block a certain caller.

###Installation

Optionally create and change into a new directory. Get the sources via git:
    
    git clone git://github.com/viat/mld.git


From here change into _mld/Release/_ and type the following and hit enter to build the software:

    make clean ; make -j4 all

Where _-j4_ is optional for using four threads while compiling.
The executable has been created one directory above.

###Usage

Since mld is a background daemon an easy way to start it is to run the following on the command line:

    ./mld mld.conf
    
You can pass the configuration file as argument.
Otherwise mld expects the configuration file to be _/etc/viat/mld.conf_.

###Logging

The logging output is in the configured directory.
You can follow the output by using tail:

    tail -f mld.log

###Process Identifier (PID)

To quit the daemon is very easy using the PID.
For example type:

    kill `cat mld.pid`

and hit enter.

###Developer's Guide

The Developer's Guide was created with Doxygen v1.7.5.1 and can be seen by opening _html/index.htm_ in a browser.