#include "homework.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int num_threads;
int resize_factor;

void readInput(const char *fileName, image *img) {
    FILE *in;
    in = fopen(fileName, "r");

    // Reading the first three lines
    fscanf(in, "P%d\n", &(img->P));
    fscanf(in, "%d %d", &(img->width), &(img->height));
    fscanf(in, "%d", &(img->maxval));

    fclose(in);

    in = fopen(fileName, "rb");

    // Space allocations for the pixel 2D array (black & white or color):
    if (img->P == 5) {
        img->bw_pic = (unsigned char**)calloc(img->height, sizeof(unsigned char*));
    } else {
        img->colored_pic = (colors**)calloc(img->height, sizeof(colors*));
    }
    int i;
    for (i = 0; i < img->height; ++i) {
        if (img->P == 5) {
            img->bw_pic[i] = (unsigned char*)calloc(img->width, sizeof(unsigned char));
        } else {
            img->colored_pic[i] = (colors*)calloc(img->width, sizeof(colors));
        }
    }

    // Skipping the first 3 lines (they were already read)
    char aux;
    i = 0;
    do {
        fread(&aux, 1, 1, in);
        if (aux == '\n') {
            ++i;
        }
    } while(i != 3);

    // Actual reading of the pixels
    for (i = 0; i < img->height; ++i) {
        if (img->P == 5) {
            fread(img->bw_pic[i], sizeof(unsigned char), img->width, in);
        } else {
            fread(img->colored_pic[i], sizeof(colors), img->width, in);
        }
    }
    fclose(in);
}

void writeData(const char * fileName, image *img) {
    // Writing the header (text part)
    FILE *out = fopen(fileName, "w");
    fprintf(out, "P%d\n%d %d\n%d\n", img->P, img->width, img->height, img->maxval);
    fclose(out);

    // Writing the image itself (the binary part)
    out = fopen(fileName, "ab");
    int i;
    if (img->P == 5) {
        // black and white
        for (i = 0; i < img->height; ++i) {
            fwrite(img->bw_pic[i], sizeof(unsigned char), img->width, out);
        }
    } else {
        // colored
        for (i = 0; i < img->height; ++i) {
            fwrite(img->colored_pic[i], sizeof(colors), img->width, out);
        }
    }
    fclose(out);
}

// Functions to convert multiple pixels to 1 pixel:

// Convert 3x3 black & white pixels to 1
unsigned char bw_3_to_1(unsigned char** pixelsToResize) {
    // Multiplication with the Gaussian Kernel values:
    const unsigned char Kernel[3][3] = {{1, 2, 1}, {2, 4, 2}, {1, 2, 1}};

    int i, j;
    unsigned int sum = 0;
    for (i = 0; i < 3; ++i) {
        for (j = 0; j < 3; ++j) {
            sum += Kernel[i][j] * pixelsToResize[i][j];
        }
    }
    sum /= 16;
    return (unsigned char)sum;
}

// Convert 3x3 colored pixels to 1
colors color_3_to_1(colors** pixelsToResize) {
    const unsigned char Kernel[3][3] = {{1, 2, 1}, {2, 4, 2}, {1, 2, 1}};
    unsigned int sumr = 0;
    unsigned int sumg = 0;
    unsigned int sumb = 0;
    int i, j;
    for (i = 0; i < 3; ++i) {
        for (j = 0; j < 3; ++j) {
            sumr += pixelsToResize[i][j].r * Kernel[i][j];
            sumg += pixelsToResize[i][j].g * Kernel[i][j];
            sumb += pixelsToResize[i][j].b * Kernel[i][j];
        }
    }

    colors sum;
    sum.r = (unsigned char) (sumr / 16);
    sum.g = (unsigned char) (sumg / 16);
    sum.b = (unsigned char) (sumb / 16);

    return sum;
}

// Convert nr x nr black & white pixels to 1 (nr is even)
unsigned char bw_even_to_1(unsigned char** pixelsToResize) {
    int i, j;
    unsigned int sum = 0;
    for (i = 0; i < resize_factor; ++i) {
        for (j = 0; j < resize_factor; ++j) {
            sum += pixelsToResize[i][j];
        }
    }
    sum /= (resize_factor * resize_factor);
    return (unsigned char) sum;
}

// Convert nr x nr colored pixels to 1 (nr is even)
colors color_even_to_1(colors** pixelsToResize) {
    unsigned int sumr = 0;
    unsigned int sumg = 0;
    unsigned int sumb = 0;
    int i, j;
    for (i = 0; i < resize_factor; ++i) {
        for (j = 0; j < resize_factor; ++j) {
            sumr += pixelsToResize[i][j].r;
            sumg += pixelsToResize[i][j].g;
            sumb += pixelsToResize[i][j].b;
        }
    }
    colors sum;
    sum.r = (unsigned char) (sumr / (resize_factor * resize_factor));
    sum.g = (unsigned char) (sumg / (resize_factor * resize_factor));
    sum.b = (unsigned char) (sumb / (resize_factor * resize_factor));

    return sum;
}

// The function each thread will execute
void* parallel_for(void *var) {
    routine_args* args = (routine_args*) var;
    int from = args->thread_id * args->each * resize_factor;
    int to;

    // Num of threads doesn't fit the number of iterations => last one does what remains (more than others)
    if (args->thread_id == num_threads-1) {
        to = args->in->height;
    } else {
        to = from + (args->each * resize_factor);
    }
    int i, j, i2, j2, k1, k2;

    // This thread will modify the out matrix starting from this line:
    i2 = args->thread_id * args->each;

    if (args->out->P == 5) {
        // Matrix that will be reduced to 1 pixel
        unsigned char **pixelsToResize = (unsigned char **)calloc(resize_factor, sizeof(unsigned char*));
        for (i = 0; i < resize_factor; ++i) {
            pixelsToResize[i] = (unsigned char*)calloc(resize_factor, sizeof(unsigned char));
        }

        for (i = from; i < to; i += resize_factor) {
            if (args->in->height - i < resize_factor) {
                // Truncate the last rows that can't make squares
                break;
            }
            j2 = 0;
            for (j = 0; j < args->in->width; j += resize_factor) {
                if (args->in->width - j < resize_factor) {
                    // Truncate the last columns that can't make squares
                    break;
                }
                // Filling in the matrix that will be reduced to a pixel
                for (k1 = 0; k1 < resize_factor; ++k1) {
                    for (k2 = 0; k2 < resize_factor; ++k2) {
                        pixelsToResize[k1][k2] = args->in->bw_pic[i+k1][j+k2];
                    }
                }
                if (resize_factor % 2 == 0) {
                    // if resize_factor is even
                    args->out->bw_pic[i2][j2] = bw_even_to_1(pixelsToResize);
                } else {
                    // if resize_factor is 3
                    args->out->bw_pic[i2][j2] = bw_3_to_1(pixelsToResize);
                }
                ++j2;
            }
            ++i2;
        }

        // Free memory
        for (i = 0; i < resize_factor; ++i) {
            free(pixelsToResize[i]);
        }
        free(pixelsToResize);
    } else {
        // Colored image
        colors **pixelsToResize = (colors **)calloc(resize_factor, sizeof(colors*));
        for (i = 0; i < resize_factor; ++i) {
            pixelsToResize[i] = (colors*)calloc(resize_factor, sizeof(colors));
        }

        for (i = from; i < to; i += resize_factor) {
            if (args->in->height - i < resize_factor) {
                break;
            }
            j2 = 0;
            for (j = 0; j < args->in->width; j += resize_factor) {
                if (args->in->width - j < resize_factor) {
                    break;
                }
                // Filling in the matrix that will be reduced to a pixel
                for (k1 = 0; k1 < resize_factor; ++k1) {
                    for (k2 = 0; k2 < resize_factor; ++k2) {
                        pixelsToResize[k1][k2] = args->in->colored_pic[i+k1][j+k2];
                    }
                }
                if (resize_factor % 2 == 0) {
                    args->out->colored_pic[i2][j2] = color_even_to_1(pixelsToResize);
                } else {
                    args->out->colored_pic[i2][j2] = color_3_to_1(pixelsToResize);
                }
                ++j2;
            }
            ++i2;
        }
        // Free mem
        for (i = 0; i < resize_factor; ++i) {
            free(pixelsToResize[i]);
        }
        free(pixelsToResize);
    }
    return NULL;
}

// Main function for resize
void resize(image *in, image *out) {
    pthread_t *tid = (pthread_t*)calloc(num_threads, sizeof(pthread_t));
    // Args that will be sent to the thread's routine
    routine_args *thread_args = (routine_args*)calloc(num_threads, sizeof(routine_args));
    out->P = in->P;
    out->width = in->width / resize_factor;
    out->height = in->height / resize_factor;
    out->maxval = in->maxval;

    // Iterations executed by each thread (if the thread_number fits perfectly)
    int each = out->height / num_threads;

    int i;
    if (out->P == 5) {
        out->bw_pic = (unsigned char**)calloc(out->height, sizeof(unsigned char*));
        for (i = 0; i < out->height; ++i) {
            out->bw_pic[i] = (unsigned char*)calloc(out->width, sizeof(unsigned char));
        }
    } else {
        out->colored_pic = (colors**)calloc(out->height, sizeof(colors*));
        for (i = 0; i < out->height; ++i) {
            out->colored_pic[i] = (colors*)calloc(out->width, sizeof(colors));
        }
    }
    for (i = 0; i < num_threads; ++i) {
        thread_args[i].thread_id = i;
        thread_args[i].in = in;
        thread_args[i].out = out;
        thread_args[i].each = each;
    }
    for (i = 0; i < num_threads; ++i) {
        pthread_create(&(tid[i]), NULL, parallel_for, &(thread_args[i]));
    }
    for(i = 0; i < num_threads; i++) {
		pthread_join(tid[i], NULL);
	}
    free(tid);
    free(thread_args);
}
