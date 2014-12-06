#!/bin/bash


if [ "$#" -ne 1 ];
then
	echo "Usage: $0 <code tar>"
	exit
fi

TAR_FILE=$1

tar xvf ${TAR_FILE}
if [ $? -ne 0 ];
then
        echo "untar error"
        exit
fi


if [ ! -d "protocols" ];
then
	echo "protocols/ doesn't exist"
	exit
fi

if [ ! -d "project3" ];
then
	echo "project3/ doesn't exist"
	exit
fi

cp -rf protocols project3/

cd project3/
make clean
make

if [ $? -ne 0 ];
then
	echo "Compile error"
	exit
fi

declare -a TRACES=("traces/4proc_validation" "traces/8proc_validation" "traces/16proc_validation")
declare -a PROTOCOLS=("MSI" "MESI" "MOSI" "MOESI" "MOESIF")


for p in "${PROTOCOLS[@]}"
do
	echo "-----------------------------"
	echo "Verifying protocol: $p"
	for t in "${TRACES[@]}"
	do
		./sim_trace -p $p -t $t > output.log 2>&1
		diff ${t}/${p}_validation.txt output.log > ../diff.log 2>&1
		if [ -s diff.log ];
		then
			echo "${t}: FAIL"
			echo "Please check diff.log for details"
			rm -rf output.log
			exit
		else
			echo "${t}: PASS"
			rm -rf output.log
			rm -rf ../diff.log
		fi
	done
done

echo "-----------------------------"


