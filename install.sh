#!/bin/bash
#this script installs the IAS Computer Simulator program

readonly SUCCESSFUL=0
readonly UNSUCCESSFUL=1
echo Installing IAS Computer Simulator..
cc ./src/ias.c -o /usr/local/bin/ias
if [ $? -eq 0 ]; then 
	echo Installation Complete
else
	echo Could not install program, exiting installer..
	exit UNSUCCESSFUL
fi

exit SUCCESSFUL

