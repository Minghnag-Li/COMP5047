#ifndef OPENAI_API_H
#define OPENAI_API_H

#include <Arduino.h>

extern const char *openai_api_key;
extern const char *openai_url;
extern WiFiClientSecure client;

String callOpenAI(const String& prompt);

#endif  // OPENAI_API_H