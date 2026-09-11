CC = gcc
CFLAGS = -Wall -O2 -pthread -Ihd

camera_app:
	$(CC) $(CFLAGS) main.c src/camera.c src/controls.c src/streaming.c -o camera_app

clean:
	rm -f camera_app
