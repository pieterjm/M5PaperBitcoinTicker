#include "M5EPD.h"
#include "binaryttf.h"
#include <WiFiManager.h> 
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <arduino-timer.h>

#define DISPLAY_PRICE 0
#define DISPLAY_BLOCKHEIGHT 1
#define DISPLAY_SATSUSD 2
#define DISPLAY_MSCW 3
#define DISPLAY_HASHRATE 4
#define DISPLAY_MAX 5

String legend[DISPLAY_MAX] = {
  "Market price of bitcoin",
  "Number of blocks in the blockchain",
  "Value of one US dollar in satoshis",
  "Moscow time",
  "Current bitcoin mining hashrate (EH/s)"
};


#define TEXT_COLOR 0
#define BG_COLOR 15

#define SCREEN_WIDTH 960
#define YPOS_LEGEND 400
#define YPOS_TICKER 150
#define XPOS_LEGEND 0
#define XPOS_TICKER 0
#define FONT_SIZE_LEGEND 48
#define FONT_SIZE_TICKER 240

int display = DISPLAY_PRICE;  // the ticker to display initially

M5EPD_Canvas canvasLegend(&M5.EPD);
M5EPD_Canvas canvasTicker(&M5.EPD);
M5EPD_Canvas canvasFull(&M5.EPD);

WiFiManager wm;
auto timer = timer_create_default();


const char* ca_coingecko = "-----BEGIN CERTIFICATE-----\n" \
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


const char* ca_mempoolspace = "-----BEGIN CERTIFICATE-----\n" \
"MIIGGTCCBAGgAwIBAgIQE31TnKp8MamkM3AZaIR6jTANBgkqhkiG9w0BAQwFADCB\n" \
"iDELMAkGA1UEBhMCVVMxEzARBgNVBAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0pl\n" \
"cnNleSBDaXR5MR4wHAYDVQQKExVUaGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNV\n" \
"BAMTJVVTRVJUcnVzdCBSU0EgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTgx\n" \
"MTAyMDAwMDAwWhcNMzAxMjMxMjM1OTU5WjCBlTELMAkGA1UEBhMCR0IxGzAZBgNV\n" \
"BAgTEkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UEBxMHU2FsZm9yZDEYMBYGA1UE\n" \
"ChMPU2VjdGlnbyBMaW1pdGVkMT0wOwYDVQQDEzRTZWN0aWdvIFJTQSBPcmdhbml6\n" \
"YXRpb24gVmFsaWRhdGlvbiBTZWN1cmUgU2VydmVyIENBMIIBIjANBgkqhkiG9w0B\n" \
"AQEFAAOCAQ8AMIIBCgKCAQEAnJMCRkVKUkiS/FeN+S3qU76zLNXYqKXsW2kDwB0Q\n" \
"9lkz3v4HSKjojHpnSvH1jcM3ZtAykffEnQRgxLVK4oOLp64m1F06XvjRFnG7ir1x\n" \
"on3IzqJgJLBSoDpFUd54k2xiYPHkVpy3O/c8Vdjf1XoxfDV/ElFw4Sy+BKzL+k/h\n" \
"fGVqwECn2XylY4QZ4ffK76q06Fha2ZnjJt+OErK43DOyNtoUHZZYQkBuCyKFHFEi\n" \
"rsTIBkVtkuZntxkj5Ng2a4XQf8dS48+wdQHgibSov4o2TqPgbOuEQc6lL0giE5dQ\n" \
"YkUeCaXMn2xXcEAG2yDoG9bzk4unMp63RBUJ16/9fAEc2wIDAQABo4IBbjCCAWow\n" \
"HwYDVR0jBBgwFoAUU3m/WqorSs9UgOHYm8Cd8rIDZsswHQYDVR0OBBYEFBfZ1iUn\n" \
"Z/kxwklD2TA2RIxsqU/rMA4GA1UdDwEB/wQEAwIBhjASBgNVHRMBAf8ECDAGAQH/\n" \
"AgEAMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjAbBgNVHSAEFDASMAYG\n" \
"BFUdIAAwCAYGZ4EMAQICMFAGA1UdHwRJMEcwRaBDoEGGP2h0dHA6Ly9jcmwudXNl\n" \
"cnRydXN0LmNvbS9VU0VSVHJ1c3RSU0FDZXJ0aWZpY2F0aW9uQXV0aG9yaXR5LmNy\n" \
"bDB2BggrBgEFBQcBAQRqMGgwPwYIKwYBBQUHMAKGM2h0dHA6Ly9jcnQudXNlcnRy\n" \
"dXN0LmNvbS9VU0VSVHJ1c3RSU0FBZGRUcnVzdENBLmNydDAlBggrBgEFBQcwAYYZ\n" \
"aHR0cDovL29jc3AudXNlcnRydXN0LmNvbTANBgkqhkiG9w0BAQwFAAOCAgEAThNA\n" \
"lsnD5m5bwOO69Bfhrgkfyb/LDCUW8nNTs3Yat6tIBtbNAHwgRUNFbBZaGxNh10m6\n" \
"pAKkrOjOzi3JKnSj3N6uq9BoNviRrzwB93fVC8+Xq+uH5xWo+jBaYXEgscBDxLmP\n" \
"bYox6xU2JPti1Qucj+lmveZhUZeTth2HvbC1bP6mESkGYTQxMD0gJ3NR0N6Fg9N3\n" \
"OSBGltqnxloWJ4Wyz04PToxcvr44APhL+XJ71PJ616IphdAEutNCLFGIUi7RPSRn\n" \
"R+xVzBv0yjTqJsHe3cQhifa6ezIejpZehEU4z4CqN2mLYBd0FUiRnG3wTqN3yhsc\n" \
"SPr5z0noX0+FCuKPkBurcEya67emP7SsXaRfz+bYipaQ908mgWB2XQ8kd5GzKjGf\n" \
"FlqyXYwcKapInI5v03hAcNt37N3j0VcFcC3mSZiIBYRiBXBWdoY5TtMibx3+bfEO\n" \
"s2LEPMvAhblhHrrhFYBZlAyuBbuMf1a+HNJav5fyakywxnB2sJCNwQs2uRHY1ihc\n" \
"6k/+JLcYCpsM0MF8XPtpvcyiTcaQvKZN8rG61ppnW5YCUtCC+cQKXA0o4D/I+pWV\n" \
"idWkvklsQLI+qGu41SWyxP7x09fn1txDAXYw+zuLXfdKiXyaNb78yvBXAfCNP6CH\n" \
"MntHWpdLgtJmwsQt6j8k9Kf5qLnjatkYYaA7jBU=\n" \
"-----END CERTIFICATE-----\n";

String blockheight = "";
String price = "";
String satsperusd = "";
String hashrate = "";

void update_blockheight()
{
  HTTPClient http;
  http.begin("https://mempool.space/api/blocks/tip/height",ca_mempoolspace); 
  int httpCode = http.GET();
  if(httpCode > 0) {
  
    // file found at server
    if(httpCode == HTTP_CODE_OK) {
      blockheight = http.getString();                
    }
  } 
  http.end();
}

int find_in_string(String str,String pat)
{
    int found = 0;
    int index = 0;
    int len;

    len = str.length();
    
    if (pat.length() > len) {
        return 0;
    }
    while (index < len) {
        if (str.charAt(index) == pat.charAt(found)) {          
            found++;
            if (pat.length() == found) {
                return (index - found);
            }
        }
        else {
            found = 0;
        }
        index++;
    }

    return 0;
}

void update_hashrate()
{
  HTTPClient http;
  http.begin("https://mempool.space/api/v1/mining/hashrate/3d",ca_mempoolspace); 
  int httpCode = http.GET();
  if(httpCode > 0) {
    // file found at server
    if(httpCode == HTTP_CODE_OK) {
      String payload = http.getString();                
      int start = find_in_string(payload,"currentHashrate");
      int stop = find_in_string(payload,"currentDifficulty");
      if ( start > 0 && stop > 0 ) {
        start += strlen("currentHashrate") + 3;
        stop -= 1;
        hashrate = payload.substring(start, stop);
        hashrate = hashrate.substring(0,hashrate.length() - 18);
      }
    }
  } 
  http.end();
}

void update_price()
{
  HTTPClient http;
  http.begin("https://api.coingecko.com/api/v3/simple/price?ids=bitcoin&vs_currencies=usd",ca_coingecko); 
  int httpCode = http.GET();
  if(httpCode > 0) {
    
    // file found at server
    if(httpCode == HTTP_CODE_OK) {
      String payload = http.getString();                
      JSONVar myObject = JSON.parse(payload);
      price = String("$") + String((int) myObject["bitcoin"]["usd"]);
      satsperusd = String(100000000 / (int) myObject["bitcoin"]["usd"]);
    }
  } 
  http.end();  
}

void display_ticker()
{
  canvasTicker.createCanvas(960,FONT_SIZE_TICKER);
  canvasTicker.fillCanvas(BG_COLOR);
  canvasTicker.setTextSize(FONT_SIZE_TICKER);
  canvasTicker.setTextColor(TEXT_COLOR);
  canvasTicker.setTextDatum(TC_DATUM);
  switch ( display ) {
    case DISPLAY_PRICE:
      canvasTicker.drawString(price, 480, 0);
      break;
     case DISPLAY_BLOCKHEIGHT:
      canvasTicker.drawString(blockheight, 480, 0);
      break;
    case DISPLAY_SATSUSD:
      canvasTicker.drawString(satsperusd, 480, 0);
      break;
    case DISPLAY_MSCW:
      canvasTicker.drawString(satsperusd.substring(0,2), 310, 0);
      canvasTicker.drawString(":", 480, -40);
      canvasTicker.drawString(satsperusd.substring(2), 640, 0);
      break;
    case DISPLAY_HASHRATE:
      canvasTicker.drawString(hashrate,480,0);
      break;
  }
  canvasTicker.pushCanvas(XPOS_TICKER, YPOS_TICKER, UPDATE_MODE_DU);  
}

void display_legend(String l) 
{
  canvasLegend.createCanvas(960,FONT_SIZE_LEGEND);
  canvasLegend.fillCanvas(BG_COLOR);
  canvasLegend.setTextColor(TEXT_COLOR);
  canvasLegend.setTextDatum(TC_DATUM);
  canvasLegend.setTextSize(FONT_SIZE_LEGEND);
  canvasLegend.drawString(l,480,0);
  canvasLegend.pushCanvas(XPOS_LEGEND, YPOS_LEGEND, UPDATE_MODE_DU);
}


bool update_display(void *)
{
  update_hashrate();
  update_price();
  update_blockheight();

  display_ticker();
  
  return true;
}

bool check_buttons(void *) 
{
  M5.update();  
  if( M5.BtnL.wasPressed()) { 
    display--;
    if (display < 0) {
      display = DISPLAY_MAX - 1;
    }
    display_legend(legend[display]);
    display_ticker();
  }
  if( M5.BtnR.wasPressed()) {
    display++;
    if (display >= DISPLAY_MAX ) {
      display = 0;
    }
    display_legend(legend[display]);
    display_ticker();
  }
  return true;
}


void setup()
{
  wm.setDebugOutput(false);
  Serial.setDebugOutput(false);  
  
  M5.begin();
  M5.EPD.Clear(true);


  canvasFull.createCanvas(960,540);
  canvasFull.fillCanvas(BG_COLOR);
  canvasFull.pushCanvas(0, 0, UPDATE_MODE_DU);  

  // initialize canvas of legend
  canvasLegend.loadFont(binaryttf, sizeof(binaryttf)); // Load font files from binary data
  canvasLegend.createRender(FONT_SIZE_TICKER);
  canvasLegend.createRender(FONT_SIZE_LEGEND);

  display_legend("Bitcoin Ticker");
  

  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  bool res;

  res = wm.autoConnect("BitcoinTicker"); // anonymous ap         
  if(!res) {
      ESP.restart();
  }     

  // Update the display and start an update time to refresh each minute
  update_hashrate();
  update_price();
  update_blockheight();

  display = 0;
  display_legend(legend[display]);
  display_ticker();
  
  //update_display((void *)false);
  timer.every(60000, update_display);    
  timer.every(100, check_buttons);
}

void loop()
{ 
  timer.tick(); 
}
