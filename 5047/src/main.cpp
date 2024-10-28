

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <driver/i2s.h>
#include <esp_sleep.h>
#include <ArduinoJson.h>
#include <iostream>
#include <string>
#include <google_tts.h>
#include <openai_api.h>
#include <global.h>

TinyGPSPlus gps;

// Define hardware serial port for GPS
HardwareSerial SerialGPS(2);



bool systemActive = false;
bool onQuest = true;

volatile unsigned long redPulseCount = 0;
volatile unsigned long greenPulseCount = 0;
volatile unsigned long bluePulseCount = 0;
// const char* openai_api_key = "";
// const char* openai_url = "https://api.openai.com/v1/chat/completions";
const char *ssid = "StephenPh";    // TODO: replace with wifi ssid
const char *password = "Abcd@123";
String textToTranserToTTS;
// WiFiClientSecure client;
int player_num = 0;
WiFiClient *streamClient;
HTTPClient http;
String prompt = R"(Please generate a 1000 words story for kids from age 5 to 10, this text generated will: 
-Have a futuristic setting 
-Will have 2 number of main characters interacting with each others \n
-Will have at least 5 quests embbeded into the story line, you can add the quest anywhere you want during the story, there are 2 types of quest I want you to add: \n
    -Find an object with color {either Red, Green or Blue} or Move a distance of {3m or 6m}. \n
-Important: There MUST have an asterisk token * after every quest introduction line and there MUST be NO token before the quest introduction line(like this: \n 
One day, Luna and Orion decided to explore the city and see what adventures awaited them. As they walked down the bustling streets, they saw a sign that read: "The Great Robot Race - Winner 
Receives a Golden Key to the City." Excited by the prospect of a challenge, Luna and Orion decided to enter the race. \n
Find an object with color Blue *). \n
-No need to add new line (\n) character in the generated text. \n
-No need to add "Quest introduce" before introducing a quest and don't put it in curly braces.)";
                                
hw_timer_t *timer = NULL;


void IRAM_ATTR onTimer() {
    redPulseCount = 0;
    greenPulseCount = 0; 
    bluePulseCount = 0;
  static int currentColor = 0; // 0: Red, 1: Green, 2: Blue
  if (currentColor == 0) {
    digitalWrite(S2, LOW);
    digitalWrite(S3, LOW);
    redPulseCount = pulseIn(OUT_PIN, LOW, 1000000);  
    currentColor = 1;  // 切换到绿色
  } else if (currentColor == 1) {
    digitalWrite(S2, HIGH);
    digitalWrite(S3, HIGH);
    greenPulseCount = pulseIn(OUT_PIN, LOW, 1000000);  // 读取脉冲
    currentColor = 2;  // 切换到蓝色
  } else if (currentColor == 2) {
    digitalWrite(S2, LOW);
    digitalWrite(S3, HIGH);
    bluePulseCount = pulseIn(OUT_PIN, LOW, 1000000);  // 读取脉冲
    currentColor = 0;  // 切换回红色
  }
}


// Function to configure I2S for audio
void setupI2S()
{
    // Configure I2S
    i2s_config_t i2s_config = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX), // Master, Transmit mode
        .sample_rate = 24000,                              // 44.1kHz sample rate
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,      // 16-bit audio
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,      // Stereo format
        .communication_format = I2S_COMM_FORMAT_I2S_MSB,   // I2S communication format
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,                             // Default interrupt allocation
        .dma_buf_count = 8,                                // Number of DMA buffers
        .dma_buf_len = 128,                                 // DMA buffer length
        .use_apll = false,
        .tx_desc_auto_clear = true};

    // I2S pin configuration
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCLK,  // Bit Clock (BCLK)
        .ws_io_num = I2S_LRCLK,  // Left-Right Clock (LRCLK)
        .data_out_num = I2S_DIN, // Data Out (DIN)
        .data_in_num = I2S_PIN_NO_CHANGE};

    // Install and start I2S driver
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
    i2s_set_clk(I2S_NUM_0, 24000, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
}

// Function to generate a 1kHz tone
void generateTone()
{
    int16_t sample = 0;
    size_t bytes_written;

    for (int i = 0; i < 44100; i++)
    {
        sample = 5000 * sin(2 * PI * 1000 * i / 44100.0); // Generate 1kHz sine wave
        i2s_write(I2S_NUM_0, &sample, sizeof(sample), &bytes_written, portMAX_DELAY);
    }
}

bool checkColorQuest(const String& color){
    if (color == "red"){
        return true;
    }
    return false;
}

#pragma region callOpenAI
// String callOpenAI(const String& prompt) {
//     String response = "";
//     if (WiFi.status() == WL_CONNECTED) {
//         // Disable SSL certificate verification
//         client.setInsecure();
        
//         HTTPClient https;
        
//         // Initialize HTTPS client with WiFiClientSecure
//         if (https.begin(client, openai_url)) {
//             // Add headers
//             https.addHeader("Content-Type", "application/json");
//             https.addHeader("Authorization", String("Bearer ") + openai_api_key);
            
//             // Create JSON document for the request
//             JsonDocument requestDoc;
//             requestDoc["model"] = "gpt-3.5-turbo";
            
//             JsonArray messages = requestDoc.createNestedArray("messages");
            
//             JsonObject userMessage = messages.createNestedObject();
//             userMessage["role"] = "user";
//             userMessage["content"] = prompt;
            
//             requestDoc["max_tokens"] = 400;
//             requestDoc["temperature"] = 0.7;
            
//             // Serialize JSON to string
//             String requestBody;
//             serializeJson(requestDoc, requestBody);
            
//             // Make POST request
//             int httpResponseCode = https.POST(requestBody);
            
//             if (httpResponseCode > 0) {
//                 response = https.getString();
                
//                 // Parse the response
//                 JsonDocument responseDoc;
//                 DeserializationError error = deserializeJson(responseDoc, response);
//                 Serial.println(response);
//                 if (!error) {
//                     // Extract the generated text from the response
//                     if (responseDoc["choices"][0]["text"]) {
//                         response = responseDoc["choices"][0]["text"].as<String>();
//                     } else {
//                         response = "Error: Unable to parse response choices";
//                     }
//                 } else {
//                     response = "Error: JSON parsing failed";
//                 }
//             } else {
//                 response = "Error: HTTP request failed with code " + String(httpResponseCode);
//             }
            
//             https.end();
//         } else {
//             response = "Error: Unable to connect to OpenAI API";
//         }
//     } else {
//         response = "Error: WiFi not connected";
//     }
    
//     return response;
// }

#pragma endregion

void setup() {
    Serial.begin(115200);
    pinMode(S0, OUTPUT);
    pinMode(S1, OUTPUT);
    pinMode(S2, OUTPUT);
    pinMode(S3, OUTPUT);
    pinMode(OUT_PIN, INPUT);
    digitalWrite(S0, HIGH);
    digitalWrite(S1, HIGH);
    timer = timerBegin(0, 80, true);  // Timer 0, 分频80
    timerAttachInterrupt(timer, &onTimer, true);  // 绑定中断服务函数
    timerAlarmWrite(timer, 1000000, true);  // 1秒触发一次
    timerAlarmEnable(timer);  // 启用定时器警报

    // Start the serial communication for debugging
    Serial.begin(115200);

    // Start the serial communication for GPS (D10=GPIO16 RX, D11=GPIO17 TX)
    SerialGPS.begin(9600, SERIAL_8N1, 16, 17);

    Serial.println("GPS Module with FireBeetle ESP32-E Setup");
    WiFi.begin(ssid, password); // Connect to the Wi-Fi network.
    int count = 0;
    while (WiFi.status() != WL_CONNECTED)
    {                                             // Check the connection status.
        delay(1000);                              // Wait 1 second before trying again.
        Serial.println("Connecting to Wi-Fi..."); // Print a message to the serial monitor.
        if (count++ > 10)
        {
            WiFi.disconnect();
            Serial.println("Disconnecting to Wi-Fi...");
            count = 0;
            delay(1000);
            WiFi.begin(ssid, password);
        }
    }
    Serial.println("Connected to Wi-Fi"); // Confirm the Wi-Fi connection.

    // Initialize I2S
    setupI2S();
    // Configure switch pin
    pinMode(SWITCH_PIN, INPUT);
   
    textToTranserToTTS = callOpenAI(prompt);
    Serial.println(textToTranserToTTS);

    SetAudioConfig(I2S_BCLK, I2S_LRCLK, I2S_DIN);


    bool result = RequestBackendTTS(textToTranserToTTS);
    
    Serial.println("RequestBackendTTS result: ");
    Serial.println(result);

    // Configure wakeup source
    esp_sleep_enable_ext0_wakeup(static_cast<gpio_num_t>(SWITCH_PIN), 1); // Wake up when switch is turned ON
}

int getRandom123() {
    // Seed the random number generator with the current time
    static std::random_device rd;  // Non-deterministic random number
    static std::mt19937 gen(rd()); // Mersenne Twister engine seeded with rd()
    static std::uniform_int_distribution<int> dist(1, 3); // Range [1, 3]
    delay(1000);
    return dist(gen); // Generate a random number in the range 1 to 3

}

void colorChecking(int random_color){

    Serial.print("Red: ");
    if (redPulseCount > 0) {
        Serial.print(1000000 / redPulseCount);
    } else {
        Serial.println("N/A");
    }
    Serial.print("  Green: ");
    if (greenPulseCount > 0) {
        Serial.print(1000000 / greenPulseCount);
    } else {
        Serial.print("N/A");
    }
    Serial.print("  Blue: ");
    if (bluePulseCount > 0) {
        Serial.print(1000000 / bluePulseCount);
    } else {
        Serial.print("N/A");
    }
    Serial.println();
    delay(1000);  
}

void GPSturnOn(){
    // If the system is active, proceed with GPS reading and tone generation
    if (systemActive)
    {
        // Read data from GPS module
        while (SerialGPS.available() > 0)
        {
            gps.encode(SerialGPS.read());
        }

        // If GPS data is valid, print it
        if (gps.location.isUpdated())
        {
            Serial.print("Latitude: ");
            Serial.println(gps.location.lat(), 6);
            Serial.print("Longitude: ");
            Serial.println(gps.location.lng(), 6);
            Serial.print("Altitude: ");
            Serial.println(gps.altitude.meters());
            Serial.print("Satellites: ");
            Serial.println(gps.satellites.value());
        }
    }
}


void loop() {

    // Check if the program start

    bool currentSwitchState = digitalRead(SWITCH_PIN);
    if (currentSwitchState == LOW) {
        Serial.println("System is OFF, entering deep sleep");
        delay(100);  // Small delay before sleep
        esp_deep_sleep_start();  // Enter deep sleep
    } else if (currentSwitchState == HIGH && !systemActive) {
        systemActive = true;
        Serial.println("System is ON");
    }
    //execute everything down here if ON
    if(player_num > 0) {
        //Put the player_num into the request body to send AI
        //Send request to AI
        //Get the story from AI
        //Send the text to Django backend
        //Django backend will generate voice file
        //Django backend send voice files to this module
        //

    }

   

    delay(100); // Small delay to prevent excessive loop iteration
}
