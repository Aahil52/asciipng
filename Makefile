# Compiler and compiler flags
CC = gcc
CFLAGS = -Wall -I./include

# Source files
DECODER_SRC = src/decoder.c
ENCODER_SRC = src/encoder.c
LODEPNG_SRC = lib/lodepng.c

# Object files
DECODER_OBJ = obj/decoder.o
ENCODER_OBJ = obj/encoder.o
LODEPNG_OBJ = obj/lodepng.o

# Executables
DECODER = bin/decoder
ENCODER = bin/encoder

all: $(DECODER) $(ENCODER)

$(DECODER): $(DECODER_OBJ) $(LODEPNG_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(ENCODER): $(ENCODER_OBJ) $(LODEPNG_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(DECODER_OBJ): $(DECODER_SRC)
	$(CC) $(CFLAGS) -o $@ -c $<

$(ENCODER_OBJ): $(ENCODER_SRC)
	$(CC) $(CFLAGS) -o $@ -c $<

$(LODEPNG_OBJ): $(LODEPNG_SRC)
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(DECODER) $(ENCODER)
	rm -f $(DECODER_OBJ) $(ENCODER_OBJ) $(LODEPNG_OBJ)
