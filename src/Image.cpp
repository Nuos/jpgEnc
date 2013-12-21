#include "Image.hpp"

#include <array>
#include <streambuf>
#include <sstream>
#include <future>
#include <omp.h>

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
    subsample_width(w), subsample_height(h),
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
    subsample_width(other.subsample_width), subsample_height(other.subsample_height),
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
    subsample_width(other.subsample_width), subsample_height(other.subsample_height),
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
        subsample_width = other.subsample_width;
        subsample_height = other.subsample_height;
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
        subsample_width = other.subsample_width;
        subsample_height = other.subsample_height;
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

                    converted.Y.data()[x]  = Flat[0] + (Yv[0] * r + Yv[1] * g + Yv[2] * b) - 128;
                    converted.Cb.data()[x] = Flat[1] + (Cb[0] * r + Cb[1] * g + Cb[2] * b) - 128;
                    converted.Cr.data()[x] = Flat[2] + (Cr[0] * r + Cr[1] * g + Cr[2] * b) - 128;
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

                    converted.Y.data()[x] =  r[0] * y + r[1] * cb + r[2] * cr + 128;
                    converted.Cb.data()[x] = g[0] * y + g[1] * cb + g[2] * cr + 128;
                    converted.Cr.data()[x] = b[0] * y + b[1] * cb + b[2] * cr + 128;
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

    subsample_width = width / hor_res_div;
    subsample_height = height / vert_res_div;

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
    std::function<void(const matrix_range<matrix<PixelDataType>>&, matrix_range<matrix<PixelDataType>>&)> dctFn;

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

    const auto blocksize = 8;

    //omp_set_num_threads(4); // can also be set via an environment variable

    DctY = zero_matrix<PixelDataType>(Y.size1(), Y.size2());
    DctCb = zero_matrix<PixelDataType>(Cb.size1(), Cb.size2());
    DctCr = zero_matrix<PixelDataType>(Cr.size1(), Cr.size2());

//#pragma omp parallel for
    for (int w = 0; w < width; w += blocksize) {
        for (int h = 0; h < height; h += blocksize) {
            // generate slices for data source and the destination of the dct result
            const matrix_range<matrix<PixelDataType>> slice_src(Y, range(w, w + blocksize), range(h, h + blocksize));
            matrix_range<matrix<PixelDataType>> slice_dst(DctY, range(w, w + blocksize), range(h, h + blocksize));
            dctFn(slice_src, slice_dst);
        }
    }

//#pragma omp parallel for
    for (int w = 0; w < subsample_width; w += blocksize) {
        for (int h = 0; h < subsample_height; h += blocksize) {
            // generate slices for data source and the destination of the dct result
            const matrix_range<matrix<PixelDataType>> slice_src(Cb, range(w, w+blocksize), range(h, h+blocksize));
            matrix_range<matrix<PixelDataType>> slice_dst(DctCb, range(w, w+blocksize), range(h, h+blocksize));
            dctFn(slice_src, slice_dst);
        }
    }

//#pragma omp parallel for
    for (int w = 0; w < subsample_width; w += blocksize) {
        for (int h = 0; h < subsample_height; h += blocksize) {
            // generate slices for data source and the destination of the dct result
            const matrix_range<matrix<PixelDataType>> slice_src(Cr, range(w, w + blocksize), range(h, h + blocksize));
            matrix_range<matrix<PixelDataType>> slice_dst(DctCr, range(w, w + blocksize), range(h, h + blocksize));
            dctFn(slice_src, slice_dst);
        }
    }
}

void Image::writeJPEG(std::wstring file)
{
    // color conversion to YCbCr
    *this = convertToColorSpace(YCbCr);

    // Cb/Cr subsampling
    //applySubsampling(SubsamplingMode::S420_m);

    // DCT
    applyDCT(DCTMode::Arai);

    // quantization
    const auto quantization_table = from_vector<Byte>({
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
    });

    QY = zero_matrix<int>(DctY.size1(), DctY.size2());
    QCb = zero_matrix<int>(DctCb.size1(), DctCb.size2());
    QCr = zero_matrix<int>(DctCr.size1(), DctCr.size2());

    for (int w = 0; w < width; w += blocksize) {
        for (int h = 0; h < height; h += blocksize) {
            // generate slices for data source and the destination of the dct result
            const matrix_range<matrix<PixelDataType>> slice_src(DctY, range(w, w + blocksize), range(h, h + blocksize));
            matrix_range<matrix<int>> slice_dst(QY, range(w, w + blocksize), range(h, h + blocksize));
            slice_dst.assign(quantize(slice_src, quantization_table));
        }
    }

    for (int w = 0; w < subsample_width; w += blocksize) {
        for (int h = 0; h < subsample_height; h += blocksize) {
            // generate slices for data source and the destination of the dct result
            const matrix_range<matrix<PixelDataType>> slice_src(DctCb, range(w, w + blocksize), range(h, h + blocksize));
            matrix_range<matrix<int>> slice_dst(QCb, range(w, w + blocksize), range(h, h + blocksize));
            slice_dst.assign(quantize(slice_src, quantization_table));
        }
    }

    for (int w = 0; w < subsample_width; w += blocksize) {
        for (int h = 0; h < subsample_height; h += blocksize) {
            // generate slices for data source and the destination of the dct result
            const matrix_range<matrix<PixelDataType>> slice_src(DctCr, range(w, w + blocksize), range(h, h + blocksize));
            matrix_range<matrix<int>> slice_dst(QCr, range(w, w + blocksize), range(h, h + blocksize));
            slice_dst.assign(quantize(slice_src, quantization_table));
        }
    }

    // DC difference coding
    int b = 0;
    for (int w = 0; w < width; w += blocksize) {
        for (int h = 0; h < height; h += blocksize) {
            auto tmp = QY(w, h);
            QY(w, h) = b - tmp;
            b = tmp;
        }
    }

    b = 0;
    for (int w = 0; w < subsample_width; w += blocksize) {
        for (int h = 0; h < subsample_height; h += blocksize) {
            auto tmp = QCb(w, h);
            QCb(w, h) = b - tmp;
            b = tmp;
        }
    }

    b = 0;
    for (int w = 0; w < subsample_width; w += blocksize) {
        for (int h = 0; h < subsample_height; h += blocksize) {
            auto tmp = QCr(w, h);
            QCr(w, h) = b - tmp;
            b = tmp;
        }
    }

    // zigzag
    ZigZagY.clear();
    ZigZagCb.clear();
    ZigZagCr.clear();

    for (int w = 0; w < width; w += blocksize) {
        for (int h = 0; h < height; h += blocksize) {
            // generate slices for data source and the destination of the dct result
            const matrix_range<matrix<int>> slice_src(QY, range(w, w + blocksize), range(h, h + blocksize));
            ZigZagY.push_back(zigzag<int>(slice_src));
        }
    }

    for (int w = 0; w < subsample_width; w += blocksize) {
        for (int h = 0; h < subsample_height; h += blocksize) {
            // generate slices for data source and the destination of the dct result
            const matrix_range<matrix<int>> slice_src(QCb, range(w, w + blocksize), range(h, h + blocksize));
            ZigZagCb.push_back(zigzag<int>(slice_src));
        }
    }

    for (int w = 0; w < subsample_width; w += blocksize) {
        for (int h = 0; h < subsample_height; h += blocksize) {
            // generate slices for data source and the destination of the dct result
            const matrix_range<matrix<int>> slice_src(QCr, range(w, w + blocksize), range(h, h + blocksize));
            ZigZagCr.push_back(zigzag<int>(slice_src));
        }
    }

    // RLE, category encoding
    CategoryCodeY.clear();
    CategoryCodeCb.clear();
    CategoryCodeCr.clear();

    for (const auto& data : ZigZagY) {
        auto rle_data = RLE_AC(data);
        auto encoded_coeffs = encode_category(rle_data);
        CategoryCodeY.push_back(encoded_coeffs);
    }

    for (const auto& data : ZigZagCb) {
        auto rle_data = RLE_AC(data);
        auto encoded_coeffs = encode_category(rle_data);
        CategoryCodeCb.push_back(encoded_coeffs);
    }

    for (const auto& data : ZigZagCr) {
        auto rle_data = RLE_AC(data);
        auto encoded_coeffs = encode_category(rle_data);
        CategoryCodeCr.push_back(encoded_coeffs);
    }

    // Huffman
    std::vector<int> DC_symbols, AC_symbols;
    for (const auto& data : CategoryCodeY) {
        DC_symbols.push_back(data[0].symbol);
        for (auto it = begin(data) + 1; it != end(data); ++it)
            AC_symbols.push_back((*it).symbol);
    }
    for (const auto& data : CategoryCodeCb) {
        DC_symbols.push_back(data[0].symbol);
        for (auto it = begin(data) + 1; it != end(data); ++it)
            AC_symbols.push_back((*it).symbol);
    }
    for (const auto& data : CategoryCodeCr) {
        DC_symbols.push_back(data[0].symbol);
        for (auto it = begin(data) + 1; it != end(data); ++it)
            AC_symbols.push_back((*it).symbol);
    }

    auto DC_huff = generateHuffmanCode(DC_symbols);
    auto AC_huff = generateHuffmanCode(AC_symbols);

    auto& DC_encoder = DC_huff.first;
    auto& DC_Huffman_Table = DC_huff.second;

    auto& AC_encoder = AC_huff.first;
    auto& AC_Huffman_Table = AC_huff.second;

    // encode symbols
    BitstreamY.clear();
    for (auto& data : CategoryCodeY) {
        Bitstream stream;

        auto encoded_DC = DC_encoder[data[0].symbol];
        stream.push_back(encoded_DC.code, encoded_DC.length);
        stream << data[0].code;

        for (auto it = begin(data) + 1; it != end(data); ++it) {
            auto& code = *it;
            auto encoded_AC = AC_encoder[code.symbol];
            stream.push_back(encoded_AC.code, encoded_AC.length);
            stream << code.code;
        }

        BitstreamY.push_back(stream);
    }

    BitstreamCb.clear();
    for (auto& data : CategoryCodeCb) {
        Bitstream stream;

        auto encoded_DC = DC_encoder[data[0].symbol];
        stream.push_back(encoded_DC.code, encoded_DC.length);
        stream << data[0].code;

        for (auto it = begin(data) + 1; it != end(data); ++it) {
            auto& code = *it;
            auto encoded_AC = AC_encoder[code.symbol];
            stream.push_back(encoded_AC.code, encoded_AC.length);
            stream << code.code;
        }

        BitstreamCb.push_back(stream);
    }

    BitstreamCr.clear();
    for (auto& data : CategoryCodeCr) {
        Bitstream stream;

        auto encoded_DC = DC_encoder[data[0].symbol];
        stream.push_back(encoded_DC.code, encoded_DC.length);
        stream << data[0].code;

        for (auto it = begin(data) + 1; it != end(data); ++it) {
            auto& code = *it;
            auto encoded_AC = AC_encoder[code.symbol];
            stream.push_back(encoded_AC.code, encoded_AC.length);
            stream << code.code;
        }

        BitstreamCr.push_back(stream);
    }

    using namespace Segment;

    std::ofstream jpeg(file, std::ios::binary);
  
    auto zigzag_qtable = zigzag<Byte>(quantization_table);

    Bitstream stream;

    jpeg << SOI
        << APP0
        << DQT.pushQuantizationTable(zigzag_qtable, sDQT::Zero)
              //.pushQuantizationTable(zigzag_qtable, sDQT::One)
        << SOF0_3c
            .setImageSizeX(width)
            .setImageSizeY(height)
            .setCompSetup({ CompSetup::Y,  CompSetup::NoSubSampling, 0,
                            CompSetup::Cb, CompSetup::NoSubSampling, 0,
                            CompSetup::Cr, CompSetup::NoSubSampling, 0 })
        << DHT.pushCodeData(DC_Huffman_Table, sDHT::DC, sDHT::First)
              .pushCodeData(AC_Huffman_Table, sDHT::AC, sDHT::First)
        << SOS
        ;

    for (auto i = 0U; i < BitstreamY.size(); ++i) {
        jpeg << BitstreamY[i] << BitstreamCb[i] << BitstreamCr[i];
    }

    jpeg << EOI;
}

