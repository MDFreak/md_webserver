#ifdef ONLY_DOCUMENTATION
  * Home <https://techtutorialsx.com/>
  * About <https://techtutorialsx.com/about/>
  ESP32 <https://techtutorialsx.com/category/esp32/> / Leave a Comment
  <https://techtutorialsx.com/2021/12/06/esp32-websocket-server-broadcast-messages/#respond>

ESP32 websocket server: Broadcast messages
    1 Introduction <#Introduction>
    2 The Arduino websocket server code <#The_Arduino_websocket_server_code>
    3 The HTML/JavaScript websocket client code <#The_HTMLJavaScript_websocket_client_code>
    4 Testing the code <#Testing_the_code>
    5 Final Notes <#Final_Notes>
    6 Suggested ESP32 readings <#Suggested_ESP32_readings>

Introduction
    In this tutorial we are going to learn how to broadcast messages to all
    the websocket clients connected to a ESP32 server. We will be using the
    Arduino core and the HTTP async web server library
    <https://github.com/me-no-dev/ESPAsyncWebServer>.

    For an introductory tutorial to this library, please check here
    <https://techtutorialsx.com/2017/12/01/esp32-arduino-asynchronous-http-webserver/>. For an introductory tutorial on how to work with websockets with the HTTP async web server, please go here <https://techtutorialsx.com/2018/08/14/esp32-async-http-web-server-websockets-introduction/>. In the “Suggested ESP32 readings” section, I’ll also leave some websocket related tutorials for the ESP32.

    The websockets feature of the HTTP server we are going to use supports
    multiple clients connected simultaneously. This means that, at some
    point, we may want to broadcast the same information to all the clients
    connected. The library offers a very easy to use method that allows us
    to do so without having to know how many clients we have connected at a
    given time, neither having to store handles to reach those clients.

    If you are curious about a real applications scenario for this
    functionality, it is used under the hood by the ESP-DASH library
    <https://github.com/ayushsharma82/ESP-DASH> we have been covered on
    previous tutorials
    <https://techtutorialsx.com/2021/10/12/esp32-real-time-web-dashboard/>
    (you can check the source code where it is used here
    <https://github.com/ayushsharma82/ESP-DASH/blob/8219f852c0f84f9f8b2fab0a416c2abf8611064a/src/ESPDash.cpp#L340>). Basically, whenever a dashboard page is served, it will have a websocket client under the hood connecting to the ESP32 server, to be notified of updates. If the user opens multiple tabs, there will be multiple websocket clients that need to get updated with the exact same information. Consequently, having a broadcast method solves this issue very easily.

    In our tutorial, we are going to focus our attention on the broadcast
    feature. As such, the websocket client that we will develop is a very
    simple application in HTML and JavaScript. To keep things simple and
    easy, we are going to run this client code from a computer on the same
    WiFi network as the ESP32, in a web browser. Then, if we run it multiple
    times, we will have multiple clients connecting to the server.

    Also, to keep the client code simple and easy to follow, we are not
    going to write any CSS to make it look better, as this is outside the
    scope of what this tutorial is intended to cover.

    The tests shown below were performed on a ESP32-E FireBeetle board
    <https://www.dfrobot.com/product-2231.html?tracking=6103a511e0361> from DFRobot <https://www.dfrobot.com/?tracking=6103a511e0361>. The Arduino core version used was *2.0.0* and the Arduino IDE version was *1.8.15*, working on Windows *8.1*. The code was also tested on Platformio <https://platformio.org/>.

The Arduino websocket server code
  explanation
    We will start by the library includes. We will need the *WiFi.h*, to be
    able to connect the ESP32 to a wireless network, and the
    *ESPAsyncWebServer.h*, which we can use to setup a HTTP server that
    supports websocket endpoints.

    1 |#include| |<ESPAsyncWebServer.h>|
    2 |#include| |<WiFi.h>|

    After that, we need the credentials of the WiFi network. More precisely,
    we will need its name (SSID) and its password. Please take in
    consideration that I’m using placeholders below, which you should
    replace by the actual credentials of your WiFi network.

    1 |const| |char||*| |ssid ||=| |"yourNetworkName"||;|
    2 |const| |char||*| |password ||=|  |"yourNetworkPassword"||;|

    Now we will create an object of class *AsyncWebServer*. As input of the
    constructor, we pass the number of the port where the HTTP server will
    be listening to incoming requests. We will use port *80*, which is the
    default HTTP port.

    1 |AsyncWebServer server(||80||);|

    We will also need an object of class *AsyncWebSocket* to be able to
    setup the websocket endpoint. As input of the constructor of this class
    we need to pass the route of the websocket endpoint. This route is
    passed as a string.

    1

    |AsyncWebSocket ws(||"/ws"||);|

    We will now move on to the Arduino setup, where we will start by opening
    a serial connection. This way, we will be able to output some
    informative messages from our program.

    1

    |Serial.begin||(||115200||);|

    Then we will connect the ESP32 to the WiFi network, using the previously
    defined credentials (network name and password). At the end we will
    print the IP address assigned to the ESP32 on the network, so we can
    later reach it from our websocket clients.

    1
    2
    3
    4
    5
    6
    7
    8

    |WiFi.begin(ssid, password);|
    | |
    |while| |(WiFi.status() ||!||=| |WL_CONNECTED) {|
    |  ||delay||(||1000||);|
    |  ||Serial.println||(||"Connecting to WiFi.."||);|
    |}|
    | |
    |Serial.println||(WiFi.localIP());|

    After this we will register a function to handle all the websocket
    related events (client connection, disconnection, etc..). This is done
    with a call to the *onEvent* method on our *AsyncWebSocket* object,
    passing as input the handling function. We will call this function
    *onWsEvent* and check its implementation later.

    1

    |ws.onEvent(onWsEvent);|

    Next we need to register our websocket endpoint in our HTTP server. To
    do so, we simply call the *addHandler* method on our *AsyncWebServer*
    object, passing as input the address of the *AsyncWebSocket* object.

    1

    |server.addHandler(||&||ws);|

    To finalize the Arduino setup, we will call the *begin *method on the
    *AsyncWebServer* object. This call will ensure that the server starts
    listening to incoming requests.

    1

    |server.begin();|

    The full setup function is available below.

    1
    2
    3
    4
    5
    6
    7
    8
    9
    10
    11
    12
    13
    14
    15
    16
    17

    |void| |setup||(){|
    |  ||Serial.begin||(||115200||);|
    | |
    |  ||WiFi.begin(ssid, password);|
    | |
    |  ||while| |(WiFi.status() ||!||=| |WL_CONNECTED) {|
    |    ||delay||(||1000||);|
    |    ||Serial.println||(||"Connecting to WiFi.."||);|
    |  ||}|
    | |
    |  ||Serial.println||(WiFi.localIP());|
    | |
    |  ||ws.onEvent(onWsEvent);|
    |  ||server.addHandler(||&||ws);|
    | |
    |  ||server.begin();|
    |}|

    Moving on to the Arduino loop, we will periodically broadcast a text
    message to all the connected clients. To do so, we simply need to call
    the *textAll* method on our *AsyncWebSocket* object, passing as input a
    string with the content we want to broadcast.

    1

    |ws.textAll(||"Broadcasted Message"||);|

    After this we will introduce a small 2 seconds (2000 milliseconds)
    delay, using the Arduino delay
    <https://www.arduino.cc/reference/en/language/functions/time/delay/>
    function. If you want to avoid the usage of the Arduino main loop and
    explicit delays, I recommend you to check the Ticker library
    <https://techtutorialsx.com/2021/08/07/esp32-ticker-library/>.

    1

    |delay||(||2000||);|

    The complete setup is available in the code snippet below.

    1
    2
    3
    4
    5
    6

    |void| |loop||(){|

    |  ||ws.textAll(||"Broadcasted Message"||);|

    |  ||delay||(||2000||);|
    |}|

    To finalize the ESP32 Arduino code, the only thing left to analyze is
    the implementation of the event handling function, which we have called
    *onWsEvent*. This handling function needs to follow the signature
    defined here
    <https://github.com/me-no-dev/ESPAsyncWebServer/blob/2f784268f0a358741ee6384480d48656e159d726/src/AsyncWebSocket.h#L240>.

    Since we are going to only print an informative message regarding the
    client connection and disconnection events, we will only make use of the
    third parameter, which is an enum of type AwsEventType
    <https://github.com/me-no-dev/ESPAsyncWebServer/blob/2f784268f0a358741ee6384480d48656e159d726/src/AsyncWebSocket.h#L81> and basically allows us to identify which event we are handling. The other parameters are useful for many other use cases (ex: receiving data from the client <https://techtutorialsx.com/2018/08/23/esp32-http-webserver-receiving-textual-data-in-websocket/> or sending a message back to the client <https://techtutorialsx.com/2018/08/14/esp32-async-http-web-server-websockets-introduction/> when it connects).

    1
    2
    3

    |void| |onWsEvent(AsyncWebSocket ||*| |server, AsyncWebSocketClient ||*| |client, AwsEventType type, ||void| |*| |arg, uint8_t ||*||data, size_t len){|
    |  ||// Print informative message about the event|
    |}|

    Like mentioned, for our example we are interested in the events of
    connection or disconnection of a client, meaning that we are looking for
    the enumerated values *WS_EVT_CONNECT *and *WS_EVT_DISCONNECT*. If we
    obtain the first, we will print a message to the serial port indicating
    the client connected. If we obtain the second, we will print a message
    indicating the client disconnected.

    The complete callback, with these prints, can be seen below.

    1
    2
    3
    4
    5
    6
    7
    8
    9
    10
    11
    12

    |void| |onWsEvent(AsyncWebSocket ||*| |server, AsyncWebSocketClient ||*| |client, AwsEventType type, ||void| |*| |arg, uint8_t ||*||data, size_t len){|
    | |
    |  ||if||(type ||=||=| |WS_EVT_CONNECT){|
    | |
    |    ||Serial.println||(||"Websocket client connection received"||);|
    |    |
    |  ||} ||else| |if||(type ||=||=| |WS_EVT_DISCONNECT){|

    |    ||Serial.println||(||"Client disconnected"||);|
    | |
    |  ||}|
    |}|

  The complete code is shown below.


#include <WiFi.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "yourNetworkName";
const char* password =  "yourNetworkPassword";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){

  if(type == WS_EVT_CONNECT)
    {

      Serial.println("Websocket client connection received");

    }
    else if(type == WS_EVT_DISCONNECT)
    {
      Serial.println("Client disconnected");
    }
}

void setup()
  {
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Connecting to WiFi..");
    }

    Serial.println(WiFi.localIP());

    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
    server.begin();
  }

void loop()
  {
    ws.textAll("Broadcasted Message");
    delay(2000);
  }

    The HTML/JavaScript websocket client code

For simplicity, we will write our HTML and JavaScript code in the same
file. We will start by the HTML code and then finish by analyzing the
JavaScript code inside.

We will have two main sections, the head and the body. In the head, we
will place the JavaScript code that will handle the websocket
connection. We will leave our body empty since we are going to add new
paragraphs <https://www.w3schools.com/html/html_paragraphs.asp>
dynamically per each message we receive on our websocket.

1
2
3
4
5
6
7
8
9
10
11
12

|<!||DOCTYPE| |html>|
|<||html||>|

|   ||<||head||>|
|      ||<||script| |type| |= ||"text/javascript"||>|
|              ||<!--JavaScript Code-->|
|      ||</||script||>|
|   ||</||head||>|
| |
|   ||<||body||>      |
|   ||</||body||>|
|</||html||>|

Moving on to the actual JS code, we will start by creating a new
WebSocket <https://developer.mozilla.org/en-US/docs/Web/API/WebSocket>
object. As input of the constructor, we should pass a string with the
websocket endpoint. The format of the URL is shown below, where
*#yourEspIp#* must be replaced by the actual IP assigned to your ESP32
when it connects to the WiFi network (its IP will get printed to the
serial port after the device is able to establish the connection).

1

|ws://#yourEspIp#/ws|

The whole constructor call is shown below. I’m using the IP address of
my ESP32, for illustration purposes.

1

|const ws = ||new| |WebSocket(||"ws://192.168.1.75/ws"||);|

Then we will register callback functions to handle the websocket
connected and disconnected events. In our simple use case, we will just
display an alert
<https://developer.mozilla.org/en-US/docs/Web/API/Window/alert>
indicating that the event happened. Note hat the onopen
<https://developer.mozilla.org/en-US/docs/Web/API/WebSocket/onopen>
property corresponds to the websocket connected event and the onclose
<https://developer.mozilla.org/en-US/docs/Web/API/WebSocket/onclose>
property to the websocket disconnected event.

1
2
3
4
5
6
7

|ws.onopen = ||function||() {|
|    ||alert(||"Connection opened"||);|
|};|

|ws.onclose = ||function||() {|
|    ||alert(||"Connection closed"||);|
|};|

Finally, we will define the handling function for when a message is
received (onmessage property
<https://developer.mozilla.org/en-US/docs/Web/API/WebSocket/onmessage>).
The implementation of this handling function will consist on appending
the received message as a paragraph, in the body.

Note that this callback function will receive as input a MessageEvent
<https://developer.mozilla.org/en-US/docs/Web/API/MessageEvent>, which
has a property called data
<https://developer.mozilla.org/en-US/docs/Web/API/MessageEvent/data>
that contains the actual content of the message. In code below, I’ve
called this parameter “*event*“.

1
2
3
4
5

|ws.onmessage = ||function||(event) {|

|  ||// Append received message in the body|

|};|

In the actual implementation of the message received handling function,
we will start by calling the createElement
<https://developer.mozilla.org/en-US/docs/Web/API/Document/createElement> method, passing as input the tag name of the element we want to create, as a string. In our case, we want to create a paragraph, so the corresponding tag name is “*p*“.

1

|const p = document.createElement(||'p'||);|

Then we will set the text content of this element (using the textContent
<https://developer.mozilla.org/en-US/docs/Web/API/Node/textContent>
property) to the data we have received in the websocket message.

1

|p.textContent = event.data;|

Finally, we will use the appendChild
<https://developer.mozilla.org/en-US/docs/Web/API/Node/appendChild>
method to append this new message to the body of the document.

1

|document.body.appendChild(p);|

|ws.onmessage = ||function||(event) {|

|    ||const p = document.createElement(||'p'||);|
|    ||p.textContent = event.data;|
|    ||document.body.appendChild(p);|

|};|

The final code, including both the HTML and the JavaScript parts, can be
seen below.

|<!||DOCTYPE| |html>|
|<||html||>|
|   ||<||head||>|
      <script type = "text/javascript">
        const ws = new WebSocket("ws://192.168.1.75/ws");
        ws.onopen = function() {
            alert("Connection opened");
        };

        ws.onclose = function() {
            alert("Connection closed");
        };

        ws.onmessage = function(event) {

            const p = document.createElement('p');
            p.textContent = event.data;
            document.body.appendChild(p);
        };

      </script>
 |
   ||</||head||>|
 |
   ||<||body||>      |
   ||</||body||>|
</||html||>|


    Testing the code

To test the end-to-end system, first compile and upload the Arduino code
and upload it to your ESP32, using a tool of your choice (Arduino IDE,
Platformio). Once the procedure finishes, open a serial monitor tool of
your choice. After the ESP32 finishes connecting to the WiFi network, it
should print the local IP address assigned to the device.

Then, use that IP address on the URL from the HTML/JS code. Then, you
can simply save that code as a *.html* file in a computer connected to
the same WiFi network as the ESP32. Open the file with a web browser of
your choice and you should see a result similar to figure 1.

Single websocket client connected to the ESP32.
<https://i0.wp.com/techtutorialsx.com/wp-content/uploads/2021/12/image.png?ssl=1>*Figure 1* – Single websocket client connected to the ESP32.

After making sure that everything is working fine with a single client,
you can open more tabs with the same file and you should see all of them
receiving the broadcasted messages, like shown in figure 2 (I have two
Google Chrome tabs and one Firefox tab opened).

ESP32 server broadcasting websocket message to multiple clients.
<https://i0.wp.com/techtutorialsx.com/wp-content/uploads/2021/12/image-1.png?ssl=1>*Figure 2 *– ESP32 server broadcasting websocket message to multiple clients.


    Final Notes

In this tutorial we have covered how to broadcast a textual message to
all the connected websocket clients. Nonetheless, websockets also
support binary content and we have a similar method to broadcast a
binary message to all the clients: binaryAll
<https://github.com/me-no-dev/ESPAsyncWebServer/blob/2f784268f0a358741ee6384480d48656e159d726/src/AsyncWebSocket.h#L296>. For an introductory tutorial on how to send binary frames to a client, please check this <https://techtutorialsx.com/2018/11/16/esp32-websocket-server-sending-binary-frame-to-client/> tutorial.

For simplicity and to focus on the broadcast functionality, we ran the
client code from a computer. Nonetheless, we could have served the
client code from the ESP32 (remember that we are running a HTTP server).
For a tutorial that involves websockets and where we are serving the
client code from the ESP32, please go here
<https://techtutorialsx.com/2018/09/13/esp32-arduino-web-server-receiving-data-from-javascript-websocket-client/>.


    Suggested ESP32 readings

  * Websocket server introduction
    <https://techtutorialsx.com/2018/08/14/esp32-async-http-web-server-websockets-introduction/>
  * Sending binary frames to websocket client
    <https://techtutorialsx.com/2018/11/16/esp32-websocket-server-sending-binary-frame-to-client/>
  * Receiving data from JavaScript websocket client
    <https://techtutorialsx.com/2018/09/13/esp32-arduino-web-server-receiving-data-from-javascript-websocket-client/>
  * Real-time web dashboard
    <https://techtutorialsx.com/2021/10/12/esp32-real-time-web-dashboard/>

Post navigation
← Previous Post
<https://techtutorialsx.com/2021/12/01/esp32-sending-an-sms-with-twilio/>


      Leave a Reply Cancel reply
      <https://techtutorialsx.com/2021/12/06/esp32-websocket-server-broadcast-messages/#respond>

Search for:

Sort by


    Categories

  * C# <https://techtutorialsx.com/category/c/> (9)
  * Electronics <https://techtutorialsx.com/category/electronics/> (8)
  * ESP32 <https://techtutorialsx.com/category/esp32/> (364)
  * ESP8266 <https://techtutorialsx.com/category/esp8266/> (99)
  * IoT <https://techtutorialsx.com/category/iot/> (3)
  * Javascript <https://techtutorialsx.com/category/javascript/> (23)
  * LinkIt Smart <https://techtutorialsx.com/category/linkit-smart/> (14)
  * Micro:bit <https://techtutorialsx.com/category/microbit/> (30)
  * Microcontrollers
    <https://techtutorialsx.com/category/microcontrollers/> (7)
  * Misc <https://techtutorialsx.com/category/misc/> (23)
  * OBLOQ <https://techtutorialsx.com/category/obloq/> (15)
  * Python <https://techtutorialsx.com/category/python/> (66)
  * Raspberry Pi <https://techtutorialsx.com/category/raspberry-pi/> (15)
  * Sipeed M1 <https://techtutorialsx.com/category/sipeed-m1/> (4)
  * SQL <https://techtutorialsx.com/category/sql/> (5)

#endif