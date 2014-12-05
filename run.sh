#!/bin/bash

for n in {1..8}; do
for p in {MSI,MESI,MOSI,MOESI,MOESIF}; do
./sim_trace -t traces/experiment$n -p $p &> traces/experiment/exp${n}_$p.out
done
done
