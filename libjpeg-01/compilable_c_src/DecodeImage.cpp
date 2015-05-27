 #include "DecodeImage.h"

typedef struct decodeImage_error_mgr * my_error_ptr;

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
DecodeImage::DecodeImage(bool debug_log)
{
    this->log("Debug: Constructor called");
    this->debug_log = debug_log;
}

/**
  * Destructor
  */
DecodeImage::~DecodeImage()
{
    this->log("Debug: Destructor called");
    this->log("Debug: Trying to release jpeg-reading object");

    // Release JPEG decompression object
    jpeg_destroy_decompress(&this->cinfo);
    this->log("Debug: Done releasing jpeg-reading object");

    this->log("Debug: Trying to free memory");
    free(this->imageData);
    this->log("Debug: Done freeing memory");

    this->log("Debug: Destructor done");
}

/**
 * Preparation method
 * sizeOfData the size of the given data
 * emscripten::val memoryview return callback for input data
 */
void DecodeImage::prepare(int sizeOfData, const std::function<void(size_t sizeOfData, uint8_t* dataBuffer, DecodeImage* decodeImage)> &callback)
{
    struct jpeg_error_mgr jerr;
    struct jpeg_decompress_struct cinfo;
    this->cinfo = cinfo;

    cout << "Size of data: " << sizeOfData << endl;
    this->sizeOfData = sizeOfData;

    this->log("Debug: prepare called");

    this->imageData = (uint8_t*) malloc(this->sizeOfData);

    this->log("Debug: running callback");
    callback(this->sizeOfData, this->imageData, this);
    this->log("Debug: callback done");

    // Allocate and initialize JPEG decompression object
    this->cinfo.err = jpeg_std_error(&jerr);

    // Now we can initialize the JPEG decompression object.
    jpeg_create_decompress(&this->cinfo);
    this->log("Debug: Done allocating and initializing jpeg decompression");


    // Specify data source (eg, a file)
    this->log("Debug: Trying to specifiy datasource");
    //my_set_source_mgr(&cinfo, this->imageData, this->sizeOfData);
    //jpeg_memory_src(&this->cinfo, this->imageData, this->imageData + sizeOfData * sizeof(uint8_t));
    jpeg_mem_src(&this->cinfo, (unsigned char*)(this->imageData), this->sizeOfData);
    this->log("Debug: Done specifying datasource");
}

/**
  * Init the internal objects
  * scale_num:   scaling factor (nominator)
  * scale_denom: scaling factor (denominator)
  */
void DecodeImage::initialize(int scale_num, int scale_denom)
{
    this->log("Debug: Trying to read jpeg header");
    // Read file parameters
    (void) jpeg_read_header(&this->cinfo, TRUE);
    this->log("Debug: Done reading jpeg header");

    this->log("Debug: Trying to set parameters for decompression");

    // Set parameters for decompression
    this->cinfo.dct_method          = JDCT_IFAST;
    this->cinfo.do_fancy_upsampling = FALSE;
    this->cinfo.two_pass_quantize   = FALSE;
    this->cinfo.dither_mode         = JDITHER_ORDERED;
    this->cinfo.scale_num           = scale_num;
    this->cinfo.scale_denom         = scale_denom;
    //this->cinfo.in_color_space      = JCS_RGB
    this->log("Debug: Done setting parameters for decompression");

    this->log("Debug: Trying to decompress image");

    // Start decompressor
    (void) jpeg_start_decompress(&this->cinfo);
    this->log("Debug: init done");
}

/**
  * Returns the scanline storage used to store a single scanline
  */
JSAMPARRAY DecodeImage::getScanlineStorage(const std::function<void(size_t, uint8_t*)> &callback)
{
    // physical row width in output buffer
    int row_stride;

    row_stride = this->cinfo.output_width * this->cinfo.output_components;

    this->buffer = (*this->cinfo.mem->alloc_sarray)((j_common_ptr) &this->cinfo, JPOOL_IMAGE, row_stride, 1);

    callback((size_t)row_stride, (uint8_t*)buffer);

    return buffer;
}

/**
  * Read the next scanline and return it as byte-array
  */
bool DecodeImage::retrieveNextScanline()
{
    if (cinfo.output_scanline < cinfo.output_height)
    {
        this->log("Debug: trying to read scanline");
        (void) jpeg_read_scanlines(&cinfo, this->buffer, 1);
        this->log("Debug: done reading scanline");
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

std::string DecodeImage::getFileName(void)
{
    return this->filename;
}

void DecodeImage::setFileName(std::string fName)
{
    this->filename = fName;
}

/**
  * Log a string message if this->debug_log is set
  */
void DecodeImage::log(std::string message)
{
    if (true)
    {
        std::cout << message << endl;
    }
}

static void inputCallback(size_t givenSize, uint8_t *data, DecodeImage* decodeImage)
{
    std::ifstream fin(decodeImage->getFileName());
    // get pointer to associated buffer object
    std::filebuf* pbuf = fin.rdbuf();
    // get file size using buffer's members
    std::size_t size = pbuf->pubseekoff (0,fin.end,fin.in);
    pbuf->pubseekpos (0,fin.in);
    // allocate memory to contain file data
    // get file data

    pbuf->sgetn ((char*)data,givenSize);
    fin.close();
}

static void storageCallback(size_t givenSize, uint8_t *data)
{
    cout << "storage callback called" << endl;
}

static void loadImage(std::string fName)
{
    std::string fileName = "/tmp/images/" + fName;
    cout << "Using filename: " << fileName << endl;

    std::ifstream fin(fileName);
    // get pointer to associated buffer object
    std::filebuf* pbuf = fin.rdbuf();
    // get file size using buffer's members
    std::size_t size = pbuf->pubseekoff (0,fin.end,fin.in);
    fin.close();

    DecodeImage decodeImage = new DecodeImage(true);

    decodeImage.setFileName(fileName);
    decodeImage.prepare(size, inputCallback);
    decodeImage.initialize(1,8);

    JSAMPARRAY jbuff = decodeImage.getScanlineStorage(storageCallback);

    unsigned long bmp_size;
    unsigned char *bmp_buffer;
    int row_stride, width, height, pixel_size;

    width = decodeImage.getImageWidth();
    height = decodeImage.getImageHeight();
    pixel_size = 3;
    bmp_size = width * height * pixel_size;
    bmp_buffer = (unsigned char*) malloc(bmp_size);

    row_stride = width * pixel_size;

    int count = 0;
    while (decodeImage.retrieveNextScanline())
    {
        unsigned char *buffer_array;
        buffer_array = bmp_buffer + count * row_stride;

        memcpy(buffer_array, jbuff[0], row_stride);
        ++count;
    }
    //delete &decodeImage;

    std::string fileNameConverted = "/tmp/converted/" + fName;
    std::cout << "Output filename: " << fileNameConverted << endl;

    int fd = open(fileNameConverted.c_str(), O_CREAT | O_WRONLY, 0666);
    char buf[1024];

    int rc = sprintf(buf, "P6 %d %d 255\n", width, height);
    write(fd, buf, rc); // Write the PPM image header before data
    write(fd, bmp_buffer, bmp_size); // Write out all RGB pixel dat
}

/**
  * !!!This is just a demo method to run the class for debugging purposes!!!
  */
int main (void)
{
    DIR *dir;
    struct dirent *ent = NULL;

    double totalTime = 0;

    if ((dir = opendir ("/tmp/images/")) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
            clock_t begin = clock();

            std::string fileName = std::string(ent->d_name);
            //std::cout << fileName << endl;

            if (fileName.compare(".") != 0 && fileName.compare("..")) {
                loadImage(ent->d_name);
            }

            clock_t end = clock();

            double elapsed_millisecs = double(end - begin);
            std::cout << "Took: " << elapsed_millisecs / 1000 / 1000 << std::endl;
            totalTime = totalTime + elapsed_millisecs;

        }
        std::cout << "Total time: " <<  totalTime / 1000 / 1000 << std::endl;
        closedir (dir);
    } else {
        /* could not open directory */
        perror ("");
    }
  return EXIT_FAILURE;
}
