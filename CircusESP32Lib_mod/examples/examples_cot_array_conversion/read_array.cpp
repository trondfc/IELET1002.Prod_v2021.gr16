/**
 * @file read_array.cpp
 * @author Trond F. Christiansen (trondfc@stud.ntnu.no)
 * @brief example of read_array function added to cot lib
 * @version 0.1
 * @date 2021-03-25
 * 
 * This sketch is an example of how to use the red_array function added to cot lib
 * The example takes reads info from cot and devides it up in an array consistinc of two
 * characters eatch
 */
#include <Arduino.h>                                                        // Include arduino lib
#include <CircusESP32Lib.h>                                                 // Include cot lib

char ssid[] = "Wifi_name";                                                  // WiFi name
char psk[] = "Wifi_passwd";                                                 // WiFi password
char token[] = "COT_token";                                                 // circus of things user key
char server[] = "www.circusofthings.com";                                   // circus of things server addres
char cot_signal_key[] = "cot_signal_key";                                   // circus of things signal key                                                                           // circus of things server addres

CircusESP32Lib circusESP32(server, ssid, psk);                              // Start cot instance
char char_lenght[10] = {2,2,2,2,2,2,2,2};                                   // Define caharacter pr cell, up to 10 cells

void setup() {
    circusESP32.begin();                                                    // Run begin function
    Serial.begin(115200);                                                   // Start serial comunication
    delay(2000);                                                            // Wait for 2s
}

void loop() {
    int *r = circusESP32.read_array(char_lenght,cot_signal_key,token);      // Run read_array function. neads array of characters pr cell, cot key and cot token
                                                                                // Returns a 10 cell array, unused cells are 0
    Serial.println("_____State_____");                                      // Print info line
    for(int i = 0; i < 10; i++){                                            // Loop trough array
            Serial.print("cell nr. ");                                      // Print info text
            Serial.print(i);                                                // Print cell number
            Serial.print(" = ");                                            // Print info text
            Serial.println(r[i]);                                           // Print cell content
    }
}