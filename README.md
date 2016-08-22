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

Compiling on Windows:
=====================

You need to have PostgreSQL 9.5 (32 bit) installed. You can use the 64 bit database if you prefer but the 32 bit
version is needed to build the server executable. You will also need CMake and Visual Studio 2015.

To setup the initial project files, create a subdirectory called build. Run CMake and
select src as the source directory, and build as the target directory. Press configure.
Configure may fail to find PostgreSQL, in which case you can specify the locations and then
press configure to continue. Once it finishes configuring successfully, press generate and
close cmake. You should have a VS2015 project file in the build directory now.

Using the project file, build the DAServer target. If it fails to link with errors related
to postgres you may need to go into the project settings and manually set the postgres
lib directories (properties, linker, general, additional library directories). This is
because cmake's FindPostgreSQL module prefers to find the 64 bit version if also installed.

Running:
========

**Configuration File:**

Before you first run the server, you have to set up a configuration file and a database. The server executable expects to find a file named `daserver.conf` in the working directory. To create the initial file, copy `daserver.conf.template` and change settings as appropriate.

If using windows, the configuration file, as well as *seedTables*, *motd.txt*, and *npc*,
should be copied to the build directory.

**Database:**
The server currently only supports *postgres*. You can set up the required tables by running `psql *connection parameters* -f db/schema.sql`.

License:
=======
GNU Affero General Public License v3.0