#if c_MCP23017 == 1

void TaskMCP23017(void *parameters)
{
  for (;;)
  {
    if (OtaStart == true)
      vTaskDelete(NULL);

    vTaskDelay(10000 / portTICK_PERIOD_MS);

    // Включаем насос циркуляции если свет горит и выключаем если нет
    // задаём свойства ШИМ-сигнала
    //const int freq = 300000;
    // if (PR > 1)
    // {

    //   if (pwd != 117)
    //   {
    //     mcp.pinMode(DRV1_A, OUTPUT);
    //     mcp.digitalWrite(DRV1_A, HIGH);
    //     pwd = 117;
    //     syslog_ng("PWD set:" + fFTS(pwd, 0));

    //     int freq = 300000;
    //     syslog_ng("PWD freq set:" + fFTS(freq, 0));

    //     // задаём свойства ШИМ-сигнала
    //     //const int freq = 45000;
    //     const int ledChannel = 0;
    //     const int resolution = 8;
    //     ledcSetup(ledChannel, freq, resolution);
    //     ledcAttachPin(PWD2, ledChannel);
    //     ledcWrite(ledChannel, 254);
    //     delay(1000);
    //     ledcWrite(ledChannel, pwd);
    //   }
    // }
    // else
    // {
    //   mcp.digitalWrite(DRV1_A, LOW);
    //   pwd = 0;
    // }
  }
}
#endif // c_MCP23017