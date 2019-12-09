//
// Created by Salomón Charabati on 08/12/19.
//

#ifndef ADVANCEDPROGRAMMING_GAME_OF_LIFE_OMP_H
#define ADVANCEDPROGRAMMING_GAME_OF_LIFE_OMP_H

#include <stdio.h>
#include <stdlib.h>
#include "pgm_image.h"
#define MAX_STRING_SIZE 50

void iterationsOfGameOfLife(pgm_t *pgm_image, int iterations, char *output_file_name);
void changePGMImage(pgm_t * pgm_image, pgm_t * new_pgm_image);
void strip_ext(char * file_name);


#endif //ADVANCEDPROGRAMMING_GAME_OF_LIFE_OMP_H
