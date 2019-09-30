#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "mpi.h"

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
    colors* colored_pic;

    // the unsigned char is for black & white images
    unsigned char* bw_pic;
}image;

void readInput(const char * fileName, image *img) {
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
        img->bw_pic = (unsigned char*)calloc((img->height * img->width), sizeof(unsigned char));
    } else {
        img->colored_pic = (colors*)calloc((img->height * img->width), sizeof(colors));
    }
    int i;

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
    for (i = 0; i < (img->height * img->width); ++i) {
        if (img->P == 5) {
            fread(&(img->bw_pic[i]), sizeof(unsigned char), 1, in);
        } else {
            fread(&(img->colored_pic[i]), sizeof(colors), 1, in);
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
        for (i = 0; i < (img->height * img->width); ++i) {
            fwrite(&(img->bw_pic[i]), sizeof(unsigned char), 1, out);
        }
    } else {
        // colored
        for (i = 0; i < (img->height * img->width); ++i) {
            fwrite(&(img->colored_pic[i]), sizeof(colors), 1, out);
        }
    }
    fclose(out);
}

unsigned char filterUseBW(unsigned char pixel[], const float filter[]) {
    float aux[9];
    for (int i = 0; i < 9; ++i) {
        aux[i] = (float)pixel[i] * filter[i];
    }
    float rez = 0;
    for (int i = 0; i < 9; ++i) {
        rez += aux[i];
    }

    return (unsigned char)rez;
}

colors filterUseColored(colors pixel[], const float filter[]) {
    float auxr[9];
    float auxg[9];
    float auxb[9];
    for (int i = 0; i < 9; ++i) {
        auxr[i] = (float)pixel[i].r * filter[i];
        auxg[i] = (float)pixel[i].g * filter[i];
        auxb[i] = (float)pixel[i].b * filter[i];
    }

    float rezr = 0;
    float rezg = 0;
    float rezb = 0;
    for (int i = 0; i < 9; ++i) {
        rezr += auxr[i];
        rezg += auxg[i];
        rezb += auxb[i];
    }
    colors ret;
    ret.r = (unsigned char)rezr;
    ret.g = (unsigned char)rezg;
    ret.b = (unsigned char)rezb;
    return ret;
}

void applyFilter(image *in, image *out, char filter[]) {
    //Filter matrixes:
    const float identity[9] = {0, 0, 0, 0, 1, 0, 0, 0, 0};
    const float smooth[9] = {1.0f/9, 1.0f/9, 1.0f/9, 1.0f/9, 1.0f/9, 1.0f/9, 1.0f/9, 1.0f/9, 1.0f/9};
    const float blur[9] = {0.0625f, 0.125f, 0.0625f, 0.125f, 0.25f, 0.125f, 0.0625f, 0.125f, 0.0625f};
    const float sharpen[9] = {0, -2.0f/3, 0, -2.0f/3, 11.0f/3, -2.0f/3, 0, -2.0f/3, 0};
    const float mean[9] = {-1, -1, -1, -1, 9, -1, -1, -1, -1};
    const float emboss[9] = {0, 1, 0, 0, 0, 0, 0, -1, 0};

    int begin = in->width;
    
    // Memory allocation for the output matrix
    out->height = in->height;
    out->width = in->width;

    unsigned char matbw[9];
    colors matcol[9];

    if (in->P == 5) {
        out->P = 5;
        out->bw_pic = (unsigned char*)calloc((out->height * out->width), sizeof(unsigned char));
    } else {
        out->P = 6;
        out->colored_pic = (colors*)calloc((out->height * out->width), sizeof(colors));
    }

    int i;
    for (i = begin; i < (in->height * in->width) + begin; ++i) {
        // The fist and last element from a row won't be processed
        if ((i % in->width != 0) && ((i + 1) % in->width != 0)) {
            if (out->P == 5) {
                matbw[0] = in->bw_pic[i - in->width - 1];
                matbw[1] = in->bw_pic[i - in->width];
                matbw[2] = in->bw_pic[i - in->width + 1];
                matbw[3] = in->bw_pic[i - 1];
                matbw[4] = in->bw_pic[i];
                matbw[5] = in->bw_pic[i + 1];
                matbw[6] = in->bw_pic[i + in->width - 1];
                matbw[7] = in->bw_pic[i + in->width];
                matbw[8] = in->bw_pic[i + in->width + 1];

                if (strcmp(filter, "smooth") == 0) {
                    out->bw_pic[i - begin] = filterUseBW(matbw, smooth);
                } else if (strcmp(filter, "blur") == 0) {
                    out->bw_pic[i - begin] = filterUseBW(matbw, blur);
                } else if (strcmp(filter, "sharpen") == 0) {
                    out->bw_pic[i - begin] = filterUseBW(matbw, sharpen);
                } else if (strcmp(filter, "mean") == 0) {
                    out->bw_pic[i - begin] = filterUseBW(matbw, mean);
                } else if (strcmp(filter, "emboss") == 0) {
                    out->bw_pic[i - begin] = filterUseBW(matbw, emboss);
                } else if (strcmp(filter, "identity") == 0) {
                    out->bw_pic[i - begin] = filterUseBW(matbw, identity);
                }
            } else {
                matcol[0] = in->colored_pic[i - in->width - 1];
                matcol[1] = in->colored_pic[i - in->width];
                matcol[2] = in->colored_pic[i - in->width + 1];
                matcol[3] = in->colored_pic[i - 1];
                matcol[4] = in->colored_pic[i];
                matcol[5] = in->colored_pic[i + 1];
                matcol[6] = in->colored_pic[i + in->width - 1];
                matcol[7] = in->colored_pic[i + in->width];
                matcol[8] = in->colored_pic[i + in->width + 1];

                if (strcmp(filter, "smooth") == 0) {
                    out->colored_pic[i - begin] = filterUseColored(matcol, smooth);
                } else if (strcmp(filter, "blur") == 0) {
                    out->colored_pic[i - begin] = filterUseColored(matcol, blur);
                } else if (strcmp(filter, "sharpen") == 0) {
                    out->colored_pic[i - begin] = filterUseColored(matcol, sharpen);
                } else if (strcmp(filter, "mean") == 0) {
                    out->colored_pic[i - begin] = filterUseColored(matcol, mean);
                } else if (strcmp(filter, "emboss") == 0) {
                    out->colored_pic[i - begin] = filterUseColored(matcol, emboss);
                } else if (strcmp(filter, "identity") == 0) {
                    out->bw_pic[i - begin] = filterUseBW(matbw, identity);
                }
            }
        } else {
            if (out->P == 5) {
                out->bw_pic[i - begin] = in->bw_pic[i];
            } else {
                out->colored_pic[i - begin] = in->colored_pic[i];
            }
        }
    }
}

int main (int argc, char **argv) {
    int  numtasks, rank;
    MPI_Status Stat;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Build a new MPI Datatype for the colors:
    MPI_Datatype mpi_colors;
    MPI_Type_contiguous(3, MPI_UNSIGNED_CHAR, &mpi_colors);
    MPI_Type_commit(&mpi_colors);


    image in;
    image myout;
    image myin;

    int each;
    int each_h;
    int each_last;

    for (int k2 = 3; k2 < argc; ++k2) {
        if (numtasks == 1) {
            if (k2 == 3) {
                // Read from file only on the first loop
                readInput(argv[1], &in);
            }
            in.height -= 2;
            applyFilter(&in, &myout, argv[k2]);
            in.height += 2;
            for (int i = 0; i < (in.height - 2) * in.width; ++i) {
                if (in.P == 5) {
                    in.bw_pic[i + in.width] = myout.bw_pic[i];
                } else {
                    in.colored_pic[i + in.width] = myout.colored_pic[i];
                }
            }
            if(in.P == 5) {
                free(myout.bw_pic);
            } else {
                free(myout.colored_pic);
            }
            continue;
        }
        if (rank == 0) {
            if (k2 == 3) {
                // Read from file only on the first loop
                readInput(argv[1], &in);
            }

            each_h = (in.height - 2) / (numtasks - 1);
            each = each_h * in.width;
            each_last = each;
            for (int i = 1; i < numtasks; ++i) {
                if (i == numtasks - 1) {
                    each_h = (in.height - 2) - each_h * (numtasks - 2);
                    each_last = each_h * in.width;
                }
                image toSend;
                toSend.height = each_h;
                toSend.width = in.width;
                toSend.P = in.P;
                if (toSend.P == 5) {
                    toSend.bw_pic = (unsigned char*) malloc((each_last + (2 * in.width)) * sizeof(unsigned char));
                    for (int j = each * (i - 1); j < (each * (i - 1)) + each_last + (2 * in.width); ++j) {
                        toSend.bw_pic[j - (each * (i - 1))] = in.bw_pic[j];
                    }
                    int vSend[3];
                    vSend[0] = toSend.P;
                    vSend[1] = toSend.width;
                    vSend[2] = toSend.height;

                    // Send the type of image and the dimensions
                    MPI_Send(vSend, 3, MPI_INT, i, 10, MPI_COMM_WORLD);

                    // Send the image itself to be processed by the other process
                    MPI_Send(toSend.bw_pic, each_last + (2 * in.width), MPI_UNSIGNED_CHAR, i, 10, MPI_COMM_WORLD);
                    free(toSend.bw_pic);
                } else {
                    // Same for colored
                    toSend.colored_pic = (colors*) malloc((each_last + (2 * in.width)) * sizeof(colors));
                    for (int j = each * (i - 1); j < (each * (i - 1)) + each_last + (2 * in.width); ++j) {
                        toSend.colored_pic[j - (each * (i - 1))] = in.colored_pic[j];
                    }
                    int vSend[3];
                    vSend[0] = toSend.P;
                    vSend[1] = toSend.width;
                    vSend[2] = toSend.height;
                    
                    MPI_Send(vSend, 3, MPI_INT, i, 10, MPI_COMM_WORLD);
                    MPI_Send(toSend.colored_pic, each_last + (2 * in.width), mpi_colors, i, 10, MPI_COMM_WORLD);
                    free(toSend.colored_pic);
                }
            }
        } else {
            // All the other processes recieve the image and modify it
            int vRecv[3];
            MPI_Recv(vRecv, 3, MPI_INT, 0, 10, MPI_COMM_WORLD, &Stat);
            myin.P = vRecv[0];
            myin.width = vRecv[1];
            myin.height = vRecv[2];
        
            if (myin.P == 5) {
                myin.bw_pic = (unsigned char*) malloc(((myin.height * myin.width) + (2 * myin.width)) * sizeof(unsigned char));
                MPI_Recv(myin.bw_pic, (myin.height * myin.width) + (2 * myin.width), MPI_UNSIGNED_CHAR, 0, 10, MPI_COMM_WORLD, &Stat);
            } else {
                myin.colored_pic = (colors*) malloc(((myin.height * myin.width) + (2 * myin.width)) * sizeof(colors));
                MPI_Recv(myin.colored_pic, (myin.height * myin.width) + (2 * myin.width), mpi_colors, 0, 10, MPI_COMM_WORLD, &Stat);
            }
        
            applyFilter(&myin, &myout, argv[k2]);

            if (myin.P == 5) {
                free(myin.bw_pic);
                MPI_Send(myout.bw_pic, myin.height * myin.width, MPI_UNSIGNED_CHAR, 0, 10, MPI_COMM_WORLD);
                free(myout.bw_pic);
            } else {
                free(myin.colored_pic);
                MPI_Send(myout.colored_pic, myin.height * myin.width, mpi_colors, 0, 10, MPI_COMM_WORLD);
                free(myout.colored_pic);
            } 
        }

        if (rank == 0) {
            // The root receives the processed images and builds the result of the filter
            image out;
            each_h = (in.height - 2) / (numtasks - 1);
            each = each_h * in.width;
            each_last = each;

            out.P = in.P;
            out.maxval = in.maxval;
            out.width = in.width;
            out.height = in.height;
            int index = 0;
            // Add the first line
            if (in.P == 5) {
                out.bw_pic = (unsigned char*)malloc((out.width * out.height) * sizeof(unsigned char));
                for (int i = 0; i < out.width; ++i) {
                    out.bw_pic[i] = in.bw_pic[i];
                }
                index += out.width;
            } else {
                out.colored_pic = (colors*)malloc((out.width * out.height) * sizeof(colors));
                for (int i = 0; i < out.width; ++i) {
                    out.colored_pic[i] = in.colored_pic[i];
                }
                index += out.width;
            }

            // Recieve and add to the result
            for (int i = 1; i < numtasks; ++i) {
                if (i == numtasks - 1) {
                    each_h = (in.height - 2) - (each_h * (numtasks - 2));
                    each_last = each_h * in.width;
                }
                if (out.P == 5) {
                    MPI_Recv(out.bw_pic + index, each_last, MPI_UNSIGNED_CHAR, i, 10, MPI_COMM_WORLD, &Stat);
                    index += each_last;
                } else {
                    MPI_Recv(out.colored_pic + index, each_last, mpi_colors, i, 10, MPI_COMM_WORLD, &Stat);
                    index += each_last;
                }
            }

            // Add the last line
            for (int i = index; i < in.height * in.width; ++i) {
                if (out.P == 5) {
                    out.bw_pic[i] = in.bw_pic[i];
                } else {
                    out.colored_pic[i] = in.colored_pic[i];
                }
            }
            if (out.P == 5) {
                for (int i = 0; i < out.width * out.height; ++i) {
                    in.bw_pic[i] = out.bw_pic[i];
                }
                free(out.bw_pic);
            } else {
                for (int i = 0; i < out.width * out.height; ++i) {
                    in.colored_pic[i] = out.colored_pic[i];
                }
                free(out.colored_pic);
            }
        }

    }

    // After all the filters were applied, the root process will write the image in file.
    if(rank == 0) {
        writeData(argv[2], &in);
    }
    MPI_Type_free(&mpi_colors);

    MPI_Finalize();
    return 0;
}
