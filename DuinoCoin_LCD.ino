/*
  If you don't know where to start, visit official Github or contact me on Facebook.
  If you need customize, add additional feature, report an error. Please contact me, I'm always happy to helping
  https://github.com/HEmbedded/Duino-coin-LCD
  https://www.facebook.com/phanvihao/
*/


/* If during compilation the line below causes a
  "fatal error: arduinoJson.h: No such file or directory"
  message to occur; it means that you do NOT have the
  ArduinoJSON library installed. To install it,
  go to the below link and follow the instructions:
  https://github.com/revoxhere/duino-coin/issues/832 */
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <stdint.h>

volatile uint8_t run_first_time = 0;

/***************** START OF USER CONFIGURATION SECTION *****************/
// Change the part in brackets to your WiFi name
const char *SSID = "Your_wifi_name";
// Change the part in brackets to your WiFi password
const char *PASSWORD = "Your_wifi_password";
// Change the part in brackets to your first opening screen (max 16 characters, include blank space)
const char *OPENING ="Ohayo onii-chan";
// Change the part in brackets to your name coin farm (max 3 characters, include blank space)
const char *NAME_COIN_FARM ="Your_name";
// Change the part in brackets to the language of your day of the week
String weekDays[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Change the number I2C pin (default is esp-01)
// ESP-01 (SDA: 2, SCL: 0), ESP-8266 (SDA: 4, SCL: 5)
const uint8_t SDA_PIN = 2;
const uint8_t SCL_PIN = 0;

// Change the part in brackets to your address link account (link to get API)
// e.i https://server.duinocoin.com/v3/users/your_id
const char *API_LINK = "https://server.duinocoin.com/v3/users/dannyfenton10";

// Comment the line below if you wish to not use an AHT10 or AHT20 sensor
#define USE_AHT
#ifdef USE_AHT
// Install "Adafruit AHTX0 Library" if you get an error
#include <Adafruit_AHTX0.h>
// AHT10/AHT20 should be connected to ESP default I2C pins
// i.e. ESP32 (I2C_SDA: GPIO_21 and I2C_SCL: GPIO_22), ESP-01s (GPIO_2: SDA, GPIO_0: SCL)
Adafruit_AHTX0 aht;
#endif

/* If you don't want to use the Serial interface comment
  the ENABLE_SERIAL definition line (#define ENABLE_SERIAL)*/
#define ENABLE_SERIAL

// Change the part in brackets to your nearest Server time NTP
// i.e Viet Nam (GMT+7): 3.vn.pool.ntp.org
const char *NTP_LINK = "3.vn.pool.ntp.org";

// Change the number to your GMT time zone (include minus sign if have)
const int8_t YOUR_GMT = +7 ;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_LINK);

// Change the number to change format of time
// 0: 24 hours, 1: 12 hours (AM,PM)
const uint8_t TIME_FORMAT_12H = 1;

// Change the number to change Date format
// 1: Day-Month-Year
// 2: Year-Month-Day
// 3: Month-Day-Year
const uint8_t DATE_FORMAT = 1;
String currentDate = "00-00-0000";

// If you use another LCD, you can change the number
// 16 is number of columns, 2 is number of rows of your LCD, 0x27 is the LCD I2C address
const uint8_t COLUMNS = 16;
const uint8_t ROWS = 2;
const uint8_t LCD_ADDRESS = 0x27;

LiquidCrystal_I2C lcd(LCD_ADDRESS, COLUMNS, ROWS);
/* ***************** END OF CONFIGURATION SECTION *****************
   Do not change the lines below. These lines are static and dynamic variables
   that will be used by the program for counters and measurements. */
   
//Custom icon, maximum 8 custom icon (0-7)
//You can design your custom icon here: https://omerk.github.io/lcdchargen/
byte miner_icon[8] = {
  0b00000,
  0b01000,
  0b10100,
  0b00010,
  0b00101,
  0b01001,
  0b10000,
  0b00000
};

//custom duino icon
byte duino_icon[8] = {
  0b11100,
  0b00010,
  0b11001,
  0b00101,
  0b00101,
  0b11001,
  0b00010,
  0b11100
};

//custom pointer stonk
byte stonk_icon[8] = {
  0b11111,
  0b00011,
  0b00101,
  0b00101,
  0b00101,
  0b01000,
  0b01000,
  0b10000
};

//Custom degree icon
byte degree_icon[8] = {
  0b00111,
  0b00101,
  0b00111,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

// Clear LCD line
void clearLCDLine(uint8_t line)
{
  lcd.setCursor(0, line);
  for (uint8_t n = 0; n < COLUMNS; n++)
  {
    lcd.print(" ");
  }
}

//Set up wifi
void SetupWifi() 
{

  #ifdef ENABLE_SERIAL
  Serial.println("Connecting to: " + String(SSID));
  #endif
  
  WiFi.mode(WIFI_STA); // Setup ESP in client mode
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.begin(SSID, PASSWORD);

  // Print to LCD connect wifi process status
  lcd.setCursor(0, 0);
  lcd.print("Connecting WIFI");

  //Make color :D
  for (uint8_t i = 0 ; i < random(4, 13) ; i++)
  {
    lcd.setCursor(i + 3, 1);
    lcd.print(".");
    delay(300);
  }

  //Wait esp connect to WIFI, if can't then reconnect
  uint8_t wait_passes = 0;
  while (WiFi.waitForConnectResult() != WL_CONNECTED) 
  {
    delay(500);
    if (++wait_passes >= 10) 
    {
      clearLCDLine(1);
      lcd.setCursor(0, 1);
      lcd.print("Connected fail !");
      delay(1000);

      clearLCDLine(1);
      lcd.setCursor(0, 1);
      lcd.print("Reconnecting...");
      delay(500);

      WiFi.begin(SSID, PASSWORD);
      wait_passes = 0;
      clearLCDLine(1);
    }
  }

  clearLCDLine(1);
  lcd.setCursor(0, 1);
  lcd.print("Connected ");
  
  // If your wifi character name is bigger than LCD column then only show "Connected"
  uint8_t SSID_length = strlen(SSID);
  if ( SSID_length + 13 <= COLUMNS ) //13 is number character of "Connected: "
  {
    lcd.print("to ");
    lcd.print(String(SSID));
  }

  #ifdef ENABLE_SERIAL
    Serial.println("\n\nSuccessfully connected to WiFi");
    Serial.println("Local IP address: " + WiFi.localIP().toString());
    Serial.println();
  #endif

  delay(1000);
  lcd.clear();
}

void setup() {

  /// Wait system power up
  delay(2000);
  // Setup I2C pins
  Wire.pins(SDA_PIN, SCL_PIN);

  // initialize the lcd
  // if you have error here then Comment lcd.begin() and Uncomment lcd.init() 
  // because some LiquidCrystal_I2C library has another function name
 
  lcd.begin();
  //lcd.init();

  // Clear lcd
  lcd.clear();
  // Turn on back light LCD
  lcd.backlight();


  // Create new characters:
  lcd.createChar(0, duino_icon);
  lcd.createChar(1, miner_icon);
  lcd.createChar(2, stonk_icon);
  lcd.createChar(3, degree_icon);


  #ifdef ENABLE_SERIAL
    Serial.begin(115200);
    Serial.setDebugOutput(true);
  #endif

  #ifdef USE_AHT
    #ifdef ENABLE_SERIAL
      Serial.println("Initializing AHT sensor");
    #endif

    if (! aht.begin()) 
    {
      #ifdef ENABLE_SERIAL
        Serial.println("Could not find AHT Sensor. Check wiring?");
      #endif

      lcd.setCursor(0, 0);
      lcd.print("AHT error !");
      delay(1000);
    }
  #endif

  lcd.setCursor(0, 0);
  lcd.print(String(OPENING));
  delay(1000);

  SetupWifi();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(String(NAME_COIN_FARM));
  lcd.print("'s coin farm");
  lcd.write(byte(0));

  lcd.setCursor(0, 1);
  lcd.print("Starting in");

  for (uint8_t t = 4; t > 0; t--) 
  {
    #ifdef ENABLE_SERIAL
      Serial.printf("[SETUP] WAIT %d...\n", t);
      Serial.flush();
    #endif

    lcd.setCursor(12, 1);
    lcd.print(t);
    delay(1000);
  }

  lcd.clear();

  timeClient.begin();
  timeClient.setTimeOffset(3600 * YOUR_GMT);
}

void loop() {

  if (run_first_time == 0)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connect -> SEVER");
    delay(500);
  }

// Start take data API

  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  //ignore the SSL certificate
  client->setInsecure();
  
  HTTPClient https;
  #ifdef ENABLE_SERIAL
    Serial.print("[HTTPS] begin...\n");
  #endif

  if (https.begin(*client, API_LINK))
  {
    if (run_first_time == 0)
    {
      lcd.setCursor(0, 1);
      lcd.print("Connected...");
      delay(1000);
    }

    if (run_first_time == 0)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Connecting HTTP");
      delay(500);
    }

    #ifdef ENABLE_SERIAL
      Serial.print("[HTTPS] GET...\n");
    #endif
    // start connection and send HTTP header
    int httpCode = https.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      
      #ifdef ENABLE_SERIAL
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
      #endif
      
      if (run_first_time == 0)
      {
        lcd.setCursor(0, 1);
        lcd.print("HTTP responsed");
        delay(1000);
      }

      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {

        if (run_first_time == 0)
        {
          lcd.clear();
          run_first_time++;
        }

        String payload = https.getString();
        
        #ifdef ENABLE_SERIAL
          Serial.println(payload);
        #endif
        
        DynamicJsonDocument doc(5000);
        const char*json = payload.c_str();
        deserializeJson(doc, json);

        float totalHashrate = 0.0;
        uint8_t total_miner = 0;

        JsonObject obj = doc.as<JsonObject>();
        JsonObject result = doc["result"];

        //If you want more information, go to: https://github.com/revoxhere/duco-rest-api
        float price = obj["result"]["prices"]["max"].as<float>();
        float balance = obj["result"]["balance"]["balance"].as<float>();

        
        for (JsonObject result_miner : result["miners"].as<JsonArray>())
        {
          float result_miner_hashrate = result_miner["hashrate"];
          totalHashrate = totalHashrate + result_miner_hashrate;
          total_miner++;
        }

        
        lcd.clear();
        //Line 0
        lcd.setCursor(0, 0);
        lcd.print("Balance:");
        lcd.print(balance);
        lcd.write(byte(0)); //Duino icon
        
        //Line 1
        lcd.setCursor(0, 1);
        lcd.print(String(total_miner));
        lcd.write(byte(1)); //Miner icon
        lcd.print(" ");
        lcd.print("kH/s:");
        lcd.print(String(totalHashrate / 1000));
        lcd.write(byte(2)); //Stonk icon
        delay(2000);
         
        //Line 1 - price
        float money = price * balance;
        clearLCDLine(1);
        lcd.setCursor(0, 1);
        lcd.print("USD:");
        lcd.print(money, 8);
        lcd.print("$");

        #ifdef ENABLE_SERIAL

          Serial.print("Price: ");
          Serial.print(price,9);
          Serial.print("\n");
          Serial.print("Total miner: ");
          Serial.println(total_miner);
          
          Serial.print("Total hash rate: ");
          Serial.println(totalHashrate / 1000);
          
          Serial.print("Balance: ");
          Serial.println(balance);
          Serial.print("USD: ");
          Serial.print(money, 8);
          Serial.print("\n");
        #endif
      }
    } 
    else 
    {
      #ifdef ENABLE_SERIAL
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      #endif
      
      lcd.setCursor(0, 1);
      lcd.print("Connected fail !");
      delay(1000);
    }
    https.end();
  } 
  else 
  {
    #ifdef ENABLE_SERIAL
      Serial.printf("[HTTPS] Unable to connect\n");
    #endif
    lcd.setCursor(0, 1);
    lcd.print("Connected fail !");
    delay(1000);
  }

//End take data from server
  delay(2000);
  
//Print time server NTP to LCD
  for (uint8_t i = 0; i < 4; i++)
  {
    delay(1100);
    clearLCDLine(1);
    timeClient.update();
    lcd.setCursor(0, 1);
    lcd.print("Time:");
    if (TIME_FORMAT_12H == 1)
    {
      //Time at 12:00:00 (24 hours format)
      if (timeClient.getHours() == 12)
      {
        lcd.print(timeClient.getFormattedTime());
        lcd.print(" ");
        lcd.print("PM");
      }
      //Time at 00:00:00 (24 hours format)
      else if (timeClient.getHours() == 0)
      {
        String time_12h;
        time_12h = timeClient.getFormattedTime();
        time_12h[0] = time_12h[0] + 1;
        time_12h[1] = time_12h[1] + 2;
        lcd.print(time_12h);
        lcd.print(" ");
        lcd.print("AM");
      }
      //Time before 12:00:00 (24 hours format)
      else if (timeClient.getHours() < 12 )
      {
        lcd.print(timeClient.getFormattedTime());
        lcd.print(" ");
        lcd.print("AM");
      }
      //Time after 12:59:59 (24 hours format)
      else if ( timeClient.getHours() == 20 || timeClient.getHours() == 21 ) //20:00:00 -> 21:59:59
      {
        String time_12h;
        time_12h = timeClient.getFormattedTime();
        time_12h[0] = time_12h[0] - 2;
        time_12h[1] = time_12h[1] + 8;
        lcd.print(time_12h);
        lcd.print(" ");
        lcd.print("PM");
      }
      else // 12:59:59 -> 19:59:59
      {
        String time_12h;
        time_12h = timeClient.getFormattedTime();
        time_12h[0] = time_12h[0] - 1;
        time_12h[1] = time_12h[1] - 2;
        lcd.print(time_12h);
        lcd.print(" ");
        lcd.print("PM");
      }
    }
    else
    {
      lcd.print(timeClient.getFormattedTime());
    }
    #ifdef ENABLE_SERIAL
      Serial.println(timeClient.getFormattedTime());
    #endif
  }
// End print time

  delay(1000);

// Start print Date
  time_t epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime ((time_t *)&epochTime);

  uint8_t monthDay = ptm->tm_mday;
  uint8_t currentMonth = ptm->tm_mon + 1;
  //String currentMonthName = months[currentMonth-1];
  int currentYear = ptm->tm_year + 1900;

  static uint8_t type = DATE_FORMAT;
  switch (type)
  {
    case 1:
      currentDate = String(monthDay) + "-" + String(currentMonth) + "-" + String(currentYear) ;
      break;
    case 2:
      currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay) ;
      break;
    case 3:
      currentDate = String(currentMonth) + "-" + String(monthDay) + "-" + String(currentYear) ;
      break;
    default:
      currentDate = "Format !!!" ;
      break;
  }

  clearLCDLine(1);
  lcd.setCursor(0, 1);
  lcd.print("Date:");
  lcd.print(String(currentDate));
  delay(2000);

  String weekDay = weekDays[timeClient.getDay()];
  clearLCDLine(1);
  lcd.setCursor(0, 1);
  lcd.print("Days:");
  lcd.print(weekDay);
  delay(2000);
// End print DATE

//Start AHT sensor
  clearLCDLine(1);
  #ifdef USE_AHT
    if (! aht.begin())
    {
      clearLCDLine(1);
      lcd.setCursor(0, 1);
      lcd.print("Couldn't find AH");
      delay(2000);
    }
    else
    {
      clearLCDLine(1);
      sensors_event_t humidity, temp;
      aht.getEvent(&humidity, &temp);
      lcd.setCursor(0, 1);
      lcd.print("Temp:");
      lcd.print(temp.temperature);
      lcd.write(byte(3)); //Degree icon
      lcd.print("C");
      delay(1000);
      for (uint8_t j = 0; j < 3 ; j++)
      {
        aht.getEvent(&humidity, &temp);
        lcd.setCursor(5, 1);
        lcd.print(temp.temperature);
        delay(1000);
      }

      clearLCDLine(1);
      aht.getEvent(&humidity, &temp);
      lcd.setCursor(0, 1);
      lcd.print("Humidity:");
      lcd.print(humidity.relative_humidity);
      lcd.print("%");
      for (uint8_t k = 0; k < 3 ; k++)
      {
        delay(1000);
        aht.getEvent(&humidity, &temp);
        lcd.setCursor(9, 1);
        lcd.print(humidity.relative_humidity);
      }
    }
  #endif
// End AHT sensor
}
