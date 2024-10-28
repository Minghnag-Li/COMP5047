

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
#include <iostream>
#include <sstream>

TinyGPSPlus gps;

// Define hardware serial port for GPS
HardwareSerial SerialGPS(2);
//timestamps
int player_num_time_stamp = 0;
int quest_time_stamp = 0;
//push button vars
int buttonState = LOW;
int newButtonState = LOW;
int buttonPressedTimeStamp = 0;
int debounceWindow = 40;
int counted = 0;
//bool var for device flow
bool isWaitingForPlayerNum = true;
bool systemActive = false;
bool player_num_button_pressed = false;
bool isWaitingPlayerNumStart = false;
bool isStoryTelling = false;
bool isOnQuest = false;
bool isWaitingForQuestCompletion = false;

//color sensor vars
volatile unsigned long redPulseCount = 0;
volatile unsigned long greenPulseCount = 0;
volatile unsigned long bluePulseCount = 0;

//Wifi vars
const char *ssid = "York148";    // TODO: replace with wifi ssid
const char *password = "King33$$";
WiFiClient *streamClient;
HTTPClient http;
//TTS vars
String textToTranserToTTS;

int player_num = 0; //TODO: player number var here
//AI API vars
bool hasMadeAIAPIrequest = false;

                                
hw_timer_t *timer = NULL;


void IRAM_ATTR onTimer() {

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



void setup() {
    Serial.begin(115200);
    pinMode(S0, OUTPUT);
    pinMode(S1, OUTPUT);
    pinMode(S2, OUTPUT);
    pinMode(S3, OUTPUT);
    pinMode(OUT_PIN, INPUT);
    pinMode(PUSH_PIN, INPUT);
    digitalWrite(PUSH_PIN, HIGH);
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
   
    //SetAudioConfig(I2S_BCLK, I2S_LRCLK, I2S_DIN);


    //bool result = RequestBackendTTS(textToTranserToTTS);
    
    //Serial.println("RequestBackendTTS result: ");
    //Serial.println(result);

    // Configure wakeup source
    esp_sleep_enable_ext0_wakeup(static_cast<gpio_num_t>(SWITCH_PIN), 1); // Wake up when switch is turned ON
}


bool checkColorQuest(const String& color){
    if (color == "red"){
        return true;
    }
    return false;
}

std::string generatePrompt(int player_num) {
    std::ostringstream oss;
    oss << R"(Please generate a 1000 words story for kids from age 5 to 10, this text generated will strictly follow these requirements: 
-Have a futuristic setting 
-Will have )" << player_num << R"( number of main characters interacting with each other
-Will have at least 5 quests embedded into the story line, there are 2 types of quest I want you to add: 
    -Find an object with color {either Red, Green or Blue} or Move a distance of {3m or 6m}.
-Important: There MUST have an asterisk token *(1, 2 or 3 regarding red, green or blue) after every color finding quest and $(1 or 2 regarding 3m or 6m) for moving a distance quest introduction line, DO NOT PUT any token (*, $) before the quest introduction line (like this example: 
One day, Luna and Orion decided to explore the city and see what adventures awaited them. As they walked down the bustling streets, they saw a sign that read: "The Great Robot Race - Winner 
Receives a Golden Key to the City." Excited by the prospect of a challenge, Luna and Orion decided to enter the race.
Find an object with color Blue *(3)  ).
-No need to add "Quest introduce" before introducing a quest and don't put it in curly braces.)";
    return oss.str();
}

//read button clicks and set player number
void setPlayerNum() {
    //read button clicks and set player number
    newButtonState = digitalRead(PUSH_PIN);
    if (newButtonState != buttonState){
        buttonPressedTimeStamp = millis();
        counted = 0;
    }
    if (millis() - buttonPressedTimeStamp >= debounceWindow){
        if(newButtonState == HIGH && counted == 0){
            player_num_button_pressed = true;
            if(player_num < 3){
                player_num ++;
                counted = 1;
            }else{
               player_num = 0;
               counted = 1; 
            }
            
        }
    }
    buttonState = newButtonState;
}
//Timer methods 
void waitTimerForPlayerNumInput(){
    //A Timer for waiting for player number input from user (20s)
    if (!isWaitingPlayerNumStart){
        player_num_time_stamp = millis();
        isWaitingPlayerNumStart = true;
    }
    if (millis() - player_num_time_stamp >= 20000){
        isWaitingForPlayerNum = false;
    }
}
void waitTimerForQuest(){
    //A Timer for waiting for quest to be completed by user (60s)
    if(!isWaitingForQuestCompletion){
        quest_time_stamp = millis();
        isWaitingForQuestCompletion = true;
    }if(millis() - quest_time_stamp >= 60000){
        isWaitingForQuestCompletion = false;
    }
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

void checkSystemTurnOn(){
    //Check if system turning on or off base on the signal read from switch button
    bool currentSwitchState = digitalRead(SWITCH_PIN);
    if (currentSwitchState == LOW) {
        Serial.println("System is OFF, entering deep sleep");
        delay(100);  // Small delay before sleep
        esp_deep_sleep_start();  // Enter deep sleep
    } else if (currentSwitchState == HIGH && !systemActive) {
        systemActive = true;
        Serial.println("System is ON");
    }
}

const int MAX_PARTS = 30; 

int splitStringWithTokens(const String& text, String result[], int maxParts) {
    int currentIndex = 0;
    int start = 0;

    // Loop until no more tokens are found, and ensure we don't exceed maxParts
    while (currentIndex < maxParts - 1) {
        int asteriskPos = text.indexOf('*', start);
        int dollarPos = text.indexOf('$', start);

        // Find the nearest token position
        int nextTokenPos = -1;
        char token = '\0';

        if (asteriskPos != -1 && (dollarPos == -1 || asteriskPos < dollarPos)) {
            nextTokenPos = asteriskPos;
            token = '*';
        } else if (dollarPos != -1) {
            nextTokenPos = dollarPos;
            token = '$';
        }

        // Break if no more tokens are found
        if (nextTokenPos == -1) {
            break;
        }

        // Add the text before the token to the result array, if there's any
        if (nextTokenPos > start && currentIndex < maxParts) {
            result[currentIndex++] = text.substring(start, nextTokenPos);
        }

        // Add the token itself as a separate element
        if (currentIndex < maxParts) {
            result[currentIndex++] = String(token);
        }

        // Check if a number follows in the format (1), (2), or (3)
        if (nextTokenPos + 2 < text.length() && text[nextTokenPos + 1] == '(' &&
            (text[nextTokenPos + 2] == '1' || text[nextTokenPos + 2] == '2' || text[nextTokenPos + 2] == '3') &&
            text[nextTokenPos + 3] == ')') {

            // Add the number part as a separate element
            if (currentIndex < maxParts) {
                result[currentIndex++] = text.substring(nextTokenPos + 1, nextTokenPos + 4); // e.g., "(1)"
            }
            start = nextTokenPos + 4; // Move past the token and the number
        } else {
            start = nextTokenPos + 1; // Move past the token if no number follows
        }
    }

    // Add the last segment after the final token, if there's space
    if (start < text.length() && currentIndex < maxParts) {
        result[currentIndex++] = text.substring(start);
    }

    return currentIndex; // Returns the number of parts in the result array
}


void loop() {
    //timer for 30s if timer done and player hasn't pressed the button, the program end.
    waitTimerForPlayerNumInput();
    //check if the player has pressed the input and set the player num if they have
    setPlayerNum();
    
    if(!player_num_button_pressed && !isWaitingForPlayerNum){
        if(player_num <= 0) {
        Serial.println("System will be off because player didn't set player num");
        esp_deep_sleep_start();
        }
    }else if (!isWaitingForPlayerNum){
        systemActive = true;
        if(!hasMadeAIAPIrequest){
            Serial.println("-----------------Making request to AI");
            //if haven't made a request to AI, make a request to get a story
            //then send the story to tts module
            textToTranserToTTS = callOpenAI(String(generatePrompt(player_num).c_str()));
            Serial.println(textToTranserToTTS);
            Serial.println("-----------------Transfer text to TTS");
            String parts[MAX_PARTS];
            //split the response string with * as delimeter and include * as a separate element
            int numParts = splitStringWithTokens(textToTranserToTTS, parts, MAX_PARTS);
            Serial.println("Split parts:");
            Serial.println("These string will be feed to TTS, needs to be cleaned of tokens");
            for (int i = 0; i < numParts; ++i) {
                if (parts[i] != "*" && 
                    parts[i] != "$" &&
                    parts[i] != "(1)" &&
                    parts[i] != "(2)" &&
                    parts[i] != "(3)"){
                    isOnQuest = false;
                    isStoryTelling = true;
                    //if not isOnQuest and isStoryTelling
                    //transportTextToTTS(parts[i]); 
                    Serial.println(i);
                    Serial.println(parts[i]);
                    //This run inside loop() so it needs something to restrain the frequency of request sending to TTS module
                }else{
                    isOnQuest = true;
                    isStoryTelling = false;
                    if (parts[i] == "*"){
                        //detect color quest
                        if (parts[i+1] == "(1)"){
                            //red
                        }else if(parts[i+1] == "(2)"){
                            //green
                        }else if(parts[i+1] == "(3)"){
                            //blue
                        }
                        //this is when the story telling needs to stop and quest handling will kick in
                        
                    }else if (parts[i] == "$") {
                        //detect movement quest
                        if (parts[i+1] == "(1)"){
                            //3m
                        }else if(parts[i+1] == "(2)"){
                            //6m
                        }
                    }
                }  
            }
            hasMadeAIAPIrequest = true;
        }else{
           
        }
    }
    delay(100);
}

