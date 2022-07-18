
#include <ESP8266WebServer.h>  // web server (optional) 
#include <ESP8266mDNS.h>       // web dns
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <NTPClient.h> //เวลา
#include <WiFiUdp.h> //เวลา
#include <DHT.h> //เซนเซอร์
#include <WiFiClientSecureAxTLS.h> //เรียก lib ครบ แต่ IDE blacked out

ESP8266WiFiMulti WiFiMulti;
ESP8266WebServer server(80);   //Object ตัว web port 80  
WiFiUDP ntpUDP;
const long utcOffsetInSeconds = 25200;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
unsigned long epochTime = timeClient.getEpochTime();
struct tm *ptm = gmtime ((time_t *)&epochTime);

#define DHTPin 5 
#define DHTTYPE DHT11   
DHT dht(DHTPin, DHTTYPE);

String formattedTime;
String Date;
int Day;
int Month;
int Year;


String SendHTML(float TemperatureWeb,float HumidityWeb, String TimeWeb, String DateWeb);
void handle_OnConnect();
void handle_NotFound();

//#define WIFI_SSID "WIFI" //ไวไฟอีกตัว optional
#define WIFI_SSID "Your SSID"
//#define WIFI_PASSWORD "pass" //ไวไฟอีกตัว optional
#define WIFI_PASSWORD "Your Pass"
#define LINE_TOKEN_PIR "Your token"  //Line token ที่ generate มา
#define PirPin D6
bool beep_state = false;
bool send_state = false;

void setup() {
  Serial.begin(115200); //(baud rate 96000, 115200)
  delay(10);
  dht.begin();
  Serial.print("\n\nElectricl Engineering Enterprise Group\n");
  WiFiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  WiFiMulti.addAP("AndroidAP", "ifmd0883");
  Serial.println("Connecting Wifi...");
  delay(2000);
  CheckWiFi();
  
  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);
  server.begin();
  timeClient.begin();
}

void loop() {
  CheckWiFi();
  float Temperature = dht.readTemperature();
  float Humidity = dht.readHumidity();
  if ((Temperature > 25) and (Humidity > 50)); {
    Serial.println(String(Temperature) +"    "+String(Humidity));
    String message = "อุณหภูมิปัจจุบัน " + String(Temperature) +"°C สูงกว่าค่ากำหนด " + "ความชื้นปัจจุบัน " + String(Humidity) + " เปอร์เซ็น" + " สูงกว่าค่ากำหนด" ;
    LINE_Notify(message);
    delay(10000);
  }
  if ((Temperature > 25) or (Humidity > 50)); {
    if (Temperature > 25) {
      Serial.println(String(Temperature) +"    "+String(Humidity));
      String message = "อุณหภูมิปัจจุบัน " + String(Temperature) +"°C สูงกว่าค่ากำหนด " + "ความชื้นปัจจุบัน " + String(Humidity) + " เปอร์เซ็น" + " ค่าปกติ" ;
      LINE_Notify(message);
      delay(10000);
      }else {
        Serial.println(String(Temperature) +"    "+String(Humidity));
        String message = "อุณหภูมิปัจจุบัน " + String(Temperature) +"°C ค่าปกติ " + "ความชื้นปัจจุบัน " + String(Humidity) + " เปอร์เซ็น" + " สูงกว่าค่ากำหนด" ;
        LINE_Notify(message);
        delay(10000);     
}
    }
server.handleClient();
}

void CheckWiFi() {      //Check wifi
  if (WiFiMulti.run() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    delay(1000);
  }
}

void LINE_Notify(String Message) {
  axTLS::WiFiClientSecure client;
  if (!client.connect("notify-api.line.me", 443)) {
    Serial.println("connection failed");
    return;
  } else {}

  String req = "";
  req += "POST /api/notify HTTP/1.1\r\n";
  req += "Host: notify-api.line.me\r\n";
  req += "Authorization: Bearer " + String(LINE_TOKEN_PIR) + "\r\n";
  req += "Cache-Control: no-cache\r\n";
  req += "User-Agent: ESP8266\r\n";
  req += "Content-Type: application/x-www-form-urlencoded\r\n";
  req += "Content-Length: " + String(String("message=" + Message).length()) + "\r\n";
  req += "\r\n";
  req += "message=" + Message;

  client.print(req);
  delay(20);

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }
  Serial.println("Send Successfully");
}

void handle_OnConnect() { //หากเชื่อมเว็บสำเร็จ

  timeClient.update();
 
  epochTime = timeClient.getEpochTime(); 
  String Time = timeClient.getFormattedTime();
  
  tm *ptm = gmtime ((time_t *)&epochTime); 

  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon+1;
  int currentYear = ptm->tm_year+1900;
 
  Time = timeClient.getFormattedTime(); 
  Date = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
  float Temperature = dht.readTemperature();
  float Humidity = dht.readHumidity();
  server.send(200, "text/html", SendHTML(Temperature,Humidity,Time,Date)); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

//HTML //Originated by Ruzell Ramirez https://www.circuitbasics.com/how-to-set-up-a-web-server-using-arduino-and-esp8266-01/
String SendHTML(float TemperatureWeb,float HumidityWeb, String TimeWeb,String DateWeb){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>ESP8266 Global Server</title>\n";

  ptr +="</head>\n";   //ใส่ CSS ยังไงวะไอสาส
  ptr +="<body>\n";
  ptr +="<div id=\"webpage\">\n";
  ptr +="<h1><center>ESP8266 Temperature and humidity sensor<center></h1>\n";
  ptr +="<h2><center>By Apiratchai Lakkum<center></h2>\n";
  //ptr +="<h6><center>Credits: Ruzell Ramirez <center></h6>\n";

  ptr +="<p><center>Date: ";
  ptr +=(String)DateWeb;
  ptr +="<center></p>";
  ptr +="<p><center>Time: ";
  ptr +=(String)TimeWeb;
  ptr +="<center></p>";
  ptr +="<p><center>Temperature: ";
  ptr +=String((int)TemperatureWeb);
  ptr +=" C<center></p>";
  ptr +="<p><center>Humidity: ";
  ptr +=String((int)HumidityWeb);
  ptr +=" %<center></p>";
  
  ptr +="</div>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
