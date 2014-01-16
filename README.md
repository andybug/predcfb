predcfb
=======
_predcfb_ is a prediction engine for college football.

Features
--------
It really isn't finished yet, bro.

Data Source
-----------
The input data for _predcfb_ can be downloaded from
[cfbstats.com](www.cfbstats.com). The is no need to unzip the file, _predcfb_
will parse the zip file as-is.

Building
--------
### Dependencies
_predcfb_ utilizes cmake to generate the makefiles used in the build,
so this will need to be installed first. _predcfb_ also depends on the zlib
and libyaml development packages. In summary, the following packages
(or their equivalents) need to be installed:

 * cmake
 * zlib-dev
 * libyaml-dev

### Generating the makefiles
After all of the prerequisites have been installed, create a directory
for the build files and have cmake generate them:

    mkdir build && cd build
    cmake ..

_**Note 1**: The C and C++ compiler can be specified by setting the CC and
CXX environment variables, like so:_ `CC=clang CXX=g++ cmake ..`

_**Note 2**: A build type can be specified by defining the CMAKE_BUILD_TYPE
variable equal to Release or Debug:_ `cmake -DCMAKE_BUILD_TYPE=Debug ..`

### Build and test
Now it is time to build and test the application, from the build directory:

    make

The unit tests for _predcfb_ require access to some files in the source tree,
so the test application must be executed from the source root. Go ahead and
go up one directory and run the test application:

    cd ..
    ./build/bin/predcfb_test

If everything went OK, then you should have a functioning executable located
at `./build/bin/predcfb`.

### Optionally, installing predcfb
To install _predcfb_, simply execute `make install` from the build directory.

Open Source Acknowledgments
----------------------------
_predcfb_, which is [GPLv2] licensed, makes use of many open source projects.
This section is to acknowledge those projects and list their respective
licenses:

 * zlib - [zlib license]
 * libyaml - [MIT license]
 * libcsv - [LGPLv2]
 * minizip - [zlib license]
 * polarssl - [GPLv2]
 * openbsd - [BSD license]

[GPLv2]: http://www.gnu.org/licenses/gpl-2.0.html
[LGPLv2]: http://www.gnu.org/licenses/lgpl-2.1.html
[zlib license]: http://www.zlib.net/zlib_license.html
[MIT license]: http://opensource.org/licenses/MIT
[BSD license]: http://www.openbsd.org/policy.html
