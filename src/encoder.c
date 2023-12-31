#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lodepng.h>

int main(int argc, char *argv[]) { 
    // Checks for proper usage
    if (argc < 2 || argc > 3) {
        printf("Usage: %s <input.png> [output.png]\n", argv[0]);
        return 1;
    }

    // Checks if specified files are png
    if (strstr(argv[1], ".png") == NULL || (argc == 3 && strstr(argv[2], ".png") == NULL)) {
        printf("Specified file(s) are not png(s)\n");
        return 1;
    }

    char *input = NULL;
    size_t input_size = 0;
    ssize_t line_length;

    printf("Message to encode: ");
    line_length = getline(&input, &input_size, stdin);

    if (line_length < 0) {
        printf("Could not read input\n");
        free(input);
        return 1;
    }

    // Removes the trailing newline character
    if (line_length > 0 && input[line_length - 1] == '\n') {
        input[line_length - 1] = '\0';
    }

    // Enforces a limit of 2^24 characters
    if (strlen(input) > 16777216) {
        printf("Message is greater than 16,777,216 characters\n");
        free(input);
        return 1;
    }

    // Random unsigned int that acts as a message signature
    const unsigned int message_signature = 2533649507;
    // Truncates message_length to 3 bytes
    const unsigned int message_length = (unsigned int)strlen(input);

    // Enforces standard sizes for each segment
    const size_t message_signature_bit_size = 32;
    const size_t message_length_bit_size = 24;
    const size_t message_length_checksum_bit_size = 8;
    const size_t char_bit_size = 8;
    const size_t message_checksum_bit_size = 32;

    // Array large enough for message_signature, message_length, message_length_checksum, message, and message_checksum split into 2 bit elements (crumbs)
    unsigned char payload[(message_signature_bit_size / 2) + (message_length_bit_size / 2) + (message_length_checksum_bit_size / 2) + ((char_bit_size * strlen(input)) / 2) + (message_checksum_bit_size / 2)];

    // Adds message_signature crumbs to payload
    for (size_t i = 0; i < message_signature_bit_size; i += 2) {
        payload[i / 2] = (message_signature >> ((message_signature_bit_size - 2) - i)) & 0b11;
    }

    // Adds message_length crumbs to payload
    size_t message_length_index = message_signature_bit_size / 2;
    unsigned char message_length_checksum = 0;
    for (size_t i = 0; i < message_length_bit_size; i += 2) {
        unsigned char crumb = (message_length >> ((message_length_bit_size - 2) - i)) & 0b11;
        payload[message_length_index + (i / 2)] = crumb;
        message_length_checksum += crumb;
    }

    // Adds message_length_checksum crumbs to payload
    size_t message_length_checksum_index = message_length_index + (message_length_bit_size / 2);
    for (size_t i = 0; i < message_length_checksum_bit_size; i += 2) {
        unsigned char crumb = (message_length_checksum >> ((message_length_checksum_bit_size - 2) - i)) & 0b11;
        payload[message_length_checksum_index + (i / 2)] = crumb;
    }

    // Adds input crumbs to payload
    size_t message_index = message_length_checksum_index + (message_length_checksum_bit_size / 2);
    unsigned int message_checksum = 0;
    for (size_t i = 0; i < strlen(input); i++) {
        char curr_char = input[i];
        for (size_t j = 0; j < char_bit_size; j += 2) {
            unsigned char crumb = (curr_char >> ((char_bit_size - 2) - j)) & 0b11;
            payload[message_index + (((i * char_bit_size) + j) / 2)] = crumb;
            message_checksum += crumb;
        }
    }

    // Adds message_checksum crumbs to payload
    size_t message_checksum_index = message_index + ((char_bit_size * strlen(input)) / 2);
    for (size_t i = 0; i < message_checksum_bit_size; i += 2) {
        unsigned char crumb = (message_checksum >> ((message_checksum_bit_size - 2) - i)) & 0b11;
        payload[message_checksum_index + (i / 2)] = crumb;
    }

    free(input);

    unsigned error;
    unsigned char* image = 0;
    unsigned int width, height;

    // Decodes png to raw pixels
    error = lodepng_decode32_file(&image, &width, &height, argv[1]);\

    if(error) {
        printf("Error %u: %s\n", error, lodepng_error_text(error));
        free(image);
        return 1;
    }

    // Enforces message size to fit within image
    if ((sizeof(payload) / sizeof(unsigned char)) / 4 > width * height) {
        printf("Message is too long for specified image\n");
        free(image);
        return 1;
    }

    // Embed payload in image
    for (size_t i = 0; i < (sizeof(payload) / sizeof(unsigned char)); i++) {
        unsigned char new_val = image[i] & 0b11111100;
        new_val |= payload[i];
        image[i] = new_val;
    }

    // Encodes raw pixels to png
    error = lodepng_encode32_file(argc == 3 ? argv[2] : "output.png", image, width, height);

    if(error) {
        printf("Error %u: %s\n", error, lodepng_error_text(error));
        free(image);
        return 1;
    }

    free(image);
    return 0;
}