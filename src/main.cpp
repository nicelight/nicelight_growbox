
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WebServer.h>
WebServer server(80);

#include <WiFiClient.h>
#include <HTTPClient.h>
#include "GyverFilters.h"
GMedian<7, int> PhMediana;    
GABfilter PhGAB(0.001, 150, 1);


#include <pre.h>

#include <func>

// Переменные
float AirTemp, AirHum, RootTemp, CO2, tVOC,hall,pHmV,pHraw,NTC,Ap,An,Dist;
TaskHandle_t TaskAHT10Handler;

#define HOSTNAME "WEGABOX" // Имя системы и DDNS .local
#define ONE_WIRE_BUS 23    // Порт 1-Wire
#include <Wire.h>          // Шина I2C
#define I2C_SDA 21         // SDA
#define I2C_SCL 22         // SCL


// Подключенные устройства
#define c_DS18B20 1 
#define c_AHT10 1
#define c_AM2320 0
#define c_CCS811 1
#define c_hall 1
#define c_MCP3421 1
#define c_ADS1115 0
#define c_NTC 1
#define c_EC 1
#define c_US025 1


#if c_DS18B20 == 1
  #include <OneWire.h>
  #include <DallasTemperature.h>
  OneWire oneWire(ONE_WIRE_BUS);
  DallasTemperature sens18b20(&oneWire);
#endif

#if c_AHT10 == 1
  #include <AHT10.h>
  AHT10 myAHT10(AHT10_ADDRESS_0X38);
#endif


#if c_CCS811 == 1
  #include "ccs811.h"  // CCS811 library
  CCS811 ccs811;
#endif

#if c_MCP3421 == 1
  #include <MCP342x.h>
  uint8_t address = 0x68;
  MCP342x adc = MCP342x(address);
#endif

#if c_ADS1115 == 1
  #include<ADS1115_WE.h> 
  #define I2C_ADDRESS 0x48
  ADS1115_WE adc = ADS1115_WE(I2C_ADDRESS);
#endif

#if c_NTC == 1
 #define NTC_port 32
 #define NTC_MiddleCount 10000
#endif

#if c_EC == 1
  #define EC_DigitalPort1 18
  #define EC_DigitalPort2 19
  #define EC_AnalogPort 33
  #define EC_MiddleCount 50000
#endif

#if c_US025 == 1
// Level ultrasound (echo, trig, temp, averaging counter) > cm 
//dst=us(13,14,25,60);
  #define US_ECHO 13
  #define US_TRIG 14
#endif // c_US025



void handleRoot() {
  String httpstr="<meta http-equiv='refresh' content='10'>";
       httpstr += "HOSTNAME=" + String(HOSTNAME) + "<br>";
       
       if(RootTemp) { httpstr +=  "RootTemp=" + fFTS(RootTemp,3) + "<br>"; }
       if(AirTemp)  { httpstr +=  "AirTemp=" +  fFTS(AirTemp,3) + "<br>"; }
       if(AirHum)   { httpstr +=  "AirHum=" +   fFTS(AirHum,3) + "<br>"; }
       if(CO2)   { httpstr +=  "CO2=" +   fFTS(CO2,0) + "<br>"; }
       if(tVOC)   { httpstr +=  "tVOC=" +   fFTS(tVOC,0) + "<br>"; }
       if(hall)   { httpstr +=  "hall=" +   fFTS(hall,3) + "<br>"; }     
       if(pHmV)   { httpstr +=  "pHmV=" +   fFTS(pHmV,3) + "<br>"; }
       if(NTC)   { httpstr +=  "NTC=" +   fFTS(NTC,3) + "<br>"; }
       if(Ap)   { httpstr +=  "Ap=" +   fFTS(Ap,3) + "<br>"; }
       if(An)   { httpstr +=  "An=" +   fFTS(An,3) + "<br>"; }
       if(Dist)   { httpstr +=  "Dist=" +   fFTS(Dist,3) + "<br>"; }

  server.send(200, "text/html",  httpstr);
  }

void TaskOTA(void * parameters){
  for(;;){
    server.handleClient();
    ArduinoOTA.handle();
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

void TaskWegaApi(void * parameters){
  for(;;){
    // Sending to WEGA-API 
    WiFiClient client;
    HTTPClient http;

    String httpstr=wegaapi;
    httpstr +=  "?db=" + wegadb;
    httpstr +=  "&auth=" + wegaauth;
    httpstr +=  "&uptime=" +fFTS(millis()/1000, 0);
    if(RootTemp) httpstr +=  "&RootTemp=" + fFTS(RootTemp,3);
    if(AirTemp) httpstr +=  "&AirTemp=" +fFTS(AirTemp, 3);
    if(AirHum) httpstr +=  "&AirHum=" +fFTS(AirHum, 3);
    if(hall) httpstr +=  "&hall=" +fFTS(hall, 3);
    if(pHmV) httpstr +=  "&pHmV=" +fFTS(pHmV, 4);
    if(pHraw) httpstr +=  "&pHraw=" +fFTS(pHraw, 4);
    if(CO2) httpstr +=  "&CO2=" +fFTS(CO2, 0);
    if(tVOC) httpstr +=  "&tVOC=" +fFTS(tVOC, 0);
    if(NTC) httpstr +=  "&NTC=" +fFTS(NTC, 3);
    if(Ap) httpstr +=  "&Ap=" +fFTS(Ap, 3);
    if(An) httpstr +=  "&An=" +fFTS(An, 3);
    if(Dist) httpstr +=  "&Dist=" +fFTS(Dist, 3);

    http.begin(client, httpstr);
    http.GET();
    http.end();

      if (WiFi.status() != WL_CONNECTED) {
        WiFi.disconnect(true);
        WiFi.begin(ssid, password);  }
    ArduinoOTA.handle();   
    delay (60000); // Периодичность отправки данных в базу (в мс)
  }
  
}

void TaskEC(void * parameters){
  for(;;){
  
  #if c_EC == 1
    float Ap0 = 0;
    float An0 = 0;
    double eccount = 0;

    #if c_NTC == 1
      float NTC0=0;
    #endif // c_NTC 

    pinMode(EC_AnalogPort, INPUT);
    pinMode(NTC_port, INPUT);
    pinMode(EC_DigitalPort1, OUTPUT);
    pinMode(EC_DigitalPort2, OUTPUT);

    while (eccount < EC_MiddleCount){
      eccount++;
      digitalWrite(EC_DigitalPort1, HIGH);
      Ap0 = analogRead(EC_AnalogPort) + Ap0;
      digitalWrite(EC_DigitalPort1, LOW);
      
      digitalWrite(EC_DigitalPort2, HIGH);
      An0 = analogRead(EC_AnalogPort) + An0;
      digitalWrite(EC_DigitalPort2, LOW);

    ArduinoOTA.handle();
    vTaskDelay(1 / portTICK_PERIOD_MS); 

    #if c_NTC == 1
      NTC0=analogRead(NTC_port)+NTC0;
    #endif // c_NTC 

    }
    
    pinMode(EC_DigitalPort1, INPUT);
    pinMode(EC_DigitalPort2, INPUT);
    pinMode(EC_AnalogPort, INPUT);

    Ap = Ap0 / eccount;
    An = An0 / eccount;

    #if c_NTC == 1
      NTC = NTC0 / eccount;
    #endif // c_NTC

    vTaskDelay(200 / portTICK_PERIOD_MS);
    server.handleClient();
  #endif // c_EC
  }

}

void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(5000);
    ESP.restart();
  }

  ArduinoOTA.setHostname( HOSTNAME );
  ArduinoOTA.begin();
  MDNS.begin( HOSTNAME );
  MDNS.addService("http", "tcp", 80);
  server.on("/",handleRoot);
  server.begin();

  #if c_DS18B20 == 1
      sens18b20.begin();
      sens18b20.setResolution(12);
  #endif

  #if c_AHT10 == 1
  Wire.begin(I2C_SDA, I2C_SCL);
  #endif

  #if c_CCS811 == 1
    // Enable CCS811
  ccs811.set_i2cdelay(50); 
  ccs811.begin();
  ccs811.start(CCS811_MODE_1SEC);
  #endif

  #if c_MCP3421 == 1
    MCP342x::generalCallReset();
    delay(1); // MC342x needs 300us to settle
      // Check device present
    Wire.requestFrom(address, (uint8_t)1);
    if (!Wire.available()) {
      Serial.print("No device found at address ");
      Serial.println(address, HEX);
      while (1);
   }
    // adc.configure(config);
    // startConversion = true;
  #endif

 
  server.handleClient();
  ArduinoOTA.handle();

  xTaskCreate(TaskOTA,"TaskOTA",10000,NULL,3,NULL);
  xTaskCreate(TaskWegaApi,"TaskWegaApi",10000,NULL,1,NULL);
  xTaskCreate(TaskEC,"TaskEC",10000,NULL,4,NULL);

  
}

void loop() {


  #if c_DS18B20 == 1
    sens18b20.begin();
    sens18b20.requestTemperatures();
    float ds0=sens18b20.getTempCByIndex(0);
    if(ds0 != -127 and ds0 !=85) RootTemp=ds0; 
  #endif

  #if c_CCS811 == 1
      // Read
    ccs811.set_envdata_Celsius_percRH(AirTemp,AirHum);
    uint16_t eco2, etvoc, errstat, raw;
    ccs811.read(&eco2,&etvoc,&errstat,&raw); 
    
    // Print measurement results based on status
    if( errstat==CCS811_ERRSTAT_OK ) { 
      CO2=eco2;
      tVOC=etvoc;
    } 
  #endif


  #if c_AHT10 == 1
    myAHT10.softReset();
    delay(100);
    myAHT10.begin();

    float AirTemp0=myAHT10.readTemperature();
    if(AirTemp0 != 255) AirTemp=AirTemp0;
    
    float AirHum0=myAHT10.readHumidity();
    if(AirHum0 != 255) AirHum=AirHum0;
  #endif

  #if c_hall == 1
      long n=0;
      double sensorValue=0;
      while ( n< 10){
       n++;
       sensorValue = hallRead()+sensorValue;
       }
    hall=sensorValue/n;
  #endif

  #if c_MCP3421 == 1
    long value = 0;
    MCP342x::Config status;
    // Initiate a conversion; convertAndRead() will wait until it can be read
    uint8_t err = adc.convertAndRead(MCP342x::channel1, MCP342x::oneShot,MCP342x::resolution18, MCP342x::gain4,1000000, value, status);
    if (err) {
      Serial.print("Convert error: ");
      Serial.println(err);
    }
    else {
      pHraw=PhMediana.filtered(value);  // Медианная фильтрация удаляет резкие выбросы показаний
      if (millis() < 60000){            // Игнорит ошибку фильтра на старте системы первые 60 сек. 
        PhGAB.setParameters(1,1,1);
        PhGAB.filtered(pHraw);
        pHmV=4096/pow(2,18)*pHraw/1;
      }else{
        PhGAB.setParameters(0.001, 200, 1);
        pHmV=4096/pow(2,18)*PhGAB.filtered(pHraw)/4;
      }
    }
  #endif // c_MCP3421

  #if c_ADS1115 == 1
    adc.setCompareChannels(ADS1115_COMP_0_3);
    pHraw=PhMediana.filtered(adc.getResult_mV());
    if (millis() < 60000){
      PhGAB.setParameters(1,1,1);
      PhGAB.filtered(pHraw);
      pHmV=pHraw;
    }else{
      PhGAB.setParameters(0.001, 200, 1);
      pHmV=PhGAB.filtered(pHraw);
    }
  #endif // c_ADS1115

  #if c_US025 == 1
    Dist=us(US_ECHO,US_TRIG,25,60);
  #endif // c_US025


} // loop






