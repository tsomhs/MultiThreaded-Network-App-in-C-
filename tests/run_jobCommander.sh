#!/bin/bash

# Command to run
COMMAND="./bin/jobCommander.exe linux01.di.uoa.gr 2020 issueJob cat Makefile" #Change linux pc number if needed
# Run the command 100 times
while true
do
    $COMMAND
    sleep 1
done
