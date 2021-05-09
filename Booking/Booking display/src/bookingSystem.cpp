//Libraries
#include <Arduino.h>
#include <Adafruit_GFX.h>    // Core library
#include <Adafruit_ST7735.h> // Library for ST7735
#include <SPI.h>// Library for Serial Peripheral Interface
#include <string.h> //Library for lettere endre strenger
#include <CircusESP32Lib.h> //For kommunikasjon med CoT

//Pins tilkoblet skjermen. I tilegg kobler man ground til ground og 3,3V til Led på skjermen. VCC kan man ignorere
#define TFT_CS    5  // Chip select
#define TFT_RST   4  // Reset på skjerm
#define TFT_DC   17  // DataCommand, A0 på skjerm
#define TFT_MOSI 19  // Data (Bildet som sendes), SDA på skjerm
#define TFT_SCLK 18  // Klokke, SCK på skjerm

//Deklarasjon for joystick. x,y, og button til pins.NB: Må bruke ADC1 pins pga WIFI!
const int VRX = 33;  
const int VRY = 34;
const int SW = 32;

//Variabler fra joystick, initialiserer med 0 som verdi(ingen bevegelse)
int button = 0;
int hoyre = 0;
int opp = 0;
int lastButton = 0;
int lastOpp = 0;
int lastHoyre = 0;

//Div konstanter og variabler for tegning på skjerm
int linjeavstand = 18; //Piksler som brukes for 1 linje(Brukes til å tegne på neste linje)
int pilplassering = 130;// Plasserer pilvelger 130 piksler mot høyre på skjerm
int menylinje = 1;
int valg = 4; //Antall elementer som kan velges på menyene(endres etter den enkelte meny)
char *screen = "hovedmeny";

//Variabler for sending til CoT
const int arrayLength = 7;
int userIdNum = 1; //nummer som identifiserer personen i rekka
unsigned long defaultList = 1100000;
int indexVar = 0;
int tidReservert = 0;
char kjokkenSignal[] = "2384"; // signal nøkler
char stueSignal[] = "23370";
char badSignal[] = "12707";
char kjokkenTidSignal[] = "8185";
char stueTidSignal[] = "15699";
char badTidSignal[] = "1662";
char badDusjSignal[] = "28076";
char kjokkenOvnSignal[] = "13261";

//Cot Server
//char ssid[] = "AndroidAP"; 
//char password[] = "melk4534"; 
char ssid[] = "Android"; //Lauritz
char password[] =  "fisker123"; 
char server[] = "www.circusofthings.com";
char token1[] = "eyJhbGciOiJIUzI1NiJ9.eyJqdGkiOiI1MTA1In0.rtwgSROsIox_OXVB5CLaltp6GdoBp0BA0DtS2Fv3fpM"; // Lau         
char token2[] = "eyJhbGciOiJIUzI1NiJ9.eyJqdGkiOiI1MDkzIn0.XAZI5Bef9tYoXnOOlOAzWouhiRUkplsBL5r7p3YjRto"; // Vil
char key_temp[] = "4891";

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST); //Starter en instans av skjermen
CircusESP32Lib circusESP32(server, ssid , password);

int arrayToInt(int liste[]){ // funksjon for å konvertere fra array to int
  int k = 0;
  for (int i = 0; i < arrayLength; i++){ 
    k = 10* k + liste[i];
  }
  return k;
}
void intToArray(int num, int liste[]){ // funksjon for int til array
  for (int i = 6; i>= 0; i--){
    liste[i] = num % 10;
    num /= 10;
  }
}
int bookFunc(char keyBook[],int reservert,char keyTime[]){ // funksjon som endrer verdi i kølisten og sender verdien til CoT
  int queueArray[arrayLength];
  int timeArray[arrayLength];

  int queueNum = circusESP32.read(keyBook,token1);
  int timeNum = circusESP32.read(keyTime,token1);

  intToArray(queueNum,queueArray);
  intToArray(timeNum,timeArray);

  int index = 0;
  for (int i = 0; i < 7; i++){
    if (queueArray[i] == 0){
      queueArray[i] = userIdNum;
      timeArray[i] = reservert;
      index = i;
      break;
    }
    //Serial.print(tidListe[i]);
  }
  Serial.println("");
  circusESP32.write(keyBook,arrayToInt(queueArray),token1); //skriver listene til cot
  circusESP32.write(keyTime,arrayToInt(timeArray),token1);
  //circusESP32.write_array(key,liste,arrayLength,token1,indexSize);
  return index;
}

void extraChoice(char key[], int index,int choice){
  int array[arrayLength];
  int num = circusESP32.read(key,token1);
  intToArray(num,array);
  array[index] = choice;
}
void joystick() {
 
 int JX = analogRead(VRX);
 int JY = analogRead(VRY);
 int B = analogRead(SW);
  if (B < 10){
    button = 1;}
  else {
    button = 0;
  }
  if (JX > 3000){
    hoyre = 2;}
  else if (JX < 1000){
    hoyre = 1;
  }
  else {hoyre = 0;
  }
  if (JY > 3000){
    opp = 1;}
  else if (JY < 1000){
    opp = 2;
  }
  else {opp = 0;
  }
}
void default_lister(){ //Skriver default lister til CoT
  circusESP32.write(badSignal,defaultList,token1); 
  circusESP32.write(kjokkenSignal,defaultList,token1);
  circusESP32.write(stueSignal,defaultList,token1);
  circusESP32.write(badTidSignal,defaultList,token1);
  circusESP32.write(kjokkenTidSignal,defaultList,token1);
  circusESP32.write(stueTidSignal,defaultList,token1);
  circusESP32.write(badDusjSignal,defaultList,token1);
  circusESP32.write(kjokkenOvnSignal,defaultList,token1);
}
void hovedmeny() {
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);
  tft.setCursor(0, 0);
  tft.print("Kjokken");
  tft.setCursor(26, 0);
  tft.print("/"); //Hacky metode for å skrive over o med en / for å lage ø
  tft.setCursor(0, linjeavstand); // 2 Parametere (Høy verdi gir offset mot høyre,Høy verdi offset nedover på skjerm)
  tft.print("Bad");
  tft.setCursor(0, 2 * linjeavstand); 
  tft.print("Stue");
  tft.setCursor(0, 3 * linjeavstand); 
  tft.print("Soverom");
  tft.setCursor(0, 4 * linjeavstand); 
  tft.print("Gjester");
  tft.setCursor(0, 110); 
  tft.print("Id:");
  tft.print(userIdNum);
  valg = 5;
  }

void setup() {
  Serial.begin(115200);
  circusESP32.begin();
  tft.initR(INITR_BLACKTAB);     // Initialiserer skjerm ST7735S 
  tft.fillScreen(ST77XX_BLACK);  //Skriver hele skjermen svart
  tft.setRotation(1);  //Roterer skjerm 90 grader ,(0-3)som gir totalt 360

  hovedmeny(); //Tegner hovedmeny 
  pinMode(VRX , INPUT); //Pins til joystick
  pinMode(VRY , INPUT);
  pinMode(SW , INPUT);
  attachInterrupt(digitalPinToInterrupt(VRX),joystick,CHANGE);
  attachInterrupt(digitalPinToInterrupt(VRY),joystick,CHANGE);
  attachInterrupt(digitalPinToInterrupt(SW),joystick,FALLING);
  
  default_lister(); //Kjører funksjon som setter default lister i CoT (uncomment)
}
void piltegner() {
   if (opp != lastOpp) {
      tft.setTextColor(ST77XX_BLACK);
      tft.setCursor(pilplassering, 0); // 2 Parametre (Høy verdi offset mot høyre,Høy verdi offset nedover på skjerm)
      tft.print("<-");
      tft.setCursor(pilplassering, linjeavstand);
      tft.print("<-");
      tft.setCursor(pilplassering, 2 * linjeavstand); 
      tft.print("<-");
      tft.setCursor(pilplassering, 3 * linjeavstand); 
      tft.print("<-");
      tft.setCursor(pilplassering, 4 * linjeavstand); 
      tft.print("<-");
      delay(3);
      tft.setTextColor(ST77XX_GREEN,ST77XX_BLACK);
    
    if (opp == 1 && menylinje < valg){
      menylinje += 1;
      
    }
    else if(opp == 2 && menylinje > 1) {
      menylinje += -1;
    }
    }
    tft.setCursor(pilplassering, menylinje * linjeavstand - linjeavstand);  
    tft.print("<-");   
}
void tidsvalg(){
  int m = 0;
  int i = 1;
  int t = 0;
  char *rom = "ingen";
  if (screen == "undermeny3"){m = 15;} //Undermeny 3 = stue, som betyr at den øker med 15 min istede for 5 min
  else {m = 5;}
  if (strstr(screen,"1")){rom = "kjokken";}
  else if (strstr(screen,"2")){rom = "bad";}
  else if (strstr(screen,"3")){rom = "stue";}
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);
  tft.setCursor(0 , 0);
  tft.print("Book");
  tft.setCursor(52 , 0);
  tft.print(rom);
  tft.print(" i");
  tft.setCursor(0 , linjeavstand);
  tft.print(m);
  tft.setCursor(30 , linjeavstand);
  tft.print(" minutter.");
  while(true){
    joystick();
      if (opp == 1){ //Stikken er ned
        if (i > 1){ i += -1;}// i går ned-> tiden går ned
        t = m * i;
        tft.setCursor(0 , linjeavstand);
        tft.fillRect(0,linjeavstand,34,2 * linjeavstand ,ST77XX_BLACK);
        tft.print(t);
        }
      else if (opp == 2){//Stikken er opp
        if (i < 9 ){ i += 1;}// i går opp -> tiden går opp
        t = m * i;
        tft.setCursor(0 , linjeavstand);
        tft.fillRect(0,linjeavstand,34,2 * linjeavstand ,ST77XX_BLACK);
        tft.print(t);
      }
      else if (button == 1){      
        
        if (screen == "undermeny1") {
            indexVar = bookFunc(kjokkenSignal,i,kjokkenTidSignal);
            rom = "kjokken";
            //extraChoice(badDusjSignal,indexVar,choice);
          }
         else if(screen == "undermeny2") {
            indexVar = bookFunc(badSignal,i,badTidSignal);
            rom = "bad";
            //extraChoice(badDusjSignal,indexVar,choice);
          }
         else if(screen == "undermeny3") {
            indexVar = bookFunc(stueSignal,i,stueTidSignal);
            rom = "stue";
         }
         //Suksessskjerm
         tft.fillScreen(ST77XX_BLACK); 
         tft.setCursor(0 , 0);
         tft.print("Du ha booket ");
         tft.print(rom);
         tft.print(" i ");
         tft.println(i * m);
         tft.print("minutter.");
         delay(6000); //Vise suksess i 6 sek
         //Tilbake til hovemeny
         tft.fillScreen(ST77XX_BLACK);
         menylinje = 1;
         button = 0;
         screen = "hovedmeny";
         hovedmeny();
         break;
      }
      else if (hoyre == 1) { //Stikken til venstre går tilbake til forrige meny
        tft.fillScreen(ST77XX_BLACK);
        menylinje = 1;
        screen = "hovedmeny";
        hovedmeny();
        break;
      }
      delay(300);
      }
       
  
  }
  
void undermeny1(){
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);
  tft.setCursor(0 , 0);
  tft.print("Spise");
  tft.setCursor(0, linjeavstand); 
  tft.print("Komfyr");
  tft.setCursor(0, 2 * linjeavstand); 
  tft.print("Stekeovn");
  tft.setCursor(0, 3 * linjeavstand); 
  tft.print("Komfyr ");
  tft.setCursor(77 , 3 * linjeavstand);
  tft.print("&");
  tft.setCursor(93 , 3 * linjeavstand);
  tft.print("Ovn");
  valg = 4;
  }
  
void undermeny2(){
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);
  tft.setCursor(0 , 0);
  tft.print("Kun bad");
  tft.setCursor(0, linjeavstand);  
  tft.print("Dusj");
  valg = 2;
  }

void undermeny3(){
    tidsvalg();
    
  }

void undermeny4(){
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);
  tft.setCursor(0 , 0);
  tft.print("Temp");
  tft.setCursor(0, linjeavstand);  
  tft.print("Manuell");
  valg = 2;   
  }

  void undermeny5(){
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);
  tft.setCursor(0 , 0);
  tft.print("Legge til");
  tft.setCursor(0, linjeavstand);  
  tft.print("Fjerne");
  valg = 2;   
  }
  
void standardtemp(){ //Meny for endring av standardverdier for temperatur på soverom
  char char_temp[7] = {1,11,11,11}; 
  int *array_temp = circusESP32.read_array(char_temp, key_temp, token2);
  int temp_natt = array_temp[1];
  int temp_dag = array_temp[2];
  int temp_ute = array_temp[3];
  tft.setCursor(0 , 0);
  tft.print("Dag:");
  tft.print(temp_dag);
  tft.setCursor(0, linjeavstand);  
  tft.print("Natt:");
  tft.print(temp_natt);
  tft.setCursor(0,  2 * linjeavstand);  
  tft.print("Borte:");
  tft.print(temp_ute);
  valg = 3;
  while(true){
    joystick();
    piltegner();
    if (hoyre == 1) { //Stikken til venstre går tilbake til forrige meny
        tft.fillScreen(ST77XX_BLACK);
        menylinje = 1;
        screen = "undermeny4";
        undermeny4();
        break;
        }
    if (button == 1){
        while(true){
          delay(300);
          int t = temp_natt;
          joystick();
          if (opp == 1){ //Stikken er ned 
              if (t > 0){ t += -1;} //Temp går ned, minimum 0C
              }
          else if (opp == 2){//Stikken er opp
              if (t < 40 ){ t += 1;}// Temp går opp max 40c
             }
          if (hoyre == 1) { //Stikken til venstre går tilbake til forrige meny
          tft.fillScreen(ST77XX_BLACK);
          screen = "standardtemp";
          standardtemp();
          break;
            }
          tft.setCursor(70 , (menylinje - 1) * linjeavstand);// Basert på menylinje , endre denne biten.
          tft.fillRect(70,(menylinje - 1) * linjeavstand,85, menylinje * linjeavstand ,ST77XX_BLACK);
          if (button == 1){
             if (menylinje == 2){
          tft.print(temp_natt); //Endre til å printe riktig temp på riktig sted
          //array_temp[1] = temp_natt;
         }
          //array_temp[0] = 1;
        
          //temp_dag, temp_ute};
          circusESP32.write_array(key_temp, array_temp, 7, token2, char_temp);
        
          break;
          // Printe sukksesmelding
          hovedmeny();
          }
      
        }
      
      delay(300);  
     }
  }
}

void manuell(){
  //vilma
}


  


void loop() {
  joystick(); 
  if (screen == "hovedmeny" ||screen == "undermeny1" ||screen == "undermeny2" ||screen == "undermeny4"||screen == "undermeny5"  ){
    piltegner();
  }
  if (button == 1 && screen == "undermeny4"){
    tft.fillScreen(ST77XX_BLACK);
      if (menylinje == 1){      
       screen = "standardtemp";
       standardtemp();   
       } 
      if (menylinje == 2){ 
       screen = "manuell";
       manuell();
      }
      menylinje = 1;
   }
    if (button == 1 && screen == "undermeny2"){
    tft.fillScreen(ST77XX_BLACK);
      if (menylinje == 1){      // ENdre. Må sette noe til booking av extraCHoice funksjonen
          
       } 
      if (menylinje == 2){ 
       
      }
      tidsvalg();
   }
    if (button == 1 && screen == "undermeny1"){
    tft.fillScreen(ST77XX_BLACK);
      if (menylinje == 1){      // ENdre. Må sette noe til booking av extraCHoice funksjonen
          
       } 
      if (menylinje == 2){ 
       
      }
      tidsvalg();
   }
  if (button == 1 && screen == "hovedmeny"){
    tft.fillScreen(ST77XX_BLACK);
       if (menylinje == 1){      
       screen = "undermeny1"; //Sier hvor vi er til resten av programmet
       undermeny1();   //Kjører tegning av menyen
       } 
       if (menylinje == 2){ 
       screen = "undermeny2";
       undermeny2();
       }
       if (menylinje == 3){
       screen = "undermeny3";
       undermeny3();
       }
       if (menylinje == 4){
       screen = "undermeny4";
       undermeny4();
       }
       if (menylinje == 5){
       screen = "undermeny5";
       undermeny5();
       }
    menylinje = 1; //Sier at pila skal plasseres på det øverste elementet i ny meny
    }
    
    if (hoyre == 1 && strstr(screen,"undermeny")){
      tft.fillScreen(ST77XX_BLACK);
      menylinje = 1;
      screen = "hovedmeny";
      hovedmeny();
    } 
    delay(150);
  lastOpp = opp;  //Brukes i piltegner for å bare gå ett steg opp og ned i menyene
  
  int printe = 0;
  if (printe == 1){ //Koden under er for printe verdier for feilsøking
  Serial.println(screen);
  Serial.println("HØYRE:");
  Serial.println(analogRead(VRX));
  Serial.println(hoyre);
  Serial.println("OPP:");
  Serial.println(analogRead(VRY));
  Serial.println(opp);
  Serial.println("KNAPP:");
  Serial.println(analogRead(SW));
  Serial.println(button);
  Serial.println("------------");
    }
  
}
