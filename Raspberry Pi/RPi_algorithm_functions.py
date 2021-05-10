# Importing librarys
import requests
import json
from datetime import datetime
from cotLib import COT               # Custome library
import math 
import pandas as pd

# Token for people dictionary
token_People_dictionary = "eyJhbGciOiJIUzI1NiJ9.eyJqdGkiOiI1MDUwIn0.kbJUuhELenracPimjgI8LYLrpIMOKGOcUHqS_-ok5z8"

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
# key dictionary, making it easy to iterate over
Key_dictionary_people = {
    0 : '32320',
    1 : '29465',
    2 : '32273',
    3 :  '4579',
    4 :  '2377',
    5 :  '3018',
    6 :  '8122'
    }

"""
Functions defining spesified for RPi algorithm file. 

First three functions is used for calculating person, condition and time usage, 
before the person gets removed from the queue list system.
"""

# Function to get people index from COT signal
def get_pers(key, token, char_list, old_pers):
    cot_ = COT(key, token)
    List = cot_.read_list(char_list)
    
    # If statement to either give new person or old, if it hasent changed
    if old_pers != List[1]:
        return List[1]
    else:
        return old_pers

# Get condition of the first person 
def get_station(key, token, char_list):
    cot_ = COT(key, token)
    List = cot_.read_list(char_list)
    return List[1]

# Get time used by first person
def get_time(key, token, char_list):
    cot_ = COT(key, token)
    List = cot_.read_list(char_list)
    return int(List[1])*5

# Function to write to database with ID, and an dynamic lokal database
def data_base_price(energy_database , ID, Person, energi_value, Kwh_price):
    
    with open(str("PowerConsumption/" + energy_database + ".csv"), "a") as myfile:
        myfile.write(str(ID))
        myfile.write(";")
        myfile.write(str(Person))
        myfile.write(";")
        myfile.write(str(energi_value))
        myfile.write(";")
        myfile.write(str(Kwh_price))
        myfile.write("\n")
  
# Calculating energy in KWH 
def energy_in_KWH(Power,time_in_min):
    return ((Power*time_in_min)/60)  
# Making an list and value for the list constructing for sending    
def list_prep(List_struktur, value_1, value_2, value_3, value_4):
    
    # Defining local variables for local use
    List = []
    List_struktur = ""
    
    # Making sure the list is empty
    if len(List) == 0:
        List += ["1"]
        List += [len(str(value_1))]
        List += [len(str(value_2))]
        List += [len(str(value_3))]
        List += [len(str(value_4))]
        for i in List:
            List_struktur += str(i)   # Suming the list variables to one string value
    return List_struktur
 
# Function for getting access to API weather sites 
def Access_granted(Website):
    # My header access values
    headers = {
    'User-Agent': "student_task_16",
    'From': 'talhanc@stud.ntnu.no'
    }
    
    # Getting responses and making sure it dosent fail
    Response = requests.get(Website, headers=headers)
    if Response.status_code == 200:
        Response = json.loads(Response.content)
    else: 
        Response = 0
    return Response

# Funtion for solar energy calculation
def solar_panel_energy(shadow_sun,solar_degrees_max):
    """
     E(energy) = A * R * H * PR
    """
    A = 127.28  # Area of solar panel coverage
    R = 0.226   # percentage output from solarpanels 
    
    # If statenent to make sure it produces energy when the sun is up
    if solar_degrees_max > 5:
        H = 200*math.sin(math.radians(20+solar_degrees_max)) # Solar tilt energy loss included
    else: 
        H = 0
        
    PR_perm = 0.75 # Performance ratio defined by losses in cables, inverter etc.
    
    # Including shadow loss and without
    if shadow_sun != 0:
        PR_from_shadow = (100 - shadow_sun)/100 # Shadow loss
        return A*R*H*PR_perm*PR_from_shadow # current with shadow loss
    else:
        return A*R*H*PR_perm # current without shadow loss

# Time function to get time values, updates etc
def time_update_function():
    
    T = "T"
    last_part_time = ":00:00+02:00"
    
    # getting date time and hour time
    now = datetime.now()
    Hour = now.strftime("%H")
    Date = now.strftime("%Y-%m-%d")
    weekNumber = datetime.today().isocalendar()[1]
    
    # Custum time string for url link
    Custom_time_string = Date + T + Hour + last_part_time
    
    return Custom_time_string, weekNumber, Date, Hour

# Function for website access and responses 
def Webside_update(current_date):
    # Defining variables
    url_weather_1 = "https://norway-power.ffail.win/?zone=NO3&date="
    
    # Getting custom link for the day 
    Website_Req_1 = "https://api.met.no/weatherapi/nowcast/2.0/complete?lat=63.4308&lon=10.4034"
    Website_Req_2 = url_weather_1 + current_date
    Website_Req_3 = "https://api.met.no/weatherapi/locationforecast/2.0/complete.json?lat=63.4308&lon=10.4034"
    Website_Req_4 = "https://api.ipgeolocation.io/astronomy?apiKey=599211aac215440baf7c0d680b4f3cd8&lat=63.4308&long=10.4034"
    
    # Accessing respons sites
    response_1 = Access_granted(Website_Req_1)
    response_2 = Access_granted(Website_Req_2)
    response_3 = Access_granted(Website_Req_3)
    response_4 = Access_granted(Website_Req_4)
    
    return response_1, response_2, response_3, response_4

# Sun values for usage in solar panel calculations
def Sun_value_update(response_3, response_4):
    # Information from response 3
    sky_cover = response_3['properties']['timeseries'][0]['data']['instant']['details']['cloud_area_fraction']
    
    # Getting Sun maximum hight in degrees 
    Sun_height = float(response_4['sun_altitude'])
    
    return sky_cover, Sun_height
# Function for prepping room values and sending them to COT
def Room_values_function(response_1, Value_COT_signal, structur_COT_signal):
    
    # Weather data from response 1 
    weather_values = response_1['properties']['timeseries'][0]['data']['instant']['details']
    
    # Getting values, and rounding them without desimals 
    temperature = round(weather_values['air_temperature'])
    wind_speed = round(weather_values['wind_speed'])
    wind_direction = round(weather_values['wind_from_direction'])
    rainfall_value = round(weather_values['precipitation_rate'])

    # temp list 
    Weather_value_list = []
    
    # If statements to determine rain in true og false
    if rainfall_value <= 0:
        rain = 0 
        if wind_direction < 0:
            wind_direction += 360
        Weather_value_list = "1"
        Weather_value_list += str(temperature)
        Weather_value_list += str(wind_speed)
        Weather_value_list += str(wind_direction)
        Weather_value_list += str(rain)
    else: 
        rain = 1
        if wind_direction < 0:
            wind_direction += 360
        Weather_value_list = "1"
        Weather_value_list += str(temperature)
        Weather_value_list += str(wind_speed)
        Weather_value_list += str(wind_direction)
        Weather_value_list += str(rain)
    
    list_size = ""
    list_size = list_prep(list_size, temperature, wind_speed, wind_direction, rain)
    
    # Sending values to COT
    Value_COT_signal.write(Weather_value_list)
    structur_COT_signal.write(list_size)

# Function to add values into COT, like weekly KWt use, solar energy and money to pay
def Update_CSV_COT_values(database_name, cot_kWt, cot_strom, cot_solar, S_energy):
    
    # Reading the local CSV database and getting latest solar value for an dynamic system
    read_csv = pd.read_csv(str("PowerConsumption/" + database_name + ".csv") ,sep = ";")
    read_csv['SK Kroner'] = read_csv['SK Kroner'].astype(float)
    read_csv['SK KWt'] = read_csv['SK KWt'].astype(float)
    
    # Determining if solar energy is producing or not
    if S_energy == 0: 
        latest_solar_KWt = 0
    else:
        latest_solar_KWt = read_csv.loc[read_csv['Person'] == 'Solar']['SK KWt']
        latest_solar_KWt = latest_solar_KWt.iloc[len(latest_solar_KWt)-1]
        latest_solar_KWt = round((-1*latest_solar_KWt*(60/5)),2)  
    
    # Getting current Dataframes for money and KWt use 
    dataframe_kroner = read_csv['SK Kroner']
    dataframe_KWt = read_csv['SK KWt']
    
    # Writing values to COT
    cot_kWt.write(str(round(dataframe_KWt.sum(),2)))
    cot_strom.write(str(round(dataframe_kroner.sum(),2)))
    cot_solar.write(str(round(latest_solar_KWt, 2)))
    
# Funtion for adding in power usage     
def Add_value_CSV(Database, index, Power, Time, KR_KWh):
    
    # Getting value in energi KWH using function in Fs
    energi_KWH = round(energy_in_KWH(Power, Time),2)
    
    # Adding to database
    data_base_price(Database, index, Person_dictionary[int(index)], round((KR_KWh*energi_KWH),2), energi_KWH)
                
# Function for adding weekly power usage
def Add_value_CSV_average_week(Database, Power, Time_value,KR_KWh):
    
    # Adding in an average weekly energy consumption value
    Add_value_CSV(Database, 0, Power, Time_value, KR_KWh)

# Function to updating individuall person current pris
def Update_person_dict(database_name, person_index):
    
    # Reading the current database and isolating pris values
    read_csv = pd.read_csv(str("PowerConsumption/" + database_name + ".csv") ,sep = ";")
    read_csv['SK Kroner'] = read_csv['SK Kroner'].astype(float)
    
    # Getting current person values and summing ut together
    latest_person_kr = read_csv.loc[read_csv['Person'] == str(Person_dictionary[person_index])]['SK Kroner']
    latest_person_kr = round(latest_person_kr.sum(),2)
    
    # If statement for sum up generell current use with solar panel energy
    if person_index == 0:
        Solar_panel_energy = read_csv.loc[read_csv['Person'] == str(Person_dictionary[7])]['SK Kroner']
        Solar_panel_energy = round(Solar_panel_energy.sum(),2)
        latest_person_kr = round(latest_person_kr + Solar_panel_energy ,2)
    
    # Writing to COT with the key dictonary
    cot_Temp = COT(Key_dictionary_people[person_index],token_People_dictionary)
    cot_Temp.write(latest_person_kr)
    
