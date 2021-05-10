# Importing librarys
from datetime import datetime
from time import sleep, time 
from cotLib import COT          # Custome library
import RPi_algorithm_functions as Fs

# Defining list structures 
test_list =[1,1,1,1,1,1,1]
liste_structure_varmeovn = []

# Defining person dictionary
Person_dictionary = {
    0 : "Felles",
    1 : "Trond",
    2 : "Lauritz",
    3 : "Astrid",
    4 : "Joakim",
    5 : "Vilma",
    6 : "Talha",
    7 : "Solar"
    }

# Variable for person energy consumption calculation
Person_nr_1 = 0
Person_nr_2 = 0
First = "Energy_consumption_week_"

# Current time values 
time_string, current_week, current_date, current_hour = Fs.time_update_function()

# Current responses from sites
resp_1, resp_2, resp_3, resp_4 = Fs.Webside_update(current_date)

# Current solar values
sky_cover, current_sun_height = Fs.Sun_value_update(resp_3, resp_4)

# Defining keys and token values 
token   = "eyJhbGciOiJIUzI1NiJ9.eyJqdGkiOiI1MDkwIn0.jPlTuw_H-Rb41ZDRMFSK_799rn4s4UUJ9ioQAXwbcjE"
token_2 = "eyJhbGciOiJIUzI1NiJ9.eyJqdGkiOiI1MTA1In0.rtwgSROsIox_OXVB5CLaltp6GdoBp0BA0DtS2Fv3fpM"
token_3 = "eyJhbGciOiJIUzI1NiJ9.eyJqdGkiOiI1MDkzIn0.XAZI5Bef9tYoXnOOlOAzWouhiRUkplsBL5r7p3YjRto"

# Defining keys
Key_1 = '20547'
Key_2 = '7748'
key_bathroom = "12707"
key_bathroom_shower = "28076"
key_bathroom_time = "1662"
key_kitchen = "2384"
key_kitchen_ovn = "13261"
key_kitchen_time = "8185"
key_dishes = "20610"
key_heater_room = "3849"
key_heater_distribution = "2866"
key_KWt_sum = "4450"
key_current_sum = "9156"
key_solar_KWt = "15864"

# Defining access values for COT through custom library
cot_verdi_rom = COT(Key_1, token)
cot_liste_rom = COT(Key_2, token)
cot_dishes = COT(key_dishes, token_2)
cot_heater_room = COT(key_heater_room, token_3)
cot_heater_distribution = COT(key_heater_distribution, token_3)
cot_kWt_sum = COT(key_KWt_sum, token)
cot_current_sum = COT(key_current_sum, token)
cot_solar_KWt = COT(key_solar_KWt, token)

# Getting current currentprice for the hour
NOK_pr_kwh = resp_2[time_string]["NOK_per_kWh"]

# Running one update for rom temp values    
Fs.Room_values_function(resp_1, cot_verdi_rom, cot_liste_rom)

# Defining previous variables 
previous_hour = current_hour
previous_current_price = NOK_pr_kwh
previoustime = time()
previoustime_2 = time()
previous_week = current_week
old_Person = 0
old_Person_2 = 0
Person = 0
Person_2 = 0
Time_value = 0

# Getting current database name 
current_database = First + str(previous_week)


# Running main loop 
while(True):
     
    # Updating time string 
    time_string, current_week, current_date, current_hour = Fs.time_update_function()
    
    # If statement to make new week database header 
    if current_week != previous_week:
        
        # Variable update
        previous_week = current_week
        
        """
        3 x Frigde                | 5.5 Watt x 3 = 16.5   på hele tiden
        1 x Heating cables        | 64 Watt 10% forbruk   10% forbruk hele dagen er de på. 
        Light/LED                 | 41,5 Watt             10 timer per dag
        TV                        | 430 Watt              5 timer = 300, 6 dager
        """
        
        # Adding generell use of electrical components in a week
        Fs.Add_value_CSV_average_week(current_database, 16.5,10080, NOK_pr_kwh)
        Fs.Add_value_CSV_average_week(current_database, 64,  10080, NOK_pr_kwh)
        Fs.Add_value_CSV_average_week(current_database, 41.5, 4200, NOK_pr_kwh)
        Fs.Add_value_CSV_average_week(current_database, 430 , 1800, NOK_pr_kwh)
        
        # Updating database value
        current_database = First + str(current_week)
        
        # Adding header in new database 
        Fs.data_base_price(current_database,"Index","Person","SK Kroner","SK KWt")
      
    # If statement to isolate time sone 5 minutes
    if int(time()) >= (int(previoustime)+300):
        
        # Updating previous time value 
        previoustime = time()
        
        # Updating one COT Value 
        Fs.Room_values_function(resp_1, cot_verdi_rom, cot_liste_rom)
        
        # Updating variables 
        resp_1, resp_2, resp_3, resp_4 = Fs.Webside_update(current_date)
        
        # Getting current sky cover and sun height 
        sky_cover, current_sun_height = Fs.Sun_value_update(resp_3, resp_4)
        
        # Getting solar energy value 
        solar_energy = round(Fs.solar_panel_energy(sky_cover,current_sun_height)*(5/60000),2)

        # If statement to ensure that zero solar energy dont get added to the local database 
        if solar_energy > 0: 
            
            # Adding amount to database
            Fs.data_base_price(current_database, 7, Person_dictionary[7] , round((-1*NOK_pr_kwh*solar_energy),2), (-1*solar_energy))
        
        # Defining person variables 
        Person = Fs.get_pers(key_bathroom, token_2, test_list, Person)
        Person_2 = Fs.get_pers(key_kitchen, token_2, test_list, Person_2)


        """-----------------------------------"""
        """   BATHROOM CURRENT CALCULATION    """
        
        # If statements for bathroom keys 
        if old_Person != int(Person):
            
            # IF statement to isolate if person have had a bath
            if (int(Fs.get_station(key_bathroom_shower, token_2, test_list)) == 1):
                
                # Getting time value for time used in bathroom
                Time_value_min = Fs.get_time(key_bathroom_time, token_2, test_list)
               
                """
                1 liter water needs 0.035kW
                8 liter per minute 
                """
                
                # Power consumption value
                Power_temp = (0.035*8*60)
                
                # Redefining old person value
                old_Person = int(Person)
                
                # Adding in current use 
                Fs.Add_value_CSV(current_database, old_Person, Power_temp, Time_value_min, NOK_pr_kwh)

        """-----------------------------------"""
        """   KITCHEN CURRENT CALCULATION     """
        
        # If statements for kitchen keys 
        if old_Person_2 != int(Person_2):
            

            # IF statement to isolate if person have using kitchen stuff 
            if (int(Fs.get_station(key_kitchen_ovn, token_2, test_list)) >= 1):
                 
                # Getting time value for time used in bathroom
                Time_value_min = Fs.get_time(key_kitchen_time, token_2, test_list)
                
                # Getting mode value for what is used in kitchen 
                # 1 is either oven or frying pan, 2 is both
                mode = int(Fs.get_station(key_kitchen_ovn, token_2, test_list))
                
                """
                2200 watt for the oven/frying pan
                """
                
                # Power value 
                Power_temp = 2.2
                
                # Redefining old person value
                old_Person_2 = int(Person_2)
                
                # Adding in current use 
                Fs.Add_value_CSV(current_database, old_Person_2, Power_temp*mode, Time_value_min, NOK_pr_kwh)


        """-----------------------------------"""
        """  DISHWASHER CURRENT CALCULATION   """

        # If statement to see if oppvaskmaskin is used 
        if int(cot_dishes.read()['Value']) == 1:
            
            """
            2000 Watt for the dishwasher
            """
            
            # Defining variables
            Power_temp = 2
            Time_value_min = 120 
            
            # Adding in current use 
            Fs.Add_value_CSV(current_database, 0, Power_temp, Time_value_min, NOK_pr_kwh)

            # Nuller ut signal 
            cot_dishes.write("0")
            
        # Adding in current price and kwt from solar panels to COT    
        Fs.Update_CSV_COT_values(current_database, cot_kWt_sum, cot_current_sum, cot_solar_KWt, solar_energy)
        
        # 
        for i in range(0,7):
            Fs.Update_person_dict(current_database, i)
            
    """-----------------------------------"""
    """   CURRENT HEATER  CALCULATION     """
    
    # If statement to isolate time sone 10 minutes
    if (int(time()) >= int(previoustime_2)+600):
        
        # Update time variable
        previoustime_2 = time()
        
        # Getting structure list from COT signal
        Structure_list = list(map(int, [char for char in str(cot_heater_distribution.read()['Value'])]))
        Value_list = cot_heater_room.read_list(Structure_list)
        
        # For loop to iterate over list 
        for i in range(1,7):
            
            # If statement to only get values above zero
            if int(Value_list[i]) != 0:
                
                """
                1200 Watt for the heaters
                """
                
                # Variable update to adding current price
                Power_temp = 1.20 #KW
                Time_value_min = float(Value_list[i])
                
                # Adding in values to CSV file, where i is the person index
                Fs.Add_value_CSV(current_database, i, Power_temp, Time_value_min, NOK_pr_kwh)
                
                # Zeroing out value place in list
                Value_list[i] = 0
                Structure_list[i] = 1
         
        # Making a structure value variable string
        structure_value = ""
        # Iterating over structure list into a variable
        for i in Structure_list:
            structure_value += str(i)
        
        # Zeroing out signal, with structure value and the valuelist
        cot_heater_room.write_list(Value_list,Structure_list)   
        cot_heater_distribution.write(structure_value)     
        
    
    """-----------------------------------"""
    """     CURRENT PRICE CALCULATION     """
       
    # If statement to update energyprice every hour
    if int(current_hour) == int(previous_hour)+1 or (current_hour == "23" and previous_hour == "00"):
        
        # Updating previous values   
        previous_hour = int(current_hour)
        previous_current_price = NOK_pr_kwh
        
        # Getting current currentprice for the hour
        NOK_pr_kwh = resp_2[time_string]["NOK_per_kWh"]
    
    
    
