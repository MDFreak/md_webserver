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
    #include <functional>

    #include <AsyncTCP.h>
    #include <ESPAsyncWebServer.h>
    #include <ArduinoJSON.h>
    #include <SPIFFS.h>

  // --- declarations
    #define WIFI_OK   false
    #define WIFI_ERR  true

    using fptr = std::function<void()>;

    enum // locIP Status
      {
        LOCIP_FREE = 0x00,
        LOCIP_IP   = 0x01,
        LOCIP_GW   = 0x02,
        LOCIP_SN   = 0x04,
        LOCIP_OK   = LOCIP_IP | LOCIP_GW | LOCIP_SN
      };

    typedef char LoginTxt_t[LOGINTXT_MAX_LEN + 1];

    enum dataFormat
      {
        MD_SINGLE  =  'S', // [typeElement][idx][value]
        MD_MULTI   =  'M', // [typeElement][idx][value],[typeElement][idx][value],...
        MD_TEXT    =  'T'  // [text value]
      };

    enum typeMessage
      {
        ME_TSOCKET =  0,
        ME_TVAL    = 'V',
        ME_TCONN   = 'C',
        ME_TREQ    = 'R'
      };

  // element of webserver
    enum typeElement
      {
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

    /* --- md_message fromat description ---
     *  _client:  client of webserver
     *  _tMsg:      + type of data (S=single, M=multi, T=text)
     *  _tData:     |+ type of message 1 char ( V=value, R=request)
     *  _payload_S: xxx
     *              ||+ value
     *              |+ index
     *              + type of element
     *  _payload_M: xxx
     *              ||+ ?
     *              |+ ?
     *              + ?
     *  _payload_T: <text>
     */
    class md_message : public md_cell
      {
        private:
          //void*   _msg     = NULL;
          uint8_t _client  = 0;
          char    _tMsg    = 0;  // ENUM typeMessage
          char    _tData   = MD_SINGLE;
          String  _payload = "";

        public:
          md_message()   { }
          md_message(uint8_t client, char msgType, char* payload)
                         { _client = client; _tMsg = msgType; _payload = payload; }
          ~md_message()  { }

          void*   pMsg   ()                 { return this;         }
          void    client (uint8_t client)   { _client  = client;   }
          void    msgType(uint8_t tMsg)     { _tMsg    = tMsg;     }
          void    dataType(uint8_t tData)   { _tData   = tData;    }
          void    payload(char*   ppayload) { _payload = ppayload; }
          uint8_t client () const           { return _client;      }
          char    msgType() const           { return _tMsg;        }
          char    dataType()                { return _tData;       }
          char*   payload()                 { return _payload.begin() ;}
          //void    valType(uint8_t tVal)     { _tVal    = tVal;     }
          //char    valType() const           { return _tVal;        }
          //void    index  (char    idx)      { _idx     = idx;      }
          //char    index  () const           { return _idx;         }
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
        extern md_msglist*inMsgs;  // (FIFO-) buffer for message requests
        extern md_msglist*outMsgs;  // (FIFO-) buffer for message requests
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
          bool    isRequest = false;
          bool    md_startServer();
          uint8_t createElement(uint8_t type, String name, String unit = "");
          void    initWebSocket();
          void    initSPIFFS();
          uint8_t getDutyCycle(uint8_t idx = 1);
          void    updateAll(const String data);
          void    handleClient(AwsEventType type, AsyncWebSocketClient *client,
                               void *arg, uint8_t *data, size_t len);

        protected:
          String  _header;   // store HTTP-request
                             //void    createDefElements(uint8_t switches, uint8_t pwms, uint8_t analogs);
      };
    extern md_server* pmdServ;

#endif