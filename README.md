sensor_reporter
======
**Sensor Reporter** is a sketch for the Particle Electron that assists getting sensor values to a website. The sensor reading interval as well as publishing interval is easily user defined. Additionally an alert feature exists where if a sensor reading changes, the sampling and publishing interval will change.

In this repository you will find the Particle Electron sketch (electron_firmware), as well as some example files to yield a web hosted graphical report.

‘particle_webhook’ is an example webhook to send data to the ‘receiver_php’ file that will reside on your webserver.

‘receiver_php’ is a php script that receives the information pushed by the particle cloud and then turns it into a useful .csv text file.

‘graph.php’ is the frontend for displaying the data contained within the csv files. It uses dygraph.min.js, dygraph.js, and dygraph.css which can be downloaded at dygraphs.com to form the charts and is very easy to modify.

