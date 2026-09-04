#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

#include "camera.h"

// Structure used to store information about an MMAP buffer.
// start stores the mapped user-space address.
// length stores the size of the mapped buffer.
struct camera_buffer
{
    void *start;
    size_t length;
};

// Indicates whether camera streaming is active.
static int camera_streaming = 0;
static int camera_fd = -1;

// Array containing information about all mapped buffers.
static struct camera_buffer buffers[BUFFER_COUNT];
static int buffer_count = 0;

// Retry ioctl if it is interrupted by a signal.
static int xioctl(unsigned long request, void *arg)
{
    int ret;

    do {
        ret = ioctl(camera_fd, request, arg);
    } while (ret == -1 && errno == EINTR);

    return ret;
}

// Open the camera device and initialize V4L2 buffers.
int camera_open(const char *device)
{
    struct v4l2_capability cap;
    struct v4l2_format fmt;
    struct v4l2_requestbuffers req;

    camera_fd = open(device, O_RDWR);

    if (camera_fd < 0)
   {
        perror("open camera");
        return -1;
    }

    printf("Camera opened: %s\n", device);

    // Query the capabilities supported by the camera driver.
    memset(&cap, 0, sizeof(cap));

    if (xioctl(VIDIOC_QUERYCAP, &cap) < 0)
     {
        perror("VIDIOC_QUERYCAP");
        close(camera_fd);
        camera_fd = -1;
        return -1;
    }

    // Check whether the device supports video capture.
    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        printf("Video capture not supported\n");
        close(camera_fd);
        camera_fd = -1;
        return -1;
    }

    // Check whether the device supports streaming I/O.
    if (!(cap.capabilities & V4L2_CAP_STREAMING)) 
   {
        printf("Streaming not supported\n");
        close(camera_fd);
        camera_fd = -1;
        return -1;
    }

    // Set the required camera resolution and pixel format.
    memset(&fmt, 0, sizeof(fmt));

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = 1920;
    fmt.fmt.pix.height = 1080;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    fmt.fmt.pix.field = V4L2_FIELD_ANY;

    // Apply the requested video format.
    if (xioctl(VIDIOC_S_FMT, &fmt) < 0) 
   {
        perror("VIDIOC_S_FMT");
        close(camera_fd);
        camera_fd = -1;
        return -1;
    }

    printf("Format: %dx%d MJPEG\n",fmt.fmt.pix.width, fmt.fmt.pix.height);

    // Request MMAP buffers from the V4L2 driver.
    memset(&req, 0, sizeof(req));
    req.count = BUFFER_COUNT;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    // Allocate the requested number of capture buffers.
    if (xioctl(VIDIOC_REQBUFS, &req) < 0)
    {
        perror("VIDIOC_REQBUFS");
        close(camera_fd);
        camera_fd = -1;
        return -1;
    }

    buffer_count = req.count;

    printf("Buffers allocated: %d\n", buffer_count);

    // Query and map each V4L2 buffer into user space.
    for (int i = 0; i < buffer_count; i++) 
   {

        struct v4l2_buffer buf;

        memset(&buf, 0, sizeof(buf));

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        // Get the size and memory offset of the buffer.
        if (xioctl(VIDIOC_QUERYBUF, &buf) < 0) 
        {
            perror("VIDIOC_QUERYBUF");
            return -1;
        }

        buffers[i].length = buf.length;

        // Map the driver buffer into the user-space address space.
        buffers[i].start = mmap(NULL,buf.length,PROT_READ | PROT_WRITE,MAP_SHARED,camera_fd,buf.m.offset);

        if (buffers[i].start == MAP_FAILED) 
       {
            perror("mmap");
            buffers[i].start = NULL;
            return -1;
        }

        printf("Buffer %d mapped\n", i);
    }

    // Queue all buffers so the driver can start filling them with frames.
    for (int i = 0; i < buffer_count; i++)
   {

        struct v4l2_buffer buf;

        memset(&buf, 0, sizeof(buf));

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        // Queue the buffer to the driver.
        if (xioctl(VIDIOC_QBUF, &buf) < 0) 
       {
            perror("VIDIOC_QBUF");
            return -1;
        }
    }

    printf("Camera initialization complete\n");

    return 0;
}

// Start camera streaming.
int camera_start(void)
{
    enum v4l2_buf_type type;

    if (camera_streaming) 
   {
        printf("Camera already streaming\n");
        return 0;
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    // Start the V4L2 capture stream.
    if (xioctl(VIDIOC_STREAMON, &type) < 0) 
    {
        perror("VIDIOC_STREAMON");
        return -1;
    }

    camera_streaming = 1;

    printf("Camera streaming started\n");

    return 0;
}

// Stop camera streaming.
int camera_stop(void)
{
    enum v4l2_buf_type type;

    if (!camera_streaming) 
   {
        printf("Camera already stopped\n");
        return 0;
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    // Stop the V4L2 capture stream.
    if (xioctl(VIDIOC_STREAMOFF, &type) < 0) 
   {
        perror("VIDIOC_STREAMOFF");
        return -1;
    }

    camera_streaming = 0;

    printf("Camera streaming stopped\n");

    return 0;
}

// Dequeue one captured frame from the driver.
int camera_get_frame(struct v4l2_buffer *buf)
{
    memset(buf, 0, sizeof(*buf));

    buf->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf->memory = V4L2_MEMORY_MMAP;

    // Get a filled buffer from the driver.
    if (xioctl(VIDIOC_DQBUF, buf) < 0)
    {

        // No frame is available yet.
        if (errno == EAGAIN)
            return 1;

        fprintf(stderr,"VIDIOC_DQBUF failed: errno=%d (%s)\n",errno, strerror(errno));

        return -1;
    }

    // Check that the returned buffer index is valid.
    if (buf->index >= (unsigned int)buffer_count)
    {
        fprintf(stderr,"Invalid buffer index: %d\n",buf->index);
        return -1;
    }

    return 0;
}

// Queue the processed buffer back to the driver.
int camera_release_frame(struct v4l2_buffer *buf)
{
    // Return the buffer to the driver for reuse.
    if (xioctl(VIDIOC_QBUF, buf) < 0)
    {

        perror("VIDIOC_QBUF");
        return -1;
    }

    return 0;
}

// Get the user-space address of a mapped camera buffer.
void *camera_get_buffer(int index)
{
    if (index < 0 || index >= buffer_count)
        return NULL;

    // Return the mapped address of the requested buffer.
    return buffers[index].start;
}

// Get the current value of a V4L2 camera control.
int camera_get_control(int id, int *value)
{
    struct v4l2_control ctrl;

    memset(&ctrl, 0, sizeof(ctrl));

    ctrl.id = id;

    // Read the current value of the camera control.
    if (xioctl(VIDIOC_G_CTRL, &ctrl) < 0)
    {

        perror("VIDIOC_G_CTRL");
        return -1;
    }

    *value = ctrl.value;

    return 0;
}

// Set the value of a V4L2 camera control.
int camera_set_control(int id, int value)
{
    struct v4l2_control ctrl;

    memset(&ctrl, 0, sizeof(ctrl));

    ctrl.id = id;
    ctrl.value = value;

    // Send the new control value to the camera driver.
    if (xioctl(VIDIOC_S_CTRL, &ctrl) < 0)
    {

        perror("VIDIOC_S_CTRL");
        return -1;
    }

    return 0;
}

// Unmap all camera buffers and close the device.
void camera_close(void)
{
    // Unmap all buffers from the user-space address space.
    for (int i = 0; i < buffer_count; i++) 
    {

        if (buffers[i].start != NULL)
       {

            munmap(buffers[i].start, buffers[i].length);

            buffers[i].start = NULL;
            buffers[i].length = 0;
        }
    }

    buffer_count = 0;

    // Close the camera device.
    if (camera_fd >= 0)
   {

        close(camera_fd);
        camera_fd = -1;
    }

    camera_streaming = 0;

    printf("Camera closed\n");
}

