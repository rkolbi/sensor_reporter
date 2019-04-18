//////////////////////////////////////////////////////////////////////////////////////////////////////
//     THIS CODE SKETCH IS TO GATHER READINGS FROM SENSORS AT A USER SET TIME INTERVAL (SET IN      //
//     SECONDS) TO THEN BE PUBLISHED OUT ONCE THE DESIRED NUMBER OF SAMPLES HAS BEEN OBTAINED.      //
//     DURING THIS, IF AN EVENT IS TRIGGERED - A STATE WHERE IF ANY ONE OF THE SENSOR'S VALUE       //
//     DIFFERS MORE THEN THE LAST FOUR VALUES AVERAGED TOGETHER + SENSOR ALERT SETPOINT             //
//     (sensor_alert_thrshld[X]), THE SENSOR SAMPLE TIME INTERVAL IS CHANGED UNTIL THE DEFINED      //
//     NUMBER OF PUBLISHED ALERT EVENTS HAS OCCURRED. THESE SETTINGS ARE BELOW, UNDER               //
//     *USER CONFIGURABLE PARAMETERS*                                                               //
//                                                                                                  //
//    * * * NOTE THAT UNDER THE 'SENSOR VALUE EVENT CHECK' THE FOLLOWING TWO LINES WILL INSERT      //
//          A FALSE EVENT ON THE TWENTIETH SAMPLE TAKEN, ON SENSOR-3 AND AGAIN AT SAMPLE            //
//          TWENTY-THREE. DO NOT FORGET TO REMOVE THESE LINES * * *                                 //
//          if (smpls_performed == 21) sensor_value[3][5] += 100; // SIMULATE EVENT                 //
//          if (smpls_performed == 23) sensor_value[3][5] += 100; // SIMULATE EVENT                 //
//                                                                                                  //
//////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Particle.h"

#include <math.h>

void setup();
void loop();

SYSTEM_MODE(AUTOMATIC)
SYSTEM_THREAD(ENABLED)

///\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ BEGIN USER CONFIGURABLE PARAMETERS \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/

// SAMPLE & PUBLISH SETTINGS //
int norm_smpl_intvl = 88;        // SAMPLE INTERVAL, HOW MANY SECONDS TO WAIT BETWEEN EACH
const int rnds_to_publish = 15;  // WAIT UNTIL x NUMBER OF SAMPLES ARE GATHERED TO BURST PUBLISH
int do_publish = 1;              // PERFORM PARTICLE.PUBLISH EVENTS

// SENSOR REPORT CONFIG
const int no_of_sensors = 6;     // THE NUMBER OF SENSOR READINGS TO COLLECT PER SAMPLE
int sensorlen[no_of_sensors] = {3, 3, 3, 3, 3, 3}; // THE LENGTH OF EACH SENSOR'S VALUE

// ALAERT SETTING //
int alert_smpl_intvl = 30;       // ALERT SAMPLE INTERVAL, HOW MANY SECONDS TO WAIT BETWEEN EACH
const int alrt_publish_rnds = 2; // WHILE IN SENSOR ALERT - HOW MANY COMPLETE PUBLISH CYCLES TO LOOP THROUGH
int sensor_alert_thrshld[no_of_sensors] = {10, 11, 12, 13, 14, 15}; // SET ARRAY FOR SENSOR ALERT

// SLEEP SETTINGS //
int enable_wop = 1;              // ENABLE WAKE-ON-PING
int sleep = 1;                   // SLEEP MODE ON = 1 / SLEEP MODE OFF = 0
int sleep_wait = 1;              // TIME TO WAIT AFTER PUBLISH TO FALL ASLEEP
int secs_less_intrvl_sleep = 3;  // SECONDS TO DELAY FROM SAMPLE INTERVAL FOR SLEEP TIME
int app_watchdog = 90000;        // APPLICATION WATCHDOG TIME TRIGGER IS MS - SLEEP TIME SHOULD NOT BE COUNTED,
//                                  BUT THE ABOVE THREE VARIABLES SHOULD BE BUT NOT LESS THEN 30000

///\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ END USER CONFIGURABLE PARAMETERS \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/

// DECLARE ITEMS USED AND SET INITIAL PARAMETERS
int alert_sample_qty = ((rnds_to_publish * alrt_publish_rnds) + 1); // HOW MANY SENSOR SAMPLES TO TAKE WHILE IN EVENT/ALERT MODE
float storage[(rnds_to_publish + 1)][no_of_sensors];                // DIMENSION THE STORAGE CONTAINER ARRAY
int current_smpl_intvl = norm_smpl_intvl;                           // SET CURRENT SAMPLE INTERVAL TO NORMAL SAMPLE INTERVAL AS ABOVE
char fullpublish[500];                                              // CONTAINS THE STRING TO BE PUBLISHED
char fullpub_temp1[14];                                             // TEMPORY HOLDING STRING USED FOR FULLPUBLISH EVENT
char fullpub_temp2[4];                                              // TEMPORY HOLDING STRING USED FOR FULLPUBLISH EVENT
int alrt_ckng_tmp = 0;                                              // USED AS A TEMP CONTAINER IN SENSOR ALERT CHECKING
int w = 0;                                                          // THE NUMBER OF SAMPLES TO GATHER
int x = 0;                                                          // THE NUMBER OF SAMPLES COUNTER
int xa = 0;                                                         // THE NUMBER OF SAMPLES COUNTER in ALERT STATE
int y = 0;                                                          // THE SAMPLE PER COUNT GATHERED
int pubs_performs = 0;                                              // COUNTER FOF ALL PUBLISHES PERFORMED
int smpls_performed = 0;                                            // COUNTER FOF ALL SAMPLES PERFORMED
int sensor_event_chk = 0;                                           // SENSOR SAMPLE EVENT CHECK
int sensor_hist_depth = 0;                                          // SENSOR SAMPLE EVENT CHECK DEPTH
int alert_cycl_mark = 0;                                            // CYCLE MARK BASED ON 'smpls_performed' TO CONTINUE ALERT REPORTING
int t2 = 0;                                                         // EQUALS TIME(0) + norm_smpl_intvl, USED TO DETERMINE WHEN TO TAKE SAMPLE
int alrt_state_chng = 0;                                            // CHANGE IN ALERT STATE FLAG
float sensor_value[no_of_sensors][6];                               // DIMENSION THE SENSOR VALUE ARRAY
int vcell;                                                          // BATTERY INFO
int vsoc;                                                           // BATTERY INFO
int a = 0;                                                          // GP TIMER USE
int published_norm1_or_alert2 = 0;                                  // TYPE OF PUBLISH LAST PERFORMED
int sla = 0;                                                        // USED IN CREATION OF PREAMBLE
int led1 = D7;                                                      // ONBOARD BLUE LED
// END OF DECLARATIONS AND INITIAL PARAMETERS

void setup()
{
  Serial.begin(9600);
  pinMode(led1, OUTPUT);
  // SOLAR SETTINGS //
  //PMIC pmic; //INITIALIZE THE PMIC CLASS TO CALL THE POWER MANAGEMENT FUNCTIONS BELOW
  //pmic.setChargeCurrent(0,0,1,0,0,0); 	//SET CHARGING CURRENT TO 1024MA (512 + 512 OFFSET)
  //pmic.setInputVoltageLimit(4840);     	//SET THE LOWEST INPUT VOLTAGE TO 4.84 VOLTS, FOR 5V SOLAR PANEL
  //pmic.setInputVoltageLimit(5080);     	//SET THE LOWEST INPUT VOLTAGE TO 5.08 VOLTS, FOR 6V SOLAR PANEL
}

ApplicationWatchdog wd(app_watchdog, System.reset);

void loop()
{

  FuelGauge fuel;

  if (a == 0 && fuel.getSoC() > 20)
  {
    Cellular.command("AT+URING=1\r\n");
    for (uint32_t ms = millis(); millis() - ms < 2000; Particle.process())
      a = 1;
  }

  if (fuel.getSoC() < 20) //LOW BATTERY DEEP SLEEP
  {
    if (a != 0)
      Particle.publish("a", "9 - DEVICE SHUTTING DOWN FOR 8 HOURS, BATTERY SoC < 20!", 60, PRIVATE, NO_ACK); // LET SERVER KNOW WE ARE GOING TO SLEEP
    for (uint32_t ms = millis(); millis() - ms < 10000; Particle.process()) //EXTRA TIME BEFORE DEEP SLEEP
      ;
    System.sleep(SLEEP_MODE_DEEP, 14400); // SLEEP 8 HOURS IF SoC < 20
  }

  if (a == 1)
  {
    Particle.publish("a", "9 - DEVICE ONLINE!", 60, PRIVATE, NO_ACK); // LET SERVER KNOW ONLINE!
    digitalWrite(led1, HIGH);
    for (uint32_t ms = millis(); millis() - ms < 2000; Particle.process());
    digitalWrite(led1, LOW);
    a = 2;
  }

  ///\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ START SENSOR VALUE GATHERING \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/

  // SENSOR VALUES GO INTO THE sensor_value[X][0] ARRAY WHERE 'X' IS THE SENSOR NUMBER
  // sensor_value[1][0] = SENSOR 1, SENSOR VALUE
  // sensor_value[1][1-4] = SENSOR 1, HISTORICAL SENSOR VALUES
  // sensor_value[1][5] = SENSOR 1, HISTORICAL SENSOR VALUE AVERAGE OF ALL FOUR

  sensor_value[0][0] = random(441, 450); // PLACES RANDOM VALUES FOR TESTING
  sensor_value[1][0] = random(461, 470); // PLACES RANDOM VALUES FOR TESTING
  sensor_value[2][0] = random(471, 480); // PLACES RANDOM VALUES FOR TESTING
  sensor_value[3][0] = random(481, 490); // PLACES RANDOM VALUES FOR TESTING
  sensor_value[4][0] = random(491, 500); // PLACES RANDOM VALUES FOR TESTING
  sensor_value[5][0] = random(501, 510); // PLACES RANDOM VALUES FOR TESTING

  ///\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ END SENSOR VALUE GATHERING \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/

  ///\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/GATHER, EVENT CHECK, AND PUBLISH \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/

  //-- BEGIN SENSOR EVENT CHECK --
  if (smpls_performed > sensor_event_chk)
  {
    sensor_event_chk = smpls_performed;
    if (sensor_hist_depth == 4)
    {
      sensor_hist_depth = 1;
    }
    else
    {
      sensor_hist_depth++;
    } // CYCLE THROUGH 1 - 4 FOR HISTORY / AVERAGING
    for (w = 0; w < no_of_sensors; w++)
    {
      sensor_value[w][5] = ((sensor_value[w][1] + sensor_value[w][2] + sensor_value[w][3] + sensor_value[w][4]) / 4);
      sensor_value[w][sensor_hist_depth] = sensor_value[w][0];
      if (smpls_performed == 21)
        sensor_value[3][5] += 100; // SIMULATE EVENT * * * *
      if (smpls_performed == 23)
        sensor_value[3][5] += 100; // SIMULATE EVENT * * * *
      if (smpls_performed > 4)
      {
        alrt_ckng_tmp = fabs(sensor_value[w][0] - sensor_value[w][5]);
        if (alrt_ckng_tmp > sensor_alert_thrshld[w])
        {
          Serial.printf("OOS Sensor %d, ", w);
          Serial.printlnf("Diff: %d, Threshold: %d.", alrt_ckng_tmp, sensor_alert_thrshld[w]);
          // DO SOMETHING HERE LIKE ALTER REPORTING TIMES OR SEND ALERT
          if (alrt_state_chng != 2)
            alrt_state_chng = 1; // SIGNIFIES FRESH CHANGE INTO ALERT STATE SO PUBLISHER WILL PUBLISH STORAGE AND THEN START ALERT REPORTING
          current_smpl_intvl = alert_smpl_intvl;
          alert_cycl_mark = smpls_performed + alert_sample_qty;
        }
      }
    }
  }
  if (smpls_performed > alert_cycl_mark)
    current_smpl_intvl = norm_smpl_intvl;
  // CHECK IF WE ARE IN AN ALERT REPORT CONDITION AND SEE IF WE SHOULD COME OUT OF IT
  ///\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ END SENSOR EVENT CHECK \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/

  ///\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ START SAMPLING TIME CHECK \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/
  if (Time.now() > t2)
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
    wd.checkin(); // SOFTWARE WATCHDOG CHECK-IN
    if (alert_cycl_mark == (smpls_performed + 1))
      alrt_state_chng = 3;
  }
  ///\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ END SAMPLING TIME CHECK \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/

  ///\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ START BEDTIME CHECK \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/
  if (sleep == 1 && published_norm1_or_alert2 == 1 && norm_smpl_intvl > 18 && t2 >= (Time.now() + sleep_wait))
  {
    if (enable_wop)
    {
      System.sleep({RI_UC, BTN}, {RISING, FALLING}, (((norm_smpl_intvl - secs_less_intrvl_sleep) - sleep_wait) - 2), SLEEP_NETWORK_STANDBY);
      Cellular.command("AT+URING=0\r\n");
      Cellular.command("AT+URING=1\r\n");
    }
    else
    {
      System.sleep(BTN, FALLING, ((norm_smpl_intvl - secs_less_intrvl_sleep) - sleep_wait), SLEEP_NETWORK_STANDBY);
    }
    published_norm1_or_alert2 = 0;
  }

  ///\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ END BEDTIME CHECK \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/

  ///\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ BEGIN ALERT STATE PUBLISH FLUSH \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/
  if ((alrt_state_chng == 3) || (alrt_state_chng == 1 && alert_cycl_mark > smpls_performed))
  {
    fullpub_temp1[0] = 0;
    fullpublish[0] = 0; // THIS IS NEW STUFF TO PUT PREAMBLE ON DATA
    snprintf(fullpub_temp1, sizeof(fullpub_temp1), "%d%d", alrt_state_chng, no_of_sensors);
    strncat(fullpublish, fullpub_temp1, sizeof(fullpublish) - strlen(fullpublish) - 1);
    for (sla = 0; sla < no_of_sensors; sla++)
    {
      snprintf(fullpub_temp1, sizeof(fullpub_temp1), "%d", sensorlen[sla]);
      strncat(fullpublish, fullpub_temp1, sizeof(fullpublish) - strlen(fullpublish) - 1);
    } // END OF NEW STUFF TO PUT PREAMBLE ON DATA
    for (xa = 0; xa < x; xa++)
    {
      for (y = 0; y < no_of_sensors; y++)
      {
        snprintf(fullpub_temp1, sizeof(fullpub_temp1), "%0*.0f", sensorlen[y], storage[x][y]);
        strncat(fullpublish, fullpub_temp1, sizeof(fullpublish) - strlen(fullpublish) - 1); // better to check the boundaries
      }
    } // BUILT FULLPUBLISH STRING
    x = 0;
    xa = 0;
    if (alrt_state_chng == 1)
      alrt_state_chng = 2;
    if (strlen(fullpublish) != 0)
    {
      FuelGauge fuel; // GET BATTERY INFO
      fullpub_temp1[0] = 0;
      snprintf(fullpub_temp1, sizeof(fullpub_temp1), "E%d%d%d", current_smpl_intvl, ((int)(fuel.getVCell() * 100)), ((int)(fuel.getSoC() * 100))); // ADDING SAMPLE RATE, BATTERY INFO
      strncat(fullpublish, fullpub_temp1, sizeof(fullpublish) - strlen(fullpublish) - 1);
      Serial.println(fullpublish);
      Serial.printf("ASP: The size of published string is %d bytes. \n", strlen(fullpublish));
      if (do_publish)
      {
        Particle.publish("a", fullpublish, 60, PRIVATE, NO_ACK); // PARTICLE.PUBLISH EVENT ! ! ! !
        digitalWrite(led1, HIGH);
        for (uint32_t ms = millis(); millis() - ms < 2000; Particle.process());
        digitalWrite(led1, LOW);
      }
    }
    fullpublish[0] = 0; // CLEAR THE FULLPUBLISH STRING
    pubs_performs++;
    if (alrt_state_chng == 3)
      alrt_state_chng = 0;
  }
  ///\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ END ALERT STATE PUBLISH FLUSH \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/

  ///\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ START SAMPLES TAKEN CHECK AND PUBLISH \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/
  if (x == rnds_to_publish)
  {
    fullpub_temp1[0] = 0;
    fullpublish[0] = 0; // THIS IS NEW STUFF TO PUT PREAMBLE ON DATA
    snprintf(fullpub_temp1, sizeof(fullpub_temp1), "%d%d", alrt_state_chng, no_of_sensors);
    strncat(fullpublish, fullpub_temp1, sizeof(fullpublish) - strlen(fullpublish) - 1);
    for (sla = 0; sla < no_of_sensors; sla++)
    {
      snprintf(fullpub_temp1, sizeof(fullpub_temp1), "%d", sensorlen[sla]);
      strncat(fullpublish, fullpub_temp1, sizeof(fullpublish) - strlen(fullpublish) - 1);
    } // END OF NEW STUFF TO PUT PREAMBLE ON DATA
    for (x = 0; x < rnds_to_publish; x++)
    {
      for (y = 0; y < no_of_sensors; y++)
      {
        snprintf(fullpub_temp1, sizeof(fullpub_temp1), "%0*.0f", sensorlen[y], storage[x][y]);
        strncat(fullpublish, fullpub_temp1, sizeof(fullpublish) - strlen(fullpublish) - 1); // check the boundaries of fullpublish
      }
    } // BUILT FULLPUBLISH STRING
    x = 0;
    if (strlen(fullpublish) != 0)
    {
      FuelGauge fuel; // GET BATTERY INFO
      fullpub_temp1[0] = 0;
      snprintf(fullpub_temp1, sizeof(fullpub_temp1), "E%d%d%d", current_smpl_intvl, ((int)(fuel.getVCell() * 100)), ((int)(fuel.getSoC() * 100))); // ADDING SAMPLE RATE, BATTERY INFO
      strncat(fullpublish, fullpub_temp1, sizeof(fullpublish) - strlen(fullpublish) - 1);
      Serial.println(fullpublish);
      Serial.printf("The size of published string is %d bytes. \n", strlen(fullpublish));
      if (do_publish)
      {
        Particle.publish("a", fullpublish, 60, PRIVATE, NO_ACK); // PARTICLE.PUBLISH EVENT ! ! ! !
        digitalWrite(led1, HIGH);
        for (uint32_t ms = millis(); millis() - ms < 2000; Particle.process());
        digitalWrite(led1, LOW);
      }
    }
    fullpublish[0] = 0; // CLEAR THE FULLPUBLISH STRING
    pubs_performs++;
  }
  ///\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ END SAMPLES TAKEN CHECK AND PUBLISH \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/

  ///\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ END GATHER, EVENT CHECK, AND PUBLISH \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/
}
