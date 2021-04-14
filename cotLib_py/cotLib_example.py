from cotLib import COT                          # Import cot library
from random import randint                      # Import randint function
from time import sleep                          # Import sleep function

cot = COT("29465","eyJhbGciOiJIUzI1NiJ9.eyJqdGkiOiI1MDUwIn0.kbJUuhELenracPimjgI8LYLrpIMOKGOcUHqS_-ok5z8")
test_size = [1,2,1,4,1,1]                       # Create list of digit size
test_number = 9876543210                          # Create Start number

print(cot.write(test_number))                   # Send start number to cot, print response
for i in range(5):                              # Loop 5 times
    data = cot.read_list(test_size)             # Get list from cot acording to test_size
    print(data)                                 # Print the list
    data[1] = randint(0,99)                     # Change list[1] to random int betwen 0 and 99
    print(data)                                 # Print the list
    print(cot.write_list(data,test_size))       # Send new list to cot 
    sleep(5)                                    # Wait for 5s    