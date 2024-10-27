

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <driver/i2s.h>
#include <esp_sleep.h>
#include <ArduinoJson.h>
<<<<<<< HEAD
#include <iostream>
#include <string>
=======
#include <google_tts.h>
#include <openai_api.h>

>>>>>>> f0a1c32a75822842f4dadaa6b18ca5ad1fce4c5d
TinyGPSPlus gps;

// Define hardware serial port for GPS
HardwareSerial SerialGPS(2);

// I2S pins for ESP32 (for the MAX98357A amplifier)
#define I2S_BCLK 4   // Bit Clock pin
#define I2S_LRCLK 12 // Left-Right Clock pin
#define I2S_DIN 22   // Data Input pin

#define SWITCH_PIN 34

#define S0 13
#define S1 14
#define S2 0
#define S3 26
#define OUT_PIN 25

bool systemActive = false;
<<<<<<< HEAD
bool onQuest = false;
const char* ssid = "H"; //TODO: replace with wifi ssid
const char* password = "88888889"; //TODO: replace with wifi password

const char* openai_api_key = "";
const char* openai_url = "https://api.openai.com/v1/chat/completions";
WiFiClientSecure client;
int player_num = 0;
std::stringstream prompt;



volatile unsigned long redPulseCount = 0;
volatile unsigned long greenPulseCount = 0;
volatile unsigned long bluePulseCount = 0;
hw_timer_t *timer = NULL;


void IRAM_ATTR onTimer() {
  static int currentColor = 0; // 0: Red, 1: Green, 2: Blue
  if (currentColor == 0) {
    digitalWrite(S2, LOW);
    digitalWrite(S3, LOW);
    redPulseCount = pulseIn(OUT_PIN, LOW, 1000000);  // 读取脉冲
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
=======
const char *ssid = "YUKARISAW";    // TODO: replace with wifi ssid
const char *password = "88888889"; // TODO: replace with wifi password
>>>>>>> f0a1c32a75822842f4dadaa6b18ca5ad1fce4c5d

WiFiClient *streamClient;
HTTPClient http;

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

<<<<<<< HEAD
bool checkColorQuest(const String& color){
    if (color == "red"){
        return true
    }
    return false
}

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
            
            requestDoc["max_tokens"] = 400;
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
    Serial.begin(115200);
    pinMode(S0, OUTPUT);
    pinMode(S1, OUTPUT);
    pinMode(S2, OUTPUT);
    pinMode(S3, OUTPUT);
    pinMode(OUT_PIN, INPUT);
    // 
    digitalWrite(S0, HIGH);
    digitalWrite(S1, HIGH);
    timer = timerBegin(0, 80, true);  // Timer 0, 分频80
    timerAttachInterrupt(timer, &onTimer, true);  // 绑定中断服务函数
    timerAlarmWrite(timer, 1000000, true);  // 1秒触发一次
    timerAlarmEnable(timer);  // 启用定时器警报
=======
void setup()
{
    // Start the serial communication for debugging
>>>>>>> f0a1c32a75822842f4dadaa6b18ca5ad1fce4c5d
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
<<<<<<< HEAD
    WiFi.begin(ssid, password); // Begin the connection to the specified Wi-Fi network.
    while (WiFi.status() != WL_CONNECTED) { // Check the connection status.
        delay(1000); // Wait 1 second before trying again.
        Serial.println("Connecting to Wi-Fi..."); // Print a message to the serial monitor.
    }
    Serial.println("Connected to Wi-Fi");
    String response = callOpenAI("Please generate a 500 words long story for kids with the theme of futuristic, this story");
    Serial.println(response);
=======


    // String response = callOpenAI("Please generate a 500 words long story for kids with the theme of futuristic");
    // Serial.println(response);

    // SetAudioConfig(I2S_BCLK, I2S_LRCLK, I2S_DIN);

    String text = R"(Once upon a time, in a village at the edge of the Whispering Woods, there was a little girl named Lila. She had sparkling eyes, a wild, adventurous heart, and a great love for the colors of the world. But among all the colors, her favorite was the rainbow. She loved the way each color blended into the next, creating a bridge between the earth and the sky.

One sunny day after a rain shower, Lila ran outside to spot the rainbow. She gasped with excitement and searched the skies. But to her surprise, there was no rainbow! The sky was clear, and all she could see were puffy white clouds. She waited and waited, but the rainbow never appeared.

Confused, Lila went to see her friend, Grandma Willow. Grandma Willow was the oldest, wisest woman in the village, and everyone trusted her. Her hair was like silver strands of moonlight, and her smile was warm as summer.

“Grandma Willow, where is the rainbow?” Lila asked. “The rain has stopped, the sun is shining, but there’s no rainbow!”

Grandma Willow closed her eyes and felt the breeze. She listened to the whispers in the wind, then opened her eyes wide. “Oh, dear Lila,” she said, “I believe the rainbow has gone missing!”

Lila’s eyes widened. “Missing? But how can a rainbow just disappear?”

Grandma Willow took her hand and led her to the edge of the Whispering Woods. “If the rainbow is missing, then it must be lost in the Magic Lands,” she said. “The Magic Lands are places only brave hearts can visit. But be careful, for each color of the rainbow has its own spirit, and they may not be so easily convinced to return.”

Lila’s heart was pounding with excitement. “I’ll bring back the rainbow, Grandma Willow! I promise!”

And with that, Lila stepped into the Whispering Woods, guided by the songs of the trees.

The first color she came upon was red. It was a fiery, lively spirit that danced like a flame, crackling with energy. “Why have you come to find me?” Red asked, its voice bold and strong.

“I need to bring you back to the rainbow,” Lila said. “Without you, the world has lost its colors.”

Red laughed a deep, rumbling laugh. “If you want me to return, show me your bravery!”

Just then, a wild storm brewed in front of her. Lightning crackled, and thunder roared. Lila took a deep breath and marched through the storm without looking back, proving her bravery. When she turned around, Red was by her side.

Next, she met Orange, who shimmered like a warm, setting sun. “I am the spirit of creativity and joy. To bring me back, you must make something beautiful.”

Lila thought for a moment, then gathered leaves, twigs, and flowers. She crafted a small crown and placed it on her head, dancing and spinning with joy. Orange chuckled with delight and decided to join her.

They moved forward, and soon, Lila found Yellow hiding in a field of golden flowers. Yellow glowed with a soft, gentle warmth. “I am the color of hope and light. To bring me back, you must prove you have a heart full of kindness.”

Just then, Lila noticed a tiny bird trapped in some thorny vines. She carefully freed the little bird, who chirped gratefully and flew away. Yellow smiled and joined her.

Green was next, and it appeared as a lush, wise tree with deep emerald leaves. “I am the spirit of nature and growth. To bring me back, you must show me you care for all living things.”

Lila knelt by a small, wilting plant and gently poured water over it from her canteen. She whispered, “Grow strong and tall, little plant,” and with that, Green was satisfied and joined her.

Blue was waiting in a peaceful lake, glimmering like a calm sea. “I am the spirit of calm and truth. To bring me back, show me your honesty.”

Lila nodded and told Blue about a time she had been afraid to admit she’d broken a vase at home. “But I told the truth,” she said, “even though it was hard.” Blue’s smile was calm, and it floated beside her, adding its color to the journey.

Finally, she reached the purple color, shimmering like a magical mist. Purple was the spirit of mystery and wisdom, and it looked at her with knowing eyes. “If you want me to come back,” Purple said, “you must tell me what you’ve learned on this journey.”

Lila thought carefully. “I’ve learned that every color is special. Each has its own beauty, and the rainbow wouldn’t be complete without every single one.”

Purple smiled and nodded, joining her.

With all the colors by her side, Lila felt a warmth glowing in her heart. They traveled together, and as they reached the edge of the Whispering Woods, the colors began to swirl and mix, forming a brilliant rainbow in the sky. Lila watched in awe as it stretched from one end of the village to the other, shining brighter than ever.

The villagers cheered and celebrated the return of the rainbow, and Grandma Willow hugged Lila with pride.

From that day on, Lila became known as the Guardian of Colors, and every time a rainbow appeared, people would remember the brave little girl who brought it back.

And in her heart, Lila knew that even when things seemed lost, sometimes all it took was a little bravery, kindness, and belief in oneself to bring back the colors of the world.

The end.)";

    // String text = "We are going to pass pervasive computing.";

    RequestBackendTTS(text);

>>>>>>> f0a1c32a75822842f4dadaa6b18ca5ad1fce4c5d
    // Configure wakeup source
    esp_sleep_enable_ext0_wakeup(static_cast<gpio_num_t>(SWITCH_PIN), 1); // Wake up when switch is turned ON
}

<<<<<<< HEAD

void colorChecking(){
    Serial.print("Red: ");
    if (redPulseCount > 0) {
        Serial.print(1000000 / redPulseCount);
    } else {
        Serial.print("N/A");
=======
void loop()
{
    // Check the state of the switch
    bool currentSwitchState = digitalRead(SWITCH_PIN);
    if (currentSwitchState == LOW)
    {
        Serial.println("System is OFF, entering deep sleep");
        delay(100);             // Small delay before sleep
        esp_deep_sleep_start(); // Enter deep sleep
    }
    else if (currentSwitchState == HIGH && !systemActive)
    {
        systemActive = true;
        Serial.println("System is ON");
>>>>>>> f0a1c32a75822842f4dadaa6b18ca5ad1fce4c5d
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

<<<<<<< HEAD
void GPSturnOn(){
     if (systemActive) {
=======
    // If the system is active, proceed with GPS reading and tone generation
    if (systemActive)
    {
>>>>>>> f0a1c32a75822842f4dadaa6b18ca5ad1fce4c5d
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

        // Play a short 1kHz tone over I2S, with breaks to ensure GPS can process data
    }
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
    if(onQuest){
        GPSturnOn();
        olorChecking();
    }
   

    delay(100); // Small delay to prevent excessive loop iteration
}
