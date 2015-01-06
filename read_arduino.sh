#!/bin/bash
# Data Slam Script v0.1
# User define Function (UDF)
LogLine(){
  echo -E "`date +%s`,${line}"
 # vcgencmd measure_temp
 sqlite3 tempDatabase.db3 "insert into tempData values(`date +%s`,${line})"
} 
### Main script stars here ###
# Store file name
FILE="read_arduino.sh"
DBFILE="tempDatabase.db3"
# Make sure we get file name as command line argument
# Else read it from standard input device
# stty -F /dev/ttyACM0 cs8 115200 ignbrk -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts
if [ "$1" == "" ]; then
   FILE="/dev/ttyACM0"
else
   FILE="$1"
   # make sure file (serial device) exist and is readable
   if [ ! -f $FILE ]; then
  	echo "$FILE : does not exists"
  	exit 1
   elif [ ! -r $FILE ]; then
  	echo "$FILE: can not read"
  	exit 2
   fi
fi
# Create Database if it does not exist
 
if [ ! -f "tempDatabase.db3" ]; then
  	echo "Creating database"
        sqlite3 tempDatabase.db3 "CREATE TABLE tempData (time_stamp DATETIME, sensor NUMERIC, temp NUMERIC);"
fi
exec 3<&0
exec 0<"$FILE"
while true
do
 
	# use $line variable to process line in processLine() function
	while read -r line
        do
              LogLine $line
        done
       sleep 2 # This delay can be changed to match the delay of the Arduino
done
exec 0<&3
exit 0
