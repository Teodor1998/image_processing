#include "homework1.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

int num_threads;
int resolution;

void initialize(image *im) {
    im->pic = (unsigned char**)calloc(resolution, sizeof(unsigned char*));
    int i;
    for (i = 0; i < resolution; ++i) {
        im->pic[i] = (unsigned char*)calloc(resolution, sizeof(unsigned char));
    }
}

double dist(double j, double i) {
    // first I'll generate the coordinates in cm
    
    double x = 100 * (j + 0.5) / (resolution - 1);
    double y = 100 * (i + 0.5) / (resolution - 1);

    double d = abs((-1 * x) + (2 * y)) / sqrt((1.0 * 1.0) + (2.0 * 2.0));

    return d;
}

void* parallel_for(void* var) {
    routine_args* args = (routine_args*) var;
    int from = args->thread_id * args->each;
    int to;
    if (args->thread_id == num_threads - 1) {
        to = resolution;
    } else {
        to = from + args->each;
    }

    int i, j;
    // i = the line number
    for (i = from; i < to; ++i) {
        // j - the column number
        for (j = 0; j < resolution; ++j) {
            if (dist((double)j, (double)i) <= 3.0) {
                args->img->pic[resolution - 1 -i][j] = 0; //black
            } else {
                args->img->pic[resolution - 1 -i][j] = 255; //white
            }
        }
    }
    return NULL;
}

void render(image *im) {
    int i;
    pthread_t *tid = (pthread_t*)calloc(num_threads, sizeof(pthread_t));
    // Args that will be sent to the thread's routine
    routine_args *thread_args = (routine_args*)calloc(num_threads, sizeof(routine_args));
    int each = resolution / num_threads;
    for (i = 0; i < num_threads; ++i) {
        thread_args[i].thread_id = i;
        thread_args[i].img = im;
        thread_args[i].each = each;
    }
    for (i = 0; i < num_threads; ++i) {
        pthread_create(&(tid[i]), NULL, parallel_for, &(thread_args[i]));
    }
    for (i = 0; i < num_threads; ++i) {
		pthread_join(tid[i], NULL);
    }
    free(tid);
    free(thread_args);
}

void writeData(const char * fileName, image *img) {
    // Writing the header (text part)
    FILE *out = fopen(fileName, "w");
    fprintf(out, "P%d\n%d %d\n%d\n", 5, resolution, resolution, 255);
    fclose(out);

    // Writing the image itself (the binary part)
    out = fopen(fileName, "ab");
    int i;
    for (i = 0; i < resolution; ++i) {
        fwrite(img->pic[i], sizeof(unsigned char), resolution, out);
    }
    fclose(out);
}
