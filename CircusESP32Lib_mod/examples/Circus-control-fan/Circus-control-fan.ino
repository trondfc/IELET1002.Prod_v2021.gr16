/*
  Circus-control-fan.ino

  This example code shows how to control a fan depending on a signal at circusofthings.com API through its CircusESP32Lib-1.0.0 library for Arduino IDE.

  Created by Jaume Miralles Isern, June 13, 2019.
*/ 

#include <CircusESP32Lib.h>

// ------------------------------------------------
// These are the CircusESP32Lib related declarations
// ------------------------------------------------

char ssid[] = "your_SSID_here"; // Place your wifi SSID here
char password[] =  "your_wifi_password_here"; // Place your wifi password here
char token[] = "your_circus_token_here"; // Place your token, find it in 'account' at Circus. It will identify you.
char server[] = "www.circusofthings.com";
char fanSpeed_key[] = "5115"; // Type the Key of the Circus Signal you want the ESP32 listen to. 
CircusESP32Lib circusESP32(server,ssid,password); // The object representing an ESP32 to whom you can order to Write or Read

// ------------------------------------------------
// These are the Fan Speed Example related declarations
// ------------------------------------------------

// Fan control pins connected to driver
int FanIN3 = 23; // pin 23 will be the ESP32 output connected to the IN3 driver input
int FanIN4 = 22; // pin 22 will be the ESP32 output connected to the IN4 driver input 
int FanENB = 21; // pin 21 will be the ESP32 output, providing PWM, connected to the ENB driver input
// PWM properties
const int freq = 30000; 
const int pwmChannel = 0; // There might be more PWM outputs, let's use the first available
const int resolution = 8; // means that we have 256 posible values to define the duty cycle of PWM in percentage. 0 means 0%, 255 means 100%
int dutyCycle = 0; // let's start with the fan stopped


void setup() {
  // Enable console
  Serial.begin(115200);
  // Let the Circus object set up itself for an SSL/Secure connection
  circusESP32.begin(); 
  // set pins as outputs
  pinMode(FanIN3, OUTPUT);
  pinMode(FanIN4, OUTPUT);
  pinMode(FanENB, OUTPUT);
  // configure LED PWM functionalitites
  ledcSetup(pwmChannel, freq, resolution);
  // attach the channel to the FanENB to be controlled
  ledcAttachPin(FanENB, pwmChannel);
  // Move DC motor forward
  digitalWrite(FanIN4, HIGH);
  digitalWrite(FanIN3, LOW);
}

void loop() {
  // This will read the value of my signal at Circus. I'm supposed to set to between 0 and 255.
  double fanSpeed_setPoint = circusESP32.read(fanSpeed_key,token); 
  // As I defined at Circus the range as the duty cycle, there's no need to make aditional conversions
  dutyCycle = fanSpeed_setPoint;
  // Set a new PWM duty cycle in output, thus a new fan speed
  ledcWrite(pwmChannel, dutyCycle);
  Serial.print("Fan duty cycle (0-255) set to: ");
  Serial.println(dutyCycle);
  delay(3000);
}
