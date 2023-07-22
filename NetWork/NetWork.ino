#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_ST7789.h>
#include <Fonts/FreeSerif12pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <SPI.h>

#include <Wire.h>
#include <BH1750.h>
BH1750 lightMeter;

int i = 0, j = 0, y_old, obs[5][2], obs_old[5][2];
int liftTime = 31, score = 0;

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 2,1,0,4,5,6,7,3,POSITIVE); //3F or 27

#define TFT_CS 10
#define TFT_DC 2
#define TFT_RST 6

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC,TFT_RST);

// WPA/WPA2 SSID and password
char ssid[] = "KuaiKuai";      // your network SSID (name)
char pass[] = "8fee5815ae3c";  // your network password
int status  = WL_IDLE_STATUS;   // the Wifi radio's status

char mqttServer[]     = "iiot.ideaschain.com.tw";  // new ideaschain dashboard MQTT server
int mqttPort          = 1883;
char clientId[]       = "DEVICE_12345";            // MQTT client ID. it's better to use unique id.
char username[]       = "lzljjXvLQ0pvVa5j3Eyp";    // device access token(存取權杖)
char password[]       = "";                        // no need password
char subscribeTopic[] = "v1/devices/me/attributes"; // Fixed topic. ***DO NOT MODIFY***
char publishTopic[]   = "v1/devices/me/telemetry";  // Fixed topic. ***DO NOT MODIFY***
char publishPayload[] = "{\"PM2.5\":\"10\",\"PM10\":\"20\"}";        // String of stringified JSON Object (key value pairs)

StaticJsonDocument<100> json_doc;
char payload[100];
const char* payload_light;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

WiFiClient wifiClient;
PubSubClient client(wifiClient);

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");

    // Attempt to connect
    if (client.connect(clientId, username, password)) {
      Serial.println("MQTT connected");
      // Once connected, publish an announcement...
      client.publish(publishTopic, payload);
      // resubscribe topics
      client.subscribe(subscribeTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(9600);
  
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  printWifiData();

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  Serial.begin(9600);


  tft.initR(INITR_BLACKTAB); //轉向

  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK); //填背景
  tft.setCursor(10,30);
  tft.setTextColor(ST77XX_CYAN); //文字顏色
  tft.setFont(&FreeSerif12pt7b);
  tft.print("Hello Everybody!");
  
  tft.setCursor(0,60);
  tft.setTextColor(ST77XX_RED); //文字顏色
  tft.setFont(&FreeSansBold9pt7b);
  tft.print("Let Start this fun game!");

  Wire.begin();

  lightMeter.begin();

  Serial.println("BH1750 TEST");
  
  showIcons();
  tft.fillScreen(ST77XX_BLACK); //填背景

  obs[0][0]=30;
  obs[1][0]=45;
  obs[2][0]=75;
  obs[3][0]=105;

  obs[0][1]=0;
  obs[1][1]=-40;
  obs[2][1]=-80;
  obs[3][1]=-120;
  
  lcd.begin(16, 2); //init
  lcd.backlight();

  pinMode(12, OUTPUT);
  

  Wire.begin();

  lightMeter.begin();
  
}


void showIcons(){

  tft.fillRoundRect(30,95,30,30,5,ST77XX_MAGENTA);
  tft.fillCircle(80,110,15, ST77XX_BLUE);
  tft.drawRect(105,95,39,39,ST77XX_GREEN);
  delay(500);
}


void showInfo(){  
  lcd.setCursor(0, 0); //設游標 字行
  lcd.print("blod:");
  for(int blod = (liftTime+2)/3; blod>0; blod--){
    lcd.print("*");
  }
  for(int space = 11-((liftTime+2)/3); space>0; space--){
    lcd.print(" ");
  }
  lcd.setCursor(0, 1);
  lcd.print("score:");
  lcd.print(score);  
}

void printWifiData() {
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  showInfo();
  tft.setCursor(0,60);
  tft.setTextColor(ST77XX_BLACK);
  tft.print("X");
  if(liftTime > 0){
    float lux = lightMeter.readLightLevel();
    Serial.print("Score:");
    Serial.println(score);
    Serial.println(liftTime);
    Serial.print("Light:");
    Serial.print(lux);
    Serial.println(" lx");
    int y = lux > 120 ? 120:(int)lux;
    Serial.println(y);
    tft.fillCircle(140,y_old,15, ST77XX_BLACK);
    tft.fillCircle(140,y,15, ST77XX_BLUE);
    y_old = y;
    
    for(int i=0; i<4; i++){
      tft.fillRect(obs_old[i][1],obs_old[i][0],10,20,ST77XX_BLACK);
      tft.fillRect(obs[i][1],obs[i][0],10,20,ST77XX_RED);
      if(obs[i][1]>120 && obs[i][1]<140){
        if(abs(y-obs[i][0]-10)<20){
          digitalWrite(12, HIGH);
          tft.setCursor(0,60);
          tft.setTextColor(ST77XX_YELLOW);
          tft.print("X");
          liftTime--;
          //digitalWrite(12, LOW);
        }
      }
      obs_old[i][1]=obs[i][1];
      obs_old[i][0]=obs[i][0];
      obs[i][1]+=3;
      if(obs[i][1]>160){
        obs[i][1]=0;
        
        obs[i][0] = (random(99) + random(99))%100;
      }
    }
  }else{
    tft.initR(INITR_BLACKTAB); //轉向
    tft.fillScreen(ST77XX_BLACK); //填背景
    tft.setCursor(20,85);
    tft.setTextColor(ST77XX_RED); //文字顏色
    tft.setFont(&FreeSansBold9pt7b);
    tft.print("You Lose!");
    showInfo();
    json_doc["Score"] = score;
    json_doc["liftTime"] = liftTime;
    
    serializeJson(json_doc, payload);
    Serial.print(" string to json = ");
    Serial.println(payload);
    client.publish(publishTopic, payload);
    delay(900000);
  }
  score+=1;
  delay(50);
  

}
