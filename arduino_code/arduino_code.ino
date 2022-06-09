#include <Blynk.h>
#include <Servo.h>
#include <string.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>


#define BLYNK_PRINT Serial // This prints to Serial Monitor

Servo Servo2;

long x;
int pos_int = 60;
int pos ;
int posw;
int STEP = D1;
int DIR = D2;

int count = 0;

String BUFFER;
String ch1;
String ch2;
int mytimeout = millis() / 1000;
int ReCnctFlag;
int ReCnctCount = 0;


char auth[] = "SWrUnZ8g1mp9O2adE70ditKDEdDtePAM";
char ssid[] = "Ductri";
char pass[] = "Abc123@@";

BlynkTimer timer;

  

void SendNotifcation()
{
  bool isOnFire = (ch1 != "");
  if (isOnFire ==  true)
  {
    Serial.println("Fire warning!");
    Blynk.notify("Fire warning");
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, pass);     // Connect to the network
  Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED && i<10) // Wait for the Wi-Fi to connect
  {
    delay(1000);
    Serial.print(++i); Serial.print(' '); 
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Still not connected after 10 seconds!");
  }

  Blynk.config(auth,"blynk-server.com",8080);
  Blynk.connect();

  timer.setInterval(1000L, UpTime);
   
  pinMode(STEP,OUTPUT); // Step pin
  pinMode(DIR,OUTPUT  ); // Dir - pin
  Servo2.attach(D7,500,2500);
  pinMode(D6, OUTPUT);
  digitalWrite(D6, LOW);
}

BLYNK_CONNECTED() 
{ Blynk.syncAll();
  Serial.println("Connected");
  ReCnctCount = 0;
}
void UpTime() {
  Serial.print("UpTime: ");
  Serial.println(millis() / 1000);  // Send UpTime seconds to Serial
 }
void loop() {
  if(Blynk.connected()) {  // If connected run as normal
    Blynk.run();
  }
  Servo2.write(pos_int);
  digitalWrite(DIR, HIGH);  //HIGH = CW
  for(x = 0; x <= 900; x++)
   {
    digitalWrite(STEP,HIGH);
    delay(15);
    digitalWrite(STEP,LOW);
    delay(15);
    Serial.print("phi =");
    Serial.print(x);
    Serial.println();
    ReadData();
    if(ch1 != "")
    {   Serial.println("FIRE");
        SendNotifcation();
        Serial.println();   
    }
    if(ch1 == "-1")
    {
    digitalWrite(DIR, LOW);
    }
    if(ch1 == 1)
    {
    digitalWrite(DIR, HIGH);  
    }
    if(ch1 == "0")
    {  
      for(pos = pos_int; pos >= 0; ){
        Servo2.write(pos);
        ReadData();
        Serial.print(pos);
        Serial.println();

        if(ch2 == "1"){
          pos-=2; //here
          ch2 = "";
        }
        if(ch2 == "-1"){
          pos+=2; //here
          ch2 = "";
        }
        if(ch2 == "0")
        { posw = pos;
          Water(posw);      
          ch2 = "";
          break;
        }
        if(Serial.available() <= 0){
           count ++;   
        }
        if(count >= 50){
          count = 0;
          break;
        }
        delay(50);
      }
      for(pos ; pos <= pos_int; pos+=1){Servo2.write(pos);delay(25);}  
      ch1 = ""; 
    }     
  }  
   
  digitalWrite(DIR, LOW); 
  for(x = 0; x <= 900; x+=1)
  {
    digitalWrite(STEP,HIGH);
    delay(15);
    digitalWrite(STEP,LOW);
    delay(15);
    Serial.print("phi =");
    Serial.print(x);
    Serial.println();
    ReadData();
    if(ch1 != "")
    {   Serial.println("FIRE");
        SendNotifcation();
//        Serial.println();
    } 
    if(ch1 == "-1")
    {
    digitalWrite(DIR, LOW);
    }
    if(ch1 == "1")
    { 
    digitalWrite(DIR, HIGH);   
    }
    if(ch1 == "0"){  
      for(pos = pos_int; pos >= 0; ){
        Servo2.write(pos);
        Serial.print(pos);
        Serial.println();
        ReadData();        
          if(ch2 == "1"){
            pos-=2; //here
            ch2 = "";
          }
          if(ch2 == "-1"){
            pos+=2; //here
            ch2 = "";
          }
          if(ch2 == "0")
            {  
            posw = pos;
            Water(posw);
            ch2 = "";
            break;
            } 
        if(Serial.available() <= 0){
           count ++;   
        }
        if(count >= 50){
          count = 0;
          break;
        }
      delay(50);
    } 
    for(pos ; pos <= pos_int; pos+=1){Servo2.write(pos);delay(25);}
    ch1 = "";        
    }    
  }  
}


String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;
    
    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
void ReadData(){
   if(Serial.available() > 0)
    { BUFFER = Serial.readString();
      ch1 = getValue(BUFFER, ':', 0);
      ch2 = getValue(BUFFER, ':', 1);
    }
}
void Water(int pos){
  digitalWrite(D6, HIGH);
  for(int i = 0; i <= 3; i ++){
    digitalWrite(DIR, LOW);  //HIGH = CW
    for(x = 0; x <= 5; x++) 
    {
      digitalWrite(STEP,HIGH);
      delay(2);
      digitalWrite(STEP,LOW);
      delay(2) ;
      
      
      {  for(int pos2 = pos + 7; pos2 >= pos - 7; pos2 -= 1)
        {
          Servo2.write(pos2);
//        Serial.println(pos2); 
          delay(30);
        }
        for(int pos2 = pos - 7; pos2 <= pos + 7; pos2 += 1)
        {
          Servo2.write(pos2);
//        Serial.println(pos2); 
          delay(30);
        }
      }
  }
  digitalWrite(DIR, LOW);  //HIGH = CW
  for(x = 0; x <= 5; x++) 
  {
    digitalWrite(STEP,HIGH);
    delay(2);
    digitalWrite(STEP,LOW);
    delay(2);
    {
      for(int pos2 = pos + 5; pos2 >= pos - 5; pos2 -= 1)
      {
          Servo2.write(pos2);
//        Serial.println(pos2); 
          delay(30);
      }
      for(int pos2 = pos - 5; pos2 <= pos + 5; pos2 += 1)
      {
          Servo2.write(pos2);
//        Serial.println(pos2); 
          delay(30);
      }
    }
   }
  }
  digitalWrite(D6, LOW);
}
