#include <analogWrite.h>
#include <ESP32_Servo.h>
#include <CircusESP32Lib.h>

int id = 1;

// Motor pin.
const int EN  = 27;

// Variabler for verdiene som hentes fra cot.
int motor;
int servo_value;

const int servo_pin = 14;           
Servo servo;

// Cot.
char ssid[] = "Vilma andrea sin iPhone";       
char password[] = "hei456hei";                       
char token_1[] = "eyJhbGciOiJIUzI1NiJ9.eyJqdGkiOiI1MDkzIn0.XAZI5Bef9tYoXnOOlOAzWouhiRUkplsBL5r7p3YjRto";   
char token_2[] = "eyJhbGciOiJIUzI1NiJ9.eyJqdGkiOiI1MDkwIn0.jPlTuw_H-Rb41ZDRMFSK_799rn4s4UUJ9ioQAXwbcjE";
char token_3[] = "eyJhbGciOiJIUzI1NiJ9.eyJqdGkiOiI0OTEwIn0.5SXKcTgsyeYGALKiUW68wQ-UA7wARGUy4dhXrjcXE-g";
char server[] = "www.circusofthings.com";

CircusESP32Lib circusESP32(server, ssid, password);

char key_motor[] = "28671";
char key_servo[] = "12005";

void setup() {
  pinMode(EN, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  servo.attach(servo_pin);
  
  Serial.begin(115200);   
  circusESP32.begin();
 
}

void loop() {
  // Leser av verdiene for motor og servo fra cot.
  motor = circusESP32.read(key_motor, token_1);
  servo_value = circusESP32.read(key_servo, token_1);
  
  analogWrite(EN, motor);               // Kjører motor på gitt hastighet.
  servo.write(servo_value);             // Setter servo til gitt antall grader.
  
  delay(1000);
}
