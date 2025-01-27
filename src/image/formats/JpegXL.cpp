#include "JpegXL.hpp"

#include <filesystem>
#include <fstream>
#include <jxl/decode_cxx.h>
#include <vector>

std::expected<cairo_surface_t*, std::string> JXL::createSurfaceFromJXL(const std::string& path) {

    if (!std::filesystem::exists(path))
        return std::unexpected("loading jxl: file doesn't exist");

    std::ifstream file(path, std::ios::binary | std::ios::ate);
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit | std::ifstream::eofbit);
    std::vector<uint8_t> bytes(file.tellg());
    file.seekg(0);
    file.read(reinterpret_cast<char*>(bytes.data()), bytes.size());

    JxlSignature signature = JxlSignatureCheck(bytes.data(), bytes.size());
    if (signature != JXL_SIG_CODESTREAM && signature != JXL_SIG_CONTAINER)
        return std::unexpected("loading jxl: not jxl");

    auto dec    = JxlDecoderMake(nullptr);

    if (JXL_DEC_SUCCESS != JxlDecoderSubscribeEvents(dec.get(), JXL_DEC_BASIC_INFO | JXL_DEC_FULL_IMAGE))
        return std::unexpected("loading jxl: JxlDecoderSubscribeEvents failed");

    JxlDecoderSetInput(dec.get(), bytes.data(), bytes.size());
    JxlDecoderCloseInput(dec.get());
    if (JXL_DEC_BASIC_INFO != JxlDecoderProcessInput(dec.get()))
        return std::unexpected("loading jxl: JxlDecoderProcessInput failed");

    JxlBasicInfo basicInfo;
    if (JXL_DEC_SUCCESS != JxlDecoderGetBasicInfo(dec.get(), &basicInfo))
        return std::unexpected("loading jxl: JxlDecoderGetBasicInfo failed");

    auto cairoSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, basicInfo.xsize, basicInfo.ysize);
    if (cairo_surface_status(cairoSurface) != CAIRO_STATUS_SUCCESS) {
        cairo_surface_destroy(cairoSurface);
        return std::unexpected("loading jxl: cairo failed");
    }

    const auto     CAIRODATA = cairo_image_surface_get_data(cairoSurface);

    JxlPixelFormat format = {
        .num_channels = 4,
        .data_type    = JXL_TYPE_UINT8,
        .endianness   = JXL_LITTLE_ENDIAN,
        .align        = (size_t)cairo_image_surface_get_stride(cairoSurface),
    };

    const auto OUTPUTSIZE = basicInfo.xsize * basicInfo.ysize * format.num_channels;

    for (;;) {
        JxlDecoderStatus status = JxlDecoderProcessInput(dec.get());
        if (status == JXL_DEC_ERROR) {
            cairo_surface_destroy(cairoSurface);
            return std::unexpected("loading jxl: JxlDecoderProcessInput failed");
        } else if (status == JXL_DEC_NEED_MORE_INPUT) {
            cairo_surface_destroy(cairoSurface);
            return std::unexpected("loading jxl: JxlDecoderProcessInput expected more input");
        } else if (status == JXL_DEC_NEED_IMAGE_OUT_BUFFER) {
            size_t bufferSize;
            if (JXL_DEC_SUCCESS != JxlDecoderImageOutBufferSize(dec.get(), &format, &bufferSize)) {
                cairo_surface_destroy(cairoSurface);
                return std::unexpected("loading jxl: JxlDecoderImageOutBufferSize failed");
            }
            if (bufferSize != OUTPUTSIZE) {
                cairo_surface_destroy(cairoSurface);
                return std::unexpected("loading jxl: invalid output size");
            }
            if (JXL_DEC_SUCCESS != JxlDecoderSetImageOutBuffer(dec.get(), &format, CAIRODATA, bufferSize)) {
                cairo_surface_destroy(cairoSurface);
                return std::unexpected("loading jxl: JxlDecoderSetImageOutBuffer failed");
            }
        } else if (status == JXL_DEC_FULL_IMAGE) {
            for (size_t i = 0; i < OUTPUTSIZE - 2; i += format.num_channels) {
                std::swap(CAIRODATA[i + 0], CAIRODATA[i + 2]);
            }
            cairo_surface_mark_dirty(cairoSurface);
            cairo_surface_set_mime_data(cairoSurface, "image/jxl", bytes.data(), bytes.size(), nullptr, nullptr);
            return cairoSurface;
        }
    }

    return nullptr;
}
