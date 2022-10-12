#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid     = "TP-Link_05AF";
const char* password = "47543454";

WiFiUDP UDPTestServer;            //UDP Server for sending the commands
unsigned int udpTargetPort = 2808;    //Port for receiving the command
const int UDP_PACKET_SIZE = 14;       //Packet size of udpBuffer
char udpBuffer[UDP_PACKET_SIZE];      //String which we send as the packet
IPAddress ipTarget(192, 168, 1, 108 );       //Target esp32 IP ADDRESS
IPAddress ipLocal(192, 168, 1, 137);         //Local esp32 IP ADDRESS

void setup() {
  Serial.begin(115200);  

 //PinModes for all the switches for controlling and functionality
  pinMode(5,INPUT);       //Moving the bot sideways
  pinMode(10,INPUT);
  
  pinMode(15,INPUT);     //Moving the bot forward
  pinMode(12,INPUT);
  
  pinMode(22,INPUT);     //Vive Tracking
  pinMode(19,INPUT);     //Wall follow
  pinMode(33, INPUT);    //Beacon function
  pinMode(23, INPUT);    //Gripper

  Serial.print("Connecting to ");  Serial.println(ssid);
  
  WiFi.config(ipLocal, IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));
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
  int a,x,y;     //variables which are to be sent
  char s[13];     //Command is stored in string
  
  if (digitalRead(12)==HIGH) a=4;      //Forward
  if (digitalRead(15)==HIGH) a=3;      //Backward
  if (digitalRead(10)==HIGH) a=2;      //Left
  if (digitalRead(5)==HIGH) a=1;       //Right
  
  if (digitalRead(19)==HIGH) a=10;    //Wallfollow
  if (digitalRead(22)==HIGH) a=11;    //Vive Track
  if (digitalRead(33)==HIGH) a=12;    //Beacon 
  if (digitalRead(23)==HIGH) a=13;    //Gripper on
  if (digitalRead(18)==HIGH) a=14;    //Gripper off
  
  if(digitalRead(12)==0 && digitalRead(15)==0 && digitalRead(10)==0 && digitalRead(5)==0 && digitalRead(19)==0 && digitalRead(22)==0) a=0;   //Send 0 command by if nothing is pressed
  
  x = 1000;    //Send x as 1000
  y = 1000;    //Send y as 1000
  
  sprintf(s,"%1d:%4d,%4d",a, x, y);
  fncUdpSend(s,13);              //Send the command
  Serial.printf("sending data: %s",s);
  delay(100);
}

void fncUdpSend(char *datastr, int len)    //Function to send command to the bot
{                                  
  UDPTestServer.beginPacket(ipTarget, udpTargetPort);   //send to udpTarget port
  UDPTestServer.printf("%s",datastr);
  UDPTestServer.endPacket();
}
