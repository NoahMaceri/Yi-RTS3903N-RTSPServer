# RTSP server for RTS3903N based YI Cameras
*While this repo is focused on Yi based cameras, it should compile and run on any RTS3903N based camera!*

## Background
This work is based on Colin Jensen's [Yi-RTS3903N-RTSPServer](https://github.com/cjj25/Yi-RTS3903N-RTSPServer)
**Important**: This method doesn't overwrite the existing flash, simply remove the SD card, and the 'hack' will be disabled.

## Known Compatible Firmware
- `7.1.00.25A_202002271051`
- `7.1.00.17A_201909271014`
- `7.0.00.73a_201812031453`

## Getting Started
- Download the latest tar from the [releases page](https://github.com/NoahMaceri/RTS3903N-RTSP/releases)
- Extract the contents to the root of a MicroSD card (minimum 2GB) that is FAT32 partitioned
- If you're using WiFi
  - This method will use the already stored WiFi credentials in the camera
  - You can overwrite the saved config by editing `wpa_supplicant_sample.conf` and renaming to `wpa_supplicant.conf` 
    - **Only replace SSID_NAME_OF_WIFI and WIFI_SECRET_KEY, unless you're using WEP encryption or open**
      
  1. Insert the SD and turn on
  2. If you have a pan/tilt camera, it _should_ perform the usual calibration
  3. A telnet server is started (if not already done so by Yi) with username `root` and no password
  4. The imager streamer will automatically start up along with the RTSP server
  6. You'll probably have a pinkish tint on the picture, the IR cut will be performed after ***30 seconds*** to ensure any other binaries have finished taking control of the GPIO
     > If the image isn't quite right (grey / too much pink), place your finger over the sensor on the front (make it very dark) and see what happens. Modify the _invert_adc_ parameter in the streamer.ini
  7. Connect to RTSP via `rtsp://[YOUR_CAMERA_IP]/[rtsp_name]`

## Features
- H264 encoded stream via `rtsp://[YOUR_CAMERA_IP]/[rtsp_name]`
- Telnet server enabled
- Configuration of camera parameters via `streamer.ini`

### In-progress
- Add audio to the feed
- Better documentation

### Planned
- Control PTZ (pan/tilt) based cameras
- ONVIF

## Compiling
For Ubuntu 20.*
```
# Install dependancies
sudo bash install_deps_ubuntu.sh
mkdir build
cmake -S . -B ./build
cd build
ninja
```

## Streaming configuration
Many imager and RTSP settings are provided in the `streamer.ini`

The range of the parameters are provided at the end of the line comment in the format [min-max,step] \
_This range is based on my camera, it might be different for yours!_
```ini
[isp]
; This file contains the ISP settings for the camera module.
; Adjust the settings below to configure the ISP parameters.
noise_reduction=4 ; Adjusts the noise reduction strength [0-7,1]
ldc=1 ; Lens distorion correction [0-1,1]
detail_enhancement=4 ; Adjusts the detail enhancement strenth [0-7,1]
three_dnr=1 ; 3D noise reduction [0-1,1]
mirror=1 ; Mirror image [0-1,1]
flip=1 ; Flip image [0-1,1]
in_out_door_mode=0 ; Indoor/outdoor mode [0-2,1]
dehaze=0 ; Dehaze [0-1,1]
; The adc_cutoff value is used to adjust when night mode is activated.
adc_cutoff=400 ; Lit values start around 200 and lower
adc_cutoff_inverted=2750 ; Lit values start around 3000 and higher
invert_ir_cut=0 ; Invert the IR cut logic

[encoder]
; This section contains settings for the video encoder.
max_bitrate=1024000 ; Max bitrate of the encoder
min_bitrate=512000 ; Minimum bitrate of the encoder
width=1920 ; Resolution of the encoder
height=1080 ; Resolution of the encoder
fps=20 ; FPS of the imager + encoder (I have noticed that most cameras can not effectively reach 30 FPS)

[rtsp]
; RTSP settings for the camera stream.
; You can leave the user and password empty for no authentication.
username= ; Username for RTSP server
password= ; Password for RTSP server
port=554 ; Port for RTSP server
name=ch0_0.h264 ; URL for RTSP server (rtsp://[YOUR_CAMERA_IP]/[name])
```

## Troubleshooting
The RTS3903N uses an ADC for sensing light. On some cameras the logic is inverted and must be set in the `streamer.ini`

## Credit
- rtsp_server
  - [`@roleoroleo`](https://github.com/roleoroleo) Original author
  - [`@alienatedsec`](https://github.com/alienatedsec/) Modified version
  - [`cjj25`](https://github.com/cjj25) Modified version
- imager_stremaer
  - [`@cjj25`](https://github.com/cjj25) Original author
- sd_payload 
  - [`@rage2dev`](https://github.com/rage2dev/) Original author
  - [`@cjj25`](https://github.com/cjj25) Modified version

## Resources
- [Compiled binaries / tools for debugging and test](https://github.com/cjj25/RTS3903N-Tools)
