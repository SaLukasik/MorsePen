/*BIBLIOTEKI*/

#include <WiFiNINA.h>
#include <SimpleTimer.h>
#include <Adafruit_SSD1306.h>
/*STAŁE*/
const int MAIN_SWITCH = A0;
const int LED_LOCAL = A1; //zapala się, gdy urządzenie nadaje
const int LED_STATUS = A6; //zapala się, gdy drugi użytkownik online
const int LED_OUT = A2; //zapala się gdy ktoś nadaje
const int SERVER = A3; //server high
char gString256[256];
int klik; //czy main_switch jest HIGH

/*DEBUGER*/
const int DBG_SERIAL_SPEED = 9600;
const int DBG_SERIAL_TYPE_USB = 0;
const int DBG_SERIAL_TYPE_UART = 1;
int gSerialType = DBG_SERIAL_TYPE_UART;
int gIsDebug = false;
/*======================= Debug port constants and variables =======================END*/
int gMainSwitchState = LOW;

/*WIFI USTAWIENIA*/
bool is_server;

char WIFI_SSID[] = "Nexus";
char WIFI_PASSWORD[] = "123456789.0";
int wifi_status = WL_IDLE_STATUS;
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

//TRYB USTAWIENIA
bool komunikacja;

/*timer*/
unsigned long time1 = 0;

void initializeSerial() {  //INICJALIZUJE DEBUG
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
  pinMode(MAIN_SWITCH, INPUT);
  pinMode(SERVER, INPUT);
  pinMode(LED_LOCAL, OUTPUT);
  pinMode(LED_OUT, OUTPUT);
  pinMode(LED_STATUS, OUTPUT);

  if (digitalRead(SERVER) == HIGH){
    is_server = true;
  }
  else{
    is_server = false;
  }
  
  /*debug*/
  initializeSerial();
  printAppHeader();

  if (digitalRead(MAIN_SWITCH) == HIGH){ //TRYB ZABAWY
    komunikacja = false;
  }

  else{ //TRYB KOMUNIKACJI
    komunikacja = true;
  }


}


  

void loop() {

       if (komunikacja == true){ //tryb komunikacji
      
        
                 main_switch_check();
                 if (WiFi.status() != WL_CONNECTED){
                        while(WiFi.status() != WL_CONNECTED){
                          connect_to_wifi();
                        }
                        
                        printlnDbgMsg("Node is connected to the network: OK");
                        IPAddress vIP = WiFi.localIP();   
                        if (is_server){ 
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
                 
                }

                
                if (is_server == true){
                        serverClient = server.available();
                        
                        if (serverClient) {
                          digitalWrite(LED_STATUS, HIGH);
                                char received = serverClient.read();
                                snprintf(gString256, 256, "Client request:%c",received);
                                printlnDbgMsg(gString256); 
                                if (received == 72){
                                  digitalWrite(LED_OUT, HIGH);
                                }
                                else{
                                  digitalWrite(LED_OUT, LOW);
                                }
                        }
                        else{
                          digitalWrite(LED_STATUS, LOW);
                        }
                        if (millis()-time1>10){
                          
                          if ( digitalRead(MAIN_SWITCH) == HIGH) {
                            printlnDbgMsg("Sending message to the client (H)...");   
                            server.print("H");
                            }
                          else {
                            printlnDbgMsg("Sending message to the client (L)...");   
                            server.print("L");
                          }
                          time1 = millis();
                        }
                        
                }
                else{                      
                  
                  
                          clientServer = client.available();
                          if (clientServer) {
                            digitalWrite(LED_STATUS, HIGH);
                            char received = client.read();
                            snprintf(gString256, 256, "Server response: %c",received);
                            printlnDbgMsg(gString256); 

                                if (received == 72){
                                  digitalWrite(LED_OUT, HIGH);
                                }
                                else if (received == 76){
                                  digitalWrite(LED_OUT, LOW);
                                }

                            
                          }
                          else{ 
                            digitalWrite(LED_STATUS, LOW); 
                          }
                              
                  
                        if (millis()-time1>10){
                          printlnDbgMsg("KLIKNIĘTO");  
                          if ( digitalRead(MAIN_SWITCH) == HIGH) {
                            printlnDbgMsg("Sending message to the client (H)...");   
                            client.print("H");
                            }
                          else {
                            printlnDbgMsg("Sending message to the client (L)...");   
                            client.print("L");
                          }
                          time1 = millis();
                        }
                      
                  
                  
                  
                }
            
                
                klik = main_switch_check();
                
                
            
      }
        else{ //tryb zabawy
        
        }
      
        // put your main code here, to run repeatedly:
      

}







void connect_to_wifi(){
  printlnDbgMsg("conenecting to wifi");


  if (is_server == true) WiFi.config(gServerIp);
  else WiFi.config(gClientIp);  
  wifi_status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  
  int blue = 0;
  for (int i = 0; i < 1000; ++i)
  { 
    if (blue == 50){
      digitalWrite(LED_STATUS, HIGH);
    }
    if (blue == 100){
      digitalWrite(LED_STATUS, LOW);
      blue = 0;
    }
    main_switch_check();
    delay(10);
    blue+=1;
  }
  digitalWrite(LED_STATUS, LOW);
  digitalWrite(LED_LOCAL, LOW);

  
}


void printAppHeader() {
  printlnDbgMsg("=========================================");
  if (is_server) printlnDbgMsg("=== MorsePen - SERVER by Sara Lukasik ===");
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

int main_switch_check(){

   if (digitalRead(MAIN_SWITCH) == HIGH){ 
    digitalWrite(LED_LOCAL, HIGH);
    return 1;
  }
  else{
    digitalWrite(LED_LOCAL, LOW);
    return 0;
  }
}
