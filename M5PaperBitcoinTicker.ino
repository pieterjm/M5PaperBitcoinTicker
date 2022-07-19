#include "M5EPD.h"
#include "binaryttf.h"
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <HTTPClient.h>
#include <Arduino_JSON.h>

#define TEXT_COLOR 15
#define BG_COLOR 0

M5EPD_Canvas canvas(&M5.EPD);
WiFiManager wm;

const char* ca = \ 
"-----BEGIN CERTIFICATE-----\n" \
"MIIDzTCCArWgAwIBAgIQCjeHZF5ftIwiTv0b7RQMPDANBgkqhkiG9w0BAQsFADBa\n" \
"MQswCQYDVQQGEwJJRTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJl\n" \
"clRydXN0MSIwIAYDVQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTIw\n" \
"MDEyNzEyNDgwOFoXDTI0MTIzMTIzNTk1OVowSjELMAkGA1UEBhMCVVMxGTAXBgNV\n" \
"BAoTEENsb3VkZmxhcmUsIEluYy4xIDAeBgNVBAMTF0Nsb3VkZmxhcmUgSW5jIEVD\n" \
"QyBDQS0zMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEua1NZpkUC0bsH4HRKlAe\n" \
"nQMVLzQSfS2WuIg4m4Vfj7+7Te9hRsTJc9QkT+DuHM5ss1FxL2ruTAUJd9NyYqSb\n" \
"16OCAWgwggFkMB0GA1UdDgQWBBSlzjfq67B1DpRniLRF+tkkEIeWHzAfBgNVHSME\n" \
"GDAWgBTlnVkwgkdYzKz6CFQ2hns6tQRN8DAOBgNVHQ8BAf8EBAMCAYYwHQYDVR0l\n" \
"BBYwFAYIKwYBBQUHAwEGCCsGAQUFBwMCMBIGA1UdEwEB/wQIMAYBAf8CAQAwNAYI\n" \
"KwYBBQUHAQEEKDAmMCQGCCsGAQUFBzABhhhodHRwOi8vb2NzcC5kaWdpY2VydC5j\n" \
"b20wOgYDVR0fBDMwMTAvoC2gK4YpaHR0cDovL2NybDMuZGlnaWNlcnQuY29tL09t\n" \
"bmlyb290MjAyNS5jcmwwbQYDVR0gBGYwZDA3BglghkgBhv1sAQEwKjAoBggrBgEF\n" \
"BQcCARYcaHR0cHM6Ly93d3cuZGlnaWNlcnQuY29tL0NQUzALBglghkgBhv1sAQIw\n" \
"CAYGZ4EMAQIBMAgGBmeBDAECAjAIBgZngQwBAgMwDQYJKoZIhvcNAQELBQADggEB\n" \
"AAUkHd0bsCrrmNaF4zlNXmtXnYJX/OvoMaJXkGUFvhZEOFp3ArnPEELG4ZKk40Un\n" \
"+ABHLGioVplTVI+tnkDB0A+21w0LOEhsUCxJkAZbZB2LzEgwLt4I4ptJIsCSDBFe\n" \
"lpKU1fwg3FZs5ZKTv3ocwDfjhUkV+ivhdDkYD7fa86JXWGBPzI6UAPxGezQxPk1H\n" \
"goE6y/SJXQ7vTQ1unBuCJN0yJV0ReFEQPaA1IwQvZW+cwdFD19Ae8zFnWSfda9J1\n" \
"CZMRJCQUzym+5iPDuI9yP+kHyCREU3qzuWFloUwOxkgAyXVjBYdwRVKD05WdRerw\n" \
"6DEdfgkfCv4+3ao8XnTSrLE=\n" \
"-----END CERTIFICATE-----\n";
   
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
  http.begin("https://api.coingecko.com/api/v3/simple/price?ids=bitcoin&vs_currencies=usd",ca); 
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
