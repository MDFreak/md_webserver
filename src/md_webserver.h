#ifndef _MD_WEB_SERVER_H_
  #define _MD_WEB_SERVER_H_

  #if defined(USE_WIFI) || defined(USE_WEBSEREVER)
      // --- includes
        #include <Arduino.h>
        #include <stdlib.h>
        #include <time.h>
        #include <md_time.h>
        #include <WiFi.h>
        #include <WiFiUdp.h>
        #include <ESPmDNS.h>
        #include <md_defines.h>
        #include <md_util.h>
        #include <md_ip_list.h>
        #include <functional>
        #include <md_wifi.h>
        #include <AsyncTCP.h>
        #include <ESPAsyncWebServer.h>
        #include <ArduinoJSON.h>
        #include <SPIFFS.h>
      // --- declarations
        using fptr = std::function<void()>;
        // --- messages
        typedef enum MD_MSG_format
          {
            MD_SINGLE  =  'S', // [typeElement][idx][value]
            MD_MULTI   =  'M', // [typeElement][idx][value],[typeElement][idx][value],...
            MD_TEXT    =  'T'  // [text value]
          } MD_MSG_format_t;
        typedef enum MD_MSG_type
          {
            ME_TSOCKET =  0,
            ME_TVAL    = 'V',
            ME_TCONN   = 'C',
            ME_TREQ    = 'R'
          } MD_MSG_type_t;
        // element of webserver
        typedef enum EL_WEBTYPE
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
          } EL_WEBTYPE_t;

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
    #endif // defined(USE_WIFI) || defined(USE_WEBSEREVER)
#endif // _MD_WEB_SERVER_H_