#include <SPI.h>
#include <WiFiNINA.h>
#include <Adafruit_SSD1306.h>

/* MorsePen - v.0.0.1       */
/* HW: Arduino NANO 33 IoT  */
/* By: Sara Lukasik         */

/*======================= General purpose constants and varialbles ===================*/


/*======================= General purpose constants and varialbles ================END*/
const int MAIN_SWITCH_PIN_IN_A0 = A0;
const int RED_LED_PIN_OUT_A1 = A1;
const int BLUE_LED_PIN_OUT = A6;
const int GREEN_LED_PIN_OUT_A2 = A2;
const int IS_SERVER_PIN_IN_A3 = A3;
/*======================= Debug port constants and variables ==========================*/
const int DBG_SERIAL_SPEED = 9600;
const int DBG_SERIAL_TYPE_USB = 0;
const int DBG_SERIAL_TYPE_UART = 1;
int gSerialType = DBG_SERIAL_TYPE_UART;
int gIsDebug = true;
/*======================= Debug port constants and variables =======================END*/
int gMainSwitchState = LOW;

/*======================= WIFI related constants and variables ========================*/
bool gIsServer = false;

//Wifi configuration
char WIFI_SSID[] = "Nexus";
char WIFI_PASSWORD[] = "123456789.0";
int gWifiStatus = WL_IDLE_STATUS;
//Server IP address
IPAddress gServerIp(192, 168, 43, 200);
const int SERVER_PORT = 5555;
//Client IP address
IPAddress gClientIp(192, 168, 43, 201);
//Objects to handle communication - server side
WiFiServer server(SERVER_PORT);
WiFiClient serverClient;
//Objects to handle communication - clinet side
WiFiClient client;
WiFiClient clientServer;
boolean gIsClientConnected = false;
const int INTER_CONNECTION_TIME = 1000;

/*======================= WIFI related constants and variables =====================END*/
/*==================== OLED DISPLAY related constants and variables ===================*/
const int OLED_ADDRESS = 0x3C;
const int SCREEN_WIDTH = 128; // OLED display width, in pixels
const int SCREEN_HEIGHT = 32; // OLED display height, in pixels
Adafruit_SSD1306 gDisplay(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
/*==================== OLED DISPLAY related constants and variables ================END*/

//Global char buffer
char gString256[256];

void initializeSerial() {
  if (true == gIsDebug) {
    if (DBG_SERIAL_TYPE_USB == gSerialType) {
      Serial.begin(DBG_SERIAL_SPEED);
      while (!Serial) {};
    }
    else {
      Serial1.begin(DBG_SERIAL_SPEED);
      while (!Serial1) {};
    }
  }
}

void setup() {
  //============================================== Configure uC pins
  pinMode(MAIN_SWITCH_PIN_IN_A0, INPUT);
  pinMode(IS_SERVER_PIN_IN_A3, INPUT);
  pinMode(RED_LED_PIN_OUT_A1, OUTPUT);
  pinMode(BLUE_LED_PIN_OUT, OUTPUT);
  pinMode(GREEN_LED_PIN_OUT_A2, OUTPUT);
  //Reading configuration pins
  //============================================= Recognizing Wifi server/ client node
  if (HIGH == digitalRead(IS_SERVER_PIN_IN_A3)) gIsServer = true;
  else gIsServer = false;
  //============================================== Initializing debug port
  initializeSerial();
  printAppHeader();
  //============================================== Initialize OLED display
  gDisplay.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
  gDisplay.clearDisplay();
  gDisplay.setTextSize(1);
  gDisplay.setTextColor(WHITE, BLACK);
  gDisplay.setCursor(1, 1);
  //============================================== Initialize WiFi module
  if (WiFi.status() == WL_NO_MODULE) printlnDbgMsg("Communication with WiFi module failed!");
  String vFirmVer = WiFi.firmwareVersion();
  if (vFirmVer < WIFI_FIRMWARE_LATEST_VERSION) {
    snprintf(gString256, 256, "Please upgrade the Wifi firmware (current version:%d)",vFirmVer);
  }
  else {
    snprintf(gString256, 256, "Wifi firmware (current version:%d): OK",vFirmVer) ;
  }
  printlnDbgMsg(gString256);
  // attempt to connect to WiFi network:
  while (gWifiStatus != WL_CONNECTED) {
    //Configure IP address
    if (gIsServer) WiFi.config(gServerIp);
    else WiFi.config(gClientIp);  
    printDbgMsg("Attempting to connect to WPA SSID: ");
    printlnDbgMsg(WIFI_SSID);
    // Connect to WPA/WPA2 network:   
    gWifiStatus = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    // wait 10 seconds for connection:
    delay(10000);
  }
  //The node is connected now
  printlnDbgMsg("Node is connected to the network: OK");

  IPAddress vIP = WiFi.localIP();                    // the IP address of your shield
  snprintf(gString256, 256, "IP address: %d:%d:%d:%d",vIP[0],vIP[1],vIP[2],vIP[3]);
  printlnDbgMsg(gString256);
  if (gIsServer){ 
      printlnDbgMsg("Starting server ...");
      server.begin();
      printlnDbgMsg("Server has been started: OK");
  }
  else {
      gIsClientConnected = false;
      while (!gIsClientConnected) {
        printlnDbgMsg("Connecting to the server ...");
        client.connect(gServerIp, SERVER_PORT);
        delay(INTER_CONNECTION_TIME);
        gIsClientConnected = client.connected();
      }
      printlnDbgMsg("Client has been connected to the server: OK");   
  }
  
  //Blink green LED
  digitalWrite(GREEN_LED_PIN_OUT_A2, HIGH);
  delay(1000);
  digitalWrite(GREEN_LED_PIN_OUT_A2, LOW);
  digitalWrite(BLUE_LED_PIN_OUT, HIGH);

  
}

void loop() {
  char vServerResponse;
  char vClientRequest;
  if (!gIsServer) { //Client is sending message 
    //Send state of the switch to server
    if ( digitalRead(MAIN_SWITCH_PIN_IN_A0) == HIGH) {
      printlnDbgMsg("Sending message to the server (H)...");   
      client.print("H");
    }
    else {
      printlnDbgMsg("Sending message to the server (L)...");   
      client.print("L");
    }
    //Receive data from Server if available
    clientServer = client.available();
    if (clientServer) {
      vServerResponse = client.read();
      snprintf(gString256, 256, "Server response: %c",vServerResponse);
      printlnDbgMsg(gString256); 
    }
  }
  else { //Server is receiving
    if ( digitalRead(MAIN_SWITCH_PIN_IN_A0) == HIGH) {
      printlnDbgMsg("Sending message to the client (H)...");   
      server.print("H");
    }
    else {
      printlnDbgMsg("Sending message to the client (L)...");   
      server.print("L");
    }
    serverClient = server.available();
    if (serverClient) {
      vClientRequest = serverClient.read();
      snprintf(gString256, 256, "Client request:%c",vClientRequest);
      printlnDbgMsg(gString256); 
    }
  }
  microswitchToDisplay();
}

void microswitchToDisplay() {
  if ( digitalRead(MAIN_SWITCH_PIN_IN_A0) == HIGH){
    digitalWrite(RED_LED_PIN_OUT_A1, HIGH);
    gDisplay.clearDisplay();
    gDisplay.setCursor(1, 1);
    gDisplay.println("Microsw is pressed");
    gDisplay.display();
  }
  else{
    digitalWrite(RED_LED_PIN_OUT_A1, LOW);
    gDisplay.clearDisplay();
    gDisplay.setCursor(1, 1);
    gDisplay.println("Microsw is released");
    gDisplay.display();
  }
}
void printAppHeader() {
  printlnDbgMsg("=========================================");
  if (gIsServer) printlnDbgMsg("=== MorsePen - SERVER by Sara Lukasik ===");
  else printlnDbgMsg("=== MorsePen - CLIENT by Sara Lukasik ===");
  printlnDbgMsg("=========================================");
}

void printlnDbgMsg(char *pMsg) {
  if (true == gIsDebug) {
    if (DBG_SERIAL_TYPE_USB == gSerialType) Serial.println(pMsg);
    else Serial1.println(pMsg);
  }
}

void printDbgMsg(char *pMsg) {
  if (true == gIsDebug) {
    if (DBG_SERIAL_TYPE_USB == gSerialType) Serial.print(pMsg);
    else Serial1.print(pMsg);
  }
}
