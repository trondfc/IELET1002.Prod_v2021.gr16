/**
 * @file read_array.cpp
 * @author Trond F. Christiansen (trondfc@stud.ntnu.no)
 * @brief example of write_array function added to cot lib
 * @version 0.1
 * @date 2021-03-25
 * 
 * This sketch is an example of how to use the write_array function added to cot lib
 * The example takes an array and combines the cells to a integer befor sending it to cot
 */
#include <Arduino.h>                                                        // Include arduino lib
#include <CircusESP32Lib.h>                                                 // Include cot lib

char ssid[] = "Wifi_name";                                                  // WiFi name
char psk[] = "Wifi_passwd";                                                 // WiFi password
char token[] = "COT_token";                                                 // circus of things user key
char server[] = "www.circusofthings.com";                                   // circus of things server addres
char cot_signal_key[] = "cot_signal_key";                                   // circus of things signal key 

int test_array[] = {62,21,22,7,5,1200};                                     // Creating the array (can be up to 10 cells or 16 siffers)
int array_lenght = 6;                                                       // Define number of cells in use, up to 10 cellsy

CircusESP32Lib circusESP32(server, ssid, psk);                              // Start cot instance

void setup() {
    Serial.begin(115200);                                                   // Start serial comunication
    Serial.println("Starting example");                                     // Print info line
    circusESP32.begin();                                                    // Run begin function
    delay(2000);                                                            // Wait for 2s
    Serial.println("\nArray is :");                                         // Print info line
    for(int i = 0; i < array_lenght; i++){                                  // Loop trough the array
        Serial.print("cell nr. ");                                          // Print info text
        Serial.print(i);                                                    // Print cell number
        Serial.print(" = ");                                                // Print info text
        Serial.println(test_array[i]);                                      // Print cell content
    } 
    Serial.print("\n\ncell 0 = ");                                          // print info text
    Serial.println(test_array[0]);                                          // Print cell 0
    test_array[1] = random(10,99);                                          // Reasign number in cell 0
    Serial.println("reasigning number");                                    // Print info line 
    Serial.print("cell 0 = ");                                              // Print info text
    Serial.println(test_array[1]);                                          // Print cell 0
    circusESP32.write_array(cot_signal_key,test_array,array_lenght,token);  // Run write_array function with cot key, array, array lenght and cot token
}

void loop() {
}