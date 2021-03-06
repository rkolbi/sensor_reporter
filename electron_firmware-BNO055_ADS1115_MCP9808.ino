//////////////////////////////////////////////////////////////////////////////////////////////////////
//     THIS SKETCH IS TO GATHER READINGS FROM SENSORS, IN THIS CASE THE BNO055, ADS1115, AND        //
//     MCP9808, AT A USER SET TIME INTERVAL (SET IN SECONDS) TO THEN BE PUBLISHED OUT ONCE THE      //
//     DESIRED NUMBER OF SAMPLES HAS BEEN OBTAINED.                                                 //
//     DURING THIS, IF AN EVENT IS TRIGGERED - A STATE WHERE IF ANY ONE OF THE SENSOR'S VALUE       //
//     DIFFERS MORE THEN THE LAST FOUR VALUES AVERAGED TOGETHER + SENSOR ALERT SETPOINT             //
//     (sensor_alert_thrshld[X]), THE SENSOR SAMPLE TIME INTERVAL IS CHANGED UNTIL THE DEFINED      //
//     NUMBER OF PUBLISHED ALERT EVENTS HAS OCCURRED.                                               //
//     CONFIGURABLE SETTINGS ARE BELOW, UNDER *USER CONFIGURABLE PARAMETERS*			    //
//												    //
//	Following components are used:								    //
//	   -Pololu #2562, 5V Step-Up Voltage Regulator, https://www.pololu.com/product/2562/   	    //
//	    (100k onboard pullup removed, replaced with external 10k pulldown) 			    //
//	   -ADS1115 16-Bit ADC - 4 Channel, https://www.adafruit.com/product/1085		    //
//	   -MCP9808 High Accuracy I2C Temperature Sensor, https://www.adafruit.com/product/1782	    //
//	   -BNO055 9-DOF Absolute Orientation IMU Fusion, https://www.adafruit.com/product/2472	    //
//////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Particle.h"
#include "particle-BNO055.h"
#include <Wire.h>
#include <ADS1115.h>
#include "MCP9808.h"
#include <math.h>

SYSTEM_MODE(AUTOMATIC)
SYSTEM_THREAD(ENABLED)

ADS1115 ads;
MCP9808 mcp = MCP9808();
Adafruit_BNO055 bno = Adafruit_BNO055(55);

///\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ BEGIN USER CONFIGURABLE PARAMETERS \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/

// SAMPLE & PUBLISH SETTINGS //
int norm_smpl_intvl = 60;	// SAMPLE INTERVAL, HOW MANY SECONDS TO WAIT BETWEEN EACH
const int rnds_to_publish = 5;	// WAIT UNTIL x NUMBER OF SAMPLES ARE GATHERED TO BURST PUBLISH
int do_publish = 1;		// PERFORM PARTICLE.PUBLISH EVENTS

// SENSOR REPORT CONFIG
const int no_of_sensors = 8; 	// THE NUMBER OF SENSOR READINGS TO COLLECT PER SAMPLE
// 				▼ THE LENGTH OF EACH SENSOR'S VALUE
int sensorlen[no_of_sensors] = {4, 4, 4, 4, 3, 3, 3, 3};

// ALAERT SETTING //
int alert_smpl_intvl = 10;	 // ALERT SAMPLE INTERVAL, HOW MANY SECONDS TO WAIT BETWEEN EACH
const int alrt_publish_rnds = 2; // WHILE IN SENSOR ALERT - HOW MANY COMPLETE PUBLISH CYCLES TO LOOP THROUGH
// 					   ▼ SET ARRAY FOR SENSOR ALERT
int sensor_alert_thrshld[no_of_sensors] = {999, 999, 999, 999, 2, 15, 15, 15};

// POWER SETTINGS //
int solar_opt = 0;		// SOLAR OPTIMIZATION, 0 = SOLAR OPT OFF, 5 = 5V SOLAR PANEL, 6 = 6V SOLAR PANEL
int enable_wop = 0;		// ENABLE WAKE-ON-PING
int sleep = 1;			// SLEEP MODE ON = 1 / SLEEP MODE OFF = 0
int sleep_wait = 1;		// TIME TO WAIT AFTER PUBLISH TO FALL ASLEEP
int secs_less_intrvl_sleep = 0; // ADDITIONAL SECONDS TO DELAY FROM SAMPLE INTERVAL FOR SLEEP TIME, 5 SECONDS ARE ALREADY SUBTRACTED
int app_watchdog = 360000;	// APPLICATION WATCHDOG TIME TRIGGER IS MS - SLEEP TIME SHOULD NOT BE COUNTED

///\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ END USER CONFIGURABLE PARAMETERS \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/

// DECLARE ITEMS USED AND SET INITIAL PARAMETERS
int alert_sample_qty = ((rnds_to_publish * alrt_publish_rnds) + 1); // HOW MANY SENSOR SAMPLES TO TAKE WHILE IN EVENT/ALERT MODE
float storage[(rnds_to_publish + 1)][no_of_sensors];	// DIMENSION THE STORAGE CONTAINER ARRAY
int current_smpl_intvl = norm_smpl_intvl;		// SET CURRENT SAMPLE INTERVAL TO NORMAL SAMPLE INTERVAL AS ABOVE
char fullpublish[500];					// CONTAINS THE STRING TO BE PUBLISHED
char fullpub_temp1[16];					// TEMPORARY HOLDING STRING USED FOR FULLPUBLISH EVENT
int alrt_ckng_tmp = 0;					// USED AS A TEMP CONTAINER IN SENSOR ALERT CHECKING
int w = 0;						// THE NUMBER OF SAMPLES TO GATHER
int x = 0;						// THE NUMBER OF SAMPLES COUNTER
int xa = 0;						// THE NUMBER OF SAMPLES COUNTER in ALERT STATE
int y = 0;						// THE SAMPLE PER COUNT GATHERED
int smpls_performed = 0;				// COUNTER FOF ALL SAMPLES PERFORMED
int sensor_event_chk = 0;				// SENSOR SAMPLE EVENT CHECK
int sensor_hist_depth = 0;				// SENSOR SAMPLE EVENT CHECK DEPTH
int alert_cycl_mark = 0;				// CYCLE MARK BASED ON 'smpls_performed' TO CONTINUE ALERT REPORTING
int t2 = 0;						// EQUALS TIME(0) + norm_smpl_intvl, USED TO DETERMINE WHEN TO TAKE SAMPLE
int alrt_state_chng = 0;				// CHANGE IN ALERT STATE FLAG
float sensor_value[no_of_sensors][7];			// DIMENSION THE SENSOR VALUE ARRAY
int vcell;						// BATTERY INFO
int vsoc;						// BATTERY INFO
int a = 0;						// GP TIMER USE
int published_norm1_or_alert2 = 0;			// TYPE OF PUBLISH LAST PERFORMED
int sla = 0;						// USED IN CREATION OF PREAMBLE
int led1 = D7;						// ONBOARD BLUE LED
int volt5 = D6;						// 5V STEP-UP VOLTAGE REGULATOR ON/OFF CONTROL (SAME CONTROL AS LED) 
int16_t adc0, adc1, adc2, adc3;				// IMU
float adcx0, adcx1, adcx2, adcx3;			// IMU
int sleeptime = 0;					// SLEEP DURATION
// END OF DECLARATIONS AND INITIAL PARAMETERS

void i2cWake()
{
	//WAKE UP i2c DEVICES
	digitalWrite(volt5, HIGH);			// TURN ON 5 VOLT REGULATOR
	ads.setMode(MODE_CONTIN);			// SET ADC SAMPLING MODE AS CONTINUOUS
	mcp.setPowerMode(MCP9808_CONTINUOUS);		// SET TEMP SENSOR MODE AS CONTINUOUS 
	Wire.beginTransmission(0x28);			// TURN IMU ON
	Wire.write(0x3E);				// TURN IMU ON
	Wire.write(0x00);				// TURN IMU ON
	Wire.endTransmission();				// TURN IMU ON
}

void i2cSleep()
{
	//PUT i2c DEVICES TO SUSPEND
	ads.setMode(MODE_SINGLE);			// SET ADC SAMPLING MODE AS CONTINUOUS
	mcp.setPowerMode(MCP9808_LOW_POWER);		// SET TEMP SENSOR MODE AS CONTINUOUS
	Wire.beginTransmission(0x28);			// TURN IMU ON
	Wire.write(0x3E);				// TURN IMU ON
	Wire.write(0x02);				// TURN IMU ON
	Wire.endTransmission();				// TURN IMU ON
	digitalWrite(volt5, LOW);			// TURN OFF 5 VOLT REGULATOR
}

void preamble()
{
	// PUT PREAMBLE ON DATA
	fullpub_temp1[0] = 0;
	fullpublish[0] = 0;

	snprintf(fullpub_temp1, sizeof(fullpub_temp1), "%d%d", alrt_state_chng, no_of_sensors);
	strncat(fullpublish, fullpub_temp1, sizeof(fullpublish) - strlen(fullpublish) - 1);

	for (sla = 0; sla < no_of_sensors; sla++)
	{
		snprintf(fullpub_temp1, sizeof(fullpub_temp1), "%d", sensorlen[sla]);
		strncat(fullpublish, fullpub_temp1, sizeof(fullpublish) - strlen(fullpublish) - 1);
	}
}

void fullpublishout()
{
	Particle.publish("a", fullpublish, 60, PRIVATE, NO_ACK); // PARTICLE.PUBLISH EVENT ! ! ! !

	digitalWrite(led1, HIGH);

	for (uint32_t ms = millis(); millis() - ms < 500; Particle.process())
		;
	digitalWrite(led1, LOW);
}

void setup()
{
	Serial.begin(9600);

	pinMode(led1, OUTPUT);
	pinMode(volt5, OUTPUT);

	digitalWrite(volt5, HIGH);

	// SOLAR SETTINGS //
	if (solar_opt)
	{
		PMIC pmic; //INITIALIZE THE PMIC CLASS TO CALL THE POWER MANAGEMENT FUNCTIONS BELOW

		pmic.setChargeCurrent(0, 0, 1, 0, 0, 0); //SET CHARGING CURRENT TO 1024MA (512 + 512 OFFSET)

		if (solar_opt == 5)
			pmic.setInputVoltageLimit(4840); //SET THE LOWEST INPUT VOLTAGE TO 4.84 VOLTS, FOR 5V SOLAR PANEL

		if (solar_opt == 6)
			pmic.setInputVoltageLimit(5080); //SET THE LOWEST INPUT VOLTAGE TO 5.08 VOLTS, FOR 6V SOLAR PANEL
	}

	Wire.begin(); // INITIALIZE I2C
	for (uint32_t ms = millis(); millis() - ms < 500;)
		;

	bno.begin(); // INITIALIZE IMU
	for (uint32_t ms = millis(); millis() - ms < 500;)
		;

	bno.setExtCrystalUse(true);
	ads.getAddr_ADS1115(ADS1115_DEFAULT_ADDRESS); // (ADDR = GND)
	ads.setGain(GAIN_TWOTHIRDS);	// 2/3x gain +/- 6.144V  1 bit = 0.1875mV
	ads.setMode(MODE_CONTIN);	// ads.setMode(MODE_CONTIN) = Continuous conversion mode - ads.setMode(MODE_SINGLE); Power-down single-shot mode (default)
	ads.setRate(RATE_128);		// 128SPS (default) 8,16,32,64,128,250,475,860
	ads.setOSMode(OSMODE_SINGLE);	// Set to start a single-conversion
	ads.begin();
	mcp.setResolution(MCP9808_SLOWEST);

	i2cWake();
}

ApplicationWatchdog wd(app_watchdog, System.reset);

void loop()
{
	FuelGauge fuel;

	if (a == 0 && fuel.getSoC() > 20)
	{
		if (enable_wop)
			Cellular.command("AT+URING=1\r\n");

		for (uint32_t ms = millis(); millis() - ms < 1000; Particle.process())
			Particle.publish("a", "9 - DEVICE ONLINE!", 60, PRIVATE, NO_ACK); // LET SERVER KNOW ONLINE!

		digitalWrite(led1, HIGH);

		for (uint32_t ms = millis(); millis() - ms < 1000; Particle.process())
			;

		digitalWrite(led1, LOW);
		a = 2;
	}

	if (fuel.getSoC() < 20) //LOW BATTERY DEEP SLEEP
	{
		if (a != 0)
			Particle.publish("a", "9 - DEVICE SHUTTING DOWN, BATTERY SoC < 20!", 60, PRIVATE, NO_ACK); // LET SERVER KNOW WE ARE GOING TO SLEEP

		for (uint32_t ms = millis(); millis() - ms < 5000; Particle.process()) //EXTRA TIME BEFORE DEEP SLEEP
			;

		System.sleep(SLEEP_MODE_DEEP, 7200); // SLEEP 2 HOURS IF SoC < 20
	}

	///\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ START SENSOR VALUE GATHERING \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/

	// SENSOR VALUES GO INTO THE sensor_value[X][0] ARRAY WHERE 'X' IS THE SENSOR NUMBER
	// sensor_value[1][0] = SENSOR 1, SENSOR VALUE
	// sensor_value[1][1-4] = SENSOR 1, HISTORICAL SENSOR VALUES
	// sensor_value[1][5] = SENSOR 1, HISTORICAL SENSOR VALUE AVERAGE OF ALL FOUR

	adc0 = ads.Measure_SingleEnded(0); //ADS1115 VOLTAGE READING 1
	adcx0 = ((0.1875 * adc0) / 1000);
	sensor_value[0][0] = adcx0;

	adc1 = ads.Measure_SingleEnded(1); //ADS1115 VOLTAGE READING 2
	adcx1 = ((0.1875 * adc1) / 1000);
	sensor_value[1][0] = adcx1;

	adc2 = ads.Measure_SingleEnded(2); //ADS1115 VOLTAGE READING 3
	adcx2 = ((0.1875 * adc2) / 1000);
	sensor_value[2][0] = adcx2;

	adc3 = ads.Measure_SingleEnded(3); //ADS1115 VOLTAGE READING 4
	adcx3 = ((0.1875 * adc3) / 1000);
	sensor_value[3][0] = adcx3;

	int tempget = fabs(10 * mcp.getTemperature()); //MCP9808 TEMP READING
	sensor_value[4][0] = tempget;

	sensors_event_t event; //BNO055 SAMPLE GATHERING
	bno.getEvent(&event);

	float orx = event.orientation.x; // BNO055 ORIENTATION X
	// MAKE ORX RELATIVE TO 180'
	int orxt1 = int(orx) - 180;
	orxt1 = abs((orxt1 + 180) % 360 - 180);
	int orxt2 = 180 - int(orx);
	orxt2 = abs((orxt2 + 180) % 360 - 180);
	if (orxt1 > orxt2)
		sensor_value[5][0] = float(orxt2);
	else
		sensor_value[5][0] = float(orxt1);

	float ory = event.orientation.y; // BNO055 ORIENTATION Y
	ory = ory + 180;
	sensor_value[6][0] = ory;

	float orz = event.orientation.z; // BNO055 ORIENTATION Z
	orz = orz + 180;
	sensor_value[7][0] = orz;

	///\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ END SENSOR VALUE GATHERING \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/

	///\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/GATHER, EVENT CHECK, AND PUBLISH \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/

	//-- ALERT CHECK --
	if (sensor_hist_depth == 4)
	{
		sensor_hist_depth = 1;
	}
	else
	{
		sensor_hist_depth++;
	}
	// CYCLE THROUGH 1 - 4 FOR HISTORY / AVERAGING
	for (w = 0; w < no_of_sensors; w++)
	{
		sensor_value[w][5] = ((sensor_value[w][1] + sensor_value[w][2] + sensor_value[w][3] + sensor_value[w][4]) / 4);
		sensor_value[w][sensor_hist_depth] = sensor_value[w][0];

		if (smpls_performed > 4)
		{
			alrt_ckng_tmp = fabs(sensor_value[w][0] - sensor_value[w][5]);
			sensor_value[w][6] = alrt_ckng_tmp;

			if (alrt_ckng_tmp > sensor_alert_thrshld[w])
			{
				Serial.printf("OOS Sensor %d, ", w);
				Serial.printlnf("Diff: %d, Threshold: %d.", alrt_ckng_tmp, sensor_alert_thrshld[w]);

				if (alrt_state_chng != 2)
				{
					alrt_state_chng = 1; // SIGNIFIES FRESH CHANGE INTO ALERT STATE SO PUBLISHER WILL PUBLISH STORAGE AND THEN START ALERT REPORTING
				}

				alert_cycl_mark = smpls_performed + alert_sample_qty;
			}
		}
	}
	//-- END OF ALERT CHECK

	//-- START SAMPLING TIME CHECK --
	if (Time.now() >= t2)
	{
		for (w = 0; w < no_of_sensors; w++)
		{
			storage[x][w] = sensor_value[w][0];
		} //PLACE SENSOR VALUES IN STORAGE ARRAY
		x++;

		t2 = Time.now() + current_smpl_intvl;

		if (alert_cycl_mark > smpls_performed)
		{
			Serial.println("!");
			published_norm1_or_alert2 = 2;
		}
		else
		{
			Serial.println(".");
			published_norm1_or_alert2 = 1;
		}

		smpls_performed++;

		if (smpls_performed > alert_cycl_mark)
			current_smpl_intvl = norm_smpl_intvl;

		wd.checkin(); // SOFTWARE WATCHDOG CHECK-IN

		if (alert_cycl_mark == (smpls_performed + 1))
			alrt_state_chng = 3;
	}
	//-- END SAMPLING TIME CHECK --

	//-- BEGIN ALERT STATE PUBLISH FLUSH --
	if ((alrt_state_chng == 3) || (alrt_state_chng == 1))
	{
		preamble();
		for (xa = 0; xa < x; xa++)
		{
			for (y = 0; y < no_of_sensors; y++)
			{
				if (y < 4)
				{ //THIS IS TO LET THE FIRST FOUR READINGS HAVE TWO DECIMAL PLACES
					snprintf(fullpub_temp1, sizeof(fullpub_temp1), "%0*.2f", sensorlen[y], storage[xa][y]);
				}
				else
				{
					snprintf(fullpub_temp1, sizeof(fullpub_temp1), "%0*.0f", sensorlen[y], storage[xa][y]);
				}
				strncat(fullpublish, fullpub_temp1, sizeof(fullpublish) - strlen(fullpublish) - 1); // better to check the boundaries
			}
		} // BUILT FULLPUBLISH STRING
		x = 0;
		xa = 0;

		FuelGauge fuel; // GET BATTERY INFO
		fullpub_temp1[0] = 0;

		snprintf(fullpub_temp1, sizeof(fullpub_temp1), "E%d%d%dT%ld", current_smpl_intvl, ((int)(fuel.getVCell() * 100)), ((int)(fuel.getSoC() * 100)), (t2 - Time.now())); // ADDING SAMPLE RATE, BATTERY INFO
		strncat(fullpublish, fullpub_temp1, sizeof(fullpublish) - strlen(fullpublish) - 1);

		Serial.println(fullpublish);
		Serial.printf("ASP: The size of published string is %d bytes. \n", strlen(fullpublish));

		if (do_publish)
			fullpublishout();

		if (alrt_state_chng == 1)
		{
			current_smpl_intvl = alert_smpl_intvl;
			t2 = Time.now() + current_smpl_intvl;
		}

		if (alrt_state_chng == 3)
		{
			alrt_state_chng = 0;
			current_smpl_intvl = norm_smpl_intvl;
			t2 = Time.now() + current_smpl_intvl;
		}

		//-- SEND OUT SYSTEM NOTE TO ALERT OOS AND FOR ON THE SPOT SENSOR VALUE REPORTING
		if (alrt_state_chng == 1)
		{
			fullpub_temp1[0] = 0;
			fullpublish[0] = 0;

			snprintf(fullpub_temp1, sizeof(fullpub_temp1), "9 - OOS: ");
			strncat(fullpublish, fullpub_temp1, sizeof(fullpublish) - strlen(fullpublish) - 1);

			for (y = 0; y < no_of_sensors; y++)
			{
				if (y < 4)
				{ //THIS IS TO LET THE FIRST FOUR READINGS HAVE TWO DECIMAL PLACES
					snprintf(fullpub_temp1, sizeof(fullpub_temp1), "  '%0*.2f'%s%.0f>%d", sensorlen[y], sensor_value[y][0], ((sensor_value[y][6] > sensor_alert_thrshld[y]) ? "▲" : ":"), sensor_value[y][6], sensor_alert_thrshld[y]);
				}
				else
				{
					snprintf(fullpub_temp1, sizeof(fullpub_temp1), "  '%0*.0f'%s%.0f>%d", sensorlen[y], sensor_value[y][0], ((sensor_value[y][6] > sensor_alert_thrshld[y]) ? "▲" : ":"), sensor_value[y][6], sensor_alert_thrshld[y]);
				}
				strncat(fullpublish, fullpub_temp1, sizeof(fullpublish) - strlen(fullpublish) - 1); // better to check the boundaries
			}

			FuelGauge fuel; // GET BATTERY INFO

			fullpub_temp1[0] = 0;

			Serial.println(fullpublish);
			Serial.printf("The size of published string is %d bytes. \n", strlen(fullpublish));

			if (do_publish)
				fullpublishout();

			alrt_state_chng = 2;
		}
		//-- END SEND OUT SYSTEM NOTE TO ALERT OOS
	}
	//-- END ALERT STATE PUBLISH FLUSH --

	else

	//-- START NORMAL CHECK AND PUBLISH --
	{
		if (x == rnds_to_publish)
		{
			preamble();
			for (x = 0; x < rnds_to_publish; x++)
			{
				for (y = 0; y < no_of_sensors; y++)
				{
					if (y < 4)
					{ //THIS IS TO LET THE FIRST FOUR READINGS HAVE TWO DECIMAL PLACES
						snprintf(fullpub_temp1, sizeof(fullpub_temp1), "%0*.2f", sensorlen[y], storage[x][y]);
					}
					else
					{
						snprintf(fullpub_temp1, sizeof(fullpub_temp1), "%0*.0f", sensorlen[y], storage[x][y]);
					}
					strncat(fullpublish, fullpub_temp1, sizeof(fullpublish) - strlen(fullpublish) - 1); // better to check the boundaries
				}
			} // BUILT FULLPUBLISH STRING
			x = 0;
			FuelGauge fuel; // GET BATTERY INFO
			fullpub_temp1[0] = 0;

			snprintf(fullpub_temp1, sizeof(fullpub_temp1), "E%d%d%dT%ld", current_smpl_intvl, ((int)(fuel.getVCell() * 100)), ((int)(fuel.getSoC() * 100)), (t2 - Time.now())); // ADDING SAMPLE RATE, BATTERY INFO
			strncat(fullpublish, fullpub_temp1, sizeof(fullpublish) - strlen(fullpublish) - 1);

			Serial.println(fullpublish);
			Serial.printf("The size of published string is %d bytes. \n", strlen(fullpublish));

			if (do_publish)
				fullpublishout();

			fullpublish[0] = 0; // CLEAR THE FULLPUBLISH STRING
		}
		//-- END SAMPLES TAKEN CHECK AND PUBLISH --
	}

	//-- BEDTIME CHECK --
	if (sleep == 1)
	{
		if (published_norm1_or_alert2 == 1 && norm_smpl_intvl > 23 && t2 >= (Time.now() + sleep_wait))
			sleeptime = (((norm_smpl_intvl - secs_less_intrvl_sleep) - sleep_wait) - 5);

		if (published_norm1_or_alert2 == 2 && alert_smpl_intvl > 23 && t2 >= (Time.now() + sleep_wait))
			sleeptime = (((alert_smpl_intvl - secs_less_intrvl_sleep) - sleep_wait) - 5);

		if (enable_wop == 1 && sleeptime)
		{
			i2cSleep();
			System.sleep({RI_UC, BTN}, {RISING, FALLING}, sleeptime, SLEEP_NETWORK_STANDBY);
			i2cWake();
			Cellular.command("AT+URING=0\r\n");
			Cellular.command("AT+URING=1\r\n");
		}

		if (enable_wop == 0 && sleeptime)
		{
			i2cSleep();
			System.sleep(BTN, FALLING, sleeptime, SLEEP_NETWORK_STANDBY);
			i2cWake();
		}

		sleeptime = 0;
		published_norm1_or_alert2 = 0;
	}
	//-- END BEDTIME CHECK --

	///\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ END GATHER, EVENT CHECK, AND PUBLISH \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/
	for (uint32_t ms = millis(); millis() - ms < 700;)
		;
}
