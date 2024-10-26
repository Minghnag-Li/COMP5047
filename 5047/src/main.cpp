

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <driver/i2s.h>
#include <esp_sleep.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
TinyGPSPlus gps;

// Define hardware serial port for GPS
HardwareSerial SerialGPS(2);

// I2S pins for ESP32 (for the MAX98357A amplifier)
#define I2S_BCLK 4   // Bit Clock pin
#define I2S_LRCLK 12  // Left-Right Clock pin
#define I2S_DIN 22    // Data Input pin

#define SWITCH_PIN 34

bool systemActive = false;
const char* ssid = "H"; //TODO: replace with wifi ssid
const char* password = "88888889"; //TODO: replace with wifi password

const char* openai_api_key = "";
const char* openai_url = "https://api.openai.com/v1/chat/completions";
WiFiClientSecure client;


// Function to configure I2S for audio
void setupI2S() {
    // Configure I2S
    i2s_config_t i2s_config = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX),  // Master, Transmit mode
        .sample_rate = 44100,   // 44.1kHz sample rate
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,  // 16-bit audio
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,  // Stereo format
        .communication_format = I2S_COMM_FORMAT_I2S_MSB,  // I2S communication format
        .intr_alloc_flags = 0,  // Default interrupt allocation
        .dma_buf_count = 8,     // Number of DMA buffers
        .dma_buf_len = 64,      // DMA buffer length
        .use_apll = false
    };

    // I2S pin configuration
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCLK,   // Bit Clock (BCLK)
        .ws_io_num = I2S_LRCLK,   // Left-Right Clock (LRCLK)
        .data_out_num = I2S_DIN,  // Data Out (DIN)
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    // Install and start I2S driver
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
}

// Function to generate a 1kHz tone
void generateTone() {
    int16_t sample = 0;
    size_t bytes_written;

    for (int i = 0; i < 44100; i++) {
        sample = 5000 * sin(2 * PI * 1000 * i / 44100.0);  // Generate 1kHz sine wave
        i2s_write(I2S_NUM_0, &sample, sizeof(sample), &bytes_written, portMAX_DELAY);
    }
}

/**
 * String callOpenAI(const String& prompt) {
    String response = "";
    if (WiFi.status() == WL_CONNECTED) {
        // Disable SSL certificate verification
        client.setInsecure();
        
        HTTPClient https;
        
        // Initialize HTTPS client with WiFiClientSecure
        if (https.begin(client, openai_url)) {
            // Add headers
            https.addHeader("Content-Type", "application/json");
            https.addHeader("Authorization", String("Bearer ") + openai_api_key);
            
            // Create JSON document for the request
            JsonDocument requestDoc;
            requestDoc["model"] = "gpt-3.5-turbo";
            requestDoc["messages"] = prompt;
            requestDoc["max_tokens"] = 100;
            requestDoc["temperature"] = 0.7;
            
            // Serialize JSON to string
            String requestBody;
            serializeJson(requestDoc, requestBody);
            
            // Make POST request
            int httpResponseCode = https.POST(requestBody);
            
            if (httpResponseCode > 0) {
                response = https.getString();
                
                // Parse the response
                JsonDocument responseDoc;
                DeserializationError error = deserializeJson(responseDoc, response);
                Serial.println(response);
                if (!error) {
                    // Extract the generated text from the response
                    if (responseDoc["choices"][0]["text"]) {
                        response = responseDoc["choices"][0]["text"].as<String>();
                    } else {
                        response = "Error: Unable to parse response choices";
                    }
                } else {
                    response = "Error: JSON parsing failed";
                }
            } else {
                response = "Error: HTTP request failed with code " + String(httpResponseCode);
            }
            
            https.end();
        } else {
            response = "Error: Unable to connect to OpenAI API";
        }
    } else {
        response = "Error: WiFi not connected";
    }
    
    return response;
}

            requestDoc["model"] = "gpt-3.5-turbo";
            
            // Create an array for messages
            JsonArray messages = requestDoc.createNestedArray("messages");
            
            // Add user message to the messages array
            JsonObject userMessage = messages.createNestedObject();
            userMessage["role"] = "user";
            userMessage["content"] = prompt;
            
            requestDoc["max_tokens"] = 100;
            requestDoc["temperature"] = 0.7;
 */

String callOpenAI(const String& prompt) {
    String response = "";
    if (WiFi.status() == WL_CONNECTED) {
        // Disable SSL certificate verification
        client.setInsecure();
        
        HTTPClient https;
        
        // Initialize HTTPS client with WiFiClientSecure
        if (https.begin(client, openai_url)) {
            // Add headers
            https.addHeader("Content-Type", "application/json");
            https.addHeader("Authorization", String("Bearer ") + openai_api_key);
            
            // Create JSON document for the request
            JsonDocument requestDoc;
            requestDoc["model"] = "gpt-3.5-turbo";
            
            JsonArray messages = requestDoc.createNestedArray("messages");
            
            JsonObject userMessage = messages.createNestedObject();
            userMessage["role"] = "user";
            userMessage["content"] = prompt;
            
            requestDoc["max_tokens"] = 100;
            requestDoc["temperature"] = 0.7;
            
            // Serialize JSON to string
            String requestBody;
            serializeJson(requestDoc, requestBody);
            
            // Make POST request
            int httpResponseCode = https.POST(requestBody);
            
            if (httpResponseCode > 0) {
                response = https.getString();
                
                // Parse the response
                JsonDocument responseDoc;
                DeserializationError error = deserializeJson(responseDoc, response);
                Serial.println(response);
                if (!error) {
                    // Extract the generated text from the response
                    if (responseDoc["choices"][0]["text"]) {
                        response = responseDoc["choices"][0]["text"].as<String>();
                    } else {
                        response = "Error: Unable to parse response choices";
                    }
                } else {
                    response = "Error: JSON parsing failed";
                }
            } else {
                response = "Error: HTTP request failed with code " + String(httpResponseCode);
            }
            
            https.end();
        } else {
            response = "Error: Unable to connect to OpenAI API";
        }
    } else {
        response = "Error: WiFi not connected";
    }
    
    return response;
}

void setup() {
    // Start the serial communication for debugging
    Serial.begin(115200);

    // Start the serial communication for GPS (D10=GPIO16 RX, D11=GPIO17 TX)
    SerialGPS.begin(9600, SERIAL_8N1, 16, 17);

    Serial.println("GPS Module with FireBeetle ESP32-E Setup");

    // Initialize I2S
    setupI2S();
    // Configure switch pin
    pinMode(SWITCH_PIN, INPUT);
    WiFi.begin(ssid, password); // Begin the connection to the specified Wi-Fi network.
    while (WiFi.status() != WL_CONNECTED) { // Check the connection status.
        delay(1000); // Wait 1 second before trying again.
        Serial.println("Connecting to Wi-Fi..."); // Print a message to the serial monitor.
    }
    Serial.println("Connected to Wi-Fi"); // Print a message when successfully connected.
    String response = callOpenAI("Please generate a 500 words long story for kids with the theme of futuristic");
    Serial.println(response);
    // Configure wakeup source
    esp_sleep_enable_ext0_wakeup(static_cast<gpio_num_t>(SWITCH_PIN), 1);  // Wake up when switch is turned ON
}

void loop() {

    // Check the state of the switch
    bool currentSwitchState = digitalRead(SWITCH_PIN);
    if (currentSwitchState == LOW) {
        Serial.println("System is OFF, entering deep sleep");
        delay(100);  // Small delay before sleep
        esp_deep_sleep_start();  // Enter deep sleep
    } else if (currentSwitchState == HIGH && !systemActive) {
        systemActive = true;
        Serial.println("System is ON");
    }

    // If the system is active, proceed with GPS reading and tone generation
    if (systemActive) {
        // Read data from GPS module
        while (SerialGPS.available() > 0) {
            gps.encode(SerialGPS.read());
        }

        // If GPS data is valid, print it
        if (gps.location.isUpdated()) {
            Serial.print("Latitude: ");
            Serial.println(gps.location.lat(), 6);
            Serial.print("Longitude: ");
            Serial.println(gps.location.lng(), 6);
            Serial.print("Altitude: ");
            Serial.println(gps.altitude.meters());
            Serial.print("Satellites: ");
            Serial.println(gps.satellites.value());
        }

        // Play a short 1kHz tone over I2S, with breaks to ensure GPS can process data
        generateTone();
    }

    delay(100);  // Small delay to prevent excessive loop iteration
}
