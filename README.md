Arduino Script and Webservice to decide if the time is right for catching the next bus.

Fork for [spark.io](http://spark.io)

Inspired and Adapted from Bastian Widmer (bastianwidmer.ch), Christian Leu (leumund.ch)

Introduction
============
This script consists of two parts. A PHP-Webservice which gets the
timetable data for the public transport from fahrplan.search.ch and
a Arduino sketch which connects to the service and signals back to the
user if he can catch the next bus or not ;-)

The Webservice makes use of the [PHP Simple HTML DOM Parser](http://simplehtmldom.sourceforge.net/).

Needed Material
===============
* Spark Core Board (has LED onboard)

Additional Resources
======================
http://leumund.ch/d-i-y-busstop-lamp-arduino-0011112
https://github.com/dasrecht/Arduino-Busstop-Light


Licence
=======

  DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
                    Version 2, December 2004

 Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>

 Everyone is permitted to copy and distribute verbatim or modified
 copies of this license document, and changing it is allowed as long
 as the name is changed.

            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

  0. You just DO WHAT THE FUCK YOU WANT TO.

