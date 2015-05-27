#include <stdio.h>
#include <setjmp.h>
#include <stdlib.h>
#include <memory.h>
#include <fstream>

#include "jpeglib.h"
#include <emscripten/bind.h>

/**
 * Wrapper class for libjpeg.
 * This wrapper takes an image as byte-array and processes it stepwise.
 */
class DecodeImage
{
    public:

        /**
         * Constructor
         */
        DecodeImage();

        /**
         * Destructor
         */
        ~DecodeImage();

        /**
         * Preparation method
         * datasize size of the content
         * memoryview return callback for input data
         */
        void prepare(int, emscripten::val callback);

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

        /**
          * memoryview return callback for scanline output
          * Returns: pointer to scanline buffer via callback
          */
        void getScanlineStorage(emscripten::val callback);

        /**
          * Retrieve next scanline
          * Returns: true if there was a next scanline, false otherwise
          */
        bool retrieveNextScanline();



    private:
        /** LibJpeg's internal datastructure for decompressing */
        struct jpeg_decompress_struct cinfo;

        /** Internal buffer storage for the scanlines */
        JSAMPARRAY buffer;

        /** Size of the given byte-array, given by the caller */
        std::size_t sizeOfData;

        /** The imagedata to process */
        uint8_t *imageData;
};
