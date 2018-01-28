#include "ESP8266WiFi.h"
#include <EEPROM.h>

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define PIN 14

const char* smoocon_ssid = "shmoocon-wpa";
const char* ssid;
String WifiSSID;
String temp_ssid;

struct RGB {
  byte r;
  byte g;
  byte b;
};

RGB flameColors[] = {
  {226, 121, 35}  // Orange flame
};

int LED_Brightness = 25;

int brightness = 255; //Flames
int NUMBER_OF_COLORS = sizeof(flameColors) / sizeof(RGB);
int currentColorIndex = 0;
int rssi_strength;
int scan_delay = 10000;  //Scan every 3 seconds

unsigned long starttime = 0;
unsigned long starttime1 = 0;

boolean newClient = false;

char rxChar[33];
char conf[33];
char buff_index = 0;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(2, PIN, NEO_GRB + NEO_KHZ800);

void fire() {
  RGB currentColor = flameColors[currentColorIndex];
  int flicker = random(0, 55);
  int r1 = currentColor.r - flicker;
  int g1 = currentColor.g - flicker;
  int b1 = currentColor.b - flicker;
  if (g1 < 0) g1 = 0;
  if (r1 < 0) r1 = 0;
  if (b1 < 0) b1 = 0;
  strip.setPixelColor(1, r1, g1, b1);
  strip.show();
}

int getRSSI(const char* target_ssid) {
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; i++) {
    if (strcmp(WiFi.SSID(i).c_str(), target_ssid) == 0) {
      return WiFi.RSSI(i);
    }
  }
  return 0;
}

void display_rssi(){
  if (rssi_strength >= -67 && rssi_strength!=0) { //Good signal
    strip.setPixelColor(0, 0, LED_Brightness, 0);  //Green
  }
  else if (rssi_strength < -67 && rssi_strength >= -70 && rssi_strength!=0) { //Okay signal
    strip.setPixelColor(0, LED_Brightness, LED_Brightness, 0);  //Yellow
  }
  else if (rssi_strength < -70 && rssi_strength!=0){  //Bad signal 
    strip.setPixelColor(0, LED_Brightness, 0, 0);  //Red
  }
  else if (rssi_strength == 0){  //No SSID 
    strip.setPixelColor(0, 0, 0, 0);  //Off
  }
  strip.show(); //Diplay color
}

uint8_t put_chr_buf(char c) {
  if (c == '\n') {
    return 1;
  }
  rxChar[buff_index] = c;
  buff_index++;
  //rxChar[index] = 0;
  return 0;
}

void check_terminal(){
  uint8_t temp = 0;
  if (newClient == false && Serial.available()>0){
    Serial.flush();
    Serial.println("Set New SSID");
    newClient = true;
  }
  
  if(newClient == true && Serial.available()>0){
    while(Serial.available()){
      temp = put_chr_buf(Serial.read());
      if(temp){
        Serial.flush();
        WifiSSID = String(rxChar);
        int len = WifiSSID.length();
        EEPROM.write(0, len);
        WifiSSID.toCharArray(conf, len+1);
        for (int i = 0; i<len+1; i++){
          EEPROM.write(1+i, conf[i]);
        }
        EEPROM.commit();
        ssid = WifiSSID.c_str();
        Serial.print("New SSID: ");
        Serial.println(ssid);
        Serial.println("");
        buff_index = 0; //Reset buffer index 
        memset(rxChar, 0, sizeof(rxChar));  //Clear Array
        newClient = false; 
        return;
      }
    }
  }
}

void setup() {
  Serial.begin(9600);
  EEPROM.begin(512);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  strip.begin();
  strip.setBrightness(brightness);
  strip.show();

   if (EEPROM.read(0)== 0xFF || EEPROM.read(0) == 0){  //Use default SSID if it has not been configured
    ssid = smoocon_ssid;  
  }

  else{ //Read SSID from EEPROM
    int len = EEPROM.read(0);
    for (int i = 0; i<len; i++){
      char c = EEPROM.read(i+1);
      temp_ssid+=c;
    }
    ssid = temp_ssid.c_str();
  }

  rssi_strength = getRSSI(ssid);
  display_rssi();

  Serial.print("Current SSID: ");
  Serial.println(ssid);
  Serial.print("RSSI: ");
  Serial.println(rssi_strength);
  Serial.println("");
}

void loop() {
  int fire_delay = (random(10, 113));
  if (millis() - starttime >= fire_delay){
    starttime = millis();
    fire();
  }
  if(millis() - starttime1 >= scan_delay){  //Update every 10 seconds 
    starttime1 = millis();
    Serial.print("Current SSID: ");
    Serial.println(ssid);
    rssi_strength = getRSSI(ssid);
    Serial.print("RSSI: ");
    Serial.println(rssi_strength);
    Serial.println("");
    display_rssi();
  }
  check_terminal(); 
}

