#ifndef HOMEWORK_H
#define HOMEWORK_H

// The pixel of a colored image
typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
}colors;

// The image structure
typedef struct {
    int P; // 5 or 6
    int width, height;
    int maxval;

    // the colors** type is for rgb images
    colors** colored_pic;

    // the unsigned char is for black & white images
    unsigned char** bw_pic;
}image;

// Argument structure for the thread routine
typedef struct {
    int thread_id;
    image* in;
    image* out;
    int each;
}routine_args;

void readInput(const char * fileName, image *img);

void writeData(const char * fileName, image *img);

void resize(image *in, image * out);

#endif /* HOMEWORK_H */