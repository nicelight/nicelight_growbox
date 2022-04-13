// В этом файле заданы начальные параметры применяющиеся при включении устройство разово

void setup()
{

#if c_EC == 1
  pinMode(EC_DigitalPort1, INPUT);
  pinMode(EC_DigitalPort2, INPUT);
  pinMode(EC_AnalogPort, INPUT);
#endif // c_EC

#if c_NTC == 1
  pinMode(NTC_port, INPUT);
#endif // c_NTC

  Serial.begin(9600);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    delay(5000);
    ESP.restart();
  }

  ArduinoOTA.setHostname(HOSTNAME);

  ArduinoOTA
      .onStart([]()
               {
                 OtaStart = true;
                 // for (uint8_t i = 0; i < appTaskCount; ++i) {
                 //   vTaskDelete(appTasks[i]);
                 //   syslog.log(LOG_INFO, "appTask" + fFTS(i,0) + " sec OTA");
                 //   //vTaskDelay(appTasks[i]);
                 // }

                 syslog_ng("OTA Start");

#if c_EC == 1
                 pinMode(EC_DigitalPort1, INPUT);
                 pinMode(EC_DigitalPort2, INPUT);
                 pinMode(EC_AnalogPort, INPUT);
#endif // c_EC

                 String type;
                 if (ArduinoOTA.getCommand() == U_FLASH)
                   type = "sketch";
                 else // U_SPIFFS
                   type = "filesystem";

                 // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
                 Serial.println("Start updating " + type);
               })
      .onEnd([]()
             { Serial.println("\nEnd"); })
      .onProgress([](unsigned int progress, unsigned int total)
                  { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); })
      .onError([](ota_error_t error)
               {
                 Serial.printf("Error[%u]: ", error);
                 if (error == OTA_AUTH_ERROR)
                   Serial.println("Auth Failed");
                 else if (error == OTA_BEGIN_ERROR)
                   Serial.println("Begin Failed");
                 else if (error == OTA_CONNECT_ERROR)
                   Serial.println("Connect Failed");
                 else if (error == OTA_RECEIVE_ERROR)
                   Serial.println("Receive Failed");
                 else if (error == OTA_END_ERROR)
                   Serial.println("End Failed");
               });

  syslog_ng("WEGA Start");

#include <dev/lcd/setup.h>
  ArduinoOTA.begin();

  while (millis() < 30000)
    ArduinoOTA.handle(); // Ожидание возможности прошивки сразу после включения до запуска всего остального

  // Сканирование устройств на шине i2c
  Wire.begin(I2C_SDA, I2C_SCL);

  I2CScanner scanner;
  scanner.Init();
  scanner.Scan();





  preferences.begin("settings", false); 

  MDNS.begin(HOSTNAME);
  MDNS.addService("http", "tcp", 80);
  server.on("/", handleRoot);
  server.on("/reset", handleReset);
  server.on("/status", handleStatus);
  server.on("/pwd", handlePWD);
  server.on("/settings", handleSettings);
  server.begin();

  xTaskCreate(TaskOTA, "TaskOTA", 5000, NULL, 3, NULL);
  xTaskCreate(TaskWegaApi, "TaskWegaApi", 5000, NULL, 1, &appTasks[appTaskCount++]);

  xSemaphoreX = xSemaphoreCreateMutex();

  adc1_config_width(ADC_WIDTH_BIT_12);

#include <dev/ntc/setup.h>
#include <dev/ds18b20/setup.h>
#include <dev/aht10/setup.h>
#include <dev/us025/setup.h>
#include <dev/ccs811/setup.h>
#include <dev/am2320/setup.h>
#include <dev/mcp3421/setup.h>
#include <dev/bmp280/setup.h>
#include <dev/mcp23017/setup.h>
#include <dev/hx710b/setup.h>
#include <dev/ads1115/setup.h>
#include <dev/pr/setup.h>
#include <dev/ec/setup.h>
#include <dev/DualBMx/setup.h>
#include <dev/sdc30/setup.h>
#include <dev/cput/setup.h>

#if c_hall == 1
  xTaskCreate(TaskHall, "TaskHall", 10000, NULL, 0, NULL);
#endif // c_hall



#if c_LCD == 1
  oled.clear();
  //  oled.autoPrintln(true);
  //  oled.setScale(1);
  oled.update();

  //xTaskCreate(TaskLCD,"TaskLCD",20000,NULL,0,NULL);

#endif // c_LCD
}
