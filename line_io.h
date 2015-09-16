/*------------------------------------------------------
 *
 *  line_io.h
 *  LyreBird
 *
 *  Created by Armour on 15/09/2015
 *  Copyright (c) 2015 Armour. All rights reserved.
 *
 *------------------------------------------------------
 */

#ifndef LYREBIRD_LINE_IO_H
#define LYREBIRD_LINE_IO_H

#include <stdio.h>
#include <stdlib.h>

#define TWEETS_MAX_LENGTH 160

/* Get one line input from file */
char *input_line(FILE *fin, int *len);

/* Output one line of characters to file */
void output_line(FILE *fout, char *tweet, int len);

#endif
