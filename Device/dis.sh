#!/bin/bash
NUM_CLIENT=$1 
CLIENT_BASE=$2
lp=$3
# set n to 0
n=$CLIENT_BASE
NUM_CLIENTS=$(( NUM_CLIENT+CLIENT_BASE ))

# continue until $n equals upper-limmit

while [ $n -lt $NUM_CLIENTS ]
do
	echo "client $n start"
	
	#out[$n]=$(docker run -it leshan-client java -jar client-1.0.jar -u 192.168.2.22 -n client1_$n) &
	out[$n]=$(docker run -it -p $lp:$lp/udp allencz/wakaama-client:v3 ./lwm2mclient -4 -r chen -l $lp -d 192.168.2.18 -h 192.168.2.18 -n $n) &
	
	n=$(( n+1 ))
	
done 
