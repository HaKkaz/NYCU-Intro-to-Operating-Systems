#!/bin/bash
rnd_n=$1
g++ hw3.cpp -o sort
g++ check.cpp -o check
g++ random.cpp -o random
./random $rnd_n
./sort
./check