mld - Matchlist Analyzer Daemon
===============================

###Build

Change into the _Release/_ directory and run:

    make clean ; make all ;
    
The executable will be created in the root directory.

###Usage

mld is a background daemon. So you just have to start the executable like this:

    ./mld
    
However, mld is expecting a configuration file at _/etc/viat/mld.conf_. You can also specify the configuration file directly by passing it:

    ./mld mld.conf