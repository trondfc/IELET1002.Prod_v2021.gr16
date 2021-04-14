/**
 * @file array_conversion.h
 * @author Trond F. Christiansen (trondfc@stud.ntnu.no)
 * @brief simple function library for conversion between array and int
 * @version 0.1
 * @date 2021-03-23
 */

#ifndef ARRAY_CONVERSION_H
#define ARRAY_CONVERSION_H

#include <Arduino.h>

/**
 * @brief define the class for the array conversion library
 */
class CONVERT {
public:
    CONVERT(bool debug);
    int * s_to_array(String string, char number_array[]);
    int * i_to_array(uint64_t number, char number_array[]);
    uint64_t to_int(int array[], int size_of_array, char size_of_index[]);
private:
    String uint64_to_string(uint64_t number);
    int num_digits(int number);
    bool _debug;
};

#endif
