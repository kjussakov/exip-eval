#!/bin/bash

./compileOpenEXI.sh
./compileEXIFicient.sh
cd exip-0.5.3/build/gcc/ && make clean && make DEBUGGING=-O3 examples
