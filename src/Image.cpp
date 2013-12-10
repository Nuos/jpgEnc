#include "Image.hpp"

#include <array>
#include <streambuf>
#include <sstream>
#include <future>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/io.hpp>

#include "JpegSegments.hpp"
#include "Dct.hpp"
#include "Huffman.hpp"

using boost::numeric::ublas::matrix_range;
using boost::numeric::ublas::range;
using boost::numeric::ublas::zero_matrix;

static const auto debug = false;

//
// CONSTRUCTORS
//
// ctor
Image::Image(uint w, uint h, ColorSpace color)
    : color_space_type(color),
    width(w), height(h),
    one(h, w), two(h, w), three(h, w),
    Y(one), Cb(two), Cr(three),
    R(one), G(two), B(three)
{
    if (debug) std::cout << "Image::Image()\n" << std::endl;
}

// copy ctor
Image::Image(const Image& other)
    : color_space_type(other.color_space_type),
    width(other.width), height(other.height),
    one(other.one), two(other.two), three(other.three),
    Y(one), Cb(two), Cr(three),
    R(one), G(two), B(three)
{
    if (debug) std::cout << "Image::Image(Image&)\n" << std::endl;
}

// move ctor
Image::Image(Image&& other)
    : color_space_type(other.color_space_type),
    width(other.width), height(other.height),
    one(std::move(other.one)), two(std::move(other.two)), three(std::move(other.three)),
    Y(one), Cb(two), Cr(three),
    R(one), G(two), B(three)
{
    if (debug) std::cout << "Image::Image(Image&&)\n" << std::endl;
}

// dtor
Image::~Image()
{}

//
// ASSIGNMENTS
//
// copy assignment
Image& Image::operator = (const Image &other) {
    if (this != &other) {
        one = other.one;
        two = other.two;
        three = other.three;
        width = other.width;
        height = other.height;
        color_space_type = other.color_space_type;
        if (debug) std::cout << "Image::operator=(Image&)\n" << std::endl;
    }
    return *this;
}

// move assignment
Image& Image::operator=(Image &&other) {
    if (this != &other) {
        one = std::move(other.one);
        two = std::move(other.two);
        three = std::move(other.three);
        width = other.width;
        height = other.height;
        color_space_type = other.color_space_type;
        if (debug) std::cout << "Image::operator=(Image&&)\n" << std::endl;
    }
    return *this;
}

Image Image::convertToColorSpace(ColorSpace target_color_space) const {
    // no converting if already in target color space
    if (color_space_type == target_color_space)
        return *this;

    Image converted(*this);
    const auto num_pixel = converted.width * converted.height;

    switch (target_color_space) {
        case ColorSpace::YCbCr:
            {
                assert(color_space_type == ColorSpace::RGB);

                // matrix factors
                /*
                  0.299  0.587  0.114
                 -0.169 -0.331  0.500
                  0.500 -0.419 -0.081
                 */
                static const float Flat[] { .0f, 256/2.f, 256/2.f };
                static const float Yv[] {  .299f,   .587f,   .114f };
                static const float Cb[] { -.1687f, -.3312f,  .5f };
                static const float Cr[] {  .5f,    -.4186f, -.0813f };

                for (uint x = 0; x < num_pixel; ++x) {
                    auto& r = R.data()[x];
                    auto& g = G.data()[x];
                    auto& b = B.data()[x];

                    converted.Y.data()[x]        = (Flat[0] + (Yv[0] * r + Yv[1] * g + Yv[2] * b));
                    converted.Cb.data()[x]        = (Flat[1] + (Cb[0] * r + Cb[1] * g + Cb[2] * b));
                    converted.Cr.data()[x]        = (Flat[2] + (Cr[0] * r + Cr[1] * g + Cr[2] * b));
                }
            }
            break;
        case ColorSpace::RGB:
            {
                assert(color_space_type == ColorSpace::YCbCr);

                // matrix factors
                /*
                 1.000  0.000  1.402
                 1.000 -0.344 -0.714
                 1.000  1.772  0.000
                */
                static const float Flat[] { .0f, 256/2.f, 256/2.f };
                static const float r[] {  1.f,  .0f,    1.402f };
                static const float g[] {  1.f, -.344f,  -.714f };
                static const float b[] {  1.f, 1.772f,   .0f };

                for (uint x = 0; x < num_pixel; ++x) {
                    auto y  =  Y.data()[x] - Flat[0];
                    auto cb = Cb.data()[x] - Flat[1];
                    auto cr = Cr.data()[x] - Flat[2];

                    converted.Y.data()[x] = (r[0] * y + r[1] * cb + r[2] * cr);
                    converted.Cb.data()[x] = (g[0] * y + g[1] * cb + g[2] * cr);
                    converted.Cr.data()[x] = (b[0] * y + b[1] * cb + b[2] * cr);
                }
            }
            break;
    }
    return converted;
}

// simple "Matrix" used by the subsampling method
struct Image::Mask
{
    std::vector<Byte> row;
    bool scanline_jump = false;

    Mask() {}

    Mask(std::vector<Byte>&& r1, bool _scanline_jump)
        : row(std::move(r1))
    {
        scanline_jump = _scanline_jump;
    }

    uint rowsize() const { return static_cast<uint>(row.size()); }
};

void Image::subsample(matrix<PixelDataType>& chan, int hor_res_div, int vert_res_div, Mask& mat, bool averaging, SubsamplingMode mode)
{
    matrix<PixelDataType> new_chan(chan.size1() / vert_res_div, chan.size2() / hor_res_div);

    // subsample Cr matrix<PixelDataType>
    // size1() = rows
    // size2() = columns
    auto pixidx = 0U;
    auto pixidx2 = 0U;
    for (auto y = 0U; y < chan.size1(); y += 2) {
        for (auto x = 0U; x < chan.size2(); x += mat.rowsize()) {
            PixelDataType pix_val = 0;
            for (auto m = 0U; m < mat.rowsize(); ++m) {
                pix_val += mat.row[m] * chan(y, x+m);
            }
            new_chan.data()[pixidx++] = pix_val;
        }

        if (!mat.scanline_jump) {
            // go through next scanline and average the pixels
            if (averaging) {
                for (auto x = 0U; x < chan.size2(); x += mat.rowsize()) {
                    PixelDataType pix_val = 0;
                    for (auto m = 0U; m < mat.rowsize(); ++m) {
                        pix_val += mat.row[m] * chan(y + 1, x + m);
                    }
                    (new_chan.data()[pixidx2++] += (pix_val)) /= ((mode == S420_m) ? 4 : 2);
                }
            }
            // go through next scanline
            else
                --y;
        }

    }

    std::swap(chan, new_chan);
}

void Image::applySubsampling(SubsamplingMode mode)
{
    //enum SubsamplingMode
    //{
    //    S444,       // full sampling
    //    S422,       // every second pixel in a row
    //    S411,       // every fourth pixel in a row
    //    S420,       // every second pixel in every second row
    //    S420_lm,    // between vertical pixels
    //    S420_m      // between vertical and horizontal pixels
    //};

    bool averaging = false;
    
    int vert_res_div = 1;
    int hor_res_div = 1;

    Mask mat;

    switch (mode) {
        case Image::S444:
            // no subsampling
            // xx xx
            // xx xx
            return;
            break;
        case Image::S422:
            // x- x-
            // x- x-
            mat = Mask{ { 1, 0 }, false };

            vert_res_div = 1;
            hor_res_div = 2;
            break;
        case Image::S411:
            // x- --
            // x- --
            mat = Mask{ { 1, 0, 0, 0 }, false };

            vert_res_div = 1;
            hor_res_div = 4;
            break;
        case Image::S420:
            // x- x-
            // -- --
            mat = Mask{ { 1, 0 }, true };

            vert_res_div = 2;
            hor_res_div = 2;
            break;
        case Image::S420_m:
            // like S420, but taking the mean in vertical and horizontal direction
            // ++ ++
            // ++ ++
            mat = Mask{ { 1, 1 }, false };

            vert_res_div = 2;
            hor_res_div = 2;
            averaging = true;
            break;
        case Image::S420_lm:
            // like S420, but taking the mean only in vertical direction
            // +- +-
            // +- +-
            mat = Mask{ { 1, 0 }, false };

            vert_res_div = 2;
            hor_res_div = 2;
            averaging = true;
            break;
        default:
            break;
    }

    // subsampling Cr matrix<PixelDataType>
    subsample(Cr, hor_res_div, vert_res_div, mat, averaging, mode);

    // subsample Cb matrix<PixelDataType>
    subsample(Cb, hor_res_div, vert_res_div, mat, averaging, mode);
}

//
// PPM loading
//

// fast version of atoi. No error checking, nothing.
int fast_atoi(const char * str)
{
    int val = 0;
    while (*str) {
        val = val * 10 + (*str++ - '0');
    }
    return val;
};
struct PPMFileBuffer
{
    std::string& file;
    std::string::size_type file_pos;
    std::string::size_type eof;

    PPMFileBuffer(std::string& file)
        : file{file},
        file_pos{0},
        eof{file.size()} {}

    Byte read_byte() {
        return file[file_pos++];
    }

    void read_word(std::string& buf) {
        buf.clear();

        auto c = read_byte();

        if (std::isspace(c)) // discard any leading whitespace
            c = discard_whitespace();

        auto first = file_pos -1;
        while (1) {
            if (c == '#') {
                discard_current_line();
                first = file_pos;
            }
            else if (isspace(c)) {
                buf.insert(0, &file[first], file_pos - 1 - first);
                break;
            }
            else if (is_eof()) {
                buf.insert(0, &file[first], file_pos - first);
                break;
            }

            c = read_byte();
        }
    }

private:
    bool is_eof() {
        return file_pos >= eof;
    }

    Byte discard_whitespace() {
        while (isspace(file[file_pos]) && !is_eof())
            ++file_pos;
        return read_byte();
    }

    void discard_current_line() {
        while ((file[file_pos++] != '\n') && !is_eof());
    }
};

// load a ppm file into an RGBImage
void loadP3PPM(PPMFileBuffer& file, Image& img) {
    std::string buf;
    buf.reserve(32);

    const auto max = img.width * img.height;
    for (auto x = 0U; x < max; ++x) {
        file.read_word(buf);
        img.R.data()[x] = fast_atoi(buf.c_str());

        file.read_word(buf);
        img.G.data()[x] = fast_atoi(buf.c_str());

        file.read_word(buf);
        img.B.data()[x] = fast_atoi(buf.c_str());
    }
}

// Needs the ifstream opened as binary!
void loadP6PPM(PPMFileBuffer& ppm, Image& img) {
    const auto max = img.width * img.height;
    for (auto x = 0U; x < max; ++x) {
        img.R.data()[x] = ppm.read_byte();
        img.G.data()[x] = ppm.read_byte();
        img.B.data()[x] = ppm.read_byte();
    }
}

// top level load function with a path to a ppm file
Image loadPPM(std::string path) {
    std::ifstream::sync_with_stdio(false);
    std::ifstream ppm_file{path, std::fstream::binary};

    if (!ppm_file.is_open())
        throw std::runtime_error("Failed to open \"" + std::string(path) + "\"");

    ppm_file.seekg(0, std::ios::end);
    auto file_size = 0 + ppm_file.tellg();
    ppm_file.seekg(0, std::ios::beg);

    // using stringstream
    std::stringstream strstrbuf;
    strstrbuf << ppm_file.rdbuf();
    auto raw_buf = strstrbuf.str();

    ppm_file.close();

    PPMFileBuffer ppm{ raw_buf };

    std::string buf;
    buf.reserve(32);

    // magic number
    ppm.read_word(buf);
    std::string magic = buf;
    if (magic != "P3" && magic != "P6")
        throw std::runtime_error("Only P3 and P6 format is supported!");

    // width and height
    ppm.read_word(buf);
    auto width = std::stoi(buf);

    ppm.read_word(buf);
    auto height = std::stoi(buf);

    ppm.read_word(buf);
    auto max_color = std::stoi(buf);

    assert((max_color < 256) && "Only 1 byte colors supported for now!");

    Image img(width, height, Image::RGB);

    if (magic == "P3")
        loadP3PPM(ppm, img);
    else if (magic == "P6") {
        loadP6PPM(ppm, img);
    }

    //adjust size of our image for using 8x8 blocks
    if (width % 8 != 0 || height % 8 != 0)
    {
        img.width = (width + 8) - (width % 8);
        img.height  = (height + 8) - (height % 8);

        img.R.resize(img.height, img.width, true);
        img.G.resize(img.height, img.width, true);
        img.B.resize(img.height, img.width, true);

        // Fill the new Pixel with data from the border. 
        // Right
        for (auto y = 0U; y < height; ++y)
        {
            for (auto x = width; x < img.width; ++x)
            {
                img.R(y, x) = img.R(y, width - 1);
                img.G(y, x) = img.G(y, width - 1);
                img.B(y, x) = img.B(y, width - 1);
            }
        }

        // Bottom
        for (auto y = height; y < img.height; ++y)
        {
            for (auto x = 0U; x < width; ++x)
            {
                img.R(y, x) = img.R(height - 1, x);
                img.G(y, x) = img.G(height - 1, x);
                img.B(y, x) = img.B(height - 1, x);
            }
        }

        // Corner
        for (auto y = height; y < img.height; ++y)
        {
            for (auto x = width; x < img.width; ++x)
            {
                img.R(y, x) = img.R(height - 1, width - 1);
                img.G(y, x) = img.G(height - 1, width - 1);
                img.B(y, x) = img.B(height - 1, width - 1);
            }
        }

    }


    return img;
}

// NOTE: only processes the Cb channel
void Image::applyDCT(DCTMode mode) 
{
    std::function<void(matrix<PixelDataType>, matrix_range<matrix<PixelDataType>>&)> dctFn;

    switch (mode) {
    case Simple:
        dctFn = dctDirect;
        break;
    case Matrix:
        dctFn = dctMat;
        break;
    case Arai:
        dctFn = dctArai;
        break;
    default:
        assert(!"This DCT mode isn't supported!");
    }

    DctCb = zero_matrix<PixelDataType>(Cb.size1(), Cb.size2());

    using mat_range = matrix_range<matrix<PixelDataType>>;
    using range_pair = std::pair<const mat_range, mat_range>;

    // make a list with work for every worker to process
    const unsigned int worker = 4;
    std::vector<range_pair> list[4];

    // reserve some space before
    const auto num_blocks = width/8 * height/8;
    for (auto& vec : list)
        vec.reserve(num_blocks/worker);

    const auto blocksize = 8u;
    unsigned int cnter = 0;
    for (uint w = 0; w < width; w += blocksize) {
        for (uint h = 0; h < height; h += blocksize) {
            // generate slices for data source and the destination of the dct result
            const matrix_range<matrix<PixelDataType>> slice_src(Cb, range(w, w+blocksize), range(h, h+blocksize));
            matrix_range<matrix<PixelDataType>> slice_dst(DctCb, range(w, w+blocksize), range(h, h+blocksize));

            list[cnter].emplace_back(std::make_pair(slice_src, slice_dst));
            (++cnter) %= worker; 
        }
    }

    auto workerFunc = [dctFn](std::vector<range_pair> &list) {
        for (auto& elem : list) {
            const auto& src = elem.first;
            auto& dst = elem.second;

            dctFn(src, dst);
        }
    };

    std::vector<std::future<void>> fts;
    for (auto i = 0U; i < worker; ++i)
        fts.emplace_back(std::async(workerFunc, list[i])); // launch the worker asynchronous
    for (auto& future : fts)
        future.get(); // waits until all threads are finished
}

void Image::writeJPEG(std::wstring file)
{
    std::ofstream jpeg(file, std::ios::binary);

    using namespace Segment;

    vector<int> text;
    for (int i = 0; i < 256; i++) {
        // to get some maximum length ones
        if (i < 10)
            text.push_back(i);
        else
            for (int j = 0; j < i; j++)
                text.push_back(i);
    }

    auto pair = generateHuffmanCode(text);
    SymbolCodeMap code_map = pair.first; // for encoding
    SymbolsPerLength codedata = pair.second;

    jpeg << SOI
        << APP0
        << SOF0_3c
            .setImageSizeX(width)
            .setImageSizeY(height)
            .setCompSetup({ CompSetup::Y, CompSetup::NoSubSampling, 0,
                            CompSetup::Cb, CompSetup::Half, 1,
                            CompSetup::Cr, CompSetup::Half, 2, })
        << DHT.setCodeData(codedata)
        << EOI
        ;
}

