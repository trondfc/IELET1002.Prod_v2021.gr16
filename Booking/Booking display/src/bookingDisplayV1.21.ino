//Libraries
#include <Adafruit_GFX.h>    // Core library
#include <Adafruit_ST7735.h> // Library for ST7735
#include <SPI.h>// Library for Serial Peripheral Interface
#include <string.h> //Library for lettere endre strenger
#include <CircusESP32Lib.h> //For kommunikasjon med CoT
#include <time.h>
#include <driver/rtc_io.h>

//Pins tilkoblet skjermen. VCC kan man ignorere
#define TFT_CS    5  // Chip select
#define TFT_RST   4  // Reset på skjerm
#define TFT_DC   17  // DataCommand, A0 på skjerm
#define TFT_MOSI 19  // Data (Bildet som sendes), SDA på skjerm
#define TFT_SCLK 18  // Klokke, SCK på skjerm
const int tftPower = 23; //Gir strøm til skjerm, kobles til LED på skjerm. Brukes for å skru av skjerm i deepsleep

//Deklarasjon for joystick. x,y, og button til pins.NB: Må ikke bruke ADC2 pins pga WIFI!
const int VRX = 33;  
const int VRY = 34;
const int SW = 32;

//Variabler fra joystick, initialiserer med 0 som verdi(ingen bevegelse)
int button = 0;
int hoyre = 0;
int opp = 0;

//Variabel for deepsleep
unsigned long myTime;

//Variabler for tegning på skjerm
int linjeavstand = 18; //Piksler som brukes for 1 linje(Brukes til å tegne på neste linje)
int pilplassering = 130;// Plasserer pilvelger 130 piksler mot høyre på skjerm
int menylinje = 1;
int valg = 4; //Antall elementer som kan velges på menyene(endres etter den enkelte meny)
char *screen = "hovedmeny";
char *names[] = {"Trond", "Lauritz", "Astrid",
                     "Joakim", "Vilma", "Talha"};

//Variabler for sending til CoT
const int arrayLength = 7; //Antall i kollektivet pluss 1, pga problemer med ledende 0
int userIdNum = 4; //nummer som identifiserer personen (knyttet til names rett over)
unsigned long defaultList = 1100000;
int indexVar = 0;
int tidReservert = 0;
char kjokkenSignal[] = "2384"; // signalnøkler
char stueSignal[] = "23370";
char badSignal[] = "12707";
char kjokkenTidSignal[] = "8185";
char stueTidSignal[] = "15699";
char badTidSignal[] = "1662";
char badDusjSignal[] = "28076";
char kjokkenOvnSignal[] = "13261";
char gjestSignal[] = "4747";
char key_temp[] = "4891";
char key_override[] = "8417";  

//Cot Server
char ssid[] = "AndroidAP"; //J
char password[] = "melk4534"; 
//char ssid[] = "Android"; //Lauritz
//char password[] =  "fisker123"; 
char server[] = "www.circusofthings.com";
char token1[] = "eyJhbGciOiJIUzI1NiJ9.eyJqdGkiOiI1MTA1In0.rtwgSROsIox_OXVB5CLaltp6GdoBp0BA0DtS2Fv3fpM"; // Lau         
char token2[] = "eyJhbGciOiJIUzI1NiJ9.eyJqdGkiOiI1MDkzIn0.XAZI5Bef9tYoXnOOlOAzWouhiRUkplsBL5r7p3YjRto"; // Vil

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
  int queueNum = circusESP32.read(keyBook,token1); //leser verdiene fra CoT
  int timeNum = circusESP32.read(keyTime,token1);

  intToArray(queueNum,queueArray); //gjør om til array
  intToArray(timeNum,timeArray);

  int index = 0;
  if(queueArray[6] != 0){ //returnerer -1 hvis listen er full
    return -1;
  }
  else{
    for (int i = 0; i < 7; i++){
      if (queueArray[i] == 0){ //leter etter ledig plass i listen
        queueArray[i] = userIdNum; //setter ny verdi i lista
        timeArray[i] = reservert;
        index = i; //hvilken index som ble endret
        circusESP32.write(keyBook,arrayToInt(queueArray),token1); //skriver listene til cot
        circusESP32.write(keyTime,arrayToInt(timeArray),token1);
        break;
       }
    }  
  }
  return index;
}

void joystick() { 
 int X = analogRead(VRX);
 int Y = analogRead(VRY);
 int B = analogRead(SW);
  if (B < 10){
    button = 1;}   
  else {
    button = 0;
  }
  if (X > 3000){
    hoyre = 2;
    myTime = millis();
  }
  else if (X < 1000){
    hoyre = 1;
    myTime = millis();
  }
  else {hoyre = 0;
  }
  if (Y > 3000){
    opp = 1;
    myTime = millis();
  }
  else if (Y < 1000){
    opp = 2;
    myTime = millis();
  }
  else {opp = 0;
  }
}

int checkTime(char keyBook[],char keyTime[]){ // funksjon som itererer gjennom tidslisten og sier hvor lang tid det tar før det er brukerens tur
  int sum = 0;
  int countDown;
  int timeArray[arrayLength];
  int timeMultiplier;
  int timeNum = circusESP32.read(keyTime,token1);
  intToArray(timeNum,timeArray);

  if(strcmp(keyBook, "23370") == 0){ //Tar hensyn til at multiplier er 15 for stue
    timeMultiplier = 15;
  }else{
    timeMultiplier = 5;
  }
  for (int i = 1; i <7; i++){
    sum += timeArray[i]; 
    Serial.println("Sum: " + String(sum)); 
  }
   countDown = sum*timeMultiplier;  
   return countDown;
}

void extraChoice(char key[], int index,int choice){ // funksjon for rom med ekstra valg lister
  int array[arrayLength];
  int num = circusESP32.read(key,token1);
  intToArray(num,array);
  array[index] = choice;
  circusESP32.write(key,arrayToInt(array),token1);
}

int guestListModify(char key[],int gjester){ //tar antall gjester -2<=gjester<=2
  int array[arrayLength];
  int num = circusESP32.read(key,token1);
  intToArray(num,array);
  array[userIdNum] += gjester;
  if (array[userIdNum] < 0){
    array[userIdNum] = 0;
    }
  if(array[userIdNum] <= 2){
      circusESP32.write(key,arrayToInt(array),token1);
      return 0;
        }
        else{
      //ERROR funksjonen returnerer int 1 når noen prøver å legge til flere enn 2
      return 1;
        }   
}

void piltegner() { //Hvis stikken er opp eller ned, endrer menylinje, tegner alt svart, skriver ny pil på plasseringen menylinje
   if (screen == "manuell2") {
     return;
   }
   else {
   if (opp != 0) {
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
}

void hovedmeny() {
  tft.fillScreen(ST77XX_BLACK); 
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
  tft.print(String(userIdNum) + ",");
  tft.print(names[userIdNum -1]);
  valg = 5;
  }
  
  void returnToMainmenu() {
         tft.fillScreen(ST77XX_BLACK);
         menylinje = 1;
         button = 0;
         hoyre = 0;
         opp = 0;
         screen = "hovedmeny";
         hovedmeny();    
        }
        
void tidsvalg(){
  tft.fillScreen(ST77XX_BLACK); 
  int m = 0; //Multiplier
  int i = 1; //Tellevariabel som er den som blir sendt til CoT
  int t = 0; // m * i = t --> Reell tid som bookes
  int choice;
  int ventetid;
  char *rom = "ingen";
  if (screen == "undermeny3"){m = 15;} //Undermeny 3 = stue, som betyr at den øker med 15 min istede for 5 min
  else {m = 5;}
  if (strstr(screen,"1")){rom = "kjokken";}
  else if (strstr(screen,"2")){rom = "bad";}
  else if (strstr(screen,"3")){rom = "stue";} 
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
        tft.setCursor(0 , 5 * linjeavstand);
        tft.print("Booker...");
        if (screen == "undermeny1") {
            ventetid = checkTime(kjokkenSignal,kjokkenTidSignal);
            indexVar = bookFunc(kjokkenSignal,i,kjokkenTidSignal);
            rom = "kjokken";
            if (menylinje != 1){
              if (menylinje < 4){
              choice = 1;//For komfyr eller stekeovn
              }
              else if (menylinje == 4){
                choice = 2; //For booking av både komfyr og stekeovn
              }
              if(indexVar != -1){
                extraChoice(kjokkenOvnSignal,indexVar,choice);
              }
            }
          }
         else if(screen == "undermeny2") {
            ventetid = checkTime(badSignal,badTidSignal);
            indexVar = bookFunc(badSignal,i,badTidSignal);
            rom = "bad";
            if (menylinje != 1){ //For booking av dusj
              choice = menylinje - 1;//Choice sendes til ekstra lista
              if(indexVar != -1){
                extraChoice(badDusjSignal,indexVar,choice);
              }
            }
          }
         else if(screen == "undermeny3") {
            ventetid = checkTime(stueSignal,stueTidSignal);
            indexVar = bookFunc(stueSignal,i,stueTidSignal);
            rom = "stue";
            }
         //Suksessskjerm
         if(indexVar != -1){
            tft.fillScreen(ST77XX_BLACK); 
            tft.setCursor(0 , 0);
            tft.print("Du ha booket ");
            tft.print(rom);
            tft.print(" i ");
            tft.println(i * m);
            tft.print("minutter.");
            tft.setCursor(0 , 3 * linjeavstand);         
            tft.print("Ledig om "+ String(ventetid));
            tft.setCursor(0 , 4 * linjeavstand);  
            tft.print("minutter.");
         }
         else{
            tft.fillScreen(ST77XX_BLACK);
            tft.setCursor(0 , 0);
            tft.print("Det er for ");
            tft.print("mange i koen.");
         }
         delay(6000);
         //Tilbake til hovemeny
         returnToMainmenu();
         break;
      }
      else if (hoyre == 1) { //Stikken til venstre går tilbake til forrige meny
       returnToMainmenu();
        break;
         }
      delay(300);
      }      
  }

void undermeny1(){ //Kjøkken
  tft.fillScreen(ST77XX_BLACK); 
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
  
void undermeny2(){ //Bad
  tft.fillScreen(ST77XX_BLACK); 
  tft.setCursor(0 , 0);
  tft.print("Kun bad");
  tft.setCursor(0, linjeavstand);  
  tft.print("Dusj");
  valg = 2;
  }

void undermeny3(){ //Stue
    tft.fillScreen(ST77XX_BLACK); 
    tidsvalg();    
  }

void undermeny4(){ //Soverom
  tft.fillScreen(ST77XX_BLACK); 
  tft.setCursor(0 , 0);
  tft.print("Temp");
  tft.setCursor(0, linjeavstand);  
  tft.print("Manuell");
  valg = 2;   
  }

void undermeny5(){ //Gjester
  tft.fillScreen(ST77XX_BLACK); 
  tft.setCursor(0 , 0);
  tft.print("Legge til");
  tft.setCursor(0, linjeavstand);  
  tft.print("Fjerne");
  valg = 2;   
  }

void standardtemp(){ //Meny for endring av standardverdier for temperatur på soverom
  tft.fillScreen(ST77XX_BLACK); 
  char char_temp[4] = {1,2,2,2}; 
  int *array_temp = circusESP32.read_array(char_temp, key_temp, token2);
  int temp_natt = array_temp[1];
  int temp_dag = array_temp[2];
  int temp_borte = array_temp[3];
  tft.setCursor(0 , 0);
  tft.print("Natt:");
  tft.setCursor(70 , 0);
  tft.print(String(temp_natt) + "\xF7" + "C");
  tft.setCursor(0, linjeavstand);  
  tft.print("Dag:");
  tft.setCursor(70 , linjeavstand);
  tft.print(String(temp_dag) + "\xF7" + "C");
  tft.setCursor(0,  2 * linjeavstand);  
  tft.print("Borte:");
  tft.setCursor(70,  2 * linjeavstand); 
  tft.print(String(temp_borte)+ "\xF7" + "C");
  valg = 3;
  while(screen == "standardtemp"){
    joystick();
    piltegner();
    delay(300); 
    if (hoyre == 1) { //Stikken til venstre går tilbake til forrige meny
        tft.fillScreen(ST77XX_BLACK);       
        hoyre = 0;
        screen = "undermeny4";
        undermeny4();       
        break;
        }
    if (button == 1){
      int t = array_temp[menylinje];
      delay(200);//For å unngå at button enda er inne
        while(true){
          delay(150);
          tft.fillRect(70,(menylinje - 1) * linjeavstand, 10 , linjeavstand ,ST77XX_BLACK);
          joystick();
          if (opp == 1){ //Stikken er ned 
              if (t > 10){ t += -1;} //Temp går ned, minimum 10C
              
              }
          else if (opp == 2){//Stikken er opp
              if (t < 40 ){ t += 1;}// Temp går opp max 40c
              
             }
          tft.setCursor(70 , (menylinje - 1) * linjeavstand);  
          tft.print(t);   
          if (hoyre == 1) { //Stikken til venstre går tilbake til forrige meny
          tft.fillScreen(ST77XX_BLACK);
          screen = "standardtemp";
          standardtemp();
          break;
            }       
          if (button == 1){
            array_temp[menylinje] = t;
            tft.setCursor(0,  5 * linjeavstand); 
            tft.print("Kontakter CoT");
            circusESP32.write_array(key_temp, array_temp, 4, token2, char_temp);
            tft.setCursor(0,  6 * linjeavstand);
            tft.print("Suksess.");
            delay(3000); 
            returnToMainmenu();
            break;
           }
          }
        }      
       
     }
}

void manuell(){
  tft.fillScreen(ST77XX_BLACK); 
  char char_override[4] = {1,1,1,1};
  int array_override[4] = {0,0,0,0};
  tft.setCursor(0 , 0);
  tft.print("Vifte"); 
  tft.setCursor(0, linjeavstand);  
  tft.print("Ovn");
  tft.setCursor(0,  2 * linjeavstand);  
  tft.print("Vindu");
  tft.setCursor(0,  3 * linjeavstand);  
  tft.print("Sett i auto");
  valg = 4;
  while(screen == "manuell"){ 
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
      screen = "manuell2";    
      int i = 0;
      button = 0;  
      delay(200);   
      while(screen == "manuell2") {
        joystick();       
        if (hoyre == 1) { //Stikken til venstre går tilbake til forrige meny
        tft.fillScreen(ST77XX_BLACK);
        menylinje = 1;
        hoyre = 0;
        screen = "undermeny4";
        undermeny4();
        break;
        }
        if (opp == 1 && i > 0){ i -= 1;}                  
        else if (opp == 2){
          switch (menylinje){
            case 1: if (i < 3) {i++; break;}
            case 2: if (i < 1) {i++; break;}
            case 3: if (i < 4) {i++; break;}
            default: break;        
          }
        }   
       
        tft.setCursor(70 ,  (menylinje - 1) * linjeavstand);
        if (menylinje != 4) {
        tft.print(i); 
         }
                              
        if (button == 1 && menylinje != 4) {
            tft.setCursor(0,  5 * linjeavstand); 
            tft.print("Kontakter CoT");
            //int *manuell = circusESP32.read_array(char_override, key_override, token2);
            array_override[0] = 1;
            array_override[menylinje] = i;           
            circusESP32.write_array(key_override, array_override,4, token2,char_override);
            tft.print("Suksess.");
            delay(3000); 
            returnToMainmenu();
            break;
          }
        else if (button == 1 && menylinje == 4) {
            tft.setCursor(0,  5 * linjeavstand); 
            tft.print("Kontakter CoT");
            array_override[0] = 2;
            circusESP32.write_array(key_override, array_override,4, token2,char_override);
            //Sukksess
            tft.setCursor(0,  6 * linjeavstand);
            tft.print("Satt i auto");
            delay(3000);
            returnToMainmenu();
            break;
            }       
         delay(200); 
         
        }
      
      }
      
      delay(200);
    }
  }



void gjester (){  
  int n = 1;
  delay(400);
  while(screen == "undermeny5"){
    joystick();
    tft.setCursor(110,(menylinje -1) * linjeavstand);
    tft.fillRect(110,(menylinje -1) * linjeavstand,10, linjeavstand ,ST77XX_BLACK);
    tft.print(n);     
      if (hoyre == 1) { //Stikken til venstre går tilbake til forrige meny
        returnToMainmenu();
        break;
        }
      else if (opp == 1){            
         if (n > 1){ n -= 1;}            
          }
      else if (opp == 2){      
        if (n < 2 ){ n += 1;}          
          }
      else if (button == 1){       
        tft.setCursor(0 , 5 * linjeavstand);
        tft.print("Sender...");
        if (menylinje == 2){
          n = -n;
          }
        int venner = guestListModify(gjestSignal,n); 
       
        //Suksessskjerm
        tft.fillScreen(ST77XX_BLACK);
        tft.setCursor(0 , 0);
        if (menylinje == 1){             
             if (venner == 1){
              tft.println("Du har for");
              tft.println("mange venner.");
                }       
             else {       
               tft.println("Du har lagt til " + String(n)+ " gjest(er)");
                }
               delay(4000);
               returnToMainmenu();
               break;  
            }  
        if (menylinje == 2){         
        tft.println("Du har fjernet " + String(-n) + " gjest(er)");                
        delay(4000);
        returnToMainmenu();
        break;
           }          
        }  
     delay(200);
     }
}

void intro() { //Hentet fra eksempler i biblioteket
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_RED);
  uint16_t color = 100;
  int i;
  int t;
  for(t = 0 ; t <= 4; t+=1) {
    int x = 0;
    int y = 0;
    int w = tft.width()-2;
    int h = tft.height()-2;
    for(i = 0 ; i <= 16; i+=1) {
      tft.drawRoundRect(x, y, w, h, 5, color);
      x+=2;
      y+=3;
      w-=4;
      h-=6;
      color+=1100;
      delay(10);
    }
    color+=100;
  }
 tft.setCursor(48 , 60);
 tft.println("\x03" + String("Gruppe16") + "\x03");
 delay(2000); 
}
void default_lister(){ //Skriver default lister til CoT
  tft.setCursor(0 , 3 * linjeavstand);
  tft.println("Setter lister");
  circusESP32.write(badSignal,defaultList,token1); 
  circusESP32.write(kjokkenSignal,defaultList,token1);
  circusESP32.write(stueSignal,defaultList,token1);
  circusESP32.write(badTidSignal,defaultList,token1);
  circusESP32.write(kjokkenTidSignal,defaultList,token1);
  circusESP32.write(stueTidSignal,defaultList,token1);
  circusESP32.write(badDusjSignal,defaultList,token1);
  circusESP32.write(kjokkenOvnSignal,defaultList,token1);
}

void setup() {
  Serial.begin(115200);
  pinMode(tftPower , OUTPUT);
  digitalWrite(tftPower, HIGH);
  //Skjerm
  tft.initR(INITR_BLACKTAB);     // Initialiserer skjerm ST7735S 
  tft.fillScreen(ST77XX_BLACK);  //Skriver hele skjermen svart
  tft.setRotation(1);  //Roterer skjerm 90 grader ,(0-3)som gir totalt 360
  intro();  //Kjører oppstartsskjerm
  delay(500);
  tft.setCursor(0 , 0);
  tft.fillScreen(ST77XX_BLACK);  
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.println("Kobler til");
  tft.println("nettverk."); //Blir stuck hvis ikke nettverk er tilgjengelig
  circusESP32.begin();
  
  //default_lister(); //Kjører funksjon som setter default lister i CoT (uncomment/comment)
  
  tft.fillScreen(ST77XX_BLACK);
  hovedmeny(); //Tegner hovedmeny 
  //Pins til joystick
  pinMode(VRX , INPUT); 
  pinMode(VRY , INPUT);
  pinMode(SW , INPUT);
  attachInterrupt(digitalPinToInterrupt(VRX),joystick,CHANGE);
  attachInterrupt(digitalPinToInterrupt(VRY),joystick,CHANGE);
  attachInterrupt(digitalPinToInterrupt(SW),joystick,FALLING);
  
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_32, 0); 
  myTime = millis();
}

void loop() {
  joystick(); 
  if (screen == "hovedmeny" ||screen == "undermeny1" ||screen == "undermeny2" ||screen == "undermeny4"||screen == "undermeny5"  ){
    piltegner();
  }   
  if (button == 1) {                
      if (screen == "undermeny5"){
        gjester ();
        }     
      else if(screen == "undermeny4") {      
            if (menylinje == 1){      //Tar menylinja og bruker den til å gå inn på rikitg meny
                screen = "standardtemp"; //Sier hvor vi er til resten av programmet
                standardtemp(); //Kjører tegning av menyen                 
                } 
            else if (menylinje == 2){ 
                screen = "manuell";
                menylinje = 1;
                manuell();                
                }
            }
            
       else if (screen == "undermeny2" || screen == "undermeny1"){     
            tidsvalg();
             }        
       else if (screen == "hovedmeny"){   
            if (menylinje == 1){    
                  screen = "undermeny1"; 
                  undermeny1();  
                  } 
            else if (menylinje == 2) {                                          
                  screen = "undermeny2";
                  undermeny2();                 
                  }  
            else if (menylinje == 3){
                  screen = "undermeny3";
                  undermeny3();
                  }                               
            else if (menylinje == 4){
                  screen = "undermeny4";
                  undermeny4(); 
                  }                               
            else if (menylinje == 5){
                  screen = "undermeny5";
                  undermeny5();                                            
                  } 
                  menylinje = 1; //Sier at pila skal plasseres på det øverste elementet i ny meny   
              } 
        }
        delay(200); //Må være her pga ikke registre dobbelt tilbake
 if (hoyre == 1 && strstr(screen,"undermeny")){
      tft.fillScreen(ST77XX_BLACK);
      screen = "hovedmeny";
      hovedmeny();    
      }
      
 if ((millis() - myTime > 120 * 1000 )){//Millisekunder før den går i sleep mode. myTime får høyere verdi når joystick brukes--> Utsetter deepsleep.
      Serial.println("Time since last action: " + String(millis() - myTime));
      Serial.println("Going to sleep now");
      digitalWrite(tftPower, LOW);
      esp_deep_sleep_start();
      rtc_gpio_deinit(GPIO_NUM_32);
     } 
     
  int printe = 1;
  if (printe == 1){ //Koden under er for printe verdier for feilsøking
       Serial.println("Tid: " + String(millis() - myTime));
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
