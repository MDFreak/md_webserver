#if defined(USE_WIFI) || defined(USE_WEBSERVER)
  #ifndef _MD_WIFI_SERVER_H_
    #define _MD_WIFI_SERVER_H_
    // --- includes
      #include <Arduino.h>
      #include <stdlib.h>
      #include <time.h>
      //#include <Time.h>
      #include <md_time.h>
      #include <WiFi.h>
      #include <WiFiUdp.h>
      #include <ESPmDNS.h>
      #include <md_defines.h>
      #include <md_util.h>
      #include <md_ip_list.h>
      #include <functional>
    // --- declarations
      #define MD_WIFI_VERSION "V0.1.0"
      #define WIFI_OK   false
      #define WIFI_ERR  true
      #ifndef LOGINTXT_MAX_LEN
          #define LOGINTXT_MAX_LEN 30
        #endif
      using fptr = std::function<void()>;
      typedef enum LOCIP_status// locIP Status
        {
          LOCIP_FREE = 0x00,
          LOCIP_IP   = 0x01,
          LOCIP_GW   = 0x02,
          LOCIP_SN   = 0x04,
          LOCIP_OK   = LOCIP_IP | LOCIP_GW | LOCIP_SN
        } LOCIP_status_t;
      typedef char LoginTxt_t[LOGINTXT_MAX_LEN + 1];
    // classes for netservice
      class md_localIP
        {
          public:
            md_localIP(){}
            ~md_localIP(){}
            bool       setIP(uint32_t ip);
            bool       setGW(uint32_t ip);
            IPAddress  getIP()    { return _IP; }
            IPAddress  getGW()    { return _GW; }
            IPAddress  getSN()    { return _SN; }
            uint8_t    getSSID( char* ssid );
            uint8_t    getPW  ( char* pw );
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
          public:
            md_wifi();
            ~md_wifi() {}
            uint8_t scanWIFI  (md_ip_list* plist);
            uint8_t startWIFI ();
            uint8_t getNTPTime(time_t *ntpEpoche) { return getTime(ntpEpoche); }
            uint8_t initNTP   ()                  { return initNTPTime(); }
            uint8_t connect   ();
            //void    setConnPar(uint32_t connectDelay_ms);
          protected:
            void    _debugConn(bool _wifi = FALSE);
        };
    #endif // _MD_WIFI_SERVER_H_
#endif // defined(USE_WIFI) || defined(USE_WEBSERVER)