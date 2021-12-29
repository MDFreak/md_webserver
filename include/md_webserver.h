#ifndef _MD_WEB_SERVER_H_
//#ifdef _MD_WEB_SERVER_H_
  #define _MD_WEB_SERVER_H_

  // --- includes
    #include <Arduino.h>
    #include <stdlib.h>
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

  // --- declarations
    #define WIFI_OK   false
    #define WIFI_ERR  true

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
        EL_TSOCKET = 0,
        EL_TFIRST  = 'A',
        EL_TANALOG = 'A', // A = 65
        EL_TSLIDER,       // B = 66
        EL_TCOLOR,        // C
        EL_TSWITCH,       // D
        EL_TTEXT,         // E
        EL_TOFFSET,       // F
        EL_TGRAPH,        // G
        EL_TPIC,          // H
        EL_TINDEX,        // I
        EL_TMAX,
        EL_TCNT    = EL_TMAX - EL_TFIRST
      };

    typedef struct mdMsg_struct
      {
        uint8_t client  = 0;
        char    type    = EL_TSOCKET;
        String  payload = "";
      } mdMSG_t;

    class md_slider : public md_cell
      {
        public:
          uint16_t destVal = 0;
          uint16_t realVal = 0;
          bool     srvSem  = OBJFREE;
          bool     cliSem  = OBJFREE;
          String   name;
          String   unit;
      };

    class md_switch : public md_cell
      {
        public:
          bool     destVal = OFF;
          bool     realVal = OFF;
          bool     srvSem  = OBJFREE;
          bool     cliSem  = OBJFREE;
          String   name;
      };

    class md_analog : public md_cell
      {
        public:
          double   destVal = 0.0;
          double   realVal = 0.0;
          bool     srvSem  = OBJFREE;
          bool     cliSem  = OBJFREE;
          String   name;
          String   unit;
      };

    class md_message : public md_cell
      {
        public:
          mdMSG_t* pMsg = new mdMSG_t;

          ~md_message() { if (pMsg != NULL) delete pMsg; }
      };

    class md_msglist : public md_list
      {
        public:
          ~md_msglist() { clear(); }

          void clear();

          bool srvSem  = OBJFREE;
          bool hostSem = OBJFREE;
      };

    #ifndef MESSAGE_LIST
        extern md_msglist* msgList;  // (FIFO-) buffer for message requests
        #define MESSAGE_LIST
      #endif
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
          bool getTime    (time_t *ntpEpoche);
          bool initNTPTime();
        protected:
          uint64_t sendNTPpacket(IPAddress& address);
          void     checkSummer();
          uint8_t  _timezone   = 1;
          int16_t  _seasontime = NN;
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
          bool    scanWIFI  (ip_list* plist);
          uint8_t startWIFI ();
          bool    getNTPTime(time_t *ntpEpoche) { return getTime(ntpEpoche); }
          bool    initNTP   (              )    { return initNTPTime(); }

        protected:
          void _debugConn(bool _wifi = FALSE);
      };

    class md_server
      {
        public:
          bool    md_startServer();
          uint8_t createElement(uint8_t type, String name, String unit = "");
          void    handleClient(AwsEventType type, AsyncWebSocketClient *client,
                               void *arg, uint8_t *data, size_t len);
          void    initWebSocket();
          void    initSPIFFS();
          uint8_t getDutyCycle(uint8_t idx = 1);

        protected:
          void    createDefElements(uint8_t switches, uint8_t pwms, uint8_t analogs);
          String  _header;   // store HTTP-request
      };


#endif