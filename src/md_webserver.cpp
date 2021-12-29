//#ifdef _MD_WEB_SERVER_H_

#include <md_webserver.h>

    #define WIFI_DEBUG_MODE  CFG_DEBUG_NONE
    #ifndef WIFI_DEBUG_MODE
        #define WIFI_DEBUG_MODE  CFG_DEBUG_STARTUP
      #endif
    #ifndef WIFI_DEBUG_MODE
        #define WIFI_DEBUG_MODE  CFG_DEBUG_ACTIONS
      #endif

// --- declaration
  #define WS_IDX_OFFSET 97   // 'a'
  // --- webserver
    AsyncWebServer webServ(80);   // Create AsyncWebServer object on port 80
    AsyncWebSocket ws("/ws");     // Create a WebSocket object
    //uint8_t dutyCycle[3] = {0,0,0};

    md_server*     pwebSrv = NULL;
    TaskHandle_t*  pmsgHdl = NULL;

    // list of active elements on website
    md_list* psliderList = new md_list();
    md_list* pswitchList = new md_list();
    md_list* panalogList = new md_list();

    md_msglist* msgList  = new md_msglist();  // (FIFO-) buffer for message requests
  // --- NTP server ---------------------
    /* time server links
       http://www.hullen.de/helmut/filebox/DCF77/ntpsrvr.html
       https://tf.nist.gov/tf-cgi/servers.cgi
      */
    unsigned int localPort = 2390;
    //IPAddress    timeServer(129, 6, 15, 28);   // NIO 18.11.2021! NIST, Gaithersburg, Maryland
    IPAddress    timeServer(134,130,4,17);     // RWTH Aachen
    const int    NTP_PACKET_SIZE = 48;
    byte         packetBuffer[NTP_PACKET_SIZE];
    WiFiUDP      udp;
//
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
              if (msgList->count() > 0)
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
//
// --- global functions
  #ifdef UNUSED
      void startMsgHandlerTask()
        {
          xTaskCreatePinnedToCore(
              msgHandlerTask, // TaskFunction_t pvTaskCode,
    					"msHandlerTask",	// const char * const pcName,
    					10000, // const uint32_t usStackDepth,
    					NULL, // void * const pvParameters,
    					0, // UBaseType_t uxPriority,
    					pmsgHdl, // TaskHandle_t * const pvCreatedTask,
    					0);  // const BaseType_t xCoreID);

          SOUTLN("msgHdl task created on core 0");
        }
    #endif
//
// --- callback functions
  //
  // --- callback WIFI ------------------
    void WiFiEvent(WiFiEvent_t event)
      {
        #if (DEBUG_MODE > CFG_DEBUG_NONE)
            Serial.printf("     [WiFi-event] event: %d\n", event);
          #endif
        switch (event)
          {
            case SYSTEM_EVENT_WIFI_READY:
                  #if (DEBUG_MODE > CFG_DEBUG_NONE)
                      Serial.println("WiFi interface ready");
                    #endif
                break;
            case SYSTEM_EVENT_SCAN_DONE:
                  #if (DEBUG_MODE > CFG_DEBUG_NONE)
                      //Serial.println("Completed scan for access points");
                    #endif
                break;
            case SYSTEM_EVENT_STA_START:
                  #if (DEBUG_MODE > CFG_DEBUG_NONE)
                      Serial.println("WiFi client started");
                    #endif
                break;
            case SYSTEM_EVENT_STA_STOP:
                  #if (DEBUG_MODE > CFG_DEBUG_NONE)
                      Serial.println("WiFi clients stopped");
                    #endif
                break;
            case SYSTEM_EVENT_STA_CONNECTED:
                  #if (DEBUG_MODE > CFG_DEBUG_NONE)
                        Serial.println("Connected to access point");
                    #endif
                break;
            case SYSTEM_EVENT_STA_DISCONNECTED:
                  #if (DEBUG_MODE > CFG_DEBUG_NONE)
                        //Serial.print("Disconnected from WiFi status = "); Serial.println(WiFi.status());
                    #endif
                break;
            case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
                  #if (DEBUG_MODE > CFG_DEBUG_NONE)
                        Serial.println("Authentication mode of access point has changed");
                    #endif
                break;
            case SYSTEM_EVENT_STA_GOT_IP:
                  #if (DEBUG_MODE > CFG_DEBUG_NONE)
                        //Serial.print("Obtained IP address: "); Serial.println(WiFi.localIP());
                    #endif
                break;
            case SYSTEM_EVENT_STA_LOST_IP:
                  #if (DEBUG_MODE > CFG_DEBUG_NONE)
                        Serial.println("Lost IP address and IP address is reset to 0");
                    #endif
                break;
            case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
                  #if (DEBUG_MODE > CFG_DEBUG_NONE)
                        Serial.println("WiFi Protected Setup (WPS): succeeded in enrollee mode");
                    #endif
                break;
            case SYSTEM_EVENT_STA_WPS_ER_FAILED:
                  #if (DEBUG_MODE > CFG_DEBUG_NONE)
                        Serial.println("WiFi Protected Setup (WPS): failed in enrollee mode");
                    #endif
                break;
            case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
                  #if (DEBUG_MODE > CFG_DEBUG_NONE)
                        Serial.println("WiFi Protected Setup (WPS): timeout in enrollee mode");
                    #endif
                break;
            case SYSTEM_EVENT_STA_WPS_ER_PIN:
                  #if (DEBUG_MODE > CFG_DEBUG_NONE)
                        Serial.println("WiFi Protected Setup (WPS): pin code in enrollee mode");
                    #endif
                break;
            #ifdef USE_WIFI_AP
              // access point events
              case SYSTEM_EVENT_AP_START:
                        Serial.println("WiFi access point started");
                  break;
              case SYSTEM_EVENT_AP_STOP:
                        Serial.println("WiFi access point  stopped");
                  break;
              case SYSTEM_EVENT_AP_STACONNECTED:
                        Serial.println("Client connected");
                  break;
              case SYSTEM_EVENT_AP_STADISCONNECTED:
                        Serial.println("Client disconnected");
                  break;
              case SYSTEM_EVENT_AP_STAIPASSIGNED:
                        Serial.println("Assigned IP address to client");
                  break;
              case SYSTEM_EVENT_AP_PROBEREQRECVED:
                         Serial.println("Received probe request");
                  break;
              case SYSTEM_EVENT_GOT_IP6:
                  Serial.println("IPv6 is preferred");
                  break;
              #endif
            #ifdef USE_WIFI_ETHERNET
                  // ethernet events
              case SYSTEM_EVENT_ETH_START:
                  Serial.println("Ethernet started");
                  break;
              case SYSTEM_EVENT_ETH_STOP:
                  Serial.println("Ethernet stopped");
                  break;
              case SYSTEM_EVENT_ETH_CONNECTED:
                  Serial.println("Ethernet connected");
                  break;
              case SYSTEM_EVENT_ETH_DISCONNECTED:
                  Serial.println("Ethernet disconnected");
                  break;
              case SYSTEM_EVENT_ETH_GOT_IP:
                  Serial.println("Obtained IP address");
                  break;
              #endif
            default:
                break;
          }
      }
  //
  // --- callback webserver -------------
    #ifdef UNUSED
        void handleWebSocketMessage(AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len)
          {
          }
      #endif

    void md_onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
                 void *arg, uint8_t *data, size_t len)
      {
        switch (type)
          {
            case WS_EVT_PONG:
              // ignore
              break;
            case WS_EVT_ERROR:
              SOUTLN("svr_onEvent ERROR receive");
              break;
            default:
              pwebSrv->handleClient(type, client, arg, data, len);
              break;
          }
      }
//
// --- classes
  // ------ class md_msglist
  // ------ class md_localIP --------------------------
    bool md_localIP::setIP(uint32_t ip)
      {
        _IP = ip;
        _stat = _stat | LOCIP_IP;
        return WIFI_OK;
      }

    bool md_localIP::setGW(uint32_t ip)
      {
        _IP = ip;
        _stat = _stat | LOCIP_GW;
        return WIFI_OK;
      }
  // ------ class md_NTPTime --------------------------
    bool md_NTPTime::initNTPTime()
      {
        _timezone = 1;
        udp.begin(localPort);
        return WIFI_OK;
      }

    bool md_NTPTime::getTime(time_t *ntpEpoche)
      {
        time_t tmpTime = 0;
        if (WiFi.status() == WL_CONNECTED)
        {
            #if (WIFI_DEBUG_MODE >= CFG_DEBUG_ACTIONS)
                SOUT(" "); SOUT(millis()); SOUT(" get NTP time ... ");
              #endif
          sendNTPpacket(timeServer);
          sleep(1);

          int cb = udp.parsePacket();
            #if (WIFI_DEBUG_MODE >= CFG_DEBUG_ACTIONS)
                Serial.print("parsePacket cb = "); Serial.println(cb);
              #endif
          if (cb > 0)
            {
              cb = udp.read(packetBuffer, NTP_PACKET_SIZE);
              #if (WIFI_DEBUG_MODE >= CFG_DEBUG_ACTIONS)
                      Serial.print("read Packet cb = "); Serial.println(cb);
                #endif
              if (cb == NTP_PACKET_SIZE)
                {
                  unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
                  unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);

                  unsigned long secsSince1900 = highWord << 16 | lowWord;

                  const unsigned long seventyYears = 2208988800UL;
                  *ntpEpoche = secsSince1900 - seventyYears + _timezone * 3600;
                  //
                  setTime(*ntpEpoche);
                  if ((year() > 2000))   // got valid time
                    { // check summertime periode
                      int8_t  tmp  = (int8_t) month();
                      SOUT("NTP time month "); SOUT(tmp);
                      if ( (tmp > 3) && (tmp < 11))
                          {
                            _seasontime = UTC_SUMMERTIME;
                            SOUT(" = summer  ");
                          }
                        else
                          {
                            _seasontime = UTC_WINTERTIME;
                            SOUTLN(" = winter");
                          }
                      *ntpEpoche += _seasontime;
                      setTime(*ntpEpoche);
                    }
                  return WIFI_OK;
                }
            }
        }
        return WIFI_ERR;
      }

    uint64_t md_NTPTime::sendNTPpacket(IPAddress& address)
      {
        //Serial.println("sending NTP packet...");
        // set all bytes in the buffer to 0
        memset(packetBuffer, 0, NTP_PACKET_SIZE);
        // Initialize values needed to form NTP request
        // (see URL above for details on the packets)
        packetBuffer[0] = 0b11100011;   // LI, Version, Mode
        packetBuffer[1] = 0;     // Stratum, or type of clock
        packetBuffer[2] = 6;     // Polling Interval
        packetBuffer[3] = 0xEC;  // Peer Clock Precision
        // 8 bytes of zero for Root Delay & Root Dispersion
        packetBuffer[12]  = 49;
        packetBuffer[13]  = 0x4E;
        packetBuffer[14]  = 49;
        packetBuffer[15]  = 52;

        // all NTP fields have been given values, now
        // you can send a packet requesting a timestamp:
        udp.beginPacket(address, 123); //NTP requests are to port 123
        udp.write(packetBuffer, NTP_PACKET_SIZE);
        udp.endPacket();
        return 0;
      }

  // ------ class md_wifi    --------------------------
    md_wifi::md_wifi()
      {
        _gateip    = (uint32_t) 0;
        _subnet    = (uint32_t) 0;
        _locip     = (uint32_t) 0;
        _ssid[0]   = 0;
        _passw[0]  = 0;
      }

    bool md_wifi::scanWIFI(ip_list* plist)
      {
        _ssid[0] = 0;
        _locip   = (uint32_t) 0;
        _gateip  = (uint32_t) 0;
        _subnet  = (uint32_t) 0;
            #if (WIFI_DEBUG_MODE > CFG_DEBUG_NONE)
                SOUTLN();
                SOUT(millis()); SOUT(" WIFI scan pList "); SOUTHEX((u_long) plist);
                SOUT(" pFirst "); SOUTHEXLN((u_long) plist->pFirst());
              #endif
        usleep(10000);

        // WiFi.scanNetworks will return the number of networks found
        int n = WiFi.scanNetworks();
        if (n == 0)
            {
              SOUTLN("  ! no networks found !");
            }
          else
            {
              SOUT(n); SOUTLN(" networks found");
              //uint8_t  s = 0;
              ip_cell* pip = (ip_cell*) plist->pFirst();
                  #if (WIFI_DEBUG_MODE > CFG_DEBUG_NONE)
                      //SOUT(" scanWIFI pList "); SOUTHEX((u_long) plist);
                      //SOUT("  pip "); SOUTHEXLN((u_long) pip);
                    #endif
              for (uint8_t i = 0; i < n; ++i)
                {
                  // Print SSID and RSSI for each network found
                  pip = plist->find(WiFi.SSID(i).c_str());
                  if ( (pip != NULL)  )
                      {
                        pip->getSSID(_ssid, LOGINTXT_MAX_LEN);
                        pip->getPW(_passw, LOGINTXT_MAX_LEN);
                        _locip  = pip->locIP();
                        _gateip = pip->gwIP();
                        _subnet = pip->snIP();
                        SOUT(" used: "); SOUT((char*) _ssid); SOUT(" - ");
                      }
                    else
                      {
                        SOUT("       ");
                      }
                  SOUT(WiFi.SSID(i));
                  SOUT(" ("); SOUT(WiFi.RSSI(i)); Serial.print(")");
                  SOUTLN((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
                  usleep(10000);
                }
            }
        if (strlen(_ssid) > 0)
            { return WIFI_OK; }
          else
            { return WIFI_ERR; };
      }

    uint8_t md_wifi::startWIFI()
      {
        #if (WIFI_DEBUG_MODE > CFG_DEBUG_NONE)
            SOUT(millis()); SOUTLN(" md_startWIFI");
          #endif
                //_debugConn();
        if (strlen(_ssid) == 0)
          { // keine SSID
            #if (WIFI_DEBUG_MODE > CFG_DEBUG_NONE)
                SOUT(millis());
                SOUT(" SSID nicht initialisiert ");
              #endif
            return WIFI_ERR;
          }
                 //_debugConn();
        if(_locip > 0)
          {
            WiFi.config(_locip, _gateip, _subnet);
            #if (WIFI_DEBUG_MODE > CFG_DEBUG_NONE)
                Serial.print("set locIP '"); Serial.print(_locip); Serial.println("'");
              #endif
          }
        SOUT("start WiFi '"); SOUT(_ssid); SOUT("' - '"); SOUT(_passw); SOUT("'");
        WiFi.begin(_ssid, _passw); // start connection
        // Wait for connection
        usleep(_conn_delay);
        uint8_t repOut = (uint8_t) _conn_rep;
        while ((WiFi.status() != WL_CONNECTED) && (repOut > 0))
          {
            Serial.print(".");
            usleep(_conn_delay);
            repOut--;
          }

        if (WiFi.status() == WL_CONNECTED)
          {
            _debugConn(TRUE);
            if (MDNS.begin("esp32"))
            {
              SOUT("    -> "); SOUTLN(WiFi.localIP());
            }
            return WIFI_OK;
          }
          else
          {
            SOUTLN(" connection failed -> timout");
          }
          return WIFI_ERR;
      }
    void md_wifi::_debugConn(bool _wifi)
      {
        #if (WIFI_DEBUG_MODE > CFG_DEBUG_NONE)
            if (_wifi == TRUE)
              {
                SOUTLN("");
                SOUT  (millis());
                SOUT  (" Connected to ");
                SOUTLN(_ssid);
                SOUT  ("IP address: ");
                SOUTLN(WiFi.localIP());
                SOUT  ("MAC address: ");
                SOUTLN(WiFi.macAddress());
              }
            else
              {
                SOUTLN("");
                SOUT  (millis());
                SOUT  (" local connection ");
                SOUT  (_ssid);
                SOUTLN(_passw);
                SOUT  ("IP address: ");
                SOUT  (_locip);     SOUT(" - ");
                SOUT  (_locip[0]);  SOUT(".");
                SOUT  (_locip[1]);  SOUT(".");
                SOUT  (_locip[2]);  SOUT(".");
                SOUT  (_locip[3]);  SOUT("  ");
                SOUT  (_subnet);    SOUT(" - ");
                SOUT  (_subnet[0]); SOUT(".");
                SOUT  (_subnet[1]); SOUT(".");
                SOUT  (_subnet[2]); SOUT(".");
                SOUT  (_subnet[3]); SOUT("  ");
                SOUT  (_gateip);    SOUT(" - ");
                SOUT  (_gateip[0]); SOUT(".");
                SOUT  (_gateip[1]); SOUT(".");
                SOUT  (_gateip[2]); SOUT(".");
                SOUTLN(_gateip[3]);
                SOUTLN();
              }
          #endif
      }
  // ------ class md_server  --------------------------
    bool    md_server::md_startServer()
      {
        initSPIFFS();
        initWebSocket();
        // Web Server Root URL
        webServ.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                    { request->send(SPIFFS, "/index.html", "text/html"); }
                  );
        webServ.serveStatic("/", SPIFFS, "/");
                  // install data handler
                  //createDefElements(switches, pwms, analogs);
        // start server
        webServ.begin();
                  // start tasks
                  //startMsgHandlerTask();
        pwebSrv = this;
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
        ws.onEvent(md_onEvent);
        webServ.addHandler(&ws);
        SOUT(" ready ");
      }

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

    void    md_server::handleClient(AwsEventType type, AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len)
      {
        AwsFrameInfo *info = (AwsFrameInfo*)arg;
        char txt[40];
        String* sTxt = new String(); //(char*) data;
        char myIn[len+1];
        md_message* message = new md_message();
        //message->pMsg = new mdMSG_t;
        message->pMsg->client = client->id();
        switch (type)
          {
            case WS_EVT_CONNECT:
              sprintf(txt, "new #%u %s", client->id(), client->remoteIP().toString().c_str());
              break;
            case WS_EVT_DISCONNECT:
              sprintf(txt, "lost #%u %s", client->id(), client->remoteIP().toString().c_str());
              //SOUTLN(txt);
              *sTxt = txt;
              break;
            case WS_EVT_DATA:
              //_pHandler(client, arg, data, len);
              //SOUTpwebSrv->handleClient(type, client, arg, data, len);
              //Serial.printf("data #%u: %s", client->id(),  data);
              txt[len] = 0;
              memcpy(txt, data, len);
              message->pMsg->type = txt[0];
              *sTxt = txt;
              break;
            default:
              break;
          }
        message->pMsg->payload = txt;
        SOUT(" sTxt "); SOUT(message->pMsg->payload);
        message->setobj(message);
        msgList->srvSem = OBJBUSY;
        //while (msgList->hostSem != OBJBUSY) { SOUTLN("busy"); usleep(10); }
        msgList->add(message);
        msgList->srvSem = false;
        SOUT(" msList.count "); SOUTLN(msgList->count());

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

  // ----- md_server private
    void    md_server::createDefElements(uint8_t switches, uint8_t pwms, uint8_t analogs)
      {
        uint8_t i;
                  //SOUT(" start_Server sw pwm ana "); SOUT(switches);
                  //SOUT(" "); SOUT(pwms); SOUT(" "); SOUTLN(analogs);
                  //SOUT(" load switches ");

        for ( i=0 ; i<switches ; i++ )
          {
            md_switch* ptmp = new md_switch();
            ptmp->name = "SW " + (i+1);
                  //SOUT(ptmp->name); SOUT(" )");
            psliderList->add((void*) ptmp);
          }
                  //SOUTLN();
                  //SOUT(" load LEDs ");
        for ( i=0 ; i<pwms ; i++ )
          {
            md_slider* ptmp = new md_slider();
            ptmp->name = "LED " + (i+1);
                  //SOUT(ptmp->idx); SOUT(" ");
                  //SOUT(*(ptmp->name)); SOUT(" ");
            psliderList->add((void*) ptmp);
          }
                  //SOUTLN();
                  //SOUT(" load anas ");
        for ( i=0 ; i<analogs ; i++ )
          {
            md_analog* ptmp = new md_analog();
            ptmp->name = "Analog " + (i+1);
            ptmp->unit = "U_" + (i+1);
                  //SOUT(ptmp->name); SOUT(" ");
            panalogList->add((void*) ptmp);
          }
                  //SOUTLN();
      }
//
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

//#endif
