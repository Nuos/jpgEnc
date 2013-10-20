#include "Image.hpp"


Image Image::convertToColorSpace(ColorSpace color_space_to) const {
    if (color_space == color_space_to) {
        return *this;
    }

    Image out(width, height, color_space_to);

    if (color_space_to == RGB) {
        assert(color_space == YCbCr);
        // YCbCr -> RGB
        for (int i=0; i < ch1.pixels.size(); ++i) {
            out.r.pixels[i] = static_cast<byte>(y.pixels[i] + 1.402*(cr.pixels[i] - 128));
            out.g.pixels[i] = static_cast<byte>(y.pixels[i] - 0.34414*(cb.pixels[i] - 128) - 0.71414*(cr.pixels[i] - 128));
            out.b.pixels[i] = static_cast<byte>(y.pixels[i] + 1.722*(cb.pixels[i] - 128));
        }
    }
    else if (color_space_to == YCbCr) {
        assert(color_space == RGB);
        // RGB -> YCbCr
        for (int i=0; i < ch1.pixels.size(); ++i) {
            out.y.pixels[i] =  static_cast<byte>(0.299*r.pixels[i] + 0.586*g.pixels[i] + 0.114*b.pixels[i]);
            out.cb.pixels[i] = 128 + static_cast<byte>(-0.168736*r.pixels[i] - 0.331264*g.pixels[i] + 0.5*b.pixels[i]);
            out.cr.pixels[i] = 128 + static_cast<byte>(0.5 * r.pixels[i] - 0.418688*g.pixels[i] - 0.081312*b.pixels[i]);
        }
    }
    else {
        assert(!"unsupported color space");
    }
    return out;
}


struct PPMFileBuffer
{
    std::string& file;
    std::string::size_type file_pos;
    std::string::size_type eof;

    PPMFileBuffer(std::string& file)
        : file(file),
        file_pos(0),
        eof(file.size()) {}

    byte read_byte() {
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

    byte discard_whitespace() {
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
        img.r.pixels[x] = (std::stoi(buf));

        file.read_word(buf);
        img.g.pixels[x] = (std::stoi(buf));

        file.read_word(buf);
        img.b.pixels[x] = (std::stoi(buf));
    }
}

// Needs the ifstream opened as binary!
void loadP6PPM(PPMFileBuffer& ppm, Image& img) {
    const auto max = img.width * img.height;
    for (auto x = 0U; x < max; ++x) {
        img.r.pixels[x] = (ppm.read_byte());
        img.g.pixels[x] = (ppm.read_byte());
        img.b.pixels[x] = (ppm.read_byte());
    }
}

// top level load function with a path to a ppm file
Image loadPPM(std::string path) {
    std::ifstream::sync_with_stdio(false);
    std::ifstream ppm_file(path, std::fstream::binary);

    if (!ppm_file.is_open())
        throw std::runtime_error("Failed to open \"" + std::string(path) + "\"");

    std::string raw_buf = std::string(std::istreambuf_iterator<char>(ppm_file), std::istreambuf_iterator<char>());
    ppm_file.close();

    PPMFileBuffer ppm(raw_buf);

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