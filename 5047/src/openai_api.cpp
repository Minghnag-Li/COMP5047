
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <openai_api.h>

const char *openai_api_key="";
const char *openai_url = "https://api.openai.com/v1/chat/completions";
WiFiClientSecure client;


String callOpenAI(const String& prompt) {
    String response = "";
    if (WiFi.status() == WL_CONNECTED) {
        // Disable SSL certificate verification
        client.setInsecure();
        
        HTTPClient https;
        https.setTimeout(60000);
        
        // Initialize HTTPS client with WiFiClientSecure
        if (https.begin(client, openai_url)) {
            // Add headers
            https.addHeader("Content-Type", "application/json");
            https.addHeader("Authorization", String("Bearer ") + openai_api_key);
            
            // Create JSON document for the request
            JsonDocument requestDoc;
            requestDoc["model"] = "gpt-4o";
            
            JsonArray messages = requestDoc.createNestedArray("messages");
            
            JsonObject userMessage = messages.createNestedObject();
            userMessage["role"] = "user";
            userMessage["content"] = prompt;
            
            requestDoc["max_tokens"] = 1000;
            requestDoc["temperature"] = 0.25;
            
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
                    if (responseDoc["choices"][0]["message"]) {
                        response = responseDoc["choices"][0]["message"]["content"].as<String>();
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


