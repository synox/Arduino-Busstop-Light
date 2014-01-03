A small LED that shows when it is the right time to leave and catch a bus. 

![Example installation](images/example.jpg)

It is realized with a [Spark Core Firmware](http://spark.io) and a PHP Webservice. The data source is http://transport.opendata.ch/

The project was inspired by Bastian Widmer (bastianwidmer.ch) and Christian Leu (leumund.ch)

Introduction
============
This script consists of two parts. A PHP-Webservice which gets the
timetable data for the public transport from http://transport.opendata.ch and
a Firmware which connects to the service and signals back to the
user if he can catch the next bus or not ;-)


Needed Material
===============
* Spark Core Board (has LED onboard)
* Webhost with PHP enabled
* WiFi Connection


Additional Resources
======================
* http://leumund.ch/d-i-y-busstop-lamp-arduino-0011112
* https://github.com/dasrecht/Arduino-Busstop-Light


Licence
=======
      To the extent possible under law, the person who associated CC0 with
      "Spark Core Bus Stop Light" has waived all copyright and related or neighboring rights
      to "Spark Core Bus Stop Light"
      
      See http://creativecommons.org/publicdomain/zero/1.0/ for a copy of the CC0 legalcode.  
