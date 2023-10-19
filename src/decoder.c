#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lodepng.h>

int main(int argc, char *argv[]) {
    // Checks for proper usage
    if (argc != 2) {
        printf("Usage: %s input.png\n", argv[0]);
        return 1;
    }

    // Checks if specified files are png
    if (strstr(argv[1], ".png") == NULL) {
        printf("Specified file is not png\n");
        return 1;
    }

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

    // Random unsigned int that acts as a message signature
    const unsigned int defined_message_signature = 2533649507;

    // Enforces standard sizes for each segment
    const size_t message_signature_bit_size = 32;
    const size_t message_length_bit_size = 24;
    const size_t message_length_checksum_bit_size = 8;
    const size_t char_bit_size = 8;
    const size_t message_checksum_bit_size = 32;

    unsigned int extracted_message_signature = 0;
    for (size_t i = 0; i < message_signature_bit_size; i += 2) {
        unsigned char crumb = image[i / 2] & 0b11;
        extracted_message_signature |= (crumb << ((message_signature_bit_size - 2) - i));
    }

    if (extracted_message_signature == defined_message_signature) {
        printf("Message signature found\n");
    } else {
        printf("Message signature not present: no message encoded\n");
        free(image);
        return 1;
    }

    unsigned int extracted_message_length = 0;
    unsigned char expected_message_length_checksum = 0;
    size_t message_length_index = message_signature_bit_size / 2;
    for (size_t i = 0; i < message_length_bit_size; i += 2) {
        unsigned char crumb = image[message_length_index + (i / 2)] & 0b11;
        extracted_message_length |= (crumb << ((message_length_bit_size - 2) - i));
        expected_message_length_checksum += crumb;
    }

    unsigned char extracted_message_length_checksum = 0;
    size_t message_length_checksum_index = message_length_index + (message_length_bit_size / 2);
    for (size_t i = 0; i < message_length_checksum_bit_size; i += 2) {
        unsigned char crumb = image[message_length_checksum_index + (i / 2)] & 0b11;
        extracted_message_length_checksum |= (crumb << ((message_length_checksum_bit_size - 2) - i));
    }

    if (extracted_message_length_checksum == expected_message_length_checksum) {
        printf("Message length successfully decoded (%u)\n", extracted_message_length);
    } else {
        printf("Error decoding message length: checksum invalid\n");
        free(image);
        return 1;
    }

    char extracted_message[extracted_message_length + 1];
    extracted_message[extracted_message_length] = '\0';

    unsigned int expected_message_checksum = 0;
    size_t message_index = message_length_checksum_index + (message_length_checksum_bit_size / 2);
    for (size_t i = 0; i < extracted_message_length; i++) {
        char extracted_char = 0;
        for (size_t j = 0; j < char_bit_size; j += 2) {
            unsigned char crumb = image[message_index + (((i * char_bit_size) + j) / 2)] & 0b11;
            extracted_char |= (crumb << ((char_bit_size - 2) - j));
            expected_message_checksum += crumb;
        }
        extracted_message[i] = extracted_char;
    }
    printf("%s\n%u\n", extracted_message, expected_message_checksum);

    unsigned char extracted_message_checksum = 0;
    size_t message_checksum_index = message_index + ((char_bit_size * extracted_message_length) / 2);
    for (size_t i = 0; i < message_checksum_bit_size; i += 2) {
        unsigned char crumb = image[message_checksum_index + (i / 2)] & 0b11;
        extracted_message_checksum |= (crumb << ((message_checksum_bit_size - 2) - i));
    }

    if (extracted_message_checksum == expected_message_checksum) {
        printf("Message successfully decoded:\n%s\n", extracted_message);
    } else {
        printf("Error decoding message: checksum invalid\n%u\n", extracted_message_checksum);
    }

    free(image);
    return 0;
}