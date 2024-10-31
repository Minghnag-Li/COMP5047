

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
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


// timestamps
int player_num_time_stamp = 0;
int quest_time_stamp = 0;
// push button vars
int buttonState = LOW;
int newButtonState = LOW;
int buttonPressedTimeStamp = 0;
int debounceWindow = 40;
int counted = 0;
// bool var for device flow
bool isWaitingForPlayerNum = true;
bool systemActive = false;
bool player_num_button_pressed = false;
bool isWaitingPlayerNumStart = false;

/// @brief Between AI generated story and story end.
bool isStoryTelling = false;
/// @brief Detect quest
bool isOnQuest = false;
/// @brief Color sensor
bool isOnColorQuest = false;

bool hasStoryVoiceReturned = false;

const int MAX_PARTS = 50;
String parts[MAX_PARTS];
// split the response string with * as delimeter and include * as a separate element
int numParts = 0;
/// @brief Use pointer to traverse all story parts
int part_pointer = 0;
// accelerometer
int stepCount = 0;
const float g_scale = 3.0 / 4095;
float threshold = 0.22;  // Scale factor for g-force
const float X_baseline = 1900;
const float Y_baseline = 1900;
const float Z_baseline = 2300;
int prev_stepCount = 0;

// Wifi vars
const char *ssid = "YUKARISAW"; // TODO: replace with wifi ssid
const char *password = "88888889";
WiFiClient *streamClient;
HTTPClient http;
// TTS vars
String textToTranserToTTS;

int player_num = 0; // TODO: player number var here
// AI API vars
bool hasMadeAIAPIrequest = false;

uint8_t quest_value = 0;
bool isOnMovingQuest = false;
bool iscompleteColorQuest = false;
bool isCompleteMovingQuest = false;

hw_timer_t *timer = NULL;
int frequency = 0;

/// @brief Color sensor
int redValue = 0;
/// @brief Color sensor
int greenValue = 0;
/// @brief Color sensor
int blueValue = 0;

/// @brief Accelerometer
int x_coor = 0;
// @brief Accelerometer
int y_coor = 0;
// @brief Accelerometer
int z_coor = 0;
 
void Initialise()
{
    player_num_time_stamp = 0;
    quest_time_stamp = 0;
    buttonState = LOW;
    newButtonState = LOW;
    buttonPressedTimeStamp = 0;
    debounceWindow = 40;
    counted = 0;
    isWaitingForPlayerNum = true;
    systemActive = false;
    player_num_button_pressed = false;
    isWaitingPlayerNumStart = false;

    isStoryTelling = false;
    isOnQuest = false;
    isOnColorQuest = false;
    hasStoryVoiceReturned = false;
    numParts = 0;
    part_pointer = 0;

    player_num = 0;
    hasMadeAIAPIrequest = false;

    quest_value = 0;
    isOnMovingQuest = false;
    iscompleteColorQuest = false;

    frequency = 0;
    redValue = 0;
    greenValue = 0;
    blueValue = 0;
    x_coor = 0;
    y_coor = 0;
    z_coor = 0;
}

void IRAM_ATTR onTimer()
{
    // redd
    digitalWrite(S2, LOW);
    digitalWrite(S3, LOW);
    frequency = pulseIn(OUT_PIN, LOW);
    // green
    delay(100);
    digitalWrite(S2, HIGH);
    digitalWrite(S3, HIGH);
    frequency = pulseIn(OUT_PIN, LOW); // 读取脉冲
    // blue

    delay(100);
    digitalWrite(S2, LOW);
    digitalWrite(S3, HIGH);
    frequency = pulseIn(OUT_PIN, LOW);
}

// Function to configure I2S for audio
void setupI2S()
{
    // Configure I2S
    i2s_config_t i2s_config = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX), // Master, Transmit mode
        .sample_rate = 24000,                              // 44.1kHz sample rate
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,      // 16-bit audio
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,       // Stereo format
        .communication_format = I2S_COMM_FORMAT_I2S_MSB,   // I2S communication format
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,          // Default interrupt allocation
        .dma_buf_count = 8,                                // Number of DMA buffers
        .dma_buf_len = 128,                                // DMA buffer length
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

void setup()
{
    Serial.begin(115200);
    //color sensor
    pinMode(S0, OUTPUT);
    pinMode(S1, OUTPUT);
    pinMode(S2, OUTPUT);
    pinMode(S3, OUTPUT);
    pinMode(OUT_PIN, INPUT);
    pinMode(PUSH_PIN, INPUT);
    pinMode(CS_LED_PIN, OUTPUT);
    pinMode(SWITCH_PIN, INPUT);

    // turn on CS LED

    digitalWrite(CS_LED_PIN, HIGH);

    digitalWrite(PUSH_PIN, HIGH);
    digitalWrite(S0, HIGH);
    digitalWrite(S1, LOW);
   // Accelerometer
    Serial.begin(115200);
    pinMode(X, OUTPUT);
    pinMode(Y, OUTPUT);
    pinMode(Z,OUTPUT);
   

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

    // Configure wakeup source
    esp_sleep_enable_ext0_wakeup(static_cast<gpio_num_t>(SWITCH_PIN), 1); // Wake up when switch is turned ON
}

std::string generatePrompt(int player_num)
{
    std::ostringstream oss;
    oss << R"(Please generate a 1000 words story for kids from age 5 to 10, this text generated will strictly follow these requirements: 
-Have a futuristic setting 
-Will have )"
        << player_num << R"( number of main characters interacting with each other
-Will have at least 5 quests embedded into the story line, there are 2 types of quest I want you to add: 
    -Find an object with color {either Red, Green or Blue} or Move a distance of {3m or 6m}.
-Important: There MUST have an asterisk token *(1, 2 or 3 regarding red, green or blue) after every color finding quest and $(1 or 2 regarding 3m or 6m) for moving a distance quest introduction line, DO NOT PUT any token (*, $) before the quest introduction line (like this example: 
One day, Luna and Orion decided to explore the city and see what adventures awaited them. As they walked down the bustling streets, they saw a sign that read: "The Great Robot Race - Winner 
Receives a Golden Key to the City." Excited by the prospect of a challenge, Luna and Orion decided to enter the race.
Find an object with color Blue *(3)  ).
-No need to add "Quest introduce" before introducing a quest and don't put it in curly braces.)";
    return oss.str();
}

// read button clicks and set player number
void setPlayerNum()
{
    // read button clicks and set player number
    newButtonState = digitalRead(PUSH_PIN);
    if (newButtonState != buttonState)
    {
        buttonPressedTimeStamp = millis();
        counted = 0;
    }
    if (millis() - buttonPressedTimeStamp >= debounceWindow)
    {
        if (newButtonState == HIGH && counted == 0)
        {
            player_num_button_pressed = true;
            if (player_num < 3)
            {
                player_num++;
                counted = 1;
            }
            else
            {
                player_num = 0;
                counted = 1;
            }

            Serial.print("player_num: ");
            Serial.print(player_num);
            switch (player_num)
            {
            case 1:
                RequestBackendPremadeTTS(PREMADE_TTS_PLAYER_NUMBER_1);
                break;
            case 2:
                RequestBackendPremadeTTS(PREMADE_TTS_PLAYER_NUMBER_2);
                break;
            case 3:
                RequestBackendPremadeTTS(PREMADE_TTS_PLAYER_NUMBER_3);
                break;
            case 0:
                RequestBackendPremadeTTS(PREMADE_TTS_WAITING_FOR_PLAYER);
            default:
                break;
            }
        }
    }
    buttonState = newButtonState;
}
// Timer methods
void waitTimerForPlayerNumInput()
{
    // A Timer for waiting for player number input from user (20s)
    if (!isWaitingPlayerNumStart)
    {
        player_num_time_stamp = millis();
        isWaitingPlayerNumStart = true;
    }
    if (millis() - player_num_time_stamp >= 20000)
    {
        isWaitingForPlayerNum = false;
    }
}
void waitTimerForQuest()
{
    if (millis() - quest_time_stamp >= 30000)
    {
        Serial.println("Skipping Quest");
        RequestBackendPremadeTTS(PREMADE_TTS_QUEST_SKIPPED);
        isOnQuest = false;
    }
}
//color detection logic
bool colorChecking(int random_color)
{

    digitalWrite(S2, LOW);
    digitalWrite(S3, LOW);
    frequency = pulseIn(OUT_PIN, LOW);
    redValue = frequency;


    digitalWrite(S2, HIGH);
    digitalWrite(S3, HIGH);
    frequency = pulseIn(OUT_PIN, LOW);
    greenValue = frequency;


    digitalWrite(S2, LOW);
    digitalWrite(S3, HIGH);
    frequency = pulseIn(OUT_PIN, LOW);
    blueValue = frequency;


    // check red
    if (random_color == 1)
    {
        if (redValue < 70 && redValue > 40 &&
            greenValue < 190 && greenValue > 150 &&
            blueValue > 125 && blueValue < 250)
        {
            return true;
        }
        return false;
        // check green
    }
    else if (random_color == 2)
    {
        if (redValue < 150 && redValue > 110 &&
            greenValue < 90 && greenValue > 50 &&
            blueValue > 90 && blueValue < 150)
        {
            return true;
        }
        return false;
        // check blue
    }
    else if (random_color == 3)
    {
        if (redValue < 170 && redValue > 120 &&
            greenValue < 140 && greenValue > 80 &&
            blueValue > 30 && blueValue < 100)
        {
            return true;
        }
        return false;
    }
    // delay(100);
    return false;
}


//Accelerometer
bool movingChecking(int distance){
    x_coor = analogRead(X);
    y_coor = analogRead(Y);
    z_coor = analogRead(Z);
    
    float xAccel = (x_coor - X_baseline) * g_scale;
    float yAccel = (y_coor - Y_baseline) * g_scale;
    float zAccel = (z_coor - Z_baseline) * g_scale;

  // Calculate magnitude
    float magnitude = sqrt(xAccel * xAccel + yAccel * yAccel + zAccel * zAccel);
    //check if a step was made
    if (magnitude > threshold) {
        stepCount++;
        delay(100);
    }

    
    Serial.print(stepCount);
    Serial.print(" ");
    Serial.print(prev_stepCount);
    Serial.println(" ");
    if (distance == 1){
        //3m
        if(stepCount - prev_stepCount >= 6){
            return true;
        }
        return false;
    }else if (distance == 2){
        //6m
        if(stepCount - prev_stepCount >= 12){
            return true;
        }
        return false;
    }

}


int splitStringWithTokens(const String &text, String result[], int maxParts)
{
    int currentIndex = 0;
    int start = 0;

    // Loop until no more tokens are found, and ensure we don't exceed maxParts
    while (currentIndex < maxParts - 1)
    {
        int asteriskPos = text.indexOf('*', start);
        int dollarPos = text.indexOf('$', start);

        // Find the nearest token position
        int nextTokenPos = -1;
        char token = '\0';

        if (asteriskPos != -1 && (dollarPos == -1 || asteriskPos < dollarPos))
        {
            nextTokenPos = asteriskPos;
            token = '*';
        }
        else if (dollarPos != -1)
        {
            nextTokenPos = dollarPos;
            token = '$';
        }

        // Break if no more tokens are found
        if (nextTokenPos == -1)
        {
            break;
        }

        // Add the text before the token to the result array, if there's any
        if (nextTokenPos > start && currentIndex < maxParts)
        {
            result[currentIndex++] = text.substring(start, nextTokenPos);
        }

        // Add the token itself as a separate element
        if (currentIndex < maxParts)
        {
            result[currentIndex++] = String(token);
        }

        // Check if a number follows in the format (1), (2), or (3)
        if (nextTokenPos + 2 < text.length() && text[nextTokenPos + 1] == '(' &&
            (text[nextTokenPos + 2] == '1' || text[nextTokenPos + 2] == '2' || text[nextTokenPos + 2] == '3') &&
            text[nextTokenPos + 3] == ')')
        {

            // Add the number part as a separate element
            if (currentIndex < maxParts)
            {
                result[currentIndex++] = text.substring(nextTokenPos + 1, nextTokenPos + 4); // e.g., "(1)"
            }
            start = nextTokenPos + 4; // Move past the token and the number
        }
        else
        {
            start = nextTokenPos + 1; // Move past the token if no number follows
        }
    }

    // Add the last segment after the final token, if there's space
    if (start < text.length() && currentIndex < maxParts)
    {
        result[currentIndex++] = text.substring(start);
    }

    return currentIndex; // Returns the number of parts in the result array
}

void loop()
{
    
    bool currentSwitchState = digitalRead(SWITCH_PIN);
    if (currentSwitchState == LOW)
    {
        Serial.println("System is OFF, entering deep sleep");
        RequestBackendPremadeTTS(PREMADE_TTS_POWER_OFF);
        delay(100);             // Small delay before sleep
        esp_deep_sleep_start(); // Enter deep sleep
    }
    else if (currentSwitchState == HIGH)
    {
        if (!systemActive)
        {
            Initialise();
            systemActive = true;
            Serial.println("System is ON");
            RequestBackendPremadeTTS(PREMADE_TTS_POWER_ON);
        }

        if (isOnQuest)
        {
            waitTimerForQuest();
            if (isOnColorQuest)
            {
                if (quest_value == 1)
                {
                    iscompleteColorQuest = colorChecking(1);
                }
                else if (quest_value == 2)
                {
                    iscompleteColorQuest = colorChecking(2);
                }
                else if (quest_value == 3)
                {
                    iscompleteColorQuest = colorChecking(3);
                }

                if (iscompleteColorQuest)
                {
                    isOnColorQuest = false;
                    Serial.print("Color check is done: ");
                    Serial.println(quest_value);
                    RequestBackendPremadeTTS(PREMADE_TTS_QUEST_DONE);
                }
            }

            if (isOnMovingQuest)
            {
                // checking logic for moving quest done here
                if(quest_value == 1){
                    isCompleteMovingQuest = movingChecking(1);
                }
                else if(quest_value == 2){
                    isCompleteMovingQuest = movingChecking(2);
                }

                if (isCompleteMovingQuest)
                {
                    isOnMovingQuest = false;
                    prev_stepCount = stepCount;
                    RequestBackendPremadeTTS(PREMADE_TTS_QUEST_DONE);
                }
            }

            if (!isOnColorQuest && !isOnMovingQuest)
            {
                isOnQuest = false;
            }
        }
        else if (isStoryTelling)
        {
            if (part_pointer < numParts)
            {
                if (parts[part_pointer] != "*" &&
                    parts[part_pointer] != "$" &&
                    parts[part_pointer] != "(1)" &&
                    parts[part_pointer] != "(2)" &&
                    parts[part_pointer] != "(3)")
                {
                    isOnQuest = false;
                    isStoryTelling = true;
                    // if not isOnQuest and isStoryTelling
                    Serial.print("--------------------- index of element sent ");
                    Serial.print(part_pointer);
                    Serial.println(parts[part_pointer]);
                    RequestBackendTTS(parts[part_pointer]);
                    hasStoryVoiceReturned = true;
                }
                else if (part_pointer + 1 < numParts)
                {
                    Serial.println("Quest detected");
                    Serial.print(parts[part_pointer]);
                    Serial.print(parts[part_pointer + 1]);
                    hasStoryVoiceReturned = false;
                    isOnQuest = true;
                    RequestBackendPremadeTTS(PREMADE_TTS_QUEST_WAITING);
                    quest_time_stamp = millis();
                    if (parts[part_pointer] == "*")
                    {
                        isOnColorQuest = true;
                        // detect color quest
                        if (parts[part_pointer + 1] == "(1)")
                        {
                            quest_value = 1;
                            // red
                            Serial.println("Quest: Find red object");
                        }
                        else if (parts[part_pointer + 1] == "(2)")
                        {
                            quest_value = 2;
                            // green
                            Serial.println("Quest: Find green object");
                        }
                        else if (parts[part_pointer + 1] == "(3)")
                        {
                            quest_value = 3;
                            // blue
                            Serial.println("Quest: Find blue object");
                        }
                        // this is when the story telling needs to stop and quest handling will kick in
                    }
                    else if (parts[part_pointer] == "$")
                    {
                        isOnMovingQuest = true;
                        // detect movement quest
                        if (parts[part_pointer + 1] == "(1)")
                        {
                            quest_value = 1;
                            // 3m
                            Serial.println("Quest: Move 3m");
                        }
                        else if (parts[part_pointer + 1] == "(2)")
                        {
                            quest_value = 2;
                            // 6m
                            Serial.println("Quest: Move 6m");
                        }
                    }
                    else
                    {
                        Serial.println("None quest type!");
                    }
                    part_pointer++;
                }
                else
                {
                    Serial.println("Error: Invalid Quest due to the end of parts.");
                }
                part_pointer++;
            }
            else
            {
                RequestBackendPremadeTTS(PREMADE_TTS_STORY_END);
                delay(100);
                isStoryTelling = false;
                systemActive = false;
                RequestBackendPremadeTTS(PREMADE_TTS_POWER_OFF);
                delay(100);             // Small delay before sleep
                esp_deep_sleep_start(); // Enter deep sleep
            }
        }
        else
        {
            // timer for 30s if timer done and player hasn't pressed the button, the program end.
            waitTimerForPlayerNumInput();
            // check if the player has pressed the input and set the player num if they have
            setPlayerNum();
            if (!player_num_button_pressed && !isWaitingForPlayerNum)
            {
                if (player_num <= 0)
                {
                    Serial.println("System will be off because player didn't set player num");
                    esp_deep_sleep_start();
                }
            }
            else if (!isWaitingForPlayerNum)
            {
                if (!hasMadeAIAPIrequest)
                {
                    Serial.println("-----------------Making request to AI");
                    // if haven't made a request to AI, make a request to get a story
                    // then send the story to tts module
                    textToTranserToTTS = callOpenAI(String(generatePrompt(player_num).c_str()));
                    Serial.println(textToTranserToTTS);
                    Serial.println("-----------------Transfer text to TTS");
                    Serial.println("Split parts:");
                    // split the response string with * as delimeter and include * as a separate element
                    numParts = splitStringWithTokens(textToTranserToTTS, parts, MAX_PARTS);
                    // start traverse
                    part_pointer = 0;
                    RequestBackendPremadeTTS(PREMADE_TTS_STORY_START);
                    isStoryTelling = true;
                    hasMadeAIAPIrequest = true;
                }
            }
        }

        delay(100);
    }
}