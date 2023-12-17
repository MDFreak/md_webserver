#if defined(USE_WIFI) || defined(USE_WEBSEREVER)

    #include <md_defines.h>
    #include <md_webserver.h>

    #define WIFI_DEBUG_MODE  CFG_DEBUG_NONE
    #ifndef WIFI_DEBUG_MODE
        #define WIFI_DEBUG_MODE  CFG_DEBUG_STARTUP
      #endif
    #ifndef WIFI_DEBUG_MODE
        #define WIFI_DEBUG_MODE  CFG_DEBUG_ACTIONS
      #endif

    // --- declaration
      #define WS_IDX_OFFSET   97   // 'a'
      // --- webserver
        AsyncWebServer webServ(80);   // Create AsyncWebServer object on port 80
        AsyncWebSocket ws("/ws");     // Create a WebSocket object
        // list of active elements on website
        md_list* psliderList = new md_list();
        md_list* pswitchList = new md_list();
        md_list* panalogList = new md_list();

        md_msglist* outMsgs  = new md_msglist();  // (FIFO-) buffer for message requests
        md_msglist* inMsgs   = new md_msglist();  // (FIFO-) buffer for message requests
    // --- tasks
      #ifdef UNUSED
          void msgHandlerTask(void* pvParameters)
            {
              #if (WIFI_DEBUG_MODE > CFG_DEBUG_NONE)
                  String taskMessage = "msHdl task on core ";
                  taskMessage = taskMessage + xPortGetCoreID();
                  SOUT(millis()); SOUT(" "); SOUTLN(taskMessage);
                #endif
              while(true)
                {
                  if (inMsgs->count() > 0)
                    {
                      int8_t  doAna = NN;
                        /*
                          server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
                            request->send(SPIFFS, "/index.html", "text/html",false, processor);
                          });
                        */
                    }
                  usleep(5000);
                }
            }
        #endif
    // --- callback functions
      // --- callback webserver -------------
        void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
                     void *arg, uint8_t *data, size_t len)
          {
            pmdServ->handleClient(type, client, arg, data, len);
          }
    // --- classes
      // ------ class md_server  --------------------------
        bool    md_server::md_startServer()
          {
            initSPIFFS();
            //initWebSocket();
            // Web Server Root URL
            webServ.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                        { request->send(SPIFFS, "/index.html", "text/html"); }
                      );
            webServ.serveStatic("/", SPIFFS, "/");
            ws.onEvent(onWsEvent);
            webServ.addHandler(&ws);
            // start server
            webServ.begin();
            pmdServ = this;
            return false;
              /*
                webServ.on("/", handleRoot);
                webServ.on("/test.svg", drawGraph);
                webServ.on("/inline", []()
                  {
                    webServ.send(200, "text/plain", "this works as well");
                  });
                webServ.onNotFound(handleNotFound);
                webServ.begin();
                #if (WIFI_DEBUG_MODE > CFG_DEBUG_NONE)
                     Serial.println("HTTP server started");
                  #endif
              */
          }
          #ifdef UNUSED
            uint8_t md_server::createElement(uint8_t type, String name, String unit)
              {
                uint8_t idx = 255;
                if      ( (psliderList != NULL) && (type == EL_TSLIDER) )
                  {
                    while ( (psliderList->mode() == OBJDEF) && (psliderList->count() > 0))
                      { // remove default elements if necessary
                        md_slider* ptmp = (md_slider*) psliderList->pLast();
                        psliderList->rem(OLAST);
                        delete ptmp;
                      }
                    psliderList->setmode(OBJUSER);
                    md_slider* ptmp = new md_slider();
                    ptmp->name = name;
                    ptmp->unit = unit;
                    idx        = ptmp->index();
                    psliderList->add(ptmp);
                  }
                else if ( (pswitchList != NULL) && (type == EL_TSWITCH) )
                  {
                    while ( (pswitchList->mode() == OBJDEF) && (pswitchList->count() > 0))
                      { // remove default elements if necessary
                        md_switch* ptmp = (md_switch*) pswitchList->pLast();
                        pswitchList->rem(OLAST);
                        delete ptmp;
                      }
                    pswitchList->setmode(OBJUSER);
                    md_switch* ptmp = new md_switch();
                    ptmp->name = name;
                    //ptmp->unit = unit;
                    idx        = ptmp->index();
                    pswitchList->add(ptmp);
                  }
                else if ( (panalogList != NULL) && (type == EL_TANALOG) )
                  {
                    while ( (panalogList->mode() == OBJDEF) && (panalogList->count() > 0))
                      { // remove default elements if necessary
                        md_analog* ptmp = (md_analog*) panalogList->pLast();
                        panalogList->rem(OLAST);
                        delete ptmp;
                      }
                    panalogList->setmode(OBJUSER);
                    md_analog* ptmp = new md_analog();
                    ptmp->name = name;
                    ptmp->unit = unit;
                    idx        = ptmp->index();
                    panalogList->add(ptmp);
                  }
                else
                  {}
                return idx;
              }

            void    md_server::initWebSocket()
              {
                SOUT(" initWebSocket ... ");
                ws.onEvent(onWsEvent);
                SOUT(" ready ");
              }
            #endif
        void    md_server::initSPIFFS()
          {
            if (!SPIFFS.begin(true))
              { Serial.println("SPIFFS mount ERROR "); }
            else
              { Serial.print("SPIFFS mounted "); }
          }
        uint8_t md_server::getDutyCycle(uint8_t idx)
          {
            md_slider* ptmp = (md_slider*) psliderList->pIndex(idx);
                    //SOUT(" getDutyCycle idx "); SOUT(idx);
                    //SOUT(" ptmp "); SOUTHEXLN((uint32_t) ptmp);
            if (ptmp != NULL)
              {    //SOUT(" val "); SOUTLN(ptmp->val);
                return ptmp->destVal;
              }
            else
              {    //SOUTLN();
                return 0;
              }
          }
        void    md_server::updateAll(const String data)
          {
            //SOUT(" md_server::updateAll '"); SOUT(data); SOUTLN("'");
            //if (outMsgs->count() > 0)
              //{
                void *pmsg = outMsgs->pFirst();
                ws.textAll(data);
              //}
          }
        void    md_server::handleClient(AwsEventType type, AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len)
          {
            AwsFrameInfo *info = (AwsFrameInfo*)arg;
            char txt[40];
            String* sTxt = new String(); //(char*) data;
            char myIn[len+1];

            md_message* message = new md_message();
              //message->pMsg = new mdMSG_t;
            message->client(client->id());
            switch (type)
              {
                case WS_EVT_PONG:
                  break;
                case WS_EVT_ERROR:
                  SOUTLN("svr_onEvent ERROR receive");
                  break;
                case WS_EVT_CONNECT:
                  message->msgType(ME_TCONN);
                  sprintf(txt, "CONN #%u %s", client->id(), client->remoteIP().toString().c_str());
                  break;
                case WS_EVT_DISCONNECT:
                  sprintf(txt, "lost #%u %s", client->id(), client->remoteIP().toString().c_str());
                    //SOUTLN(txt);
                    //*sTxt = txt;
                  break;
                case WS_EVT_DATA:
                  memcpy(txt, &data[2], len - 1);
                  SOUT(" WS_EVT_DATA "); SOUTLN(txt);
                  message->msgType(data[0]);
                  txt[len] = 0;
                  break;
                default:
                  break;
              }
            message->payload(txt);
            SOUT(" txt "); SOUT(message->payload());
            message->setobj(message);
            inMsgs->srvSem = OBJBUSY;
              //while (inMsgs->hostSem != OBJBUSY) { SOUTLN("busy"); usleep(10); }
            inMsgs->add(message);
            inMsgs->srvSem = false;
            SOUT(" msList.count "); SOUTLN(inMsgs->count());
            delete sTxt;
            #ifdef UNUSED
              if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
                {
                  data[len] = 0;
                  uint8_t elType  = txt[0];  // extract obj type
                  uint8_t index = txt[1] - WS_IDX_OFFSET;  // extract index
                  int16_t value = atoi(&txt[2]);
                            //SOUT(" Payload type "); SOUT(type);
                            //SOUT(" index "); SOUT(index); SOUT(" len "); SOUT(len);
                            //SOUT(" data '"); SOUT(&txt[2]); SOUT(" = "); SOUT(value);
                            //SOUT(" ledList cnt "); SOUTLN(psliderList->count());

                  if (elType == EL_TSLIDER)
                    {
                      md_slider* psl = (md_slider*) psliderList->pIndex(index);
                            //SOUT(" psl "); SOUTHEX((uint32_t) psl);
                      if (psl != NULL)
                        {
                          psl->destVal = value;
                          SOUT(" slider "); SOUT((index+1)); SOUT("="); SOUTLN(value);
                        }
                    }

                  else if (elType == EL_TSWITCH)
                    {
                      md_switch* psw = (md_switch*) pswitchList->pIndex(index);
                      while (psw != NULL)
                        {
                          psw->destVal = value; SOUT(" switch "); SOUTLN(value);
                        }
                    }

                  else if (elType == EL_TANALOG)
                    {
                      md_analog* pana = (md_analog*) panalogList->pIndex(index);
                      while (pana != NULL)
                        {
                          pana->destVal = value; SOUT(" analog "); SOUTLN(value);
                        }
                    }

                  else { }
                }
              #endif
          }
  #endif // defined(USE_WIFI) || defined(USE_WEBSEREVER)
      // ----- md_server private
      /*
          void drawGraph()
            {
              String out = "";
              char temp[100];
              out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";
              out += "<rect width=\"400\" height=\"150\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
              out += "<g stroke=\"black\">\n";
              int y = rand() % 130;
              for (int x = 10; x < 390; x += 10)
              {
                int y2 = rand() % 130;
                sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, 140 - y, x + 10, 140 - y2);
                out += temp;
                y = y2;
              }
              out += "</g>\n</svg>\n";

              webServ.send(200, "image/svg+xml", out);
            }

          void handleRoot()
            {
              #ifdef BOARD_LED
                digitalWrite(BOARD_LED, 1);
              #endif
              char temp[400];
              int sec = millis() / 1000;
              int min = sec / 60;
              int hr = min / 60;

              snprintf(temp, 400,

                "<html>\
                  <head>\
                    <meta http-equiv='refresh' content='5'/>\
                    <title>ESP32 Gruss an Mathi</title>\
                    <style>\
                      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
                    </style>\
                  </head>\
                  <body>\
                    <h1>Hallo Mathi vom ESP32!</h1>\
                    <p>Uptime: %02d:%02d:%02d</p>\
                    <img src=\"/test.svg\" />\
                  </body>\
                </html>",

                         hr, min % 60, sec % 60
                        );
              webServ.send(200, "text/html", temp);
              #ifdef BOARD_LED
                digitalWrite(BOARD_LED, 0);
              #endif
            }

          void handleNotFou
          nd()
            {
              #ifdef BOARD_LED
                digitalWrite(BOARD_LED, 1);
              #endif
              String message = "File Not Found\n\n";
              message += "URI: ";
              message += webServ.uri();
              message += "\nMethod: ";
              message += (webServ.method() == HTTP_GET) ? "GET" : "POST";
              message += "\nArguments: ";
              message += webServ.args();
              message += "\n";

              for (uint8_t i = 0; i < webServ.args(); i++)
              {
                message += " " + webServ.argName(i) + ": " + webServ.arg(i) + "\n";
              }

              webServ.send(404, "text/plain", message);
              Serial.println(message);
              #ifdef BOARD_LED
                digitalWrite(BOARD_LED, 0);
              #endif
            }
        */

