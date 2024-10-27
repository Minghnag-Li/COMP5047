#ifndef GOOGLE_TTS_H
#define GOOGLE_TTS_H

#include <Arduino.h>

extern bool bool_pause;
extern bool requesting;

extern WiFiClient *stream;
extern size_t bytesRead;
extern uint8_t buffer[512];

// void base64UrlEncode(const unsigned char *input, size_t length, String &output);
// void signJWT(const String &message, const char *private_key, String &signature);
// bool SetupNTPTime();
// String createJWT();
// String getAccessToken(const String &jwt);
// String requestGoogleTTS(const String &text, const String &accessToken);
// String GetJWTToken();
bool RequestBackendTTS(String &text);
void EndHTTPTTSResult();
// void SetAudioConfig(int I2S_BCLK, int I2S_LRCLK, int I2S_DIN, int volume = 21);
// void AudioLoop();
void Pause();
void Loop();

#endif  // GOOGLE_TTS_H