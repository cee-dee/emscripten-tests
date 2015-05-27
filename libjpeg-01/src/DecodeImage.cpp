#include "DecodeImage.h"

/**
  * Error struct used by libjpeg to communicate
  * internal errors to the caller
  */
struct decodeImage_error_mgr
{
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
};

/**
  * Constructor
  */
DecodeImage::DecodeImage() {}

/**
  * Destructor
  */
DecodeImage::~DecodeImage()
{
    // Release JPEG decompression object
    jpeg_destroy_decompress(&this->cinfo);

    // free input image storage
    free(this->imageData);
}

/**
 * Preparation method
 * sizeOfData the size of the given data
 * emscripten::val memoryview return callback for input data
 */
void DecodeImage::prepare(int sizeOfData, emscripten::val callback)
{
    // create and assign libjpegs internal data structure
    struct jpeg_error_mgr jerr;
    struct jpeg_decompress_struct cinfo;
    this->cinfo = cinfo;
    this->sizeOfData = sizeOfData;

    // Allocate Memory
    this->imageData = (uint8_t*) malloc(this->sizeOfData);
    callback(emscripten::memory_view<uint8_t>(this->sizeOfData, this->imageData));

    // Allocate and initialize JPEG decompression object
    this->cinfo.err = jpeg_std_error(&jerr);

    // Now we can initialize the JPEG decompression object.
    jpeg_create_decompress(&this->cinfo);

    // Specify data source
    jpeg_mem_src(&this->cinfo, (unsigned char*)(this->imageData), this->sizeOfData);
}

/**
  * Init the internal objects
  * scale_num:   scaling factor (nominator)
  * scale_denom: scaling factor (denominator)
  */
void DecodeImage::initialize(int scale_num, int scale_denom)
{
    // Read file parameters
    (void) jpeg_read_header(&this->cinfo, TRUE);

    // Set parameters for decompression
    this->cinfo.dct_method          = JDCT_IFAST;
    this->cinfo.do_fancy_upsampling = FALSE;
    this->cinfo.two_pass_quantize   = FALSE;
    this->cinfo.dither_mode         = JDITHER_ORDERED;
    this->cinfo.scale_num           = scale_num;
    this->cinfo.scale_denom         = scale_denom;

    // Start decompressor
    (void) jpeg_start_decompress(&this->cinfo);
}

/**
  * Returns the scanline storage used to store a single scanline
  */
void DecodeImage::getScanlineStorage(emscripten::val callback)
{
    // physical row width in output buffer
    int row_stride;

    // calculate row size
    row_stride = this->cinfo.output_width * this->cinfo.output_components;

    // allocate memory for a single scanline
    this->buffer = (*this->cinfo.mem->alloc_sarray)((j_common_ptr) &this->cinfo, JPOOL_IMAGE, row_stride, 1);

    // run the callback from js
    callback(emscripten::memory_view<uint8_t>((size_t)row_stride, (uint8_t*)buffer[0]));
}

/**
  * Read the next scanline and return it as byte-array
  */
bool DecodeImage::retrieveNextScanline()
{
    // if the is a scanline left, overwrite the memory-buffer with the new scanline
    if (cinfo.output_scanline < cinfo.output_height)
    {
        (void) jpeg_read_scanlines(&cinfo, this->buffer, 1);
        return true;
    }
    else
    {
        return false;
    }
}

/**
  * Returns the width of the image
  */
int DecodeImage::getImageWidth()
{
    return this->cinfo.output_width;
}

/**
  * Returns the height of the image
  */
int DecodeImage::getImageHeight()
{
    return this->cinfo.output_height;
}

// Binding code
EMSCRIPTEN_BINDINGS(decode_image) {
  emscripten::class_<DecodeImage>("DecodeImage")
    .constructor<>()
    .function("prepare", &DecodeImage::prepare)
    .function("initialize", &DecodeImage::initialize)
    .function("getImageWidth", &DecodeImage::getImageWidth)
    .function("getImageHeight", &DecodeImage::getImageHeight)
    .function("getScanlineStorage", &DecodeImage::getScanlineStorage)
    .function("retrieveNextScanline", &DecodeImage::retrieveNextScanline)
    ;
}
