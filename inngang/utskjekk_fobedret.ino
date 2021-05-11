// importerer biblioteker
#include <SPI.h>
#include <RFID.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_BusIO_Register.h>
#include <CircusESP32Lib.h>

// kobler til internett og CoT
char ssid[] = "HUAWEI"; // nettverk 
char password[] = "l1tag0..";  // passord
char server[] = "www.circusofthings.com"; // nettside, CoT
// nøkkler til de ulike signalende
char key1[] = "3410";
char key2[] = "13546";
char key3[] = "24546";
char key4[] = "8253";
char key5[] = "21039";
char key6[] = "20365";
char token[] = "eyJhbGciOiJIUzI1NiJ9.eyJqdGkiOiI0OTEwIn0.5SXKcTgsyeYGALKiUW68wQ-UA7wARGUy4dhXrjcXE-g"; // token 

CircusESP32Lib circusESP32(server, ssid, password);  // kobler til CoT
 
//pinner for skjerm
#define TFT_CS 5 // Cs
#define TFT_RST 4 // Reset
#define TFT_DC 17 // A0
#define TFT_MOSI 26 // SDA
#define TFT_SCLK 16 // SCK
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

//farger til displayet
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

// pinner for kortleser
#define SS_PIN 21 //SDA
#define RST_PIN 22 // Reset


//Kobler til kortleseren
RFID rfid(SS_PIN, RST_PIN);

// diverse variabler
int power = 32;
int led = 25;
int serNum[5];
int type = 0;
int counter;
#define num_cards 6

// kode til gyldige kort
char cards[][5] = {
  {68, 236, 219, 49, 66},
  {34, 53, 137, 23, 137},
  {214, 60, 225, 31, 20},
  {211, 77, 23, 254, 119},
  {131, 19, 246, 26, 124},
  {115, 111, 171, 26, 173}
};

// navn på kort eiere
char name1[] = {'A', 's', 't', 'r', 'i', 'd', '\0'};
char name2[] = {'T', 'a', 'l', 'h', 'a', '\0'};
char name3[] = {'L', 'a', 'u', 'r', 'i', 't', 'z', ' ', '\0'};
char name4[] = {'V', 'i', 'l', 'm', 'a', '\0'};
char name5[] = {'T', 'r', 'o', 'n', 'd', '\0'};
char name6[] = {'J', 'o', 'a', 'k', 'i','m', '\0'};


//pointer
char* names[] = {name1, name2, name3, name4, name5, name6};

// brukes til å skrive riktig antal bokstaver per navn
byte name_flag = 0;
byte nameEndFlag = 0;


void setup() {
  circusESP32.begin();
  Serial.begin(115200);
  SPI.begin();
  rfid.init();

  // setter farg, plasering, skriftfarge og skrivstørelse for skjermen
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(BLUE);
  tft.setRotation(3);
  tft.setTextColor(BLACK);
  tft.setTextSize(1);
  tft.setCursor(0, 0);


  // definerer pinMOde
  pinMode(led, OUTPUT);
  pinMode (power, OUTPUT);
  digitalWrite(led, LOW);
  digitalWrite (power, LOW);
}

void loop() {

  if (rfid.isCard()) {
    // print kort nummere i serial monitor
    if (rfid.readCardSerial()) {
    }

    // kjører funksjonen med gyldige kort og skanned kort og returner true om det samsvarer
    char* card_now = (char*)rfid.serNum;
    int flag = compare_nested_array(cards, card_now);
    
   
    // om gyldig kort, print "Good bye, "NAVN", you are a legend!!"
    if (flag >= 0) {
      tft.print("Good bye ");
      for (int i = 0; i < sizeof(name3); i++) {
        if ((*(names[flag] + i) != '\0') && (nameEndFlag == 0)) {
          tft.print(*(names[flag] + i));
        }
        else {
          nameEndFlag++;
        }
      }
      tft.println(" You are a legend!! ");
      nameEndFlag = 0;
      delay(2000);
      tft.fillScreen(BLUE);
      



      //gyldig kort skrur på gul led for 0.5 sek
      digitalWrite(led, HIGH);
      delay(500);

      //gyldig kort, skrur på blå led, for 1 sek
      digitalWrite(power, HIGH);
      delay(1000);
      digitalWrite(power, LOW);
      digitalWrite(led, LOW);
      type = 0;


    }
    // Access denied
    else {

      //Ikke gyldig kort, skrur på led i 0.5 sek

      tft.println("Not allowed!");
      delay(1500);
      tft.fillScreen(BLUE);
      digitalWrite(led, HIGH);
      delay(500);
      digitalWrite(led, LOW);
      delay(500);
      digitalWrite(led, HIGH);
      delay(500);
      digitalWrite(led, LOW);

    }

  
// sender beskjed til det unike signalet for hver beboer om hvem som drar
  if (flag == 0) {
    circusESP32.write(key1, 0, token);
  }
  if (flag == 1) {
    circusESP32.write(key2, 0, token);
  }
  if (flag == 2) {
    circusESP32.write(key3, 0, token);
  }
  if (flag == 3) {
    circusESP32.write(key4, 0, token);
  }
  if (flag == 4) {
    circusESP32.write(key5, 0, token);
  }
  if (flag == 5) {
    circusESP32.write(key5, 0, token);
    
  }}
  rfid.halt();
}

bool compare_array(char *a, char *b) {
  if (sizeof(a) == sizeof (b)) {
    for (int i = 0; i < sizeof(a); i++) {
      if (a[i] != b[i]) {
        return (false);
      }
    }
    return (true);
  }
  return (false);
}

int compare_nested_array(char a[num_cards][5], char *b) {
  bool flag = false;
  for (int i = 0; i < num_cards; i++) {
    flag = compare_array(a[i], b);
    if (flag == true) {
      return (i);
    }
  }
  return (-1);
}
