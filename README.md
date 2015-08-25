DA emulation project Renaissance

Dependencies:
=============

You need development versions of the following libraries in order to build
the renaissance server:

* postgresql-server
* libpqxx
* lua
* log4cplus
* config++
* zlib

Compiling:
==========

The server is set up to be compiled with cmake. On most Linux distributions, you should be able to get started by running:


    mkdir debug
    cd debug && cmake -DCMAKE_BUILD_TYPE=Debug ../src && make


`cmake` only needs to be run once. It's sufficient to run `make` to recompile.

Running:
========

**Configuration File:**

Before you first run the server, you have to set up a configuration file and a database. The server executable expects to find a file named `daserver.conf` in the working directory. To create the initial file, copy `daserver.conf.template` and change settings as appropriate.

**Database:**
The server currently only supports *postgres*. You can set up the required tables by running `psql *connection parameters* -f db/schema.sql`.

License:
=======
GNU Affero General Public License v3.0