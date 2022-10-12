
#include <WiFi.h>       //WIFI library
#include <WiFiUdp.h>    
#include <Servo.h>       //Servo library

#include "vive510.h"

#define SIGNALPIN1 19     //Vive input pin

Vive510 vive1(SIGNALPIN1);     //Creating instance for Vive

const char* ssid     = "TP-Link_05AF";     //Router SSID
const char* password = "47543454";         //Router password

WiFiUDP UDPTestServer;              //UDP server for communication between controller and bot
unsigned int UDPPort = 2808;         //Using UDP port for receiving data from controller
IPAddress myIPaddress(192, 168, 1, 108);            //UDP Receiver IP address

const int UDP_PACKET_SIZE = 14;         //Packet size for UDP message
uint8_t packetBuffer[UDP_PACKET_SIZE+1];    //Packet buffer

WiFiUDP UDPCanServer;         //UDP server for communication between can and bot
unsigned int UDPPortCan = 1510;         //Using UDP Can port for receiving can location
   
const int UDP_PACKET_SIZE_CAN = 14;         //Packet size for UDP message for can location data
uint8_t packetBufferCan[UDP_PACKET_SIZE+1];     //Packet buffer

Servo myservo;            //Creating servomotor instance
Servo yourservo;            //Creating servomotor instance for gripper

#define echoPin1 21     //Front Ultrasonic sensor echoPin input 
#define trigPin1 22     //Front Ultrasonic sensor TriggerPin input 
#define echoPin2 18     //Side Ultrasonic sensor echoPin input
#define trigPin2 23     //Side Ultrasonic sensor TriggerPin input

void setup() {
  Serial.begin(115200);
  
  WiFi.config(myIPaddress, IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));   //WIFI begin
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected"); 
  
  vive1.begin(SIGNALPIN1);    //Initiate VIVE tracking
  
  Serial.println("Vive trackers started"); 

  myservo.attach(32);    //Servo motor PWM pin
  myservo.attach(33);    //Servo motor PWM pin
    
  pinMode(2,OUTPUT);      //stepper motor direction pin
  pinMode(4,OUTPUT);      //stepper motor Pulse pin
  pinMode(9,OUTPUT);      //stepper motor direction pin

  pinMode(trigPin1, OUTPUT);   //Ultrasonic sensor trigger pin(front)
  pinMode(echoPin1, INPUT);   //Ultrasonic sensor echo input pin(front)
  pinMode(trigPin2, OUTPUT);   //Ultrasonic sensor trigger pin(side)
  pinMode(echoPin2, INPUT);    //Ultrasonic sensor echo input pin(side)

  pinMode(5, INPUT);        //Input from Arduino

  UDPTestServer.begin(UDPPort);     //Begin UDP Bot server
  UDPCanServer.begin(UDPPortCan);    //Begin UDP Can server
     
  packetBuffer[UDP_PACKET_SIZE] = 0;    //null terminate buffer
  packetBufferCan[UDP_PACKET_SIZE] = 0;   //null terminate buffer
}

//Function to give X location of Can
int handleUDPServer_x() {      
  int cb = UDPCanServer.parsePacket();
  int x;
  UDPCanServer.read(packetBufferCan, UDP_PACKET_SIZE);
  x = atoi((char *)packetBufferCan+2);         //#,####,#### 2nd indexed char
  return x;
  }

//Function to give Y location of Can
int handleUDPServer_y() {
  int cb = UDPCanServer.parsePacket();
  int y;
  UDPCanServer.read(packetBufferCan, UDP_PACKET_SIZE);
  y = atoi((char *)packetBufferCan+7);          //#,####,#### 7th indexed char
  return y;
 }

//Function which returns distance between two points
int check_distance(int x_target, int y_target, int x_current, int y_current){
  int dist = 0;
  dist =  sqrt((pow(x_target-x_current,2)) + (pow(y_target-y_current,2))) ;
  return dist  ;
}

//Function to move the bot in forward direction
void forward(){
int i = 0;
 digitalWrite(2,1);   //Direction for stepper 1
 digitalWrite(9,0);   //Direction for stepper 2
 
 for (i=0;i<300;i++){
    digitalWrite(4,1);    //Pulse High for both steppers
    delayMicroseconds(900);   //Delay to control the speed
    digitalWrite(4,0);    //Pulse low for both steppers
    delayMicroseconds(900);
  }  
}

void left(int angle){
int i = 0;
 digitalWrite(2,0);
 digitalWrite(9,0);
  for (i=0;i<angle*1.15;i++){
    digitalWrite(4,1);
    delayMicroseconds(900);
    digitalWrite(4,0);
    delayMicroseconds(900);
  }  
}

void right(int angle){
int i = 0;
 digitalWrite(2,1);
 digitalWrite(9,1);
  for (i=0;i<angle*1.15;i++){
    digitalWrite(4,1);
    delayMicroseconds(900);
    digitalWrite(4,0);
    delayMicroseconds(900);
  }  
}


void back(){
int i = 0;
 digitalWrite(2,0);
 digitalWrite(9,1);
  for (i=0;i<300;i++){
    digitalWrite(4,1);
    delayMicroseconds(900);
    digitalWrite(4,0);
    delayMicroseconds(900);
  }  
}

void vive_track(){
  int rdist=9999;   //Declaring distance as 9999 by default 
  int temp = 0;     //Variable to store the distance temporarily
  int k=90,        //Variable to store the angle
  int x_co, y_co;   //Variable to get X,Y locations of cans

  x_co = handleUDPServer_x();    //X location of Can from function
  y_co = handleUDPServer_y();    //Y location of Can from function
   
  for(int jk=0;jk<180;jk++){    
    myservo.write(jk);   //Rotate servo from 0 to 180 
    delay(10);
    temp = check_distance(x_co, y_co, vive1.xCoord(), vive1.yCoord());  //Store distance at each point in temp variable
      
  if(temp<rdist){   //If distance is minimum, store in rdist
    rdist=temp; 
    k=jk;         //Store the angle corresponding to min distance in variable k
    }
  }
 
   delay(500); myservo.write(k); delay(1000);     //Move the servo to the angle corresponding to min distance
   if(k<20){right(10);forwardwall(70);}           //If angle less than 20, move right
   if(k<90 && k>20){right(90-k);forwardwall(70);}    //If angle between 20 and 90, move right 
   if(k>90){left(k-90);forward();}                  //If angle greater than 90, move left
   if(k>=85&&k<=95){forwardwall(100);}              //If angle between 85 and 95, move forward
   Serial.printf("Min distance %d, %d ",rdist,k);  //Print Min distance and angle

}

//Function to receive command from controller
int udpReceive(){
  int cb = UDPTestServer.parsePacket();
  int df;                                //variable to store the command
  UDPTestServer.read(packetBuffer, UDP_PACKET_SIZE);
  df = atoi((char *)packetBuffer); // #,####,#### 2nd indexed char
  Serial.printf("Command = %d",df);
  return df;
}

void loop() {
  if (vive1.status() == VIVE_LOCKEDON ) {
    Serial.printf("X1 %d, Y1 %d\n",vive1.xCoord(),vive1.yCoord());
    char s[13];
  int x, y;        //Declaring variables for current X,Y locations
    
  x = vive1.xCoord();      //Current X location of bot
  y = vive1.yCoord();      //Current Y location of bot
  delay(100);
  }
  else {
    vive1.sync(15);       //Sync bot
    delay(15);};

  
int gh = udpReceive();     //Store the command received in variable

   if(gh==4){                        //Move the bot FORWARD
    for(int i=0;i<=50;i++){
      digitalWrite(2, 1);
      digitalWrite(9, 0);  
      
      digitalWrite(4,1);
      delayMicroseconds(1000);
      digitalWrite(4,0);
      delayMicroseconds(1000);
      }
    }

   if(gh==3){                       //Move the bot BACKWARD
    for(int i=0;i<=50;i++){
      digitalWrite(2, 0);
      digitalWrite(9, 1);  
      
      digitalWrite(4,1);
      delayMicroseconds(1000);
      digitalWrite(4,0);
      delayMicroseconds(1000);
      }
      
    }
    
   if(gh==10){                       //Call the function Wall follow
     wallfollow();
    }

    if(gh==12){                       //Call the function Beacon
     beacon();
    }

   if(gh==11 ){                      //Call the function for vive tracking
      if(vive1.status() == VIVE_LOCKEDON)
            vive_track();
    } else {vive1.sync(15);}

   if(gh==2){                           //Move the bot LEFT
      
      for(int i=0;i<=100;i++){
      digitalWrite(2, 0);
      digitalWrite(9, 0);
        
      digitalWrite(4,1);
      delayMicroseconds(1000);
      digitalWrite(4,0);
      delayMicroseconds(1000);
      }
      
    }

    if(gh==1){                            //Move the bot RIGHT
      for(int i=0;i<=100;i++){
      digitalWrite(2, 1); 
      digitalWrite(9, 1); 
      
      digitalWrite(4,1);
      delayMicroseconds(1000);
      digitalWrite(4,0);
      delayMicroseconds(1000);
      }
    }

    if(gh==0){                         //STOP the bot
      
      digitalWrite(2, 0); 
      digitalWrite(9, 0); 
      digitalWrite(4,0);
      delayMicroseconds(800);
            
    }

     if(gh==13){
      yourservo.write(155);      
    }

    if(gh==141){
       yourservo.write(60);     
    }
  }

//Function for Wall-following
void wallfollow(){
  long duration1;   //variables for ultrasonic distance timing(front)
  long duration2;   //variables for ultrasonic distance timing(side)
    
  int distance1;   //Side distance
  int distance2;    //Front distance
  
  int avg1 = 0;   //Variable for averaging distance(side)
  int avg2 = 0;   //Variable for averaging distance(front)
  
  int n = 0;
 
  for(int l=0; l<5; l++){       //Creating FOR loop to take multiple readings to reduce noise by taking average
    
    digitalWrite(trigPin1, LOW);           //Side ultrasonic sensor triggering
    delayMicroseconds(2);
    digitalWrite(trigPin1, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin1, LOW);

    duration1 = pulseIn(echoPin1, HIGH);    //Echo time receiving
    distance1 = duration1 * 0.034 / 2;      //Side distance calculation
    avg1 = avg1 + distance1;                //Adding distance

    digitalWrite(trigPin2, LOW);           //Front ultrasonic sensor triggering
    delayMicroseconds(2);
    digitalWrite(trigPin2, HIGH); 
    delayMicroseconds(10);
    digitalWrite(trigPin2, LOW);

    duration2 = pulseIn(echoPin2, HIGH);       //Echo time receiving
    distance2 = duration2 * 0.034 / 2;        //Front distance calculation
    avg2 = avg2 + distance2;                //Adding distance
    
    n = n + 1;
  }
  
  int r = avg1/n;            //Averaging the total distance to reduce noise(Side)
  int f = avg2/n;            //Averaging the total distance to reduce noise(Front)

  if(f>25){ forwardwall(250);     //If forward_distance>25, move forward And check side distance
    if(r<15) leftwall(30);             //If distance<15, move left
    if(r>20) rightwall(15);            //If distance>20, move right
  }
  if(f<25){ leftwall(95);}         //If forward_distance>25, move forward And then left
  }

//Function for moving forward while wall-following
void forwardwall(int s){
  int i = 0;
  digitalWrite(2,1);         //Direction pins for stepper motors
  digitalWrite(9,0);
  for (i=0;i<s;i++){
    digitalWrite(4,1);
    delayMicroseconds(1200);
    digitalWrite(4,0);
    delayMicroseconds(1200);
  }  
}

//Function for moving leftward while wall-following 
void leftwall(int angle){
  int i = 0;
  digitalWrite(2,0);    //Direction pins for stepper motors
  digitalWrite(9,0);
  for (i=0;i<angle*1.16;i++){
    digitalWrite(4,1);
    delayMicroseconds(1000);
    digitalWrite(4,0);
    delayMicroseconds(1000);
  }  
}

//Function for moving rightward while wall-following 
void rightwall(int angle){
  int i = 0;
  digitalWrite(2,1);           //Direction pins for stepper motors
  digitalWrite(9,1);
  for (i=0;i<angle*1.16;i++){
    digitalWrite(4,1);
    delayMicroseconds(1000);
    digitalWrite(4,0);
    delayMicroseconds(1000);
  }  
}

//Function for beacon-Tracking
void beacon() {
   digitalWrite(4,0);
  int i=0;
  int u=0;
  for(i=0;i<180;i++){         //Rotate the servo form 0 to 180 degrees
      myservo.write(i);
      delay(6);
      
      if(digitalRead(5)==HIGH){   //if Arduino input is high, that is, if beacon spotted....
        if(i<80){right(90-i);}     //if angle less than 80, turn the bot right by the respective angle
        if(i>100){left(i-90);}     //if angle more than 100, turn the bot left by the respective angle
        if(i>=80&&i<=100){forward();}      //if angle less between 80 and 100, move the bot forward
        u=1;
        delay(100);
      }
    }   
    
 for(int i=180;i>0;i--){     //repeat above operations from 180 to 0 degrees
    myservo.write(i);
      delay(6);
      
      if(digitalRead(5)==HIGH){
        if(i<80){right(90-i);}
        if(i>100){left(i-90);}
        if(i>=80&&i<=100){forward();}
        u=1;
        delay(100);
      }
   }
   for(i=0;i<180;i++){   //Repeat above operations(twice)
      myservo.write(i);
      delay(6);
      
      if(digitalRead(5)==HIGH){
        if(i<80){right(90-i);}
        if(i>100){left(i-90);}
        if(i>=80&&i<=100){forward();}
        u=1;
        delay(100);
      }
    }   
    
 for(int i=180;i>0;i--){      //repeat above operations(twice)
    myservo.write(i);
      delay(6);
      
      if(digitalRead(5)==HIGH){
        if(i<80){right(90-i);}
        if(i>100){left(i-90);}
        if(i>=80&&i<=100){forward();}
        u=1;
        delay(100);
      }
   }
   if(u==0){right(180);}       //Turn the bot 180 degrees if beacon not found
}

  




      
