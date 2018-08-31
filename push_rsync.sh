#!/bin/bash

# rsync -avzhe ssh $1 pi@raspberrypi:/home/pi/Work/connections/$1
rsync -avzhe ssh $1 pi@192.168.1.111:/home/pi/Work/connections/$1