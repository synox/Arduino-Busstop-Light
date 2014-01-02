<?php 
/* *
 * PHP Webservice for the Busstop Light
 * The Parameters from= and to= must be passed in the request. 
 * example: index.php?from=bern&to=Bern,Ostermundigen
 * */

// check params
if(!array_key_exists("from", $_GET) || !array_key_exists("to", $_GET) ) {
	die("params from and to missing");
}

// set timezone
date_default_timezone_set("Europe/Zurich");


// http://transport.opendata.ch/#connections
$url = 'http://transport.opendata.ch/v1/connections?from='.urldecode($_GET["from"]).'&to='.urldecode($_GET["to"]).'&fields[]=connections/from/departure&limit=3'; 
$html = file_get_contents($url);

// example response:'{"connections":[{"from":{"departure":"2014-01-02T15:38:00+0100"}},{"from":{"departure":"2014-01-02T15:40:00+0100"}},{"from":{"departure":"2014-01-02T15:43:00+0100"}},{"from":{"departure":"2014-01-02T15:45:00+0100"}}]}';
$connections = json_decode($html)->connections;

// resulting colors, array of options. in the end the post color will be shown.
$colors = array();
$now = time(); 

foreach($connections as $con) {
	$timeStr =  $con->from->departure; 
	$ts = strtotime($time); // Strip unneccesary data and convert to a timestamp

  $diff = $nextbus - $now; // calculate the difference for the events
    
    if ($diff < 90) {
    	// too late
    	$colors["red"] = true;
    } else if ($diff < 3 * 60 ) {
    	// hurry
    	$colors["orange"] = true;
    } else if ($diff < 5 * 60 ) {
    	// now it is good
    	$colors["green"] = true;
    } else {
    	// longer, then turn led off
    } 
}
if(array_key_exists("green",$colors) ) {
	echo "(--)green\n";
} else if(array_key_exists("orange",$colors) ) {
	echo "(--)orange\n";
} else if(array_key_exists("red",$colors) ) {
	echo "(--)red\n";
} else {
	echo "(--)off\n";
}
exit;
?>