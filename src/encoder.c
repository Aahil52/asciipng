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

    int input_bits[strlen(input) * 8];

    // Iterates through each char in input
    for (int i = 0; i < strlen(input); i++) {
        char curr_char = input[i];
        // Iterates through each bit in curr_char and adds it to the correct index in input_bits
        for (int j = 0; j < 8; j++) {
            int bit = (curr_char >> (7 - j)) & 0x01;
            input_bits[(i * 8) + j] = bit;
        }
    }

    for (int i = 0; i < strlen(input); i++) {
        for (int j = 0; j < 8; j++) {
            printf("%i", input_bits[(i * 8) + j]);
        }
        printf("\n");
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