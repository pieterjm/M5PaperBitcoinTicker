# M5PaperBitcoinTicker
A bitcoin price ticker for the M5Stack M5Paper epaper device. Periodically retrieves the bitcoin price from the Coingecko API and displays that on the display.

# Installation

Installation can be done through the Arduino IDE in the following steps:

  1. Install the following Arduino libraries
    * Arduino_JSON (version 0.1.0)
    * M5EPD (version 0.1.4)
    * WifiManager (version 2.0.11-beta)
  2. Add the following board manager URL (under file, preferences) for M5Stack devices: https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/arduino/package_m5stack_index.json
  3. Clone this github repository
    git clone https://github.com/pieterjm/M5PaperBitcoinTicker.git
  4. Open M5PaperBitcoinTicker.ino in the Arduino IDE, compile and upload to the M5Paper

# Configuration of the ticker

The ticker requires access to a wireless network. To configure that, connect to the 'BitcoinTicker' network to configure the wireless network.




