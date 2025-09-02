import requests
import wave
import sys

# --- Configuration ---
# Replace with the actual URL of the continuous WAV audio stream
STREAM_URL = "http://10.92.204.98:82/audio"

# Input stream parameters (fixed based on your description)
INPUT_BIT_DEPTH = 32
INPUT_CHANNELS = 2  # Assuming stereo based on standard practice

# Output WAV file parameters
OUTPUT_BIT_DEPTH = 24
OUTPUT_CHANNELS = INPUT_CHANNELS
SAMPLE_RATE = 22050 

# Duration to capture in seconds
CAPTURE_DURATION_SECONDS = 10

# Calculate the number of bytes to capture for the specified duration for the OUTPUT file
# Formula: (sample rate * channels * bit depth / 8) * duration
BYTES_PER_SECOND_OUTPUT = SAMPLE_RATE * OUTPUT_CHANNELS * (OUTPUT_BIT_DEPTH / 8)
TOTAL_BYTES_TO_CAPTURE_OUTPUT = int(BYTES_PER_SECOND_OUTPUT * CAPTURE_DURATION_SECONDS)

# --- Script Logic ---
print(f"Attempting to connect to the 32-bit big-endian stream at: {STREAM_URL}")
print(f"Capturing {CAPTURE_DURATION_SECONDS} seconds and converting to 24-bit...")

try:
    # Use requests.get with stream=True for continuous streams
    response = requests.get(STREAM_URL, stream=True, timeout=10)
    response.raise_for_status()

    # Open a new WAV file to write the captured audio data
    with wave.open("captured_audio_24bit.wav", "wb") as wav_file:
        # Set the parameters for the output WAV file
        wav_file.setnchannels(OUTPUT_CHANNELS)
        wav_file.setsampwidth(OUTPUT_BIT_DEPTH // 8)  # Sets sample width to 3 bytes
        wav_file.setframerate(SAMPLE_RATE)

        bytes_written = 0

        # We need to process the stream in chunks and convert the data
        # before writing to the file.
        for chunk in response.iter_content(chunk_size=4096):
            if bytes_written >= TOTAL_BYTES_TO_CAPTURE_OUTPUT:
                break

            # Create a bytearray to hold the converted 24-bit data
            converted_data = bytearray()

            # Process the 32-bit chunk, 4 bytes at a time
            for i in range(0, len(chunk), INPUT_BIT_DEPTH // 8):
                # Since the data is big-endian, the top 24 bits are the
                # first 3 bytes. We discard the last byte (LSB).
                converted_data.extend(chunk[i : i + 3])

            # Write the converted 24-bit data to the file
            bytes_to_write = min(
                len(converted_data), TOTAL_BYTES_TO_CAPTURE_OUTPUT - bytes_written
            )
            wav_file.writeframes(converted_data[:bytes_to_write])
            bytes_written += bytes_to_write

            # Print progress
            progress_percent = (bytes_written / TOTAL_BYTES_TO_CAPTURE_OUTPUT) * 100
            sys.stdout.write(
                f"\rProgress: {progress_percent:.2f}% | Bytes written: {bytes_written}"
            )
            sys.stdout.flush()

        print("\nCapture complete!")
        print(
            f"Saved {bytes_written} bytes of 24-bit audio to 'captured_audio_24bit.wav'"
        )

except requests.exceptions.RequestException as e:
    print(f"Error connecting to or reading from the stream: {e}")
    sys.exit(1)

except wave.Error as e:
    print(f"Error writing to the WAV file: {e}")
    sys.exit(1)
