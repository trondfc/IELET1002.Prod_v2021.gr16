/*
  	CircusESP32Lib.h  (Version 1.0.0)

	Implements the circusofthings.com API in ESP32 boards.

  	Created by Jaume Miralles Isern, April 10, 2019.
*/


#ifndef CircusESP32Lib_h
#define CircusESP32Lib_h

#include "Arduino.h"
#include "WiFi.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "array_conversion.h"  

 
class CircusESP32Lib
{
  	public:
		CircusESP32Lib(char *server, char *ssid, char *pass);
		void begin();
		void write(char *key, double value, char *token);
		void write_array(char *key, int array[], int size_of_array, char *token, char size_of_index[]);
        double read(char *key, char *token);
        int * read_array(char number_array[], char *key, char *token);
	private:
		void getCertificate();
		char* parseServerResponse(char *r, char *label, int offset);
                char* waitResponse(int timeout, WiFiClientSecure *client);
		int count(char *text);
		char _charBuf_rootCA[1250];
		char *_server;
		char *_ssid;
                char *_pass;
};

#endif
