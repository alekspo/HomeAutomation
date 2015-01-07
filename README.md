HomeAutomation
==============

A little project to monitor sensors around the house, storing them in a database and displaying them with a web interface.

Borrowed code from http://combustory.com/wiki/index.php/Arduino_Communications_-_SQLite

And got the idé from
http://raspberrywebserver.com/cgiscripting/rpi-temperature-logger/

Now there is only one temperature sensor active, the read_arduino.sh reads the
sensor and stores it in the database tempDatabase.db3. It is stored in table
tempData.
This is an exsample of the data in tempData table:
The first colomn is unix time teh measurement was taken, colomn two is the
sensor name, the last is the sensor data.

#+BEGIN_EXSAMPLE
1420626555|1|22.6
1420626557|1|23.39
1420626559|1|23.84
1420626561|1|24.08
1420626563|1|24.24
1420626565|1|24.34
1420626567|1|24.46
1420626569|1|25.22
1420626571|1|24.96
1420626573|1|24.97
1420626575|1|25.18
1420626577|1|24.72
1420626579|1|24.34
1420626581|1|23.99
1420626583|1|23.67
#+END_EXSAMPLE
