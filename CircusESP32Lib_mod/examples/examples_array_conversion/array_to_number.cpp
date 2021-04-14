/**
 * @file array_to_number.cpp
 * @author Trond F. Christiansen (trondfc@stud.ntnu.no)
 * @brief Example of how to use the array_conversion library
 * @version 0.1
 * @date 2021-03-25
 * 
 * This sketch uses the .to_int function in the array_conversion library in order to convert from an array
 * to an 64bit integer
 * The array can be up to 10 cells or 18 siffers
 */

#include <Arduino.h>                                                // include arduino functions
#include "array_conversion.h"                                       // include array_conversion functions

CONVERT Convert(false);                                             // Start a CONVERT class (debuging = false)

int test_array[] = {6,21,22,7,5,1200};                              // Creating the array (can be up to 10 cells or 18 siffers)
int array_lenght = 6;                                               // Lenght of the array


void setup(){
    Serial.begin(115200);                                           // Start swerial comunication 
    delay(2000);                                                    // Wait for 2s
    Serial.println("Converting an array of numbers to a number:");  // Print info line
    for(int i=0; i < array_lenght; i++){                            // Loop trough the array
        Serial.print("  array cell ");                              // Print info text
        Serial.print(i);                                            // Print cell number
        Serial.print(" is ");                                       // Print info text
        Serial.println(test_array[i]);                              // Print cell content
    }
    uint64_t out = Convert.to_int(test_array,array_lenght);         // Run the function using an array an the length of the array
                                                                        // Returns a uint64_t number (2^64 bit). can be up to 18 siffers
    Serial.print("output "); Serial.println(out);                   // Print the output 
}
void loop(){}