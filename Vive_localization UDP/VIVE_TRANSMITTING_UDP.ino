#include <WiFi.h>
#include <WiFiUdp.h>

#include "vive510.h"
#define SIGNALPIN1 23         //pin receiving signal from Vive circuit

Vive510 vive1(SIGNALPIN1);

const char* ssid     = "TP-Link_05AF";
const char* password = "47543454";

WiFiUDP UDPTestServer;
unsigned int udpTargetPort=2510;   //Any port# upto 65535 will work
const int UDP_PACKET_SIZE = 14;       //Packet size of udpBuffer
char udpBuffer[UDP_PACKET_SIZE];      //String which we send as the packet
IPAddress ipTarget(192, 168, 1, 255 );       //Target esp32 IP ADDRESS
IPAddress ipLocal(192, 168, 1, 110);         //Local esp32 IP ADDRESS

void setup() {
  Serial.begin(115200);  

  //Declaring the pinMode for Dipswitch
  pinMode(26,INPUT_PULLUP);     
  pinMode(32,INPUT_PULLUP);
  pinMode(33,INPUT_PULLUP);
  pinMode(27,INPUT_PULLUP);
  
  vive1.begin(SIGNALPIN1);
  Serial.println("Vive trackers started");
  
  Serial.print("Connecting to ");  Serial.println(ssid);
  
  WiFi.config(ipLocal, IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));     //WIFI connection
  WiFi.begin(ssid, password);

  UDPTestServer.begin(udpTargetPort);      //strange bug needs to come after WiFi.begin but before connect
  
  while(WiFi.status()!=WL_CONNECTED){         //Connecting WiFi
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected as "); Serial.print(WiFi.localIP());
}

// send udp packets periodically                                                         
void loop() {
  
  if (vive1.status() == VIVE_LOCKEDON ) {
    Serial.printf("X1 %d, Y1 %d\n",vive1.xCoord(),vive1.yCoord());
    char s[13];
  int x, y; // some data, like xy position
  // store into a string with format #:####,####, which is robotid, x, y 
  
  int a;           //Variable to store the id of the bot
  
  if (digitalRead(26)==0) a=1;
  if (digitalRead(32)==0) a=2;
  if (digitalRead(33)==0) a=3;
  if (digitalRead(27)==0) a=4;
    
  x = vive1.xCoord();                  //X location of bot
  y = vive1.yCoord();                  //Y location of bot
  sprintf(s,"%1d:%4d,%4d",a, x, y);
  fncUdpSend(s,13);                    //Send the location and id
  Serial.printf("sending data: %s",s); 
  delay(100);
  }
  else {
    vive1.sync(15);     //Sync the vive
    delay(15);};

}

void fncUdpSend(char *datastr, int len)    //Function to send id and location of bot
{
                                         
  UDPTestServer.beginPacket(ipTarget, udpTargetPort);   //send to udpTarget port
  UDPTestServer.printf("%s",datastr);       //Send the string
  UDPTestServer.endPacket();
  
}
