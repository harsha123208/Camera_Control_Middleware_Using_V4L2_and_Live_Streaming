
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "camera.h"
#include "stream.h"

static int sock = -1;

// Create a TCP socket and connect to the receiver.
int stream_connect(const char *ip, int port)
{
    struct sockaddr_in addr;

    // Create an IPv4 TCP socket.
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        perror("socket");
        return -1;
    }

    // Set the receiver address and port.
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    // Convert the IP address to network format.
    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0)
    {

        perror("inet_pton");
        close(sock);
        sock = -1;
        return -1;
    }

    printf("Connecting to %s:%d...\n", ip, port);

    // Establish the TCP connection.
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {

        perror("connect");
        close(sock);
        sock = -1;
        return -1;
    }

    printf("Connected to PC\n");

    return 0;
}

// Send all requested data through the socket.
static int send_all(const void *data, size_t size)
{
    size_t sent = 0;

    while (sent < size) {

        ssize_t n = send(sock,(const char *)data + sent,size - sent,0);

        if (n <= 0)
            return -1;

        sent += n;
    }

    return 0;
}

// Capture one frame and send it to the receiver.
int stream_frame(void)
{
    struct v4l2_buffer buf;

    // Dequeue a captured frame from the V4L2 driver.
    int ret = camera_get_frame(&buf);

    if (ret != 0)
        return ret;

    // Get the mapped address of the captured frame.
    void *frame = camera_get_buffer(buf.index);

    if (frame == NULL) 
   {

        camera_release_frame(&buf);
        return -1;
    }

    // Send the frame size before the frame data.
    uint32_t size = htonl(buf.bytesused);

    if (send_all(&size, sizeof(size)) < 0)
    {

        camera_release_frame(&buf);
        return -1;
    }

    // Send the MJPEG frame data.
    if (send_all(frame, buf.bytesused) < 0) 
   {

        camera_release_frame(&buf);
        return -1;
    }

    // Queue the buffer back for the next frame.
    camera_release_frame(&buf);

    return 0;
}

// Close the TCP socket.
void stream_close(void)
{
    if (sock >= 0) 
   {

        close(sock);
        sock = -1;
    }
}

