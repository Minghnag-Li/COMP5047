# Backend



To use Google TTS, you need to set up a [Python Client for Google Cloud Text-to-Speech](https://cloud.google.com/python/docs/reference/texttospeech/latest?_gl=1*78cmib*_ga*MTgwNzc3NTk2LjE3MjY3ODMyMzk.*_ga_WH2QY8WWF5*MTczMDAwMzkzNy4xNS4xLjE3MzAwMDM5NzMuMjQuMC4w).



The backend uses the Django framework to process HTTP requests.

1. You need to install [Python 3](https://www.python.org/downloads/release/python-3127/) to run the server.

2. For the dependencies of the backend, you need to install them by command:

   ```cmd
   pip install -r requirements.txt
   ```

3. Then use 

   ``` to 
   python manage.py runserver 0.0.0.0:8000
   ```

   to run the backend.

4. GET http://localhost:8000/tts to generate a TTS wav file on the server.

5. GET http://192.168.137.1:8000/tts/result to get the result of TTS.

