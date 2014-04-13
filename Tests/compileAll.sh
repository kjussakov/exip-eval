#!/bin/bash

cd OpenEXI/ && ./compile.sh
cd ../EXIFicient/ && ./compile.sh
cd ../exip-0.5.3/build/gcc/ && make examples
