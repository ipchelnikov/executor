# Executor

Task executor provides an ability to run tasks on an allocated thread pool.

## Build

To clone and build the project, run the following:
> **_NOTE:_** Tested on 'Ubuntu 20.04.1 LTS'
> Requires CMake 3.14 or newer
    git clone https://github.com/ipchelnikov/executor.git
    cd executor
    cmake ./
    make all

## Run

From the project folder

    ./bin/example       # Start exapmle
    ./bin/executor_test # Start unit tests

    make check # To verify all tests