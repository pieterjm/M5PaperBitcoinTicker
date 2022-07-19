#include "M5EPD.h"
#include "binaryttf.h"
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <HTTPClient.h>
#include <Arduino_JSON.h>

#define TEXT_COLOR 15
#define BG_COLOR 0

M5EPD_Canvas canvas(&M5.EPD);
WiFiManager wm;
   
void setup()
{
  // disable WifiManager debugging
  wm.setDebugOutput(false);
  
    M5.begin();
    M5.EPD.Clear(true);

    Serial.setDebugOutput(false);  

    // initialize canvas
    canvas.loadFont(binaryttf, sizeof(binaryttf)); // Load font files from binary data
    canvas.createCanvas(960,540);
    canvas.createRender(240);
    canvas.createRender(64);
    canvas.createRender(48);
    canvas.fillCanvas(BG_COLOR);
    canvas.setTextColor(TEXT_COLOR);

    canvas.setTextDatum(CC_DATUM);
    canvas.setTextSize(48);
    canvas.drawString("Bitcoin Ticker",480,420);
    canvas.pushCanvas(0, 0, UPDATE_MODE_DU);


    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    bool res;

    res = wm.autoConnect("BitcoinTicker"); // anonymous ap         
    if(!res) {
        ESP.restart();
    }     
}

void sleep_for_a_minute()
{   
    rtc_time_t current_time;

    // Calculate remaining minute and shutdown for that.
    M5.RTC.getTime(&current_time);
    int sleep_period = 60 - current_time.sec; 
    M5.shutdown(sleep_period);

    // In case we're plugged in and loop is still running
    delay(sleep_period * 1000);
}

void loop()
{
  HTTPClient http;
  http.begin("https://api.coingecko.com/api/v3/simple/price?ids=bitcoin&vs_currencies=usd"); 
  int httpCode = http.GET();
  if(httpCode > 0) {
    
    // file found at server
    if(httpCode == HTTP_CODE_OK) {
      String payload = http.getString();                
      JSONVar myObject = JSON.parse(payload);
      String price = String((int) myObject["bitcoin"]["usd"]);
     
      canvas.fillCanvas(BG_COLOR);
      canvas.setTextSize(240);
      canvas.setTextColor(TEXT_COLOR);
      canvas.setTextDatum(CL_DATUM);
      canvas.drawString("$", 150, 270);
      canvas.drawString(price, 270, 270);
      canvas.setTextSize(64);
      canvas.drawString("BTC", 20, 220);
      canvas.drawLine(30,270,130,270,10,TEXT_COLOR);
      canvas.drawString("USD", 20, 320);
      canvas.setTextDatum(CC_DATUM);
      canvas.setTextSize(48);
      canvas.drawString("Market price of bitcoin",480,420);

      canvas.pushCanvas(0, 0, UPDATE_MODE_DU);   // does not blink, but has trace
    }
  } 
  http.end();

  sleep_for_a_minute(); 
}
