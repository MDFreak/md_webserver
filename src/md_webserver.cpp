#include <md_webserver.h>

// --- declaration
  //
  // --- webserver
    WebServer webServ(80);
    //md_server webMD   = md_server();
    //#define WIFI_DEBUG_MODE  CFG_DEBUG_NONE
    #ifndef WIFI_DEBUG_MODE
        #define WIFI_DEBUG_MODE  CFG_DEBUG_STARTUP
      #endif
    #ifndef WIFI_DEBUG_MODE
        #define WIFI_DEBUG_MODE  CFG_DEBUG_ACTIONS
      #endif
  //
  // --- NTP server ---------------------
    unsigned int localPort = 2390;
    IPAddress    timeServer(129, 6, 15, 28);
    const int    NTP_PACKET_SIZE = 48;
    byte         packetBuffer[ NTP_PACKET_SIZE];
    WiFiUDP      udp;
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

    void handleNotFound()
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




//
// --- classes
  //
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
  //
  // ------ class md_NTPTime --------------------------
    bool md_NTPTime::initNTPTime(uint8_t summer)
      {
        _timezone = 1 + summer;
        udp.begin(localPort);
        return WIFI_OK;
      }

    bool md_NTPTime::getTime(time_t *ntpEpoche)
      {
        if (WiFi.status() == WL_CONNECTED)
        {
          sendNTPpacket(timeServer);
          sleep(1);

          int cb = udp.parsePacket();
            #if (WIFI_DEBUG_MODE > CFG_DEBUG_ACTIONS)
                Serial.print("parsePacket cb = "); Serial.println(cb);
              #endif
          if (cb > 0)
            {
              cb = udp.read(packetBuffer, NTP_PACKET_SIZE);
              #if (WIFI_DEBUG_MODE > CFG_DEBUG_ACTIONS)
                      Serial.print("read Packet cb = "); Serial.println(cb);
                #endif
              if (cb == NTP_PACKET_SIZE)
                {
                  unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
                  unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);

                  unsigned long secsSince1900 = highWord << 16 | lowWord;

                  const unsigned long seventyYears = 2208988800UL;
                  //unsigned long epoch = secsSince1900 - seventyYears;
                  *ntpEpoche = secsSince1900 - seventyYears + _timezone * 3600;
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

  //
  // ------ class md_wifi --------------------------
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
              #if (WIFI_DEBUG_MODE > CFG_DEBUG_NONE)
                  SOUTLN("no networks found");
                #endif
          }
        else
          {
                #if (WIFI_DEBUG_MODE > CFG_DEBUG_NONE)
                    SOUT(n); SOUTLN(" networks found");
                  #endif
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
                    pip->getSSID(_ssid);
                    pip->getPW(_passw);
                    _locip  = pip->locIP();
                    _gateip = pip->gwIP();
                    _subnet = pip->snIP();
                    #if (WIFI_DEBUG_MODE > CFG_DEBUG_NONE)
                        SOUT(" used: "); SOUT((char*) _ssid); SOUT(" - ");
                      #endif
                  }
                else
                  {
                    #if (WIFI_DEBUG_MODE > CFG_DEBUG_STARTUP)
                        SOUT("       ");
                      #endif
                  }
                #if (WIFI_DEBUG_MODE > CFG_DEBUG_STARTUP)
                    SOUT(WiFi.SSID(i));
                    SOUT(" ("); SOUT(WiFi.RSSI(i)); Serial.print(")");
                    SOUTLN((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
                  #endif
                usleep(10000);
              }
          }
      //  WiFi.disconnect();
        if (strlen(_ssid) > 0) { return WIFI_OK; }
        else                    { return WIFI_ERR; };
      }

    bool md_wifi::startWIFI(bool _useLocID)
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
        if(_useLocID)
          {
            WiFi.config(_locip, _gateip, _subnet);
                    #if (WIFI_DEBUG_MODE > CFG_DEBUG_NONE)
                        Serial.print("set locIP '"); Serial.print(_locip); Serial.println("'");
                      #endif
          }
                    #if (WIFI_DEBUG_MODE > CFG_DEBUG_NONE)
                        Serial.print("Starte '"); Serial.print(_ssid); Serial.print("' - '");  Serial.print(_passw); Serial.println("'");
                      #endif
        WiFi.begin(_ssid, _passw); // start connection
                 //_debugConn();
        // Wait for connection
        usleep(_conn_delay);
        uint8_t repOut = (uint8_t) _conn_rep;
        while ((WiFi.status() != WL_CONNECTED) && (repOut > 0))
          {
                    #if (WIFI_DEBUG_MODE > CFG_DEBUG_NONE)
                        Serial.print(".");
                      #endif
            usleep(_conn_delay);
            repOut--;
          }

        if (WiFi.status() == WL_CONNECTED)
          {
            _debugConn(TRUE);
            if (MDNS.begin("esp32"))
            {
                    #if (WIFI_DEBUG_MODE > CFG_DEBUG_NONE)
                        Serial.println("MDNS responder started");Serial.println();
                      #endif
            }
            return WIFI_OK;
          }
          else
          {
                    #if (WIFI_DEBUG_MODE > CFG_DEBUG_NONE)
                        Serial.println("Connection failed -> timout");
                      #endif
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
  //
  // --- class md_server --------------------------
    bool md_server::md_startServer()
      {
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
        return false;
      }

    bool md_server::md_handleClient()
      {
        webServ.handleClient();
        return false;
      }

//
// --- templates
  //
  // template callback WIFI
  #ifdef Dummy
      void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
    {
      Serial.print("WiFi connected - ");
      Serial.print("IP: ");
      Serial.println(IPAddress(info.got_ip.ip_info.ip.addr));
    }
  #endif

