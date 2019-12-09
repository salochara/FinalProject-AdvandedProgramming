// Created by Salomón Charabati
// Game of Life (OMP version) - Program in C
// Assignment #9
// 27-11-2019

#include "game_of_life_omp.h"

void iterationsOfGameOfLife(pgm_t *pgm_image, int iterations, char *output_file_name)
{
    char file[MAX_STRING_SIZE];

    // Initialize a new_pgm image struct
    pgm_t new_pgm_image;
    new_pgm_image.image.height = pgm_image->image.height;
    new_pgm_image.image.width = pgm_image->image.width;
    new_pgm_image.max_value = pgm_image->max_value;
    sprintf(new_pgm_image.magic_number,"%s",pgm_image->magic_number);

    // Allocate memory for the image in the new_image struct
    allocateImage(&(new_pgm_image.image));

    // For loop for the number of iterations.
    // The pointer to the original and new image will change every
    // other iteration
    for (int i = 0; i < iterations ; ++i)
    {
        // Write the file name with 'OMP' at the end. i.e. OMP.
        // Also include the number of iteration
        sprintf(file,"%s_OMP%d.pgm",output_file_name,i+1);

        // Changing of pointers according to the iteration
        if(i % 2 == 0)
        {
            changePGMImage(pgm_image,&new_pgm_image);
            writePGMFile(file,&new_pgm_image);
        }else{
            changePGMImage(&new_pgm_image,pgm_image);
            writePGMFile(file,pgm_image);
        }
    }

    // Free the allocated memory
    freeImage(&new_pgm_image.image);
}
// Function for changing the PGM image according to the Game of Life rules
// The struct passed to the function depend on the iteration where the above function is at.
void changePGMImage(pgm_t * pgm_image, pgm_t * new_pgm_image)
{
    int neighbors;
    int above;
    int left; int right;
    int below;

    /*
                                            Game of Life Rules
            Any live cell with fewer than two live neighbours dies, as if by underpopulation.
            Any live cell with two or three live neighbours lives on to the next generation.
            Any live cell with more than three live neighbours dies, as if by overpopulation.
            Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
    */

    // Using pragma omp to parallelize the for loop
#pragma omp parallel default(none) shared(pgm_image,new_pgm_image) private(neighbors,above,below,right,left)
    {
#pragma omp for
        for (int i = 0; i < pgm_image->image.height; ++i)
        {
            for (int j = 0; j < pgm_image->image.width; ++j)
            {
                neighbors = 0;

                // Get the neighbors from the every pixel.
                // Using modulo to cover the edge pixels from any image
                above = (i - 1 + pgm_image->image.height) % pgm_image->image.height;
                below = (i+1) % pgm_image->image.height;
                left = (j-1 + pgm_image->image.width) % pgm_image->image.width;
                right = (j+1) % pgm_image->image.width;

                if(pgm_image->image.pixels[above][left].value == 1)
                    neighbors++;
                if (pgm_image->image.pixels[above][j].value == 1)
                    neighbors++;
                if(pgm_image->image.pixels[above][right].value == 1)
                    neighbors++;
                if(pgm_image->image.pixels[i][left].value == 1)
                    neighbors++;
                if(pgm_image->image.pixels[i][right].value == 1)
                    neighbors++;
                if(pgm_image->image.pixels[below][left].value == 1)
                    neighbors++;
                if(pgm_image->image.pixels[below][j].value == 1)
                    neighbors++;
                if(pgm_image->image.pixels[below][right].value == 1)
                    neighbors++;

                // Any living cell
                if(pgm_image->image.pixels[i][j].value == 1)
                {
                    if(neighbors < 2)
                    {
                        new_pgm_image->image.pixels[i][j].value = 0;
                    }
                    else if(neighbors == 2 || neighbors == 3)
                    {
                        new_pgm_image->image.pixels[i][j].value = 1;
                    }
                    else if(neighbors > 3)
                    {
                        new_pgm_image->image.pixels[i][j].value = 0;
                    }
                }
                    // Any dead cell
                else
                {
                    if(neighbors == 3)
                    {
                        new_pgm_image->image.pixels[i][j].value = 1;
                    }
                    else
                    {
                        new_pgm_image->image.pixels[i][j].value = 0;
                    }
                }
            }
        }
    }
}


/*
	Remove extensions from filename
	From: https://stackoverflow.com/questions/43163677/how-do-i-strip-a-file-extension-from-a-string-in-c/43163740
*/
void strip_ext(char *file_name)
{
    char *end = file_name + strlen(file_name);

    while (end > file_name && *end != '.' && *end != '\\' && *end != '/') {
        --end;
    }
    if ((end > file_name && *end == '.') &&
        (*(end - 1) != '\\' && *(end - 1) != '/')) {
        *end = '\0';
    }
}
