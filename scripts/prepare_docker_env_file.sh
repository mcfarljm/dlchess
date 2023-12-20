#!/bin/bash

# Create the docker .env file that stores user id

echo USERID=`id -u`:`id -g` > .env
