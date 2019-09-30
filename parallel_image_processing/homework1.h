#ifndef HOMEWORK_H1
#define HOMEWORK_H1


// The image structure
typedef struct {
    unsigned char** pic;
}image;

// Argument structure for the thread routine
typedef struct {
    int thread_id;
    image* img;
    int each;
}routine_args;

void initialize(image *im);
void render(image *im);
void writeData(const char * fileName, image *img);

#endif /* HOMEWORK_H1 */