#!/bin/bash

# TODO(heios): use gnu parallel

for j in {1..100}; do
echo "-----------  $j  ----------------"
for i in {1..100}; do ./23a.out i /run/user/1000/data0005.bin 5000 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out p /run/user/1000/data0005.bin 5000 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out f /run/user/1000/data0005.bin 5000 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out ff /run/user/1000/data0005.bin 5000 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out s /run/user/1000/data0005.bin 5000 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out c /run/user/1000/data0005.bin 5000 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out m /run/user/1000/data0005.bin 5000 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
echo "-----------------------------------"
for i in {1..100}; do ./23a.out i /run/user/1000/data006.bin 65123 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out p /run/user/1000/data006.bin 65123 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out f /run/user/1000/data006.bin 65123 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out ff /run/user/1000/data006.bin 65123 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out s /run/user/1000/data006.bin 65123 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out c /run/user/1000/data006.bin 65123 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out m /run/user/1000/data006.bin 65123 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
echo '-----------------------------------'
for i in {1..100}; do ./23a.out i /run/user/1000/data03.bin 307656 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out p /run/user/1000/data03.bin 307656 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out f /run/user/1000/data03.bin 307656 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out ff /run/user/1000/data03.bin 307656 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out s /run/user/1000/data03.bin 307656 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out c /run/user/1000/data03.bin 307656 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out m /run/user/1000/data03.bin 307656 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
echo '-----------------------------------'
for i in {1..100}; do ./23a.out i /run/user/1000/data1.bin 1555111 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out p /run/user/1000/data1.bin 1555111 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out f /run/user/1000/data1.bin 1555111 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out ff /run/user/1000/data1.bin 1555111 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out s /run/user/1000/data1.bin 1555111 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out c /run/user/1000/data1.bin 1555111 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out m /run/user/1000/data1.bin 1555111 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
echo '-----------------------------------'
for i in {1..100}; do ./23a.out i /run/user/1000/data3.bin 3123123 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out p /run/user/1000/data3.bin 3123123 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out f /run/user/1000/data3.bin 3123123 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out ff /run/user/1000/data3.bin 3123123 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out s /run/user/1000/data3.bin 3123123 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out c /run/user/1000/data3.bin 3123123 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out m /run/user/1000/data3.bin 3123123 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
echo '-----------------------------------'
for i in {1..100}; do ./23a.out i /run/user/1000/data15.bin 15740985 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out p /run/user/1000/data15.bin 15740985 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out f /run/user/1000/data15.bin 15740985 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out ff /run/user/1000/data15.bin 15740985 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out s /run/user/1000/data15.bin 15740985 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out c /run/user/1000/data15.bin 15740985 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out m /run/user/1000/data15.bin 15740985 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
echo '-----------------------------------'
for i in {1..100}; do ./23a.out i /run/user/1000/data150.bin 157409856 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out p /run/user/1000/data150.bin 157409856 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out f /run/user/1000/data150.bin 157409856 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out ff /run/user/1000/data150.bin 157409856 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out s /run/user/1000/data150.bin 157409856 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out c /run/user/1000/data150.bin 157409856 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
for i in {1..100}; do ./23a.out m /run/user/1000/data150.bin 157409856 ; sleep 1 ; done | stdbuf -oL -eL tee -a results-run2.txt
echo "-----------------------------------"

done
