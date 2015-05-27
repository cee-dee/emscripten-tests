#include <stdio.h>
#include "jpeglib.h"
#include <setjmp.h>
#include <stdlib.h>
#include <sys/time.h>
#include <dirent.h>

#include <stddef.h>
#include <memory.h>
#include <fstream>
#include <iostream>
#include <functional>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

using namespace std;

/**
 * Wrapper class for libjpeg.
 * This wrapper takes an image as byte-array and processes it stepwise.
 */
class DecodeImage
{
    public:

        DecodeImage(bool debug_log=false);
        ~DecodeImage();

        /**
         * Preparation method
         * datasize
         * memoryview return callback for input data
         */
        void prepare(int, const std::function<void(size_t, uint8_t*, DecodeImage*)> &callback);

        /**
         * initializer method -- to be called after prepare (and after
         * the calling code has input the image data)
         * scaling nominator
         * scaling denominator
         */
        void initialize(int, int);

        /** Returns the width of the image */
        int getImageWidth();

        /** Returns the height of the image */
        int getImageHeight();

        /** Returns the width of the image */
        std::string getFileName(void);

        /** Returns the height of the image */
        void setFileName(std::string);

        /**
          * memoryview return callback for scanline output
          * Returns: pointer to scanline buffer via callback
          */
        JSAMPARRAY getScanlineStorage(const std::function<void(size_t, uint8_t*)> &callback);

        /**
          * Retrieve next scanline
          * Returns: true if there was a next scanline, false otherwise
          */
        bool retrieveNextScanline();



    private:

        std::string filename;

        /** Logging method */
        void log(std::string);

        /** If set to true, debugging log will be written to stdout */
        bool debug_log;

        /** LibJpeg's internal datastructure for decompressing */
        struct jpeg_decompress_struct cinfo;

        /** Internal buffer storage for the scanlines */
        JSAMPARRAY buffer;

        /** Size of the given byte-array, given by the caller */
        std::size_t sizeOfData;

        /** The imagedata to process */
        uint8_t *imageData;
};

typedef struct decodeImage_error_mgr * my_error_ptr;
