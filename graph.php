<html>
  <head>
    <script type="text/javascript"
            src="helpers/dygraph.js">
    </script>
    <link rel="stylesheet" src="helpers/dygraph.css" />
  </head>
  <body>
    <br>
    <b>
      <a href="ark/lf.csv">Sensors 1 though 6
      </a>
      <br>
    </b>
    <div id="graphdiv2"
         style="width:1000px; height:300px;">
    </div>
    <?PHP
$file = escapeshellarg("ark/p1f.csv"); // for the security concious (should be everyone!)
$linep1f = `tail -n 1 $file`; ?>
    <br>
    <b>
      <a href="ark/p1f.csv">Power (V)
      </a>
      <br>
    </b>
    <div id="graphdiv3"
         style="width:1000px; height:100px;">
    </div>
    <?PHP
$file = escapeshellarg("ark/p2f.csv"); // for the security concious (should be everyone!)
$linep2f = `tail -n 1 $file`; ?>
    <br>
    <b>
      <a href="ark/p2f.csv">Power (%)
      </a>
      <br>
    </b>
    <div id="graphdiv4"
         style="width:1000px; height:100px;">
    </div>
    <script type="text/javascript">
      g2 = new Dygraph(
        document.getElementById("graphdiv2"),
        "ark/lf.csv", // path to CSV file
        {
        }
        // options
      );
    </script>
    <script type="text/javascript">
      g2 = new Dygraph(
        document.getElementById("graphdiv3"),
        "ark/p1f.csv", // path to CSV file
        {
        }
        // options
      );
    </script>
    <script type="text/javascript">
      g2 = new Dygraph(
        document.getElementById("graphdiv4"),
        "ark/p2f.csv", // path to CSV file
        {
        }
        // options
      );
    </script>
    <?PHP
$file = escapeshellarg("ark/system.csv"); // for the security concious (should be everyone!)
$line = `tail -n 1 $file`;
$lines = (explode(",",$line));
echo "\n<br>Last system message received at " . $lines[0] . ", " . $lines[1];
$linesp1f = (explode(",",$linep1f));
$linesp2f = (explode(",",$linep2f));
echo "\n<br>Last publish received at " . $linesp1f[0] . ".\n<br>Volts: " . $linesp1f[1] . "- SoC: " . $linesp2f[1];
?>
  </body>
</html>

