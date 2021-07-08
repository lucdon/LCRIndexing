# LCR Indexing

This repository contains a framework for quickly evaluating and testing LCR indexes. The code has been used for my
master thesis at the Technical University of Eindhoven, July 2021.

## Building and running

The code has been written in C++-17. Numerous scripts have been included, which have all been written in python (3.7). The
project has been tested on Windows and Linux. It should also run and compile on Mac.

### Requirements

- CMake version 3.10 or later
- c++ compiler that supports C++-17
- python 3.7 or later
- boost library (must be present in folder /include/boost/)

### Building the C++ code
#### linux and mac
```shell
cmake -DCMAKE_BUILD_TYPE=Release -DNO_GTEST=True -DNO_PRECOMPILED_HEADER=True -G"Unix Makefiles" ./
```

Remove `-DNO_GTEST=True` if the tests should be build. 
For CMake 3.16 or later, `-DNO_PRECOMPILED_HEADER=True` can be removed, which allows for faster re-build times.
It is recommended to target `Release` builds. See CMake documentation for more options.

After running the cmake command, the following command should be used to build the executables:

```shell
make -j4
```

When rebuilding, only the above command is required. Unless files have been added/removed, then the cmake command must be re-executed. 

#### Windows

for windows the following command can be used:
```shell
cmake -DCMAKE_BUILD_TYPE=Release -DNO_GTEST=True -DNO_PRECOMPILED_HEADER=True -G"Visual Studio 16 2019" -A x64 ./
```

Now open the visual studio projects to build and run the code.

### Running
After building the executables the following command is available to run indexes:

```shell
MasterThesis[.exe] [reach|lcr] 
                  --graphFile [graphFile] 
                  --queryFile [queriesFile]                
                  --index [indexName] 
                  --indexParams [parameterList]
                  --timeLimit [timeLimitInSeconds] 
                  --memoryLimit [memoryLimitInMBs]
                  <[--control]> 
```

In order to generate queries for a graph, the following command may be used:

```shell
LCRQueryGenerator[.exe] 
                  --graphFile [graphFileIn]
                  --randomQueries [count]
                  --connectedQueries [count]
                  <[--printStats]>
                  <[--allInOne]>
                  <[--splitRandomFromConnected]>
```

The last command can be used to import a graph for graph statistics. It is also possible to convert between graph representations.
For example changing from .rdf file to .nt file, which can be achieved by specifying the --graphFileOut parameter.

```shell
GraphUtilities[.exe] [graphFileIn] 
                  [--printStats] 
                  [--graphFileOut] [graphFileOut]
```

## Python scripts
Running the 'runner' scripts, requires the c++ executables to be build.
The requirements for the python scripts can be found in [runner/requirements.txt](https://github.com/lucdon/LCRIndexing/runner/requirements.txt).

- In [graphProcessing](https://github.com/lucdon/LCRIndexing/graphProcessing/README.md), numerous scripts for
  downloading and processing real datasets can be found.
- In [runner](https://github.com/lucdon/LCRIndexing/runner/README.md), numerous scripts for running and creating
  benchmarks on real and synthetic datasets can be found.
- In [evaluation](https://github.com/lucdon/LCRIndexing/evaluation/README.md), numerous scripts for visualizing and
  evaluation benchmark results can be found.