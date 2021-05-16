#ifndef _MD_WEB_SERVER_H_
//#ifdef _MD_WEB_SERVER_H_
  #define _MD_WEB_SERVER_H_

  #include <Arduino.h>
  #include <time.h>
  //#include <Time.h>
  #include <md_time.hpp>
  #include <WiFi.h>
  #include <WiFiUdp.h>
  #include <ESPmDNS.h>
  #include <md_defines.h>
  #include <md_util.h>
  #include <ip_list.hpp>

  #include <AsyncTCP.h>
  #include <ESPAsyncWebServer.h>
  #include "SPIFFS.h"

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

  // element of webserver
    enum typeElement
      {
        WS_TYPE_OFFSET = 65,
        WS_TYPE_SLIDER = 65,    // 'A'
        WS_TYPE_SWITCH,
        WS_TYPE_ANALOG,
        WS_TYPE_GRAPH,
        WS_TYPE_PIC,
        WS_TYPE_ANZ
      };

    class md_slider : public md_cell
      {
        public:
          uint16_t val = 0;
          String*  pName;
      };

    class md_switch : public md_cell
      {
        public:
          bool     state = OFF;
          String*  pName;
      };

    class md_analog : public md_cell
      {
        public:
          double   val;
          String*  pName;
      };


  // classes for netservice
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
          bool    md_startServer(uint8_t switches = 0, uint8_t pwms = 0, uint8_t analogs = 0);
          bool    md_handleClient();
          void    initWebSocket();
          void    initSPIFFS();
          uint8_t getDutyCycle(uint8_t idx = 1);

        protected:
          String   _header;   // store HTTP-request
          md_list* _pswList;   // list of switches
          md_list* _pslidList; // list of sliders
          md_list* _panaList;  // list of analog values
      };


#endif