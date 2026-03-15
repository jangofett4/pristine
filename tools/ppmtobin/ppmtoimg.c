/*
 * Pristine
 * ppmtoimg: convert PPM to compact binary
 * SPDX-License-Identifier MIT
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s input-ppm output-file\n", argv[0]);
        return 1;
    }

    FILE *ppmfile = fopen(argv[1], "rb");
    if (!ppmfile) {
        fprintf(stderr, "Error: Unable to open file '%s'!\n", argv[1]);
        perror("fopen");
        return 1;
    }

    FILE *outputfile = fopen(argv[2],"wb+");
    
    int read = 0;
    uint8_t data[3];
    if ((read = fread(data, 1, 3, ppmfile)) < 0 || read < 3) {
        fprintf(stderr, "Error: Unable to read file '%s'!\n", argv[1]);
        perror("fread");
        return 1;
    }
    if (data[0] != 'P' && data[1] != '6' && data[1] != 0x0A) {
        fprintf(stderr, "Error: File doesn't have a valid PPM signature '%s'!\n", argv[1]);
        return 1;
    }

    long w = 0;
    long h = 0;
    long c = 0;
    long r = -1, g = -1, b = -1;
    char data0;
    while ((read = fread(&data0, 1, 1, ppmfile)) > 0) {
        if (data0 == '#') {
            while ((read = fread(&data0, 1, 1, ppmfile)) > 0 && data0 != 0x0A);
        }
        if (w == 0) {
            fseek(ppmfile, -1, SEEK_CUR);
            if (!fscanf(ppmfile, "%li ", &w)) {
                fprintf(stderr, "Error: unable to read width!\n");
                return 1;
            }
            printf("Width: %li\n", w);
            fwrite(&w, sizeof(uint32_t), 1, outputfile);
            continue;
        }
        if (h == 0) {
            fseek(ppmfile, -1, SEEK_CUR);
            if (!fscanf(ppmfile, "%li\n", &h)) {
                fprintf(stderr, "Error: unable to read height!\n");
                return 1;
            }
            printf("Height: %li\n", h);
            fwrite(&h, sizeof(uint32_t), 1, outputfile);
            continue;
        }
        if (c == 0) {
            fseek(ppmfile, -1, SEEK_CUR);
            if (!fscanf(ppmfile, "%li\n", &c)) {
                fprintf(stderr, "Error: unable to read color size!\n");
                return 1;
            }
            printf("Color size: %li\n", c);
            continue;
        }
        fwrite(&data0, 1, 1, outputfile);
    }
    
    fclose(ppmfile);
    fclose(outputfile);
    return 0;
}