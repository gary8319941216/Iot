
typedef struct Serv
{
  int angle;
  int UVvalue;
}serv;

serv ser[18]; 

#include <Servo.h> 
#include <stdlib.h> 
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <HttpClient.h>
//#include <LTask.h>
#include <LWiFi.h>
#include <LWiFiClient.h>
#include <LDateTime.h>
#include "LDHT.h"
#include <LGPS.h>
#include <math.h>
#include <Tone.h>

#define WIFI_AP "HTC Portable Hotspot DFA0"
#define WIFI_PASSWORD "08E830B27AABD"
#define WIFI_AUTH LWIFI_WPA  // choose from LWIFI_OPEN, LWIFI_WPA, or LWIFI_WEP.
#define per 50
#define per1 3
#define DEVICEID "DBxOZMOG" // Input your deviceId
#define DEVICEKEY "01lkd3Jx4VF0drFz"// Input your deviceKey
#define SITE_URL "api.mediatek.com"

LWiFiClient c;
//gpsSentenceInfoStruct info;
char buff[256];
const int pinLight = A1;
unsigned int rtc;
unsigned int lrtc;
unsigned int rtc1;
unsigned int lrtc1;
char port[4]={0};
char connection_info[21]={0};
char ip[21]={0};             
int portnum;
int val = 0;
double latitude;
double longitude;
double companylatitude = 24.803129;
double companylongitude = 120.973006;
double distance;
String tcpdata = String(DEVICEID) + "," + String(DEVICEKEY) + ",0";
String upload_distance;
String upload_led;
String upload_buzz;
String upload_gps;
String upload_current;
int mode = 0;
float AcsValue=0.0,Samples=0.0,AvgAcs=0.0, AcsValueF=0.0;
boolean isbuzz = false;
int servocount = 5;

LWiFiClient c2;
HttpClient http(c2);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo myservo,MYS;

static unsigned char getComma(unsigned char num,const char *str)
{
  unsigned char i,j = 0;
  int len=strlen(str);
  for(i = 0;i < len;i ++)
  {
     if(str[i] == ',')
      j++;
     if(j == num)
      return i + 1; 
  }
  return 0; 
}

static double getDoubleNumber(const char *s)
{
  char buf[10];
  unsigned char i;
  double rev;
  
  i=getComma(1, s);
  i = i - 1;
  strncpy(buf, s, i);
  buf[i] = 0;
  rev=atof(buf);
  return rev; 
}

static double getIntNumber(const char *s)
{
  char buf[10];
  unsigned char i;
  double rev;
  
  i=getComma(1, s);
  i = i - 1;
  strncpy(buf, s, i);
  buf[i] = 0;
  rev=atoi(buf);
  return rev; 
}

void parseGPGGA(const char* GPGGAstr)
{
  /* Refer to http://www.gpsinformation.org/dale/nmea.htm#GGA
   * Sample data: $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
   * Where:
   *  GGA          Global Positioning System Fix Data
   *  123519       Fix taken at 12:35:19 UTC
   *  4807.038,N   Latitude 48 deg 07.038' N
   *  01131.000,E  Longitude 11 deg 31.000' E
   *  1            Fix quality: 0 = invalid
   *                            1 = GPS fix (SPS)
   *                            2 = DGPS fix
   *                            3 = PPS fix
   *                            4 = Real Time Kinematic
   *                            5 = Float RTK
   *                            6 = estimated (dead reckoning) (2.3 feature)
   *                            7 = Manual input mode
   *                            8 = Simulation mode
   *  08           Number of satellites being tracked
   *  0.9          Horizontal dilution of position
   *  545.4,M      Altitude, Meters, above mean sea level
   *  46.9,M       Height of geoid (mean sea level) above WGS84
   *                   ellipsoid
   *  (empty field) time in seconds since last DGPS update
   *  (empty field) DGPS station ID number
   *  *47          the checksum data, always begins with *
   */
  int tmp, hour, minute, second, num ;
  if(GPGGAstr[0] == '$')
  {
    tmp = getComma(1, GPGGAstr);
    hour     = (GPGGAstr[tmp + 0] - '0') * 10 + (GPGGAstr[tmp + 1] - '0');
    minute   = (GPGGAstr[tmp + 2] - '0') * 10 + (GPGGAstr[tmp + 3] - '0');
    second    = (GPGGAstr[tmp + 4] - '0') * 10 + (GPGGAstr[tmp + 5] - '0');
    
    sprintf(buff, "UTC timer %2d-%2d-%2d", hour, minute, second);
    //Serial.println(buff);
    
    tmp = getComma(2, GPGGAstr);
    latitude = getDoubleNumber(&GPGGAstr[tmp]);
    tmp = getComma(4, GPGGAstr);
    longitude = getDoubleNumber(&GPGGAstr[tmp]);
    sprintf(buff, "latitude = %10.4f, longitude = %10.4f", latitude, longitude);
    //Serial.println(buff); 
    
    tmp = getComma(7, GPGGAstr);
    num = getIntNumber(&GPGGAstr[tmp]);    
    sprintf(buff, "satellites number = %d", num);
    //Serial.println(buff); 
  }
  else
  {
    Serial.println("Not get data"); 
  }
}

void setup()
{
  
  LTask.begin();
  LWiFi.begin();
  //LGPS.powerOn();
  //lcd.begin();
  //lcd.backlight();
  myservo.attach(3); //太陽能板
  myservo.write(0);
  MYS.attach(9); //LED
  MYS.write(0);
  Serial.begin(115200);
  while(!Serial) delay(1000); /* comment out this line when Serial is not present, ie. run this demo without connect to PC */

  Serial.println("Connecting to AP");
  while (0 == LWiFi.connect(WIFI_AP, LWiFiLoginInfo(WIFI_AUTH, WIFI_PASSWORD)))
  {
    delay(1000);
  }
  
  Serial.println("calling connection");

  while (!c2.connect(SITE_URL, 80))
  {
    Serial.println("Re-Connecting to WebSite");
    delay(1000);
  }
  delay(100);
  
  //pinMode(13, OUTPUT);
  pinMode(5,OUTPUT); //buzzer
  pinMode(6,OUTPUT); //LED
  getconnectInfo();
  connectTCP();
}

void getconnectInfo(){
  //calling RESTful API to get TCP socket connection
  c2.print("GET /mcs/v2/devices/");
  c2.print(DEVICEID);
  c2.println("/connections.csv HTTP/1.1");
  c2.print("Host: ");
  c2.println(SITE_URL);
  c2.print("deviceKey: ");
  c2.println(DEVICEKEY);
  c2.println("Connection: close");
  c2.println();
  
  delay(500);

  int errorcount = 0;
  while (!c2.available())
  {
    Serial.println("waiting HTTP response: ");
    Serial.println(errorcount);
    errorcount += 1;
    if (errorcount > 10) {
      c2.stop();
      return;
    }
    delay(100);
  }
  int err = http.skipResponseHeaders();

  int bodyLen = http.contentLength();
  Serial.print("Content length is: ");
  Serial.println(bodyLen);
  Serial.println();
  char c;
  int ipcount = 0;
  int count = 0;
  int separater = 0;
  while (c2)
  {
    int v = c2.read();
    if (v != -1)
    {
      c = v;
      Serial.print(c);
      connection_info[ipcount]=c;
      if(c==',')
      separater=ipcount;
      ipcount++;    
    }
    else
    {
      Serial.println("no more content, disconnect");
      c2.stop();

    }
    
  }
  Serial.print("The connection info: ");
  Serial.println(connection_info);
  int i;
  for(i=0;i<separater;i++)
  {  ip[i]=connection_info[i];
  }
  int j=0;
  separater++;
  for(i=separater;i<21 && j<5;i++)
  {  port[j]=connection_info[i];
     j++;
  }
  Serial.println("The TCP Socket connection instructions:");
  Serial.print("IP: ");
  Serial.println(ip);
  Serial.print("Port: ");
  Serial.println(port);
  portnum = atoi (port);
  Serial.println(portnum);

} //getconnectInfo

void uploadstatus(){
  //calling RESTful API to upload datapoint to MCS to report LED status
  Serial.println("calling connection");
  LWiFiClient c2;  

  while (!c2.connect(SITE_URL, 80))
  {
    Serial.println("Re-Connecting to WebSite");
    delay(1000);
  }
  delay(100);

  if(mode==0){

  //latitude = 24.792368;
  //longitude = 120.995477;
  double radLat1 = latitude * PI / 180;
  double radLat2 = companylatitude * PI / 180;
  double a = radLat1 - radLat2;
  double b = longitude * PI / 180 - companylongitude * PI / 180;
  //distance = 2 * 6378.137 * asin(sqrt(sin(a/2) * sin(a/2) + acos(radLat1) * acos(radLat2) * sin(b/2) * sin(b/2)));
  distance = 6378.137 * acos(sin(radLat1) * sin(radLat2) + cos(radLat1) * cos(radLat2) * cos(companylongitude * PI / 180 - longitude * PI / 180));
  Serial.println(distance);
  upload_distance = "distance,,";
  upload_distance = upload_distance + (String)distance;
  int thislength = upload_distance.length();
  HttpClient http(c2);
  c2.print("POST /mcs/v2/devices/");
  c2.print(DEVICEID);
  c2.println("/datapoints.csv HTTP/1.1");
  c2.print("Host: ");
  c2.println(SITE_URL);
  c2.print("deviceKey: ");
  c2.println(DEVICEKEY);
  c2.print("Content-Length: ");
  c2.println(thislength);
  c2.println("Content-Type: text/csv");
  c2.println("Connection: close");
  c2.println();
  c2.println(upload_distance);
  mode = 1;
  }

  else if(mode==1){
    if(digitalRead(6)==1)
    upload_led = "led,,1";
    else
    upload_led = "led,,0";
    int thislength = upload_led.length();
    HttpClient http(c2);
    c2.print("POST /mcs/v2/devices/");
    c2.print(DEVICEID);
    c2.println("/datapoints.csv HTTP/1.1");
    c2.print("Host: ");
    c2.println(SITE_URL);
    c2.print("deviceKey: ");
    c2.println(DEVICEKEY);
    c2.print("Content-Length: ");
    c2.println(thislength);
    c2.println("Content-Type: text/csv");
    c2.println("Connection: close");
    c2.println();
    c2.println(upload_led);
    mode = 2;
  }
  
  else if(mode==2){

  upload_gps = "GPS,,";
  latitude = 24.792368;
  longitude = 120.995477;
  Serial.println(latitude);
  Serial.println(longitude);
  upload_gps = upload_gps + (String)latitude;
  upload_gps = upload_gps + "," + (String)longitude;
  upload_gps = upload_gps + "," + "0";
  
  int thislength = upload_gps.length();
  HttpClient http(c2);
  c2.print("POST /mcs/v2/devices/");
  c2.print(DEVICEID);
  c2.println("/datapoints.csv HTTP/1.1");
  c2.print("Host: ");
  c2.println(SITE_URL);
  c2.print("deviceKey: ");
  c2.println(DEVICEKEY);
  c2.print("Content-Length: ");
  c2.println(thislength);
  c2.println("Content-Type: text/csv");
  c2.println("Connection: close");
  c2.println();
  c2.println(upload_gps);
  mode = 3;
  }

  else if(mode==3){

  upload_current = "current,,";
  upload_current = upload_current + (String)AcsValueF;
  Serial.println(AcsValueF);
  
  int thislength = upload_current.length();
  HttpClient http(c2);
  c2.print("POST /mcs/v2/devices/");
  c2.print(DEVICEID);
  c2.println("/datapoints.csv HTTP/1.1");
  c2.print("Host: ");
  c2.println(SITE_URL);
  c2.print("deviceKey: ");
  c2.println(DEVICEKEY);
  c2.print("Content-Length: ");
  c2.println(thislength);
  c2.println("Content-Type: text/csv");
  c2.println("Connection: close");
  c2.println();
  c2.println(upload_current);
  mode = 4;
  }

  else if(mode==4){
    if(isbuzz==true)
    upload_buzz = "buzzer,,1";
    else
    upload_buzz = "buzzer,,0";
    int thislength = upload_buzz.length();
    HttpClient http(c2);
    c2.print("POST /mcs/v2/devices/");
    c2.print(DEVICEID);
    c2.println("/datapoints.csv HTTP/1.1");
    c2.print("Host: ");
    c2.println(SITE_URL);
    c2.print("deviceKey: ");
    c2.println(DEVICEKEY);
    c2.print("Content-Length: ");
    c2.println(thislength);
    c2.println("Content-Type: text/csv");
    c2.println("Connection: close");
    c2.println();
    c2.println(upload_buzz);
    mode = 0;
  }

  delay(500);

  int errorcount = 0;
  while (!c2.available())
  {
    Serial.print("waiting HTTP response: ");
    Serial.println(errorcount);
    errorcount += 1;
    if (errorcount > 10) {
      c2.stop();
      return;
    }
    delay(100);
  }
  int err = http.skipResponseHeaders();

  int bodyLen = http.contentLength();
  Serial.print("Content length is: ");
  Serial.println(bodyLen);
  Serial.println();
  while (c2)
  {
    int v = c2.read();
    if (v != -1)
    {
      Serial.print(char(v));
    }
    else
    {
      Serial.println("no more content, disconnect");
      c2.stop();

    }
    
  }
}



void connectTCP(){
  //establish TCP connection with TCP Server with designate IP and Port
  c.stop();
  Serial.println("Connecting to TCP");
  Serial.println(ip);
  Serial.println(portnum);
  while (0 == c.connect(ip, portnum))
  {
    Serial.println("Re-Connecting to TCP");    
    delay(1000);
  }  
  Serial.println("send TCP connect");
  c.println(tcpdata);
  c.println();
  Serial.println("waiting TCP response:");
} //connectTCP

void heartBeat(){
  Serial.println("send TCP heartBeat");
  c.println(tcpdata);
  c.println();
    
} //heartBeat

boolean disconnectedMsg = false;

void loop()
{
  //Check for TCP socket command from MCS Server 
  String tcpcmd="";
  //LGPS.getData(&info);
  //parseGPGGA((const char*)info.GPGGA);
  unsigned int x=0;
  AcsValue=0.0;
  Samples=0.0;
  AvgAcs=0.0;
  AcsValueF=0.0; 
    for( int xi = 0 ; xi<=170 ; xi=xi+10){
        ser[xi/10].angle = xi;
    }
    for( int xi = 0 ; xi<=170 ; xi=xi+10){
        ser[xi/10].UVvalue = analogRead(pinLight);
        //Serial.println(ser[xi/5].UVvalue);
        myservo.write(xi);
        delay(100);
    }
    qsort(ser,18,sizeof(ser[0]),compare);
    myservo.write(ser[0].angle);
  /*Serial.println(ser[0].angle);
  Serial.println(ser[0].UVvalue);*/
  for(int x=0;x<150;x++){
    AcsValue=analogRead(A0);
    Samples=Samples+AcsValue;
    delay(3);
  }
  AvgAcs=Samples/150.0;
  AcsValueF=((AvgAcs*(5.0/1024.0))-2.5)/0.185;
  AcsValueF = AcsValueF * 1000;
  Serial.println("current");
  Serial.println(AcsValueF);
  delay(50);
  if (AcsValueF< 20){
      isbuzz = true;
      digitalWrite(5,HIGH);
      digitalWrite(6,HIGH);
      for(int pos = 0; pos <= 180; pos += 10) // goes from 0 degrees to 180 degrees 
      {                                  // in steps of 1 degree 
      MYS.write(pos);              // tell servo to go to position in variable 'pos' 
      delay(50);    
      //digitalWrite(6, HIGH);   // turn the LED on (HIGH is the voltage level)
      }
  } 
  else{
      isbuzz = false;
      digitalWrite(5,LOW);
      digitalWrite(6,LOW);
  }

  LDateTime.getRtc(&rtc);
  if ((rtc - lrtc) >= per) {
    heartBeat();
    lrtc = rtc;
  }
  //Check for report datapoint status interval
  LDateTime.getRtc(&rtc1);
  if ((rtc1 - lrtc1) >= per1) {
    uploadstatus();
    lrtc1 = rtc1;
  }
  
}

int compare (const void  * a, const void * b)
{
  
  if ((*(struct Serv *) a).UVvalue > (*(struct Serv *) b).UVvalue)
      return -1;

   else if ((*(struct Serv *) a).UVvalue < (*(struct Serv *) b).UVvalue)
      return 1;

   else
      return 0;
}
