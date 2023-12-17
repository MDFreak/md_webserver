#if defined(USE_WIFI) || defined(USE_WEBSERVER)
    #if defined(MD_WIFI_DEBUG_MODE) && defined(DEBUG_MODE)
        #define MD_WIFI_DEBUG_MODE DEBUG_MODE
      #else
        #define MD_WIFI_DEBUG_MODE CFG_DEBUG_NONE
      #endif // defined(MD_WIFI_DEBUG_MODE) && defined(DEBUG_MODE)
    #include <md_defines.h>
    #include <md_wifi.h>

        //#define WIFI_DEBUG_MODE  CFG_DEBUG_NONE
        #ifndef WIFI_DEBUG_MODE
            #define WIFI_DEBUG_MODE  CFG_DEBUG_STARTUP
          #endif
        #ifndef WIFI_DEBUG_MODE
            #define WIFI_DEBUG_MODE  CFG_DEBUG_ACTIONS
          #endif

    // --- declaration
      // --- WiFi
        #define WIFI_DEF_ALIVE_TIME_MS 10000ul
        #define WIFI_DEF_TIMEOUT_MS    20000ul
        #define WIFI_DEF_CONN_DELAY_MS 500ul
        #define WIFI_DEF_CONN_REPEAT   10
        static  md_wifi*    _pmd_wifi  = NULL;
        static  uint8_t     _wifi_conn = OFF;
        static  LoginTxt_t  _ssid;
        static  LoginTxt_t  _passw;
        static  IPAddress   _gateip; // used local gateway
        static  IPAddress   _subnet; // used local subnet
        static  IPAddress   _locip;  //used local ipaddress
        static  uint32_t    _startAttTime_ms = 0;
        static  uint32_t    _wifiTout_ms     = WIFI_DEF_TIMEOUT_MS;
        static  uint32_t    _wifiConDelay_ms = WIFI_DEF_CONN_DELAY_MS;
        static  uint32_t    _wifiAliveTime_ms= WIFI_DEF_ALIVE_TIME_MS;
        static  uint8_t     _wifiConnRepeat  = WIFI_DEF_CONN_REPEAT;
        static  uint8_t     _tskWiFi = OFF;
        static
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

    // --- tasks
      void keepWiFiAlive(void * parameters)
        {
          // first run: set flag for running, send serial message
          if (!_tskWiFi)
            {
              #if (WIFI_DEBUG_MODE >= CFG_DEBUG_STARTUP)
                  SOUT(millis()); SOUT(" Task keepWiFiAlive on core "); SOUTLN(CONFIG_ARDUINO_RUNNING_CORE);
                  Serial.flush();
                #endif
              _tskWiFi = ON;
            }
          //  reconnect loop if connection lost
          //  check wifi online: do nothing nothing
          for(;;)
            {
              for(;;)
                {
                  if (WiFi.status() == WL_CONNECTED)
                    {
                      _wifi_conn = ON;
                          //SOUT(millis()); SOUTLN(" Task keepWiFiAlive wakeup");
                          //Serial.flush();
                      vTaskDelay(_wifiAliveTime_ms / portTICK_PERIOD_MS);
                      continue; // start again
                    }
                }
              // only runs if not connected
              _wifi_conn = OFF;
              WiFi.mode(WIFI_STA);
              if(_locip > 0)
                {
                  WiFi.config(_locip, _gateip, _subnet);
                  #if (WIFI_DEBUG_MODE >= CFG_DEBUG_STARTUP)
                      SOUT("set locIP '"); SOUT(_locip); SOUTLN("'");
                    #endif
                }
              //      WiFi.begin(_ssid, _passw);
              _pmd_wifi->connect();
              _startAttTime_ms = millis();
              while(   (WiFi.status() != WL_CONNECTED)
                    && (millis() - _startAttTime_ms < WIFI_DEF_TIMEOUT_MS))
                {
                  if (WiFi.status() != WL_CONNECTED)
                    {
                      SOUT(millis()); SOUTLN(" ERR  connect WiFi failed");
                    }
                  else
                    {
                      _wifi_conn = ON;
                      #if (WIFI_DEBUG_MODE >= CFG_DEBUG_STARTUP)
                          SOUT(millis()); SOUTLN(" WiFi connected");
                        #endif
                      vTaskDelay(_wifiAliveTime_ms / portTICK_PERIOD_MS);
                      continue;
                    }
                }
            }
        }
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
    // --- callback functions
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
      // --- callback webserver -------------
        #ifdef UNUSED
            void handleWebSocketMessage(AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len)
              {
              }
          #endif

        #ifdef NU_WEBSERVER
            void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
                         void *arg, uint8_t *data, size_t len)
              {
                pmdServ->handleClient(type, client, arg, data, len);
              }
          #endif
    //
    // --- classes
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
        uint8_t md_localIP::getSSID( char* ssid )
          {
            strcpy(ssid, _ssid);
            return strlen(_ssid);
          }
        uint8_t md_localIP::getPW( char* pw )
          {
            strcpy(pw, _passw);
            return strlen(_passw);
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
                                SOUTLN(" = summer  ");
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
            _pmd_wifi = this;
          }
        uint8_t md_wifi::scanWIFI(md_ip_list* plist)
          {
            if (!plist)
              {
                SOUT(millis()); SOUTLN(" scanWIFI ERR pList=NULL");
                return WIFI_ERR;
              }
            _ssid[0] = 0;
            _locip   = (uint32_t) 0;
            _gateip  = (uint32_t) 0;
            _subnet  = (uint32_t) 0;
                #if (WIFI_DEBUG_MODE > CFG_DEBUG_NONE)
                    SOUTLN();
                    SOUT(millis()); SOUTLN(" WIFI scan pList "); //SOUTHEX((u_long) plist);
                    //SOUT(" pFirst "); SOUTHEXLN((u_long) plist->pFirst());
                  #endif
            usleep(10000);
            // WiFi.scanNetworks will return the number of networks found
            int n = WiFi.scanNetworks();
            bool ready = false;
            if (n == 0)
                {
                  SOUTLN("  ! no networks found !");
                }
              else
                {
                  SOUT(millis()); SOUT(" "); SOUT(n); SOUTLN(" networks found");
                  //uint8_t  s = 0;
                  md_ip_cell* pip = (md_ip_cell*) plist->pFirst();
                      #if (WIFI_DEBUG_MODE > CFG_DEBUG_NONE)
                          //SOUT(" scanWIFI pList "); SOUTHEX((u_long) plist);
                          //SOUT("  pip "); SOUTHEXLN((u_long) pip);
                        #endif
                  for (uint8_t i = 0 ; !ready && (i < n); ++i)
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
                            ready = true;
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
            int32_t iret = 0;
            uint8_t ret = WIFI_ERR;
            #if (WIFI_DEBUG_MODE > CFG_DEBUG_NONE)
                //SOUT(millis()); SOUTLN(" md_startWIFI");
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
                    //Serial.print("set locIP '"); Serial.print(_locip); Serial.println("'");
                  #endif
              }
                /*    SOUT(millis()); SOUT(" start WiFi '"); SOUT(_ssid); SOUT("' - '"); SOUT(_passw); SOUT("'");
                    WiFi.begin(_ssid, _passw); // start connection
                    // Wait for connection
                          //SOUTLN(); SOUT(millis()); SOUTLN(" nach WiFi begin");
                    usleep(WIFI_CONN_DELAY);
                    uint8_t repOut = (uint8_t) WIFI_CONN_REP;
                          //SOUT(millis()); SOUTLN(" nach usleep");
                    while ((WiFi.status() != WL_CONNECTED) && (repOut > 0))
                      {
                        SOUT(".");
                        usleep(WIFI_CONN_DELAY);
                        repOut--;
                      }
                          //SOUTLN(); SOUT(millis()); SOUTLN(" nach while");
                    SOUTLN(); SOUT(millis()); SOUT("WiFi.status "); SOUT(WiFi.status() == WL_CONNECTED ? "WLAN connected" : "WLAN not connected");
                  */
            iret = xTaskCreatePinnedToCore
                    ( keepWiFiAlive,
                      "keep Wifi alive",
                      2000,
                      NULL,
                      1,
                      NULL,
                      CONFIG_ARDUINO_RUNNING_CORE
                    );
            connect();
            if (WiFi.status() == WL_CONNECTED)
              {
                //_debugConn(TRUE);
                //SOUT("    -> "); SOUTLN(WiFi.localIP());
                    //if (MDNS.begin("esp32"))
                    //{
                      //SOUT("    -> "); SOUTLN(WiFi.localIP());
                    //}
                ret = ISOK;
              }
              else
              {
                SOUTLN(" connection failed -> timout");
              }
            return ret;
          }
        uint8_t md_wifi::connect()
          {
             #if (WIFI_DEBUG_MODE > CFG_DEBUG_NONE)
                SOUT(millis()); SOUTLN(" md_connect_wifi");
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
                    SOUT(millis()); SOUT(" set locIP '"); SOUT(_locip); SOUTLN("'");
                  #endif
              }
            SOUT(millis()); SOUT(" start WiFi '"); SOUT(_ssid); SOUT("' - '"); SOUT(_passw); SOUTLN("'");
            WiFi.begin(_ssid, _passw); // start connection
            // Wait for connection
            usleep(_wifiConDelay_ms*1000);
            uint8_t repOut = (uint8_t) _wifiConnRepeat;
            while ((WiFi.status() != WL_CONNECTED) && (repOut > 0))
              {
                Serial.print(".");
                usleep(_wifiConDelay_ms*1000);
                repOut--;
              }

            if (WiFi.status() == WL_CONNECTED)
              {
                _debugConn(TRUE);
                if (MDNS.begin("esp32"))
                {
                  //SOUT("    -> "); SOUTLN(WiFi.localIP());
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
  #endif // defined(USE_WIFI) || defined(USE_WEBSERVER)