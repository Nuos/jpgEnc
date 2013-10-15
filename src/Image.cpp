#include "Image.hpp"


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

    Image img(width, height, ColorSpace::RGB);

    if (magic == "P3")
        loadP3PPM(ppm, img);
    else if (magic == "P6") {
        loadP6PPM(ppm, img);
    }

    return img;
}