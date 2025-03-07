#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <time.h>

#define DHTPIN 14        
#define DHTTYPE DHT11  
#define LDR_PIN 32     

DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "FTTX-cad8fe";
const char* password = "fe210873";

const char* firestoreUrl = "https://firestore.googleapis.com/v1/projects/integ-act4/databases/(default)/documents/act4?key=AIzaSyBH7LGvnXaptk4m6KWCirP1aFThFCjmL38";

const char* ntpServer = "time.google.com"; 
const long gmtOffset_sec = 8 * 3600; 
const int daylightOffset_sec = 0; 

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    dht.begin();

    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi!");

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    Serial.println("Time synchronization complete.");
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo)) {
            Serial.println("Failed to obtain time");
            return;
        }

        char timestamp[30];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", &timeinfo);

        
        float temperature = dht.readTemperature(); 
        float humidity = dht.readHumidity();      

        
        int ldrValue = analogRead(LDR_PIN); 

        
        if (isnan(temperature) || isnan(humidity)) {
            Serial.println("Failed to read from DHT sensor!");
            return;
        }

        HTTPClient http;
        http.begin(firestoreUrl);
        http.addHeader("Content-Type", "application/json");

        
        String jsonData = String(R"({
            "fields": {
                "temperature": {"doubleValue": )") + temperature + R"(},
                "humidity": {"doubleValue": )" + humidity + R"(},
                "light_level": {"integerValue": )" + ldrValue + R"(},
                "timestamp": {"stringValue": ")" + timestamp + R"("}
            }
        })";

        int httpResponseCode = http.POST(jsonData);
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);

        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("Response: " + response);
        } else {
            Serial.println("Error: " + http.errorToString(httpResponseCode));
        }

        http.end();
    } else {
        Serial.println("WiFi disconnected. Reconnecting...");
        WiFi.begin(ssid, password);
    }

    delay(5000); 
}
