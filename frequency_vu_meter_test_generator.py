import math
import struct
import wave
from pathlib import Path

SAMPLE_RATE = 48_000
FREQUENCIES = [
    31.6, 50.2, 79.4, 126.5, 200, 316.2, 502, 793.7,
    1264.9, 2000, 3162.3, 5020, 7937, 12649, 17888,
]

AMPLITUDE = 10 ** (-26 / 20)  # Peak amplitude for -26 dBFS
TONE_SAMPLES = SAMPLE_RATE
SILENCE_SAMPLES = SAMPLE_RATE // 2
FADE_SAMPLES = round(0.005 * SAMPLE_RATE)
EDGE_SILENCE_SAMPLES = 2 * SAMPLE_RATE

output_path = Path(__file__).with_name("frequency_vu_meter_test.wav")
frames = [(0, 0)] * EDGE_SILENCE_SAMPLES

for frequency in FREQUENCIES:
    for n in range(TONE_SAMPLES):
        # Apply a 5 ms fade at each edge of the tone.
        envelope = min(
            1.0,
            (n + 1) / FADE_SAMPLES,
            (TONE_SAMPLES - n) / FADE_SAMPLES,
        )
        value = AMPLITUDE * envelope * math.sin(
            2 * math.pi * frequency * n / SAMPLE_RATE
        )
        sample = max(-32768, min(32767, round(value * 32767)))
        frames.append((sample, sample))  # Identical left and right channels

    frames.extend([(0, 0)] * SILENCE_SAMPLES)

frames.extend([(0, 0)] * EDGE_SILENCE_SAMPLES)

with wave.open(str(output_path), "wb") as wav_file:
    wav_file.setnchannels(2)
    wav_file.setsampwidth(2)  # 16-bit PCM
    wav_file.setframerate(SAMPLE_RATE)
    wav_file.writeframes(
        b"".join(struct.pack("<hh", left, right) for left, right in frames)
    )

print(f"Created: {output_path}")