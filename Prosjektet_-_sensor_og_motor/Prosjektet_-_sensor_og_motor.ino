#include <analogWrite.h>
#include <ESP32_Servo.h>
#include <CircusESP32Lib.h>

int id = 1;

// variabler fra cot for ønsket temperatur.
int *array_temp;                      // Array som leser ønskede temperaturer.
int *override_array;                  // Array som leser override verdiene.
int state_in;                         // Verdi for om vedkommende er hjemme, sendt fra inngangsparti.
int *array_ver;                       // Array som leser ulike tilstander ute, sendt fra Rapi.
int *array_del;                       // Array for signal som anngir hvordan array_ver skal deles opp.

// Variabler for override fra cot.
int override_temp;                    // Styrer ovn av/på.
int override_fan;                     // Stryrer takvifte 0-3.
int override_wind;                    // Åpner lukker vindu 0-3.
int override_state;                   // Styringssiffer 1/2. 2 betyr automatsk styring. 1 betyr at overide kjøres med gitte verdier.

// Variaber for vær.
int wind_1;                           // Variabel for vindstyrke.
int wind_2;                           // Variabel for vindretning.
int rain;                             // Variabel for om det regner, 1/0.
int temp_out;                         // Variabel for utetemperatur.

int window_state = 0;                 
int fan_state = 0;                    

// Tids variabler.
unsigned long time_1 = millis();      // Fire ulike timere som sjekkes underveis i koden.
unsigned long time_2 = millis();
unsigned long time_3 = millis();
unsigned long time_4 = millis();
long interval_manuel = 5000;          // Tid mellom hver gang signalet for manuel styring leses.
long interval_temp_cot = 5000;        // Tid mellom hver gang temperatur signalet skal leses.
long interval_temp_1 = 1000;          // Tid mellom hver gang sensorverdier skal leses.
long interval_temp_2 = 60000;         // Tid mellom hver gang snitt fra sensormålinger skal regnes ut.
unsigned long log_time;               // Variabel som lagrer tid ovn har stått på.
unsigned long time_tot;               // Variabel som lagrer total tid ovn har stått på.

// Vriabler for utregning av pris for ovn.
int ovn;
int ovn_cot;
int ovn_state;

// Sensor pins,
const int temp_sensor = 35;
const int light_sensor = 34;
int temp_ideal;                       // Ønsket temp fra cot.
float temp = 0;                       // Variabel som samler verdi fra sensor.
float temp_tot = 19;                  // Variabel for snittet. Startverdi 19 frem til første snitt er regnet ut.
int light_limit = 100;                // Grenseverdi for når det er nok lys og anntas dag.
float light = 0;                      // Variabel som samler verdier fra photoresistor.
float light_tot = 150;                // Variabel for snittet til light. Startverdi 150 frem til første snitt er regnet ut.
int counter = 0;                      // Teller som brukes for å regne snitt.

// Motor pins.
const int IN1 = 32;                   // Trenger kun en IN pin som settes høy. EN endres på ESP nr. 2.

// Led pins.
const int blue_led = 25;              // LED som lyser når manuell styring av temperatur kjøres.

// Cot.
char ssid[] = "Vilma andrea sin iPhone";
char password[] = "hei456hei";
char token_1[] = "eyJhbGciOiJIUzI1NiJ9.eyJqdGkiOiI1MDkzIn0.XAZI5Bef9tYoXnOOlOAzWouhiRUkplsBL5r7p3YjRto";      // Token for signaler som sendes fra denne enheten.
char token_2[] = "eyJhbGciOiJIUzI1NiJ9.eyJqdGkiOiI1MDkwIn0.jPlTuw_H-Rb41ZDRMFSK_799rn4s4UUJ9ioQAXwbcjE";      // Token for signaler som sendes fra RPi.
char token_3[] = "eyJhbGciOiJIUzI1NiJ9.eyJqdGkiOiI0OTEwIn0.5SXKcTgsyeYGALKiUW68wQ-UA7wARGUy4dhXrjcXE-g";      // Token for signaler som sendes fra inngangsparti.
char server[] = "www.circusofthings.com";

char key_temp[] = "4891";                        // Key for temperatur signal.
char key_override[] = "8417";                    // Key for override signal.
char key_inout[] = "3410";                       // Key for inne / ute signal.
char key_ver[] = "20547";                        // Key for å lese av værdata fra RPi.
char key_del[] = "7748";                         // Key for styrings array som bestemmer hvordan array_ver skal deles.
char key_ovn_forbruk[] = "3849";                 // Key for array som håndterer tid varmeovn har vært på for per beboer.
char key_ovn_styring[] = "2866";                 // Key for styrings arry som bestenmmer hvordan key_ovn_styring skal deles opp.
char key_motor[] = "28671";                      // Key for verdi som skal skrives til motor.
char key_servo[] = "12005";                      // Key for verdi som skal skrives til servo.

CircusESP32Lib circusESP32(server, ssid, password);
char char_four[4] = {1, 1, 1, 1};                   // Standard char som brukes for å hente array signaler med 4 elementer der hver er 1 siffer.
char char_temp[4] = {1, 2, 2, 2};                   // Char for å hente temp_array
char char_ver[4] = {1, 1, 1, 1};                    // Dynamisk char som endres etter fordelingen angitt i array_del.
char char_ovn_styring[7] = {1, 1, 1, 1, 1, 1, 1};   // Statisk char som henter array som tilsvarer char ovn.
char char_ovn[7] = {1, 1, 1, 1, 1, 1, 1};           // Dynamisk char som endres etter fordelingen angitt i ovn_styring array.


// Funksjon som leser fra override signal og lagrer verdiene.
void read_manuel() {
  override_array = circusESP32.read_array(char_four, key_override, token_1);
  override_state = override_array[0];

  if (override_state == 1) {            // Sjekker om manuel styring skal kjøres.
    override_fan = override_array[1];   // Lagrer verdi for styring av takvifte 0-3.
    ovn = override_array[2];            // Lagrer verdi for ovn 1/0.
    override_wind = override_array[3];  // Lagrer verdi for åpning av vindu 0-3.

    analogWrite(blue_led, 2000);        // Skrur på led for å vise at manuell styring er på.
  }
  else if (override_state == 2) {
    analogWrite(blue_led, 0);           // Om manuell styring ikke skal kjøres skrus led av, og ingen verdier lagres.
  }
}

// Funksjon som leser fra temp_signal. Og bestemmer hvilken verdi som skal lagres.
int read_temp_cot() {
  array_temp = circusESP32.read_array(char_temp, key_temp, token_1);
  state_in = circusESP32.read(key_inout, token_3);

  if (state_in == 0) {                      // Sjekker først om beber ikke er hjemme.
    return array_temp[3];                   // Returnerer element 3 i arrayet.
  }
  else if (light_tot >= 100) {              // Sjekker om mengde lys tilsvarer dag.
    return array_temp[2];
  }
  else if (light_tot < 100) {               // Sjekker om mengde lys tilsvarer natt.
    return array_temp[1];
  }
}

// Funksjon som leser og lagrer verdiene fra sensorene hvert 30 sekund og regner snittet etter 10 min.
void read_sensors() {
  if ((millis() - time_3) >= interval_temp_1) {                         // Hvert 30 sekund leses sensorene.
    time_3 = millis();
    temp += (((analogRead(temp_sensor) * (3300 / 1024)) - 50) / 100);   // Lesing av temp sensor med kalibrering.
    light += analogRead(light_sensor);
    counter += 1;
    
    if ((millis() - time_4) >= interval_temp_2) {                       // Hvert 10 minutt regnes snittet av temperaturen og photoresistor.
      time_4 = millis();
      temp_tot = temp / counter;                                        // Tot verdiene lagres mens verdiene som summerer nullstilles. 
      light_tot = light / counter;
      temp = 0;
      light = 0;
      counter = 0;
    }
  }
}

// Funksjon for styring av motor. 
void fan_controll(int setting) {
  int value;
  switch (setting) {        // Switch case som sjekker setting og setter value til tilsvarende verdi.
    case 0:
      value = 0;
      break;
    case 1:
      value = 60;
      break;
    case 2:
      value = 80;
      break;
    case 3:
      value = 100;
      break;
  }
  circusESP32.write(key_motor, value, token_1);         // Skriver value til cot slik at esp nr. 2 kan hente verdi og kjøre motor.
}

// Funksjon for styring av servo.
void window_controll(int degree) {
  int value;
  switch (degree) {           // Switch case som sjekker degree og setter valu til tilsvarende grader.
    case 0:
      value = 0;
      break;
    case 1:
      value = 20;
      break;
    case 2:
      value = 50;
      break;
    case 3:
      value = 80;
      break;
  }
  circusESP32.write(key_servo, value, token_1);         // Skriver value til cot slik at esp nr. 2 kan hente verdi og skrivet til servo.
}

// Funksjon for loggign av ovn over tid.
int ovn_start() {
  if (log_time == 0) {    // Sjekker at ovn ikke er skrudd på fra før. 
    ovn = 1;              // Skrur ovn "på".
    log_time = millis();  // Starter tidtelling.
  }
}

// Funksjon som finner antall siffere i et tall.
int num_digits(int number) {
  int digits = 0;
  if (number <= 0) {
    digits = 1;
  }
  while (number) {
    number /= 10;
    digits++;
  }
  return digits;
}

// Funksjon som sjekker om ovnene er blitt skrudd av, og gjør om verdiene og sender til cot.
int ovn_stop() {
  if ((log_time > 0) && (ovn == 0)) {       // Sjekker om ovn er skrud av og om log_time ikke er behandlet.
    int *array_char_ovn;
    array_char_ovn = circusESP32.read_array(char_ovn_styring, key_ovn_styring, token_1);

    for (int i = 0; i < 7; i++) {      // Sjekker hvert element i array_char_ovn og erstatter verdien char_ovn med verdien.
      switch (array_char_ovn[i]) {
        case 1:
          char_ovn[i] = char(1);
          break;
        case 2:
          char_ovn[i] = char(2);
          break;
        case 3:
          char_ovn[i] = char(3);
          break;
      }
    }

    int *array_ovn = circusESP32.read_array(char_ovn, key_ovn_forbruk, token_1);    // Henter array med tidligere forbruk.

    time_tot = int(millis() - log_time) / 1000;                                     // Regner ut hvor lang tid ovnen har stått på i minutter
    ovn_state = array_ovn[id];                                                      // Sjekker om det legger verdi for beboer i arrayet.

    log_time = 0;
    time_tot += ovn_state;                                                          // Leger sammen tideligere forbruk med nytt forbruk. 

    array_ovn[id] = time_tot;                                                       // Erstatter gammelt forbruk i array_ovn med ny verdi, og skriver til cot.
    circusESP32.write_array(key_ovn_forbruk, array_ovn, 7, token_1);

    array_char_ovn = circusESP32.read_array(char_ovn_styring, key_ovn_styring, token_1);      // Henter array_char_ovn igjen ersater element med nytt antall siffer og sender tilbake til cot.
    array_char_ovn[id] = num_digits(time_tot);
    circusESP32.write_array(key_ovn_styring, array_char_ovn, 7, token_1);

  }
}

// Funksjon for å sjekke værdata.
int read_weather() {
  int *array_ver;         // Array forværdata.
  int *array_del;         // Array for oppdeling av værdata.
  array_del = circusESP32.read_array(char_four, key_del, token_2);
  for (int i = 0; i < 4; i++) {       // Henter array_del og erstatter hvert element i char ver, med tilsvarende element i array_del.
    switch (array_ver[i]) {
      case 1:
        char_ver[i] = 1;
        break;
      case 2:
        char_ver[i] = 2;
        break;
      case 3:
        char_ver[i] = 3;
        break;
    }
  }
  int window;

  array_ver = circusESP32.read_array(char_ver, key_ver, token_2);
  temp_out = array_ver[1];          // Element 1 = utetemperatur.
  wind_1 = array_ver[2];            // Element 2 = vindstyrke.
  wind_2 = array_ver[3];            // Element 3 = vindretning.
  rain = array_ver[4];              // Element 4 = om det regner.

  // Ulike kriterier basert på vædataene som bestemmer hvilken instilling som returneres.
  if ((temp_out >= temp_ideal) ||                  
      ((wind_2 >= 70) && (wind_2 <= 110)) ||
      ((wind_1 >= 12))) {
    window = 0;
  }
  else if ((rain == 1) ||
           ((wind_2 >= 50) && (wind_2 <= 130) && (wind_1 >= 7))) {
    window = 1;
  }
  else if (wind_1 <= 5 ) {
    window = 2;
  }
  else {
    window = 3;
  }
  return window;
}


void setup() {
  pinMode(temp_sensor, INPUT);
  pinMode(light_sensor, INPUT);

  pinMode(blue_led, OUTPUT);

  pinMode(IN1, OUTPUT);

  digitalWrite(IN1, HIGH);

  Serial.begin(115200);
  circusESP32.begin();

  read_temp_cot();        // Leser av temp signal.
  read_manuel();          // Leser av manuel signal.
}

void loop() {
  read_sensors();                                       // Sjekker sensorer.

  if (millis() > time_1 + interval_manuel) {            // Sjekker override hvert 5 minutt.
    time_1 = millis();
    read_manuel();
  }
  if (millis() > time_2 + interval_temp_cot) {          // Sjekker temperaturverdier hvert 10 minutt.
    time_2 = millis();
    temp_ideal = read_temp_cot();
  }

  // If override == false
  if (override_state == 2) {                            // Om override = 2 kjøres automatisk temperatur styring.

    // Finner ut om
    if (temp_tot < temp_ideal) {                        // Om det er kaldere enn ønsket skrus ovn på og vifte og vindu av.
      window_controll(0);
      fan_controll(0);
      ovn_start();
    }
    else if (temp_tot > temp_ideal) {                   // Om det er varmere enn ønsket skrus ovn av. 
      ovn = 0;
      window_state = read_weather();                    // Værdata sjekkes for å bestemme om det er gunstig å åpne vindu fremor å kjøre takvifte.
      if (window_state >= 1) {                          // Om window state er større enn 0 åpnes vindu deretter og vifta skru av.
        window_controll(window_state);
        fan_controll(0);
      }
      else {
        fan_state = map(temp_tot, temp_ideal, 40, 0, 3);    // Bruker map for å bestmme hvor for viften skal gå i forheld til målt temperatur og ønsket temperatur.;
        fan_controll(fan_state);                        // Setter vifte på og lukker vindu.
        window_controll(0);
      }
    }
    else {                                              // Om det skulle skje at temperaturen stemmer, skrus alt av.
    window_controll(0);
    fan_controll(0);
    ovn_stop();
    }
  }

  // If override == true
  else if (override_state == 1) {         // Manuell styring er på og verdier hentet fra read_manuel() brukes i de ulike funksjonene.
    fan_controll(override_fan);
    window_controll(override_wind);
    if (ovn >= 1) {                       // Om verdien til ovn er større enn 0 startes ovnen.
      ovn_start();
    }
  }

  ovn_stop();      //Sjekker om ovnene er blitt skrudd av og sender verdi til cot.
}
