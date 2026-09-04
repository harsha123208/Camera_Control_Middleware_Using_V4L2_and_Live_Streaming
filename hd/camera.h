#ifndef CAMERA_H
#define CAMERA_H
#include <stddef.h>
#include <linux/videodev2.h>
#define BUFFER_COUNT 2
int camera_open(const char *device);
int camera_start(void);
int camera_stop(void);
int camera_get_frame(struct v4l2_buffer *buf);
int camera_release_frame(struct v4l2_buffer *buf);
void *camera_get_buffer(int index);
int camera_get_control(int id, int *value);
int camera_set_control(int id, int value);
void camera_close(void);
#endif
