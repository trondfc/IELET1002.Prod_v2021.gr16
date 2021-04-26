/*
  	CircusESP32Lib.cpp  (Version 1.0.0)

	Implements the circusofthings.com API in ESP32 boards.

  	Created by Jaume Miralles Isern, April 10, 2019.
*/


#include "CircusESP32Lib.h"

CircusESP32Lib::CircusESP32Lib(char *server, char *ssid, char *pass)
{
	_server = server; 
        _ssid = ssid;
        _pass = pass;
    
}

void CircusESP32Lib::begin(){
	WiFi.begin(_ssid, _pass); // Connect to your Wifi with the paremeters you provided at the begining
    	while (WiFi.status() != WL_CONNECTED) {
		delay(1000);        
    	}
    	if ((WiFi.status() == WL_CONNECTED)) {
        	Serial.println("Connected to WiFi");
        	getCertificate(); // Don't bother for SSL certificates. This method will fetch it for you
    	}
}

void CircusESP32Lib::getCertificate() {
	HTTPClient http;
        http.begin("http://circusofthings.com:8021/rootCA.txt");
        int httpCode = http.GET();     
        if (httpCode > 0) { 
              	String str_root_ca = http.getString();
              	str_root_ca.toCharArray(_charBuf_rootCA, 1250);
              	Serial.println("We got certificate");
		//_charBuf_rootCA = charBuf_rootCA;
        } else {
            	Serial.println("Error on certificate request");
        }
        http.end();
}

bool CircusESP32Lib::write(char *key, double value, char *token) {
	WiFiClientSecure client;
  	client.setCACert(_charBuf_rootCA);
    	Serial.println("SSL connection attempt");
    	if (!client.connect(_server, 443)){
		Serial.println("Connection failed!");
		return(false);
    	} else {
        	Serial.println("Connected to server!");
        	char bufValue[30]; 
        	dtostrf(value,1,4,bufValue);
		char body[150];
        	for( int i = 0; i < sizeof(body);  ++i )
                	body[i] = (char)0;
        	strcat(body, "{\"Key\":\""); strcat(body, key);
        	strcat(body, "\",\"Value\":"); strcat(body, bufValue);
        	strcat(body, ",\"Token\":\""); strcat(body, token); strcat(body, "\"}");
        	String strBody(body);
        	client.println("PUT /WriteValue HTTP/1.0");
        	client.println("Host: www.circusofthings.com");
        	client.println("User-Agent: CircusESP32Lib-1.0.0");
        	client.println("Content-Type:application/json");
        	client.print("Content-Length: ");
        	client.println(strBody.length());
        	client.println();
        	client.println(body);

		Serial.print("Waiting for response ");
		while (!client.available()){
		    	delay(50); //
		    	Serial.print(".");
		}  
		Serial.println("[Circus]");
		while (client.available()) {
		    	char c = client.read();
		    	Serial.write(c);
		}
		if (!client.connected()) {
		    	Serial.println();
		    	Serial.println("Server disconnected");
		    	client.stop();
		}
		return(true);
    	}
}

bool CircusESP32Lib::write_array(char *key, int array[], int size_of_array, char *token,char size_of_index[]){
	CONVERT Convert(false); 
    uint64_t numb_out = Convert.to_int(array,size_of_array,size_of_index);
    bool out = write(key,numb_out,token);
	return(out);
}















double CircusESP32Lib::read(char *key, char *token) {
	WiFiClientSecure client;
  	client.setCACert(_charBuf_rootCA);
    	Serial.println("SSL connection attempt");
    	if (!client.connect(_server, 443)){
		Serial.println("Connection failed!");
		return(-1);
    	} else {
        	Serial.println("Connected to server!");
		char q[250]; // nou
    		sprintf_P(q, PSTR("GET /ReadValue?Key=%s&Token=%s HTTP/1.1"), key, token); // nou
        	//client.println("GET /ReadValue HTTP/1.0");
		client.println(q); // nou
        	client.println("Host: www.circusofthings.com");
        	client.println("User-Agent: CircusESP32Lib-1.0.0");
        	client.println("Content-Type:application/json");
        	client.println("Connection: close");
        	client.println();

		Serial.print("Waiting for response ");
		while (!client.available()){
		    	delay(50); //
		    	Serial.print(".");
		}  
		/*Serial.println("");
		Serial.println("[Circus]");
		while (client.available()) {			
		    	char c = client.read();
		    	Serial.write(c);
		}
		Serial.println("");*/
		if (!client.connected()) {
		    	Serial.println();
		    	Serial.println("Server disconnected");
		    	client.stop();
		}
		
		char *responsebody = waitResponse(5000,&client);
		if (responsebody!=(char)0) {
			char labelk[] = "Key";
			char *key = parseServerResponse(responsebody, labelk, 3);
			char labelm[] = "Message";
			char *message = parseServerResponse(responsebody, labelm, 3);
			char labelv[] = "\"Value";// Double quote because can be confused with LastValueIp
			char *value = parseServerResponse(responsebody, labelv, 2); 
			return(atof(value));
	    	} else {
			Serial.println("No connection");
	    	}
    	}
}

int * CircusESP32Lib::read_array(char number_array[], char *key, char *token){
    CONVERT Convert(false); 
    uint64_t numb_inn = read(key,token);
	if(numb_inn == -1){
		int error[] = {-1};
		return(error);
	}else{
    int *recived = Convert.i_to_array(numb_inn,number_array);
    return recived;
	}
}

char* CircusESP32Lib::waitResponse(int timeout, WiFiClientSecure *client) {
	static char responsebody[200];
	for( int i = 0; i < sizeof(responsebody);  ++i )
        	responsebody[i] = (char)0;
	int j = 0;
	int pick = 0;
    	while (client->available()) {
                char c = client->read();
		Serial.write(c);
                //if(_debug>1) {printer->print(c);}
                if (c=='{') {pick=1;}
                if(pick){responsebody[j]=c;j++;}
                if (c=='}') {responsebody[j]='\0';pick=0;}                        
	}
	Serial.println("");
    	return responsebody;
}

char* CircusESP32Lib::parseServerResponse(char *r, char *label, int offset) {
    int labelsize = count(label);
//Serial.println(labelsize);
    char *ini = strstr(r,label) + labelsize + offset;
    static char content[100];
    int i=0;
    while (*ini!='\0'){ // may truncated occur
	//Serial.println();
	//Serial.print(*ini);
	//Serial.println();
        if((*ini!='\"')&&(*ini!=','))
            content[i]=*ini;
        else {
            content[i]='\0';
		//Serial.println(label);
		//Serial.println(content);
            return content;
        }
        i++; ini++;
    }
	
    return (char)0;
}

int CircusESP32Lib::count(char *text) {
    for(int i=0;i<300;i++){
        if(text[i]=='\0'){
            return(i);
        }
    }
    return -1;
}


