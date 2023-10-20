# ASCII PNG

This project allows the user to encode an ASCII message within a PNG file without visually alterating the image. This project also allows the user to decode the message at a later time and is capable of detecting errors.

## Build

```bash
$ make clean
$ make
```

## Encoder Usage

```bash
$ ./bin/encoder <input.png> [output.png]
- (Required) Path to the input PNG file.
- (Optional) Path to the output PNG file where message will be encoded. If none provided, output.png will be used.
```

```bash
$ ./bin/encoder image.png
Message to encode: STOP DOING COMPUTER SCIENCE
```

## Decoder Usage

```bash
$ ./bin/decoder <input.png>
- (Required) Path to input PNG file with message encoded.
```

```bash
$ ./bin/decoder output.png
Decoded message: STOP DOING COMPUTER SCIENCE
```

## Encoding Method

The payload consists of a fixed 32 bit message signature, a 24 bit message length, an 8 bit message length checksum, a variable length message, and a 32 bit message checksum. The message signature is included to tell the decoder that a message is present, the message length is included to delimit (specify the boundaries of) the message content, and the checksums are included for data redundancy.

```
Payload (Conceptual Representation):

[Message Signature] [Message Length] [Length Checksum] [Message Content] [Message Checksum]
+-----------------+ +--------------+ +---------------+ +---------------+ +----------------+
| Signature Bits  | | Length Bits  | | Checksum Bits | | Content Bits  | | Checksum Bits  |
| (32 bits)       | | (24 bits)    | | (8 bits)      | | (Variable)    | | (32 bits)      |
+-----------------+ +--------------+ +---------------+ +---------------+ +----------------+
```

The payload is then split into 2-bit "crumbs" and embedded into the two least significant bits (LSBs) of each channel in an RGBA image. Therefore, one byte (8 bits) can be embedded in each pixel without visually altering the image.

### Limitations

With this specification the message is limited to 2<sup>24</sup> (16,777,216) characters given an adequate size image (~17 megapixels) due to the message length field only being 24 bits. Although this could be easily increased by making message length a standard 32-bit integer, this was an intentional decision to avoid undefined behavior at large message sizes.

Theoretically, this specification could be extended to utilize the maximum number of pixels allowed by the PNG standard (2^31 - 1) with careful consideration of edge cases.

With smaller images, the message is limited to (width * height) - 12 characters due to the size of the image.

## Data Redundancy

Checksums for message length and message content were included to allow the decoder to detect errors. Both checksums are sums of each 2-bit "crumb" belonging to the corresponding field. The decoder calculates an expected checksum for the decoded message length and message. If the expected checksum doesn't match the decoded checksum, an error is present.

### Limitations

This technique can only detect errors, but cannot correct them. A more advanced technique such as Hamming codes would be required to correct errors.

## Dependencies

This project uses LodePNG to decode a PNG file into raw pixel data and to encode modified pixel data back to PNG.

LodePNG was chosen for its simplicity and ease of use. LodePNG is lightweight and has no external dependencies, making it easy to incorporate into the project. Additionally, both encoding and decoding can be done with only one function call. As well as that, the data structure used for the raw pixel data lends itself well to the chosen encoding method.

## Target Platform

This project was developed and tested on WSL Ubuntu.

## Possible Improvements

- Extend the encoding specification to utilize the maximum number of pixels allowed by the PNG standard (2^31 - 1).

- Utilize a more advanced data redundancy technique such as Hamming codes to allow the decoder to correct errors.

- Improve overall code structure and readablity by abstracting and reusing code in functions.