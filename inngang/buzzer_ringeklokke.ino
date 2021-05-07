// definerer variabler
#define buzzer_pin 14
#define button_pin 13

void setup() {
  ledcSetup(0, 5000, 8); // ledckanal, frekvens og antal bit
  ledcAttachPin(buzzer_pin, 0); // kobler buzzer_pin til kanal 0
  pinMode(button_pin, INPUT); // definerer button_pin som input
  Serial.begin(115200);  
}

void loop() {
 int buttonState = digitalRead(button_pin); // lagrer verdiene fra butto_pin i variabelen buttonState
 Serial.print(buttonState);

 // om knappen trykkes vil mellodien spille
 if(buttonState == LOW){
  ledcWriteTone(0, 500);
  delay(350);
  ledcWriteTone(0, 0);
  delay(30);
  ledcWriteTone(0, 400);
  delay(700);
  ledcWriteTone(0, 0);
  delay(1000);
 
 }

else{
  ledcWriteTone(0,0); //om knappen ikke trykkes vil det ikke komme noe lyd
 }
 delay(10);
}

  
