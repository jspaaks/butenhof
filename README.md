# notes

Exploring multithreaded applications including mutexes and condition variables with POSIX threads.

## CMake

The project has been initialized with a [CMakeLists.txt](CMakeLists.txt)-based
configuration for building with CMake:

```console
# make a build directory
$ mkdir build

# change into the build directory
$ cd build/

# generate the build files
$ cmake -DCMAKE_BUILD_TYPE=Debug ..

# build the project
$ cmake --build .

# install the project to <repo>/build/dist
$ cmake --install . --prefix dist/
```

Output should be something like below. Program `hello` illustrates multiple threads
concurrently saying hello.

```console
$ ./dist/bin/hello
hello from main
hello from thread 3
hello from thread 1
hello from thread 0
hello from thread 2
goodbye from thread 1
goodbye from thread 3
goodbye from thread 2
goodbye from thread 0
goodbye from main
```

`bank` is a program that illustrates the use of mutexes to control access to data shared
between threads, in this case a bank account.

```console
$ ./dist/bin/bank
Concurrently increment a shared variable a=0 one million times in each of 4 threads
a=4000000
```

`warehouse` is a program that illustrates conditional wait variables on a threadsafe stack
that simulates the stock in a warehouse, with a producer and a consumer adding and taking
items from said stack.

```
$ ./dist/bin/warehouse
___ ___ ___
___ ___ ___
produced item 1 of 10 with id 716
716 ___ ___
716 ___ ___
consumed item 1 of 10 with id 716
produced item 2 of 10 with id 965
965 ___ ___
965 ___ ___
consumed item 2 of 10 with id 965
___ ___ ___
___ ___ ___
___ ___ ___
Out of stock, waiting...
produced item 3 of 10 with id 111
consumed item 3 of 10 with id 111
___ ___ ___
___ ___ ___
Out of stock, waiting...
___ ___ ___
___ ___ ___
___ ___ ___
produced item 4 of 10 with id 801
consumed item 4 of 10 with id 801
___ ___ ___
Out of stock, waiting...
___ ___ ___
___ ___ ___
produced item 5 of 10 with id 351
consumed item 5 of 10 with id 351
___ ___ ___
produced item 6 of 10 with id 822
822 ___ ___
822 ___ ___
822 ___ ___
produced item 7 of 10 with id 524
822 524 ___
produced item 8 of 10 with id 701
822 524 701
822 524 701
822 524 701
822 524 701
822 524 701
consumed item 6 of 10 with id 701
produced item 9 of 10 with id 117
822 524 117
822 524 117
822 524 117
822 524 117
822 524 117
822 524 117
produced item 10 of 10 with id 941
Out of space, waiting...
822 524 117
822 524 117
822 524 117
consumed item 7 of 10 with id 117
822 524 941
822 524 941
822 524 941
822 524 941
822 524 941
822 524 941
822 524 941
822 524 941
consumed item 8 of 10 with id 941
822 524 ___
822 524 ___
822 524 ___
822 524 ___
822 524 ___
822 524 ___
consumed item 9 of 10 with id 524
822 ___ ___
822 ___ ___
822 ___ ___
822 ___ ___
822 ___ ___
822 ___ ___
822 ___ ___
822 ___ ___
consumed item 10 of 10 with id 822
```

`pipeline` is a program that illustrates pipelining, with threadsafe ring buffer queues
in between each stage of the pipeline. This example builds on the examples that use
mutexes and condition variables.

```
$ ./dist/bin/pipeline
vin 0: spawned
vin 0: start installing frame
vin 1: spawned
vin 2: spawned
vin 0: completed installing frame
vin 1: start installing frame
vin 0: start installing engine
vin 3: spawned
vin 0: completed installing engine
vin 0: start installing body
vin 1: completed installing frame
vin 2: start installing frame
thread 'source' complete
vin 1: start installing engine
vin 2: completed installing frame
vin 3: start installing frame
vin 0: completed installing body
vin 0: start installing wheels
vin 1: completed installing engine
vin 2: start installing engine
vin 1: start installing body
vin 1: completed installing body
vin 2: completed installing engine
vin 0: completed installing wheels
vin 1: start installing wheels
vin 2: start installing body
vin 0: complete
vin 3: completed installing frame
thread 'install_frame' complete
vin 3: start installing engine
vin 2: completed installing body
vin 1: completed installing wheels
vin 2: start installing wheels
vin 1: complete
vin 3: completed installing engine
thread 'install_engine' complete
vin 3: start installing body
vin 2: completed installing wheels
vin 2: complete
vin 3: completed installing body
thread 'install_body' complete
vin 3: start installing wheels
vin 3: completed installing wheels
thread 'install_wheels' complete
vin 3: complete
thread 'sink' complete
```

## Acknowledgements

_This project was initialized using [Copier](https://pypi.org/project/copier) and the [copier-template-for-c-projects](https://github.com/jspaaks/copier-template-for-c-projects)._
