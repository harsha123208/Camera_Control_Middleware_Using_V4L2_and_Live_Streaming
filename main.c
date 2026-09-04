#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "camera.h"
#include "controls.h"
#include "stream.h"
#define CAMERA_DEVICE "/dev/video0"
#define PC_IP   "192.168.0.86"
#define PC_PORT 5000
static volatile int running = 1;
static volatile int streaming = 0;

// Streaming thread
void *stream_thread(void *arg)
{

    while (running) 
   {

        if (!streaming) 
        {
            continue;
        }

        if (stream_frame() < 0) 
        {
            printf("Frame streaming error\n");
            break;
        }
    }

    return NULL;
}

// Process user commands
void process_command(char *cmd)
{
    int value;

    if (strcmp(cmd, "help") == 0) 
    {
        show_controls();
        return;
    }

    if (strcmp(cmd, "get brightness") == 0) 
    {
        get_brightness();
        return;
    }

    if (sscanf(cmd, "brightness %d", &value) == 1) 
    {
        set_brightness(value);
        return;
    }

    if (strcmp(cmd, "get contrast") == 0) 
    {
        get_contrast();
        return;
    }

    if (sscanf(cmd, "contrast %d", &value) == 1) 
    {
        set_contrast(value);
        return;
    }

    if (strcmp(cmd, "get saturation") == 0) 
    {
        get_saturation();
        return;
    }

    if (sscanf(cmd, "saturation %d", &value) == 1) 
    {
        set_saturation(value);
        return;
    }

    if (strcmp(cmd, "get auto exposure") == 0) 
     {
        get_auto_exposure();
        return;
    }

    if (sscanf(cmd, "auto exposure %d", &value) == 1) 
    {
        set_auto_exposure(value);
        return;
    }

    if (strcmp(cmd, "get exposure") == 0) 
  {
        get_exposure();
        return;
    }

    if (sscanf(cmd, "exposure %d", &value) == 1) 
    {
        set_exposure(value);
        return;
    }

    if (strcmp(cmd, "get auto whitebalance") == 0) {
        get_auto_whitebalance();
        return;
    }

    if (sscanf(cmd, "auto whitebalance %d", &value) == 1)
    {
        set_auto_whitebalance(value);
        return;
    }

    if (strcmp(cmd, "get whitebalance") == 0) 
    {
        get_whitebalance();
        return;
    }

    if (sscanf(cmd, "whitebalance %d", &value) == 1) {
        set_whitebalance(value);
        return;
    }

    if (strcmp(cmd, "start") == 0) 
      {

        if (streaming) 
        {
            printf("Already streaming\n");
            return;
        }

        if (camera_start() < 0) 
        {
            printf("Failed to start camera\n");
            return;
        }

        streaming = 1;

        printf("Streaming started\n");

        return;
    }

    if (strcmp(cmd, "stop") == 0) 
        {

        if (!streaming) 
        {
            printf("Already paused\n");
            return;
        }

        streaming = 0;

        printf("Streaming paused\n");

        return;
    }

    if (strcmp(cmd, "resume") == 0) 
        {

        if (streaming) {
            printf("Already streaming\n");
            return;
        }

        streaming = 1;

        printf("Streaming resumed\n");

        return;
    }

    if (strcmp(cmd, "default") == 0) {
        default_controls();
        return;
    }

    printf("Unknown command\n");
    printf("Type 'help'\n");
}

int main(void)
{
    pthread_t thread;
    char command[100];

    if (stream_connect(PC_IP, PC_PORT) < 0)
        return 1;

    if (camera_open(CAMERA_DEVICE) < 0) 
   {

        stream_close();

        return 1;
    }

    printf("\nCamera initialized\n");
    printf("Type 'help' for commands\n\n");

    if (pthread_create(&thread, NULL, stream_thread, NULL) != 0) {

        perror("pthread_create");

        camera_stop();
        camera_close();
        stream_close();

        return 1;
    }

    while (running) 
     {

        printf("camera> ");
        fflush(stdout);

        if (fgets(command, sizeof(command), stdin) == NULL)
            break;

        // Remove newline from command
        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "quit") == 0 ||strcmp(command, "exit") == 0) 
        {

            running = 0;
            break;
        }

        process_command(command);
    }

    pthread_join(thread, NULL);

    camera_stop();
    camera_close();
    stream_close();

    printf("Application closed\n");

    return 0;
}
