<?php 
/* *
 * PHP Webservice for the Arduino Busstop Light
 *
 * Written by Bastian Widmer (bastianwidmer.ch)
 *
 * Just change the $url $walkingtime and $*Time to your needs.
 * 
 * The Parameters from= and to= must be passed in the request. 
 * example: index.php?from=bern&to=Bern,Ostermundigen
 *
 * */
include ('simple_html_dom.php');

$walkingtime = 180; // Define walking Time to the Bus Stop
$redTime = 60; // too late
$orangeTime = 3*60; // run!
$greenTime = 7*60; // walk now
$offTime = $greenTime; // before green, it is off



$url = 'http://fahrplan.search.ch/'.urldecode($_GET["from"]).'/'.urldecode($_GET["to"]).''; // Get the timetable data url from fahrplan.search.ch

$html = file_get_html($url);

// if there's a error returned in our query return error and the message and exit
if ($html->find('div.sl_warning', 0)) {
    echo "error - ".$html->find('div.sl_warning', 0)->plaintext;
    exit;
}

// set timezone
date_default_timezone_set("Europe/Zurich");


// TODO: it should show green if there is at least one good connection. (Problem now: if there are many connections, it shows always red)

// calculate next busstop
foreach ($html->find('span.oev_printbold') as $e) {//Get the First DIV with StartData 
    $time = substr(trim($e->innertext), 0, 5);
    $nextbus = strtotime($time); // Strip unneccesary data and convert to a timestamp
    $now = time(); 
    $diff = $nextbus - $now; // calculate the difference for the events
    if ($diff < $walkingtime) {
        if ($diff < -60) {
        	// if we get a negative value of time (can happen if the server time is not 100% accurate) skip this result and wait for the next bus
          continue;
        } 
        echo "red - ".$diff;
    } else {
        if ($diff <= $redTime) {
            echo "red - ".$diff;
        } if ($diff <= $orangeTime) {
            echo "orange - ".$diff;
        } if ($diff <= $greenTime) {
            echo "green - ".$diff;
        } else {
            echo "off - ".$diff;
        }
    }
    echo "\n";
    exit;
}
