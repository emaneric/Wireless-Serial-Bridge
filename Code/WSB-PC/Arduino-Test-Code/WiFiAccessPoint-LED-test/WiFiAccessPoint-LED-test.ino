#include <WiFi.h>
#include <NetworkClient.h>
#include <WiFiAP.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN 3  
#define LED_COUNT 4

// Set these to your desired credentials.
const char *ssid = "yourAP";
const char *password = "yourPassword";

NetworkServer server(80);
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// We store the HTML and CSS in a raw string literal to avoid escaping quotes
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { 
      font-family: Arial, sans-serif; 
      text-align: center; 
      margin-top: 50px; 
      background-color: #222; 
      color: #fff;
    }
    h1 { margin-bottom: 40px; }
    .button {
      display: inline-block; 
      padding: 15px 40px; 
      font-size: 20px; 
      font-weight: bold;
      cursor: pointer; 
      text-align: center; 
      text-decoration: none; 
      outline: none;
      color: #fff; 
      border: none; 
      border-radius: 8px; 
      box-shadow: 0 5px #555; 
      margin: 10px;
      transition: background-color 0.3s;
    }
    .btn-on { background-color: #4CAF50; } /* Green */
    .btn-on:hover { background-color: #45a049; }
    .btn-on:active { box-shadow: 0 2px #555; transform: translateY(4px); }
    
    .btn-off { background-color: #f44336; } /* Red */
    .btn-off:hover { background-color: #da190b; }
    .btn-off:active { box-shadow: 0 2px #555; transform: translateY(4px); }
  </style>
</head>
<body>
  <h1>NeoPixel Controller</h1>
  <a href="/H" class="button btn-on">Turn ON</a>
  <a href="/L" class="button btn-off">Turn OFF</a>
</body>
</html>
)rawliteral";

void setup() {
  
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(255); // Set BRIGHTNESS to about 1/5 (max = 255)

  Serial.begin(115200);
  Serial.println();
  Serial.println("Configuring access point...");

  // You can remove the password parameter if you want the AP to be open.
  // a valid password must have more than 7 characters
  if (!WiFi.softAP(ssid, password)) {
    log_e("Soft AP creation failed.");
    while (1);
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();

  Serial.println("Server started");
}

void loop() {
  NetworkClient client = server.accept();  // listen for incoming clients

  if (client) {                     // if you get a client,
    Serial.println("New Client.");  // print a message out the serial port
    String currentLine = "";        // make a String to hold incoming data from the client
    while (client.connected()) {    // loop while the client's connected
      if (client.available()) {     // if there's bytes to read from the client,
        char c = client.read();     // read a byte, then
        Serial.write(c);            // print it out the serial monitor
        if (c == '\n') {            // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // Send the HTML page
            client.print(htmlPage);

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {  // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          strip.setPixelColor(0, strip.Color(0, 255, 0));
          strip.setPixelColor(1, strip.Color(0, 255, 0));
          strip.setPixelColor(2, strip.Color(0, 255, 0));
          strip.setPixelColor(3, strip.Color(0, 255, 0));
          strip.show();
        }
        if (currentLine.endsWith("GET /L")) {
          strip.clear(); 
          strip.show();    
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}