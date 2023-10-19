#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lodepng.h>

int main(int argc, char *argv[]) { 
    // Checks for proper usage
    if (argc < 2 || argc > 3) {
        printf("Usage: %s input.png [output.png]\n", argv[0]);
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

    const unsigned int message_indicator = 2533649507;
    const unsigned int message_length = (unsigned int)strlen(input);

    size_t message_indicator_bit_size = sizeof(message_indicator) * 8;
    size_t message_length_bit_size = sizeof(message_length) * 8;
    size_t char_bit_size = sizeof(char) * 8;

    // Array large enough for message indicator, message length, and message split into 2 bit elements (crumbs)
    unsigned char message_crumbs[(message_indicator_bit_size / 2) + (message_length_bit_size / 2) + ((char_bit_size * strlen(input)) / 2)];

    // Adds message_indicator crumbs to message_crumbs
    for (size_t i = 0; i < message_indicator_bit_size; i += 2) {
        message_crumbs[i / 2] = (message_indicator >> ((message_indicator_bit_size - 2) - i)) & 0b11;
    }

    // Adds message_length crumbs to message_crumbs
    size_t message_length_index = message_indicator_bit_size / 2;
    for (size_t i = 0; i < message_length_bit_size; i += 2) {
        message_crumbs[message_length_index + (i / 2)] = (message_length >> ((message_length_bit_size - 2) - i)) & 0b11;
    }

    // Adds input crumbs to message_crumbs
    size_t message_index = message_length_index + (message_length_bit_size / 2);
    for (size_t i = 0; i < strlen(input); i++) {
        char curr_char = input[i];
        for (size_t j = 0; j < char_bit_size; j += 2) {
            message_crumbs[message_index + (((i * char_bit_size) + j) / 2)] = (curr_char >> ((char_bit_size - 2) - j)) & 0b11;
        }
    }

    for (size_t i = 0; i < message_indicator_bit_size / 2; i++) {
        printf("%u", (message_crumbs[i] >> 1) & 0b1);
        printf("%u", message_crumbs[i] & 0b1);
    }
    printf("\n");
    for (size_t i = 0; i < message_length_bit_size / 2; i++) {
        printf("%u", (message_crumbs[message_length_index + i] >> 1) & 0b1);
        printf("%u", message_crumbs[message_length_index + i] & 0b1);
    }
    printf("\n");
    for (size_t i = 0; i < char_bit_size * strlen(input) / 2; i++) {
        printf("%u", (message_crumbs[message_index + i] >> 1) & 0b1);
        printf("%u", message_crumbs[message_index + i] & 0b1);
    }
    printf("\n");

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

    // Embed in image

    // Encodes raw pixels to png
    error = lodepng_encode32_file(argc == 3 ? argv[2] : "output.png", image, width, height);

    if(error) {
        printf("Error %u: %s\n", error, lodepng_error_text(error));
        free(image);
        return 1;
    }

    free(image);
}