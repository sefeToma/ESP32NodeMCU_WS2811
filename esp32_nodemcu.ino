//https://randomnerdtutorials.com/esp32-web-server-arduino-ide/
//https://randomnerdtutorials.com/esp32-esp8266-rgb-led-strip-web-server/

// Load Wi-Fi library
#include <WiFi.h>
#include <FastLED.h>

#define LED_PIN CustomBoardPin
#define NUM_LEDS CustomNumberLEDs

#define LED_TYPE WS2811
#define COLOR_ORDER RGB
CRGB leds[NUM_LEDS];

// Replace with your network credentials
const char* ssid = "WiFi name";
const char* password = "WiFi password";

// Set web server port number to 80
WiFiServer server(81);

// Variable to store the HTTP request
String header;

// Variables to extract RGB values from HTTP request
int pos1 = 0;
int pos2 = 0;
int pos3 = 0;
int pos4 = 0;

// Variables to store RGB values
int redValue = 245;
int blueValue = 52;
int greenValue = 60;

CRGB RBG_Color;

int BRIGHTNESS = 80;

bool boolOnOff = 1;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;


/*
  CUSTOM FUNCTIONS
*/
int checkValue(int valueRGB){
  if (valueRGB > 255){
    return 255;
  }
  return valueRGB;
}

int checkBRIGHT(int valueBRIGHT){
  if (valueBRIGHT > 100){
    return 100;
  }
  return valueBRIGHT;
}

void setup() {
  // Open Serial connection
  Serial.begin(115200);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();


  RBG_Color = CRGB(greenValue, redValue, blueValue);

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // Set RGB values
            // Example http://BoardCustomIP:Port/?r221g32b65&
            if (header.indexOf("GET /?r") >= 0){
              pos1 = header.indexOf("r");
              pos2 = header.indexOf("g");
              pos3 = header.indexOf("b");
              pos4 = header.indexOf("&");

              redValue = checkValue(header.substring(pos1 + 1, pos2).toInt());
              greenValue = checkValue(header.substring(pos2 + 1, pos3).toInt());
              blueValue = checkValue(header.substring(pos3 + 1, pos4).toInt());

              Serial.println(redValue);
              Serial.println(greenValue);
              Serial.println(blueValue);

              RBG_Color = CRGB(greenValue, redValue, blueValue);
            }

            // Set LEDs BRIGHTNESS
            // Example http://BoardCustomIP:Port/?b80&
            else if (header.indexOf("GET /?b") >= 0){
              pos1 = header.indexOf("b");
              pos2 = header.indexOf("&");

              BRIGHTNESS = checkBRIGHT(header.substring(pos1 + 1, pos2).toInt());

              Serial.println(BRIGHTNESS);

              FastLED.setBrightness(BRIGHTNESS);
            }

            // Turn LEDs ON or OFF
            // Example http://BoardCustomIP:Port/?o
            else if (header.indexOf("GET /?o") >= 0){
              if (boolOnOff){
                boolOnOff = false;
              }
              else{
                boolOnOff = true;
              }
              Serial.println(boolOnOff);
            }

            // Break out of the while loop
            break;
          } 
          else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } 
        else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;    // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";

    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
  
  // Show LEDs
  if (boolOnOff) {
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = RBG_Color;
    }
    FastLED.show();
  }
}