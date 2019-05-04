<?php

if (!isset($_GET["data"]))
	{
	echo "404";
	die();
	}

$incoming = htmlspecialchars($_GET["data"]); // GET THE PUBLISHED INFORMATION SENT
$coreid = htmlspecialchars($_GET["coreid"]);
$published = htmlspecialchars($_GET["published_at"]); // GET THE TIME SENT
$published = str_replace("-", "/", $published);
$date = substr($published, 0, 10); // CLEAN UP THE DATE / TIME RVCD SO WE CAN USE IT BETTER IN CSV FILE LATER
$time = substr($published, 11, 8);
$datetime = $date . " " . $time;
$datetime2 = $datetime;

if ((substr($incoming, 0, 1)) == "9")
	{
	echo "* * * " . $incoming . " * * *";
	$fp = fopen('ark/system.csv', 'a');
	fwrite($fp, $datetime . "," . substr($incoming,4,200) . "\n");
	fclose($fp);

// THIS IS TO WRITE TO ALERT INFORMATION 'ON THE INSTANT' TO THE CSV FILES, THEY CAN'T REALLY BE IN THE NORMAL STRING SINCE THEY BREAK
// THE TIME FORMAT THAT IS USED A A STANDARD WITHIN THE REPORTING FEATURE. IT GRABS THIS DATA FROM THE SYSTEM STATUS PUBLISH '9'

	if (strpos($incoming, "9 - OOS:") !== false) {		
		$pieces = explode("'", $incoming);	
		$odd=array();
		$even=array();
		$count=1;
		foreach($pieces as $val)
		{
			if($count%2==1)
			{ $odd[]=$val;
			} else 
			{ $even[]=$val;
			} $count++;
		}

		$fp = fopen('ark/volts.csv', 'a'); // WRITE SENSOR DATA TO CSV FILE NAMED lf.csv LOCATED IN THE ark FOLDER
		fwrite($fp, $datetime . "," . $even[0] . ",". $even[1] . ",". $even[2] . ",". $even[3] . "\n");
		fclose($fp);

		$fp = fopen('ark/temperature.csv', 'a'); // WRITE SENSOR DATA TO CSV FILE NAMED lf.csv LOCATED IN THE ark FOLDER
		fwrite($fp, $datetime . "," . ($even[4] / 10) . "\n");
		fclose($fp);

		$fp = fopen('ark/orientation.csv', 'a'); // WRITE SENSOR DATA TO CSV FILE NAMED lf.csv LOCATED IN THE ark FOLDER
		fwrite($fp, $datetime . "," . $even[5] . ",". $even[6] . ",". $even[7] . "\n");
		fclose($fp);

}

// END OF ON THE INSTANT ALERT VALUE PUBLISHING

	die();
	}

echo "Publish string: \n" . $incoming . "\n\n";
$timeoffset = substr($incoming, (strpos($incoming, "T")+1));
$striplen1 = strlen($timeoffset) + 1; // CALCULATE WHERE TO CUT THE EXTRA DATA FROM SENSOR DATA
$incoming = substr($incoming, 0, ($length - $striplen1)); // CUT OFF EXTRA DATA - JUST LEAVE SENSOR DATA
echo "Publish string: \n" . $incoming . "\n\n";



$inlength = strlen($incoming);

//	$rssi = substr($incoming, -5);
//	$qual = substr($incoming, -10, 5);
//	$involts = substr($incoming, -14, 4);  // GET THE VOLTAGE
//	$insoc = substr($incoming, -17, 3); // GET THE SOC

$involts = substr($incoming, -4); // GET THE VOLTAGE
$insoc = substr($incoming, -7, 3); // GET THE SOC
$intrvl = substr($incoming, strpos($incoming, 'E') + 1, ($length - 7));
$striplen = strlen($intrvl) + 8; // CALCULATE WHERE TO CUT THE EXTRA DATA FROM SENSOR DATA
$process = substr($incoming, 0, ($length - $striplen)); // CUT OFF EXTRA DATA - JUST LEAVE SENSOR DATA
$isalert = substr($incoming, 0, 1);
$number_sensors = substr($incoming, 1, 1);

/////////////////////////////
$sectodelc = $intrvl - $timeoffset;
$datetime2 = date('Y/m/d H:i:s', strtotime('-' . $sectodelc . ' seconds', strtotime($datetime)));
/////////////////////////////

echo "Length: " . $inlength . ", Battery SoC: " . ($involts / 100) . ", Battery Volts: " . ($insoc / 100) . "\n";
echo "Alert Flag: " . $isalert . ", Number of sensors: " . $number_sensors . ".\n";

for ($i = 0; $i < $number_sensors; $i++)
	{
	echo "Sensor " . ($i + 1) . ": " . substr($incoming, ($i + 2) , 1) . " chars    ";
	}

$preamble = $number_sensors + 2;
$rawdata = substr($process, $preamble);

for ($a = 0; $a < $number_sensors; $a++)
	{
	$sensetln = $sensetln + (substr($incoming, ($a + 2) , 1));
	}

$setspublished = strlen($rawdata) / $sensetln;

$roundmin = ($intrvl * $setspublished) / 60;

$roundtime = number_format((float)$roundmin, 1, '.', '');

echo "Sample Interval: " . $intrvl . " - Sample Rounds: " . $setspublished . ", with a publish interval time of " . $roundtime. " minutes.\n";

$delin = $setspublished;

$b = 0;
$build2 = "";

        for ($x = 0; $x < $setspublished; $x++)
        	{
        	///volts
        		$buildv = substr($rawdata, $b, (substr($incoming, (0 + 2) , 1)));
        		$b = $b + (substr($incoming, (0 + 2) , 1));
        		$buildv2.= $buildv . ",";
        		
        		$buildv = substr($rawdata, $b, (substr($incoming, (1 + 2) , 1)));
        		$b = $b + (substr($incoming, (1 + 2) , 1));
        		$buildv2.= $buildv . ",";
        		
        		$buildv = substr($rawdata, $b, (substr($incoming, (2 + 2) , 1)));
        		$b = $b + (substr($incoming, (2 + 2) , 1));
        		$buildv2.= $buildv . ",";
        		
        		$buildv = substr($rawdata, $b, (substr($incoming, (3 + 2) , 1)));
        		$b = $b + (substr($incoming, (3 + 2) , 1));
        		$buildv2.= $buildv . ",";
        	///volts
        	
        	///temp
        		$buildt = substr($rawdata, $b, (substr($incoming, (4 + 2) , 1)));
        		$buildt = $buildt / 10; //FIX TEMP
        		$b = $b + (substr($incoming, (4 + 2) , 1));
        		$buildt2.= $buildt . ",";
        	///temp
        	
        	///orientation
        		$buildo = substr($rawdata, $b, (substr($incoming, (5 + 2) , 1)));
        		$b = $b + (substr($incoming, (5 + 2) , 1));
        		$buildo2.= $buildo . ",";
        		
        		$buildo = substr($rawdata, $b, (substr($incoming, (6 + 2) , 1)));
        		$b = $b + (substr($incoming, (6 + 2) , 1));
        		$buildo2.= $buildo . ",";
        		
        		$buildo = substr($rawdata, $b, (substr($incoming, (7 + 2) , 1)));
        		$b = $b + (substr($incoming, (7 + 2) , 1));
        		$buildo2.= $buildo . ",";
            ///orientation
            
        	$buildv3 = substr($buildv2, 0, (strlen($buildv2) - 1));
        	$inArray[0][$x] = $buildv3;
        	$buildv1 = ""; $buildv2 = ""; $buildv3 = "";
        	
        	$buildt3 = substr($buildt2, 0, (strlen($buildt2) - 1));
        	$inArray[1][$x] = $buildt3;
        	$buildt1 = ""; $buildt2 = ""; $buildt3 = "";
        	
        	$buildo3 = substr($buildo2, 0, (strlen($buildo2) - 1));
        	$inArray[2][$x] = $buildo3;
        	$buildo1 = ""; $buildo2 = ""; $buildo3 = "";
        	}


$sectodel = $intrvl * $setspublished;
$timestamp = date('Y/m/d H:i:s', strtotime('-' . $sectodel . ' seconds', strtotime($datetime2)));
$datetime = $timestamp;

for ($i = 0; $i < $setspublished; $i++)
	{
	$sectoadd = ($intrvl * ($i + 1));
	$datetimeN = date('Y/m/d H:i:s', strtotime('+' . $sectoadd . ' seconds', strtotime($timestamp)));
	$inArray[0][$i] = $datetimeN . "," . $inArray[0][$i] . "\n";
	}

for ($i = 0; $i < $setspublished; $i++)
	{
	$sectoadd = ($intrvl * ($i + 1));
	$datetimeN = date('Y/m/d H:i:s', strtotime('+' . $sectoadd . ' seconds', strtotime($timestamp)));
	$inArray[1][$i] = $datetimeN . "," . $inArray[1][$i] . "\n";
	}
	
for ($i = 0; $i < $setspublished; $i++)
	{
	$sectoadd = ($intrvl * ($i + 1));
	$datetimeN = date('Y/m/d H:i:s', strtotime('+' . $sectoadd . ' seconds', strtotime($timestamp)));
	$inArray[2][$i] = $datetimeN . "," . $inArray[2][$i] . "\n";
	}	


$fp = fopen('ark/volts.csv', 'a'); // WRITE SENSOR DATA TO CSV FILE NAMED lf.csv LOCATED IN THE ark FOLDER

for ($i = 0; $i < ($delin); $i++)
	{
	fwrite($fp, $inArray[0][$i]);
	}
fclose($fp);


$fp = fopen('ark/temperature.csv', 'a'); // WRITE SENSOR DATA TO CSV FILE NAMED lf.csv LOCATED IN THE ark FOLDER

for ($i = 0; $i < ($delin); $i++)
	{
	fwrite($fp, $inArray[1][$i]);
	}
fclose($fp);

$fp = fopen('ark/orientation.csv', 'a'); // WRITE SENSOR DATA TO CSV FILE NAMED lf.csv LOCATED IN THE ark FOLDER

for ($i = 0; $i < ($delin); $i++)
	{
	fwrite($fp, $inArray[2][$i]);
	}
fclose($fp);


$fp = fopen('ark/p1f.csv', 'a'); // WRITE SOC DATA TO CSV FILE NAMED p1f.csv
fwrite($fp, $datetime2 . "," . ($insoc / 100) . "\n");
fclose($fp);

$fp = fopen('ark/p2f.csv', 'a'); // WRITE VOLTS DATA TO CSV FILE NAMED p2f.csv
fwrite($fp, $datetime2 . "," . ($involts / 100) . "\n");
fclose($fp);

echo "Received particle publish marked with time: " . $datetime2 . ", calculated time: " . $datetimeN . "!"; // LET THE WEBHOOK KNOW WE RCVD AND AT WHAT TIME

?>
