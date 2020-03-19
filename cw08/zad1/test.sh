#!/bin/bash

INPUT=saturn.pgm
OUTPUT=satout.pgm


FILTER1_SIZE=4
FILTER2_SIZE=16
FILTER3_SIZE=32

FILTER1_DATA=0.04
FILTER2_DATA=0.01
FILTER3_DATA=0.0025

FILTER1=filter1
FILTER2=filter2
FILTER3=filter3

echo -e "====================make main===================="
make clean
make all
echo -e "=================================================\n"

echo -e "====================filter gen===================="
ruby filtrgenerator.rb $FILTER1 $FILTER1_SIZE $FILTER1_DATA
ruby filtrgenerator.rb $FILTER2 $FILTER2_SIZE $FILTER2_DATA
ruby filtrgenerator.rb $FILTER3 $FILTER3_SIZE $FILTER3_DATA
echo -e "==================================================\n"

MODE=block
echo -e "====================block test====================\n"
FILTER=FILTER1
echo -e "------------------filter 1 (4x4)------------------"
echo -e "\n1 THREAD"
./main 1 $MODE $INPUT $FILTER $OUTPUT
echo -e "\n2 THREADS"
./main 2 $MODE $INPUT $FILTER $OUTPUT
echo -e "\n4 THREADS"
./main 4 $MODE $INPUT $FILTER $OUTPUT
echo -e "\n8 THREADS"
./main 8 $MODE $INPUT $FILTER $OUTPUT
echo -e "--------------------------------------------------\n"
FILTER=FILTER2
echo -e "-----------------filter 2 (16x16)-----------------"
echo -e "\n1 THREAD"
./main 1 $MODE $INPUT $FILTER $OUTPUT
echo -e "\n2 THREADS"
./main 2 $MODE $INPUT $FILTER $OUTPUT
echo -e "\n4 THREADS"
./main 4 $MODE $INPUT $FILTER $OUTPUT
echo -e "\n8 THREADS"
./main 8 $MODE $INPUT $FILTER $OUTPUT
echo -e "--------------------------------------------------\n"
FILTER=FILTER3
echo -e "-----------------filter 3 (64x64)-----------------"
echo -e "\n1 THREAD"
./main 1 $MODE $INPUT $FILTER $OUTPUT
echo -e "\n2 THREADS"
./main 2 $MODE $INPUT $FILTER $OUTPUT
echo -e "\n4 THREADS"
./main 4 $MODE $INPUT $FILTER $OUTPUT
echo -e "\n8 THREADS"
./main 8 $MODE $INPUT $FILTER $OUTPUT
echo -e "--------------------------------------------------\n"
echo -e "==================================================\n\n"

echo -e "=================intervaled test==================\n"
MODE=intervaled
FILTER=FILTER1
echo -e "------------------filter 1 (4x4)------------------"
echo -e "\n1 THREAD"
./main 1 $MODE $INPUT $FILTER $OUTPUT
echo -e "\n2 THREADS"
./main 2 $MODE $INPUT $FILTER $OUTPUT
echo -e "\n4 THREADS"
./main 4 $MODE $INPUT $FILTER $OUTPUT
echo -e "\n8 THREADS"
./main 8 $MODE $INPUT $FILTER $OUTPUT
echo -e "--------------------------------------------------\n"
FILTER=FILTER2
echo -e "-----------------filter 2 (16x16)-----------------"
echo -e "\n1 THREAD"
./main 1 $MODE $INPUT $FILTER $OUTPUT
echo -e "\n2 THREADS"
./main 2 $MODE $INPUT $FILTER $OUTPUT
echo -e "\n4 THREADS"
./main 4 $MODE $INPUT $FILTER $OUTPUT
echo -e "\n8 THREADS"
./main 8 $MODE $INPUT $FILTER $OUTPUT
echo -e "--------------------------------------------------\n"
FILTER=FILTER3
echo -e "-----------------filter 3 (64x64)-----------------"
echo -e "\n1 THREAD"
./main 1 $MODE $INPUT $FILTER $OUTPUT
echo -e "\n2 THREADS"
./main 2 $MODE $INPUT $FILTER $OUTPUT
echo -e "\n4 THREADS"
./main 4 $MODE $INPUT $FILTER $OUTPUT
echo -e "\n8 THREADS"
./main 8 $MODE $INPUT $FILTER $OUTPUT
echo -e "--------------------------------------------------\n"
echo -e "==================================================\n\n"
