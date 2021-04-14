/*
  WriteFromESP32.ino

  This example code that shows how to feed a signal using the circusofthings.com API through its CircusESP32Lib-1.0.0 library for Arduino IDE.

  With this code you can feed a signal taking the values read from a temperature sensor. 
  
  In this case, we assume we have a DHT11 sensor that is controlled by a propietary protocol implemented in its specific library.

  There are no 3rd part libraries to use, beside DHT Sensor Library Built in by Adafruit Version 1.3.0 
  
  Created by Jaume Miralles Isern, April 10, 2019.
  Reviewd by Jaume Miralles Isern, April 10, 2019.
*/ 


#include <CircusESP32Lib.h>
#include <DHT.h>


// ------------------------------------------------
// These are the CircusESP32Lib related declarations
// ------------------------------------------------

char ssid[] = "your_SSID_here"; // Place your wifi SSID here
char password[] =  "your_password_here"; // Place your wifi password here
char token[] = "your_token_here"; // Place your token, find it in 'account' at Circus. It will identify you.
char server[] = "www.circusofthings.com";
char temperature_key[] = "31898";  // Place the Key of the signal you created at Circus Of Things for the Temperature
char humidity_key[] = "6066";  // Place the Key of the signal you created at Circus Of Things for the Humidity
CircusESP32Lib circusESP32(server,ssid,password); // The object representing an ESP32 to whom you can order to Write or Read

// ------------------------------------------------
// These are the Temperature Example related declarations
// ------------------------------------------------

#define DHTPIN 27      // digital of your ESP32 connected to DHT11
#define DHTTYPE DHT11 // exact model of temperature sensor DHT 11 for the general library
DHT dht(DHTPIN, DHTTYPE); // The object representing your DHT11 sensor


 
void setup() {
    Serial.begin(115200); // Remember to match this value with the baud rate in your console
    dht.begin(); // Set the DHT11 ready
    circusESP32.begin(); // Let the Circus object set up itself for an SSL/Secure connection
}
 
void loop() { // Now that all is set up, let's begin with the tasks

    delay(10000);
    // Let the library get the Temperature that DHT11 probe is measuring.
    float t = dht.readTemperature(); 
    if (isnan(t))
        t=-1; // if so, check the connection of your DHT11 sensor... something is disconnected ;-)
    float h = dht.readHumidity();
    if (isnan(h))
        h=-1; // if so, check the connection of your DHT11 sensor... something is disconnected ;-)
    // Show values, just for debuging
    Serial.println(""); Serial.print("Temperature: "); Serial.println(t); Serial.print("Humidity: "); Serial.println(h);

    // Report the values gathered by the sensor to the Circus
    circusESP32.write(temperature_key,t,token); // Report the temperature measured to Circus.
    circusESP32.write(humidity_key,h,token); // Report the humidity measured to Circus.
}
