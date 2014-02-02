A small Colored LED that shows when it is the right time to leave and catch a bus. (Green=Good to go, Orange=Run!, Red=too late)

![Example installation](images/display.jpg)  
_Example with display._

![Example installation](images/example.jpg)  
_Example installation made out of paper._




**Data**: The data source is http://transport.opendata.ch/. A php script is used for parsing the data and passing to the device.. 

**Device**: It is realized with a [Spark Core Firmware](http://spark.io). It calls the transport api over HTTP and parses the result. 


Needed Material
----------------
* Spark Core Board (has LED onboard)
* WiFi Connection


Installation 
----------------
1. Configure the values and url in the [Spark Core firmware](/spark_core/application.cpp)
2. [Connect the Spark Core](https://www.spark.io/start) to your wifi
3. Deploy the firmware using the [Spark Cloud](https://www.spark.io/build).

Additional Resources
----------------
* Thanks: The project was inspired by Bastian Widmer (bastianwidmer.ch) and Christian Leu (leumund.ch)
* http://leumund.ch/d-i-y-busstop-lamp-arduino-0011112
* https://github.com/dasrecht/Arduino-Busstop-Light


Licence
----------------
      To the extent possible under law, the person who associated CC0 with
      "Spark Core Bus Stop Light" has waived all copyright and related or neighboring rights
      to "Spark Core Bus Stop Light"
      
      See http://creativecommons.org/publicdomain/zero/1.0/ for a copy of the CC0 legalcode.  
