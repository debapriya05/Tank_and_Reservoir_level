
#include <WiFi.h>
#include<PubSubClient.h>
#include<ArduinoJson.h>
#include <EEPROM.h>

#define r 4
#define r2 14
#define t1 12
#define e1 13
int swi=0;
int stt=0;
int chst=0;
int upl=40;
int dwl=300;
int hl=200;
int hll=360;
int dw=0;
float hgt=300;
int waiting=0;
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "broker.emqx.io";

WiFiClient n;
PubSubClient client(n);
void changest();

void setup_wifi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void callback(char* topic, byte* payload, unsigned int length) {
  StaticJsonDocument<200> d;
  deserializeJson(d,payload);
  hgt =(float)d["height"];
  Serial.print("got the height:");
  Serial.println(hgt);
  waiting=0;
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
  
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("Connected");
      client.subscribe("topicName/height");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  EEPROM.begin(3);
  Serial.begin(115200);
  pinMode(r, OUTPUT);
  pinMode(r2, OUTPUT);
  pinMode(t1, OUTPUT);
  pinMode(e1,INPUT);
  swi=EEPROM.read(0);
  stt=EEPROM.read(1);
  chst=EEPROM.read(2);
  if(swi==255)
  {swi=chst=stt=0;
    EEPROM.write(0,swi);
    EEPROM.write(1,stt);
    EEPROM.write(2,chst);
EEPROM.commit();
  }
    setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);


  Serial.println("Hello, ESP32!");
}

void loop() {
  if(WiFi.status()!=3){
    Serial.println("setting up wifi");
    setup_wifi();}
    else if(WiFi.RSSI()>-80){
    if (!client.connected()) {
    reconnect();
  }
    }

  waiting++;
  //if(waiting==32676)waiting=1;

  digitalWrite(t1,LOW);
  delayMicroseconds(2);
  digitalWrite(t1,HIGH);
  delayMicroseconds(10); 
  digitalWrite(t1,LOW);
  int a1=pulseIn(e1,HIGH);
  float b1=(float)a1;
  b1=b1*0.034/2;
  if(hgt<=upl){swi=0;changest();EEPROM.write(0,swi);EEPROM.commit();}
  else if(hgt>=dwl){swi=1;EEPROM.write(0,swi);EEPROM.commit();}
  if((swi==3&&waiting<100)
  ||(swi==2&&(waiting<100&&b1<=hl))
  ){swi=1;EEPROM.write(0,swi);EEPROM.commit();}

if(waiting>100&&b1<hll)
{swi=3;EEPROM.write(0,swi);EEPROM.commit();}
else if(b1>=hll){swi =2;EEPROM.write(0,swi);EEPROM.commit(); Serial.println("reservoir is empty");}
     int dr=digitalRead(r);
    int dr2=digitalRead(r2);
 if(swi==1){
    if(chst>200 && hgt>=100){
      changest();}
    else{chst++;
    if(chst%40==0){
      EEPROM.write(2,chst);
      EEPROM.commit();

    }
    
    }
    Serial.print(stt);Serial.print(":");
    Serial.print(chst);Serial.print(":");

    Serial.println();
    if(!stt){
      if(!dr){
    digitalWrite(r,HIGH);Serial.println("turning relay high");}
    else{Serial.println("relay is already  high");}
    if(dr2){
    digitalWrite(r2,LOW);}
    }
    else{
      if(!dr2){
    digitalWrite(r2,HIGH);Serial.println("turning relay2 high");}
    else{Serial.println("relay is already  high");}
    if(dr){
    digitalWrite(r,LOW);}}}
    else{
      if(dr)
         digitalWrite(r,LOW);
     if( dr2)
         digitalWrite(r2,LOW); 
    
    }
    Serial.print("tank level from top:");
  Serial.println(hgt);
    Serial.print("reservoir level from top:");
  Serial.println(b1);  

  // put your main code here, to run repeatedly:
   // this speeds up the simulation
if(client.connected())client.loop();
delay(100);
}
void changest(){
  stt=stt%2;
  EEPROM.write(1,stt);
  EEPROM.commit();
  chst=0;
}