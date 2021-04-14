/**
 * @file array_conversion.cpp
 * @author Trond F. Christiansen (trondfc@stud.ntnu.no)
 * @brief simple function library for conversion between array and int
 * @version 0.1
 * @date 2021-03-23
 */

#include "array_conversion.h"

/**
 * @brief Construct a new :CONVERT object
 * 
 * @param debug: bool
 */
CONVERT::CONVERT(bool debug = false){
    _debug = debug; // Save value localy
}

/**
 * @brief local function to convert a uint64_t number to a string
 * 
 * @param number: unit64_t
 * @return String: string version of number
 */
String CONVERT::uint64_to_string(uint64_t number){
    String result = "";
    uint8_t base = 10;

    do{
        char c = number % base;
        number /= base;

        if(c < 10){c +='0';}
        else{c += 'A' - 10;}
        result = c+ result;
    }while(number);
    return result;
}

/**
 * @brief function to convert a uint64_t number to an array
 *          e.g. (123456,{1,2,3}) -> {1,23,456}
 * @param number: uint64_t. number to be converted
 * @param number_array: char array. number of siffers pr array cell
 * @return int*: array with numbers
 */
int * CONVERT::i_to_array(uint64_t number, char number_array[]){
    String str = uint64_to_string(number);
    int *return_array;
    return_array = s_to_array(str, number_array);
    return return_array;
}

/**
 * @brief function to convert a string of numbers to an array
 *          e.g. ("123456",{1,2,3})
 * @param string: String. String of numbers to be converted
 * @param number_array: char array. number of siffers pr array cell
 * @return int*: array with numbers  -> {1,23,456}
 */
int * CONVERT::s_to_array(String string, char number_array[]){
    static int return_array[10];
    memset(return_array,0,sizeof(return_array));
    int char_position = 0;
    if(_debug){
        Serial.println("Passed inn :");
        Serial.println(string);
        for(int i=0;i<10;i++){Serial.print(int(number_array[i]));Serial.print(" ");}
        Serial.println("");
    }
    for(int i=0;i<10;i++){
        for(int n = int(number_array[i]);n>0;n--){
        return_array[i] += (int(string[char_position]-'0')*(int(pow(10,n-1))));
        if(_debug){
            Serial.print("  ");Serial.print(string[char_position]); Serial.print(" * ");Serial.print(int(pow(10,(n-1))));Serial.print(" + ");
        }
        char_position++;
        }
    if(_debug){
    Serial.println("");
    }
    }
    return return_array;
}

/**
 * @brief function to convert an array of numbers to a number
 * 
 * @param array: int array. List of numbers to be converted
 * @param size_of_array: int. number of cells in the list
 * @return uint64_t: uint64_t number created from list
 */
uint64_t CONVERT::to_int(int array[], int size_of_array, char size_of_index[]){
    String number;
    for(int i = 0; i < size_of_array; i++){
        int number_lenght = num_digits(array[i]);
        while(number_lenght < size_of_index[i]){
            number.concat(0);
            number_lenght++;
        }
    if(_debug){
        Serial.print(i); Serial.print(" = "); Serial.println(array[i]);
    }
    number.concat(array[i]);
    }
    if(_debug){
        Serial.print("  "); Serial.println(number);
    }
    int len = number.length();
    uint64_t  result = 0;
    for(int i = 0; i < len; i++){
        result = result*10 +(int(number[i]-'0'));
    }
    return result;
}

/**
 * @brief function to count number of digits in the number
 * 
 * @param number 
 * @return int number of digitis
 */
int CONVERT::num_digits(int number){
    int digits = 0;
    if(number <= 0 ){digits = 1;}
    while(number){
        number /= 10;
        digits++;
    }
    return digits;
}