# -*- coding: utf-8 -*-
"""
@file CotLib.py
@author Trond F. Christiansen (trondfc@stud.ntnu.no)
@brief function library managing writing/reading values and arrays from cot
@version 0.1
@date 2021-04-4
"""
import requests                                                                 # Import requests library
import json                                                                     # Import json library


"""
@brief class managing writing/reading values and arrays from cot
"""
class COT:
    """
    @brief initialasion of cot class
    """
    def __init__(self,key,token):
        self.key = key
        self.token = token
        
    """
    @brief function to read from cot
    
    @return (json) value from cot
    """
    def read(self):
        response = requests.get('https://circusofthings.com/ReadValue',
                                params = {'Key':self.key,'Token':self.token})
        response = json.loads(response.content)
        return response
    
    """
    @brief function to write to cot
    
    @param value: (int) value to write
    @return (json) response from cot
    """
    def write(self,value):
        data ={'Key':self.key,'Value':value,'Token':self.token}
        response = requests.put('https://circusofthings.com/WriteValue',
                        data = json.dumps(data),
                        headers = {'Content-Type':'application/json'})
        return response
        
    """
    @brief function to count number of digits
    
    @param number: (int) number to count
    @return (int) number of digits
    """
    def lenght(self,number):
        digits = len(str(number))
        return digits
    
    """
    @brief function to convert from list to number
    
    @param list_in: (list) list to convert
    @param list_position: (list) number of digits in eatch list position
    @return (int) list as a number
    """
    def to_str(self,list_in,list_position):
        out = ""
        for i in range(len(list_in)):
            number_lenght = self.lenght(list_in[i])
            while number_lenght < list_position[i]:
                out+='0'
                number_lenght += 1
            out+=str(list_in[i])
        return out

    """
    @brief function to convert from number to list
    
    @param number: (int) number to convert
    @param list_position: (list) number of digits in eatch list position
    @return (list) List of numbers
    """
    def to_list(self,number,list_position):
        counter = 0
        new_list = []
            
        string = str(number)
        for i in range(len(list_position)):
            number_out = ""
            number = list_position[i]
            for n in range(number):
                number_out += string[counter]
                counter += 1
            new_list.append(number_out)
        return new_list
    
    """
    @brief function to write list to cot
    
    @param list_in: (list) list to write
    @param list_position: (list) number of digits in eatch list position
    @return (json) response from cot
    """
    def write_list(self,list_in,list_position):
        value = self.to_str(list_in,list_position)
        response = self.write(value)
        return response
    
    """
    @brief function to read list from cot
    
    @param list_position: (list) number of digits in eatch list position
    @return (list) List of numbers
    """
    def read_list(self,list_position):
        value = self.read()["Value"]
        out = self.to_list(value,list_position)
        return out
