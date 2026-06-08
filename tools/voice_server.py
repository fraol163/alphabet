#!/usr/bin/env python3
"""
Alphabet Language Voice-to-Text Server
======================================
Offline speech-to-text for all 5 Alphabet languages.
Uses Vosk for en/es/fr/de and Whisper for am (Amharic).

Protocol:
  Input:  JSON lines on stdin
  Output: JSON lines on stdout

Commands:
  {"cmd": "init", "lang": "en"}           — Initialize STT for language
  {"cmd": "listen", "timeout": 10}        — Listen for speech, return text
  {"cmd": "status"}                       — Check server status
  {"cmd": "quit"}                         — Shutdown

Response:
  {"status": "ok", "text": "hello world"} — Transcribed text
  {"status": "error", "msg": "..."}       — Error message
  {"status": "ready"}                     — Server ready
"""

import sys
import json
import os
import tempfile
import wave
import struct
import subprocess
import threading

# ─── Configuration ───────────────────────────────────────────
VOSK_MODELS = {
    "en": "vosk-model-en-us-0.22",
    "es": "vosk-model-es-0.42",
    "fr": "vosk-model-fr-0.22",
    "de": "vosk-model-de-0.21",
}
VOSK_MODEL_DIR = os.path.expanduser("~/.alphabet/vosk_models")
WHISPER_MODEL = "small"  # tiny/base/small/medium/large
WHISPER_MODEL_DIR = os.path.expanduser("~/.alphabet/whisper_models")
SAMPLE_RATE = 16000
CHANNELS = 1

# ─── State ───────────────────────────────────────────────────
current_lang = None
vosk_model = None
whisper_loaded = False


def log(msg):
    """Log to stderr (doesn't interfere with stdout protocol)."""
    print(f"[voice] {msg}", file=sys.stderr, flush=True)


def send(obj):
    """Send JSON response on stdout."""
    print(json.dumps(obj), flush=True)


def ensure_vosk():
    """Check if vosk is available."""
    try:
        import vosk
        return True
    except ImportError:
        return False


def ensure_whisper():
    """Check if whisper is available."""
    try:
        import whisper
        return True
    except ImportError:
        return False


def download_vosk_model(lang):
    """Download Vosk model if not present."""
    model_name = VOSK_MODELS.get(lang)
    if not model_name:
        return False

    model_path = os.path.join(VOSK_MODEL_DIR, model_name)
    if os.path.exists(model_path):
        return True

    log(f"Downloading Vosk model for '{lang}'...")
    os.makedirs(VOSK_MODEL_DIR, exist_ok=True)

    url = f"https://alphacephei.com/vosk/models/{model_name}.zip"
    zip_path = os.path.join(VOSK_MODEL_DIR, f"{model_name}.zip")

    try:
        import urllib.request
        urllib.request.urlretrieve(url, zip_path)

        import zipfile
        with zipfile.ZipFile(zip_path, 'r') as z:
            z.extractall(VOSK_MODEL_DIR)

        os.remove(zip_path)
        log(f"Vosk model downloaded: {model_path}")
        return True
    except Exception as e:
        log(f"Failed to download Vosk model: {e}")
        return False


def init_vosk(lang):
    """Initialize Vosk for a language."""
    global vosk_model, current_lang

    if not ensure_vosk():
        send({"status": "error", "msg": "Vosk not installed. Run: pip install vosk"})
        return

    model_name = VOSK_MODELS.get(lang)
    if not model_name:
        send({"status": "error", "msg": f"No Vosk model for language '{lang}'"})
        return

    model_path = os.path.join(VOSK_MODEL_DIR, model_name)
    if not os.path.exists(model_path):
        if not download_vosk_model(lang):
            send({"status": "error", "msg": f"Could not download Vosk model for '{lang}'"})
            return

    try:
        import vosk
        vosk_model = vosk.Model(model_path)
        current_lang = lang
        log(f"Vosk initialized for '{lang}'")
        send({"status": "ready"})
    except Exception as e:
        send({"status": "error", "msg": f"Vosk init failed: {e}"})


def init_whisper():
    """Initialize Whisper for Amharic."""
    global whisper_loaded, current_lang

    if not ensure_whisper():
        send({"status": "error", "msg": "Whisper not installed. Run: pip install openai-whisper"})
        return

    current_lang = "am"
    whisper_loaded = True
    log("Whisper initialized for Amharic")
    send({"status": "ready"})


def record_audio(duration=10):
    """Record audio from microphone."""
    try:
        import pyaudio
        pa = pyaudio.PyAudio()
        stream = pa.open(
            format=pyaudio.paInt16,
            channels=CHANNELS,
            rate=SAMPLE_RATE,
            input=True,
            frames_per_buffer=4096
        )

        log(f"Listening for up to {duration}s...")
        frames = []
        silent_frames = 0
        max_silent = int(SAMPLE_RATE / 4096 * 2)  # 2s silence = stop

        for i in range(0, int(SAMPLE_RATE / 4096 * duration)):
            data = stream.read(4096, exception_on_overflow=False)
            frames.append(data)

            # Simple silence detection
            samples = struct.unpack(f'{len(data)//2}h', data)
            rms = (sum(s*s for s in samples) / len(samples)) ** 0.5
            if rms < 500:  # silence threshold
                silent_frames += 1
                if silent_frames > max_silent and len(frames) > 20:
                    log("Silence detected, stopping")
                    break
            else:
                silent_frames = 0

        stream.stop_stream()
        stream.close()
        pa.terminate()

        # Save to WAV
        tmp = tempfile.NamedTemporaryFile(suffix=".wav", delete=False)
        wf = wave.open(tmp.name, 'wb')
        wf.setnchannels(CHANNELS)
        wf.setsampwidth(2)
        wf.setframerate(SAMPLE_RATE)
        wf.writeframes(b''.join(frames))
        wf.close()

        return tmp.name
    except ImportError:
        # Fallback: use arecord (Linux) or sox
        return record_audio_fallback(duration)


def record_audio_fallback(duration=10):
    """Record audio using system tools (arecord/sox)."""
    tmp_path = tempfile.mktemp(suffix=".wav")

    # Try arecord (Linux/ALSA)
    if subprocess.run(["which", "arecord"], capture_output=True).returncode == 0:
        log(f"Recording with arecord for {duration}s...")
        try:
            proc = subprocess.run(
                ["arecord", "-f", "S16_LE", "-r", str(SAMPLE_RATE),
                 "-c", str(CHANNELS), "-d", str(duration), tmp_path],
                timeout=duration + 2,
                capture_output=True
            )
            if proc.returncode == 0 and os.path.exists(tmp_path):
                return tmp_path
        except subprocess.TimeoutExpired:
            pass

    # Try sox (cross-platform)
    if subprocess.run(["which", "sox"], capture_output=True).returncode == 0:
        log(f"Recording with sox for {duration}s...")
        try:
            proc = subprocess.run(
                ["sox", "-d", "-r", str(SAMPLE_RATE), "-c", str(CHANNELS),
                 "-b", "16", tmp_path, "trim", "0", str(duration)],
                timeout=duration + 2,
                capture_output=True
            )
            if proc.returncode == 0 and os.path.exists(tmp_path):
                return tmp_path
        except subprocess.TimeoutExpired:
            pass

    log("No audio recording tool found (need pyaudio, arecord, or sox)")
    return None


def transcribe_vosk(wav_path):
    """Transcribe audio using Vosk."""
    import vosk

    wf = wave.open(wav_path, "rb")
    if wf.getnchannels() != 1 or wf.getsampwidth() != 2:
        send({"status": "error", "msg": "Audio must be mono 16-bit PCM"})
        return None

    rec = vosk.KaldiRecognizer(vosk_model, wf.getframerate())
    rec.SetWords(True)

    results = []
    while True:
        data = wf.readframes(4000)
        if len(data) == 0:
            break
        if rec.AcceptWaveform(data):
            result = json.loads(rec.Result())
            if result.get("text"):
                results.append(result["text"])

    final = json.loads(rec.FinalResult())
    if final.get("text"):
        results.append(final["text"])

    wf.close()
    return " ".join(results)


def transcribe_whisper(wav_path):
    """Transcribe audio using Whisper."""
    import whisper

    log("Transcribing with Whisper (may take 5-15s)...")
    model = whisper.load_model(WHISPER_MODEL, download_root=WHISPER_MODEL_DIR)
    result = model.transcribe(
        wav_path,
        language="am",
        task="transcribe",
        fp16=False
    )
    return result["text"]


def handle_listen(timeout=10):
    """Listen for speech and transcribe."""
    global current_lang

    if current_lang is None:
        send({"status": "error", "msg": "No language initialized. Call init first."})
        return

    # Record audio
    wav_path = record_audio(timeout)
    if not wav_path:
        send({"status": "error", "msg": "Failed to record audio"})
        return

    try:
        # Transcribe based on language
        if current_lang == "am":
            if not whisper_loaded:
                send({"status": "error", "msg": "Whisper not initialized"})
                return
            text = transcribe_whisper(wav_path)
        else:
            if vosk_model is None:
                send({"status": "error", "msg": "Vosk not initialized"})
                return
            text = transcribe_vosk(wav_path)

        if text:
            log(f"Transcribed: {text}")
            send({"status": "ok", "text": text.strip()})
        else:
            send({"status": "ok", "text": ""})
    except Exception as e:
        send({"status": "error", "msg": f"Transcription failed: {e}"})
    finally:
        # Cleanup temp file
        try:
            os.remove(wav_path)
        except:
            pass


def handle_status():
    """Report server status."""
    status = {
        "status": "ok",
        "lang": current_lang,
        "vosk": vosk_model is not None,
        "whisper": whisper_loaded,
        "pyaudio": False,
        "arecord": False,
        "sox": False,
    }

    try:
        import pyaudio
        status["pyaudio"] = True
    except:
        pass

    if subprocess.run(["which", "arecord"], capture_output=True).returncode == 0:
        status["arecord"] = True
    if subprocess.run(["which", "sox"], capture_output=True).returncode == 0:
        status["sox"] = True

    send(status)


def main():
    """Main server loop — reads JSON commands from stdin."""
    log("Alphabet Voice Server started")
    log("Commands: init, listen, status, quit")

    for line in sys.stdin:
        line = line.strip()
        if not line:
            continue

        try:
            cmd = json.loads(line)
        except json.JSONDecodeError:
            send({"status": "error", "msg": f"Invalid JSON: {line}"})
            continue

        action = cmd.get("cmd", "")

        if action == "init":
            lang = cmd.get("lang", "en")
            if lang == "am":
                init_whisper()
            else:
                init_vosk(lang)

        elif action == "listen":
            timeout = cmd.get("timeout", 10)
            handle_listen(timeout)

        elif action == "status":
            handle_status()

        elif action == "quit":
            send({"status": "ok", "msg": "bye"})
            break

        else:
            send({"status": "error", "msg": f"Unknown command: {action}"})


if __name__ == "__main__":
    main()
