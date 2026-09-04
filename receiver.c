#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <jpeglib.h>
#include <SDL2/SDL.h>

#define PORT 5000

// Receive the complete amount of data requested.
int recv_all(int sock, void *buffer, size_t size)
{
    size_t received = 0;

    while (received < size) {

        ssize_t n = recv(sock, (char *)buffer + received,
                         size - received, 0);

        if (n <= 0)
            return -1;

        received += n;
    }

    return 0;
}

// Decode JPEG data and convert it to RGB format.
int decode_jpeg(unsigned char *jpeg_data, unsigned long jpeg_size,unsigned char **rgb_data, int *width, int *height)
{
    struct jpeg_decompress_struct cinfo;

    // Create and initialize the JPEG decompression object.
    jpeg_create_decompress(&cinfo);

    // Give the JPEG decoder the compressed data directly from memory.
    jpeg_mem_src(&cinfo, jpeg_data, jpeg_size);

    // Read the JPEG header.
    if (jpeg_read_header(&cinfo, TRUE) != JPEG_HEADER_OK) {

        jpeg_destroy_decompress(&cinfo);
        return -1;
    }

    // Request RGB output from the JPEG decoder.
    cinfo.out_color_space = JCS_RGB;

    // Start the JPEG decompression process.
    jpeg_start_decompress(&cinfo);

    // Get the image dimensions.
    *width = cinfo.output_width;
    *height = cinfo.output_height;

    // Get the number of bytes per pixel.
    int channels = cinfo.output_components;

    printf("the channels =%d\n", channels);

    // Allocate memory for the decoded RGB image.
    *rgb_data = malloc((*width) * (*height) * 3);

    if (*rgb_data == NULL) {

        jpeg_destroy_decompress(&cinfo);
        return -1;
    }

    // Decode the JPEG image one row at a time.
    while (cinfo.output_scanline < cinfo.output_height) {

        // Calculate the address of the current row.
        unsigned char *row =*rgb_data + cinfo.output_scanline * (*width) * 3;

        JSAMPROW row_pointer[1];

        row_pointer[0] = row;

        // Decode one scanline into the RGB buffer.
        jpeg_read_scanlines(&cinfo, row_pointer, 1);
    }

    // Finish the JPEG decompression.
    jpeg_finish_decompress(&cinfo);

    jpeg_destroy_decompress(&cinfo);

    return 0;
}

int main(void)
{
    int server_fd;
    int client_fd;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    socklen_t client_len = sizeof(client_addr);

    // Create a TCP socket for the receiver.
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {

        perror("socket");
        return 1;
    }

    // Allow the server to reuse the same port after restart.
    int opt = 1;

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Initialize the server address structure.
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;

    // Accept connections from any network interface.
    server_addr.sin_addr.s_addr = INADDR_ANY;

    server_addr.sin_port = htons(PORT);

    // Bind the socket to port 5000.
    if (bind(server_fd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) == -1) {

        perror("bind");
        close(server_fd);
        return 1;
    }

    // Put the socket into listening mode.
    if (listen(server_fd, 1) == -1) {

        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("Waiting for Raspberry Pi on port %d...\n", PORT);

    // Accept the TCP connection from the Pi.
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr,&client_len);

    if (client_fd == -1) {

        perror("accept");
        close(server_fd);
        return 1;
    }

    printf("Raspberry Pi connected\n");

    // Initialize SDL video subsystem.
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {

        printf("SDL_Init failed: %s\n", SDL_GetError());

        close(client_fd);
        close(server_fd);

        return 1;
    }

    // Create a window for displaying the camera frames.
    SDL_Window *window =SDL_CreateWindow("Raspberry Pi Camera",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,1280, 720,SDL_WINDOW_SHOWN);

    if (window == NULL) {

        printf("SDL_CreateWindow failed: %s\n", SDL_GetError());

        SDL_Quit();

        close(client_fd);
        close(server_fd);

        return 1;
    }

    // Create an SDL renderer.
    SDL_Renderer *renderer =SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (renderer == NULL) {

        printf("SDL_CreateRenderer failed: %s\n", SDL_GetError());

        SDL_DestroyWindow(window);
        SDL_Quit();

        close(client_fd);
        close(server_fd);

        return 1;
    }

    // Buffer used to store the compressed JPEG frame.
    unsigned char *jpeg_data = NULL;

    // Receive and display frames continuously.
    while (1) {

        uint32_t network_size;

        // Receive the 4-byte frame size.
        if (recv_all(client_fd, &network_size,
                     sizeof(network_size)) == -1) {

            printf("Connection closed\n");
            break;
        }

        // Convert frame size from network byte order to host byte order.
        uint32_t frame_size = ntohl(network_size);

        printf("Receiving JPEG: %u bytes\n", frame_size);

        // Check the received frame size.
        if (frame_size == 0 || frame_size > 10 * 1024 * 1024) {

            printf("Invalid frame size\n");
            break;
        }

        // Allocate or resize memory for the JPEG frame.
        jpeg_data = realloc(jpeg_data, frame_size);

        if (jpeg_data == NULL) {

            printf("Memory allocation failed\n");
            break;
        }

        // Receive the complete JPEG frame.
        if (recv_all(client_fd, jpeg_data, frame_size) == -1) {

            printf("Failed to receive frame\n");
            break;
        }

        unsigned char *rgb_data = NULL;

        int width;
        int height;

        // Decode the JPEG frame into RGB format.
        if (decode_jpeg(jpeg_data, frame_size,
                        &rgb_data, &width, &height) == -1) {

            printf("JPEG decode failed\n");
            continue;
        }

        // Create an SDL texture for the decoded RGB image.
        SDL_Texture *texture =SDL_CreateTexture(renderer,SDL_PIXELFORMAT_RGB24,SDL_TEXTUREACCESS_STREAMING,width, height);

        if (texture == NULL) {

            printf("Texture creation failed: %s\n", SDL_GetError());

            free(rgb_data);
            continue;
        }

        // Copy the RGB frame into the SDL texture.
        SDL_UpdateTexture(texture, NULL, rgb_data, width * 3);

        // Clear the previous frame.
        SDL_RenderClear(renderer);

        // Copy the texture to the window.
        SDL_RenderCopy(renderer, texture, NULL, NULL);

        // Display the rendered frame.
        SDL_RenderPresent(renderer);

        // Destroy the texture after displaying the frame.
        SDL_DestroyTexture(texture);

        free(rgb_data);

        // Check for SDL events such as closing the window.
        SDL_Event event;

        while (SDL_PollEvent(&event)) {

            if (event.type == SDL_QUIT)
                goto cleanup;
        }
    }

cleanup:

    // Release JPEG buffer and network resources.
    free(jpeg_data);

    close(client_fd);
    close(server_fd);

    // Release SDL resources.
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}

