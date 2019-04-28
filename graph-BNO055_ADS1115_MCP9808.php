<html>
<head>
<script type="text/javascript"
  src="helpers/dygraph.js"></script>
<link rel="stylesheet" src="helpers/dygraph.css" />
</head>
<body>
<big><b><center>Unit1</center></b></big>


<br><b><a href="ark/volts.csv">Volts Data</a><br></b>
<div id="graphdiv2"
  style="width:1000px; height:150px;"></div>
  
<br><b><a href="ark/orientation.csv">Orientation Data</a><br></b>
<div id="graphdiv4"
  style="width:1000px; height:150px;"></div>
  
      
<?PHP
$file = escapeshellarg("ark/p1f.csv"); // for the security concious (should be everyone!)
$linep1f = `tail -n 1 $file`; ?>

<br><b><a href="ark/temperature.csv">Temperature Data</a><br></b>
<div id="graphdiv3"
  style="width:1000px; height:50px;"></div>

<br><b><a href="ark/p1f.csv">Power (V)</a><br></b>
<div id="graphdiv5"
  style="width:1000px; height:50px;"></div>

<?PHP
$file = escapeshellarg("ark/p2f.csv"); // for the security concious (should be everyone!)
$linep2f = `tail -n 1 $file`; ?>
<br><b><a href="ark/p2f.csv">Power (%)</a><br></b>

    <div id="graphdiv6"
  style="width:1000px; height:50px;"></div>
<script type="text/javascript">
  g2 = new Dygraph(
    document.getElementById("graphdiv2"),
    "ark/volts.csv", // path to CSV file
    {}          // options
  );
</script>
<script type="text/javascript">
  g2 = new Dygraph(
    document.getElementById("graphdiv3"),
    "ark/temperature.csv", // path to CSV file
    {}          // options
  );
</script>
<script type="text/javascript">
  g2 = new Dygraph(
    document.getElementById("graphdiv4"),
    "ark/orientation.csv", // path to CSV file
    {}          // options
  );
</script>
<script type="text/javascript">
  g2 = new Dygraph(
    document.getElementById("graphdiv5"),
    "ark/p1f.csv", // path to CSV file
    {}          // options
  );
</script>
<script type="text/javascript">
  g2 = new Dygraph(
    document.getElementById("graphdiv6"),
    "ark/p2f.csv", // path to CSV file
    {}          // options
  );
</script>
<?PHP
$file = escapeshellarg("ark/system.csv"); // for the security concious (should be everyone!)
$line = `tail -n 1 $file`;
$lines = (explode(",",$line));
echo "\n<br>Last system message received at " . $lines[0] . ", " . $lines[1];
$linesp1f = (explode(",",$linep1f));
$linesp2f = (explode(",",$linep2f));
echo "\n<br>Last publish received at " . $linesp1f[0] . ".  Volts: " . $linesp1f[1] . "- SoC: " . $linesp2f[1];
?>
</body>
</html>
