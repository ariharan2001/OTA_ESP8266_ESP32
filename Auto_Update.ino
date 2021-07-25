//-----------------------------------------------------------------------------------------------------------------------
//------------------------------------------------HEADER FILES-----------------------------------------------------------

#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>
#include "time.h";

//-----------------------------------------------------------------------------------------------------------------------
//---------------------------------------------ROOT CA CERTIFICATION-----------------------------------------------------

#ifndef CERT_H
#define CERT_H
const char * rootCACertificate = \
  "-----BEGIN CERTIFICATE-----\n"
"MIIDxTCCAq2gAwIBAgIQAqxcJmoLQJuPC3nyrkYldzANBgkqhkiG9w0BAQUFADBs\n"
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
"d3cuZGlnaWNlcnQuY29tMSswKQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5j\n"
"ZSBFViBSb290IENBMB4XDTA2MTExMDAwMDAwMFoXDTMxMTExMDAwMDAwMFowbDEL\n"
"MAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcGA1UECxMQd3d3\n"
"LmRpZ2ljZXJ0LmNvbTErMCkGA1UEAxMiRGlnaUNlcnQgSGlnaCBBc3N1cmFuY2Ug\n"
"RVYgUm9vdCBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMbM5XPm\n"
"+9S75S0tMqbf5YE/yc0lSbZxKsPVlDRnogocsF9ppkCxxLeyj9CYpKlBWTrT3JTW\n"
"PNt0OKRKzE0lgvdKpVMSOO7zSW1xkX5jtqumX8OkhPhPYlG++MXs2ziS4wblCJEM\n"
"xChBVfvLWokVfnHoNb9Ncgk9vjo4UFt3MRuNs8ckRZqnrG0AFFoEt7oT61EKmEFB\n"
"Ik5lYYeBQVCmeVyJ3hlKV9Uu5l0cUyx+mM0aBhakaHPQNAQTXKFx01p8VdteZOE3\n"
"hzBWBOURtCmAEvF5OYiiAhF8J2a3iLd48soKqDirCmTCv2ZdlYTBoSUeh10aUAsg\n"
"EsxBu24LUTi4S8sCAwEAAaNjMGEwDgYDVR0PAQH/BAQDAgGGMA8GA1UdEwEB/wQF\n"
"MAMBAf8wHQYDVR0OBBYEFLE+w2kD+L9HAdSYJhoIAu9jZCvDMB8GA1UdIwQYMBaA\n"
"FLE+w2kD+L9HAdSYJhoIAu9jZCvDMA0GCSqGSIb3DQEBBQUAA4IBAQAcGgaX3Nec\n"
"nzyIZgYIVyHbIUf4KmeqvxgydkAQV8GK83rZEWWONfqe/EW1ntlMMUu4kehDLI6z\n"
"eM7b41N5cdblIZQB2lWHmiRk9opmzN6cN82oNLFpmyPInngiK3BD41VHMWEZ71jF\n"
"hS9OMPagMRYjyOfiZRYzy78aG6A9+MpeizGLYAiJLQwGXFK3xPkKmNEVX58Svnw2\n"
"Yzi9RKR/5CYrCsSXaQ3pjOLAEFe4yHYSkVXySGnYvCoCWw9E1CAx2/S6cCZdkGCe\n"
"vEsXCS+0yx5DaMkHJ8HSXPfqIbloEpw8nL+e/IBcm2PN7EeqJSdnoDfzAIJ9VNep\n"
"+OkuE6N36B9K\n"
"-----END CERTIFICATE-----\n";
#endif

//--------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------GLOBAL VARIABLES---------------------------------------------------------

const char * ssid = "ariharan";
const char * password = "ariharand";

volatile int cur_hour,cur_min;
volatile bool semaphore = false;

const long  gmtOffset_sec = 19800; // in INDIA, the gmtOffset_sec will be (5+0.5)*3600 = 19800
const int   daylightOffset_sec = 0; 

hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

const char* ntpServer = "pool.ntp.org";

String FirmwareVer = {
  "2.2"
};

//--------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------HTTPS--------------------------------------------------------------

#define URL_fw_Version "https://raw.githubusercontent.com/programmer131/ESP8266_ESP32_SelfUpdate/master/esp32_ota/bin_version.txt"
#define URL_fw_Bin "https://raw.githubusercontent.com/programmer131/ESP8266_ESP32_SelfUpdate/master/esp32_ota/fw.bin"

//--------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------FUNCTION DECLARATION------------------------------------------------------

void connect_wifi();
void localTime();
void firmwareUpdate();
int FirmwareVersionCheck();

//--------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------INTERUPT HANDLER---------------------------------------------------------

void IRAM_ATTR onTime() {
   portENTER_CRITICAL_ISR(&timerMux);
   cur_hour = (cur_hour+1)%24;
   if(cur_hour == 23) semaphore = true;
   portEXIT_CRITICAL_ISR(&timerMux);
}

//--------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------SETUP FUNCTION-----------------------------------------------------------

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  connect_wifi();
   // 80 MHZ quart and devide it by 8000 and totally ten thousand ticks per second 
   // 80000000 / 8000 = 10000 tics / second 
  timer = timerBegin(0, 8000, true);                
  timerAttachInterrupt(timer, &onTime, true);    
   // Sets an alarm to sound some required period
  timerAlarmWrite(timer, 10000 , true);     // 1 HOUR = 10000 * 60 * 60 = 10000 * 3600 = 36000000      
  timerAlarmEnable(timer);
  Serial.print("Active firmware version:");
  Serial.println(FirmwareVer);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  localTime(); // to get time and store them into cur_hour and cur_min
  if(cur_hour == 23) semaphore = true;
}

//--------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------LOOP FUNCTION-----------------------------------------------------------

void loop() {
   delay(1000);
   Serial.print("current hour : ");
   Serial.println(cur_hour);
   digitalWrite(LED_BUILTIN, LOW);
   if(semaphore)
   {
     digitalWrite(LED_BUILTIN, HIGH);
     Serial.println("Time to check for new update");
     if(FirmwareVersionCheck)
     {
       Serial.println("Update process started..."); 
       firmwareUpdate();        
     } 
     semaphore = false;
   }
   delay(1000);
}

//--------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------CONNECTING TO WIFI--------------------------------------------------------

void connect_wifi() {
  Serial.println("Waiting for WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("");
  delay(2000);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("WiFi connected");
}

//---------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------LOCAL TIME FROM NTK SERVER-------------------------------------------------

void localTime()
{
   struct tm timeinfo;
   if(!getLocalTime(&timeinfo)){
      Serial.println("Failed to obtain time");
      return;
   }
   cur_hour = timeinfo.tm_hour;
   cur_min = timeinfo.tm_min;
   Serial.print("current hour: "); 
   Serial.println(cur_hour);
   Serial.print("current minutes: ");
   Serial.println(cur_min);
}

//---------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------FIRMWARE VERSION CHECK----------------------------------------------------

int FirmwareVersionCheck(void) {
  String payload;
  int httpCode;
  String fwurl = "";
  fwurl += URL_fw_Version;
  fwurl += "?";
  fwurl += String(rand());
  Serial.println(fwurl);
  WiFiClientSecure * client = new WiFiClientSecure;

  if (client) 
  {
    client -> setCACert(rootCACertificate);
    
    // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is 
    HTTPClient https;

    if (https.begin( * client, fwurl)) 
    { // HTTPS      
      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      delay(100);
      httpCode = https.GET();
      delay(100);
      if (httpCode == HTTP_CODE_OK) // if version received
      {
        payload = https.getString(); // save received version
      } else {
        Serial.print("error in downloading version file:");
        Serial.println(httpCode);
      }
      https.end();
    }
    delete client;
  }
      
  if (httpCode == HTTP_CODE_OK) // if version received
  {
    payload.trim();
    if (payload.equals(FirmwareVer)) {
      Serial.printf("\nDevice already on latest firmware version:%s\n", FirmwareVer);
      return 0;
    } 
    else 
    {
      Serial.println(payload);
      Serial.println("New firmware detected");
      return 1;
    }
  } 
  return 0;  
}

//----------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------FIRMWARE UPDATE-----------------------------------------------------------

void firmwareUpdate(void) {
  WiFiClientSecure client;
  client.setCACert(rootCACertificate);
  httpUpdate.setLedPin(LED_BUILTIN, LOW);
  t_httpUpdate_return ret = httpUpdate.update(client, URL_fw_Bin);

  switch (ret) {
  case HTTP_UPDATE_FAILED:
    Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
    break;

  case HTTP_UPDATE_NO_UPDATES:
    Serial.println("HTTP_UPDATE_NO_UPDATES");
    break;

  case HTTP_UPDATE_OK:
    Serial.println("HTTP_UPDATE_OK");
    break;
  }
}

//-----------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------END--------------------------------------------------------------------
