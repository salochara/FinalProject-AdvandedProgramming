/*
   Definition of an image data structure for PGM format images
   Uses typedef, struct
   Uses functions to read and write a file in PGM format, described here:
    http://netpbm.sourceforge.net/doc/pgm.html
    http://rosettacode.org/wiki/Bitmap/Write_a_PGM_file#C

   Gilberto Echeverria
   gilecheverria@yahoo.com
   06/10/2016
*/

#include "pgm_image.h"

//// FUNCTION DECLARATIONS

// Generate space to store the image in memory
void allocateImage(image_t * image)
{
    int i;

    // printf("\nGetting memory for the pixels\n");
    // printf("Size: %d x %d = %d\n", image->width, image->height, image->width * image->height);

    // Allocate memory for the pointers to the rows
    image->pixels = (pixel_t **) malloc(image->height * sizeof (pixel_t *));
    // Validate that the memory was assigned
    if (image->pixels == NULL)
    {
        printf("Error: Could not allocate memory for the matrix rows!\n");
        exit(EXIT_FAILURE);
    }

    // Allocate the memory for the whole image in a single block.
    // This will make the memory contiguous.
    image->pixels[0] = (pixel_t *) malloc(image->width * image->height * sizeof (pixel_t));
    // Validate that the memory was assigned
    if (image->pixels[0] == NULL)
    {
        printf("Error: Could not allocate memory for the matrix!\n");
        exit(EXIT_FAILURE);
    }

    // Assign the pointers to the rows
    for (i=0; i<image->height; i++)
    {
        image->pixels[i] = image->pixels[0] + image->width * i;
    }

    // printf("Done!\n");
}

// Release the dynamic memory used by an image
void freeImage(image_t * image)
{
    // printf("\nReleasing the memory for the pixels\n");

    // Free the memory where the data is stored
    free( image->pixels[0] );
    // Free the array for the row pointers
    free(image->pixels);
    // Set safe values for the variables
    image->pixels = NULL;
    image->width = 0;
    image->height = 0;
    // printf("Done!\n");
}

// Copy an image to another structure
void copyPGM(const pgm_t * source, pgm_t * destination)
{
    if (source->image.width == destination->image.width && source->image.height == destination->image.height)
    {
        memcpy( *(destination->image.pixels), *(source->image.pixels), source->image.width * source->image.height);
    }
    else
    {
        printf("Error: The matrices to be copied are of different sizes!\n");
    }
}

// Read the contents of a text file and store them in an image structure
void readPGMFile(const char * filename, pgm_t * pgm_image)
{
    FILE * file_ptr = NULL;

    // printf("\nReading file: '%s'\n", filename);
    // Open the file
    file_ptr = fopen(filename, "r");
    if (file_ptr == NULL)
    {
        printf("Error: Unable to open file '%s'\n", filename);
        exit(EXIT_FAILURE);
    }

    // Read the header
    readPGMHeader(pgm_image, file_ptr);

    // Allocate the memory for the array of pixels
    allocateImage( &(pgm_image->image) );

    // Read the data acording to the type
    if ( !strncmp(pgm_image->magic_number, "P2", 3) )
    {
        readPGMTextData(pgm_image, file_ptr);
    }
    else if ( !strncmp(pgm_image->magic_number, "P5", 3) )
    {
        readPGMBinaryData(pgm_image, file_ptr);
    }
    else
    {
        printf("Invalid file format. Unknown type: %s\n", pgm_image->magic_number);
        exit(EXIT_FAILURE);
    }

    // Close the file
    fclose(file_ptr);
    // printf("Done!\n");
}

// Read the first lines of the file and store them in the correct fields in the structure
void readPGMHeader(pgm_t * pgm_image, FILE * file_ptr)
{
    char line[LINE_SIZE];

    // Get the type of PGM file
    inputString(line, LINE_SIZE, file_ptr);
    sscanf(line, "%s", pgm_image->magic_number);

    // Read the width and height of the image
    inputString(line, LINE_SIZE, file_ptr);
    // Ignore the line if it has a comment
    if (line[0] == '#')
    {
        inputString(line, LINE_SIZE, file_ptr);
    }
    sscanf(line, "%d %d", &pgm_image->image.width, &pgm_image->image.height);

    // Read the maximum value used in the image format
    inputString(line, LINE_SIZE, file_ptr);
    sscanf(line, "%d", &pgm_image->max_value);
}

// Read data in plain text format (PGM P2)
void readPGMTextData(pgm_t * pgm_image, FILE * file_ptr)
{
    // Read the data for the pixels
    for (int i=0; i<pgm_image->image.height; i++)
    {
        for (int j=0; j<pgm_image->image.width; j++)
        {
            // Read the value for the pixel
            fscanf(file_ptr, "%hhu", &(pgm_image->image.pixels[i][j].value));
        }
    }
}

// Read data in binary format (PGM P5)
void readPGMBinaryData(pgm_t * pgm_image, FILE * file_ptr)
{
    // Count the number of pixels
    size_t pixels = pgm_image->image.height * pgm_image->image.width;

    // Read all the data in one go into the contiguous array
    if ( ! fread(pgm_image->image.pixels[0], sizeof (unsigned char), pixels, file_ptr) )
    {
        perror("Unable to read data from the PGM file\n");
        exit(EXIT_FAILURE);
    }
}

// Write the data in the image structure into a new PGM file
// Receive a pointer to the image, to avoid having to copy the data
void writePGMFile(const char * filename, const pgm_t * pgm_image)
{
    FILE * file_ptr = NULL;

    //printf("\nWriting file: '%s'\n", filename);
    // Open the file
    file_ptr = fopen(filename, "w");
    if (file_ptr == NULL)
    {
        printf("Error: Unable to open file '%s'\n", filename);
        exit(EXIT_FAILURE);
    }

    // Write the header for the file
    fprintf(file_ptr, "%s\n", pgm_image->magic_number);
    fprintf(file_ptr, "# %s\n", filename);
    fprintf(file_ptr, "%d %d\n", pgm_image->image.width, pgm_image->image.height);
    fprintf(file_ptr, "%d\n", pgm_image->max_value);
    // Write the data acording to the type
    if ( !strncmp(pgm_image->magic_number, "P2", 3) )
    {
        writePGMTextData(pgm_image, file_ptr);
    }
    else if ( !strncmp(pgm_image->magic_number, "P5", 3) )
    {
        writePGMBinaryData(pgm_image, file_ptr);
    }
    else
    {
        printf("Invalid file format. Unknown type: %s\n", pgm_image->magic_number);
        exit(EXIT_FAILURE);
    }
    fclose(file_ptr);
    //printf("Done!\n");
}

// Write the data in the image structure into a new PGM file
// Receive a pointer to the image, to avoid having to copy the data
void writePGMTextData(const pgm_t * pgm_image, FILE * file_ptr)
{
    // Write the pixel data
    for (int i=0; i<pgm_image->image.height; i++)
    {
        for (int j=0; j<pgm_image->image.width; j++)
        {
            fprintf(file_ptr, "%d", pgm_image->image.pixels[i][j].value);
            // Separate pixels in the same row with tabs
            if (j < pgm_image->image.width-1)
                fprintf(file_ptr, "\t");
            else
                fprintf(file_ptr, "\n");
        }
    }
}

// Write the data in the image structure into a new PGM file
// Receive a pointer to the image, to avoid having to copy the data
void writePGMBinaryData(const pgm_t * pgm_image, FILE * file_ptr)
{
    // Write the pixel data
    size_t pixels = pgm_image->image.height * pgm_image->image.width;

    // Write the binary data from the array to the file
    fwrite(pgm_image->image.pixels[0], sizeof (unsigned char), pixels, file_ptr);
}

// Invert the colors in the image
// Since a PGM format will only use integers for the encoding,
//  the negative is obtained by substracting each color component
//  from the maximum value defined in the PGM header
void negativePGM(pgm_t * pgm_image)
{
    printf("\nInverting image\n");
    for (int i=0; i<pgm_image->image.height; i++)
    {
        for (int j=0; j<pgm_image->image.width; j++)
        {
            // Substract the current value from the maximum value
            pgm_image->image.pixels[i][j].value = pgm_image->max_value - pgm_image->image.pixels[i][j].value;
        }
    }
    printf("Done!\n");
}

// Apply a blur algorithm to the image
void blurPGM(pgm_t * pgm_image, int radius)
{
    printf("\nBlurring an image with radius %d\n", radius);
    unsigned int total_value;
    int i;
    int j;
    int rx;
    int ry;
    int x;
    int y;
    int counter;

    // Create a temporary image
    pgm_t image_copy;

    // Initialize the new image
    image_copy.image.height = pgm_image->image.height;
    image_copy.image.width = pgm_image->image.width;
    allocateImage(&image_copy.image);

    // Loops for the pixels in the image
    for (i=0; i<pgm_image->image.height; i++)
    {
        for (j=0; j<pgm_image->image.width; j++)
        {
            // Initialize the final colors
            total_value = 0;
            // Initialize the average counter
            counter = 0;
            // Inner loops for the averaging window
            for (ry=-radius; ry<=radius; ry++)
            {
                y = i + ry;
                // Do not use pixels outside the image range
                if (y < 0)
                { continue; }
                if (y > (pgm_image->image.height - 1))
                { break; }
                // Loop for the columns, using a circular radius
                for (rx=-radius+abs(ry); rx<=radius-abs(ry); rx++)
                {
                    x = j + rx;
                    // Do not use pixels outside the image range
                    if (x < 0)
                    { continue; }
                    if (x > (pgm_image->image.width - 1))
                    { break; }
                    total_value += pgm_image->image.pixels[y][x].value;
                    counter++;
                }
            }

            // Average the colors in the copy
            image_copy.image.pixels[i][j].value = total_value / counter;
        }
    }
    printf("Done!\n");

    // Copy the new image back to the initial structure
    copyPGM(&image_copy, pgm_image);

    // Release the memory in the temporary image
    freeImage(&image_copy.image);
}

// Convert an image into ASCII art
// Color scales from:
//  http://paulbourke.net/dataformats/asciiart/
// Converts two pixels vertically to a single character
void asciiArtPGM(pgm_t * pgm_image, const char * out_filename)
{
    FILE * ascii_file = NULL;
    int total;
    // Array with the characters to use for 10 levels of grey
    char grey_levels[] = " .:-=+*#%@";
    int max_levels = strlen(grey_levels);
    // Get the range of each level of grey
    int grey_increment = pgm_image->max_value / max_levels;
    int level;
    int grey;

    // Open the text file
    ascii_file = fopen(out_filename, "w");
    if (!ascii_file)
    {
        fprintf(stderr, "ERROR: fopen file '%s'\n", out_filename);
        exit(EXIT_FAILURE);
    }

    printf("\nConverting to ASCII image\n");

    // Move two pixels down each time
    for (int i=0; i<pgm_image->image.height; i+=2)
    {
        for (int j=0; j<pgm_image->image.width; j++)
        {
            // Add the values for the pixel and the one below
            total = pgm_image->image.pixels[i][j].value + pgm_image->image.pixels[i+1][j].value;
            // Average them
            total /= 2;

            // Get the scale of grey in 10 levels
            level = 0;
            grey = pgm_image->max_value;
            while (grey > total)
            {
                grey -= grey_increment;
                level++;
            }
            // Avoid going over the maximum level
            if (level > max_levels - 1)
            {
                //printf("SOME OVERFLOW HERE! [%d, %d]\n", i, j);
                level = max_levels - 1;
            }
            // Print the correct character
            fprintf(ascii_file, "%c", grey_levels[level]);
        }
        fprintf(ascii_file, "\n");
    }
    printf("Writing file: '%s'\n", out_filename);
    printf("Done!\n");

    fclose(ascii_file);
}
