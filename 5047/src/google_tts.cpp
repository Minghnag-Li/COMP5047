#include <HTTPClient.h>
#pragma region Process_by_ESP32
// #include <ArduinoJson.h>
// #include <mbedtls/base64.h>
// #include <mbedtls/md.h>
// #include <mbedtls/pk.h>
// #include <mbedtls/ctr_drbg.h>
// #include <mbedtls/entropy.h>
// #include "time.h"
// #include <WiFiClientSecure.h>
#pragma endregion
#include <google_tts.h>

#include <driver/i2s.h>

// bool bool_pause = false;
// bool requesting = false;
WiFiClient *stream;
size_t bytesRead;
uint8_t buffer[512];
SemaphoreHandle_t ttsMutex = xSemaphoreCreateMutex();

// HTTPClient http_tts_result;

#pragma region Process_by_ESP32
// // The private key of Google service account
// const char *private_key = "-----BEGIN PRIVATE KEY-----\nMIIEvwIBADANBgkqhkiG9w0BAQEFAASCBKkwggSlAgEAAoIBAQDkgs6YdHoBR+Sy\nT/vNM4hEIr7rTtfwArBDc/O/hnHDPVFCf3YhkkDLKkhnqc4Ihpqpvfimk1NzDlrX\nVa5OeSNtRbNTfOOmxZUSE+VdT5C0aHdwU1B/jVB/QNM2GGCcRbEHWYW9CC1tWY/1\nglHR7eeczOj5F/1e4aqB9YOTr2f++326HdMW71Z0g2KQNS3MoBxYFFbeVpeLmxVZ\nVdG24HP+k0uGS3h0/95kwvvoCzk1dLbb+jMwIeIQxImfhcHup68QGS8aZjB9cJZK\ntKLBlkrW4VQ3eAKMbPNcys3YpYI4rpqleN0CRcqQ5NL1thKCK4oxK6qapDOvFRRv\nIcCygpUZAgMBAAECggEAAvuLvr8jvVHKUbDcGxyCkDD3RF+cBkGMJ/apt3gzgGUm\nJ5KZJ5PFrGkxP/60/QKtwyfQmxeRp4FRQ7AWMx4M1rzOgJlFhA6MDAv7080E+RfZ\n560DknwQldKf8o4vFDTPl8ZgYKDjUjCw2Hc+C0MezyPdDmO0wMszAfQ1Qe50+6Ep\n3ExgvzJCUKRq5uMNCMvDS3Bt/Xy51YhAIqsNIPml5BE0TtwGex4y0+uWWzFrXyN/\n/E1QIev+XUr/8UKBAq1T8m3ZAM7nYWIJg8rTp4Yn3CS858mYoZniAGqNQRCNsfoG\n7yV/jnhA98wMAYNhecYzBhv6MRS5+9/FYynZ8EXBYQKBgQD7gRfyHoTrYf48VSaV\nJnFfEer+8R/q7rG7izgkSLIp0TENISLwykMtPMJW0U28ZTR577r2fkuTsR2aPq9b\nOA0ykW9biGer7QOzZnjtYTUFTvZ94qS59f6oqRgkhKINDTvkOShPRvYyLI55R9hL\nISvTisQLljOV3Zu5D2HDWmvCeQKBgQDomH53X/N/SWNgkoMH9bdCmmijnnu3FGRG\nQ7zkT+M5MCD95irezulHP4KRoIlMfDGvprMxOw7fpfZ1y+AKlGgYxWrCXRaiBUyq\nQIPZT/SbTjmQI6XrjoMPeAv238Wk/Z1KDQnOWLLvRFklPZaDRmWYfdeqwcp9oI1o\n97wyYo2/oQKBgQD2BTXTVh9X0afMhM8VH2HPScvCT0SVn6CkgG8Orz34wfVljoCL\nYo+L9N915ZBRNf67ONJ+xNRRSPRYKfGHHbFrQfDYbPpcp6DzIM4yBuDCrJdaVRYm\nAoh1m9NOBaAvfD6V5YOU6Slzszo7URg3hd9FM3zKiMc1Vm45/N35Jr46oQKBgQCS\nkO04KeVVMxiFFS2uSV62gHgpikdpAVeCk1KnB9QmCDbx5yTii72h4NpVC7UY1jW0\n+rV180IXGgxpBBRkdkn2Oghg+6m0ZOR/b/HlcSWw66UIN/5opb/ecoYPhciUhB+m\nNvsT5P60GPyXo+yBQk3ovnu1JTKE4TPajikwErxBoQKBgQD13ufd3lUZSQBkUe1N\nghxdOjcnHHx2odouxU2Hnr3Rij8JitjK7D6U+9hKHnYXVqoXStSo+5A0LWS1MLM6\n8ZupFxnhRKP//Ar1RlL+gLLvaU32ar2nFk9N/NogMi4SLT2y+aQDYAQ+NN6xj9dc\ntaXrQrY0F/SIR0MvEmyZGdfXBQ==\n-----END PRIVATE KEY-----\n";

// const char *ntpServer = "pool.ntp.org";
// const long gmtOffset_sec = 0;
// const int daylightOffset_sec = 0;

// // Google service account
// const char *client_email = "comp5047-tts@arched-cabinet-439420-m1.iam.gserviceaccount.com";
// const char *token_uri = "https://oauth2.googleapis.com/token";

// const char *tts_endpoint = "https://texttospeech.googleapis.com/v1/text:synthesize";

// void base64UrlEncode(const unsigned char *input, size_t length, String &output)
// {
//     size_t output_length = 0;
//     unsigned char encoded[512];

//     // use mbedtls_base64_encode to Base64 encode url
//     int ret = mbedtls_base64_encode(encoded, sizeof(encoded), &output_length, input, length);

//     if (ret == 0)
//     {
//         output = "";
//         for (size_t i = 0; i < output_length; i++)
//         {
//             output += static_cast<char>(encoded[i]);
//         }

//         // Replace incompatible characters
//         output.replace("+", "-");
//         output.replace("/", "_");
//         output.replace("=", "");
//     }
//     else
//     {
//         Serial.println("Base64 encode failed.");
//         output = "";
//     }
// }

// void signJWT(const String &message, const char *private_key, String &signature)
// {
//     mbedtls_pk_context pk_context;
//     mbedtls_pk_init(&pk_context);

//     mbedtls_entropy_context entropy;
//     mbedtls_ctr_drbg_context ctr_drbg;
//     mbedtls_entropy_init(&entropy);
//     mbedtls_ctr_drbg_init(&ctr_drbg);
//     mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, nullptr, 0);

//     // Parse private key
//     if (mbedtls_pk_parse_key(&pk_context, (const unsigned char *)private_key, strlen(private_key) + 1, NULL, 0) != 0)
//     {
//         Serial.println("Private key parsing failed");
//         signature = "";
//         return;
//     }

//     unsigned char hash[32];
//     mbedtls_md(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), (const unsigned char *)message.c_str(), message.length(), hash);

//     unsigned char sig[512];
//     size_t sig_len;

//     // use private key to sign hashed JWT
//     if (mbedtls_pk_sign(&pk_context, MBEDTLS_MD_SHA256, hash, 0, sig, &sig_len, mbedtls_ctr_drbg_random, &ctr_drbg) != 0)
//     {
//         Serial.println("Sign failed");
//         signature = "";
//     }
//     else
//     {
//         String base64Sig;
//         base64UrlEncode(sig, sig_len, base64Sig);
//         signature = base64Sig;
//     }

//     mbedtls_pk_free(&pk_context);
//     mbedtls_ctr_drbg_free(&ctr_drbg);
//     mbedtls_entropy_free(&entropy);
// }

// String createJWT()
// {
//     struct tm timeinfo;
//     if (!getLocalTime(&timeinfo))
//     {
//         Serial.println("Cannot get local time.");
//         return "";
//     }

//     // Get timestamp
//     time_t now = mktime(&timeinfo);
//     long iat = now;
//     long exp = now + 3600; // Set expiration time to 1 hour later

//     // JWT Header
//     String header = "{\"alg\":\"RS256\",\"typ\":\"JWT\"}";

//     // JWT Payload，use iat and exp
//     String payload = "{\"iss\":\"" + String(client_email) + "\",\"scope\":\"https://www.googleapis.com/auth/cloud-platform\",\"aud\":\"" + String(token_uri) + "\",\"exp\":" + String(exp) + ",\"iat\":" + String(iat) + "}";

//     // encode Header and Payload
//     String encodedHeader, encodedPayload;
//     base64UrlEncode((const unsigned char *)header.c_str(), header.length(), encodedHeader);
//     base64UrlEncode((const unsigned char *)payload.c_str(), payload.length(), encodedPayload);

//     // Create signature
//     String signatureBase = encodedHeader + "." + encodedPayload;
//     String signature;
//     signJWT(signatureBase, private_key, signature);

//     // return JWT
//     return signatureBase + "." + signature;
// }

// String getAccessToken(const String &jwt)
// {
//     HTTPClient http;
//     http.begin(token_uri);

//     http.addHeader("Content-Type", "application/x-www-form-urlencoded");

//     // use generated JWT as assertion parameter
//     String requestBody = "grant_type=urn%3Aietf%3Aparams%3Aoauth%3Agrant-type%3Ajwt-bearer&assertion=" + jwt;

//     int httpResponseCode = http.POST(requestBody);
//     String accessToken = "";

//     if (httpResponseCode == 200)
//     {
//         String response = http.getString();
//         JsonDocument jsonDoc;
//         deserializeJson(jsonDoc, response);
//         accessToken = jsonDoc["access_token"].as<String>();
//     }
//     else
//     {
//         Serial.printf("Get access token failed, HTTP response code: %d\n", httpResponseCode);
//         Serial.println("response: " + http.getString());
//     }
//     http.end();
//     return accessToken;
// }

// String requestGoogleTTS(const String &text, const String &accessToken)
// {
//     WiFiClientSecure client;
//     client.setInsecure(); // 仅用于调试，正式环境下建议使用证书
//     if (!client.connect("texttospeech.googleapis.com", 443))
//     {
//         Serial.println("connect to Google TTS server failed.");
//         return "";
//     }

//     // request body
//     String requestBody = R"(
//         {
//             "input": {
//                 "text": ")" +
//                          text + R"("
//             },
//             "voice": {
//                 "languageCode": "en-US",
//                 "name": "en-US-Studio-O"
//             },
//             "audioConfig": {
//                 "audioEncoding": "MP3"
//             }
//         }
//     )";

//     // generate HTTP request
//     String request = String("POST ") + "/v1/text:synthesize HTTP/1.1\r\n" +
//                      "Host: texttospeech.googleapis.com\r\n" +
//                      "Authorization: Bearer " + accessToken + "\r\n" +
//                      "Content-Type: application/json\r\n" +
//                      "Content-Length: " + String(requestBody.length()) + "\r\n\r\n" +
//                      requestBody;

//     // send request
//     client.print(request);

//     String line;
//     while (client.connected())
//     {
//         line = client.readStringUntil('\n');
//         // Serial.println("header: " + line);
//         if (line == "\r")
//         {
//             break;
//         }
//     }

//     while (client.connected())
//     {
//         while (client.available())
//         {
//             String body = client.readStringUntil('\n');
//             String audioContentStart = "  \"audioContent\": \"";
//             if (body.startsWith(audioContentStart))
//             {
//                 Serial.println("Found audioContent");
//                 client.stop();
//                 return body.substring(audioContentStart.length(), body.length() - 1);
//             }
//         }
//     }
//     client.stop();
//     return "";
// }

// bool SetupNTPTime()
// {
//     // Set NTP to sync time
//     configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
//     struct tm timeinfo;
//     if (!getLocalTime(&timeinfo))
//     {
//         Serial.println("Cannot get NTP time");
//         return false;
//     }
//     Serial.println("NTP time sync succeed");
//     return true;
// }

// String GetJWTToken()
// {
//     String jwt = createJWT();
//     return getAccessToken(jwt);
// }

// const char base64_chars[] PROGMEM = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// // Base64 解码函数
// int base64_decode(uint8_t *output, const char *input, int inputLen)
// {
//     int outputLen = 0;
//     uint8_t dtable[256] = {0};
//     for (int i = 0; i < 64; i++)
//     {
//         dtable[(uint8_t)pgm_read_byte(&base64_chars[i])] = i;
//     }
//     dtable['='] = 0;

//     for (int i = 0; i < inputLen;)
//     {
//         uint8_t a = dtable[(uint8_t)input[i++]];
//         uint8_t b = dtable[(uint8_t)input[i++]];
//         uint8_t c = dtable[(uint8_t)input[i++]];
//         uint8_t d = dtable[(uint8_t)input[i++]];

//         output[outputLen++] = (a << 2) | (b >> 4);
//         if (input[i - 2] != '=')
//             output[outputLen++] = (b << 4) | (c >> 2);
//         if (input[i - 1] != '=')
//             output[outputLen++] = (c << 6) | d;
//     }
//     return outputLen;
// }

// void TTS()
// {
//     if (SetupNTPTime())
//     {
//         String accessToken = GetJWTToken();
//         if (!accessToken.isEmpty())
//         {
//             Serial.println("Got access token. "); // + accessToken);
//             String audioContent = requestGoogleTTS("In the year 3020, the world had changed drastically. Technology had advanced beyond anyone's wildest dreams, and people lived in a futuristic society where robots were a common sight and flying cars zipped through the sky.\n\nOne day, a young girl named Lily was walking home from school when she saw a strange object lying on the ground. It was a small, shiny device that looked like a miniature spaceship. Curious, Lily picked it up and examined it closely. Suddenly, the device lit", accessToken);
//             if (!audioContent.isEmpty())
//             {
//                 Serial.println("audioContent: ");
//                 Serial.println(audioContent);
//                 // 使用自定义 Base64 解码
//                 // int decodedLength = (audioContent.length() * 3) / 4;
//                 // uint8_t *mp3Data = (uint8_t *)malloc(decodedLength);
//                 // int actualLength = base64_decode(mp3Data, audioContent.c_str(), audioContent.length());
//                 // Serial.println("actualLength");
//                 // Serial.println(actualLength);
//             }
//             else
//             {
//                 Serial.println("Empty result from requestGoogleTTS");
//             }
//         }
//         else
//         {
//             Serial.println("Cannot get access token");
//         }
//     }
//     else
//     {
//         Serial.println("Set NTP time failed.");
//     }
// }

// void SetAudioConfig(int I2S_BCLK, int I2S_LRCLK, int I2S_DIN, int volume)
// {
//     audio.setPinout(I2S_BCLK, I2S_LRCLK, I2S_DIN);
//     audio.setVolume(volume);
//     Serial.println("SetAudio");
// }
#pragma endregion

String urlEncode(String str)
{
    String encodedString = "";
    char c;
    for (size_t i = 0; i < str.length(); i++)
    {
        c = str[i];
        if (isalnum(c))
        {
            encodedString += c;
        }
        else
        {
            encodedString += '%';
            if (c < 16)
                encodedString += '0';
            encodedString += String((unsigned char)c, HEX);
        }
    }
    return encodedString;
}

// void TTS_Pause()
// {
//     bool_pause = !bool_pause;
// }

// void TTS_Loop()
// {
//     if (stream != nullptr and !bool_pause and !requesting)
//     {
//         if (stream->connected() && (bytesRead = stream->readBytes(buffer, sizeof(buffer))) > 0)
//         {
//             size_t bytes_written;
//             i2s_write(I2S_NUM_0, buffer, bytesRead, &bytes_written, portMAX_DELAY);
//         }
//     }
// }

// void EndHTTPTTSResult()
// {
//     http_tts_result.end();
// }

bool RequestBackendTTS(String &text)
{
    if (xSemaphoreTake(ttsMutex, portMAX_DELAY) == pdTRUE) // Wait indefinitely to acquire the mutex
    {
        bool result = false;

        HTTPClient http;
        http.setConnectTimeout(60000);
        http.setTimeout(60000);
        http.begin("http://192.168.137.1:8000/tts");
        http.addHeader("text", urlEncode(text));
        int httpCode = http.GET();

        if (httpCode == HTTP_CODE_OK)
        {
            HTTPClient http_tts_result;
            http_tts_result.setConnectTimeout(60000);
            http_tts_result.setTimeout(60000);
            http_tts_result.begin("http://192.168.137.1:8000/tts/result");

            int httpCode = http_tts_result.GET();

            if (httpCode == HTTP_CODE_OK)
            {
                stream = http_tts_result.getStreamPtr();
                while (stream->connected())
                {

                    int remaining_data = stream->available();
                    if (remaining_data > sizeof(buffer))
                    {
                        bytesRead = stream->readBytes(buffer, sizeof(buffer));
                    }
                    else if (remaining_data > 0)
                    {
                        bytesRead = stream->readBytes(buffer, remaining_data);
                    }

                    if (remaining_data <= 0) break;

                    for (size_t i = 0; i < bytesRead / sizeof(int16_t); i++)
                    {
                        int16_t *sample = ((int16_t *)buffer) + i;
                        *sample = (int16_t)(*sample * 0.02);
                    }

                    size_t bytes_written;
                    i2s_write(I2S_NUM_0, buffer, bytesRead, &bytes_written, portMAX_DELAY);
                }

                Serial.println("------------------Finish this story text");
            }
            else
            {
                Serial.printf("Failed to connect, HTTP code: %d\n", httpCode);
            }
            http_tts_result.end();
        }
        else
        {
            Serial.printf("Request tts api failed: %d\n", httpCode);
        }
        http.end();

        // Release the mutex to allow the next request
        xSemaphoreGive(ttsMutex);
        return result;
    }
    else
    {
        Serial.println("Failed to acquire TTS mutex");
        return false;
    }
}

bool RequestBackendPremadeTTS_(String &url)
{
    if (xSemaphoreTake(ttsMutex, portMAX_DELAY) == pdTRUE) // Wait indefinitely to acquire the mutex
    {
        bool result = false;

        HTTPClient http_tts_result;
        http_tts_result.setConnectTimeout(60000);
        http_tts_result.setTimeout(60000);
        http_tts_result.begin(url);

        int httpCode = http_tts_result.GET();

        if (httpCode == HTTP_CODE_OK)
        {
            stream = http_tts_result.getStreamPtr();
            while (stream->connected())
            {
                int remaining_data = stream->available();
                if (remaining_data > sizeof(buffer))
                {
                    bytesRead = stream->readBytes(buffer, sizeof(buffer));
                }
                else if (remaining_data > 0)
                {
                    bytesRead = stream->readBytes(buffer, remaining_data);
                }
                
                if (remaining_data <= 0) break;

                for (size_t i = 0; i < bytesRead / sizeof(int16_t); i++)
                {
                    int16_t *sample = ((int16_t *)buffer) + i;
                    *sample = (int16_t)(*sample * 0.02); // Scale sample to 20% volume
                }

                size_t bytes_written;
                i2s_write(I2S_NUM_0, buffer, bytesRead, &bytes_written, portMAX_DELAY);
            }
        }
        else
        {
            Serial.printf("Failed to connect premade TTS, HTTP code: %d\n", httpCode);
        }

        http_tts_result.end();

        // Release the mutex to allow the next request
        xSemaphoreGive(ttsMutex);
        return result;
    }
    else
    {
        Serial.println("Failed to acquire TTS mutex");
        return false;
    }
}

bool RequestBackendPremadeTTS(uint8_t premade_tts_code)
{
    String url = "http://192.168.137.1:8000/tts/premade/";
    switch (premade_tts_code)
    {
    case PREMADE_TTS_STORY_START:
        url += "story_start";
        break;
    case PREMADE_TTS_QUEST_WAITING:
        url += "quest_waiting";
        break;
    case PREMADE_TTS_QUEST_DONE:
        url += "quest_done";
        break;
    case PREMADE_TTS_STORY_END:
        url += "story_end";
        break;
    case PREMADE_TTS_QUEST_SKIPPED:
        url += "quest_skipped";
        break;
    default:
        url += "undefined";
        break;
    }
    return RequestBackendPremadeTTS_(url);
}

// void AudioLoop()
// {
//     audio.loop();
//     vTaskDelay(1);
// }