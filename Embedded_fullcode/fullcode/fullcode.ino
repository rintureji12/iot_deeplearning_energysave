#include <WiFi.h>
#include <PubSubClient.h>
#include "EmonLib.h" 
//#include "ACS712.h"

float KWH=0;
float totalenergyused=0;
float val=0;
int count=0;
int num=0;
int ledbulb1=0;
int ledbulb2=0;
int ledbulb3=0;
int ledbulb4=0;

int led1 = 12;
int led2 = 13;
int led3 = 26;
int led4 = 27;
const char* ssid = "FLEMING 4G";
const char* password = "Fleming#2023";
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);   //
EnergyMonitor energy; 
unsigned long lastMsg = 0;

//  Arduino UNO has 5.0 volt with a max ADC value of 1023 steps
//  ACS712 5A  uses 185 mV per A
//  ACS712 20A uses 100 mV per A
//  ACS712 30A uses  66 mV per A


//ACS712  ACS(A0, 3.3, 4095, 66);
//  ESP 32 example (might requires resistors to step down the logic voltage)
//  ACS712  ACS(34, 3.3, 4095, 185);

char msg[50];
char sbuf[15];

void setup_wifi() {
  delay(10);
  //We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);          //station mode, access point-hotspot
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

void callback(char* topic, byte* payload, unsigned int length) 
{  
  
 Serial.println();
  for (int i = 0; i < length; i++) {    //
    Serial.print((char)payload[i]); 
    sbuf[i]=(char)payload[i];
  }

  if(strncmp(sbuf,"01",2)==0)
  {
    function0001();
    ledbulb1=1;
    ledbulb2=1;
    ledbulb3=0;
    ledbulb4=0;
   
    num=1;
  }
  else if(strncmp(sbuf,"10",2)==0)
  {
    function0010();
    ledbulb1=0;
    ledbulb2=0;
    ledbulb3=1;
    ledbulb4=1;
    num=1;
  }
  else if(strncmp(sbuf,"11",2)==0)
  {
    function0011();
   ledbulb1=1;
    ledbulb2=1;
    ledbulb3=1;
    ledbulb4=1;
    num=1;
  }
 
  else
  {
    function0000();
     ledbulb1=0;
    ledbulb2=0;
    ledbulb3=0;
    ledbulb4=0;
    num=0;
  }
}


void reconnect() {
  
  while (!client.connected()) {           // Loop until we're reconnected
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);    // client id
    
    // Attempt to connect
    if (client.connect(clientId.c_str())) {   //
      Serial.println("connected");
      
      client.publish("Energymeter@123", "hello world");    // Once connected, publish an announcement...
      
      client.subscribe("Energymeter@123recive");    // ... and resubscribe
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());             //
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setup()
{
 
  Serial.begin(9600);
Serial.print("Connecting to ");
   setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
//  
//  ACS.autoMidPoint();
//  Serial.print("MidPoint: ");
//  Serial.print(ACS.getMidPoint());
//  Serial.print(". Noise mV: ");
//  Serial.println(ACS.getNoisemV());
  pinMode(13,OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(26, OUTPUT);
  pinMode(27, OUTPUT);
  
  
  energy.voltage(35, 592, 1.7);  // Voltage: input pin, calibration, phase_shift
  energy.current(34, 0.6);       // Current: input pin, calibration.

}


void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  energy.calcVI(20,2000);         // Calculate all. No.of half wavelengths (crossings), time-out

  float supplyVoltage   = energy.Vrms;             //extract Vrms into Variable
  float Irms            = (energy.Irms,2);             //extract Irms into Variable

 // starttime = micros();
 unsigned long microseconds = micros(); // get the current time in microseconds
  float hours = (float)microseconds / 3600000000.0; // convert microseconds to hours

//  Serial.print("Hours: ");
//  Serial.print(hours);
  



    if (hours>=0.01 && energy.Vrms>100)
{
  totalenergyused = (energy.Vrms*energy.Irms*hours)/1000;
  val+=totalenergyused;
  KWH=val/1000;
  //Serial.println("string");
}

//Serial.print("\t energy");
//Serial.println(energy.Vrms*energy.Irms*hours/1000,6);

int supply = energy.Vrms;
String Current = String(energy.Irms,2);
int Watt = energy.Vrms*energy.Irms;

  
  
//  String Current2 = String((ACS.mA_AC()-ACS.getNoisemV()),2);
  
  
  unsigned long now = millis();
  if (now - lastMsg > 30000) {
    lastMsg = now;
    sprintf(msg,"%d,%s,%d,%f,%d%d%d,%d%d%d",supply,Current,Watt,KWH,num,ledbulb4,ledbulb3,num,ledbulb2,ledbulb1);
   count++;
   if (count==24)
  { 
    count=0;
    val=0;
    
  }
 
 // }
    Serial.print(msg);
    Serial.println();
    client.publish("Energymeter@123", msg);

    
//energy.calcVI(20,2000);         // Calculate all. No.of half wavelengths (crossings), time-out
//energy.serialprint();           // Print out all variables (realpower, apparent power, Vrms, Irms, power factor)
//  
//  float realPower       = energy.realPower;        //extract Real Power into variable
//  float apparentPower   = energy.apparentPower;    //extract Apparent Power into variable
//  float powerFActor     = energy.powerFactor;      //extract Power Factor into Variable
//  float supplyVoltage   = energy.Vrms;             //extract Vrms into Variable
//  float Irms            = energy.Irms;   
  
 delay(1000);
}
}

void function0000() {
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);
  digitalWrite(led4, LOW);
}

void function0001() {
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, HIGH);
  digitalWrite(led4, HIGH);
}

void function0010() {
  digitalWrite(led1, HIGH);
  digitalWrite(led2, HIGH);
  digitalWrite(led3, LOW);
  digitalWrite(led4, LOW);
}


void function0011() {
  digitalWrite(led1, HIGH);
  digitalWrite(led2, HIGH);
  digitalWrite(led3, HIGH);
  digitalWrite(led4, HIGH);
}
