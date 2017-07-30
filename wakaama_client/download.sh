#!/bin/bash

IMAGE_NAME=$1               #input image name
IMAGE_SIZE=$2          	    #input image size                #input image argument
FILE=$3

FOLDER="download"

SUCCESS_DOWNLOAD=1
SUCCESS_UPDATE=11
FAILED_DOWNLOAD=7
FAILED_UPDATE=8

E_PARA_INVALI=2
E_INVALID_NAME=3
E_NETWORK=4=4
E_OUT_OF_MEM=5
E_UNKNOWN=6

LOW_LIMMIT_STORAGE=128
FLASH_MEM=$(df -m . | tail -1 | awk '{print $4}')


function early_check(){
	
	if [[ ${EUID} -ne 0 ]]; then   
		echo " !!! This tool must be run as root"
		exit $E_PARA_INVALI
		
	fi
	
	
	if [[ -z "$1" ]]; then
		echo "-Docker image name is empty, please write a valid image name.-"
		exit $E_PARA_INVALI            #empty image name
	
	fi		

	if [[ -z "$2" ]] || ! [[ $2 =~ ^[0-9]+$ ]]; then  
		echo "-Docker image size is empty or invalid, please write a valid image size-"
		exit $E_PARA_INVALI
	fi

	if [ -f $FILE ]; then
		echo "File $FILE exists, removing..."
		remove=$(sudo rm $FILE)
	fi 
}


function image_check(){
	
	images=$(docker images)
	
	if [[ $images == *"$1"* ]]; then
		echo "image is existed"
		exit $SUCCESS_DOWNLOAD
	fi
	

}

#function file_check(){
#	if [ -f $FILE ]; then
#		echo "File $FILE is downloaded"
#		exit $SUCCESS_DOWNLOAD
#	else
#		echo "File $FILE fails to download"
#		exit $FAILED_DOWNLOAD
#	fi
#		
#}

function flash_mem_check {

	if [[ $FLASH_MEM -lt $IMAGE_SIZE ]]; then
		echo "-flash memory is not enough-"
		exit $E_OUT_OF_MEM
	fi
}


function update_run(){
	
	if [ ! -d "$FOLDER" ]; then
		mkdir $FOLDER
	fi
	mv -f $FILE $FOLDER
	cd $FOLDER
	tar -xopf $FILE
	exe_file=$(echo $(find ./ -iname "*.sh"))
	exe=$(./$exe_file)&
	#echo "successfully updated"
	#exit $SUCCESS_DOWNLOAD
	
}


early_check $IMAGE_NAME $IMAGE_SIZE
echo "input is correct"
flash_mem_check
echo "downloading"
OUTPUT=$(wget $IMAGE_NAME 2>&1)
OUTPUT_RES=$?
echo "download ends"
#process_check $OUTPUT $OUTPUT_RES

if [[ $OUTPUT_RES -eq 0 ]]; then
	echo "the download is successful"
	update_run
	echo "successfully updated"
	exit $SUCCESS_DOWNLOAD
	
else
	exit $FAILED_DOWNLOAD
fi



