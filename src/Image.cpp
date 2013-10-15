#include "Image.hpp"

Image Image::convertToColorSpace(ColorSpace target_space) const {
    // matrix factors
    static const float Flat[] { .0f, 256/2.f, 256/2.f };
    static const float Y[] {   .299f,   .587f,   .114f };
    static const float Cb[] { -.1687f, -.3312f,  .5f };
    static const float Cr[] {  .5f,    -.4186f, -.0813f };

    Image converted(*this);
    auto max = converted.width * converted.height;
    for (uint x = 0; x < max; ++x) {
        auto& r = R.at(x);
        auto& g = G.at(x);
        auto& b = B.at(x);

        converted.Y.at(x)  = static_cast<Byte>(Flat[0] + ( Y[0] * r +  Y[1] * g +  Y[2] * b));
        converted.Cb.at(x) = static_cast<Byte>(Flat[1] + (Cb[0] * r + Cb[1] * g + Cb[2] * b));
        converted.Cr.at(x) = static_cast<Byte>(Flat[2] + (Cr[0] * r + Cr[1] * g + Cr[2] * b));
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