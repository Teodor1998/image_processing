
                        #################################
                        #             README            #
                        #                               #
                        # Antialiasing & micro renderer #
                        #    Made by: Teodor Apostol    #
                        #                               #
                        #################################



    ------------------------------ Antialiasing ------------------------------
        This task was implemented in the "homework.c" file and the header file
    in which I declared the data structures used is "homework.h".

    Data Structures used
    ====================
    -> colors: This is the representation of a colored pixel. It contains three
        bytes, one for each color: R, G, B.

    -> image: In this data structure I store each image. The fields are:
        - P : Integer that can be 5 (black & white) or 6 (colored)
        - width and height : Integers that represent the sizes of the image
        - maxval : Integer that represents the max value of each color byte
        - colored_pic : Matrix of colored pixels (uses colors data structures)
        - bw_pc : Matrix of black & white pixels
    
    -> routine_args: The arguments that will be sent for each thread's routine:
        - thread_id : Integer that represents the id: 0 to (num_threads - 1)
        - in : The input image
        - out : The output image
        - each : Integer that represents the number of iterations for each thread 

    Program flow
    ============
        The reading:
        This part is done by calling the readInput() funciton, which has the
    fileName and img parameters.
        This funciton opens the file for a text reading, using fscanf(), in
    order to get the header. After the header is read, Memory is dynamically
    allocated for the pixel matrix, depending on the number of lines and columns
    and the type of image (black & white or color).
        After the memory allocation, the file is once again opened, but this
    time for a binary reading. The program skips the header, and begins storing
    the pixel matrix into the data structure.

        The resize:
        This operation uses the main function resize() and some auxiliary ones.
        The resize() function has the input and output images as parameters.
        It starts with a dynamic allocation for the output image, knowing that
    the sizes are the input image sizes divided be the resize_factor and for
    the array of threads and argument structure.
        The each variable is computed by dividing the number of lines of the
    output image by the number of threads.
        The next step is building the arguments array for each thread, giving
    the values mentioned in the "Data Structures used" section.
        The program is then split into "num_threads" execution threads.

        The routine of each thread is the parallel_for() function.
        Based on the number of iterations each thread will do, on the index
    of the thread and on the resize_factor, The function begins by computing the
    value of the starting line, this thread will process and stores it in the
    "from" variable. The "to" variable is the number of the line this thread will
    stop at. Depending on the matching of the num_threads, this value may be
    the from + how many rows of the input matrix will be processed or the end of
    the matrix.
        All the threads execute the exact number of loops, excepting the last
    one that may execute more.
        The routine has two identically parts. One for the black & white image
    and one for the colored image.
        Each part does the following things:
        1. Dynamic allocation for the resize_factor x resize_factor matrix, that
            will be converted into one pixel.
        2. Calls an auxiliary function that generates the pixel based on this
            matrix.
        This operations are made by multiple loops:
        i index: on each loop it gets a + resize_facton increment and represents
        the line number in the input image from which we will begin building the
        square to be converted to a pixel.
        j index: on each loop it gets a + resize_factor increment and represents
        the column number in the input image from which we will begin building the
        square to be converted to a pixel.
        k1 & k2: are the indexes for the square matrix (the one to be converted),
        each goes from 0 to (resize_factor - 1).
        i2: has a 1 increment and represents the row of the output matrix.
        j2: has a 1 increment and represents the column of the output matrix.

        The auxiliary functions used to make the conversion from matrix to 1
    pixel:
        color_even_to_1 and bw_even_to_1: Functions that make the arithmetic
    average of each pixel color and returns the result. This is colled if the
    resize_factor is an even number.
        color_3_to_1 and bw_3_to_1: Functions that multiplies the VALUES of the
    matrix to be reduced with the VALUES of the Gaussian Kernel and divides the
    result by 16. This is the color value of the resulted pixel.

    ----------------------------- Micro renderer -----------------------------
        This task was implemented in the "homework1.c" file and the header file
    in which I declared the data structures used is "homework1.h".

    Data Structures used
    ====================
    -> image: The pixel matrix of the image.
    -> routine_args: The argument list that is used by every thread.

    Program flow
    ============
        The rendering:
        After the initialization, where the program dynamically allocates
    memory for the empty pixel matrix that will represent the image, the
    thread array and the argument list array is dynamically allocated.
        Each thread will execute a function called parallel_for. This
    function will loop from "from" to "to" (values computed using the thread
    index and the each value which represents the number of loops each thread
    executes).
        For each pixel in the image, the program computes the distance to the
    black line using the dist() function. The arguments for the dist() function
    are the coordinates of the pixels and the function returns the distance in
    centimeters. The comparation is done and so is decided the color of the
    pixel: white or black.
        After the join, the matrix is printed in the destination file, in
    the same way as on the first task.
