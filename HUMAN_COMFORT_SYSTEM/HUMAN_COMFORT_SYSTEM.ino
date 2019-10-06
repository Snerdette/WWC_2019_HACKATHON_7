/* Create a WiFi access point and provide a web server on it. */

#include <ESP8266WiFi.h>
#include "./DNSServer.h"                  // Patched lib
#include <ESP8266WebServer.h>
#include <Servo.h>

const byte        DNS_PORT = 53;          // Capture DNS requests on port 53
IPAddress         apIP(192,168,5,1);    // Private network for server
DNSServer         dnsServer;              // Create the DNS object
ESP8266WebServer  webServer(80);          // HTTP server

int blink_wait = 100000; // Blink wait in seconds
int loop_count = 0; // Keep track of loops
bool quality_check_switch = 0;
bool quality_alert = 0;
int incomingByte = 0; // for incoming serial data

int servoPin = 2;

bool move_servo_from_azure = 0;

Servo servo; 
int servoAngle = 0;   // servo position in degrees

String responseHTML = "<!DOCTYPE html>"
    "<html><head><title>Internet of Batteries</title>"
    "<meta charset='utf-8'>"
    "<style>"
    ".center {"
    "      "
    "      margin: 0 auto;"
    "      "
    "    }"
    ""
    "    .awesome {"
    "      "
    "      font-family: futura;"
    "      font-style: italic;"
    "      "
    "      width:100%;"
    "      "
    "      margin: 0 auto;"
    "      text-align: center;"
    "      "
    "      color:#313131;"
    "      font-size:5vw;"
    "      font-weight: bold;"
    "      "
    "      -webkit-animation:colorchange 7s infinite alternate;"
    "      "
    "      "
    "    }"
    ""
    "    @-webkit-keyframes colorchange {"
    "      0% {"
    "        "
    "        color: blue;"
    "      }"
    "      "
    "      10% {"
    "        "
    "        color: #8e44ad;"
    "      }"
    "      "
    "      20% {"
    "        "
    "        color: #1abc9c;"
    "      }"
    "      "
    "      30% {"
    "        "
    "        color: #d35400;"
    "      }"
    "      "
    "      40% {"
    "        "
    "        color: blue;"
    "      }"
    "      "
    "      50% {"
    "        "
    "        color: #34495e;"
    "      }"
    "      "
    "      60% {"
    "        "
    "        color: blue;"
    "      }"
    "      "
    "      70% {"
    "        "
    "        color: #2980b9;"
    "      }"
    "      80% {"
    "     "
    "        color: #f1c40f;"
    "      }"
    "      "
    "      90% {"
    "     "
    "        color: #2980b9;"
    "      }"
    "      "
    "      100% {"
    "        "
    "        color: pink;"
    "      }"
    "    }"
    "    </style>"
    "</head><body>"
    "<div class='center'><p class='awesome'>WWC!!!</p></div></br></br>"
    "<div><p class='awesome'>I'm just a stupid ARDUINO at WWC!!!</p></div></br></br>"
    "<div><p class='awesome'>WITH WIFI!!!</p></br></br>"
    "<div><p class='awesome'>DANGER ZONE!!!</p></br></br>"
    "<div><p class='awesome'>Deal with it. &macr;\\_(ツ)_/&macr; </p></div></br></br>"
    "<div><p class='awesome'>I'M AN ARDUINO!!!</p></div>"
    "</body></html>";
  
void handleRoot() {
  webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  webServer.sendHeader("Pragma", "no-cache");
  webServer.sendHeader("Expires", "-1");
  webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  webServer.send(200, "text/html", responseHTML);
  //webServer.client().stop(); // Stop is needed because we sent no content length
}

void setup() {
  // turn the LED on (HIGH is the voltage level)
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // Set up serial out
  Serial.begin(115200);

  // Set up the servo
  servo.attach(servoPin);

  // configure access point
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("🚧I'M A WWC PROJECT🚧!"); // WiFi name

  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  dnsServer.start(DNS_PORT, "*", apIP);

  // replay to all requests with same HTML
  //webServer.onNotFound([]() {
  //  webServer.send(200, "text/html", responseHTML);
  //});
  webServer.on("/", handleRoot);
  webServer.on("/fwlink", handleRoot);  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
  webServer.on("/generate_204", handleRoot);  //Android captive portal. Maybe not needed. Might be handled by notFound handler.
  webServer.onNotFound(handleRoot);
  webServer.begin();
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
  
  if(loop_count % blink_wait == 0){
    // Read the gas sensor
    int reading = analogRead(A0);

    if(quality_alert == 1){
      if(reading <= 75){
        quality_alert = 0;
        Serial.print("ALERT_CLEARED");
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.print(reading);
        servo.write(0);
        servoAngle = 0;
      }
    }else{
      Serial.print("NOMINAL");
      digitalWrite(LED_BUILTIN, HIGH);
    }
    
    if(reading > 75){
      if(quality_alert == 0){
        Serial.print("INHABITABLE");
        //digitalWrite(LED_BUILTIN, LOW);
        servo.write(90);
        servoAngle = 90;
        quality_alert = 1;
      }
    }
    //Print out our reading every time
    Serial.print("DATA:");
    Serial.print(reading);
  }

  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();

    // say what you got:
    Serial.print("Azure Command Received: ");
    Serial.print(incomingByte, DEC);
    if(move_servo_from_azure == 1){
      servo.write(0);
      move_servo_from_azure = 0;
    }else{
      servo.write(180);
      move_servo_from_azure = 1;
    }
  }
  loop_count ++;
}
