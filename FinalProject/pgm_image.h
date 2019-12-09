/*
   Definition of an image data structure for PGM format images
   Uses typedef, struct
   Uses functions to read and write a file in PGM format, described here:
    http://netpbm.sourceforge.net/doc/pgm.html
    http://rosettacode.org/wiki/Bitmap/Write_a_PGM_file#C

   Gilberto Echeverria
   gilecheverria@yahoo.com
   04/15/2017
*/

#ifndef PGM_IMAGE_H
#define PGM_IMAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "string_functions.h"
#include "pgm_image.h"

// Constant for the size of strings to be read from a PGM file header
#define LINE_SIZE 255

//// TYPE DECLARATIONS


// Structure for a pixel color information, using a single grayscale value in the range 0-255
typedef struct pixel_struct
{
    unsigned char value;
} pixel_t;

// Structure to store full image data of any size
typedef struct image_struct
{
    int width;
    int height;
    pixel_t ** pixels;
} image_t;

// Structure for an image in PGM format
typedef struct pgm_struct
{
    char magic_number[3];           // String for the code indicating the type of PGM file
    int max_value;                  // Maximum value for pixel data in each component
    image_t image;
} pgm_t;

// Structure to pass information to the threads
typedef struct start_stop_struct {
    pgm_t * pgm_image;
    pgm_t * pgm_new_image;
    unsigned int start;
    unsigned int stop;
} start_stop_t;

//// FUNCTION PROTOTYPES
void allocateImage(image_t * image);
void freeImage(image_t * image);
void copyPGM(const pgm_t * source, pgm_t * destination);
void readPGMFile(const char * filename, pgm_t * pgm_image);
void readPGMHeader(pgm_t * pgm_image, FILE * file_ptr);
void readPGMTextData(pgm_t * pgm_image, FILE * file_ptr);
void readPGMBinaryData(pgm_t * pgm_image, FILE * file_ptr);
void writePGMFile(const char * filename, const pgm_t * pgm_image);
void writePGMTextData(const pgm_t * pgm_image, FILE * file_ptr);
void writePGMBinaryData(const pgm_t * pgm_image, FILE * file_ptr);
void negativePGM(pgm_t * pgm_image);
void greyscalePGM(pgm_t * pgm_image);
void blurPGM(pgm_t * pgm_image, int radius);
void asciiArtPGM(pgm_t * pgm_image, const char * out_filename);

#endif
