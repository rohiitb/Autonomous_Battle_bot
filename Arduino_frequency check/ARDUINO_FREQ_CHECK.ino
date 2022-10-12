void setup() {
    
  pinMode(11, INPUT);    //Input from IR receiver
  pinMode(10, OUTPUT);    //Output to ESP32

}

void loop() {
     digitalWrite(10,LOW);
     int t1 = pulseIn(11,HIGH);    //Calculating the frequency
     int t2 = pulseIn(11,LOW);
     int t=t1+t2;
     int freq = 1000000/t;
     if (freq>680 && freq<720){digitalWrite(10,HIGH); delay(5);}   //For 700 Hz
      
}
