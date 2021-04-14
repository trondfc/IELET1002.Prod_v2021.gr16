/**
 * @file number_to_array.cpp
 * @author Trond F. Christiansen (trondfc@stud.ntnu.no)
 * @brief Example of how to use the array_conversion library
 * @version 0.1
 * @date 2021-03-25
 * 
 * This sketch uses the .i_to_array function in the array_conversion library in order to convert from
 * a integer to an array
 * The integer can be up to 18 siffers
 * The returned array are 10 cells
 */

#include <Arduino.h>                                                // include arduino functions
#include "array_conversion.h"                                       // include array_conversion functions

CONVERT Convert(false);                                             // Start a CONVERT class (debuging = false)

uint64_t test_number = 12345678901;                                 // Creating the input number (can be up to 18 siffers)
char char_lenght[10] = {1,2,2,1,1,4};                               // Creating an array of siffer positions for output array
                                                                        // First cell have 1 siffer, second have 2, third have 2 ...
int array_lenght = 6;                                               // Lenght of the array

void setup(){
    Serial.begin(115200);                                           // Start serial comunication 
    delay(2000);                                                    // Wait for 2s
    Serial.println("converting a number to an array of numbers:");  // Print info line
    Serial.print("input number is : ");                             // Print info text
    Serial.println(test_number);                                    // Print input number
    int *r;                                                         // Create an int array for the return
    r = Convert.i_to_array(test_number,char_lenght);                // Run to array function using the test number and the posittion array
                                                                        // Returns a 10 cell array with numbers, unused cells are 0
    Serial.print("  cell nr. 1 = ");                                // print info text
    Serial.println(r[1]);                                           // Print cell content
    r[1] = random(0,100);                                           // Asign random int to cell 1
    Serial.println("reasigning number");                            // Print info text
    Serial.print("  cell nr. 1 = ");                                // print info text
    Serial.println(r[1]);                                           // Print cell content
    test_number = Convert.to_int(r,array_lenght);                   // Run to int function using returned array and the lenght of the array
                                                                        // Returns a uint64_t number (2^64 bit). can be up to 18 siffers
    Serial.print("new number = ");                                  // Print info text
    Serial.println(test_number);                                    // Print the new number 
}

void loop(){}