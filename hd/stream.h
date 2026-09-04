#ifndef STREAM_H
#define STREAM_H
int stream_connect(const char *ip, int port);
int stream_frame(void);
void stream_close(void);
#endif
