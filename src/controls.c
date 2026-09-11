#include <stdio.h>
#include <linux/videodev2.h>

#include "camera.h"
#include "controls.h"


void set_brightness(int value)
{
    if (camera_set_control(V4L2_CID_BRIGHTNESS, value) == 0)
        printf("Brightness set to %d\n", value);
}

void get_brightness(void)
{
    int value;

    if (camera_get_control(V4L2_CID_BRIGHTNESS, &value) == 0)
        printf("Brightness: %d\n", value);
}


void set_contrast(int value)
{
    if (camera_set_control(V4L2_CID_CONTRAST, value) == 0)
        printf("Contrast set to %d\n", value);
}

void get_contrast(void)
{
    int value;

    if (camera_get_control(V4L2_CID_CONTRAST, &value) == 0)
        printf("Contrast: %d\n", value);
}


void set_saturation(int value)
{
    if (camera_set_control(V4L2_CID_SATURATION, value) == 0)
        printf("Saturation set to %d\n", value);
}

void get_saturation(void)
{
    int value;

    if (camera_get_control(V4L2_CID_SATURATION, &value) == 0)
        printf("Saturation: %d\n", value);
}


/* Auto exposure */

void set_auto_exposure(int value)
{
    int mode;

    if (value == 0)
        mode = V4L2_EXPOSURE_MANUAL;

    else if (value == 1)
        mode = V4L2_EXPOSURE_APERTURE_PRIORITY;

    else {
        printf("Use 0 or 1\n");
        return;
    }

    if (camera_set_control(V4L2_CID_EXPOSURE_AUTO,mode) == 0) {

        printf("Auto Exposure: %s\n",
               value ? "ON" : "OFF");
    }
}


void get_auto_exposure(void)
{
    int value;

    if (camera_get_control(V4L2_CID_EXPOSURE_AUTO,&value) == 0) {

        if (value == V4L2_EXPOSURE_MANUAL)
            printf("Auto Exposure: OFF\n");
        else
            printf("Auto Exposure: ON\n");
    }
}


/* Manual exposure */

void set_exposure(int value)
{
    int mode;

    if (camera_get_control(V4L2_CID_EXPOSURE_AUTO,&mode) < 0)
        return;

    if (mode != V4L2_EXPOSURE_MANUAL) {
        printf("Turn auto exposure OFF first\n");
        return;
    }

    if (camera_set_control(V4L2_CID_EXPOSURE_ABSOLUTE,value) == 0) {

        printf("Exposure set to %d\n", value);
    }
}


void get_exposure(void)
{
    int value;

    if (camera_get_control(V4L2_CID_EXPOSURE_ABSOLUTE,&value) == 0) {

        printf("Exposure: %d\n", value);
    }
}


/* Auto white balance */

void set_auto_whitebalance(int value)
{
    if (value != 0 && value != 1) {
        printf("Use 0 or 1\n");
        return;
    }

    if (camera_set_control(V4L2_CID_AUTO_WHITE_BALANCE,value) == 0) {

        printf("Auto White Balance: %s\n",
               value ? "ON" : "OFF");
    }
}


void get_auto_whitebalance(void)
{
    int value;

    if (camera_get_control(V4L2_CID_AUTO_WHITE_BALANCE,&value) == 0) {

        printf("Auto White Balance: %s\n",
               value ? "ON" : "OFF");
    }
}


/* Manual white balance */

void set_whitebalance(int value)
{
    int auto_wb;

    if (camera_get_control(V4L2_CID_AUTO_WHITE_BALANCE,&auto_wb) < 0)
        return;

    if (auto_wb) {
        printf("Turn auto white balance OFF first\n");
        return;
    }

    if (camera_set_control(V4L2_CID_WHITE_BALANCE_TEMPERATURE,value) == 0) {

        printf("White Balance: %d K\n", value);
    }
}


void get_whitebalance(void)
{
    int value;

    if (camera_get_control(V4L2_CID_WHITE_BALANCE_TEMPERATURE,&value) == 0) {

        printf("White Balance: %d K\n", value);
    }
}


/* Restore defaults */

void default_controls(void)
{
    printf("Restoring camera defaults...\n");

    camera_set_control(V4L2_CID_BRIGHTNESS, 0);
    camera_set_control(V4L2_CID_CONTRAST, 34);
    camera_set_control(V4L2_CID_SATURATION, 56);
    camera_set_control(V4L2_CID_AUTO_WHITE_BALANCE,1);


    camera_set_control(V4L2_CID_EXPOSURE_AUTO,V4L2_EXPOSURE_APERTURE_PRIORITY);

    printf("Camera controls restored to default\n");
}


/* Help */

void show_controls(void)
{
    printf("\nCamera Controls:\n");

    printf("brightness <value>\n");
    printf("get brightness\n");

    printf("contrast <value>\n");
    printf("get contrast\n");

    printf("saturation <value>\n");
    printf("get saturation\n");

    printf("auto exposure <0|1>\n");
    printf("get auto exposure\n");

    printf("exposure <value>\n");
    printf("get exposure\n");

    printf("auto whitebalance <0|1>\n");
    printf("get auto whitebalance\n");

    printf("whitebalance <value>\n");
    printf("get whitebalance\n");

    printf("start\n");
    printf("stop\n");
    printf("resume\n");

    printf("default\n");
    printf("help\n");
    printf("quit\n");
}
