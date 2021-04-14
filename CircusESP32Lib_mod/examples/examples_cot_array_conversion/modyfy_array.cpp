/**
 * @file read_array.cpp
 * @author Trond F. Christiansen (trondfc@stud.ntnu.no)
 * @brief example of write_array and read_array functions added to cot lib
 * @version 0.1
 * @date 2021-03-25
 * 
 * This sketch is an example of how to use the wwrite_array and read_array functions added to cot lib toghether
 * The example takes reads ai integer form cot and devides it up acording to the char_lenght array. then it changes 
 * the number in the first array cell before turning it to an integer again and uploads it to cot
 */
#include <Arduino.h>                                                        // Include arduino lib
#include <CircusESP32Lib.h>                                                 // Include cot lib

char ssid[] = "Wifi_name";                                                  // WiFi name
char psk[] = "Wifi_passwd";                                                 // WiFi password
char token[] = "COT_token";                                                 // circus of things user key
char server[] = "www.circusofthings.com";                                   // circus of things server addres
char cot_signal_key[] = "cot_signal_key";                                   // circus of things signal key

uint64_t test_number = 1234567890123456;                                    // Create a number to use during testing, can be up to 16 characters long
char char_lenght[10] = {2,1,1,4,2,1,3,2};                                   // Define caharacter pr cell, up to 10 cells
int array_lenght = 8;                                                       // Define number of cells in use, up to 10 cells

CircusESP32Lib circusESP32(server, ssid, psk);                              // Start cot instance

void setup() {
    circusESP32.begin();                                                    // Run begin function
    Serial.begin(115200);                                                   // Start serial comunication
    delay(2000);                                                            // Wait for 2s
    Serial.println("Starting example");                                     // Print info line
    circusESP32.write(cot_signal_key,test_number,token);                    // Write test number to cot 
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
    Serial.print("  cell 0 = ");                                            // Print info text
    Serial.println(r[0]);                                                   // Print cell 0
    r[0] = random(10,99);                                                   // Reasign number in cell 0
    Serial.println("reasigning number");                                    // Print info line 
    Serial.print("cell 0 = ");                                              // Print info text
    Serial.println(r[0]);                                                   // Print cell 0
    circusESP32.write_array(cot_signal_key,r,array_lenght,token,char_lenght);           // Run write_array function with cot key, array, array lenght and cot token
}