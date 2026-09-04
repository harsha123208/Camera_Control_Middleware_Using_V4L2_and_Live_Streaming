# Camera Control Middleware Using V4L2

A Linux-based camera control middleware implemented in C using the **V4L2 (Video4Linux2) API**. The project supports USB UVC camera configuration, MJPEG video capture, MMAP buffer management, runtime camera controls, and live video streaming over TCP/IP.

## Project Files

| File              | Purpose                                       |
| ----------------- | --------------------------------------------- |
| `main.c`          | Main application and camera control interface |
| `receiver.c`      | Receives and displays the streamed video      |
| `src/camera.c`    | V4L2 camera initialization and frame capture  |
| `src/controls.c`  | Camera control handling                       |
| `src/streaming.c` | TCP/IP video streaming                        |

## Hardware and Software

| Item             | Configuration  |
| ---------------- | -------------- |
| Platform         | Raspberry Pi 5 |
| Operating System | Linux          |
| Camera           | USB UVC Camera |
| Video Format     | MJPEG          |
| Resolution       | 1920 × 1080    |
| Buffer Type      | MMAP           |
| Interface        | V4L2           |
| Streaming        | TCP/IP         |

## Features

* USB UVC camera support
* V4L2 camera configuration
* MJPEG video capture
* MMAP buffer management
* Multiple V4L2 buffers
* Camera control using V4L2 IOCTLs
* Brightness control
* Contrast control
* Saturation control
* Exposure control
* White balance control
* Auto exposure control
* Auto white balance control
* Live video streaming using TCP/IP
* POSIX thread-based streaming

## Camera Controls

The middleware allows camera parameters to be changed during runtime.

Supported controls include:

* Brightness
* Contrast
* Saturation
* Exposure
* White Balance
* Auto Exposure
* Auto White Balance

Example commands:

```text
brightness 50
get brightness

contrast 40
get contrast

saturation 60
get saturation
```

## V4L2 Capture Flow

The camera capture follows the standard V4L2 buffer flow:

```text
Open Camera
    ↓
Set Format
    ↓
Request Buffers
    ↓
MMAP
    ↓
QBUF
    ↓
STREAMON
    ↓
DQBUF
    ↓
Process Frame
    ↓
QBUF
    ↓
Repeat
```

## Build

Install the required packages:

```bash
sudo apt update
sudo apt install gcc v4l-utils libjpeg-dev libsdl2-dev
```

Build the camera application:

```bash
gcc -o camera_app main.c src/camera.c src/controls.c src/streaming.c -lpthread
```

Build the receiver:

```bash
gcc -o receiver receiver.c -ljpeg -lSDL2
```

## Check Camera

List connected video devices:

```bash
v4l2-ctl --list-devices
```

Check supported formats:

```bash
v4l2-ctl --device=/dev/video0 --list-formats-ext
```

Check camera controls:

```bash
v4l2-ctl --device=/dev/video0 --list-ctrls
```

## Run

Start the camera application:

```bash
./camera_app
```

Start the receiver on the PC:

```bash
./receiver
```

The camera application captures MJPEG frames using V4L2 and sends them to the receiver through a TCP socket.

## Technologies Used

* C
* Linux
* V4L2
* USB UVC
* MMAP
* MJPEG
* POSIX Threads
* TCP/IP
* Socket Programming
* GCC

## Learning Outcomes

This project demonstrates practical knowledge of **Linux V4L2 camera programming, MMAP buffer management, V4L2 camera controls, POSIX threads, socket programming, and live video streaming**.
