#pragma once

#include <unordered_map>

#include <boost/utility/string_view.hpp>

namespace request_handler::content_type {
using namespace std::literals;

constexpr static boost::string_view TEXT_HTML = "text/html";
constexpr static boost::string_view JSON = "application/json";
constexpr static boost::string_view CSS = "text/css";
constexpr static boost::string_view TEXT = "text/plain";
constexpr static boost::string_view JS = "text/javascript";
constexpr static boost::string_view XML = "application/xml";
constexpr static boost::string_view PNG = "image/png";
constexpr static boost::string_view JPG = "image/jpeg";
constexpr static boost::string_view GIF = "image/gif";
constexpr static boost::string_view BMP = "image/bmp";
constexpr static boost::string_view ICO = "image/vnd.microsoft.icon";
constexpr static boost::string_view TIFF = "image/tiff";
constexpr static boost::string_view SVG = "image/svg+xml";
constexpr static boost::string_view MP3 = "audio/mpeg";
constexpr static boost::string_view OCTET_STREAM = "application/octet-stream";

inline boost::string_view ExtensionToContentType(std::string extension) {
    static std::unordered_map<std::string, boost::string_view>
        extension_content_type{
            {".css", CSS},  {".htm", TEXT_HTML}, {".html", TEXT_HTML},
            {".txt", TEXT}, {".js", JS},         {".json", JSON},
            {".xml", XML},  {".png", PNG},       {".jpg", JPG},
            {".jpe", JPG},  {".jpeg", JPG},      {".gif", GIF},
            {".bmp", BMP},  {".ico", ICO},       {".tiff", TIFF},
            {".tif", TIFF}, {".svg", SVG},       {".svgz", SVG},
            {".mp3", MP3},
        };

    auto found_extension = extension_content_type.find(extension);
    if (found_extension == extension_content_type.end()) {
        return OCTET_STREAM;
    }
    return extension_content_type[extension];
}

}  // namespace request_handler::content_type
