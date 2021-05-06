#ifndef _MD_WEB_SERVER_H_
//#ifdef _MD_WEB_SERVER_H_
  #define _MD_WEB_SERVER_H_

  //#define USE_ASYNCWEBSERVER

  #include <Arduino.h>
  #include <time.h>
  //#include <Time.h>
  //#include <TimeLib.h>
  #include <WiFi.h>
  #include <WiFiUdp.h>
  #include <ESPmDNS.h>
  #include <md_defines.h>
  #include <md_util.h>
  #include <ip_list.hpp>

  #if defined (USE_ASYNCWEBSERVER)
    #include <AsyncTCP.h>
    //#include <ESPAsyncWebServer.h>
  #else
    #include <WebServer.h>
  #endif

  #define WIFI_OK   false
  #define WIFI_ERR  true
  #define LOGINTXT_MAX_LEN 14

  enum // locIP Status
    {
      LOCIP_FREE = 0x00,
      LOCIP_IP   = 0x01,
      LOCIP_GW   = 0x02,
      LOCIP_SN   = 0x04,
      LOCIP_OK   = LOCIP_IP | LOCIP_GW | LOCIP_SN
    };

  typedef char LoginTxt_t[LOGINTXT_MAX_LEN + 1];

  class md_localIP
    {
      public:
        md_localIP(){}
        ~md_localIP(){}
        bool       setIP(uint32_t ip);
        bool       setGW(uint32_t ip);
        IPAddress  getIP()  { return _IP; }
        IPAddress  getGW()  { return _GW; }
        IPAddress  getSN()  { return _SN; }
        uint8_t    status() { return ((uint8_t) _stat); }

      protected:
        uint8_t    _stat = LOCIP_FREE;
        IPAddress  _IP;
        IPAddress  _GW;
        IPAddress  _SN;
    };

  class md_NTPTime
    {
      public:
        md_NTPTime(){}
        ~md_NTPTime(){}
        bool getTime    (time_t *ntpEpoche );
        bool initNTPTime(uint8_t summer);
      protected:
        uint64_t sendNTPpacket(IPAddress& address);
        uint8_t  _timezone = 0;
    };

  class md_wifi: public md_localIP, public md_NTPTime
    {
      private:
        IPAddress   _gateip; //IPAddress
        IPAddress   _subnet; //IPAddress
        IPAddress   _locip;  //IPAddress
        LoginTxt_t  _ssid;
        LoginTxt_t  _passw;
        uint64_t    _conn_delay = 1500000ul;
        uint8_t     _conn_rep   = 5;

      public:
        md_wifi();
        ~md_wifi(){}
        bool scanWIFI  (ip_list* plist);
        bool startWIFI (bool _useLocID = FALSE);
        bool getNTPTime(time_t *ntpEpoche) { return getTime(ntpEpoche); }
        bool initNTP   (uint8_t summer)    { return initNTPTime(summer); }

      protected:
        void _debugConn(bool _wifi = FALSE);
    };

  class md_server
    {
      public:
        bool md_startServer();
        bool md_handleClient();

      protected:
        String       _header;          // store HTTP-request
    };

//extern md_wifi   wifi;
//extern md_server webMD;

#endif