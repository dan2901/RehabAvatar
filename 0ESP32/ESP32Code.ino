// SCL on MPU9250 - D22 on ESP32
// SCL - D22 
// SDA - D21
// AD0 - GND
// VCC - 3V3
// GND - GND

//sorry if the explanations are too thorough

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <Wire.h>
#include "paginaWeb.h"
#include <WebSocketsClient.h>

//here, Reg means register
#define LPF10Hz 0x1D 
#define rangeReg 0x1C //register for setting accelerometer reading intervale
#define wakeReg 0x6B
#define clearingReg 0x6A
#define resetReg 0x68

const char* ssid = ""; //put here your WiFi network's ID
const char* password = ""; //put here your WiFi network's passwork

AsyncWebServer server(80); //async server on port 80 where ESP32 serves the wep page
WebSocketsServer webSocket = WebSocketsServer(81); //web socket server on port 81 where data from accelerometer is served live
WebSocketsClient webSocketC; //variable of WebSocketsClient type, named webSocketC

const char* websocket_proxy = "172.20.10.2"; //here you put your laptop's local IP adress (the proxy server in our project)
const uint16_t websocket_port = 8081;

String JSONtxt; //variable of string type, named JSONtxt

int MPU9250 = 0x68; //variable of integer type, named MPU9250

int i;
int16_t ax, ay, az;
float ax2, ay2, ay2ms, az2, az2ms, am, rotX, rotY, myRotY, sole; //ik maybe there are too many variables

//managing connections to the ESP32 through the websocket
//i.e. user of the project connects to ESP32 to access the web page
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
        
        String welcome = "{\"status\":\"connected\"}";
        webSocket.sendTXT(num, welcome);
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] Text: %s\n", num, payload);
      break;
  }
}

//managing the connection ESP32 does as a client
//i.e. ESP32 connects to the proxy server (laptop) to send data for encryption
void webSocketClientEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.println("WebSocket to proxy disconnected");
      break;
    case WStype_CONNECTED:
      Serial.println("WebSocket to proxy connected!");
      break;
    case WStype_TEXT:
      Serial.printf("Received from proxy: %s\n", payload);
      break;
  }
}

//function for writing to Accelerometer registers
void codeAccel(uint8_t reg, uint8_t val){
  Wire.beginTransmission(MPU9250);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission(true);
}

//reading accelerometer data
void accel_readings(void) {
  //LPF 10Hz
  codeAccel(LPF10Hz, 0b00000101);

  // +-2g
  codeAccel(rangeReg, 0);

  //actual reading
  Wire.beginTransmission(MPU9250);
  Wire.write(0x3B); //ACCEL_XOUT_H register
  Wire.endTransmission(false);
  Wire.requestFrom(MPU9250, 6); //we read from 6 register in a row
  //ax is a 16bit integer, but data from accel register is 8 bits
  //so we read 8 bits, put them into a 16 bit variable
  //move the whole 8 bits fully to the left
  //then add another 8 bits to ax, thus filling the whole 16 bits
  ax = (Wire.read() << 8 | Wire.read() ); 
  ay = (Wire.read() << 8 | Wire.read() ); 
  az = (Wire.read() << 8 | Wire.read() ); 

  ax2 = (ax/16384.0); //datasheet conversion factor
  ay2 = (ay/16384.0); //here we turn raw values into g
  az2 = (az/16384.0); 

  az2ms = (az2*9.80665); //g -> m/s2 
  ay2ms = (ay2*9.80665); 
 
  //calculate resultant acceleration out of its vertical and horizontal components
  //I chose a 2D plane to analyse gait
  am = sqrt(pow(ay2ms, 2) + pow(az2ms, 2));

  //calculate roll and pitch of accelerometer
  rotX = atan(ay2/sqrt(pow(ax2,2)+pow(az2,2)))*180/PI; //roll  
  rotY = (atan(-1*ax2/sqrt(pow(ay2,2)+pow(az2,2)))*180/PI); //pitch
}

void setup() {
  Serial.begin(115200);
  pinMode(33, INPUT);

  WiFi.begin(ssid, password); //connecting to a random WebPage

  //when a client (meaning user of the web page) connect to root folder "/" of our web project
  //ESP32 sends the status 200 (OK), and serves the webpage to the client as a html file
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request ->send(200, "text/html", pageContent);
  });
  
  webSocket.begin(); //generating the server on port81
  webSocket.onEvent(webSocketEvent); //function call for managing connections to ESP32

  webSocketC.begin(websocket_proxy, websocket_port, "/"); //accessing proxy server's root folder
  webSocketC.onEvent(webSocketClientEvent); //function call for managing connection to proxy server as a client
  webSocketC.setReconnectInterval(5000);

  server.begin(); //generating the server on port80

  Wire.begin(); //initializing i2c communication between ESP32 and Accelerometer
  Wire.setClock(400000); //i2c frequency
  delay(250); //time for setup

  //setting accel in wake mode
  codeAccel(wakeReg, 0);

  //reseting accelerometer and gyroscope registers
  codeAccel(clearingReg, 1);
  codeAccel(resetReg, 0b00000110);
}

void loop() {
  webSocket.loop(); //continuously sending data on port81
  webSocketC.loop(); //continuously sending data on port8081 (to proxy server)
  accel_readings(); //calling function to read accel data

  //because of the particular position the accelerometer has on my leg
  //I need to do a couple offsets, thus reading the value 0 when resting
  rotY = rotY - 85;
  
  if(az2 > 0) rotY = 0 - rotY;
  myRotY = -rotY;
  
  if(abs(myRotY) > 65 ){
    rotX = rotX + 71;
  }else{
    rotX = 0;
  }

  if(ay2ms < -1.5) myRotY = 0;

  //create some string variables where we store different data we want to send to our client
  String sentZAccel = String(az2ms);
  String sentYAccel = String(ay2ms);
  String sentRoll = String(rotX);
  String sentPitch = String(myRotY);
  
  //here we bascially create an JSON object 
  //ex. {"POT":"1023"}, they are called key-value pairs
  //JSONtxt = "{\"ABS\":\""+sentAbsAccel+"\"}";
  JSONtxt = "{\"Zaccel\":\"" +sentZAccel+ "\",\"Yaccel\":\"" +sentYAccel+ "\",\"Pitch\":\"" +sentPitch+ "\",\"Roll\":\"" +sentRoll+ "\"}";
  //whenever we add double quotes inside a string, we need to escape it by putting \ in front
  
  webSocket.broadcastTXT(JSONtxt); //ESP32 sending the JSON object to all the clients connected in a non secure manner

  if (webSocketC.isConnected()){ //ESP32 sends the JSON object to the proxy server, also in a non secure manner
    webSocketC.sendTXT(JSONtxt);
  }

  delay(10);
}