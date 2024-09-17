#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266Ping.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

// ----------------------------(DECLARATION)----------------------------

WiFiUDP udp;
HTTPClient http;
WiFiClient wifiClient;
ESP8266WebServer server(80);

const byte server_mac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
IPAddress server_ip(192, 168, 0, 0);
IPAddress broadcast_ip(192, 168, 0, 0);
const char shutdown_url[] = "your_shutdown_url";
const char ssid[] = "your_ssid";
const char pass[] = "your_password";

void sendMagicPacket(const byte mac[]);
void connectToWiFi(const char ssid[], const char pass[]);
bool isAwake(IPAddress ip);
void awakeServer();
void handleConnect();
String sendHTML(String message);
void configAndStartWebServer();
void shutdownServer();
void initHTTPClient();

// ----------------------------(MAIN)----------------------------

void setup()
{
  // Start the serial communication
  Serial.begin(9600);
  // Connect to the WiFi
  connectToWiFi(ssid, pass);
  configAndStartWebServer();
  initHTTPClient();
}

void loop()
{
  server.handleClient();
}

// ----------------------------(DEFINITION)----------------------------

void initHTTPClient()
{
  http.begin(wifiClient, shutdown_url);
  http.addHeader("Content-Type", "application/json");
  Serial.println("HTTP client initialized");
}

void shutdownServer()
{
  http.POST("");
  server.send(200, "text/html", sendHTML("The server is shutting down"));
}

void awakeServer()
{
  sendMagicPacket(server_mac);
  server.send(200, "text/html", sendHTML("The server is waking up"));
}

String sendHTML(String message)
{
  String ptr = "<!DOCTYPE html>\n";
  ptr += "<html>\n";
  ptr += "<head>\n";
  ptr += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n";
  ptr += "<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/4.5.2/css/bootstrap.min.css\">\n";
  ptr += "<title>Server status</title>\n";
  ptr += "</head>\n";
  ptr += "<body class=\"d-flex flex-column text-center min-vh-100\">\n";
  ptr += "<div class=\"container\">\n";
  ptr += "<h1 class=\"mt-5\">Server status</h1>\n";
  ptr += "<div class=\"d-flex flex-column justify-content-center flex-grow-1 mt-5\">\n";
  ptr += "<h3 class=\"mt-5\">" + message + "</h3>\n";
  ptr += "<br/>\n";
  ptr += "<a class=\"btn btn-primary\" href=\"/awake\">Wake up</a>\n";
  ptr += "<br/><br/>\n";
  ptr += "<a class=\"btn btn-danger\" href=\"/shutdown\">Shutdown</a>\n";
  ptr += "<br/><br/>\n";
  ptr += "<a class=\"btn btn-secondary\" href=\"/\">Refresh</a>\n";
  ptr += "</div>\n";
  ptr += "</div>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}

void configAndStartWebServer()
{
  server.on("/", handleConnect);
  server.on("/awake", awakeServer);
  server.on("/shutdown", shutdownServer);
  server.begin();
  Serial.println("Web server started");
}

void handleConnect()
{
  if (isAwake(server_ip))
  {
    server.send(200, "text/html", sendHTML("The server is awake"));
  }
  else
  {
    server.send(200, "text/html", sendHTML("The server is shut"));
  }
}

void connectToWiFi(const char ssid[], const char pass[])
{
  Serial.println();
  Serial.println("Connecting to WiFi");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected to WiFi");
}

void sendMagicPacket(const byte mac[])
{
  byte packet[102];
  byte index = 6;
  for (byte i = 0; i < 6; i++)
  {
    packet[i] = 0xFF;
  }
  for (byte i = 0; i < 16; i++)
  {
    for (byte j = 0; j < 6; j++)
    {
      packet[index] = mac[j];
      index++;
    }
  }
  udp.beginPacket(broadcast_ip, 9);
  udp.write(packet, 102);
  udp.endPacket();
}

bool isAwake(IPAddress ip)
{
  if (Ping.ping(ip, 3))
  {
    return true;
  }
  else
  {
    return false;
  }
}
