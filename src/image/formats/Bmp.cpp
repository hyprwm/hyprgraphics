#include "Bmp.hpp"

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <filesystem>
#include <optional>
#include <fstream>
#include <vector>
#include <cstring>

class BmpHeader {
  public:
    unsigned char              format[2];
    uint32_t                   sizeOfFile;
    uint16_t                   reserved1;
    uint16_t                   reserved2;
    uint32_t                   dataOffset;
    uint32_t                   sizeOfBitmapHeader;
    uint32_t                   width;
    uint32_t                   height;
    uint16_t                   numberOfColors;
    uint16_t                   numberOfBitPerPixel;
    uint32_t                   compressionMethod;
    uint32_t                   imageSize;
    uint32_t                   horizontalResolutionPPM;
    uint32_t                   verticalResolutionPPM;
    uint32_t                   numberOfCollors;
    uint32_t                   numberOfImportantCollors;

    std::optional<std::string> load(std::ifstream& file) {
        file.seekg(0, std::ios::end);
        uint32_t streamLength = file.tellg();
        file.seekg(0, std::ios::beg);

        file.read(reinterpret_cast<char*>(&format), sizeof(format));
        if (format[0] != 66 || format[1] != 77)
            return "Unable to parse bitmap header: wrong bmp file type";

        file.read(reinterpret_cast<char*>(&sizeOfFile), sizeof(sizeOfFile));

        if (sizeOfFile != streamLength)
            return "Unable to parse bitmap header: wrong value of file size header";

        file.read(reinterpret_cast<char*>(&reserved1), sizeof(reserved1));
        file.read(reinterpret_cast<char*>(&reserved2), sizeof(reserved2));
        file.read(reinterpret_cast<char*>(&dataOffset), sizeof(dataOffset));
        file.read(reinterpret_cast<char*>(&sizeOfBitmapHeader), sizeof(sizeOfBitmapHeader));
        file.read(reinterpret_cast<char*>(&width), sizeof(width));
        file.read(reinterpret_cast<char*>(&height), sizeof(height));
        file.read(reinterpret_cast<char*>(&numberOfColors), sizeof(numberOfColors));
        file.read(reinterpret_cast<char*>(&numberOfBitPerPixel), sizeof(numberOfBitPerPixel));
        file.read(reinterpret_cast<char*>(&compressionMethod), sizeof(compressionMethod));
        file.read(reinterpret_cast<char*>(&imageSize), sizeof(imageSize));
        file.read(reinterpret_cast<char*>(&horizontalResolutionPPM), sizeof(horizontalResolutionPPM));
        file.read(reinterpret_cast<char*>(&verticalResolutionPPM), sizeof(verticalResolutionPPM));
        file.read(reinterpret_cast<char*>(&numberOfCollors), sizeof(numberOfCollors));
        file.read(reinterpret_cast<char*>(&numberOfImportantCollors), sizeof(numberOfImportantCollors));

        if (!imageSize)
            imageSize = sizeOfFile - dataOffset;

        if (imageSize != (width * height * numberOfBitPerPixel / 8))
            return "Unable to parse bitmap header: wrong image size";

        file.seekg(dataOffset);

        return std::nullopt;
    };
};

static void reflectImage(unsigned char* image, uint32_t numberOfRows, int stride) {
    int                        rowStart = 0;
    int                        rowEnd   = numberOfRows - 1;
    std::vector<unsigned char> temp;
    temp.resize(stride);
    while (rowStart < rowEnd) {
        memcpy(&temp[0], &image[rowStart * stride], stride);
        memcpy(&image[rowStart * stride], &image[rowEnd * stride], stride);
        memcpy(&image[rowEnd * stride], &temp[0], stride);
        rowStart++;
        rowEnd--;
    }
};

static void convertRgbToArgb(std::ifstream& imageStream, unsigned char* outputImage, uint32_t newImageSize) {
    uint8_t       forthBitCounter = 0;
    unsigned long imgCursor       = 0;
    while (imgCursor < newImageSize) {
        imageStream.read(reinterpret_cast<char*>(&outputImage[imgCursor]), 1);
        imgCursor++;
        forthBitCounter++;
        if (forthBitCounter == 3) {
            outputImage[imgCursor] = 0;
            imgCursor++;
            forthBitCounter = 0;
        }
    }
};

std::expected<cairo_surface_t*, std::string> BMP::createSurfaceFromBMP(const std::string& path) {
    if (!std::filesystem::exists(path))
        return std::unexpected("loading bmp: file doesn't exist");

    std::ifstream bitmapImageStream(path);
    BmpHeader     bitmapHeader;
    if (const auto RET = bitmapHeader.load(bitmapImageStream); RET.has_value())
        return std::unexpected("loading bmp: " + *RET);

    cairo_format_t format    = CAIRO_FORMAT_ARGB32;
    int            stride    = cairo_format_stride_for_width(format, bitmapHeader.width);
    unsigned char* imageData = (unsigned char*)malloc(bitmapHeader.height * stride);

    if (bitmapHeader.numberOfBitPerPixel == 24)
        convertRgbToArgb(bitmapImageStream, imageData, bitmapHeader.height * stride);
    else if (bitmapHeader.numberOfBitPerPixel == 32)
        bitmapImageStream.read(reinterpret_cast<char*>(&imageData), bitmapHeader.imageSize);
    else {
        bitmapImageStream.close();
        return std::unexpected("loading bmp: unsupported bmp stream");
    }
    bitmapImageStream.close();
    reflectImage(imageData, bitmapHeader.height, stride);
    return cairo_image_surface_create_for_data(imageData, format, bitmapHeader.width, bitmapHeader.height, stride);
}
