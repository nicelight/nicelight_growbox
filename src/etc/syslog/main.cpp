#if c_syslog == 1
    #include <Syslog.h>
    
    WiFiUDP udpClient;
    Syslog syslog(udpClient, SYSLOG_SERVER, SYSLOG_PORT, HOSTNAME, Firmware, LOG_KERN);
#endif //c_syslog