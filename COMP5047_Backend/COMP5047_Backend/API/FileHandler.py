"""Synthesizes speech from the input string of text."""
import threading
import urllib.parse
import wave

import numpy as np
from django.db import connection
from django.http import FileResponse, JsonResponse
from google.cloud import texttospeech

from COMP5047_Backend.Tools.Logger import TLogger

SUCCESS = 200
FALSE = 204
UNAUTHORIZED = 401
CLIENT_INVALID_PARAMETER = 400
NOT_FOUND = 404
METHOD_NOT_ALLOWED = 405
INTERNAL_SERVER_ERROR = 500

GOOGLE_TTS_CONNECTION_POOL_MAX = 10


def get_result(error_code=SUCCESS, data=None, error_msg=None):
    return {'error_code': error_code, 'data': data, 'error_msg': error_msg}


def get_json_response(result):
    return JsonResponse(result, status=result['error_code'])


def get_json_response_result(error_code=SUCCESS, data=None, error_msg=None):
    return JsonResponse(get_result(error_code=error_code, data=data, error_msg=error_msg), status=error_code)


client = texttospeech.TextToSpeechClient()

# Note: the voice can also be specified by name.
# Names of voices can be retrieved with client.list_voices().
voice = texttospeech.VoiceSelectionParams(
    language_code="en-US",
    name="en-US-Studio-O",
)

audio_config = texttospeech.AudioConfig(
    audio_encoding=texttospeech.AudioEncoding.LINEAR16,
    speaking_rate=1
)


def request_once(text, file_index):
    input_text = texttospeech.SynthesisInput(text=text)

    response = client.synthesize_speech(
        request={"input": input_text, "voice": voice, "audio_config": audio_config}
    )

    with open(f"output_{file_index}.wav", "wb") as out:
        out.write(response.audio_content)
        TLogger.tlog(f"Audio content written to file output_{file_index}.wav")

def split_text(text):
    long_sentence = False

    def split_text_by_space(t2):
        long_word = False
        for t3 in t2.split(" "):
            if len(t3) >= 200:
                long_word = True
        if long_word:
            TLogger.twarn('Long word detected')
        tt3s = []
        tt3 = ""
        for t3 in t2.split(" "):
            if len(t3) >= 200:
                tt3s.append(t3)
            elif len(tt3 + t3) < 200:
                tt3 += f"{t3} "
            else:
                tt3s.append(tt3)
                tt3 = ""
        return tt3s


    def split_text_by_comma(t1):
        long_clause = False
        for t2 in t1.split(','):
            if len(t2) >= 200:
                long_clause = True
        if long_clause:
            return split_text_by_space(t1)
        tt2s = []
        tt2 = ""
        for t2 in t1.split(','):
            if len(tt2 + t2) < 200:
                tt2 += f"{t2},"
            else:
                tt2s.append(tt2)
                tt2 = ""
        return tt2s

    tt1s = []
    tt1 = ""
    for t1 in text.split("."):
        if len(t1) > 200:
            long_sentence = True
        if long_sentence:
            tt1s += split_text_by_comma(tt1)
        if len(tt1 + t1) < 200:
            tt1 += f"{t1}."
        else:
            tt1s.append(tt1 + t1)
            tt1 = ""

    return tt1s

def url_encode(input_str):
    return urllib.parse.quote(input_str, safe='')

def url_decode(encoded_str):
    return urllib.parse.unquote(encoded_str)

def tts(request):
    try:
        text = request.GET.get('text')
        if not text:
            if 'text' in request.headers:
                text = request.headers["text"]
            else:
                text = request.body

        TLogger.tlog(f"Got text: {text}")
        text = url_decode(text)
        TLogger.tlog(f"Got decoded text: {text}")
        texts = split_text(text)
        TLogger.tlog(f"split text: {texts}")
        threads_number = len(texts)

        running_threads = []
        for i in range(threads_number):
            work_thread = threading.Thread(target=request_once, args=(texts[i], i))
            work_thread.start()
            running_threads.append(work_thread)
            # wait connection pool release
            if len(running_threads) >= GOOGLE_TTS_CONNECTION_POOL_MAX:
                for thread in running_threads:
                    thread.join()
                running_threads.clear()

        for thread in running_threads:
            thread.join()

        data = []
        for i in range(len(texts)):
            w = wave.open(f"output_{i}.wav", 'rb')
            data.append([w.getparams(), w.readframes(w.getnframes())])
            w.close()

        output = wave.open("output.wav", 'wb')
        output.setparams(data[0][0])
        for i in range(len(data)):
            output.writeframes(data[i][1])

        TLogger.tlog(f"Combined audio content written to file output.wav")

        return get_json_response_result(SUCCESS)
    except Exception as e:
        TLogger.terror(e)
        return get_json_response_result(FALSE)


def get_tts_result(requset):
    try:
        TLogger.tlog(f"Return wav file")
        return FileResponse(open("output.wav", "rb"), as_attachment=True)
    except Exception as e:
        TLogger.terror(e)
        return get_json_response_result(FALSE)
