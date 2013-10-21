#include "Image.hpp"


static const auto debug = false;

//
// CHANNEL
//
// ctor
Image::Channel::Channel(uint w, uint h)
: width{ w },
height{ h }
{
    pixels.reserve(width*height);
    if (debug) std::cout << "Channel::Channel()" << std::endl;
}

// copy ctor
Image::Channel::Channel(const Channel& other)
: pixels(other.pixels),
width(other.width), height(other.height)
{
    if (debug) std::cout << "Channel::Channel(Channel&)" << std::endl;
}

// move ctor
Image::Channel::Channel(Channel&& other)
: pixels(std::move(other.pixels)),
width(other.width), height(other.height)
{
    if (debug) std::cout << "Channel::Channel(Channel&&)" << std::endl;
}

// ASSIGNMENT
// copy assignment
Image::Channel& Image::Channel::operator=(const Channel &other)
{
    if (this != &other) {
        pixels = other.pixels;
        width = other.width;
        height = other.height;
        if (debug) std::cout << "Channel::operator=(Channel&)" << std::endl;
    }
    return *this;
}

// move assignment
Image::Channel& Image::Channel::operator=(Channel &&other)
{
    if (this != &other) {
        pixels = std::move(other.pixels);
        width = other.width;
        height = other.height;
        if (debug) std::cout << "Channel::operator=(Channel&)" << std::endl;
    }
    return *this;
}

// INDEXING
// for one-dimensional indexing
Byte& Image::Channel::at(uint x)
{
    return pixels[x];
}

// for two-dimensional indexing
// stops at pixel border, so no out-of-bounds indexing possible
// practically duplicates pixel at the border 
Byte& Image::Channel::at(uint x, uint y)
{
    x = std::min(x, width - 1);
    y = std::min(y, height - 1);

    return pixels[y * width + x];
}

//
// CONSTRUCTORS
//
// ctor
Image::Image(uint w, uint h, ColorSpace color)
    : color_space_type(color),
    width(w), height(h),
    one(w, h), two(w, h), three(w, h),
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
                    auto& r = R.at(x);
                    auto& g = G.at(x);
                    auto& b = B.at(x);

                    converted.Y.at(x)  = static_cast<Byte>(Flat[0] + (Yv[0] * r + Yv[1] * g + Yv[2] * b));
                    converted.Cb.at(x) = static_cast<Byte>(Flat[1] + (Cb[0] * r + Cb[1] * g + Cb[2] * b));
                    converted.Cr.at(x) = static_cast<Byte>(Flat[2] + (Cr[0] * r + Cr[1] * g + Cr[2] * b));
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
                    auto y  =  Y.at(x) - Flat[0];
                    auto cb = Cb.at(x) - Flat[1];
                    auto cr = Cr.at(x) - Flat[2];

                    converted.Y.at(x)  = static_cast<Byte>(r[0] * y + r[1] * cb + r[2] * cr);
                    converted.Cb.at(x) = static_cast<Byte>(g[0] * y + g[1] * cb + g[2] * cr);
                    converted.Cr.at(x) = static_cast<Byte>(b[0] * y + b[1] * cb + b[2] * cr);
                }
            }
            break;
    }
    return converted;
}

// simple "Matrix" used by the subsampling method
struct Mat
{
    std::vector<Byte> row;
    bool scanline_jump = false;

    Mat() {}

    Mat(std::vector<Byte>&& r1, bool _scanline_jump)
        : row(std::move(r1))
    {
        scanline_jump = _scanline_jump;
    }
};

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

    Mat mat;

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
            mat = Mat{{1, 0}, false};

            vert_res_div = 1;
            hor_res_div = 2;
            break;
        case Image::S411:
            // x- --
            // x- --
            mat = Mat{{1, 0, 0, 0}, false};

            vert_res_div = 1;
            hor_res_div = 4;
            break;
        case Image::S420:
            // x- x-
            // -- --
            mat = Mat{{1, 0}, true};

            vert_res_div = 2;
            hor_res_div = 2;
            break;
        case Image::S420_m:
            // like S420, but taking the mean in vertical and horizontal direction
            // ++ ++
            // ++ ++
            mat = Mat{{1, 1}, false};

            vert_res_div = 2;
            hor_res_div = 2;
            averaging = true;
            break;
        case Image::S420_lm:
            // like S420, but taking the mean only in vertical direction
            // +- +-
            // +- +-
            mat = Mat{{1, 0}, false};

            vert_res_div = 2;
            hor_res_div = 2;
            averaging = true;
            break;
        default:
            break;
    }

    // subsampling Cr channel
    {
        Channel new_chan(Cr.width / hor_res_div, Cr.height / vert_res_div);
        auto& chan = Cr;

        // subsample Cr channel
        for (auto y = 0U; y < chan.height; y += 2) {
            for (auto x = 0U; x < chan.width; x += mat.row.size()) {
                auto pix_val = 0;
                for (auto m = 0U; m < mat.row.size(); ++m) {
                    pix_val += mat.row[m] * chan.at(x + m, y);
                }
                new_chan.pixels.push_back(pix_val);
            }

            if (!mat.scanline_jump) {
                // go through next scanline and average the pixels
                if (averaging) {
                    for (auto x = 0U; x < chan.width; x += mat.row.size()) {
                        auto pix_val = 0;
                        for (auto m = 0U; m < mat.row.size(); ++m) {
                            pix_val += mat.row[m] * chan.at(x + m, y + 1);
                        }
                        (new_chan.at(x, y) += (pix_val)) /= ((mode == S420_m) ? 4 : 2);
                    }
                }
                // go through next scanline
                else
                    --y;
            }
        }

        std::swap(chan, new_chan);
    }

    // subsample Cb channel
    {
        Channel new_chan(Cb.width / hor_res_div, Cb.height / vert_res_div);
        auto& chan = Cb;

        // subsample Cr channel
        for (auto y = 0U; y < chan.height; y += 2) {
            for (auto x = 0U; x < chan.width; x += mat.row.size()) {
                auto pix_val = 0;
                for (auto m = 0U; m < mat.row.size(); ++m) {
                    pix_val += mat.row[m] * chan.at(x + m, y);
                }
                new_chan.pixels.push_back(pix_val);
            }

            if (!mat.scanline_jump) {
                // go through next scanline and average the pixels
                if (averaging) {
                    for (auto x = 0U; x < chan.width; x += mat.row.size()) {
                        auto pix_val = 0;
                        for (auto m = 0U; m < mat.row.size(); ++m) {
                            pix_val += mat.row[m] * chan.at(x + m, y + 1);
                        }
                        (new_chan.at(x, y) += (pix_val)) /= ((mode == S420_m) ? 4 : 2);
                    }
                }
                // go through next scanline
                else
                    --y;
            }
        }

        std::swap(chan, new_chan);
    }
}

//
// PPM loading
//
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
        return file.at(file_pos++);
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
        img.R.pixels.push_back(std::stoi(buf));

        file.read_word(buf);
        img.G.pixels.push_back(std::stoi(buf));

        file.read_word(buf);
        img.B.pixels.push_back(std::stoi(buf));
    }
}

// Needs the ifstream opened as binary!
void loadP6PPM(PPMFileBuffer& ppm, Image& img) {
    const auto max = img.width * img.height;
    for (auto x = 0U; x < max; ++x) {
        img.R.pixels.push_back(ppm.read_byte());
        img.G.pixels.push_back(ppm.read_byte());
        img.B.pixels.push_back(ppm.read_byte());
    }
}

// top level load function with a path to a ppm file
Image loadPPM(std::string path) {
    std::ifstream::sync_with_stdio(false);
    std::ifstream ppm_file{path, std::fstream::binary};

    if (!ppm_file.is_open())
        throw std::runtime_error("Failed to open \"" + std::string(path) + "\"");

    std::string raw_buf{std::istreambuf_iterator<char>(ppm_file), std::istreambuf_iterator<char>()};
    ppm_file.close();

    PPMFileBuffer ppm{raw_buf};

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

    return img;
}