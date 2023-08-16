# ESP32-CAM_Audio

This sketch allows you to use the ESP32-CAM as an MJPEG & WAV streaming webserver.
Audio (your voice) is captured by an INMP441 microphone and stream to the WAV format.

The code was tested  with AI-Thinker ESP32-CAM and OV2640 camera module.
It should also work with other boards, OV5640 camera module and other MEMS microphones with some adaptation but I didn't test it yet.

The sketch uses the basic Arduino CameraWebServer example to which I added an audio server.

<u>ESP32-CAM Urls : </u>

- Camera settings : http://ESPIPADDRESS
- Video stream : http://ESPIPADDRESS:81/stream
- Audio stream : http://ESPIPADDRESS:82/audio
- Video + Audio stream : http://ESPIPADDRESS:83


Video + Audio stream is made as an example but is not ideal because there is an audio delay related to the web browser buffer.
I recommend using go2rtc streaming application in order to have almost ZERO latency.

## Wiring

![Alt text](/img/image-4.png)

## Sketch configuration

Configure your WiFi network credentials.
```
const char* ssid = "ssid";
const char* password = "password";
```
Edit the audio_server.h file to configure microphone I2S pins (select the pins according to your connections).
```
#define I2S_WS            2 
#define I2S_SCK           14 
#define I2S_SD            15 
```
## Usage

Here is the method I use to access the streams from a web page.
In order to have a single point for broadcasting video and audio with low latency, I use 
[go2rtc](https://github.com/AlexxIT/go2rtc) which is a camera streaming application.

- [Install go2rtc](https://github.com/AlexxIT/go2rtc#fast-start)
- [Configure the video and audio source stream](#Video-and-audio-source-stream-configuration)
- [Custom HTML page](#Custom-HTML-page).

## Video and audio source stream configuration
You must edit the go2rtc.yaml configuration file.This can be done directly from the web interface.

1. Go to the Config menu.
2. Add the ESP32-CAM stream links.
3. Click the Save & Restart button.

![Alt text](/img/image-1.png)

```yaml
streams:

  ESP32-CAM_video: http://192.168.10.55:81/stream
  ESP32-CAM_audio: http://192.168.10.55:82/audio#audio=opus
```
Alternative to avoid creating a custom HTML web page.
```yaml
streams:

  ESP32-CAM:
    - ffmpeg:http://192.168.10.55:81/stream#video=h264
    - ffmpeg:http://192.168.10.55:82/audio#audio=opus
```
Link to the stream: https://go2rtc.local/stream.html?src=ESP32-CAM

![Alt text](/img/image-3.png)

## Custom HTML page
The custom html page contains two Iframes that allow video and audio to be combined.
Adapt the ESP32CAM_stream.html file with the following links :

- Video : https://go2rtc.local/stream.html?src=ESP32-CAM_video&mode=mjpeg
- Audio : https://go2rtc.local/stream.html?src=ESP32-CAM_audio

![Alt text](/img/image-2.png)

### Home Assistant

It is also possible to integrate into Home Assistant.

Webpage card without custom HTLM page.
```yaml
type: iframe
url: https://go2rtc.local/stream.html?src=ESP32-CAM
aspect_ratio: 100%
```
